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
 >					Online Building and Editing Module						<
\****************************************************************************/

#include <ctype.h>
#include <errno.h>

#include "mud.h"
#include "admin.h"
#include "affect.h"
#include "build.h"
#include "fread.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "interpret.h"
#include "movement.h"
#include "mprog.h"
#include "reset.h"
#include "room.h"
#include "shop.h"
#include "special.h"


/*
 * Variabili
 */
REL_DATA	*first_relation = NULL;
REL_DATA	*last_relation  = NULL;


/*
 * Parte di codice che era identico in can_modify_room, can_object_modify e can_modify_mob.
 */
bool can_modify_area( CHAR_DATA *ch, VNUM vnum )
{
	AREA_DATA *pArea;

	if ( !ch->pg || !(pArea = ch->pg->build_area) )
	{
		send_to_char( ch, "You must have an assigned area to modify this room.\r\n" );
		return FALSE;
	}

	if ( vnum >= pArea->vnum_low && vnum <= pArea->vnum_high )
		return TRUE;

	return FALSE;
}


bool can_modify_room( CHAR_DATA *ch, ROOM_DATA *room )
{
	VNUM	vnum;

	if ( IS_MOB(ch) )
		return FALSE;

	if ( get_trust(ch) >= TRUST_BUILDER )
		return TRUE;

	vnum = room->vnum;
	if ( can_modify_area(ch, vnum) )
		return TRUE;

	send_to_char( ch, "Questa stanza non è nel tuo range di vnum.\r\n" );
	return FALSE;
}


bool can_modify_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	VNUM	vnum;

	if ( IS_MOB(ch) )
		return FALSE;

	if ( get_trust(ch) >= TRUST_BUILDER )
		return TRUE;

	vnum = obj->vnum;
	if ( can_modify_area(ch, vnum) )
		return TRUE;

	send_to_char( ch, "Questo oggetto non è nel tuo range di vnum.\r\n" );
	return FALSE;
}


bool can_modify_mob( CHAR_DATA *ch, CHAR_DATA *mob )
{
	VNUM	vnum;

	if ( mob == ch )
		return TRUE;

	if ( IS_MOB(ch) )
		return FALSE;

	if ( IS_PG(mob) )
	{
		if ( get_trust(ch) < TRUST_BUILDER )
			return FALSE;

		/* Controllo di trust nel caso che un admin stia utilizzando un mob */
		if ( get_trust(ch) > get_trust(mob) )
		{
			return TRUE;
		}
		else
		{
			send_to_char( ch, "You can't do that.\r\n" );
			return FALSE;
		}
	}

	if ( get_trust(ch) >= TRUST_MASTER )
		return TRUE;

	vnum = mob->pIndexData->vnum;
	if ( can_modify_area(ch, vnum) )
		return TRUE;

	send_to_char( ch, "Questo mob non è nel tuo range di vnum.\r\n" );
	return FALSE;
}


/*
 * Controlla se ch può editare un mob indicizzato
 */
bool can_edit_mob( CHAR_DATA *ch, MOB_PROTO_DATA *imob )
{
	VNUM	   vnum;

	if ( IS_MOB(ch) )
		return FALSE;

	if ( get_trust(ch) >= TRUST_BUILDER )
		return TRUE;

	vnum = imob->vnum;
	if ( can_modify_area(ch, vnum) )
		return TRUE;

	send_to_char( ch, "That mobile is not in your allocated range.\r\n" );
	return FALSE;
}


EXTRA_DESCR_DATA *SetRExtra( ROOM_DATA *room, char *keywords )
{
	EXTRA_DESCR_DATA *ed;

	for ( ed = room->first_extradescr;  ed;  ed = ed->next )
	{
		if ( is_name(keywords, ed->keyword) )
			break;
	}

	if ( !ed )
	{
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword	= str_dup( keywords );
		ed->description	= str_dup( "" );
		LINK( ed, room->first_extradescr, room->last_extradescr, next, prev );
		top_ed++;
	}

	return ed;
}


bool DelRExtra( ROOM_DATA *room, char *keywords )
{
	EXTRA_DESCR_DATA *rmed;

	for ( rmed = room->first_extradescr;  rmed;  rmed = rmed->next )
	{
		if ( is_name( keywords, rmed->keyword ) )
			break;
	}

	if ( !rmed )
		return FALSE;
	UNLINK( rmed, room->first_extradescr, room->last_extradescr, next, prev );
	DISPOSE( rmed->keyword );
	DISPOSE( rmed->description );
	DISPOSE( rmed );
	top_ed--;
	return TRUE;
}


EXTRA_DESCR_DATA *set_obj_extra( OBJ_DATA *obj, char *keywords )
{
	EXTRA_DESCR_DATA *ed;

	for ( ed = obj->first_extradescr;  ed;  ed = ed->next )
	{
		if ( is_name(keywords, ed->keyword) )
			break;
	}

	if ( !ed )
	{
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		LINK( ed, obj->first_extradescr, obj->last_extradescr, next, prev );
		ed->keyword	= str_dup( keywords );
		ed->description	= str_dup( "" );
		top_ed++;
	}

	return ed;
}


/*
 * Nel caso che un Admin vi chieda come fare a togliere delle extra di un oggetto
 *	le cui extra fanno riferimento ad un oggetto indicizzato l'unica soluzione per ora
 *	è quella di aggiungere una extra con lo stessa keyword di quella dell'oggetto
 *	indicizzato che si vorrebbe togliere.
 */
bool del_obj_extra( OBJ_DATA *obj, char *keywords )
{
	EXTRA_DESCR_DATA *rmed;

	for ( rmed = obj->first_extradescr;  rmed;  rmed = rmed->next )
	{
		if ( is_name(keywords, rmed->keyword) )
			break;
	}

	if ( !rmed )
		return FALSE;

	UNLINK( rmed, obj->first_extradescr, obj->last_extradescr, next, prev );
	DISPOSE( rmed->keyword );
	DISPOSE( rmed->description );
	DISPOSE( rmed );
	top_ed--;

	return TRUE;
}


/*
 * Relations created to fix a crash bug with oset on and rset on code by: gfinello@mail.karmanet.it
 */
void RelCreate( relation_type tp, void *actor, void *subject )
{
	REL_DATA *tmp;

	if ( tp < relMRESTRING_ON || tp > relORESTRING_ON )
	{
		send_log( NULL, LOG_BUG, "RelCreate: invalid type (%d)", tp );
		return;
	}

	if ( !actor )
	{
		send_log( NULL, LOG_BUG, "RelCreate: NULL actor" );
		return;
	}

	if ( !subject )
	{
		send_log( NULL, LOG_BUG, "RelCreate: NULL subject" );
		return;
	}

	for ( tmp = first_relation; tmp; tmp = tmp->next )
	{
		if ( tmp->type == tp && tmp->actor == actor && tmp->subject == subject )
		{
			send_log( NULL, LOG_BUG, "RelCreate: duplicated relation" );
			return;
		}
		CREATE( tmp, REL_DATA, 1 );
		tmp->type = tp;
		tmp->actor = actor;
		tmp->subject = subject;
		LINK( tmp, first_relation, last_relation, next, prev );
	}
}


/*
 * Relations created to fix a crash bug with oset on and rset on code by: gfinello@mail.karmanet.it
 */
void RelDestroy( relation_type tp, void *actor, void *subject )
{
	REL_DATA * rq;

	if ( tp < relMRESTRING_ON || tp > relORESTRING_ON )
	{
		send_log( NULL, LOG_BUG, "RelDestroy: invalid type (%d)", tp );
		return;
	}

	if ( !actor )
	{
		send_log( NULL, LOG_BUG, "RelDestroy: NULL actor" );
		return;
	}

	if ( !subject )
	{
		send_log( NULL, LOG_BUG, "RelDestroy: NULL subject" );
		return;
	}

	for ( rq = first_relation; rq; rq = rq->next )
	{
		if ( rq->type == tp && rq->actor == actor && rq->subject == subject )
		{
			UNLINK(  rq, first_relation, last_relation, next, prev );
			/* Dispose will also set to NULL the passed parameter */
			DISPOSE( rq );
			break;
		}
	}
}


DO_RET do_mrestring( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	char	   *origarg = argument;
	char		arg1[MIL];
	char		arg2[MIL];
	char		arg3[MIL];
	char		buf[MSL];
	bool		lockvictim;

	set_char_color( AT_PLAIN, ch );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Mob's can't mrestring\r\n" );
		return;
	}

	if ( !ch->desc )
	{
		send_to_char( ch, "You have no descriptor\r\n" );
		return;
	}

	switch ( ch->substate )
	{
	  default:
		break;

	  case SUB_MOB_DESCR:
		if ( !ch->dest_buf )
		{
			send_to_char( ch, "do_mrestring: errore fatale, informare il Coder.\r\n" );
			send_log( NULL, LOG_BUILD, "do_restring: ch->dest_buf è NULL." );
			ch->substate = SUB_NONE;
			return;
		}

		victim = ch->dest_buf;
		if ( char_died(victim) )
		{
			send_to_char( ch, "Your victim died!\r\n" );
			stop_editing(ch);
			return;
		}

		DISPOSE( victim->description );
		victim->description = copy_buffer( ch );

		stop_editing( ch );
		ch->substate = ch->tempnum;
		return;
	}

	victim = NULL;
	lockvictim = FALSE;
	smash_tilde( argument );

	if ( ch->substate == SUB_REPEAT_CMD )
	{
		victim = ch->dest_buf;

		if ( !victim )
		{
			send_to_char( ch, "La tua vittima non c'è più!\r\n" );
			argument = "done";
		}

		if ( !VALID_STR(argument) || !str_cmp(argument, "stat") )
		{
			if ( victim )
			{
				sprintf( buf, "mstat %s", victim->name );
				send_command( ch, buf, CO );
			}
			else
			{
				send_to_char( ch, "No victim selected.  Type '?' for help.\r\n" );
			}

			return;
		}

		if ( !str_cmp(argument, "fatto") || !str_cmp(argument, "no")
		  || !str_cmp(argument, "done")  || !str_cmp(argument, "off") )
		{
			if ( ch->dest_buf )
				RelDestroy( relMRESTRING_ON, ch, ch->dest_buf );

			send_to_char( ch, "MRestring mode off.\r\n" );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;

			if ( ch->pg && ch->pg->subprompt )
			{
				DISPOSE( ch->pg->subprompt );
				ch->pg->subprompt = NULL;
			}
			return;
		}
	} /* chiude l'if */

	if ( victim )
	{
		lockvictim = TRUE;
		strcpy( arg1, victim->name );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}
	else
	{
		lockvictim = FALSE;
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}

	if ( !str_cmp(arg1, "si") || !str_cmp(arg1, "on") )
	{
		send_to_char( ch, "&YSintassi&w: mrestring <victim|vnum> on.\r\n" );
		return;
	}

	if (   !VALID_STR(arg1)
	  || ((!VALID_STR(arg2)) && ch->substate != SUB_REPEAT_CMD)
	  || arg1[0] == '?')
	{
		if ( ch->substate == SUB_REPEAT_CMD )
		{
			if ( victim )
				send_to_char( ch, "&YSintassi&w: <field>  <value>\r\n" );
			else
				send_to_char( ch, "&YSintassi&w: <victim> <field>  <value>\r\n" );
		}
		else
		{
			send_to_char( ch, "&YSintassi&w: mrestring <victim> <field>  <value>\r\n" );
		}

		send_to_char( ch, "\r\n"								);
		send_to_char( ch, "Field being one of:\r\n"				);
		send_to_char( ch, "  name short long description\r\n"	);

		return;
	}

	if ( !victim )
	{
		victim = get_char_mud( ch, arg1, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "No one like that in all the realms.\r\n" );
			return;
		}
	}

	if ( IS_PG(victim) && get_trust(ch) < get_trust(victim) )
	{
		send_to_char( ch, "You can't do that!\r\n" );
		ch->dest_buf = NULL;
		return;
	}

	if ( lockvictim )
		ch->dest_buf = victim;

	if ( !str_cmp(arg2, "si") || !str_cmp(arg2, "on") )
	{
		CHECK_SUBRESTRICTED( ch );
		ch_printf( ch, "Mrestring mode on. (Editing %s).\r\n", victim->name );
		ch->substate = SUB_REPEAT_CMD;
		ch->dest_buf = victim;

		if ( ch->pg )
		{
			if ( IS_MOB(victim) )
				sprintf( buf, "<&Cmrestring &W#%d&w> %%i", victim->pIndexData->vnum );
			else
				sprintf( buf, "<&Cmrestring &W%s&w> %%i", victim->name );

			DISPOSE( ch->pg->subprompt );
			ch->pg->subprompt = str_dup( buf );
		}
		RelCreate( relMRESTRING_ON, ch, victim );
		return;
	}

	if ( !str_cmp(arg2, "name") || !str_cmp(arg2, "nome") )
	{
		if ( IS_PG(victim) )
		{
			send_to_char( ch, "Not on PC's. Se hai abbastanza trust utilizza rename\r\n" );
			return;
		}

		if ( !VALID_STR(arg3) )
		{
			send_to_char( ch, "Names can not be set to an empty string.\r\n" );
			return;
		}

		DISPOSE( victim->name );
		victim->name = str_dup( arg3 );
		return;
	}

	if ( !str_cmp(arg2, "short") )
	{
		DISPOSE( victim->short_descr );
		victim->short_descr = str_dup( arg3 );
		return;
	}

	if ( !str_cmp(arg2, "long") )
	{
		DISPOSE( victim->long_descr );
		strcpy( buf, arg3 );
		strcat( buf, "\r\n" );
		victim->long_descr = str_dup( buf );
		return;
	}

	if ( !str_cmp(arg2, "description") )
	{
		if ( VALID_STR(arg3) )
		{
			DISPOSE( victim->description );
			victim->description = str_dup( arg3 );
			return;
		}

		CHECK_SUBRESTRICTED( ch );
		if ( ch->substate == SUB_REPEAT_CMD )
			ch->tempnum = SUB_REPEAT_CMD;
		else
			ch->tempnum = SUB_NONE;

		ch->substate = SUB_MOB_DESCR;
		ch->dest_buf = victim;
		start_editing( ch, victim->description );
		return;
	}

	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEAT_CMD )
	{
		ch->substate = SUB_RESTRICTED;
		/* Si suppone che ch abbia inviato argument nella sua lingua quindi niente send_command() */
		interpret( ch, origarg );
		ch->substate = SUB_REPEAT_CMD;
		ch->last_cmd = do_mrestring;
	}
	else
	{
		send_command( ch, "mrestring", CO );
	}
}


/*
 * Comando di edit per fare i restring agli oggetti, deriva da do_orestring
 *	è più comodo di ooedit
 */
DO_RET do_orestring( CHAR_DATA *ch, char *argument )
{
	EXTRA_DESCR_DATA *ed;
	OBJ_DATA		 *obj;
	OBJ_DATA		 *tmpobj;
	char			  arg1[MIL];
	char			  arg2[MIL];
	char			  arg3[MIL];
	char			  buf [MSL];
	char			 *origarg = argument;
	int				  value;
	bool			  lockobj;

	set_char_color( AT_PLAIN, ch );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Mob's can't restring\r\n" );
		return;
	}

	if ( !ch->desc )
	{
		send_to_char( ch, "You have no descriptor\r\n" );
		return;
	}

	switch ( ch->substate )
	{
	  default:
		break;

	  case SUB_OBJ_EXTRA:
		if ( !ch->dest_buf )
		{
			send_to_char( ch, "Fatal error: report to Coder.\r\n" );
			send_log( NULL, LOG_BUILD, "do_orestring: sub_obj_extra: NULL ch->dest_buf" );
			ch->substate = SUB_NONE;
			return;
		}

		/* Hopefully the object didn't get extracted...
		 * If you're REALLY paranoid, you could always go through
		 *  the object and index-object lists, searching through the
		 *  extra_descr lists for a matching pointer...
		 */
		ed  = ch->dest_buf;
		DISPOSE( ed->description );
		ed->description = copy_buffer( ch );
		tmpobj = ch->spare_ptr;
		stop_editing( ch );
		ch->dest_buf = tmpobj;
		ch->substate = ch->tempnum;
		return;

	  case SUB_OBJ_LONG:
		if ( !ch->dest_buf )
		{
			send_to_char( ch, "Fatal error: report to Coder.\r\n" );
			send_log( NULL, LOG_BUILD, "do_orestring: sub_obj_long: NULL ch->dest_buf" );
			ch->substate = SUB_NONE;
			return;
		}
		obj = ch->dest_buf;

		if ( obj && obj_extracted(obj) )
		{
			send_to_char( ch, "Your object was extracted!\r\n" );
			stop_editing( ch );
			return;
		}
		DISPOSE( obj->long_descr );
		obj->long_descr = copy_buffer( ch );

		tmpobj = ch->spare_ptr;
		stop_editing( ch );
		ch->substate = ch->tempnum;
		ch->dest_buf = tmpobj;
		return;
	}

	obj = NULL;
	smash_tilde( argument );

	if ( ch->substate == SUB_REPEAT_CMD )
	{
		obj = ch->dest_buf;

		if ( !obj )
		{
			send_to_char( ch, "Il tuo oggetto è stato estratto!\r\n" );
			argument = "done";
		}

		if ( !VALID_STR(argument) || !str_cmp(argument, "stat") )
		{
			if ( obj )
			{
				sprintf( buf, "ostat %s", obj->name );
				send_command( ch, buf, CO );
			}
			else
				send_to_char( ch, "No object selected.  Type '?' for help.\r\n" );
			return;
		}

		if ( !str_cmp(argument, "fatto") || !str_cmp(argument, "no")
		  || !str_cmp(argument, "done")  || !str_cmp(argument, "off") )
		{
			if ( ch->dest_buf )
				RelDestroy( relORESTRING_ON, ch, ch->dest_buf );

			send_to_char( ch, "Oset mode off.\r\n" );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;

			if ( ch->pg && ch->pg->subprompt )
			{
				DISPOSE( ch->pg->subprompt );
				ch->pg->subprompt = NULL;
			}
			return;
		}
	}

	if ( obj )
	{
		lockobj = TRUE;
		strcpy( arg1, obj->name );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}
	else
	{
		lockobj = FALSE;
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}

	if ( !str_cmp(arg1, "si") || !str_cmp(arg1, "on") )
	{
		send_to_char( ch, "&YSintassi&w: oset <object|vnum> on.\r\n" );
		return;
	}

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || arg1[0] == '?' )
	{
		if ( ch->substate == SUB_REPEAT_CMD )
		{
			if ( obj )
				send_to_char( ch, "&YSintassi&w: <field>  <value>\r\n" );
			else
				send_to_char( ch, "&YSintassi&w: <object> <field>  <value>\r\n" );
		}
		else
		{
			send_to_char( ch, "&YSintassi&w: oset <object> <field>  <value>\r\n" );
		}

		send_to_char( ch, "\r\n"													);
		send_to_char( ch, "Field being one of:\r\n"									);
		send_to_char( ch, "  name short long ed rmed actiondesc\r\n"				);
		return;
	}

	if ( !obj && get_trust(ch) < TRUST_BUILDER )
	{
		obj = get_obj_here( ch, arg1 );
		if ( !obj )
		{
			send_to_char( ch, "You can't find that here.\n\r" );
			return;
		}
	}
	else if ( !obj )
	{
		obj = get_obj_world( ch, arg1 );
		if ( !obj )
		{
			send_to_char( ch, "There is nothing like that in all the realms.\r\n" );
			return;
		}
	}

	if ( lockobj )
		ch->dest_buf = obj;
	else
		ch->dest_buf = NULL;

	split_obj( obj, 1 );
	value = atoi( arg3 );

	if ( !str_cmp(arg2, "si") || !str_cmp(arg2, "on") )
	{
		CHECK_SUBRESTRICTED(ch);
		ch_printf( ch, "Oset mode on. (Editing '%s' vnum %d).\r\n", obj->name, obj->vnum );
		ch->substate = SUB_REPEAT_CMD;
		ch->dest_buf = obj;

		if ( ch->pg )
		{
			sprintf( buf, "<&COset &W#%d&w> %%i", obj->vnum );
			DISPOSE( ch->pg->subprompt );
			ch->pg->subprompt = str_dup( buf );
		}
		RelCreate( relORESTRING_ON, ch, obj );
		return;
	}

	if ( !str_cmp(arg2, "name") || !str_cmp(arg2, "nome") )
	{
		DISPOSE( obj->name );
		obj->name = str_dup( arg3 );

		return;
	}

	if ( !str_cmp(arg2, "short") )
	{
		 /* Add the word 'rename' to the keywords if it is not already there. */
		DISPOSE( obj->short_descr );
		obj->short_descr = str_dup( arg3 );
		return;
	}

	if ( !str_cmp(arg2, "long") )
	{
		if ( VALID_STR(arg3) )
		{
			DISPOSE( obj->long_descr );
			obj->long_descr = str_dup( arg3 );
			return;
		}
		CHECK_SUBRESTRICTED( ch );

		if ( ch->substate == SUB_REPEAT_CMD )
			ch->tempnum = SUB_REPEAT_CMD;
		else
			ch->tempnum = SUB_NONE;

		if ( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;

		ch->substate = SUB_OBJ_LONG;
		ch->dest_buf = obj;
		start_editing( ch, obj->long_descr );
		return;
	}

	if ( !str_cmp(arg2, "ed") )
	{
		if ( !VALID_STR(arg3) )
		{
			send_to_char( ch, "&YSintassi&w: oset <object> ed <keywords>\r\n" );
			return;
		}

		CHECK_SUBRESTRICTED( ch );

		if ( obj->timer )
		{
			send_to_char( ch, "It's not safe to edit an extra description on an object with a timer.\r\nTurn it off first.\r\n" );
			return;
		}

		if ( obj->type == OBJTYPE_PAPER && get_trust(ch) < TRUST_MASTER )
		{
			send_to_char( ch, "You can not add an extra description to a note paper at the moment.\n\r" );
			return;
		}

		ed = set_obj_extra( obj, arg3 );

		if ( ch->substate == SUB_REPEAT_CMD )
			ch->tempnum = SUB_REPEAT_CMD;
		else
			ch->tempnum = SUB_NONE;

		if ( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;

		ch->substate = SUB_OBJ_EXTRA;
		ch->dest_buf = ed;
		start_editing( ch, ed->description );
		return;
	}

	if ( is_name(arg2, "rmed red") )
	{
		if ( !VALID_STR(arg3) )
		{
			send_to_char( ch, "&YSintassi&w: oset <object> rmed <keywords>\r\n" );
			return;
		}

		if ( del_obj_extra(obj, arg3) )
			send_to_char( ch, "Deleted.\r\n" );
		else
			send_to_char( ch, "Not found.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "actiondesc") )
	{
		if ( !can_modify_obj( ch, obj ) )
			return;
		if ( strstr(arg3, "%n")
		  || strstr(arg3, "%d")
		  || strstr(arg3, "%l") )
		{
			send_to_char( ch, "Illegal characters!\r\n" );
			return;
		}
		DISPOSE( obj->action_descr );
		obj->action_descr = str_dup( arg3 );
		return;
	}

	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEAT_CMD )
	{
		ch->substate = SUB_RESTRICTED;
		/* Si suppone che ch abbia inviato argument nella sua lingua quindi niente send_command() */
		interpret( ch, origarg );
		ch->substate = SUB_REPEAT_CMD;
		ch->last_cmd = do_orestring;
	}
	else
	{
		send_command( ch, "orestring", CO );
	}
}


/*
 * Ritorna da 0 a 9 in base alla stringa passata rappresentate la direzione
 */
int get_dir( const char *txt )
{
	int		edir;
	char	c1;
	char	c2;

	if ( !str_cmp(txt, "northeast") || !str_cmp(txt, "nordest"	) )		return DIR_NORTHEAST;
	if ( !str_cmp(txt, "northwest") || !str_cmp(txt, "nordovest") )		return DIR_NORTHWEST;
	if ( !str_cmp(txt, "southeast") || !str_cmp(txt, "sudest"	) )		return DIR_SOUTHEAST;
	if ( !str_cmp(txt, "southwest") || !str_cmp(txt, "sudovest" ) )		return DIR_SOUTHWEST;
	if ( !str_cmp(txt, "somewhere") || !str_cmp(txt, "altrove"	) )		return DIR_SOMEWHERE;

	c1 = txt[0];
	if ( c1 == '\0' )
		return 0;

	c2 = txt[1];
	edir = 0;
	switch ( c1 )
	{
	  case 'n':
		switch ( c2 )
		{
		  default:					edir = 0;	break;		/* nord		*/
		  case 'e':					edir = 6;	break;		/* ne		*/
		  case 'w': case 'o':		edir = 7;	break;		/* nw - no	*/
	}
		break;

	  case '0':						edir = 0;	break;		/* nord		*/
	  case 'e': case '1':			edir = 1;	break;		/* est		*/

	  case 's':
		switch ( c2 )
		{
		  default:					edir = 2;	break;		/* sud		*/
		  case 'e':					edir = 8;	break;		/* se		*/
		  case 'w': case 'o':		edir = 9;	break;		/* sw - so	*/
		}
		break;

	  case '2':						edir = 2;	break;		/* sud		*/
	  case 'w': case 'o': case '3':	edir = 3;	break;		/* west - ovest	*/

	  case 'u': case 'a': case '4': edir = 4;	break;		/* up - alto	*/
	  case 'd': case 'b': case '5':	edir = 5;	break;		/* down - basso	*/

	  case '6':						edir = 6;	break;		/* ne	 */
	  case '7':						edir = 7;	break;		/* no	 */
	  case '8':						edir = 8;	break;		/* se	 */
	  case '9':						edir = 9;	break;		/* so	 */

	  case '?':						edir = 10;	break;		/* altrove */
	}

	return edir;
}


char *sprint_reset( CHAR_DATA *ch, RESET_DATA *pReset, int num, bool rlist )
{
	static ROOM_DATA 	   *room;
	static OBJ_PROTO_DATA  *obj;
	static OBJ_PROTO_DATA  *obj2;
	static MOB_PROTO_DATA  *mob;
	static char				buf		[MSL];
	char					mobname [MSL];
	char					roomname[MSL];
	char					objname [MSL];
	VNUM					rvnum = -1;

	if ( ch->in_room )
		rvnum = ch->in_room->vnum;

	if ( num == 1 )
	{
		room = NULL;
		obj  = NULL;
		obj2 = NULL;
		mob  = NULL;
	}

	switch ( pReset->command )
	{
	  default:
		sprintf( buf, "%2d) *** BAD RESET: %c %d %d %d %d ***\r\n",
			num,
			pReset->command,
			pReset->extra,
			pReset->arg1,
			pReset->arg2,
			pReset->arg3 );
		break;

	  case 'M':
		mob = get_mob_index( NULL, pReset->arg1 );
		room = get_room_index( NULL, pReset->arg3 );

		if ( mob )
			strcpy( mobname, mob->name );
		else
			strcpy( mobname, "Mobile: *BAD VNUM*" );

		if ( room )
			strcpy( roomname, room->name );
		else
			strcpy( roomname, "Room: *BAD VNUM*" );

		sprintf( buf, "%2d) %s (%d) -> %s (%d) [%d]\r\n",
			num,
			mobname,
			pReset->arg1,
			roomname,
			pReset->arg3,
			pReset->arg2 );
		break;

	  case 'E':
		if ( !mob )
			strcpy( mobname, "* ERROR: NO MOBILE! *" );

		obj = get_obj_proto( NULL, pReset->arg1 );
		if ( !obj )
			strcpy( objname, "Object: *BAD VNUM*" );
		else
			strcpy( objname, obj->name );

		sprintf( buf, "%2d) %s (%d) -> %s (%s) [%d]\r\n",
			num,
			objname,
			pReset->arg1,
			mobname,
			code_name(NULL, pReset->arg3, CODE_WEARLOC),
			pReset->arg2 );
		break;

	  case 'H':
		obj = get_obj_proto( NULL, pReset->arg1 );
		if ( !obj )
		{
			if ( pReset->arg1 > 0 )
				strcpy( objname, "Object: *BAD VNUM*" );
			else
				strcpy( objname, "Object: *NULL obj*" );
		}

		sprintf( buf, "%2d) Hide %s (%d)\r\n", num, objname, (obj)  ?  obj->vnum  :  pReset->arg1 );
		break;

	  case 'G':
		if ( !mob )
			strcpy( mobname, "* ERROR: NO MOBILE! *" );

		obj = get_obj_proto( NULL, pReset->arg1 );
		if ( !obj )
			strcpy( objname, "Object: *BAD VNUM*" );
		else
			strcpy( objname, obj->name );

		sprintf( buf, "%2d) %s (%d) -> %s (carry) [%d]\r\n",
			num,
			objname,
			pReset->arg1,
			mobname,
			pReset->arg2 );
		break;

	  case 'O':
		obj = get_obj_proto( NULL, pReset->arg1 );
		if ( !obj )
			strcpy( objname, "Object: *BAD VNUM*" );
		else
			strcpy( objname, obj->name );

		room = get_room_index( NULL, pReset->arg3 );
		if ( !room )
			strcpy( roomname, "Room: *BAD VNUM*" );
		else
			strcpy( roomname, room->name );

		sprintf( buf, "%2d) (object) %s (%d) -> %s (%d) [%d]\r\n",
			num,
			objname,
			pReset->arg1,
			roomname,
			pReset->arg3,
			pReset->arg2 );
		break;

	  case 'P':
		obj2 = get_obj_proto( NULL, pReset->arg1 );
		if ( !obj2 )
			strcpy( objname, "Object1: *BAD VNUM*" );
		else
			strcpy( objname, obj2->name );

		obj = get_obj_proto( NULL, pReset->arg3 );
		if ( !obj )
		{
			if ( pReset->arg3 > 0 )
				strcpy( roomname, "Object2: *BAD VNUM*" );
			else
				strcpy( roomname, "Object2: *NULL obj*" );
		}
		else
		{
			strcpy( roomname, obj->name );
		}

		sprintf( buf, "%2d) (Put) %s (%d) -> %s (%d) [%d]\r\n",
			num,
			objname,
			pReset->arg1,
			roomname,
			(obj)  ?  obj->vnum  :  pReset->arg3,
			pReset->arg2 );
		break;

	  case 'D':
		if ( pReset->arg2 < 0 || pReset->arg2 >= MAX_DIR )
			pReset->arg2 = 0;

		room = get_room_index( NULL, pReset->arg1 );
		if ( !room )
		{
			strcpy( roomname, "Room: *BAD VNUM*" );
			sprintf( objname, "%s (no exit)", code_name(NULL, pReset->arg2, CODE_DIR) );
		}
		else
		{
			strcpy( roomname, room->name );
			sprintf( objname, "%s%s",
				code_name(NULL, pReset->arg2, CODE_DIR),
				get_exit(room, pReset->arg2) ? "" : " (NO EXIT!)" );
		}

		switch ( pReset->arg3 )
		{
		  default:	strcpy( mobname, "(Errore)" );			break;
		  case 0:	strcpy( mobname, "Open" );				break;
		  case 1:	strcpy( mobname, "Close" );				break;
		  case 2:	strcpy( mobname, "Close and lock" );	break;
		}

		sprintf( buf, "%2d) %s [%d] the %s [%d] door %s (%d)\r\n",
			num,
			mobname,
			pReset->arg3,
			objname,
			pReset->arg2,
			roomname,
			pReset->arg1 );
		break;

	  case 'R':
		if ( (room = get_room_index(NULL, pReset->arg1)) == NULL )
			strcpy( roomname, "Room: *BAD VNUM*" );
		else
			strcpy( roomname, room->name );

		sprintf( buf, "%2d) Randomize exits 0 to %d -> %s (%d)\r\n",
			num,
			pReset->arg2,
			roomname,
			pReset->arg1 );
		break;

	  case 'T':
	  {
		BITVECTOR *temp;

		temp = convert_bitvector( pReset->extra );
		sprintf( buf, "%2d) TRAP: %d %d %d %d (%s)\r\n",
			num,
			pReset->extra,
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			code_bit(NULL, temp, CODE_TRAPFLAG) );
		destroybv( temp );
		break;
	  }
	}

	/* Invia solamente i reset della stanza quindi */
	if ( rlist && (!room || (room && room->vnum != rvnum)) )
		return NULL;

	return buf;
}


/* Snippet di Shogar. Il commento non è da togliere o modificare,
 *	serve da credito a chi ha creato lo snippet.
------------------------------------------------------------------------------
 * Shogar's code to hunt for exits/entrances to/from a zone, very nice
 * Display improvements and overland support by Samson of Alsherok
------------------------------------------------------------------------------
 * Nel qual caso non si utilizzasse la wild overland togliere il codice relativo o modificarlo.
 */
DO_RET do_aexit( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*room;
	AREA_DATA	*tarea;
	AREA_DATA	*otherarea;
	EXIT_DATA	*pexit;
	int			 x;
	int			 lrange;
	int			 trange;
	VNUM		 vnum;
	bool		 found = FALSE;

	if ( !VALID_STR(argument) )
		tarea = ch->in_room->area;
	else
	{
		for ( tarea = first_area;  tarea;  tarea = tarea->next )
		{
			if ( !str_cmp(tarea->filename, argument) )
			{
				found = TRUE;
				break;
			}
		}

		if ( !found )
		{
			for ( tarea = first_build;  tarea;  tarea = tarea->next )
			{
				if ( !str_cmp(tarea->filename, argument) )
				{
					found = TRUE;
					break;
				}
			}
		}

		if ( !found )
		{
			send_to_char( ch, "Area not found. Check 'zones' for the filename.\r\n" );
			return;
		}
	}

	trange = tarea->vnum_high;
	lrange = tarea->vnum_low;

	for ( vnum = lrange;  vnum <= trange;  vnum++ )
	{
		if ( (room = get_room_index(NULL, vnum)) == NULL )
			continue;

		if ( HAS_BIT(room->flags, ROOM_TELEPORT)
		  && (room->tele_vnum < lrange || room->tele_vnum > trange) )
		{
			pager_printf( ch, "From: %-20.20s Room: %5d  To  : Room: %5d (Teleport)\r\n",
				tarea->filename, vnum, room->tele_vnum );
		}

		for ( x = 0;  x < MAX_DIR;  x++ )
		{
			if ( (pexit = get_exit(room, x)) == NULL )
				continue;

			if ( pexit->to_room->area != tarea )
			{
				pager_printf( ch, "To  : %-20.20s Room: %5d  From: %-20.20s Room: %5d (%s)\r\n",
					pexit->to_room->area->filename,
					pexit->vnum,
					tarea->filename,
					vnum,
					code_name(NULL, x, CODE_DIR) );
			}
		}
	}

	for ( otherarea = first_area;  otherarea;  otherarea = otherarea->next )
	{
		if ( tarea == otherarea )
			continue;

		trange = otherarea->vnum_high;
		lrange = otherarea->vnum_low;
		for ( vnum = lrange;  vnum <= trange;  vnum++ )
		{
			if ( (room = get_room_index(NULL, vnum)) == NULL )
				continue;

			if ( HAS_BIT(room->flags, ROOM_TELEPORT) )
			{
				if ( room->tele_vnum >= tarea->vnum_low
				  && room->tele_vnum <= tarea->vnum_high )
				{
					pager_printf( ch, "From: %-20.20s Room: %5d  To  : %-20.20s Room: %5d (Teleport)\r\n",
						otherarea->filename, vnum, tarea->filename, room->tele_vnum );
				}
			}

			for ( x = 0;  x < MAX_DIR;  x++ )
			{
				if ( (pexit = get_exit(room, x)) == NULL )
					continue;

				if ( pexit->to_room->area == tarea )
				{
					pager_printf( ch, "From: %-20.20s Room: %5d  To  : %-20.20s Room: %5d (%s)\r\n",
						otherarea->filename,
						vnum,
						pexit->to_room->area->filename,
						pexit->vnum,
						code_name(NULL, x, CODE_DIR) );
				}
			}
		}
	}
}


DO_RET do_redit( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA		 *location;
	ROOM_DATA		 *tmp;
	EXTRA_DESCR_DATA *ed;
	EXIT_DATA		 *xit;
	EXIT_DATA		 *texit;
	char			 *origarg = argument;
	char			  arg [MIL];
	char			  arg2[MIL];
	char			  arg3[MIL];
	char			  buf [MSL];
	char			  dir = 0;
	int				  value;
	int				  edir = 0;
	int				  ekey;
	VNUM			  evnum;

	if ( !ch->desc )
	{
		send_to_char( ch, "You have no descriptor.\r\n" );
		return;
	}

	set_char_color( AT_PLAIN, ch );

	switch ( ch->substate )
	{
	  default:
		break;
	  case SUB_ROOM_DESCR:
		location = ch->dest_buf;
		if ( !location )
		{
			send_log( NULL, LOG_BUILD, "redit: sub_room_desc: NULL ch->dest_buf" );
			location = ch->in_room;
		}
		DISPOSE( location->description );
		location->description = copy_buffer( ch );
		stop_editing( ch );
		ch->substate = ch->tempnum;
		return;
	  case SUB_ROOM_EXTRA:
		ed = ch->dest_buf;
		if ( !ed )
		{
			send_log( NULL, LOG_BUILD, "redit: sub_room_extra: NULL ch->dest_buf" );
			stop_editing( ch );
			return;
		}
		DISPOSE( ed->description );
		ed->description = copy_buffer( ch );
		stop_editing( ch );
		ch->substate = ch->tempnum;
		return;
	}

	location = ch->in_room;

	smash_tilde( argument );
	argument = one_argument( argument, arg );
	if ( ch->substate == SUB_REPEAT_CMD )
	{
		if ( !VALID_STR(arg) )
		{
			send_command( ch, "rstat", CO );
			return;
		}
		if ( !str_cmp(arg, "fatto") || !str_cmp(arg, "no")
		  || !str_cmp(arg, "done")  || !str_cmp(arg, "off") )
		{
			send_to_char( ch, "Redit mode off.\r\n" );
			if ( ch->pg && ch->pg->subprompt )
			{
				DISPOSE( ch->pg->subprompt );
				ch->pg->subprompt = NULL;
			}
			ch->substate = SUB_NONE;
			return;
		}
	}

	if ( !VALID_STR(arg) || arg[0] == '?' )
	{
		if ( ch->substate == SUB_REPEAT_CMD )
			send_to_char( ch, "&YSintassi&w: <field> value\r\n\r\n" );
		else
			send_to_char( ch, "&YSintassi&w: redit <field> value\r\n\r\n" );
		send_to_char( ch, "Field being one of:\r\n" );
		send_to_char( ch, "  name desc ed rmed\r\n" );
		send_to_char( ch, "  exit bexit exdesc exflags exname exkey\r\n" );
		send_to_char( ch, "  flags sector teledelay televnum tunnel\r\n" );
		send_to_char( ch, "  rlist exdistance pulltype pull push\r\n" );
		return;
	}

	if ( !can_modify_room( ch, location ) )
		return;

	if ( !str_cmp(arg, "si") || !str_cmp(arg, "on") )
	{
		CHECK_SUBRESTRICTED( ch );
		send_to_char( ch, "Redit mode on.\r\n" );
		ch->substate = SUB_REPEAT_CMD;
		if ( ch->pg )
		{
			DISPOSE( ch->pg->subprompt );
			ch->pg->subprompt = str_dup( "<&CRedit &W#%r&w> %i" );
		}
		return;
	}

	if ( !str_cmp(arg, "name") || !str_cmp(arg, "nome") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Set the room name.  A very brief single line room description.\r\n" );
			send_to_char( ch, "Usage: redit name <Room summary>\r\n" );
			return;
		}
		DISPOSE( location->name );
		location->name = str_dup( argument );
		return;
	}

	if ( !str_cmp(arg, "desc") )
	{
		if ( ch->substate == SUB_REPEAT_CMD )
			ch->tempnum = SUB_REPEAT_CMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_DESCR;
		ch->dest_buf = location;
		start_editing( ch, location->description );
		return;
	}

	if ( !str_cmp(arg, "tunnel") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Set the maximum characters allowed in the room at one time. (0 = unlimited).\r\n" );
			send_to_char( ch, "Usage: redit tunnel <value>\r\n" );
			return;
		}
		location->tunnel = URANGE( 0, atoi(argument), 1000 );
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	/* Crash fix and name support */
	if ( !str_cmp(arg, "affect") )
	{
		AFFECT_DATA *paf;
		int			 loc;

		argument = one_argument(argument, arg2);
		if ( !VALID_STR(arg2) || !VALID_STR(argument) )
		{
			send_to_char( ch, "Usage: redit affect <field> <value>\r\n" );
			return;
		}

		loc = code_num( NULL, arg2, CODE_APPLY );
		if ( loc < 1 )
		{
			ch_printf( ch, "Unknown field: %s\r\n", arg2 );
			return;
		}

		if ( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
		{
			BITVECTOR *bits = NULL;

			while ( VALID_STR(argument) )
			{
				argument = one_argument( argument, arg3 );
				if ( loc == APPLY_AFFECT )
				{
					value = code_num( NULL, arg3, CODE_AFFECT );
					if ( value < 0 || value >= MAX_AFFECT )
						ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
					else
						SET_BIT( bits, value );
				}
				else
				{
					value = code_num( NULL, arg3, CODE_RIS );
					if ( value < 0 || value >= MAX_RIS )
						ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
					else
						SET_BIT( bits, value );
				}
			}
			if ( !bits )
				return;
			value = *(bits->flags + bits->size-1);	/* (FF) anche qui da sistemare gli affect */
			destroybv( bits );
		}
		else
		{
			one_argument( argument, arg3 );
			if ( loc == APPLY_WEARSPELL && !is_number(arg3) )
			{
				value = bsearch_skill_exact( arg3, gsn_first_spell, gsn_first_skill - 1 );
				if ( value == -1 )
				{
/*					printf( "%s\r\n", arg3 ); */
					send_to_char( ch, "Unknown spell name.\r\n" );
					return;
				}
			}
			else
				value = atoi( arg3 );
		}

		CREATE( paf, AFFECT_DATA, 1 );
		paf->type = -1;
		paf->duration = -1;
		paf->location = loc;
		paf->modifier = value;

		LINK( paf, location->first_affect, location->last_affect, next, prev );
		top_affect++;

		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "rmaffect") )
	{
		AFFECT_DATA *paf;
		int			 loc,
					 count;

		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Usage: redit rmaffect <affect#>\r\n" );
			return;
		}

		loc = atoi( argument );
		if ( loc < 1 )
		{
			send_to_char( ch, "Invalid number.\r\n" );
			return;
		}

		count = 0;

		for ( paf = location->first_affect; paf; paf = paf->next )
		{
			if ( ++count == loc )
			{
				UNLINK( paf, location->first_affect, location->last_affect, next, prev );
				DISPOSE( paf );
				send_to_char( ch, "Removed.\r\n" );
				--top_affect;
				return;
			}
		}
		send_to_char( ch, "Not found.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "ed") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Create an extra description.\r\n" );
			send_to_char( ch, "You must supply keyword(s).\r\n" );
			return;
		}

		CHECK_SUBRESTRICTED( ch );

		ed = SetRExtra( location, argument );
		if ( ch->substate == SUB_REPEAT_CMD )
			ch->tempnum = SUB_REPEAT_CMD;
		else
			ch->tempnum = SUB_NONE;

		ch->substate = SUB_ROOM_EXTRA;
		ch->dest_buf = ed;
		start_editing( ch, ed->description );
		return;
	}

	if ( !str_cmp(arg, "rmed") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Remove an extra description.\r\n" );
			send_to_char( ch, "You must supply keyword(s).\r\n" );
			return;
		}

		if ( DelRExtra( location, argument ) )
			send_to_char( ch, "Deleted.\r\n" );
		else
			send_to_char( ch, "Not found.\r\n" );

		return;
	}

	if ( !str_cmp(arg, "rlist") )
	{
		RESET_DATA	*pReset;
		char		*bptr;
		AREA_DATA	*tarea;
		int			 num;

		tarea = location->area;
		if ( !tarea->first_reset )
		{
			send_to_char( ch, "This area has no resets to list.\r\n" );
			return;
		}

		num = 0;
		for ( pReset = tarea->first_reset; pReset; pReset = pReset->next )
		{
			num++;
			if ( (bptr = sprint_reset(ch, pReset, num, TRUE)) == NULL )
				continue;
			send_to_char( ch, bptr );
		}

		return;
	}

	if ( !str_cmp(arg, "flags") )
	{
		if ( !VALID_STR(argument) )
		{
		    send_to_char( ch, "Toggle the room flags.\r\n" );
		    send_to_char( ch, "Usage: redit flags <flag> [flag]...\r\n" );
		    return;
		}

		while ( VALID_STR(argument) )
		{
			argument = one_argument( argument, arg2 );
			value = code_num( NULL, arg2, CODE_ROOM );
			if ( value < 0 || value >= MAX_ROOM )
				ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
			else
				TOGGLE_BIT( location->flags, value );
		}

		return;
	}

	if ( !str_cmp(arg, "teledelay") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Set the delay of the teleport. (0 = off).\r\n" );
			send_to_char( ch, "Usage: redit teledelay <value>\r\n" );
			return;
		}

		location->tele_delay = atoi( argument );
		send_to_char( ch, "Fatto.\r\n" );

		return;
	}

	if ( !str_cmp(arg, "televnum") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Set the vnum of the room to teleport to.\r\n" );
			send_to_char( ch, "Usage: redit televnum <vnum>\r\n" );
			return;
		}

		location->tele_vnum = atoi( argument );
		send_to_char( ch, "Fatto.\r\n" );

		return;
	}

	if ( !str_cmp(arg, "sector") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Set the sector type.\r\n" );
			send_to_char( ch, "Usage: redit sector <value>\r\n" );
			return;
		}

		location->sector = atoi( argument );
		if ( location->sector < 0 || location->sector >= MAX_SECTOR )
		{
			location->sector = 1;
			send_to_char( ch, "Out of range.\r\n" );
		}
		else
		{
			send_to_char( ch, "Fatto.\r\n" );
		}

		return;
	}

	if ( !str_cmp(arg, "exkey") )
	{
		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );
		if ( !VALID_STR(arg2) || !VALID_STR(arg3) )
		{
			send_to_char( ch, "Usage: redit exkey <dir> <key vnum>\r\n" );
			return;
		}

		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		value = atoi( arg3 );
		if ( !xit )
		{
			send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
			return;
		}

		xit->key = value;
		send_to_char( ch, "Fatto.\r\n" );

		return;
	}

	if ( !str_cmp(arg, "exname") )
	{
		argument = one_argument( argument, arg2 );
		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Change or clear exit keywords.\r\n" );
			send_to_char( ch, "Usage: redit exname <dir> [keywords]\r\n" );
			return;
		}

		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		if ( !xit )
		{
			send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
			return;
		}

		DISPOSE( xit->keyword );
		xit->keyword = str_dup( argument );
		send_to_char( ch, "Fatto.\r\n" );

		return;
	}

	if ( !str_cmp(arg, "exflags") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Toggle or display exit flags.\r\n" );
			send_to_char( ch, "Usage: redit exflags <dir> <flag> [flag]...\r\n" );
			return;
		}

		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		if ( !xit )
		{
			send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
			return;
		}

		if ( !VALID_STR(argument) )
		{
			sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\r\n[ ", xit->vdir, xit->keyword, xit->key );

			for ( value = 0;  value < MAX_EXIT;  value++ )
			{
				if ( HAS_BIT(xit->flags, value) )
				{
				    strcat( buf, code_name(NULL, value, CODE_EXIT) );
				    strcat( buf, " " );
				}
			}
			strcat( buf, "]\r\n" );
			send_to_char( ch, buf );
			return;
		}

		while ( VALID_STR(argument) )
		{
			argument = one_argument( argument, arg2 );
			value = code_num( NULL, arg2, CODE_EXIT );
			if ( value < 0 || value >= MAX_EXIT )
				ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
			else
				TOGGLE_BIT( xit->flags, value );
		}
		return;
	}

	if ( !str_cmp(arg, "ex_flags") )
	{
		argument = one_argument( argument, arg2 );

		value = code_num( NULL, arg2, CODE_EXIT );
		if ( value < 0 )
		{
			send_to_char( ch, "Bad exit flag. \r\n" );
			return;
		}

		if ( (xit = get_exit(location,edir)) == NULL )
		{
			sprintf( buf, "redit exit %c 1", dir );
			send_command( ch, buf, CO );
			xit = get_exit( location, edir );
		}
		TOGGLE_BIT( xit->flags, value );
		return;
	}


	if ( !str_cmp(arg, "ex_to_room") )
	{
		argument = one_argument( argument, arg2 );
		evnum = atoi( arg2 );
		if ( evnum < 1 || evnum >= MAX_VNUM )
		{
			send_to_char( ch, "Invalid room number.\r\n" );
			return;
		}

		if ( (tmp = get_room_index(NULL, evnum)) == NULL )
		{
			send_to_char( ch, "Non-existant room.\r\n" );
			return;
		}

		if ( (xit = get_exit(location,edir)) == NULL )
		{
			sprintf( buf, "redit exit %c 1", dir );
			send_command( ch, buf, CO );
			xit = get_exit(location,edir);
		}
		xit->vnum = evnum;
		return;
	}

	if ( !str_cmp(arg, "ex_key") )
	{
		argument = one_argument( argument, arg2 );
		if ( (xit = get_exit(location,edir)) == NULL )
		{
			sprintf( buf, "redit exit %c 1", dir );
			send_command( ch, buf, CO );
			xit = get_exit( location, edir );
		}
		xit->key = atoi(arg2);
		return;
	}

	if ( !str_cmp(arg, "ex_exdesc") )
	{
		if ( (xit = get_exit(location, edir)) == NULL )
		{
			sprintf( buf, "redit exit %c 1", dir );
			send_command( ch, buf, CO );
		}
		sprintf( buf, "redit exdesc %c %s", dir, argument );
		send_command( ch, buf, CO );
		return;
	}

	if ( !str_cmp(arg, "ex_keywords") )		/* not called yet */
	{
		if ( (xit = get_exit(location, edir)) == NULL )
		{
			sprintf( buf, "redit exit %c 1", dir );
			send_command( ch, buf, CO );
			if ( (xit = get_exit(location, edir)) == NULL )
				return;
		}
		sprintf( buf, "%s %s", xit->keyword, argument );
		DISPOSE( xit->keyword );
		xit->keyword = str_dup( buf );
		return;
	}

	if ( !str_cmp(arg, "exit") )
	{
		bool	addexit,
				numnotdir;

		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );

		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Create, change or remove an exit.\r\n" );
			send_to_char( ch, "Usage: redit exit <dir> [room] [flags] [key] [keywords]\r\n" );
			return;
		}

		addexit = numnotdir = FALSE;
		switch ( arg2[0] )
		{
		  default:		edir = get_dir(arg2);							  break;
		  case '+':		edir = get_dir(arg2+1);		addexit = TRUE;		  break;
		  case '#':		edir = atoi(arg2+1);		numnotdir = TRUE;	  break;
		}

		if ( !VALID_STR(arg3) )
			evnum = 0;
		else
			evnum = atoi( arg3 );

		if ( numnotdir )
		{
			if ( (xit = get_exit_num(location, edir)) != NULL )
				edir = xit->vdir;
		}
		else
			xit = get_exit( location, edir );

		if ( !evnum )
		{
			if ( xit )
			{
				extract_exit(location, xit);
				send_to_char( ch, "Exit removed.\r\n" );
				return;
			}
			send_to_char( ch, "No exit in that direction.\r\n" );
			return;
		}

		if ( evnum < 1 || evnum >= MAX_VNUM )
		{
			send_to_char( ch, "Invalid room number.\r\n" );
			return;
		}

		if ( (tmp = get_room_index(NULL, evnum)) == NULL )
		{
			send_to_char( ch, "Non-existant room.\r\n" );
			return;
		}

		if ( addexit || !xit )
		{
			if ( numnotdir )
			{
				send_to_char( ch, "Cannot add an exit by number, sorry.\r\n" );
				return;
			}
			if ( addexit && xit && get_exit_to(location, edir, tmp->vnum) )
			{
				send_to_char( ch, "There is already an exit in that direction leading to that location.\r\n" );
				return;
			}
			xit = make_exit( location, tmp, edir );
			DISPOSE( xit->keyword );
			xit->keyword		= str_dup( "" );
			DISPOSE( xit->description );
			xit->description	= str_dup( "" );
			xit->key			= -1;
			xit->flags		= 0;
			act( AT_ADMIN, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
		}
		else
		{
			act( AT_ADMIN, "Something is different...", ch, NULL, NULL, TO_ROOM );
		}

		if ( xit->to_room != tmp )
		{
			xit->to_room = tmp;
			xit->vnum = evnum;
			texit = get_exit_to( xit->to_room, table_dir[edir].rev_dir, location->vnum );
			if ( texit )
			{
				texit->rexit = xit;
				xit->rexit = texit;
			}
		}

		argument = one_argument( argument, arg3 );
		if ( VALID_STR(arg3) )
			xit->flags = convert_bitvector( atoi(arg3) );	/* (FF) mhhm.. */

		if ( VALID_STR(argument) )
		{
			one_argument( argument, arg3 );
			ekey = atoi( arg3 );
			if ( ekey != 0 || !VALID_STR(arg3) )	/* (bb) forse da controllare */
			{
				argument = one_argument( argument, arg3 );
				xit->key = ekey;
			}

			if ( VALID_STR(argument) )
			{
				DISPOSE( xit->keyword );
				xit->keyword = str_dup( argument );
			}
		}
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	/*
	 * Twisted and evil, but works
	 * Makes an exit, and the reverse in one shot.
	 */
	if ( !str_cmp(arg, "bexit") )
	{
		EXIT_DATA	*rxit;
		ROOM_DATA	*tmploc;
		VNUM		 vnum;
		int			 exnum;
		char		 rvnum[MIL];
		bool		 numnotdir;

		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );

		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Create, change or remove a two-way exit.\r\n" );
			send_to_char( ch, "Usage: redit bexit <dir> [room] [flags] [key] [keywords]\r\n" );
			return;
		}

		numnotdir = FALSE;
		switch ( arg2[0] )
		{
		  default:
			edir = get_dir( arg2 );
			break;
		  case '#':
			numnotdir = TRUE;
			edir = atoi( arg2+1 );
			break;
		  case '+':
			edir = get_dir( arg2+1 );
			break;
		}

		tmploc = location;
		exnum = edir;
		if ( numnotdir )
		{
			if ( (xit = get_exit_num(tmploc, edir)) != NULL )
				edir = xit->vdir;
		}
		else
			xit = get_exit( tmploc, edir );

		rxit = NULL;
		vnum = 0;
		rvnum[0] = '\0';
		if ( xit )
		{
			vnum = xit->vnum;
			if ( VALID_STR(arg3) )
				sprintf( rvnum, "%d", tmploc->vnum );

			if ( xit->to_room )
				rxit = get_exit( xit->to_room, table_dir[edir].rev_dir );
			else
				rxit = NULL;
		}
		sprintf( buf, "redit exit %s %s %s", arg2, arg3, argument );
		send_command( ch, buf, CO );

		if ( numnotdir )
			xit = get_exit_num( tmploc, exnum );
		else
			xit = get_exit( tmploc, edir );

		if ( !rxit && xit )
		{
			vnum = xit->vnum;
			if ( VALID_STR(arg3) )
				sprintf( rvnum, "%d", tmploc->vnum );

			if ( xit->to_room )
				rxit = get_exit( xit->to_room, table_dir[edir].rev_dir );
			else
				rxit = NULL;
		}

		if ( vnum )
		{
			sprintf( buf, "at %d %s exit %d %s %s",
				vnum, translate_command(ch, "redit"), table_dir[edir].rev_dir, rvnum, argument );
			send_command( ch, buf, CO );
		}
		return;
	}

	if ( !str_cmp(arg, "pulltype") || !str_cmp(arg, "pushtype") )
	{
		int pt;

		argument = one_argument( argument, arg2 );
		if ( !VALID_STR(arg2) )
		{
			ch_printf( ch, "Set the %s between this room, and the destination room.\r\n", arg );
			ch_printf( ch, "Usage: redit %s <dir> <type>\r\n", arg );
			return;
		}

		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		if ( xit )
		{
			pt = code_num( NULL, argument, CODE_PULL );
			if ( pt == -1 )
			{
				ch_printf( ch, "Unknown pulltype: %s.  (See help PULLTYPES)\r\n", argument );
			}
			else
			{
				xit->pulltype = pt;
				send_to_char( ch, "Fatto.\r\n" );
				return;
			}
		}
		send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "pull") )
	{
		argument = one_argument(argument, arg2);
		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Set the 'pull' between this room, and the destination room.\r\n" );
			send_to_char( ch, "Usage: redit pull <dir> <force (0 to 100)>\r\n" );
			return;
		}

		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		if ( xit )
		{
			xit->pull = URANGE( -100, atoi(argument), 100 );
			send_to_char( ch, "Fatto.\r\n" );
			return;
		}
		send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "push") )
	{
		argument = one_argument( argument, arg2 );
		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Set the 'push' away from the destination room in the opposite direction.\r\n" );
			send_to_char( ch, "Usage: redit push <dir> <force (0 to 100)>\r\n" );
			return;
		}

		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		if ( xit )
		{
			xit->pull = URANGE(-100, -(atoi(argument)), 100);
			send_to_char( ch, "Fatto.\r\n" );
			return;
		}
		send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "exdistance") )
	{
		argument = one_argument( argument, arg2 );
		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Set the distance (in rooms) between this room, and the destination room.\r\n" );
			send_to_char( ch, "Usage: redit exdistance <dir> [distance]\r\n" );
			return;
		}

		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		if ( xit )
		{
			xit->distance = URANGE( 1, atoi(argument), 50 );
			send_to_char( ch, "Fatto.\r\n" );
			return;
		}
		send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "exdesc") )
	{
		argument = one_argument( argument, arg2 );
		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Create or clear a description for an exit.\r\n" );
			send_to_char( ch, "Usage: redit exdesc <dir> [description]\r\n" );
			return;
		}

		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}

		if ( xit )
		{
			DISPOSE( xit->description );
			if ( !VALID_STR(argument) )
				xit->description = str_dup( "" );
			else
			{
				sprintf( buf, "%s\r\n", argument );
				xit->description = str_dup( buf );
			}
			send_to_char( ch, "Fatto.\r\n" );
			return;
		}
		send_to_char( ch, "No exit in that direction.  Use 'redit exit ...' first.\r\n" );
		return;
	}

	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEAT_CMD )
	{
		ch->substate = SUB_RESTRICTED;
		/* Si suppone che ch abbia inviato argument nella sua lingua quindi niente send_command() */
		interpret( ch, origarg );
		ch->substate = SUB_REPEAT_CMD;
		ch->last_cmd = do_redit;
	}
	else
	{
		send_command( ch, "redit", CO );
	}
}


DO_RET do_ocreate( CHAR_DATA *ch, char *argument )
{
	OBJ_PROTO_DATA	*pObj;		/* Oggetto prototipo creato nell'area */
	OBJ_DATA		*obj;			/* Oggetto dato all'admin */
	char			 arg [MIL];
	char			 arg2[MIL];
	VNUM			 vnum;
	VNUM			 cvnum;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Mobiles cannot create.\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	vnum = ( is_number(arg) )  ?  atoi( arg )  :  -1;
	if ( vnum == -1 || !VALID_STR(argument) )
	{
		send_to_char( ch, "Usage:  ocreate <vnum> [copy vnum] <item name>\r\n" );
		return;
	}

	if ( vnum < 1 || vnum >= MAX_VNUM )
	{
		send_to_char( ch, "Vnum out of range.\r\n" );
		return;
	}

	one_argument( argument, arg2 );
	cvnum = atoi( arg2 );

	if ( cvnum != 0 )
		argument = one_argument( argument, arg2 );
	if ( cvnum < 1 )
		cvnum = 0;

	if ( get_obj_proto(NULL, vnum) )
	{
		send_to_char( ch, "An object with that number already exists.\r\n" );
		return;
	}

	if ( get_trust(ch) < TRUST_BUILDER )
	{
		AREA_DATA *pArea;

		if ( !ch->pg || !(pArea = ch->pg->build_area) )
		{
			send_to_char( ch, "You must have an assigned area to create objects.\n\r" );
			return;
		}

		if ( vnum < pArea->vnum_low || vnum > pArea->vnum_high )
		{
			send_to_char( ch, "That number is not in your allocated range.\n\r" );
			return;
		}
	}

	pObj = make_object_proto( vnum, cvnum, argument );
	if ( !pObj )
	{
		send_to_char( ch, "Errore.\r\n" );
		send_log( NULL, LOG_BUILD, "do_ocreate: make_object failed." );
		return;
	}

	pObj->area = ch->pg->build_area;
	obj = make_object( pObj, get_level(ch) );
	obj_to_char( obj, ch );

	ch_printf( ch, "&YCrei l'oggetto %s. ObjVnum:  &W%d   &YKeywords:  &W%s\r\n",
		obj->short_descr, obj->vnum, obj->name );
}


DO_RET do_mcreate( CHAR_DATA *ch, char *argument )
{
	MOB_PROTO_DATA	*pMobIndex;
	CHAR_DATA		*mob;
	char			 arg [MIL];
	char			 arg2[MIL];
	VNUM			 vnum;
	VNUM			 cvnum;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Mobiles cannot create.\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	vnum = ( is_number(arg) )  ?  atoi( arg )  :  -1;
	if ( vnum == -1 || !VALID_STR(argument) )
	{
		send_to_char( ch, "Usage:  mcreate <vnum> [cvnum] <mobile name>\r\n" );
		return;
	}

	if ( vnum < 1 || vnum >= MAX_VNUM )
	{
		send_to_char( ch, "Vnum out of range.\r\n" );
		return;
	}

	one_argument( argument, arg2 );
	cvnum = atoi( arg2 );

	if ( cvnum != 0 )
		argument = one_argument( argument, arg2 );

	if ( cvnum < 1 )
		cvnum = 0;

	if ( get_mob_index(NULL, vnum) )
	{
		send_to_char( ch, "A mobile with that number already exists.\r\n" );
		return;
	}

	if ( get_trust(ch) < TRUST_BUILDER )
	{
		AREA_DATA *pArea;

		if ( !ch->pg || !(pArea = ch->pg->build_area) )
		{
			send_to_char( ch, "You must have an assigned area to create mobiles.\n\r" );
			return;
		}

		if ( vnum < pArea->vnum_low || vnum > pArea->vnum_high )
		{
			send_to_char( ch, "That number is not in your allocated range.\n\r" );
			return;
		}
	}

	pMobIndex = make_mobile_proto( vnum, cvnum, argument );
	if ( !pMobIndex )
	{
		send_to_char( ch, "Errore.\r\n" );
		send_log( NULL, LOG_BUILD, "do_mcreate: make_mobile failed." );
		return;
	}

	pMobIndex->area = ch->pg->build_area;
	mob = make_mobile( pMobIndex );
	char_to_room( mob, ch->in_room );
	act( AT_ADMIN, "$n waves su$x arms about, and $N appears at su$x command!", ch, NULL, mob, TO_ROOM );
	ch_printf( ch,
		"&YYou wave your arms about, and %s appears at your command!\r\n"
		"MobVnum:  &W%d   &YKeywords:  &W%s\r\n",
		pMobIndex->short_descr,
		pMobIndex->vnum,
		pMobIndex->name );
}


void free_reset( AREA_DATA *are, RESET_DATA *res )
{
	UNLINK( res, are->first_reset, are->last_reset, next, prev );
	DISPOSE( res );
}


void free_area( AREA_DATA *are )
{
	DISPOSE( are->name );
	DISPOSE( are->filename );

	while ( are->first_reset )
		free_reset( are, are->first_reset );

	DISPOSE( are );
	are = NULL;
}


void assign_area( CHAR_DATA *ch )
{
	char	   buf[MSL];
	char	   buf2[MSL];
	char	   taf[MIL];
	AREA_DATA *tarea,
			  *tmp;
	bool	   created = FALSE;

	if ( IS_MOB(ch) )
		return;

	if ( get_trust(ch) >= TRUST_BUILDER
	  && ch->pg->vnum_range_low
	  && ch->pg->vnum_range_high )
	{
		tarea = ch->pg->build_area;
		sprintf( taf, "%s.are", capitalize(ch->name) );

		if ( !tarea )
		{
			for ( tmp = first_build;  tmp;  tmp = tmp->next )
			{
				if ( !str_cmp(taf, tmp->filename) )
				{
					tarea = tmp;
					break;
				}
			}
		}

		if ( !tarea )
		{
			send_log( NULL, LOG_BUILD, "Creating area entry for %s", ch->name );
			CREATE( tarea, AREA_DATA, 1 );
			LINK( tarea, first_build, last_build, next, prev );
			tarea->first_reset	= NULL;
			tarea->last_reset	= NULL;
			sprintf( buf, "{PROTO} %s's area in progress", ch->name );
			tarea->name			= str_dup( buf );
			tarea->filename		= str_dup( taf );
			strcpy( buf2, ch->name );
			tarea->author 		= str_dup( buf2 );
			tarea->age			= 0;
			tarea->nplayer		= 0;

			CREATE(tarea->weather, WEATHER_DATA, 1); /* FB */
			tarea->weather->temp = 0;
			tarea->weather->precip = 0;
			tarea->weather->wind = 0;
			tarea->weather->temp_vector = 0;
			tarea->weather->precip_vector = 0;
			tarea->weather->wind_vector = 0;
			tarea->weather->climate_temp = 2;
			tarea->weather->climate_precip = 2;
			tarea->weather->climate_wind = 2;
			tarea->weather->first_neighbor = NULL;
			tarea->weather->last_neighbor = NULL;

			created = TRUE;
		}
		else
		{
			send_log( NULL, LOG_BUILD, "Updating area entry for %s", ch->name );
		}

		tarea->vnum_low		= ch->pg->vnum_range_low;
		tarea->vnum_high	= ch->pg->vnum_range_high;
		ch->pg->build_area	= tarea;
	}
}


DO_RET do_aassign( CHAR_DATA *ch, char *argument )
{
	char	   buf[MSL];
	AREA_DATA *tarea;
	AREA_DATA *tmp;

	set_char_color( AT_ADMIN, ch );

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: aassign <filename.are>\r\n" );
		return;
	}

	if ( !str_cmp(argument, "nessuno") || !str_cmp(argument, "none" )
	  || !str_cmp(argument, "nulla"	 ) || !str_cmp(argument, "null" )
	  || !str_cmp(argument, "pulisci") || !str_cmp(argument, "clear") )
	{
		ch->pg->build_area = NULL;
		assign_area(ch);

		if ( !ch->pg->build_area )
			send_to_char( ch, "Area pointer cleared.\r\n" );
		else
			send_to_char( ch, "Originally assigned area restored.\r\n" );

		return;
	}

	strcpy( buf, argument );
	tarea = NULL;

	if ( get_trust(ch) >= TRUST_BUILDER )
	{
		for ( tmp = first_area;  tmp;  tmp = tmp->next )
		{
			if ( !str_cmp(buf, tmp->filename) )
			{
				tarea = tmp;
				break;
			}
		}
	}

	if ( !tarea )
	{
		for ( tmp = first_build;  tmp;  tmp = tmp->next )
		{
			if ( !str_cmp(buf, tmp->filename) )
			{
				if ( get_trust(ch) >= TRUST_BUILDER )
				{
					tarea = tmp;
					break;
				}
				else
				{
					send_to_char( ch, "You do not have permission to use that area.\n\r" );
					return;
				}
			}
		}
	}

	if ( !tarea )
	{
		if ( get_trust(ch) >= TRUST_BUILDER )
			send_to_char( ch, "No such area.  Use 'zones'.\n\r" );
		else
			send_to_char( ch, "No such area.  Use 'newzones'.\n\r" );

		return;
	}

	ch->pg->build_area = tarea;
	ch_printf( ch, "Assigning you: %s\r\n", tarea->name );
}


void fold_area( AREA_DATA *tarea, char *filename, bool install )
{
	RESET_DATA		 *treset;
	ROOM_DATA		 *room;
	MOB_PROTO_DATA	 *pMobIndex;
	OBJ_PROTO_DATA	 *pObj;
	EXIT_DATA		 *xit;
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA		 *paf;
	SHOP_DATA		 *pShop;
	REPAIR_DATA		 *pRepair;
	NEIGHBOR_DATA	 *neigh;
	MUD_FILE		 *fpout;
	char			  buf[MSL];
	char			  temp[MSL];
	int				  x;
	VNUM			  vnum;
	bool			  complexmob;

	send_log( NULL, LOG_BUILD, "Salvataggio di %s...", tarea->filename );

	sprintf( buf, "%s.bak", filename );
	if ( rename(filename, buf) != 0 )
	{
		send_log( NULL, LOG_BUILD, "fold_area: impossibile rinominare da %s a %s: %s",
			filename, buf, strerror(errno) );
	}

	fpout = mud_fopen( "", filename, "w", TRUE );
	if ( !fpout )
		return;

	/* (RR) da rifare completamente questa parte, la sezione #AREA */
	fprintf( fpout->file, "#AREA   %s~\n\n\n\n", tarea->name );
#define AREA_VERSION_WRITE 1
	fprintf( fpout->file, "#VERSION %d\n", AREA_VERSION_WRITE );	/* Qui ci và l'ultimo numero di versione dell'area gestita dal mud */
#undef AREA_VERSION_WRITE
	fprintf( fpout->file, "#AUTHOR %s~\n\n", tarea->author );
	fprintf( fpout->file, "#RANGES\n");
	fprintf( fpout->file, "%d %d\n",
		tarea->level_low, tarea->level_high );
	fprintf( fpout->file, "$\n\n");
	fprintf( fpout->file, "#SPELL_LIMIT %d\n", tarea->spell_limit );
	if ( tarea->resetmsg )	/* Rennard */
		fprintf( fpout->file, "#RESETMSG %s~\n\n", tarea->resetmsg[0] );
	if ( tarea->reset_frequency )
		fprintf( fpout->file, "#FLAGS\n%s %d\n\n", code_bit(fpout, tarea->flags, CODE_AREAFLAG), tarea->reset_frequency );
	else
		fprintf( fpout->file, "#FLAGS\n%s\n\n", code_bit(fpout, tarea->flags, CODE_AREAFLAG) );

	fprintf( fpout->file, "#ECONOMY %d %d\n\n", tarea->economy_high, tarea->economy_low );

	/* Climate info */
	fprintf( fpout->file, "#CLIMATE %d %d %d\n\n",
		tarea->weather->climate_temp,
		tarea->weather->climate_precip,
		tarea->weather->climate_wind);

	/* neighboring weather systems */
	for ( neigh = tarea->weather->first_neighbor;  neigh;  neigh = neigh->next)
		fprintf( fpout->file, "#NEIGHBOR %s~\n\n", neigh->name );

	/* save mobiles */
	fprintf( fpout->file, "#MOBILES\n" );
	for ( vnum = tarea->vnum_low;  vnum <= tarea->vnum_high;  vnum++ )
	{
		MPROG_DATA *mprg;

		pMobIndex = get_mob_index( NULL, vnum );
		if ( !pMobIndex )
			continue;

		if ( pMobIndex->attr_perm[ATTR_STR] != 40 || pMobIndex->attr_perm[ATTR_RES] != 40 || pMobIndex->attr_perm[ATTR_CON] != 40
		  || pMobIndex->attr_perm[ATTR_INT] != 40 || pMobIndex->attr_perm[ATTR_COC] != 40 || pMobIndex->attr_perm[ATTR_WIL] != 40
		  || pMobIndex->attr_perm[ATTR_AGI] != 40 || pMobIndex->attr_perm[ATTR_REF] != 40 || pMobIndex->attr_perm[ATTR_SPE] != 40
		  || pMobIndex->attr_perm[ATTR_SPI] != 40 || pMobIndex->attr_perm[ATTR_MIG] != 40 || pMobIndex->attr_perm[ATTR_ABS] != 40
		  || pMobIndex->attr_perm[ATTR_EMP] != 40 || pMobIndex->attr_perm[ATTR_BEA] != 40 || pMobIndex->attr_perm[ATTR_LEA] != 40
		  || pMobIndex->hitroll	 != 0		|| pMobIndex->damroll	  != 0
		  || pMobIndex->race	 != 0		||	pMobIndex->class	  != 3
		  || !IS_EMPTY(pMobIndex->attacks)
		  || !IS_EMPTY(pMobIndex->defenses)
		  || pMobIndex->height	 != 0		||	pMobIndex->weight	  != 0
		  || !IS_EMPTY(pMobIndex->speaks)	||	pMobIndex->speaking	  != 0
		  || !IS_EMPTY(pMobIndex->xflags)
		  || pMobIndex->numattacks != 0 )
		{
			complexmob = TRUE;
		}
		else
		{
			complexmob = FALSE;
		}

		fprintf( fpout->file, "#%d\n",	vnum );
		fprintf( fpout->file, "%s~\n",	pMobIndex->name );
		fprintf( fpout->file, "%s~\n",	pMobIndex->short_descr );
		fprintf( fpout->file, "%s~\n",	strip_char(pMobIndex->long_descr, '\r') );
		fprintf( fpout->file, "%s~\n",	strip_char(pMobIndex->description, '\r') );
		fprintf( fpout->file, "%d ",		*pMobIndex->act->flags );	/* (FF) sì vabbe', anche qui fatto modello pezza */
		fprintf( fpout->file, "%d %d %c\n", *pMobIndex->affected_by->flags,	/* (FF) sì vabbe', anche qui fatto modello pezza */
			pMobIndex->alignment, complexmob  ?  'C'  :  'S' );

		fprintf( fpout->file, "%d %d %d ", pMobIndex->level, 0, pMobIndex->ac );
		fprintf( fpout->file, "%dd%d+%d ", pMobIndex->hitnodice, pMobIndex->hitsizedice, pMobIndex->hitplus );
		fprintf( fpout->file, "%dd%d+%d\n", pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus );
		fprintf( fpout->file, "%d %d\n", pMobIndex->gold, pMobIndex->exp );
		/* Need to convert to new positions correctly on loadup sigh */
		fprintf( fpout->file, "%d %d %d\n",	pMobIndex->position+100, pMobIndex->defposition+100, pMobIndex->sex );

		if ( complexmob )
		{
			buf[0] = '\0';
			for ( x = 0;  x < MAX_ATTR;  x++ )
			{
				sprintf( temp, "%d ", pMobIndex->attr_perm[x] );
				strcat( buf, temp );
			}
			fprintf( fpout->file, "%s\n", buf );

			buf[0] = '\0';
			for ( x = 0;  x < MAX_SAVETHROW;  x++ )
			{
				sprintf( temp, "%d ", pMobIndex->saving_throw[x] );
				strcat( buf, temp );
			}
			fprintf( fpout->file, "%s\n", buf );

			fprintf( fpout->file, "%d %d %d %d %d %d %d\n",
				pMobIndex->race,
				pMobIndex->class,
				pMobIndex->height,
				pMobIndex->weight,
				*pMobIndex->speaks->flags,	/* (FF) altra ciofeca-flag */
				pMobIndex->speaking,
				pMobIndex->numattacks );

			fprintf( fpout->file, "%d %d %d %d %d %d %d ",
				pMobIndex->hitroll,
				pMobIndex->damroll,
				*(pMobIndex->xflags->flags		+ pMobIndex->xflags->size-1),		/* (FF) altra ciofeca-flag */
				*(pMobIndex->resistant->flags	+ pMobIndex->resistant->size-1),	/* (FF) altra ciofeca-flag */
				*(pMobIndex->immune->flags		+ pMobIndex->immune->size-1),		/* (FF) altra ciofeca-flag */
				*(pMobIndex->susceptible->flags	+ pMobIndex->susceptible->size-1),	/* (FF) altra ciofeca-flag */
				*(pMobIndex->attacks->flags		+ pMobIndex->attacks->size-1) );	/* (FF) altra ciofeca-flag */

			fprintf( fpout->file, "%d\n", *(pMobIndex->defenses->flags+pMobIndex->defenses->size-1) );	/* (FF) altra ciofeca-flag */
		}

		if ( pMobIndex->first_mudprog )
		{
			for ( mprg = pMobIndex->first_mudprog;  mprg;  mprg = mprg->next )
			{
				fputs( "Mudprog ", fpout->file );
				fwrite_mudprog( fpout, mprg );
			}

			fputs( "|\n", fpout->file );
		}
	}
	fprintf( fpout->file, "#0\n\n\n" );
	if ( install && vnum < tarea->vnum_high )
		tarea->vnum_high = vnum - 1;

	/* save objects */
	fprintf( fpout->file, "#OBJECTS\n" );
	for ( vnum = tarea->vnum_low;  vnum <= tarea->vnum_high;  vnum++ )
	{
		MPROG_DATA *mprg;

		if ( (pObj = get_obj_proto(fpout, vnum)) == NULL )
			continue;

		fprintf( fpout->file, "#%d\n",	vnum				);
		fprintf( fpout->file, "%s~\n",	pObj->name			);
		fprintf( fpout->file, "%s~\n",	pObj->short_descr	);
		fprintf( fpout->file, "%s~\n",	pObj->long_descr	);
		fprintf( fpout->file, "%s~\n",	pObj->action_descr	);
		if ( !IS_EMPTY(pObj->layers) )
		{
			fprintf( fpout->file, "%d %d %d %d\n",
				pObj->type,
				print_bitvector(pObj->extra_flags),
				print_bitvector(pObj->wear_flags),
				print_bitvector(pObj->layers) );
		}
		else
		{
			fprintf( fpout->file, "%d %d %d\n",
				pObj->type,
				print_bitvector(pObj->extra_flags),
				print_bitvector(pObj->wear_flags) );
		}

/* (FF) Voglia di farli ora = 0, quindi decommento i value, tanto è tutto da rifare */
#ifdef T2_ALFA
		val0 = obj->value[0];
		val1 = obj->value[1];
		val2 = obj->value[2];
		val3 = obj->value[3];
		val4 = obj->value[4];
		val5 = obj->value[5];

		switch ( pObj->type )
		{
		  case OBJTYPE_PILL:
		  case OBJTYPE_POTION:
		  case OBJTYPE_SCROLL:
			if ( is_valid_sn(val1) )
				val1 = HAS_SPELL_INDEX;

			if ( is_valid_sn(val2) )
				val2 = HAS_SPELL_INDEX;

			if ( is_valid_sn(val3) )
				val3 = HAS_SPELL_INDEX;
			break;

		  case OBJTYPE_STAFF:
		  case OBJTYPE_WAND:
			if ( is_valid_sn(val3) )
				val3 = HAS_SPELL_INDEX;
			break;

		  case OBJTYPE_SALVE:
			if ( is_valid_sn(val4) )
				val4 = HAS_SPELL_INDEX;

			if ( is_valid_sn(val5) )
					val5 = HAS_SPELL_INDEX;
			break;
		}

		if ( val4 || val5 )
		{
			fprintf( fpout->file, "%d %d %d %d %d %d\n",
				val0,
				val1,
				val2,
				val3,
				val4,
				val5 );
		}
		else
		{
			fprintf( fpout->file, "%d %d %d %d\n",
				val0,
				val1,
				val2,
				val3 );
		}
#endif

		fprintf( fpout->file, "%d %d %d\n",
			pObj->weight,
			pObj->cost,
			0 );	/* ex-rent */

/* (FF) idem come sopra */
#ifdef T2_ALFA
		switch ( pObj->type )
		{
		  case OBJTYPE_PILL:
		  case OBJTYPE_POTION:
		  case OBJTYPE_SCROLL:
			fprintf( fpout->file, "'%s' '%s' '%s'\n",
				is_valid_sn(pObj->value[1])  ?
				table_skill[pObj->value[1]]->name  :  "NONE",		/* (FF) table_spell o table_skill */
				is_valid_sn(pObj->value[2])  ?
				table_skill[pObj->value[2]]->name  :  "NONE",		/* (FF) table_spell o table_skill */
				is_valid_sn(pObj->value[3])  ?
				table_skill[pObj->value[3]]->name  :  "NONE" );	/* (FF) table_spell o table_skill */
			break;

		  case OBJTYPE_STAFF:
		  case OBJTYPE_WAND:
			fprintf( fpout->file, "'%s'\n",
				is_valid_sn(pObj->value[3])  ?
				table_skill[pObj->value[3]]->name  :  "NONE" );	/* (FF) table_spell o table_skill */
			break;

		  case OBJTYPE_SALVE:
			fprintf( fpout->file, "'%s' '%s'\n",
				is_valid_sn(pObj->value[4])  ?
				table_skill[pObj->value[4]]->name  :  "NONE",		/* (FF) table_spell o table_skill */
				is_valid_sn(pObj->value[5])  ?
				table_skill[pObj->value[5]]->name  :  "NONE" );	/* (FF) table_spell o table_skill */
			break;
		}
#endif

		for ( ed = pObj->first_extradescr;  ed;  ed = ed->next )
			fprintf( fpout->file, "E\n%s~\n%s~\n", ed->keyword, strip_char(ed->description, '\r') );

		for ( paf = pObj->first_affect;  paf;  paf = paf->next )
		{
			if ( (paf->location == APPLY_WEAPONSPELL
			  ||  paf->location == APPLY_WEARSPELL
			  ||  paf->location == APPLY_REMOVESPELL
			  ||  paf->location == APPLY_STRIPSN
			  ||  paf->location == APPLY_RECURRINGSPELL)
			  && is_valid_sn(paf->modifier) )
			{
				fprintf( fpout->file, "A\n%d %d\n", paf->location, table_skill[paf->modifier]->slot );
			}
			else
			{
				fprintf( fpout->file, "A\n%d %d\n", paf->location, paf->modifier );
			}
		}

		if ( pObj->first_mudprog )
		{
			for ( mprg = pObj->first_mudprog;  mprg;  mprg = mprg->next )
			{
				fputs( "Mudprog ", fpout->file );
				fwrite_mudprog( fpout, mprg );
			}

			fputs( "|\n", fpout->file );
		}
	} /* chiude il for */
	fprintf( fpout->file, "#0\n\n\n" );
	if ( install && vnum < tarea->vnum_high )
		tarea->vnum_high = vnum - 1;

	/* save rooms   */
	fprintf( fpout->file, "#ROOMS\n" );
	for ( vnum = tarea->vnum_low;  vnum <= tarea->vnum_high;  vnum++ )
	{
		MPROG_DATA *mprg;

		if ( (room = get_room_index(fpout, vnum)) == NULL )
			continue;

		if ( install )
		{
			CHAR_DATA	*victim;
			CHAR_DATA	*vnext;
			OBJ_DATA	*obj;
			OBJ_DATA	*obj_next;

			/* purge room of mobiles */
			for ( victim = room->first_person; victim; victim = vnext )
			{
				vnext = victim->next_in_room;

				if ( IS_MOB(victim) )
					extract_char( victim, TRUE );
			}

			/* purge room of objects */
			for ( obj = room->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				free_object( obj );
			}
		}
		fprintf( fpout->file, "#%d\n",	vnum						);
		fprintf( fpout->file, "%s~\n",	room->name					);
		fprintf( fpout->file, "%s~\n",	strip_char(room->description, '\r')	);

		if ( (room->tele_delay > 0 && room->tele_vnum > 0) || room->tunnel > 0 )
		{
			fprintf( fpout->file, "0 %d %d %d %d %d\n",
				*room->flags->flags,	/* (FF) ciofeca flags */
				room->sector,
				room->tele_delay,
				room->tele_vnum,
				room->tunnel );
		}
		else
			fprintf( fpout->file, "0 %d %d\n", *room->flags->flags, room->sector );	/* (FF) ciofeca flags */

		for ( xit = room->first_exit;  xit;  xit = xit->next )
		{
			if ( HAS_BIT(xit->flags, EXIT_PORTAL) )	/* don't fold portals */
				continue;

			fprintf( fpout->file, "D%d\n", xit->vdir );
			fprintf( fpout->file, "%s~\n", strip_char(xit->description, '\r') );
			fprintf( fpout->file, "%s~\n", strip_char(xit->keyword, '\r') );

			REMOVE_BIT( xit->flags, EXIT_BASHED );
			if ( xit->distance > 1 || xit->pull )
			{
				fprintf( fpout->file, "%d %d %d %d %d %d\n",
					*xit->flags->flags,
					xit->key,
					xit->vnum,
					xit->distance,
					xit->pulltype,
					xit->pull );
			}
			else
			{
				fprintf( fpout->file, "%d %d %d\n",
					*xit->flags->flags,
					xit->key,
					xit->vnum );
			}
		}

		for ( ed = room->first_extradescr;  ed;  ed = ed->next )
			fprintf( fpout->file, "E\n%s~\n%s~\n", ed->keyword, strip_char(ed->description, '\r') );

		if ( room->first_mudprog )
		{
			for ( mprg = room->first_mudprog;  mprg;  mprg = mprg->next )
			{
				fputs( "Mudprog ", fpout->file );
				fwrite_mudprog( fpout, mprg );
			}
		}

		fprintf( fpout->file, "S\n" );
	} /* chiude il for */
	fprintf( fpout->file, "#0\n\n\n" );
	if ( install && vnum < tarea->vnum_high )
		tarea->vnum_high = vnum - 1;

	/* save resets   */
	fprintf( fpout->file, "#RESETS\n" );
	for ( treset = tarea->first_reset;  treset;  treset = treset->next )
	{
		switch ( treset->command ) /* extra arg1 arg2 arg3 */
		{
		  default:  case '*': break;
		  case 'm': case 'M':
		  case 'o': case 'O':
		  case 'p': case 'P':
		  case 'e': case 'E':
		  case 'd': case 'D':
		  case 't': case 'T':
		  case 'b': case 'B':
		  case 'h': case 'H':
			fprintf( fpout->file, "%c %d %d %d %d\n",
				toupper(treset->command),
				treset->extra,
				treset->arg1,
				treset->arg2,
				treset->arg3 );
		    break;
		  case 'g': case 'G':
		  case 'r': case 'R':
			fprintf( fpout->file, "%c %d %d %d\n",
				toupper(treset->command),
				treset->extra,
				treset->arg1,
				treset->arg2 );
		    break;
		}
	}
	fprintf( fpout->file, "S\n\n\n" );

	/* save shops */
	fprintf( fpout->file, "#SHOPS\n" );
	for ( vnum = tarea->vnum_low;  vnum <= tarea->vnum_high;  vnum++ )
	{
		if ( (pMobIndex = get_mob_index(NULL, vnum)) == NULL )
			continue;

		if ( (pShop = pMobIndex->pShop) == NULL )
			continue;

		fprintf( fpout->file, " %d   %2d %2d %2d %2d %2d   %3d %3d",
			pShop->keeper,
			pShop->buy_type[0],
			pShop->buy_type[1],
			pShop->buy_type[2],
			pShop->buy_type[3],
			pShop->buy_type[4],
			pShop->profit_buy,
			pShop->profit_sell );

		fprintf( fpout->file, "        %2d %2d    ; %s\n",
			pShop->open_hour,
			pShop->close_hour,
			pMobIndex->short_descr );
	}
	fprintf( fpout->file, "0\n\n\n" );

	/* save repair shops */
	fprintf( fpout->file, "#REPAIRS\n" );
	for ( vnum = tarea->vnum_low;  vnum <= tarea->vnum_high;  vnum++ )
	{
		if ( (pMobIndex = get_mob_index(NULL,vnum)) == NULL )
			continue;

		if ( (pRepair = pMobIndex->rShop) == NULL )
			continue;

		fprintf( fpout->file, " %d   %2d %2d %2d         %3d %3d",
			pRepair->keeper,
			pRepair->fix_type[0],
			pRepair->fix_type[1],
			pRepair->fix_type[2],
			pRepair->profit_fix,
			pRepair->shop_type );

		fprintf( fpout->file, "        %2d %2d    ; %s\n",
			pRepair->open_hour,
			pRepair->close_hour,
			pMobIndex->short_descr );
	}
	fprintf( fpout->file, "0\n\n\n" );

	/* save specials */
	fputs( "#SPECIALS\n", fpout->file );
	for ( vnum = tarea->vnum_low;  vnum <= tarea->vnum_high;  vnum++ )
	{
		if ( (pMobIndex = get_mob_index(NULL, vnum)) == NULL )
			continue;

		if ( !pMobIndex->spec_fun )
			continue;

		fprintf( fpout->file, "M  %d %s\n", pMobIndex->vnum, pMobIndex->spec_funname );
	}

	fprintf( fpout->file, "S\n\n\n" );

	/* (CC) END */
	fprintf( fpout->file, "#$\n" );

	MUD_FCLOSE( fpout );
}


DO_RET do_savearea( CHAR_DATA *ch, char *argument )
{
	AREA_DATA	*tarea;
	char		 filename[MIL];

	set_char_color( AT_ADMIN, ch );

	if ( IS_MOB(ch) || get_trust(ch) < TRUST_BUILDER
	  || (!VALID_STR(argument) && !ch->pg->build_area) )
	{
		send_to_char( ch, "You don't have an assigned area to save.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
		tarea = ch->pg->build_area;
	else
	{
		bool found;

		if ( get_trust(ch) < TRUST_BUILDER )
		{
			send_to_char( ch, "You can only save your own area.\n\r" );
			return;
		}

		for ( found = FALSE, tarea = first_build;  tarea;  tarea = tarea->next )
		{
			if ( !str_cmp(tarea->filename, argument) )
			{
				found = TRUE;
				break;
			}
		}

		if ( !found )
		{
			send_to_char( ch, "Area not found.\r\n" );
			return;
		}
	}

	if ( !tarea )
	{
		send_to_char( ch, "No area to save.\r\n" );
		return;
	}

	/* Ensure not wiping out their area with save before load */
	if ( !HAS_BIT(tarea->flags, AREAFLAG_LOADED) )
	{
		send_to_char( ch, "Your area is not loaded!\r\n" );
		return;
	}

	sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
	send_to_char( ch, "Salvataggio dell'area..\r\n" );
	fold_area( tarea, filename, FALSE );
	set_char_color( AT_ADMIN, ch );
	send_to_char( ch, "Fatto.\r\n" );
}


/*
 * Da rivedere dopo le modifiche eseguite di vario genere (FF)
 */
DO_RET do_loadarea( CHAR_DATA *ch, char *argument )
{
	AREA_DATA	*tarea;
	MUD_FILE		*fp;
	int			 tmp;

	set_char_color( AT_ADMIN, ch );

	if ( IS_MOB(ch) || get_trust(ch) < TRUST_BUILDER
	  || (!VALID_STR(argument) && !ch->pg->build_area) )
	{
		send_to_char( ch, "You don't have an assigned area to load.\n\r" );
		return;
	}

	if ( !VALID_STR(argument) )
		tarea = ch->pg->build_area;
	else
	{
		bool found;

		if ( get_trust(ch) < TRUST_BUILDER )
		{
			send_to_char( ch, "You can only load your own area.\n\r" );
			return;
		}

		for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
		{
			if ( !str_cmp(tarea->filename, argument) )
			{
				found = TRUE;
				break;
			}
		}

		if ( !found )
		{
			send_to_char( ch, "Area not found.\r\n" );
			return;
		}
	}

	if ( !tarea )
	{
		send_to_char( ch, "No area to load.\r\n" );
		return;
	}

	/* Stops char from loading when already loaded */
	if ( HAS_BIT(tarea->flags, AREAFLAG_LOADED) )
	{
		send_to_char( ch, "Your area is already loaded.\r\n" );
		return;
	}

	send_to_char( ch, "Caricamento..\r\n" );
	fp = mud_fopen( BUILD_DIR, tarea->filename, "r", TRUE );
	if ( !fp )
		return;

	load_area_file( tarea, fp );

	send_to_char( ch, "Linking exits...\r\n" );
	fix_area_exits( tarea );

	if ( tarea->first_reset )
	{
		tmp = tarea->nplayer;
		tarea->nplayer = 0;
		send_to_char( ch, "Resetting area...\r\n" );
		reset_area( tarea );
		tarea->nplayer = tmp;
	}

	send_to_char( ch, "Fatto.\r\n" );
}


/*
 * Dangerous command.  Can be used to install an area that was either:
 *  (a) already installed but removed from area.lst
 *  (b) designed offline
 * The mud will likely crash if:
 *  (a) this area is already loaded
 *  (b) it contains vnums that exist
 *  (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.
 */
DO_RET do_unfoldarea( CHAR_DATA *ch, char *argument )
{
	MUD_FILE		*fp;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Unfold what?\r\n" );
		return;
	}

	fp = mud_fopen( BUILD_DIR, argument, "r", TRUE );
	if ( !fp )
		return;

	fBootDb = TRUE;
	load_area_file( last_area, fp );	/* (FF) Troppo pericoloso mettere fBootDb a true quando il mud è up, trovare un altro sistema, magari passando un argomento */
	fBootDb = FALSE;
}


DO_RET do_foldarea( CHAR_DATA *ch, char *argument )
{
	AREA_DATA	*tarea;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Fold what?\r\n" );
		return;
	}

	for ( tarea = first_area;  tarea;  tarea = tarea->next )
	{
		if ( !str_cmp(tarea->filename, argument) )
		{
			char	filename[MIL];

			send_to_char( ch, "Folding area...\r\n" );
			sprintf( filename, "%s%s", AREA_DIR, tarea->filename );
			fold_area( tarea, filename, FALSE );
			set_char_color( AT_ADMIN, ch );
			send_to_char( ch, "Fatto.\r\n" );
			return;
		}
	}

	send_to_char( ch, "No such area exists.\r\n" );
}


void write_area_list( void )
{
	AREA_DATA *tarea;
	MUD_FILE	  *fpout;

	fpout = mud_fopen( AREA_DIR, AREA_LIST, "w", TRUE );
	if ( !fpout )
		return;

	/* (TT) Inserisco qui tutti i file aiuto_*.are, me li scriverà così nella area.lst */
	fprintf( fpout->file, "aiuto_generale.are\n" );
	fprintf( fpout->file, "aiuto_comandi.are\n" );
	fprintf( fpout->file, "aiuto_mobprog.are\n" );

	for ( tarea = first_area;  tarea;  tarea = tarea->next )
		fprintf( fpout->file, "%s\n", tarea->filename );

	fprintf( fpout->file, "$\n" );
	MUD_FCLOSE( fpout );
}


/*
 * A complicated to use command as it currently exists.
 * Once area->author and area->name are cleaned up... it will be easier
 */
DO_RET do_installarea( CHAR_DATA *ch, char *argument )
{
	AREA_DATA		*tarea;
	CHAR_DATA		*author;
	char			 arg[MIL];
	char			 buf[MSL];
	int				 num;

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "&YSintassi&w: installarea <filename> [Area title]\r\n" );
		return;
	}

	for ( tarea = first_build;  tarea;  tarea = tarea->next )
	{
		if ( !str_cmp(tarea->filename, arg) )
		{
			if ( VALID_STR(argument) )
			{
				DISPOSE( tarea->name );
				tarea->name = str_dup( argument );
			}

			/* Fold area with install flag */
			send_to_char( ch, "Saving and installing file...\r\n" );
			sprintf( buf, "%s%s", AREA_DIR, tarea->filename );
			fold_area( tarea, buf, TRUE );

			/* Remove from prototype area list */
			UNLINK( tarea, first_build, last_build, next, prev );

			/* Add to real area list */
			LINK( tarea, first_area, last_area, next, prev );

			/* Fix up author if online */
			for ( author = first_player;  author;  author = author->next_player )
			{
				if ( IS_MOB(author) )
					continue;

				if ( author->pg->build_area == tarea )
				{
					/* remove area from author */
					author->pg->build_area = NULL;

					/* clear out author vnums  */
					author->pg->vnum_range_low = 0;
					author->pg->vnum_range_high = 0;
				}
			}
			top_area++;
			send_to_char( ch, "Writing area.lst...\r\n" );
			write_area_list( );
			send_to_char( ch, "Resetting new area.\r\n" );
			num = tarea->nplayer;
			tarea->nplayer = 0;
			reset_area( tarea );
			tarea->nplayer = num;
			send_to_char( ch, "Renaming author's building file.\r\n" );
			sprintf( buf, "%s%s.installed", BUILD_DIR, tarea->filename );
			sprintf( arg, "%s%s", BUILD_DIR, tarea->filename );
			if ( rename(arg, buf) != 0 )
				send_log( NULL, LOG_BUILD, "do_installarea: impossibile rinominare da %s a %s: %s", arg, buf, strerror(errno) );
			else
				send_to_char( ch, "Fatto.\r\n" );
			return;
		} /* chiude l'if */
	} /* chiude il for */
	send_to_char( ch, "No such area exists.\r\n" );
}


void add_reset_nested( AREA_DATA *tarea, OBJ_DATA *obj )
{
	int limit;

	for ( obj = obj->first_content;  obj;  obj = obj->next_content )
	{
		limit = obj->pObjProto->count;

		if ( limit < 1 )
			limit = 1;
		add_reset( tarea, 'P', 1, obj->vnum, limit, obj->in_obj->vnum );

		if ( obj->first_content )
			add_reset_nested( tarea, obj );
	}
}


/*
 * Parse a reset command string into a reset_data structure
 */
RESET_DATA *parse_reset( char *argument, CHAR_DATA *ch )
{
	ROOM_DATA	*room;
	EXIT_DATA	*pexit;
	char		 arg1[MIL];
	char		 arg2[MIL];
	char		 arg3[MIL];
	char		 arg4[MIL];
	char		 letter;
	int			 extra;
	int			 val1;
	int			 val2;
	int			 val3;
	int			 value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	extra	 = 0;
	letter = '*';
	val1	 = atoi( arg2 );
	val2	 = atoi( arg3 );
	val3	 = atoi( arg4 );

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "Reset commands: mob obj give equip door rand trap hide.\r\n" );
		return NULL;
	}

	if ( !str_cmp(arg1, "nascosto") || !str_cmp(arg1, "hide") )
	{
		if ( VALID_STR(arg2) && !get_obj_proto(NULL, val1) )
		{
			send_to_char( ch, "Reset: HIDE: no such object\r\n" );
			return NULL;
		}

		val1 = 0;
		extra = 1;
		val2  = 0;
		val3  = 0;
		letter = 'H';
	}
	else if ( !VALID_STR(arg2) )
	{
		send_to_char( ch, "Reset: not enough arguments.\r\n" );
		return NULL;
	}
	else if ( val1 < 1 || val1 >= MAX_VNUM )
	{
		send_to_char( ch, "Reset: value out of range.\r\n" );
		return NULL;
	}
	else if ( !str_cmp(arg1, "mob") )
	{
		if ( !get_mob_index(NULL, val1) )
		{
			send_to_char( ch, "Reset: MOB: no such mobile\r\n" );
			return NULL;
		}

		if ( !get_room_index(NULL, val2) )
		{
			send_to_char( ch, "Reset: MOB: no such room\r\n" );
			return NULL;
		}

		if ( val3 < 1 )
			val3 = 1;

		letter = 'M';
	}
	else if ( !str_cmp(arg1, "oggetto") || !str_cmp(arg1, "obj") )
	{
		if ( !get_obj_proto(NULL, val1) )
		{
			send_to_char( ch, "Reset: OBJ: no such object\r\n" );
			return NULL;
		}

		if ( !get_room_index(NULL, val2) )
		{
			send_to_char( ch, "Reset: OBJ: no such room\r\n" );
			return NULL;
		}

		if ( val3 < 1 )
			val3 = 1;

		letter = 'O';
	}
	else if ( !str_cmp(arg1, "dai") || !str_cmp(arg1, "give") )
	{
		if ( !get_obj_proto(NULL, val1) )
		{
			send_to_char( ch, "Reset: GIVE: no such object\r\n" );
			return NULL;
		}

		if ( val2 < 1 )
			val2 = 1;

		val3 = val2;
		val2 = 0;
		extra = 1;
		letter = 'G';
	}
	else if ( !str_cmp(arg1, "equip") )
	{
		if ( !get_obj_proto(NULL, val1) )
		{
			send_to_char( ch, "Reset: EQUIP: no such object\r\n" );
			return NULL;
		}

		if ( !is_number(arg3) )
			val2 = code_num( NULL, arg3, CODE_WEARLOC );

		if ( val2 < 0 || val2 >= MAX_WEARLOC )
		{
			send_to_char( ch, "Reset: EQUIP: invalid wear location\r\n" );
			return NULL;
		}

		if ( val3 < 1 )
			val3 = 1;

		extra  = 1;
		letter = 'E';
	}
	else if ( !str_cmp(arg1, "metti") || !str_cmp(arg1, "put") )
	{
		if ( !get_obj_proto(NULL, val1) )
		{
			send_to_char( ch, "Reset: PUT: no such object\r\n" );
			return NULL;
		}

		if ( val2 > 0 && !get_obj_proto(NULL, val2) )
		{
			send_to_char( ch, "Reset: PUT: no such container\r\n" );
			return NULL;
		}
		extra	 = UMAX(val3, 0);
		argument = one_argument(argument, arg4);
		val3	 = (is_number(argument)  ?  atoi(arg4)  :  0);

		if ( val3 < 0 )
			val3 = 0;

		letter = 'P';
	}
	else if ( !str_cmp(arg1, "porta") || !str_cmp(arg1, "door") )
	{
		if ( (room = get_room_index(NULL, val1)) == NULL )
		{
			send_to_char( ch, "Reset: DOOR: no such room\r\n" );
			return NULL;
		}

		if ( val2 < 0 || val2 > 9 )
		{
			send_to_char( ch, "Reset: DOOR: invalid exit\r\n" );
			return NULL;
		}

		if ( (pexit = get_exit(room, val2)) == NULL
		  || !HAS_BIT( pexit->flags, EXIT_ISDOOR ) )
		{
			send_to_char( ch, "Reset: DOOR: no such door\r\n" );
			return NULL;
		}

		if ( val3 < 0 || val3 > 2 )
		{
			send_to_char( ch, "Reset: DOOR: invalid door state (0 = open, 1 = close, 2 = lock)\r\n" );
			return NULL;
		}
		letter = 'D';
		value = val3;
		val3  = val2;
		val2  = value;
	}
	else if ( !str_cmp(arg1, "casuale") || !str_cmp(arg1, "rand") )
	{
		if ( !get_room_index(NULL, val1) )
		{
			send_to_char( ch, "Reset: RAND: no such room\r\n" );
			return NULL;
		}

		if ( val2 < 0 || val2 > 9 )
		{
			send_to_char( ch, "Reset: RAND: invalid max exit\r\n" );
			return NULL;
		}
		val3 = val2;
		val2 = 0;
		letter = 'R';
	}
	else if ( !str_cmp(arg1, "trappola") || !str_cmp(arg1, "trap") )
	{
		BITVECTOR	*bits = NULL;

		if ( val2 < 1 || val2 >= MAX_TRAPTYPE )
		{
			send_to_char( ch, "Reset: TRAP: invalid trap type\r\n" );
			return NULL;
		}

		if ( val3 < 0 || val3 > 10000 )
		{
			send_to_char( ch, "Reset: TRAP: invalid trap charges\r\n" );
			return NULL;
		}

		while ( VALID_STR(argument) )
		{
			argument = one_argument( argument, arg4 );
			value = code_num( NULL, arg4, CODE_TRAPFLAG );
			if ( value < 0 || value >= MAX_TRAPFLAG )
			{
				send_to_char( ch, "Reset: TRAP: bad flag\r\n" );
				return NULL;
			}

			SET_BIT( bits, value );
		}

		if ( HAS_BIT(bits, TRAPFLAG_ROOM) && HAS_BIT(bits, TRAPFLAG_OBJ) )
		{
			send_to_char( ch, "Reset: TRAP: Must specify room OR object, not both!\r\n" );
			destroybv( bits );
			return NULL;
		}

		if ( HAS_BIT(bits, TRAPFLAG_ROOM) && !get_room_index(NULL, val1) )
		{
			send_to_char( ch, "Reset: TRAP: no such room\r\n" );
			destroybv( bits );
			return NULL;
		}

		if ( HAS_BIT(bits, TRAPFLAG_OBJ)  && val1>0 && !get_obj_proto(NULL, val1) )
		{
			send_to_char( ch, "Reset: TRAP: no such object\r\n" );
			destroybv( bits );
			return NULL;
		}

		if ( !HAS_BIT(bits, TRAPFLAG_ROOM) && !HAS_BIT(bits, TRAPFLAG_OBJ) )
		{
			send_to_char( ch, "Reset: TRAP: Must specify ROOM or OBJECT\r\n" );
			destroybv( bits );
			return NULL;
		}

		/* fix order */
		if ( bits )
			extra = *(bits->flags + bits->size-1);	/* (FF) solita storia dei bits */
		value = val1;
		val1  = val2;
		val2  = value;
		letter = 'T';
		destroybv( bits );
	}
	if ( letter == '*' )
		return NULL;

	return make_reset( letter, extra, val1, val3, val2 );
}


DO_RET do_astat( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *tarea;
	int		   pdeaths = 0;
	int		   pkills  = 0;
	int		   mdeaths = 0;
	int		   mkills  = 0;
	bool	   proto = FALSE;
	bool	   found = FALSE;

	set_char_color( AT_PLAIN, ch );

	if ( !str_cmp(argument, "sommario") || !str_cmp(argument, "summary") )
	{
		for ( tarea = first_area;  tarea;  tarea = tarea->next )
		{
			pdeaths += tarea->pdeaths;
			mdeaths += tarea->mdeaths;
			pkills  += tarea->pkills;
			mkills  += tarea->mkills;
		}
		ch_printf( ch, "&WTotal pdeaths:      &w%d\r\n", pdeaths );
		ch_printf( ch, "&WTotal pkills:       &w%d\r\n", pkills );
		ch_printf( ch, "&WTotal mdeaths:      &w%d\r\n", mdeaths );
		ch_printf( ch, "&WTotal mkills:       &w%d\r\n", mkills );
		return;
	}

	for ( tarea = first_area;  tarea;  tarea = tarea->next )
	{
		if ( !str_cmp(tarea->filename, argument) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		for ( tarea = first_build;  tarea;  tarea = tarea->next )
		{
			if ( !str_cmp(tarea->filename, argument) )
			{
				found = TRUE;
				proto = TRUE;
				break;
			}
		}
	}

	if ( !found )
	{
		if ( !VALID_STR(argument) )
			tarea = ch->in_room->area;
		else
		{
			send_to_char( ch, "Area not found.  Check 'zones'.\r\n" );
			return;
		}
	}

	ch_printf( ch, "\r\n&wName:     &W%s\r\n&wFilename: &W%-20s  &wPrototype: &W%s\r\n&wAuthor:   &W%s\r\n",
		tarea->name,
		tarea->filename,
		proto  ?  "si"  :  "no",
		tarea->author );

	ch_printf( ch, "&wAge: &W%-3d  &wCurrent number of players: &W%-3d  &wMax players: &W%d\r\n",
		tarea->age,
		tarea->nplayer,
		tarea->max_players );

	if ( !proto )
	{
		if ( tarea->economy_high )
		{
			ch_printf( ch,  "&wArea economy: &W%d &wbillion and &W%d gold coins.\r\n",
				tarea->economy_high, tarea->economy_low );
		}
		else
			ch_printf( ch, "&wArea economy: &W%d &wgold coins.\r\n", tarea->economy_low );
			ch_printf( ch, "&wGold Looted:  &W%d\r\n", tarea->gold_looted );
			ch_printf( ch, "&wMdeaths: &W%d   &wMkills: &W%d   &wPdeaths: &W%d   &wPkills: &W%d   &wIllegalPK: &W%d\r\n",
				tarea->mdeaths,
				tarea->mkills,
				tarea->pdeaths,
				tarea->pkills,
				tarea->illegal_pk );
	}

	/* (FF) Maggiori informazioni riguardo ai vnum, per esempio quelli liberi per mob, obj e room */
	ch_printf( ch, "&wVnum_low: &W%5d    &wVnumhigh: &W%5d\r\n", tarea->vnum_low, tarea->vnum_high );

	ch_printf( ch, "&wSoft range: &W%d - %d", tarea->level_low, tarea->level_high );

	ch_printf( ch, " &wArea flags: &W%s\r\n", code_bit(NULL, tarea->flags, CODE_AREAFLAG) );
	ch_printf( ch, "&wResetmsg1: &W%s\r\n", tarea->resetmsg[0] ? tarea->resetmsg[0] : "(default)" );
	ch_printf( ch, "&wResetmsg2: &W%s\r\n", tarea->resetmsg[1] ? tarea->resetmsg[1] : "(default)" );
	ch_printf( ch, "&wResetmsg3: &W%s\r\n", tarea->resetmsg[2] ? tarea->resetmsg[2] : "(default)" );
	ch_printf( ch, "&wReset frequency: &W%d &wminutes.\r\n",
		tarea->reset_frequency ? tarea->reset_frequency : 15 );
}


DO_RET do_aset( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *tarea;
	char	   arg1[MIL];
	char	   arg2[MIL];
	char	   arg3[MIL];
	bool	   proto;
	bool	   found;
	VNUM	   vnum;
	int		   value;

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	vnum = atoi( argument );

	/* (GG) da aggiungere gli altri campi */
	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Usage: aset <area filename> <field> <value>\r\n"				);
		send_to_char( ch, "\r\nField being one of:\r\n"									);
		send_to_char( ch, "  vnum_low vnum_high\r\n"									);
		send_to_char( ch, "  name filename low_soft hi_soft\r\n"						);
		send_to_char( ch, "  restr_lvl_min restr_lvl_max\r\n"							);
		send_to_char( ch, "  author resetmsg1 resetmsg2 resetmsg3 resetfreq flags\r\n"	);
		return;
	}

	found = FALSE;
	proto = FALSE;
	for ( tarea = first_area;  tarea;  tarea = tarea->next )
	{
		if ( !str_cmp(tarea->filename, arg1) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		for ( tarea = first_build;  tarea;  tarea = tarea->next )
		{
			if ( !str_cmp(tarea->filename, arg1) )
			{
				found = TRUE;
				proto = TRUE;
				break;
			}
		}
	}

	if ( !found )
	{
		send_to_char( ch, "Area not found.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "name") || !str_cmp(arg2, "nome") )
	{
		DISPOSE( tarea->name );
		tarea->name = str_dup( argument );
	}
	else if ( !str_cmp(arg2, "nomefile") || !str_cmp(arg2, "filename") )
	{
		DISPOSE( tarea->filename );
		tarea->filename = str_dup( argument );
		write_area_list( );
	}
	else if ( !str_cmp(arg2, "economy_low") )
	{
		tarea->economy_low = vnum;
	}
	else if ( !str_cmp(arg2, "economy_high") )
	{
		tarea->economy_high = vnum;
	}
	else if ( !str_cmp(arg2, "vnum_low") )
	{
		tarea->vnum_low = vnum;
	}
	else if ( !str_cmp(arg2, "vnum_high") )
	{
		tarea->vnum_high = vnum;
	}
	else if ( !str_cmp(arg2, "low_soft") )
	{
		if ( vnum < 0 || vnum > LVL_LEGEND )
		{
			send_to_char( ch, "That is not an acceptable value.\r\n" );
			return;
		}

		tarea->level_low = vnum;
	}
	else if ( !str_cmp(arg2, "hi_soft") )
	{
		if ( vnum < 0 || vnum > LVL_LEGEND )
		{
			send_to_char( ch, "That is not an acceptable value.\r\n" );
			return;
		}

		tarea->level_high = vnum;
	}
	else if ( !str_cmp(arg2, "autore") || !str_cmp(arg2, "author") )
	{
		DISPOSE( tarea->author );
		tarea->author = str_dup( argument );
	}
	else if ( !str_cmp(arg2, "resetmsg1") )
	{
		DISPOSE( tarea->resetmsg[0] );
		tarea->resetmsg[0] = str_dup( argument );
	}
	else if ( !str_cmp(arg2, "resetmsg2") )
	{
		DISPOSE( tarea->resetmsg[1] );
		tarea->resetmsg[1] = str_dup( argument );
	}
	else if ( !str_cmp(arg2, "resetmsg3") )
	{
		DISPOSE( tarea->resetmsg[2] );
		tarea->resetmsg[2] = str_dup( argument );
	}
	else if ( !str_cmp(arg2, "resetfreq") )
	{
		tarea->reset_frequency = vnum;
	}
	else if ( !str_cmp(arg2, "flags") )
	{
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Usage: aset <filename> flags <flag> [flag]...\r\n" );
			return;
		}

		while ( VALID_STR(argument) )
		{
			argument = one_argument( argument, arg3 );
			value = code_num( NULL, arg3, CODE_AREAFLAG );

			if ( value < 0 || value >= MAX_AREAFLAG )
			{
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			}
			else
			{
				if ( HAS_BIT(tarea->flags, value) )
					REMOVE_BIT( tarea->flags, value );
				else
					SET_BIT( tarea->flags, value );
			}
		}
		return;
	}
	else
		send_command( ch, "aset", CO );

	send_to_char( ch, "Fatto.\r\n" );
}


DO_RET do_blist( CHAR_DATA *ch, char *argument )
{
	AREA_DATA		*tarea;
	char			 arg1[MIL];
	char			 arg2[MIL];
	char			 arg3[MIL];
	int				 lrange;
	int				 trange;
	VNUM			 vnum;

	set_char_color( AT_PLAIN, ch );

	if ( IS_MOB(ch) || !VALID_STR(argument)
	  || (!ch->pg->build_area && get_trust(ch) < TRUST_BUILDER) )	/* (TT) */
	{
		send_to_char( ch, "&YYou don't have an assigned area.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_command( ch, "blist m", CO );
		send_command( ch, "blist o", CO );
		send_command( ch, "blist r", CO );
		return;
	}

	tarea = ch->pg->build_area;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if ( VALID_STR(arg1) && is_number(arg1) )
	{
		send_to_char( ch, "Devi inserire il tipo di lista che vuo visualizzare. (mob, obj, room)" );
		return;
	}
	if ( VALID_STR(arg2) && !is_number(arg2) )
	{
		send_to_char( ch, "Devi inserire come vnum minore un numero." );
		return;
	}
	if ( VALID_STR(arg3) && !is_number(arg3) )
	{
		send_to_char( ch, "Devi inserire come vnum maggiore un numero." );
		return;
	}

	if ( tarea )
	{
		if ( !VALID_STR(arg2) )
			lrange = tarea->vnum_low;
		else
			lrange = atoi( arg2 );

		if ( !VALID_STR(arg3) )
			trange = tarea->vnum_high;
		else
			trange = atoi( arg3 );

		if ( get_trust(ch) < TRUST_BUILDER
		  && (lrange < tarea->vnum_low || trange > tarea->vnum_high) )
		{
			send_to_char( ch,"&YThat is out of your vnum range.\r\n" );
			return;
		}
	}
	else
	{
		lrange = ( is_number(arg2)  ?  atoi(arg2)  :  1 );
		trange = ( is_number(arg3)  ?  atoi(arg3)  :  1 );
	}

	for ( vnum = lrange;  vnum <= trange;  vnum++ )
	{
		if ( str_prefix(arg1, "rooms") || str_prefix(arg1, "stanze") )
		{
			ROOM_DATA	*room;

			room = get_room_index( NULL, vnum );
			if ( !room )
				continue;
			pager_printf( ch, "%5d) %s\r\n", vnum, room->name );
		}
		else if ( is_name_prefix(arg1, "objects oggetti") )
		{
			OBJ_PROTO_DATA *pObj;

			pObj = get_obj_proto( NULL, vnum );
			if ( !pObj )
				continue;

			pager_printf( ch, "%5d) %-20s (%s)\r\n",
				vnum, pObj->name, pObj->short_descr );
		}
		else if ( str_prefix(arg1, "mobiles") )
		{
			MOB_PROTO_DATA *pMobIndex;

			pMobIndex = get_mob_index( NULL, vnum );
			if ( !pMobIndex )
				continue;

			pager_printf( ch, "%5d) %-20s '%s'\r\n",
				vnum, pMobIndex->name, pMobIndex->short_descr );
		}
	}
}


DO_RET do_rdelete( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*location;
	char		 arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_rdelete: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	argument = one_argument( argument, arg );
	return;									/* <----- Temporarily disable this command. */
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Delete which room?\r\n" );
		return;
	}

	/* Find the room. */
	location = find_location( ch, arg );
	if ( !location )
	{
		send_to_char( ch, "No such location.\r\n" );
		return;
	}

	/* Does the player have the right to delete this room? */
	if ( get_trust(ch) < TRUST_BUILDER
	  && (location->vnum < ch->pg->vnum_range_low
	  ||  location->vnum > ch->pg->vnum_range_high) )
	{
		send_to_char( ch, "That room is not in your assigned range.\r\n" );
		return;
	}

	/* We could go to the trouble of clearing out the room, but why? */
	/* free_room does that anyway, but this is probably safer:) */
	if ( location->first_person || location->first_content )
	{
		send_to_char( ch, "The room must be empty first.\r\n" );
		return;
	}

	/* Ok, we've determined that the room exists, it is empty and the
	   player has the authority to delete it, so let's dump the thing.
	   The function to do it is in db.c so it can access the top-room
	   variable. */
	/* Wipe out resets too */
	{
		RESET_DATA *pReset;
		AREA_DATA *pArea;

		pArea = location->area;

		for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
		{
			if ( is_room_reset( pReset, location, pArea ))
				delete_reset( pArea, pReset );
		}
	}

	free_room( location );
	send_to_char( ch, "Room deleted.\r\n" );
}


/*
 * Comando builder per distruggere un oggetto indicizzato
 */
DO_RET do_odelete( CHAR_DATA *ch, char *argument )
{
	OBJ_PROTO_DATA	*pObj;
	OBJ_DATA		*temp;
	char			 arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_odelete: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Delete which object?\r\n" );
		return;
	}

	/* Find the object. */
	pObj = get_obj_proto( NULL, atoi(arg) );
	if ( !pObj )
	{
		temp = get_obj_here( ch, arg );
		if ( !temp )
		{
			send_to_char( ch, "No such object.\r\n" );
			return;
		}

		pObj = temp->pObjProto;
	}

	/* Does the player have the right to delete this object? */
	if ( get_trust(ch) < TRUST_BUILDER
	  && (pObj->vnum < ch->pg->vnum_range_low
	  ||  pObj->vnum > ch->pg->vnum_range_high) )
	{
		send_to_char( ch, "That object is not in your assigned range.\r\n" );
		return;
	}

	/* Ok, we've determined that the room exists, it is empty and the
	   player has the authority to delete it, so let's dump the thing.
	   The function to do it is in db.c so it can access the top-room
	   variable. */
	free_object_proto( pObj );
	send_to_char( ch, "Object deleted.\r\n" );
}


DO_RET do_mdelete( CHAR_DATA *ch, char *argument )
{
	MOB_PROTO_DATA *pMobIndex;
	CHAR_DATA	   *temp;
	char			arg[MIL];

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Delete which mob?\r\n" );
		return;
	}

	if ( !is_number(arg) )
	{
		send_to_char( ch, "Quale vnum di mob?" );
		return;
	}

	/* Find the mob. */
	pMobIndex = get_mob_index( NULL, atoi(arg) );
	if ( !pMobIndex )
	{
		temp = get_mob_room( ch, arg, TRUE );
		if ( !temp )
		{
			send_to_char( ch, "No such mob.\r\n" );
			return;
		}

		pMobIndex = temp->pIndexData;
	}

	/* Does the player have the right to delete this mobile? */
	if ( get_trust(ch) < TRUST_BUILDER
	  && (pMobIndex->vnum < ch->pg->vnum_range_low
	  ||  pMobIndex->vnum > ch->pg->vnum_range_high) )
	{
		send_to_char( ch, "That mob is not in your assigned range.\r\n" );
		return;
	}

	/* Ok, we've determined that the mob exists and the player has the
	   authority to delete it, so let's dump the thing.
	   The function to do it is in db.c so it can access the top_mob_proto
	   variable. */
	free_mobile_proto( pMobIndex );
	send_to_char( ch, "Mob deleted.\r\n" );
}
