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
 >				Modulo dei comandi degli Implementatori			   			<
\****************************************************************************/

#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mud.h"
#include "affect.h"
#include "bank.h"
#include "build.h"
#include "calendar.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "editor.h"
#include "gramm.h"
#include "interpret.h"
#include "liquid.h"
#include "locker.h"
#include "mccp.h"
#include "movement.h"
#include "mprog.h"
#include "nanny.h"
#include "note.h"
#include "object_interact.h"
#include "morph.h"
#include "reset.h"
#include "room.h"
#include "save.h"
#include "shop.h"
#include "timer.h"
#include "values.h"


/*
 * Ritorna la stanza di vnum passato, oppure il mob o l'oggetto di vnum o nome passato
 */
ROOM_DATA *find_location( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	char	   arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "find_location: ch è NULL" );
		return NULL;
	}

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "find_location: argument è NULL con ch %s",
			IS_MOB(ch) ? ch->short_descr : ch->name );
		return NULL;
	}

	argument = one_argument( argument, arg );

	if ( is_number(arg) )
		return get_room_index( NULL, atoi(arg) );

	if ( !str_cmp(arg, "_pk") )	/* "Goto _pk", "at _pk", etc */
		return get_room_index( NULL, last_pkroom );

	if ( !str_cmp(arg, "_m") )
		argument = one_argument( argument, arg );
	victim = get_char_mud( ch, arg, TRUE );
	if ( victim )
		return victim->in_room;

	if ( !str_cmp(arg, "_o") )
		argument = one_argument( argument, arg );
	obj = get_obj_world( ch, arg );
	if ( obj )
		return obj->in_room;

	return NULL;
}


/*
 * Funzione per i comandi dei vari tipi di goto
 */
static DO_RET goto_handler( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*location;
	ROOM_DATA	*in_room;
	CHAR_DATA	*fch;
	CHAR_DATA	*fch_next;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Andare dove?\r\n" );
		return;
	}

	location = find_location( ch, argument );
	if ( !location )
	{
		send_to_char( ch, "No such location.\r\n" );
		return;
	}

	in_room = ch->in_room;
	if ( ch->fighting )
		stop_fighting( ch, TRUE );

	act( AT_ACTION, "[ADM] $n se ne va usando il goto.", ch, NULL, NULL, TO_ADMIN );

	ch->regoto = ch->in_room->vnum;
	char_from_room( ch );
	if ( ch->mount )
	{
		char_from_room( ch->mount );
		char_to_room( ch->mount, location );
	}
	char_to_room( ch, location );

	act( AT_ACTION, "[ADM] $n arriva usando il goto.", ch, NULL, NULL, TO_ADMIN );

	send_command( ch, "look", CO );

	if ( ch->in_room == in_room )
		return;

	for ( fch = in_room->first_person;  fch;  fch = fch_next )
	{
		fch_next = fch->next_in_room;

		if ( fch->master == ch && IS_ADMIN(fch) )
		{
			char	buf[MSL];

			act( AT_ACTION, "Segui $N.", fch, NULL, ch, TO_CHAR );
			sprintf( buf, "goto %s", argument );
			send_command( fch, buf, CO );
		}
/*		Experimental change so adm's personal mobs follow them */
		else if ( IS_MOB(fch) && fch->master == ch )
		{
			char_from_room (fch);
			char_to_room( fch, location );
		}
	}
}

/* goto che porta da un mob */
DO_RET do_mgoto( CHAR_DATA *ch, char *argument )
{
	char	buf[MIL];

	sprintf( buf, "_m %s", argument );
	goto_handler( ch, buf );
}

/* goto che porta da un oggetto */
DO_RET do_ogoto( CHAR_DATA *ch, char *argument )
{
	char	buf[MIL];

	sprintf( buf, "_o %s", argument );
	goto_handler( ch, buf );
}

/* goto che porta da un stanza */
DO_RET do_rgoto( CHAR_DATA *ch, char *argument )
{
	goto_handler( ch, argument );
}

DO_RET do_regoto( CHAR_DATA *ch, char *argument )
{
	char	buf[MIL];

	sprintf( buf, "%d", ch->regoto );
	goto_handler( ch, buf );
}


DO_RET do_transfer( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	ROOM_DATA		*location;
	CHAR_DATA		*victim;
	char			 arg[MIL];

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Transfer whom (and where)?\r\n" );
		return;
	}

	argument = one_argument( argument, arg );

	if ( !str_cmp_all(arg, FALSE) && get_trust(ch) >= TRUST_NEOBUILDER )
	{
		for ( d = first_descriptor;  d;  d = d->next )
		{
			char	buf[MSL];

			if ( d->connected != CON_PLAYING )	continue;
			if ( d->character == ch )			continue;
			if ( !d->character->in_room )		continue;

			sprintf( buf, "transfer %s %s", d->character->name, argument );
			send_command( ch, buf, CO );
		}
		return;
	}

	/* Optional location parameter. */
	if ( !VALID_STR(argument) )
	{
		location = ch->in_room;
	}
	else
	{
		location = find_location( ch, argument );
		if ( !location )
		{
			send_to_char( ch, "No such location.\r\n" );
			return;
		}
	}

	victim = get_char_mud( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "They aren't here.\r\n" );
		return;
	}

	if ( !victim->in_room )
	{
		send_to_char( ch, "They have no physical location!\r\n" );
		return;
	}

	if ( victim->fighting )
		stop_fighting( victim, TRUE );

	act( AT_MAGIC, "D'improvviso un tunnel astrale si apre davanti a $n risucchiandolo.", victim, NULL, NULL, TO_ROOM );
	victim->retran = victim->in_room->vnum;
	char_from_room( victim );
	char_to_room( victim, location );
	act( AT_MAGIC, "Un tunnel astrale si apre d'improvviso e ne compare $n.", victim, NULL, NULL, TO_ROOM );

	if ( ch != victim )
		act( AT_ADMIN, "D'improvviso un tunnel astrale si apre davanti a me e ne vengo risucchiato.", ch, NULL, victim, TO_VICT );

	send_command( victim, "look", CO );
}


DO_RET do_retran( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   buf[MIL];

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Retransfer whom?\r\n" );
		return;
	}

	victim = get_char_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "They aren't here.\r\n" );
		return;
	}

	sprintf( buf, "transfer '%s' %d", victim->name, victim->retran );
	send_command( ch, buf, CO );
}


/*
 * Invia un comando alla stanza, oggetto, ultima zona pk oppure nome pg.
 */
DO_RET do_at( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*location = NULL;
	ROOM_DATA	*original;
	CHAR_DATA	*wch = NULL;
	OBJ_DATA	*obj;
	char		 arg[MIL];

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_to_char( ch, "At where what?\r\n" );
		return;
	}

	/* ex-atobj */
	if ( is_name(arg, "ogg obj") )
	{
		argument = one_argument( argument, arg );
		if ( (obj = get_obj_world(ch, arg)) == NULL || !obj->in_room )
		{
			send_to_char( ch, "No such object in existance.\r\n" );
			return;
		}

		location = obj->in_room;
	}
	else
	{
		if ( is_number(arg) )
			location = get_room_index( NULL, atoi(arg) );
		else if ( is_name(arg, "stanza room") )
		{
			argument = one_argument( argument, arg );
			location = get_room_index( NULL, atoi(arg) );
		}
		else if ( !str_cmp(arg, "pk") )
		{
			location = get_room_index( NULL, last_pkroom );
		}
		else if ( (wch = get_char_mud(ch, arg, TRUE)) == NULL || wch->in_room == NULL )
		{
			send_to_char( ch, "No such mobile or player in existance.\r\n" );
			return;
		}

		if ( !location && wch )
			location = wch->in_room;
	}

	if ( !location )
	{
		send_to_char( ch, "No such location exists.\r\n" );
		return;
	}

	set_char_color( AT_PLAIN, ch );
	original = ch->in_room;
	char_from_room( ch );
	char_to_room( ch, location );

	interpret( ch, argument );

	if ( !char_died(ch) )
	{
		char_from_room( ch );
		char_to_room( ch, original );
	}
}


DO_RET do_rat( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*location;
	ROOM_DATA	*original;
	char		 arg1[MIL];
	char		 arg2[MIL];
	int			 Start;
	int			 End;
	VNUM		 vnum;

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: rat <start> <end> <command>\r\n" );
		return;
	}

	Start = atoi( arg1 );
	End   = atoi( arg2 );
	if ( Start < 1 || End >= MAX_VNUM || End < Start || Start > End || Start == End )
	{
		send_to_char( ch, "Invalid range.\r\n" );
		return;
	}

	if ( is_name(argument, "fine quit") )
	{
		send_to_char( ch, "I don't think so!\r\n" );
		return;
	}

	original = ch->in_room;
	for ( vnum = Start;  vnum <= End;  vnum++ )
	{
		if ( (location = get_room_index(NULL, vnum)) == NULL )
			continue;

		char_from_room( ch );
		char_to_room( ch, location );
		interpret( ch, argument );
	}
	char_from_room( ch );
	char_to_room( ch, original );
	send_to_char( ch, "Fatto.\r\n" );
}


/*
 * Elenca i maestri mob in tutto il mud
 * (FF) Elencare anche quello che insegna per i maestri
 */
DO_RET do_masters( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *mob;
	bool	   found = FALSE;

	send_to_pager( ch, "Maestri del Mud:\r\n" );
	for ( mob = first_char;  mob;  mob = mob->next )
	{
		if ( IS_PG(mob) )
			continue;

		if ( HAS_BIT_ACT(mob, MOB_PRACTICE) )
		{
			found = TRUE;
			pager_printf( ch, "Pratiche: #%-5d %s\r\n", mob->pIndexData->vnum, mob->short_descr );
		}

		if ( HAS_BIT_ACT(mob, MOB_SCHOLAR) )
		{
			int x;

			found = TRUE;
			pager_printf( ch, "Lingue:   #%-5d %s\r\n", mob->pIndexData->vnum, mob->short_descr );
			send_to_pager( ch, "  insegna:" );
			for ( x = 0;  x < MAX_LANGUAGE;  x++ )
			{
				if ( knows_language(mob, x, mob) )
					pager_printf( ch, " %s", table_language[x]->name );
			}
			send_to_pager( ch, "\r\n" );
		}

		if ( HAS_BIT_ACT(mob, MOB_TRAIN) )
		{
			found = TRUE;
			pager_printf( ch, "Train:    #%-5d %s\r\n", mob->pIndexData->vnum, mob->short_descr );
		}
	}

	/* (FF) bisogna aggiungere i mob particolari per l'insegnamento di alcune skills */

	if ( found == FALSE )
		send_to_pager( ch, "Nessuno.\r\n" );
}


/*
 * Elenca le cavalcature del mud
 */
DO_RET do_riders( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		*mob;
	CHAR_DATA		*vch;
	bool			 found = FALSE;

	send_to_pager( ch, "Creature cavalcabili del Mud:\r\n" );
	for ( mob = first_char;  mob;  mob = mob->next )
	{
		if ( IS_PG(mob) )
			continue;

		if ( HAS_BIT_ACT(mob, MOB_MOUNTABLE) )
		{
			found = TRUE;
			pager_printf( ch, "#%-5d %s\r\n", mob->pIndexData->vnum, mob->short_descr );
		}
	}

	if ( found == FALSE )
		send_to_pager( ch, "Nessuno.\r\n" );

	/* Ora elenca i giocatori e cosa cavalcano */
	found = FALSE;
	send_to_pager( ch, "\r\nPlayer che cavalcano creature:\r\n" );
	for ( vch = first_player;  vch;  vch = vch->next_player )
	{
		if ( vch->mount )
		{
			found = TRUE;

			pager_printf( ch, "%-16s cavalca #%-5d %s\r\n",
				vch->name,
				vch->mount->pIndexData->vnum,
				vch->mount->short_descr );
		}
	}

	if ( !found )
		send_to_pager( ch, "Nessuno.\r\n" );
}


/*
 * Elenca tutte le dt del Mud
 */
DO_RET do_listdt( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;

	send_to_pager( ch, "Lista delle DT nelle aree.\r\n" );
	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		ROOM_DATA	*pRoom;
		int			 cou;
		bool		 show;

		show = FALSE;
		for ( cou = 0;  cou < MAX_KEY_HASH;  cou++ )
		{
			if ( !room_index_hash[cou] )
				continue;

			for ( pRoom = room_index_hash[cou];  pRoom;  pRoom = pRoom->next )
			{
				if ( pRoom->area != pArea )
					continue;

				if ( HAS_BIT(pRoom->flags, ROOM_DEATH) )
				{
					if ( !show )
						pager_printf( ch, "\r\n&G- %s&w:\r\n", pArea->name );

					pager_printf( ch, "#%5d %-16s %s\r\n",
						pRoom->vnum,
						code_name(NULL, pRoom->sector, CODE_SECTOR),
						pRoom->name );
				}
			}
		}
	}
}


/*
 * Elenca tutti i pg con un obiettivo
 */
DO_RET do_listobjective( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	bool		 offline = FALSE;

	send_to_pager( ch, "&YSintassi&w:  listobjective\r\n" );
	send_to_pager( ch, "&YSintassi&w:  listobjective offline\r\n\r\n" );

	if ( !str_prefix(argument, "offline") )
		offline = TRUE;

	pager_printf( ch, "%-12.12s %5s %s\r\n", "Nome", "Room", "Obiettivo" );
	for ( victim = offline ? first_offline : first_player;  victim;  victim = offline ? victim->next : victim->next_player )
	{
		if ( victim == ch )
			continue;

		/* se viene richiesta la lista di offline salta i pg che sono attualmente online */
		if ( offline && get_player_mud(ch, victim->name, FALSE) )
			continue;

		if ( VALID_STR(victim->pg->objective) )
		{
			pager_printf( ch, "%-12.12s %5d %s\r\n",
				victim->name,
				victim->in_room->vnum,
				victim->pg->objective );
		}
	}
}


/*
 * Comando admin per visualizzare le statistiche di una stanza
 */
DO_RET do_rstat( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*location;
	OBJ_DATA	*obj;
	CHAR_DATA	*rch;
	EXIT_DATA	*pexit;
	AFFECT_DATA	*paf;
	char		*sect;
	char		 buf[MSL];
	int			 cnt;
	static char	*dir_text[] = { "n",  "e",  "s",  "w",  "u", "d",
									"ne", "no", "se", "so", "?" };

	if ( !VALID_STR(argument) )
		location = ch->in_room;
	else
		location = find_location( ch, argument );

	if ( !location )
	{
		send_to_char( ch, "No such location.\r\n" );
		return;
	}

	ch_printf( ch, "&cName: &w%s\r\n&cArea: &w%s  &cFilename: &w%s\r\n",
		location->name,
		location->area  ?  location->area->name      :  "None????",
		location->area  ?  location->area->filename  :  "None????" );

	sect = code_name( NULL, ch->in_room->sector, CODE_SECTOR );

	ch_printf( ch, "&cVnum: &w%d  &cSector: &w%d (%s)  &cLight: &w%d",
		location->vnum, location->sector, sect, location->light );

	if ( location->tunnel > 0 )
		ch_printf( ch, "   &cTunnel: &W%d", location->tunnel );

	send_to_char( ch, "\r\n" );

	if ( location->tele_delay > 0 || location->tele_vnum > 0 )
		ch_printf( ch, "&cTeleDelay: &R%d   &cTeleVnum: &R%d\r\n", location->tele_delay, location->tele_vnum );

	ch_printf( ch, "&cRoom flags: &w%s\r\n", code_bit(NULL, location->flags, CODE_ROOM) );
	ch_printf( ch, "&cDescription:\r\n&w%s", location->description );

	if ( location->first_extradescr )
	{
		EXTRA_DESCR_DATA *ed;

		send_to_char( ch, "&cExtra description keywords: &w'" );
		for ( ed = location->first_extradescr;  ed;  ed = ed->next )
		{
			send_to_char( ch, ed->keyword );

			if ( ed->next )
				send_to_char( ch, " " );
		}
		send_to_char( ch, "'\r\n" );
	}

	for ( paf = location->first_affect;  paf;  paf = paf->next )
	{
		ch_printf( ch, "&cAffect: &w%s &cby &w%d.\r\n",
			code_name(NULL, paf->location, CODE_APPLY), paf->modifier );
	}

	send_to_char( ch, "&cCharacters:&w" );
	for ( rch = location->first_person;  rch;  rch = rch->next_in_room )
	{
		one_argument( rch->name, buf );
		ch_printf( ch, " %s", buf );
	}

	ch_printf( ch, "\r\n&cObjects:    &w" );
	for ( obj = location->first_content;  obj;  obj = obj->next_content )
	{
		one_argument( obj->name, buf );
		ch_printf( ch, " %s", buf );
	}
	send_to_char( ch, "\r\n" );

	if ( location->first_exit )
		send_to_char( ch,  "&c------------------- &wEXITS &c-------------------\r\n" );

	ch_printf( ch, "&cUscite per la stanza '&W%s&c'  Vnum &W%d\r\n", location->name, location->vnum );

	for ( cnt = 0, pexit = location->first_exit;  pexit;  pexit = pexit->next )
	{
		ch_printf( ch,
			"&W%2d) &w%2s to %-5d  &cKey: &w%d  &cFlags: &w%s  &cKeywords: '&w%s&c'\r\n"
			"     Exdesc: &w%s     &cBack link: &w%d  &cVnum: &w%d  &cDistance: &w%d  &cPulltype: &w%s  &cPull: &w%d\r\n",
			++cnt,
			dir_text[pexit->vdir],
			(pexit->to_room)  ?  pexit->to_room->vnum  :  0,
			pexit->key,
			code_bit(NULL, pexit->flags, CODE_EXIT),
			pexit->keyword,
			(VALID_STR(pexit->description))  ?  pexit->description  :  "(none).\r\n",
			(pexit->rexit)  ?  pexit->rexit->vnum  :  0,
			pexit->rvnum,
			pexit->distance,
			code_name(NULL, pexit->pulltype, CODE_PULL),
			pexit->pull );
	}
}


/*
 * Face-lift
 */
DO_RET do_ostat( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *paf;
	OBJ_DATA	*obj;

	set_char_color( AT_CYAN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Ostat what?\r\n" );
		return;
	}

	if ( (obj = get_obj_world(ch, argument)) == NULL )
	{
		send_to_char( ch, "Nothing like that in hell, earth, or heaven.\r\n" );
		return;
	}

	pager_printf( ch, "&cName: &C%s\r\n",				obj->name );
	pager_printf( ch, "&cVnum: &w%d  ",					obj->vnum );
	pager_printf( ch, "&cArea: &w%s\n\r",				obj->area->name );
	pager_printf( ch, "&cType: &w%s  ",					code_name(NULL, obj->type, CODE_OBJTYPE) );
	pager_printf( ch, "&cCount:  &w%d  ",				obj->pObjProto->count );
	pager_printf( ch, "&cGroupCount: &w%d\r\n",			obj->count );
	pager_printf( ch, "&cSerial#: &w%d  ",				obj->serial );
	pager_printf( ch, "&cTopSerialProto#: &w%d  ",		obj->pObjProto->serial );
	pager_printf( ch, "&cTopSerial#: &w%d\r\n",			cur_obj_serial );
	pager_printf( ch, "&cShort description: &C%s\r\n",	obj->short_descr );
	pager_printf( ch, "&cLong description : &C%s\r\n",	obj->long_descr );
#ifdef T2_ALFA
	pager_printf( ch, "&cDescription      : &C%s\r\n",	obj->description );
#endif
	if ( VALID_STR(obj->action_descr) )
		pager_printf( ch, "&cAction description: &w%s\r\n", obj->action_descr );

	pager_printf( ch, "&cWear flags : &w%s\r\n",	code_bit(NULL, obj->wear_flags, CODE_WEARFLAG) );
	pager_printf( ch, "&cExtra flags: &w%s\r\n",	code_bit(NULL, obj->extra_flags, CODE_OBJFLAG) );
	pager_printf( ch, "&cMagic flags: &w%s\r\n",	code_bit(NULL, obj->magic_flags, CODE_OBJMAGIC) );
	pager_printf( ch, "&cNumber: &w%d/%d   ", 1,	obj->count );
	pager_printf( ch, "&cWeight: &w%d/%d   ",		obj->weight, get_obj_weight(obj) );
	pager_printf( ch, "&cLayers: &w%d   ",			print_bitvector(obj->layers) );	/* (FF) mhhmm, il layers, altra cosa antipatica da analizzare */
	pager_printf( ch, "&cWear_loc: &w%d\r\n",		obj->wear_loc );
	pager_printf( ch, "&cCost: &Y%d  ",				obj->cost );
	pager_printf( ch, "&cTimer: " );

	if ( obj->timer > 0 )
		pager_printf( ch, "&R%d  ", obj->timer );
	else
		pager_printf( ch, "&w%d  ", obj->timer );

	pager_printf( ch, "&cLevel: &P%d\r\n",		obj->level );
	pager_printf( ch, "&cIn room: &w%d  ",		(obj->in_room) ? obj->in_room->vnum : 0 );
	pager_printf( ch, "&cIn object: &w%s  ",	(obj->in_obj) ? obj->in_obj->short_descr : "Nessuno" );

	if ( obj->carried_by )
	{
		pager_printf( ch, "&cCarried by: &C%s", obj->carried_by->name );
		if ( IS_MOB(obj->carried_by) )
			pager_printf( ch, " (#%d)", obj->carried_by->pIndexData->vnum );
		send_to_pager( ch, "\r\n" );
	}
	else
	{
		pager_printf( ch, "&cCarried by: &C%s\r\n", "Nessuno" );
	}

	show_values( ch, obj );

	/* Elenca le keyword per le extra */
	if ( obj->first_extradescr )
	{
		EXTRA_DESCR_DATA *ed;

		send_to_pager( ch, "Extra description keywords: '" );
		for ( ed = obj->first_extradescr;  ed;  ed = ed->next )
			pager_printf( ch, "%s ", ed->keyword );

		send_to_pager( ch, "'.\r\n" );
	}

	for ( paf = obj->first_affect;  paf;  paf = paf->next )
	{
		ch_printf( ch, "&cAffects &w%s &cby &w%d.\r\n",
			code_name(NULL, paf->location, CODE_APPLY), paf->modifier );
	}
}


DO_RET do_mstat( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *paf;
	CHAR_DATA	*victim;
	SKILL_DATA	*skill;
	char		 buf[MSL];
	char		 temp[MSL];
	char		 lfbuf[MSL];
	char		 mnbuf[MSL];
	char		 mvbuf[MSL];
	char		 slbuf[MSL];
	int			 x;

	set_char_color( AT_CYAN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Mstat chi?\r\n" );
		return;
	}

	victim = get_char_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo mob non c'è.\r\n" );
		return;
	}

	if ( get_trust(ch) < get_trust(victim) )
	{
		set_char_color( AT_ADMIN, ch );
		send_to_char( ch, "L'aura divina che lo attornia previene uno sguardo attento.\r\n" );
		return;
	}

	pager_printf( ch, "\r\n&c%s: &C%-20s", IS_MOB(victim)  ?  "Mobile name"  :  "Name", victim->name );
	if ( IS_PG(victim) )
		pager_printf( ch, "&cPuò killare? : &w%s", can_pkill(victim, NULL) ? "SI" : "no" );

	if ( IS_MOB(victim) )
		ch_printf( ch, "&cArea: &w%s\n\r", victim->pIndexData->area->name );

	if ( IS_PG(victim) && victim->pg->clan )
		pager_printf( ch, "   &cClan: &w%s", victim->pg->clan->name );

	pager_printf( ch, "\r\n" );
	if ( IS_PG(victim) )
	{
		if ( ch->pg->trust <= TRUST_MASTER )
		{
			pager_printf( ch, "&cTrust: &w%s\r\n",
				code_str(NULL, get_trust(victim), CODE_TRUST) );
		}
		else
		{
			if ( VALID_STR(victim->desc->host) )
			{
				pager_printf( ch, "&cHost: &w%s   Descriptor: %d  &cTrust: &w%s\r\n",
					victim->desc->host,
					victim->desc->descriptor,
					code_str(NULL, get_trust(victim), CODE_TRUST) );
			}
			else
			{
				pager_printf( ch, "&cHost: &w%s   Descriptor: %s  &cTrust: &w%s\r\n",
					get_host(victim),
					victim->desc ? itostr(victim->desc->descriptor): "(n/a)",
					code_str(NULL, get_trust(victim), CODE_TRUST) );
			}
		}
	}

	if ( IS_PG(victim) && victim->pg->release_date != 0 )
	{
		pager_printf( ch, "&cHelled until %21.21s by %s.\r\n",
			friendly_ctime(&victim->pg->release_date), victim->pg->helled_by);
	}

	pager_printf( ch, "&cVnum: &w%-5d    &cSex: &w%-6s    &cRoom: &w%-5d    &cCount: &w%d   &cKilled: &w%d\r\n",
		(IS_MOB(victim))  ?  victim->pIndexData->vnum  :  0,
		(victim->sex == SEX_FEMALE)  ?  "femmina"  :  "maschio",
		(victim->in_room == NULL)  ?  0  :  victim->in_room->vnum,
		(IS_MOB(victim))  ?  victim->pIndexData->count   :  1,
		(IS_MOB(victim))  ?  victim->pIndexData->killed  :  victim->pg->mdeaths+victim->pg->pdeaths );

	pager_printf( ch, "&cStr: &C%3d&c  Res: &C%3d&c  Con: &C%3d&c  Int: &C%3d&c  Coc: &C%3d&c  Wil: &C%3d&c\r\n",
		get_curr_attr(victim, ATTR_STR),
		get_curr_attr(victim, ATTR_RES),
		get_curr_attr(victim, ATTR_CON),

		get_curr_attr(victim, ATTR_INT),
		get_curr_attr(victim, ATTR_COC),
		get_curr_attr(victim, ATTR_WIL) );

	pager_printf( ch, "Agi: &C%3d&c  Ref: &C%3d&c  Spe: &C%3d&c  Spi: &C%3d&c  Mig: &C%3d&c  Abs: &C%3d&c\r\n",
		get_curr_attr(victim, ATTR_AGI),
		get_curr_attr(victim, ATTR_REF),
		get_curr_attr(victim, ATTR_SPE),

		get_curr_attr(victim, ATTR_SPI),
		get_curr_attr(victim, ATTR_MIG),
		get_curr_attr(victim, ATTR_ABS) );

	pager_printf( ch, "Emp: &C%3d&c  Bea: &C%3d&c  Lea: &C%3d&c\r\n",
		get_curr_attr(victim, ATTR_EMP),
		get_curr_attr(victim, ATTR_BEA),
		get_curr_attr(victim, ATTR_LEA) );

	pager_printf( ch, "&cLevel   : &P%-2d              ", victim->level );
	pager_printf( ch, "&cRazza  &w: %s&w   &cClasse  &w: %s&w\r\n", get_race(victim, TRUE), get_class(victim, TRUE) );

	sprintf( lfbuf, "%d/%d", victim->points[POINTS_LIFE], victim->points_max[POINTS_LIFE] );
	sprintf( mnbuf, "%d/%d", victim->points[POINTS_MANA], victim->points_max[POINTS_MANA] );
	sprintf( mvbuf, "%d/%d", victim->points[POINTS_MOVE], victim->points_max[POINTS_MOVE] );
	sprintf( slbuf, "%d/%d", victim->points[POINTS_SOUL], victim->points_max[POINTS_SOUL] );
	pager_printf( ch, "&cLife  : &w%-12s    &cMana  : &w%-12s    &cMove  : &w%-12s    &cSoul  : &w%-12s \r\n",
		lfbuf, mnbuf, mvbuf, slbuf );

	pager_printf( ch,  "&cHitroll : &C%-5d           &cAlign  : &w%-5d           &cArmorClass: &w%d\r\n",
		get_hitroll(victim), victim->alignment, get_ac(victim) );
	pager_printf( ch, "&cDamroll : &C%-5d           &cWimpy  : &w%-3d%%          &cPosition  : &w%s\r\n",
		get_damroll(victim), victim->wimpy, code_str(NULL, victim->position, CODE_POSITION) );
	pager_printf( ch, "&cFighting: &w%-13s   &cMaster : &w%-13s   &cLeader    : &w%s\r\n",
		victim->fighting  ?  victim->fighting->who->name  :  "(none)",
		victim->master	  ?  victim->master->name		  :  "(none)",
		victim->leader	  ?  victim->leader->name		  :  "(none)" );

	if ( IS_MOB(victim) )
	{
		pager_printf( ch, "&cHating  : &w%-13s   &cHunting: &w%-13s   &cFearing   : &w%s\r\n",
		victim->hating   ?  victim->hating->name   :  "(none)",
		victim->hunting  ?  victim->hunting->name  :  "(none)",
		victim->fearing  ?  victim->fearing->name  :  "(none)" );
	}
	else
	{
		pager_printf( ch, "&cGlory     : &w%-d (%d)\r\n",
			victim->pg->glory,
			victim->pg->glory_accum );
	}

	if ( IS_MOB(victim) )
	{
		pager_printf( ch, "&cMob hitdie : &C%dd%d+%d    &cMob damdie : &C%dd%d+%3d    &cIndex damdie : &C%dd%d+%3d\r\n&c&cNumAttacks : &C%d\r\n",
			victim->pIndexData->hitnodice,
			victim->pIndexData->hitsizedice,
			victim->pIndexData->hitplus,
			victim->barenumdie,
			victim->baresizedie,
			victim->damplus,
			victim->pIndexData->damnodice,
			victim->pIndexData->damsizedice,
			victim->pIndexData->damplus,
			victim->numattacks );
	}
	pager_printf( ch, "&cMentalState: &w%-3d   &cEmotionalState: &w%-3d   ",
		victim->mental_state, victim->emotional_state );

	if ( IS_PG( victim ) )
	{
		pager_printf( ch, "&cThirst: &w%d   &cFull: &w%d   &cDrunk: &w%d\r\n",
			victim->pg->condition[CONDITION_THIRST],
			victim->pg->condition[CONDITION_FULL],
			victim->pg->condition[CONDITION_DRUNK] );
	}
	else
	{
		send_to_pager( ch, "\r\n" );
	}

	buf[0] = '\0';
	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
	{
		sprintf( temp, "%d ", victim->saving_throw[x]);
		strcat( buf, temp );
	}
	pager_printf( ch, "&cSave versus: &w%s", buf);

	pager_printf( ch, "&cItems: &w(%d/%d)   &cWeight &w(%d/%d)\r\n",
		victim->carry_number,
		can_carry_number(victim),
		victim->carry_weight,
		can_carry_weight(victim) );

	pager_printf( ch, "&cYear: &w%-5d  &cSecs: &w%d  &cTimer: &w%d  &cGold: &Y%d\r\n",
		get_age(victim, TIME_YEAR),
		(IS_MOB(victim))  ?  -1  :  (int) victim->pg->played,
		victim->timer,
		victim->gold );

	if ( get_timer(victim, TIMER_PKILLED) )
		pager_printf( ch, "&cTimerPkilled:  &R%d\r\n", get_timer(victim, TIMER_PKILLED) );
	if ( get_timer(victim, TIMER_KILLED) )
		pager_printf( ch, "&cTimerKilled:  &R%d\r\n", get_timer(victim, TIMER_KILLED) );
	if ( get_timer(victim, TIMER_RECENTFIGHT) )
		pager_printf( ch, "&cTimerRecentfight:  &R%d\r\n", get_timer(victim, TIMER_RECENTFIGHT) );
	if ( get_timer(victim, TIMER_ASUPRESSED) )
		pager_printf( ch, "&cTimerAsupressed:  &R%d\r\n", get_timer(victim, TIMER_ASUPRESSED) );

	if ( IS_MOB(victim) )
		pager_printf( ch, "&cAct Flags  : &w%s\r\n", code_bit(NULL, victim->act, CODE_MOB) );
	else
		pager_printf( ch, "&cPlayerFlags: &w%s\r\n", code_bit(NULL, victim->pg->flags, CODE_PLAYER) );

	if ( victim->morph )
	{
		if ( victim->morph->morph )
		{
			pager_printf( ch, "&cMorphed as : (&C%d&c) &C%s    &cTimer: &C%d\r\n",
				victim->morph->morph->vnum,
				victim->morph->morph->short_desc,
				victim->morph->timer );
		}
		else
			send_to_pager( ch, "&cMorphed as: Morph was deleted.\r\n" );
	}

	pager_printf( ch, "&cAffected by: &C%s\r\n",
		code_bit(NULL, victim->affected_by, CODE_AFFECT) );
	pager_printf( ch, "&cSpeaks: &w%s   &cSpeaking: &w%d   &cExperience: &w%d",
		code_bit(NULL, victim->speaks, CODE_LANGUAGE),
		victim->speaking,
		victim->exp );

	if ( IS_PG(victim) && victim->wait )
		pager_printf( ch, "   &cWaitState: &R%d\r\n", victim->wait/12 );
	else
		send_to_pager( ch, "\r\n" );

	if ( VALID_STR(victim->short_descr) )
		pager_printf( ch, "&cShortdesc  : &w%s\r\n", victim->short_descr );
	if ( VALID_STR(victim->long_descr) )
		pager_printf( ch, "&cLongdesc   : &w%s\r\n", victim->long_descr );
	if ( VALID_STR(victim->description) )
		pager_printf( ch, "&cDescription: &w%s\r\n", victim->description );

	if ( IS_MOB(victim) && victim->spec_fun )
		pager_printf( ch, "&cMobile has spec fun: &w%s\r\n", victim->spec_funname );

	if ( IS_MOB(victim) )
		pager_printf( ch, "&cBody Parts : &w%s\r\n", code_bit(NULL, victim->xflags, CODE_PART) );

	if ( !IS_EMPTY(victim->resistant) )
		pager_printf( ch, "&cResistant  : &w%s\r\n", code_bit(NULL, victim->resistant, CODE_RIS) );

	if ( !IS_EMPTY(victim->immune) )
		pager_printf( ch, "&cImmune     : &w%s\r\n", code_bit(NULL, victim->immune, CODE_RIS) );

	if ( !IS_EMPTY(victim->susceptible) )
		pager_printf( ch, "&cSusceptible: &w%s\r\n", code_bit(NULL, victim->susceptible, CODE_RIS) );

	if ( IS_MOB(victim) )
	{
		pager_printf( ch, "&cAttacks    : &w%s\r\n",
			code_bit(NULL, victim->attacks, CODE_ATTACK) );
		pager_printf( ch, "&cDefenses   : &w%s\r\n",
			code_bit(NULL, victim->defenses, CODE_DEFENSE) );
	}

#ifdef T2_ALFA
	/* Elenca le extra */
	if ( mob->first_extradescr )
	{
		EXTRA_DESCR_DATA *ed;

		send_to_pager( ch, "Secondary description keywords: '" );
		for ( ed = mob->first_extradescr;  ed;  ed = ed->next )
			pager_printf( ch, "%s ", ed->keyword );

		send_to_pager( ch, "'.\r\n" );
	}
#endif

	for ( paf = victim->first_affect;  paf;  paf = paf->next )
	{
		skill = get_skilltype( paf->type );
		if ( skill )
		{
			pager_printf( ch, "&c%s: &w'%s' mods %s by %d for %d rnds with bits %s.",
				code_name(NULL, skill->type, CODE_SKILLTYPE),
				skill->name,
				code_name(NULL, paf->location, CODE_APPLY),
				paf->modifier,
				paf->duration,
				code_bit(NULL, paf->bitvector, CODE_AFFECT) );
		}
		send_to_pager( ch, "\r\n" );
	}
}


DO_RET do_mfind( CHAR_DATA *ch, char *argument )
{
	MOB_PROTO_DATA	*pMobIndex;
	int				 nMatch;
	bool			 fAll;

	set_char_color( AT_PLAIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  mfind <nomemob per intero>\r\n" );
		send_to_char( ch, "           mfind tutti\r\n" );

		return;
	}

	fAll	= ( !str_cmp_all(argument, FALSE) );
	nMatch	= 0;

	/*
	 * This goes through all the hash entry points (1024), and is therefore
	 * much faster, though you won't get your vnums in order... oh well. :)
	 *
	 * Tests show that Furey's method will usually loop 32,000 times, calling
	 * get_mob_index()... which loops itself, an average of 1-2 times...
	 * So theoretically, the above routine may loop well over 40,000 times,
	 * and my routine bellow will loop for as many index_mobiles are on
	 * your mud... likely under 3000 times.
	 */
	for ( pMobIndex = first_mob_proto;  pMobIndex;  pMobIndex = pMobIndex->next )
	{
		if ( fAll || nifty_is_name(argument, pMobIndex->name) )
		{
			nMatch++;
			pager_printf( ch, "[%5d] %s\r\n", pMobIndex->vnum, capitalize(pMobIndex->short_descr) );
		}
	}

	if ( nMatch )
		pager_printf( ch, "Number of matches: %d\n", nMatch );
	else
		send_to_char( ch, "Nothing like that in hell, earth, or heaven.\r\n" );
}


DO_RET do_ofind( CHAR_DATA *ch, char *argument )
{
	OBJ_PROTO_DATA	*pObj;
	int				 nMatch = 0;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Ofind what?\r\n" );
		return;
	}

	for ( pObj = first_object_proto;  pObj;  pObj = pObj->next )
	{
		if ( nifty_is_name_prefix(argument, pObj->name) )
		{
			nMatch++;
			pager_printf( ch, "[%5d] %s\r\n", pObj->vnum, capitalize(pObj->short_descr) );
		}
	}

	if ( nMatch )
		pager_printf( ch, "Numero di oggetti trovati: %d\n", nMatch );
	else
		pager_printf( ch, "Nessun oggetto trovato cercando %s.\r\n", argument );
}


DO_RET do_wizhelp( CHAR_DATA *ch, char argument )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_wizhelp: ch è NULL" );
		return;
	}

	send_command( ch, "commands fiducia", CO );
}


/*
 * bool pg serve a gestire sia il comando pgwhere che quello mwhere
 */
static DO_RET where_handler( CHAR_DATA *ch, char *argument, bool pg )
{
	CHAR_DATA  *victim;
	char		arg[MIL];	/* ricerca su area o mud? */
	bool		area = TRUE;
	bool		found;

	argument = one_argument( argument, arg );
	if ( !str_cmp(arg, "area") )
	{
		area = TRUE;
	}
	else if ( !str_cmp(arg, "mud") )
	{
		area = FALSE;
		/* Evita una ricerca globale sui mob, troppo lunga sarebbe la lista */
		if ( !pg && !VALID_STR(argument) )
		{
			send_to_char( ch, "Se vuoi fare una ricerca mwhere sul mud devi passare il prefisso o il nome di un mob da cercare.\r\n" );
			return;
		}
	}
	else if ( VALID_STR(arg) )
	{
		send_to_char( ch, "Tipo di ricerca in area o su tutto il mud?\r\n" );
		return;
	}

	if ( VALID_STR(argument) )
	{
		/* Controlla che almeno un pg o mob esista con l'argomento passato */
		victim = get_char_mud( ch, argument, TRUE );
		if ( !victim || (pg && IS_MOB(victim)) )	/* Non si può fare lo stesso test con !pg visto che anche se trovasse victim come player non è detto che non vi siano mob con lo stesso argument per come è strutturata la get_char_mud() */
		{
			ch_printf( ch, "Questo %s non c'è.\r\n", (pg) ? "giocatore" : "mob" );
			return;
		}
	}

	set_char_color( AT_PLAIN, ch );

	found = FALSE;
	for ( victim = first_char;  victim;  victim = victim->next )
	{
		if ( !pg && IS_PG (victim) )			continue;
		if (  pg && IS_MOB(victim) )			continue;
		if (  pg && !victim->desc )				continue;
		if (  pg && !is_in_game(victim->desc) )	continue;
		if ( !victim->in_room )					continue;
		if ( area && !victim->in_room->area )	continue;

		if ( area && victim->in_room->area != ch->in_room->area )
			continue;

		if ( !VALID_STR(argument) || nifty_is_name(argument, victim->name) )
		{
			found = TRUE;
			if ( pg )
			{
				pager_printf( ch, "%-16s  [%5d] %s\r\n",
					victim->name,
					victim->in_room->vnum,
					victim->in_room->name );
			}
			else
			{
				pager_printf( ch, "[%5d] %-28s [%5d] %s\r\n",
					victim->pIndexData->vnum,
					victim->short_descr,
					victim->in_room->vnum,
					victim->in_room->name );
			}
		}
	}

	if ( !found )
		ch_printf( ch, "You didn't find any%s%s.", VALID_STR(argument) ? " " : "", argument );
	else
		send_to_pager( ch, "\r\n" );
}


/*
 * Serve per cercare i pg o mob nell'area
 */
DO_RET do_where( CHAR_DATA *ch, char *argument )
{
	if ( !VALID_STR(argument) )
	{
		send_to_char( ch , "&YSintassi&w:  where area\r\n" );
		send_to_char( ch , "           where area <nomepg>\r\n" );
		send_to_char( ch , "           where mud\r\n" );
		send_to_char( ch , "           where mud <nomepg>\r\n\r\n" );

		send_to_char( ch , "Ricerca di default su area:\r\n\r\n" );
		where_handler( ch, "area", TRUE );

		return;
	}

	where_handler( ch, argument, TRUE );
}


DO_RET do_mwhere( CHAR_DATA *ch, char *argument )
{
	if ( !VALID_STR(argument) )
	{
		send_to_char( ch , "&YSintassi&w:  mwhere area\r\n" );
		send_to_char( ch , "           mwhere area <nomemob>\r\n" );
		send_to_char( ch , "           mwhere mud <nomemob>\r\n\r\n" );

		send_to_char( ch , "Ricerca di default su area:\r\n\r\n" );
		where_handler( ch, "area", FALSE );

		return;
	}

	where_handler( ch, argument, FALSE );
}


/*
 * Comando admin di global where
 */
DO_RET do_gwhere( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	char		 arg1[MIL];
	char		 arg2[MIL];
	bool		 found = FALSE;
	bool		 pmobs = FALSE;
	int			 low = 1;
	int			 high = LVL_LEGEND;
	int			 count = 0;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !is_number(arg1) || !is_number(arg2) )
	{
		send_to_pager( ch, "\r\n&w&YSintassi&w:  gwhere <low> <high>" );
		send_to_pager( ch, "                 gwhere <low> <high> mob\r\n" );

		return;
	}

	low = atoi( arg1 );
	high = atoi( arg2 );

	if ( low < 1 || high > LVL_LEGEND || high < low || low > high )
	{
		send_to_pager( ch, "&wInvalid level range.\r\n" );
		return;
	}

	if ( is_name(argument, "mob mobs mobile mobiles") )
		pmobs = TRUE;

	for ( victim = first_char;  victim;  victim = victim->next )
	{
		if ( !pmobs && IS_MOB(victim) )		continue;
		if (  pmobs && IS_PG (victim) )		continue;
		if ( !victim->in_room )				continue;
		if ( victim->level < low )			continue;	/* niente get_level per elencare anche i pg di livelli alti */
		if ( victim->level > high )			continue;

		if ( IS_PG(victim) && !is_in_game(victim->desc) )
			continue;

		found = TRUE;
		pager_printf( ch, "&c(&C%2d&c) &w%-12.12s   [%-5d - %-19.19s]   &c%-25.25s\r\n",
			victim->level,
			victim->name,
			victim->in_room->vnum,
			victim->in_room->area->name,
			victim->in_room->name );
		count++;
	}

	pager_printf( ch, "&c%d %s found.\r\n", count, (pmobs)  ?  "mob"  :  "giocatori" );
}


/*
 * Comando admin di global fightning
 */
DO_RET do_gfighting( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		*victim;
	char			 arg1[MIL];
	char			 arg2[MIL];
	int				 low	  = 1;
	int				 high	  = LVL_LEGEND;
	int				 count	  = 0;
	bool			 found	  = FALSE;
	bool			 pmobs	  = FALSE;
	bool			 phating  = FALSE;
	bool			 phunting = FALSE;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !is_number(arg1) || !is_number(arg2) )
	{
		send_to_char( ch, "\r\n&w&YSintassi&w:  gfighting <low> <high>" );
		send_to_char( ch, "                 gfighting <low> <high> mobs\r\n" );
		send_to_char( ch, "                 gfighting <low> <high> hating\r\n" );
		send_to_char( ch, "                 gfighting <low> <high> hunting\r\n" );

		return;
	}

	low = atoi( arg1 );
	high = atoi( arg2 );

	if ( low < 1 || high > LVL_LEGEND || high < low || low > high )
	{
		send_to_pager( ch, "&wInvalid level range.\r\n" );
		return;
	}

	if		( is_name(argument, "mob mobs mobile mobiles") )	pmobs	 = TRUE;
	else if ( is_name(argument, "odiando hating"		 ) )	phating	 = TRUE;
	else if ( is_name(argument, "cacciando hunting"		 ) )	phunting = TRUE;

	pager_printf( ch, "\r\n&cGlobal %s conflict:\r\n", pmobs ? "mob" : "giocatori" );
	for ( victim = first_char;  victim;  victim = victim->next )
	{
		if ( IS_MOB(victim) && !pmobs && !phating && !phunting  )	continue;
		if ( IS_PG (victim) && (pmobs ||  phating ||  phunting) )	continue;
		if ( !victim->in_room )										continue;
		if ( !phating && !phunting && !victim->fighting )			continue;
		if ( !phunting && phating && !victim->hating )				continue;
		if ( phunting && victim->hunting )							continue;
		if ( victim->level < low )									continue;	/* niente get_level per elencare anche oggetti di livello molto alto */
		if ( victim->level > high )									continue;
		if ( IS_PG(victim) && !is_in_game(victim->desc) )			continue;

		found = TRUE;

		pager_printf( ch, "&w%-12.12s &C|%2d &wvs &C%2d|",
			victim->name,
			victim->level,
			victim->fighting->who->level );

		if ( !phating && !phunting )
		{
			pager_printf( ch, " &w%-16.16s [%5d]",
				IS_MOB(victim->fighting->who)  ?  victim->fighting->who->short_descr  :  victim->fighting->who->name,
				IS_MOB(victim->fighting->who)  ?  victim->fighting->who->pIndexData->vnum  :  0 );
		}
		else if ( !pmobs && !phunting && phating )
		{
			pager_printf( ch, " &w%-16.16s [%5d]",
				IS_MOB( victim->hating->who )  ?  victim->hating->who->short_descr  :  victim->hating->who->name,
				IS_MOB( victim->hating->who )  ?  victim->hating->who->pIndexData->vnum  :  0 );
		}
		else if ( !pmobs && !phating && phunting )
		{
			pager_printf( ch, " &w%-16.16s [%5d]",
				IS_MOB(victim->hunting->who)  ?  victim->hunting->who->short_descr  :  victim->hunting->who->name,
				IS_MOB(victim->hunting->who)  ?  victim->hunting->who->pIndexData->vnum  :  0 );
		}

		pager_printf( ch, "  &c%-20.20s [%5d]\r\n",
			victim->in_room->area->name,
			(victim->in_room == NULL)  ?  0  :  victim->in_room->vnum );

		count++;
	}

	pager_printf( ch, "&c%d %s conflicts located.\r\n", count, pmobs  ?  "mob"  :  "character" );
}


DO_RET do_snoop( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA		*victim;

	set_char_color( AT_ADMIN, ch );

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_snoop: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Snoop whom?\r\n" );
		return;
	}

	victim = get_player_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	if ( !victim->desc )
	{
		send_to_char( ch, "No descriptor to snoop.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Cancelling all snoops.\r\n" );
		for ( d = first_descriptor; d; d = d->next )
		{
			if ( d->snoop_by == ch->desc )
				d->snoop_by = NULL;
		}
		return;
	}

	if ( victim->desc->snoop_by )
	{
		send_to_char( ch, "Busy already.\r\n" );
		return;
	}

	/* Minimum snoop level... a secret mset value makes the snooper
	 *	think that the victim is already being snooped. */
	if ( get_trust(victim) >= get_trust(ch)
	  || (victim->pg && victim->pg->min_snoop > get_trust(ch)) )
	{
		send_to_char( ch, "Busy already.\r\n" );
		return;
	}

	if ( ch->desc )
	{
		for ( d = ch->desc->snoop_by;  d;  d = d->snoop_by )
		{
			if ( d->character == victim || d->original == victim )
			{
				send_to_char( ch, "No snoop loops.\r\n" );
				return;
			}
		}
	}

	victim->desc->snoop_by = ch->desc;
	send_to_char( ch, "Ok.\r\n" );
	send_log( NULL, LOG_SNOOP, "%s comincia a snoopare %s", ch->name, victim->name );
}


DO_RET do_switch( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Switch into whom?\r\n" );
		return;
	}

	if ( !ch->desc )
		return;

	if ( ch->desc->original )
	{
		send_to_char( ch, "You are already switched.\r\n" );
		return;
	}

	victim = get_char_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "They aren't here.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Ok.\r\n" );
		return;
	}

	if ( victim->desc )
	{
		send_to_char( ch, "Character in use.\r\n" );
		return;
	}

	if ( IS_PG(victim) )
	{
		send_to_char( ch, "You cannot switch into a player!\r\n" );
		return;
	}

	if ( victim->switched )
	{
		send_to_char( ch, "You can't switch into a player that is switched!\r\n" );
		return;
	}

	ch->desc->character	= victim;
	ch->desc->original	= ch;
	victim->desc		= ch->desc;
	ch->desc			= NULL;
	ch->switched		= victim;

	send_to_char( victim, "Ok.\r\n" );
}


/*
 * Ritorna da uno switch, ha trust per player
 */
DO_RET do_return( CHAR_DATA *ch, char *argument )
{
	if ( IS_PG(ch) && !IS_ADMIN(ch) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	if ( !ch->desc )
		return;

	set_char_color( AT_ADMIN, ch );

	if ( !ch->desc->original )
	{
		send_to_char( ch, "You aren't switched.\r\n" );
		return;
	}

	send_to_char( ch, "You return to your original body.\r\n" );

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_POSSESS) )
	{
		affect_strip( ch, gsn_possess );
		REMOVE_BIT( ch->affected_by, AFFECT_POSSESS );
	}

	ch->desc->character		 	  = ch->desc->original;
	ch->desc->original		 	  = NULL;
	ch->desc->character->desc	  = ch->desc;
	ch->desc->character->switched = NULL;
	ch->desc					  = NULL;
}


/*
 * Quando ci sarà un sistema dinamico (CC) inserire anche il campo level come oinvoke
 *	per creare un mob come si vuole.
 */
DO_RET do_minvoke( CHAR_DATA *ch, char *argument )
{
	MOB_PROTO_DATA		  *pMobIndex;
	CHAR_DATA			  *victim;
	VNUM				   vnum;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  minvoke <vnum>\r\n" );
		return;
	}

	if ( !is_number(argument) )
	{
		char arg[MIL];
		int	 cnt = 0;
		int	 count = number_argument( argument, arg );

		vnum = -1;
		for ( pMobIndex = first_mob_proto;  pMobIndex;  pMobIndex = pMobIndex->next )
		{
			if ( nifty_is_name(arg, pMobIndex->name) && ++cnt == count )
			{
				vnum = pMobIndex->vnum;
				break;
			}
		}

		if ( vnum == -1 )
		{
			send_to_char( ch, "No such mobile exists.\r\n" );
			return;
		}
	}
	else
		vnum = atoi( argument );

	if ( !IS_ADMIN(ch) )
	{
		AREA_DATA *pArea;

		if ( IS_MOB(ch) )
		{
			send_to_char( ch, NOFOUND_MSG );
			return;
		}

		if ( !ch->pg || !(pArea = ch->pg->build_area) )
		{
			send_to_char( ch, "You must have an assigned area to invoke this mobile.\r\n" );
			return;
		}

		if ( vnum < pArea->vnum_low && vnum > pArea->vnum_high )
		{
			send_to_char( ch, "That number is not in your allocated range.\r\n" );
			return;
		}
	}

	if ( (pMobIndex = get_mob_index(NULL, vnum)) == NULL )
	{
		send_to_char( ch, "No mobile has that vnum.\r\n" );
		return;
	}

	victim = make_mobile( pMobIndex );
	char_to_room( victim, ch->in_room );
	act( AT_ADMIN, "$n invokes $N!", ch, NULL, victim, TO_ROOM );

	/* How about seeing what we're invoking for a change */
	ch_printf( ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d&Y)\r\n",
		pMobIndex->short_descr,
		pMobIndex->vnum,
		pMobIndex->name,
		victim->level );
}


DO_RET do_oinvoke( CHAR_DATA *ch, char *argument )
{
	OBJ_PROTO_DATA	*pObj;
	OBJ_DATA	*obj;
	char		 arg[MIL];
	VNUM		 vnum;
	int			 level;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  oinvoke <vnum> <level>\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(argument) )
	{
		level = get_level( ch );
	}
	else
	{
		if ( !is_number(argument) )
		{
			send_command( ch, "oinvoke", CO );
			return;
		}

		level = atoi( argument );
		if ( level < 0 || level > LVL_LEGEND )
		{
			ch_printf( ch, "Livello minimo 0, massimo %d.\r\n", LVL_LEGEND );
			return;
		}
	}

	if ( !is_number(arg) )
	{
		char arg2[MIL];
		int	 cnt = 0;
		int  count = number_argument( arg, arg2 );

		vnum = -1;
		for ( pObj = first_object_proto;  pObj;  pObj = pObj->next )
		{
			if ( nifty_is_name(arg2, pObj->name) && ++cnt == count )
			{
				vnum = pObj->vnum;
				break;
			}
		}

		if ( vnum == -1 )
		{
			send_to_char( ch, "No such object exists.\r\n" );
			return;
		}
	}
	else
	{
		vnum = atoi( arg );
	}

	if ( !IS_ADMIN(ch) )
	{
		AREA_DATA *pArea;

		if ( IS_MOB(ch) )
		{
			send_to_char( ch, "Huh?\n\r" );
			return;
		}

		if ( !ch->pg || !(pArea = ch->pg->build_area) )
		{
			send_to_char( ch, "You must have an assigned area to invoke this object.\n\r" );
			return;
		}

		if ( vnum < pArea->vnum_low && vnum > pArea->vnum_high )
		{
			send_to_char( ch, "That number is not in your allocated range.\n\r" );
			return;
		}
	}

	pObj = get_obj_proto( NULL, vnum );
	if ( !pObj )
	{
		send_to_char( ch, "No object has that vnum.\r\n" );
		return;
	}

	if ( level == 0 )
	{
		AREA_DATA *temp_area;

		temp_area = get_area_obj( pObj );
		if ( !temp_area )
		{
			level = get_level( ch );
		}
		else
		{
			level = generate_itemlevel( temp_area, pObj );
			level = URANGE( 0, level, LVL_LEGEND );
		}
	}

	obj = make_object( pObj, level );
	if ( HAS_BIT( obj->wear_flags, OBJWEAR_TAKE) )
	{
		obj = obj_to_char( obj, ch );
	}
	else
	{
		obj = obj_to_room( obj, ch->in_room );
		act( AT_ADMIN, "$n fashions $p from ether!", ch, obj, NULL, TO_ROOM );
	}

	/* I invoked what? */
	ch_printf( ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d&Y)\r\n",
		obj->short_descr, obj->vnum, obj->name, obj->level );
}


DO_RET do_purge( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *obj;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		/* 'purge' */
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for ( victim = ch->in_room->first_person;  victim;  victim = vnext )
		{
			vnext = victim->next_in_room;

			if ( IS_MOB(victim) && victim != ch )
				extract_char( victim, TRUE );
		}

		for ( obj = ch->in_room->first_content;  obj;  obj = obj_next )
		{
			obj_next = obj->next_content;
			free_object( obj );
		}

		send_to_char( ch, "Ripulisci la stanza." );
		return;
	}

	victim = NULL;
	obj = NULL;
	/* Fixed to get things in room first -- i.e., purge portal (obj),
	 * no more purging mobs with that keyword in another room first. */
	victim = get_char_room( ch, argument, TRUE );
	obj    = get_obj_here ( ch, argument );
	if ( !victim && !obj )
	{
		send_to_char( ch, "They aren't here.\r\n" );
		return;
	}

	/* Single object purge in room for high level purge */
	if ( obj )
	{
		split_obj( obj, 1 );
		ch_printf( ch, "Ripulisci %s.", obj->short_descr );
		free_object( obj );
		return;
	}

	if ( IS_PG(victim) )
	{
		send_to_char( ch, "Not on PC's.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "You cannot purge yourself!\r\n" );
		return;
	}

	ch_printf( ch, "Ripulisci %s.", victim->name );
	extract_char( victim, TRUE );
}


/*
 * This could have other applications too.. move if needed.
 */
void close_area( AREA_DATA *pArea )
{
	CHAR_DATA		 *ech;
	CHAR_DATA		 *ech_next;
	OBJ_DATA		 *eobj;
	OBJ_DATA		 *eobj_next;
	ROOM_DATA		 *rd;
	ROOM_DATA		 *rd_next;
	OBJ_PROTO_DATA	 *opd;
	OBJ_PROTO_DATA	 *opd_next;
	MOB_PROTO_DATA	 *mpd;
	MOB_PROTO_DATA	 *mpd_next;
	RESET_DATA		 *ereset;
	EXTRA_DESCR_DATA *edd;
	EXIT_DATA		 *xit;
	EXIT_DATA		 *xit_next;
	MPROG_ACT_LIST	 *mpact;
	AFFECT_DATA		 *paf;
	int				  icnt;

	for ( ech = first_char;  ech;  ech = ech_next )
	{
		ech_next = ech->next;

		if ( ech->fighting )
			stop_fighting( ech, TRUE );

		if ( IS_MOB(ech) )
		{
			/* if mob is in area, or part of area. */
			if ( URANGE(pArea->vnum_low, ech->pIndexData->vnum, pArea->vnum_high) == ech->pIndexData->vnum
			  || (ech->in_room && ech->in_room->area == pArea) )
			{
				extract_char( ech, TRUE );
			}
			continue;
		}
		if ( ech->in_room && ech->in_room->area == pArea )
			spell_word_of_recall( -1, -1, ech, NULL );
	}

	for ( eobj = first_object;  eobj;  eobj = eobj_next )
	{
		eobj_next = eobj->next;
		/* if obj is in area, or part of area. */
		if ( URANGE(pArea->vnum_low, eobj->pObjProto->vnum, pArea->vnum_high) == eobj->pObjProto->vnum
		  || (eobj->in_room && eobj->in_room->area == pArea) )
		{
			free_object( eobj );
		}
	}

	for ( icnt = 0;  icnt < MAX_KEY_HASH;  icnt++ )
	{
		for ( rd = room_index_hash[icnt];  rd;  rd = rd_next )
		{
			rd_next = rd->next;

			for ( xit = rd->first_exit;  xit;  xit = xit_next )
			{
				xit_next = xit->next;

				if ( rd->area == pArea || xit->to_room->area == pArea )
				{
					UNLINK( xit, rd->first_exit, rd->last_exit, next, prev );

					DISPOSE( xit->keyword );
					DISPOSE( xit->description );
					DISPOSE( xit );

					/* Crash bug fix.  I know it could go from the start several times
					 * But you CAN NOT iterate over a link-list and DELETE from it or
					 * Nasty things can and will happen.
					 */
					xit = rd->first_exit;
				}
			}

			if ( rd->area != pArea )
				continue;

			DISPOSE( rd->name );
			DISPOSE( rd->description );

			if ( rd->first_person )
			{
				send_log( NULL, LOG_BUG, "close_area: room with people #%d", rd->vnum );
				for ( ech = rd->first_person;  ech;  ech = ech_next )
				{
					ech_next = ech->next_in_room;

					if ( ech->fighting )
						stop_fighting( ech, TRUE );

					if ( IS_MOB(ech) )
						extract_char( ech, TRUE );
					else
						send_command( ech, "recall", CO );	/* (bb) bisogna mettere word_of_recall qui altrimenti non riporta indietro se il pg è di livello alto */
				}
			}

			if ( rd->first_content )
			{
				send_log( NULL, LOG_BUG, "close_area: room with people #%d", rd->vnum );

				for ( eobj = rd->first_content;  eobj;  eobj = rd->first_content )
					free_object( eobj );
			}

			for ( edd = rd->first_extradescr;  edd;  edd = rd->first_extradescr )
			{
				DISPOSE( edd->keyword );
				DISPOSE( edd->description );
				DISPOSE( edd );
			}

			for ( mpact = rd->mpact;  mpact;  mpact = rd->mpact )
			{
				DISPOSE( mpact->buf );
				DISPOSE( mpact );
			}

			free_mudprog( rd->first_mudprog );

			if ( rd == room_index_hash[icnt] )
			{
				room_index_hash[icnt] = rd->next;
			}
			else
			{
				ROOM_DATA *trd;

				for ( trd = room_index_hash[icnt];  trd;  trd = trd->next )
				{
					if ( trd->next == rd )
						break;
				}

				if ( !trd )
					send_log( NULL, LOG_BUG, "close_area: rd not in hash list %d", rd->vnum );
				else
					trd->next = rd->next;
			}
			DISPOSE( rd );
		} /* chiude il for */

		for ( mpd = first_mob_proto;  mpd;  mpd = mpd_next )
		{
			mpd_next = mpd->next;

			if ( mpd->vnum < pArea->vnum_low || mpd->vnum > pArea->vnum_high )
				continue;

			DISPOSE( mpd->name );
			DISPOSE( mpd->short_descr );
			DISPOSE( mpd->long_descr  );
			DISPOSE( mpd->description );

			if ( mpd->pShop )
			{
				UNLINK( mpd->pShop, first_shop, last_shop, next, prev );
				DISPOSE( mpd->pShop );
			}

			if ( mpd->rShop )
			{
				UNLINK( mpd->rShop, first_repair, last_repair, next, prev );
				DISPOSE( mpd->rShop );
			}

			free_mudprog( mpd->first_mudprog );

			UNLINK( mpd, first_mob_proto, last_mob_proto, next, prev );
			DISPOSE( mpd );
		} /* chiude il for */

		for ( opd = first_object_proto;  opd;  opd = opd_next )
		{
			opd_next = opd->next;

			if ( opd->vnum < pArea->vnum_low || opd->vnum > pArea->vnum_high )
				continue;

			DISPOSE( opd->name );
			DISPOSE( opd->short_descr );
			DISPOSE( opd->long_descr );
			DISPOSE( opd->action_descr );

			for ( edd = opd->first_extradescr;  edd;  edd = opd->first_extradescr )
			{
				DISPOSE( edd->keyword );
				DISPOSE( edd->description );
				DISPOSE( edd );
			}

			for ( paf = opd->first_affect;  paf;  paf = opd->first_affect )
				DISPOSE( paf );

			free_mudprog( opd->first_mudprog );

			UNLINK( opd, first_object_proto, last_obj_proto, next, prev );
			DISPOSE( opd );
		} /* chiude il for */
	} /* chiude il for */

	for ( ereset = pArea->first_reset;  ereset;  ereset = pArea->first_reset )
		DISPOSE( ereset );

	DISPOSE( pArea->name );
	DISPOSE( pArea->filename );
	DISPOSE( pArea->author );

	UNLINK( pArea, first_area, last_area, next, prev );
	UNLINK( pArea, first_build, last_build, next, prev );
	top_area--;

	DISPOSE( pArea );
}


/* (RR) controllare i comandi originali immortalize, mortalize e advance,
 *	che potrebbero essere utili per una integrazione qui.
 * Comando trust, dona o abbassa il grado di fiducia.
 * Distrugge anche i dati da admin se diminuisce la fiducia.
 * Attualmente un admin può diminuire la fiducia ad un altro admin.
 */
DO_RET do_trust( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg[MIL];
	int		   trust;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_trust: ch è NULL" );
		return;
	}

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  trust <nomepg> <fiducia>\r\n" );
		send_to_char( ch, "Tipi di fiducia:\r\n" );
		send_to_char( ch, code_all(CODE_TRUST, FALSE) );
		return;
	}

	victim = get_player_mud( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	/* Non puoi diminuire la fiducia di chi ne ha superiore o uguale a te a meno che non sia tu stesso. */
	if ( get_trust(ch) <= get_trust(victim) && ch != victim )
	{
		send_to_char( ch, "Non puoi fare ciò.\n\r" );
		return;
	}

	trust = code_num( NULL, argument, CODE_TRUST );
	if ( trust < 0 || trust > MAX_TRUST )
	{
		send_command( ch, "trust", CO );
		return;
	}

	if ( trust == victim->pg->trust )
	{
		ch_printf( ch, "%s è già di trust %s\r\n", victim->name, code_name(NULL, trust, CODE_TRUST) );
		return;
	}

	/* Se la trust voluta è minore di quella attuale della vittima gliela diminuisce */
	if ( trust < victim->pg->trust )
	{
		set_char_color( AT_ADMIN, victim );

		if ( trust == TRUST_PLAYER && IS_ADMIN(victim) )
		{
			char		 file[MIL];
			AREA_DATA	*pArea;

			REMOVE_BIT( victim->pg->flags, PLAYER_ADMSIGHT );
			/* Fixed bug here, was removing the immortal data of the person who used trust */
			sprintf( file, "%s%s", ADMIN_DIR, capitalize(victim->name) );

			/* Added to notify of removal of Immortal data */
			if ( !remove(file) )
			{
				send_to_char( ch, "Dati di amministratore distrutti.\n\r" );
			}
			else if ( errno != ENOENT )
			{
				send_log( NULL, LOG_BUG, "do_trust: unknown error #%d - %s (admin data).\r\n",
					errno, strerror(errno) );
				send_to_char( ch, "Dati di amministratore non distrutti. Report to Coder\r\n" );
			}

			sprintf( file, "%s.are", capitalize(victim->name)  );
			for ( pArea = first_build;  pArea;  pArea = pArea->next )
			{
				char	areafile[MIL];

				if ( !str_cmp(pArea->filename, file) )
				{
					sprintf( areafile, "%s%s", BUILD_DIR, file );
					if ( HAS_BIT(pArea->flags, AREAFLAG_LOADED) )
						fold_area( pArea, areafile, FALSE );
					close_area( pArea );
					sprintf( file, "%s.bak", areafile );
					set_char_color(AT_RED, ch);		/* Log message changes colors */
					if ( rename(areafile, file) == 0 )
					{
						send_to_char( ch, "Player's area data destroyed.  Area saved as backup.\r\n" );
					}
					else if ( errno != ENOENT )
					{
						send_log( NULL, LOG_BUG, "do_trust: unknown error #%d - %s (admin data).\r\n",
							errno, strerror(errno) );
						send_to_char( ch, "Dati di amministratore non distrutti. Report to Coder\r\n" );
					}
					break;
				}
			}
		}

		/* Diminuisce la trust */
		victim->pg->trust = trust;

		/* Stuff added to make sure character's wizinvis level doesn't stay
		 * higher than actual level */
		if ( !IS_ADMIN(victim) )
			victim->pg->incognito = 0;

		return;
	}
	else
	{
		/* Altrimenti se trust voluta è maggiore di quella attuale della vittima gliela aumenta */
		ch_printf( ch, "Aumenti la fiducia di %s da %s a %s!\r\n",
			victim->name, code_name(NULL, victim->pg->trust, CODE_TRUST), code_name(NULL, trust, CODE_TRUST) );
		ch_printf( victim, "Ti viene aumentata la fiducia da %s a %s!\r\n",
			code_name(NULL, victim->pg->trust, CODE_TRUST), code_name(NULL, trust, CODE_TRUST) );

		victim->pg->trust = trust;
	}

	/* Imposta i msglog di default per la sua trust */
	send_command( victim, "msglog default", CO );

	save_player( victim );
}


/*
 * Comando admin che permette di ristorare il pg in vari stati e punti
 */
DO_RET do_restore( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg1[MIL];
	char	   arg2[MIL];
	char	   type[MIL];
	int		   value = 100;
	bool	   all = FALSE;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  restore <nomepg>\r\n" );
		send_to_char( ch, "           restore <nomepg> vita|movimento|mana|anima (num%)\r\n" );
		send_to_char( ch, "           restore <nomepg> ubriaco|fame|sete|setesangue|veleno (num%)\r\n" );
		return;
	}

	argument = one_argument( argument, arg1 );
	victim = get_char_mud( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non c'è.\r\n" );
		return;
	}

	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg2) )
	{
		all = TRUE;

		if ( !VALID_STR(argument) )
			value = 100;
		else if ( !is_number(argument) || (value = atoi(argument)) < 1 || value > 100 )
		{
			send_command( ch, "restore", CO );
			return;
		}
	}

	if ( all == TRUE || is_name(arg2, "vita life") )
	{
		victim->points[POINTS_LIFE] = victim->points_max[POINTS_LIFE] * value / 100;
		sprintf( type, " nella vitalità" );
	}
	if ( all == TRUE || is_name(arg2, "movimento move") )
	{
		victim->points[POINTS_MOVE] = victim->points_max[POINTS_MOVE] * value / 100;
		sprintf( type, " nel movimento" );
	}
	if ( all == TRUE || is_name(arg2, "mana") )
	{
		victim->points[POINTS_MANA] = victim->points_max[POINTS_MANA] * value / 100;
		sprintf( type, " nel mana" );
	}
	if ( all == TRUE || is_name(arg2, "anima soul") )
	{
		victim->points[POINTS_SOUL] = victim->points_max[POINTS_SOUL] * value / 100;
		sprintf( type, " nell'anima" );
	}
	if ( all == TRUE || is_name(arg2, "veleno poison") )
	{
		REMOVE_BIT( victim->affected_by, AFFECT_POISON );
		sprintf( type, " nel veleno" );
	}

	/* Ristora fame, sete, sete di sangue */
	if ( IS_PG(victim) )
	{
		if ( all == TRUE || is_name(arg2, "ubriaco sobre") )
		{
			victim->pg->condition[CONDITION_DRUNK] = 48 - ( 48 * value / 100 );
			sprintf( type, " nell'embreagadura" );
		}
		if ( all == TRUE || is_name(arg2, "fame full") )
		{
			victim->pg->condition[CONDITION_FULL] = 48 * value / 100;
			sprintf( type, " nella fame" );
		}
		if ( all == TRUE || is_name(arg2, "sete thirst") )
		{
			victim->pg->condition[CONDITION_THIRST] = 48 * value / 100;
			sprintf( type, " nella sete" );
		}
		if ( all == TRUE || is_name(arg2, "sangue setesangue blood bloodthirst") )
		{
			victim->pg->condition[CONDITION_BLOODTHIRST] = 48 * value / 100;
			sprintf( type, " nella sete di sangue" );
		}
	}

	/* Ristora stato mentale ed emozionale */
	if ( all )
	{
		victim->mental_state	= 0;
		victim->emotional_state	= 0;
	}

	update_position( victim );	/* Servirebbe in realtà solamente il restore della vita penso, cmq non fa male tenerlo anche qui volendo */

	if ( ch != victim )
	{
		ch_printf( victim, "Un flusso di energia ti attraversa e ti senti ristorat%c%s\r\n",
			 gramm_ao(victim), (all == TRUE) ? "." : type );
	}

	ch_printf( ch, "%s è stat%c ristorat%c.\r\n",
		victim->name, gramm_ao(victim), gramm_ao(victim) );
}


DO_RET do_log( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Loggare chi?\r\n" );
		return;
	}

	if ( !str_cmp_all(argument, FALSE) )
	{
		if ( fLogAll )
		{
			fLogAll = FALSE;
			send_to_char( ch, "Log ALL off.\r\n" );
		}
		else
		{
			fLogAll = TRUE;
			send_to_char( ch, "Log ALL on.\r\n" );
		}
		return;
	}

	victim = get_player_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	/*
	 * No level check, admin can log anyone.
	 */
	if ( HAS_BIT_PLR(victim, PLAYER_LOG) )
	{
		REMOVE_BIT(victim->pg->flags, PLAYER_LOG );
		ch_printf( ch, "LOG removed from %s.\r\n", victim->name );
	}
	else
	{
		SET_BIT(victim->pg->flags, PLAYER_LOG );
		ch_printf( ch, "LOG applied to %s.\r\n", victim->name );
	}

	save_player( ch );
}


/*
 * Staccato da newbieset perché richiamato in nanny.c
 */
void equip_newbieset( CHAR_DATA *victim )
{
	OBJ_DATA	*obj;

	obj = make_object(get_obj_proto(NULL, VNUM_OBJ_SCHOOL_VEST), 1 );
	obj_to_char( obj, victim );
	obj = make_object(get_obj_proto(NULL, VNUM_OBJ_SCHOOL_SHIELD), 1 );
	obj_to_char( obj, victim );
	obj = make_object(get_obj_proto(NULL, VNUM_OBJ_SCHOOL_BANNER), 1 );
	obj_to_char( obj, victim );

	if ( victim->class == CLASS_MAGE
	  || victim->class == CLASS_THIEF )
	{
		obj = make_object(get_obj_proto(NULL, VNUM_OBJ_SCHOOL_DAGGER), 1 );
		obj_to_char( obj, victim );
	}
	else if ( victim->class == CLASS_PRIEST || victim->class == CLASS_DRUID )
	{
		obj = make_object( get_obj_proto(NULL, VNUM_OBJ_SCHOOL_MACE), 1 );
		obj_to_char(obj, victim);
	}
	else if ( victim->class == CLASS_FIGHTER || victim->class == CLASS_WANDERING
		|| 	  victim->class == CLASS_PALADIN )
	{
		obj = make_object( get_obj_proto(NULL, VNUM_OBJ_SCHOOL_SWORD), 1 );
		obj_to_char( obj, victim );
	}

	obj = make_object( get_obj_proto(NULL, VNUM_OBJ_SCHOOL_GUIDE), 1 );
	obj_to_char( obj, victim );

	/* Added the burlap sack to the newbieset.
		The sack is part of sgate.are called Spectral Gate */
	obj = make_object( get_obj_proto(NULL, 719), 1 );
	obj_to_char( obj, victim );

	act( AT_ADMIN, "Piccoli Portali si aprono attorno a me, da essi ne sgorga del Flusso Astrale che plasma per me degli oggetti.", victim, NULL, NULL, TO_CHAR );
	act( AT_ADMIN, "Si aprono improvvisamente piccoli Portali da cui esce del Flusso Astrale che si amalgama nelle mani di $n plasmando vari oggetti.", victim, NULL, NULL, TO_ROOM );
}


/*
 * Riveste un pg niubbo con del'equip di default.
 * Occhio che ora come ora mette 6 oggett nell'inventario del pg, ma se si vuole
 *	aggiungerne bisogna aumentare il valore trasportabile della can_carry_number.
 */
DO_RET do_newbieset( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		*victim;

    set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSyntassi&w:  newbieset <nomepg>\r\n" );
		return;
	}

	victim = get_char_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\n\r" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_to_char( ch, "Non sui Mob.\r\n" );
		return;
	}

	if ( get_level(victim) < 1 || get_level(victim) > LVL_NEWBIE )
	{
		ch_printf( ch, "Non è un niubbo, il suo livello deve essere minore o uguale a %d.\r\n", LVL_NEWBIE );
		return;
	}

	equip_newbieset( victim );

	ch_printf( ch, "A %s sono stati dati gli oggetti da newbie.\r\n", victim->name );
}


DO_RET do_peace( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*rch;

	for ( rch = ch->in_room->first_person;  rch;  rch = rch->next_in_room )
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

	act( AT_ADMIN, "Spirali di energia astrale scendono dall'alto donandomi una tranquillità e pace interiore, mi pesano anche le gambe..",
		ch, NULL, NULL, TO_ROOM );
	send_to_char( ch, "Spirali di energia astrale scendono dall'alto donandomi tranquillità e pace interiore." );
}


/*
 * Comando admin che elenca i cadaveri dei pg nel mud e li può caricare
 */
DO_RET do_bodybag( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *owner;
	OBJ_DATA  *obj;
	char	   arg[MIL];
	bool	   found = FALSE;
	bool	   bag	 = FALSE;
	bool	   all	 = FALSE;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  bodybag <character>\r\n" );
		send_to_char( ch, "           bodybag <character> si\r\n\r\n" );

		all = TRUE;
	}
	else
	{
		argument = one_argument( argument, arg );

		if ( is_name(argument, "yes si") )
			bag = TRUE;

		pager_printf( ch, "\r\n&P%s remains of %s..\r\n",
			(bag) ? "Retrieving" : "Searching for",
			capitalize(arg) );
	}

	pager_printf( ch, "&P%s:\r\n", (bag) ? "Bagging&Y" : "Corpse&w" );
	for ( obj = first_object;  obj;  obj = obj->next )
	{
		char	name[MIL];

		if ( !obj->in_room )					continue;
		if ( obj->vnum != VNUM_OBJ_CORPSE_PC )	continue;

		one_argument( obj->name, name );

		if ( all || (!all && !str_cmp(name, arg)) )
		{
			found = TRUE;

			pager_printf( ch, "%-12.12s %s  &PIn:  &w%-22.22s  &P[&w%5d&P]   &PTimer:  %s%2d\r\n",
				capitalize(name),
				HAS_BIT(obj->extra_flags, OBJFLAG_CLANCORPSE)  ?  "&RPK"  :  "&R  ",
				obj->in_room->area->name,
				obj->in_room->vnum,
				(obj->timer < 1)  ?  "&w"  :  (obj->timer < 5)  ?  "&R"  :  (obj->timer < 10)  ?  "&Y"  :  "&w",
				obj->timer );

			if ( bag )
			{
				obj_from_room( obj );
				obj = obj_to_char( obj, ch );
				obj->timer = -1;
				save_player( ch );
			}
		}
	}

	if ( found == FALSE )
	{
		send_to_pager( ch, "&PNessun cadavere trovato.\r\n" );
		return;
	}
	send_to_pager( ch, "\r\n" );

	for ( owner = first_player;  owner;  owner = owner->next_player )
	{
		if ( !str_cmp(arg, owner->name) )
			break;
	}

	if ( !owner && VALID_STR(arg) )
	{
		pager_printf( ch, "&P%s is not currently online.\r\n", capitalize(arg) );
		return;
	}
}


/*
 * Comando admi che elenca le ultime uccisioni di un pg
 */
DO_RET do_khistory( CHAR_DATA *ch, char *argument )
{
	MOB_PROTO_DATA	*tmob;
	CHAR_DATA		*victim;
	int				 track;

	if ( IS_MOB(ch) || !IS_ADMIN(ch) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: khistory <player>\r\n" );
		return;
	}

	victim = get_player_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	set_char_color( AT_DRED, ch );
	ch_printf( ch, "Kill history for %s:\r\n", victim->name );

	for ( track = 0;  track < MAX_KILLTRACK && victim->pg->killed[track].vnum;  track++ )
	{
		tmob = get_mob_index( NULL, victim->pg->killed[track].vnum );

		if ( !tmob )
		{
			send_log( NULL, LOG_BUG, "khistory: unknown mob vnum" );
			continue;
		}

		ch_printf( ch, "   &R%-30s&r(&R#%-5d&r)&R    - uccis%c %d volte\r\n",
			capitalize(tmob->short_descr),
			tmob->vnum,
			(tmob->sex == SEX_FEMALE) ? 'a' : 'o',
			victim->pg->killed[track].count );
	}
}


DO_RET do_users( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		*ldch;
	DESCRIPTOR_DATA *d;
	char			 buf[MSL];
	int				 count;

	count	= 0;
	buf[0]	= '\0';

	set_char_color( AT_PLAIN, ch );

	send_to_pager( ch, "\r\nDesc| Connected        |Idle|Port | Player     @HostIP          |\r\n" );
	send_to_pager( ch,     "----+------------------+----+-----+-----------------------------+\r\n" );

	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( !d->character )
			continue;

		if ( !VALID_STR(argument)
		  || (!str_prefix(argument, d->host) || (d->character && !str_prefix(argument, d->character->name))) )
		{
			count++;
			pager_printf( ch, "%4d|%-18.18s|%4d|%5d|%-12s@%-16s|\r\n",
				d->descriptor,
				code_name(NULL, d->connected, CODE_CONNECTED),
				d->idle / PULSE_IN_SECOND,
				d->port,
				d->original  ?  d->original->name  :
				d->character ? d->character->name  :  "(none)",
				d->host );
		}
	}

	for ( ldch = first_player;  ldch;  ldch = ldch->next_player )
	{
		if ( ldch->desc )	continue;	/* solo quelli in ld */
		if ( !ldch->pg )	continue;
		
		if ( !VALID_STR(argument)
		  || (!str_prefix(argument, ldch->pg->host) || (!str_prefix(argument, ldch->name))) )
		{
			count++;
			pager_printf( ch, "%4s|%-18.18s|%4s|%5s|%-12s@%-16s|\r\n",
						  "(n/a)", "Link dead", "(n/a)", "(n/a)", ldch->name, ldch->pg->host );
		}
	}

	pager_printf( ch, "%d utent%c.\r\n", count, (count == 1) ? 'e' : 'i' );
}


/*
 * (FF) aggiungere l'opzione force su tutti i mob oltre che su tutti i pg
 */
DO_RET do_force( CHAR_DATA *ch, char *argument )
{
	COMMAND_DATA *cmd;			/* Comando da inviare, per controllare che lingua sia */
	char		  arg[MIL];		/* all o nome vittima */
	char		  argc[MIL];	/* Comando da inviare */
	bool		  ita;			/* indica il linguaggio del comando inviato */

	set_char_color( AT_ADMIN, ch );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono farlo.\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_to_char( ch, "Forzare chi a fare cosa?\r\n" );
		return;
	}

	ita = ( HAS_BIT_PLR(ch, PLAYER_ITALIAN) );
	one_argument( argument, argc );
	cmd = find_command( ch, argc, FALSE, ita );
	if ( !cmd )
	{
		/* Cerca il comando nell'altra lingua, se non lo trova l'admin ha sbagliato a digitare */
		cmd = find_command( ch, argc, FALSE, !ita );
		ita = !ita;	/* Lo ha trovato nell'altra lingua, quindi la cambia per la send_command sotto */
		if ( !cmd )
		{
			ch_printf( ch, "Comando inesistente: %s\r\n", argc );
			return;
		}
	}
	one_argument( cmd->name, argc );	/* acquisisce il nome del comando per intero così la send_command lo trova nella ricerca esatta del comando */

	if ( !str_cmp_all(arg, FALSE) )
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next_player;

		for ( vch = first_player;  vch;  vch = vch_next_player )
		{
			vch_next_player = vch->next_player;

			if ( get_trust(vch) >= get_trust(ch) )
				continue;

			/* Attraverso la send_command invia il comando secondo la lingua della vittima */
			send_command( vch, argc, ita ? IT : EN );

			if ( IS_ADMIN(vch) )
				act( AT_ADMIN, "$n ti obbliga a '$t'.", ch, argument, vch, TO_VICT );
		}
	}
	else
	{
		CHAR_DATA *victim;

		victim = get_char_mud( ch, arg, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "Non si trova nella stanza.\r\n" );
			return;
		}

		if ( victim == ch )
		{
			act( AT_ADMIN, "Fai prima a farlo da sol$x.", ch, NULL, NULL, TO_CHAR );
			return;
		}

		if ( get_trust(victim) >= get_trust(ch) )
		{
			act( AT_ADMIN, "Non puoi farlo su di $E.", ch, NULL, victim, TO_CHAR );
			return;
		}

		/* Attraverso la send_command invia il comando con la lingua del comando trovata */
		send_command( victim, argc, ita ? IT : EN );

		if ( IS_ADMIN(victim) )
			act( AT_ADMIN, "$n ti obbliga a '$t'.", ch, argument, victim, TO_VICT );
	}

	send_to_char( ch, "Comando inviato.\r\n" );
}


/*
 * Incognito is a level based command.
 * Once cloaked, all players up to the level set will not be able to see you.
 */
DO_RET do_incognito( CHAR_DATA *ch, char *argument )
{
	int		number;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) || !is_number(argument) )
	{
		send_to_char( ch, "Sintassi: incognito <livello>\r\n" );
		ch_printf( ch, "<livello>: da 0 a %d\r\n", LVL_LEGEND );
		ch_printf( ch, "Livello di incognito attuale %d\r\n", ch->pg->incognito );

		return;
	}

	number = atoi( argument );
	if ( number < 0 || number > LVL_LEGEND )
	{
		send_command( ch, "incognito", CO );
		return;
	}

	if ( IS_PG(ch) )
	{
		ch->pg->incognito = number;
		ch->reply = NULL;
		if ( number == 0 )
			send_to_char( ch , "Disattivi l'incognito.\r\n" );
		ch_printf( ch, "Setti il tuo livello di incognito a %d.\r\n", number );
	}
	else
	{
		ch->mobinvis = number;
		ch->reply = NULL;
		if ( number == 0 )
			send_to_char( ch , "Disattivi il mobinvis.\r\n" );
		ch_printf( ch, "Setti il livello di mobinvis a %d.\r\n", number );
	}
}


DO_RET do_vassign( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg1[MIL];
	char	   arg2[MIL];
	char	   arg3[MIL];
	VNUM	   v_low;
	VNUM	   v_high;

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	v_low	 = atoi( arg2 );
	v_high	 = atoi( arg3 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(arg3)
	  || !is_number(arg1) || !is_number(arg2) || !is_number(arg3)
	  || v_low  < 1		  || v_high < 1
	  || v_low >= MAX_VNUM || v_high >= MAX_VNUM )
	{
		send_to_char( ch, "&YSintassi&w:  vassign <nomepg> <low> <high>\r\n" );
		return;
	}

	victim = get_player_mud( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	if ( v_low > v_high )
	{
		send_to_char( ch, "Unacceptable vnums range.\r\n" );
		return;
	}

	if ( v_low == 0 )
		v_high = 0;

	victim->pg->vnum_range_low = v_low;
	victim->pg->vnum_range_high = v_high;
	assign_area( victim );

	set_char_color( AT_ADMIN, victim );
	ch_printf	( victim, "%s has assigned you the vnum range %d - %d.\r\n", ch->name, v_low, v_high );
	send_to_char( ch, "Fatto.\r\n" );

	if ( !victim->pg->build_area )
	{
		send_log( NULL, LOG_BUILD, "vassign: assign_area failed" );
		return;
	}

	if ( v_low == 0 )
	{
		REMOVE_BIT( victim->pg->build_area->flags, AREAFLAG_LOADED );
		SET_BIT	  ( victim->pg->build_area->flags, AREAFLAG_DELETED );
	}
	else
	{
		SET_BIT	  ( victim->pg->build_area->flags, AREAFLAG_LOADED );
		REMOVE_BIT( victim->pg->build_area->flags, AREAFLAG_DELETED );
	}
}


/*
 * Load up a player file
 */
DO_RET do_loadup( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA		*temp;
	struct stat		 fst;
	char			 fname[1024];
	VNUM			 old_room_vnum;
	bool			 loaded;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Usage: loadup <playername>\r\n" );
		return;
	}

	for ( temp = first_player;  temp;  temp = temp->next_player )
	{
		if ( !str_cmp(argument, temp->name) )
			break;
	}

	if ( temp != NULL )
	{
		send_to_char( ch, "They are already playing.\r\n" );
		return;
	}

	sprintf( fname, "%s%s", PLAYER_DIR, capitalize(argument) );

	if ( !check_parse_name(argument, TRUE) || stat(fname, &fst) == -1 )
		send_to_char( ch, "No such player.\r\n" );	/* Nessun file di quel pg */

	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->next		 = NULL;
	d->prev		 = NULL;
	d->connected = CON_GET_NAME;
	d->outsize	 = 2000;
	CREATE( d->outbuf, char, d->outsize );

	loaded = load_player( d, argument, FALSE );

	if ( !loaded )
	{
		ch_printf( ch, "Caricamento file %s non riuscito\r\n", argument );
		return;
	}

	LINK( d->character, first_char, last_char, next, prev );
	LINK( d->character, first_player, last_player, next_player, prev_player );

	old_room_vnum = d->character->in_room->vnum;
	char_to_room( d->character, ch->in_room );

	if ( get_trust(d->character) >= get_trust(ch) )
	{
		send_command( d->character, "say Do *NOT* disturb me again!", CO );
		send_to_char( ch, "I think you'd better leave that player alone!\r\n" );
		d->character->desc = NULL;
		send_command( d->character, "quit", CO );
		return;
	}

	d->character->desc	 = NULL;
	d->character->retran = old_room_vnum;
	d->character		 = NULL;
	DISPOSE( d->outbuf );	/* (bb) attenzione che qui magari servono altri dispose */
	DISPOSE( d );

	ch_printf( ch, "Player %s loaded from room %d (usare retran finito l'utilizzo di loadup).\r\n",
		capitalize(argument), old_room_vnum );
}


/*
 * Gestisce il fixchar di un pg o di tutti i pg
 */
void fix_char( CHAR_DATA *victim )
{
	AFFECT_DATA *aff;
	OBJ_DATA	*obj;
	int			 x;

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "fixchar: victim passata è NULL" );
		return;
	}

	de_equip_char( victim );

	for ( aff = victim->first_affect;  aff;  aff = aff->next )
		affect_modify( victim, aff, FALSE );

	CLEAR_BITS( victim->affected_by );
	SET_BITS( victim->affected_by, table_race[victim->race]->affected );

	victim->mental_state = -10;
	for ( x = 0;  x < MAX_ATTR;  x++ )
		victim->attr_mod[x] = 0;

	for ( x = 0;  x < MAX_SENSE;  x++ )
		victim->sense_mod[x] = 0;

	for ( x = 0;  x < MAX_POINTS;  x++ )
		victim->points[x] = UMAX( 1, victim->points_max[x] );

	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		victim->saving_throw[x]	= 0;

	victim->armor		= 100;
	victim->damroll		= 0;
	victim->hitroll		= 0;
	victim->alignment	= URANGE( -1000, victim->alignment, 1000 );

	for ( aff = victim->first_affect;  aff;  aff = aff->next )
		affect_modify( victim, aff, TRUE );

	victim->carry_weight = 0;
	victim->carry_number = 0;

	for ( obj = victim->first_carrying;  obj;  obj = obj->next_content )
	{
		if ( obj->wear_loc == WEARLOC_NONE )
			victim->carry_number += obj->count;
		if ( !HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
			victim->carry_weight += get_obj_weight(obj);
	}

	re_equip_char( victim );
}

DO_RET do_fixchar( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Sintassi: fixchar <nomepg>\r\n" );
		send_to_char( ch, "Sintassi: fixchar all\r\n" );
		return;
	}

	if ( !str_cmp_all(argument, FALSE) )
	{
		for ( victim = first_player;  victim;  victim = victim->next_player )
			fix_char( victim );
		send_to_char( ch, "Tutti i personaggi sono stati corretti.\r\n" );
	}
	else
	{
		victim = get_player_mud( ch, argument, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "Non si trova qui.\r\n" );
			return;
		}
		fix_char( victim );
		ch_printf( ch, "Personaggio %s corretto.\r\n", victim->name );
	}
}


/*
---------------------------------------------------------------------
 * Password resetting command, added by Samson 2-11-98
 * Code courtesy of John Strange - Triad Mud
---------------------------------------------------------------------
 */
DO_RET do_formpass( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	char	   *pwdnew;
	char		arg[MIL];

	if ( IS_MOB(ch) )
		return;

	argument = one_argument( argument, arg );
	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Syntax: newpass <char> <newpassword>.\n\r" );
		return;
	}

	victim = get_player_mud( ch, arg, TRUE );
	if ( !victim )
	{
		ch_printf( ch, "%s isn't here.\n\r", arg );
		return;
	}

	/* Admin trust check */
	if ( get_trust(ch) <= get_trust(victim) )
	{
		send_to_char( ch, "You can't change of view that person's password!\n\r" );
		return;
	}

	pwdnew = check_password( ch, argument );

	DISPOSE( victim->pg->pwd );
	victim->pg->pwd = str_dup( pwdnew );
	save_player( victim );
	ch_printf( ch, "&R%s's password has been changed to: %s\r\n&w", victim->name, argument );
	ch_printf( victim, "&RTi è stata cambiata la password in: %s\r\n&w", argument );
}


/*
 * Super-AT command:
 * FOR ALL <action>
 * FOR MORTALS <action>
 * FOR ADMINS <action>
 * FOR MOBS <action>
 * FOR EVERYWHERE <action>
 *
 * Executes action several times, either on ALL players (not including yourself),
 * MORTALS (including trusted characters), ADMINS (characters with level higher than
 * L_HERO), MOBS (Not recommended) or every room (not recommended either!)
 *
 * If you insert a # in the action, it will be replaced by the name of the target.
 *
 * If # is a part of the action, the action will be executed for every target
 * in game. If there is no #, the action will be executed for every room containg
 * at least one target, but only once per room. # cannot be used with FOR EVERY-
 * WHERE. # can be anywhere in the action.
 *
 * Example:
 *
 * FOR ALL SMILE -> you will only smile once in a room with 2 players.
 * FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.
 *
 * Destroying the characters this command acts upon MAY cause it to fail. Try to
 * avoid something like FOR MOBS PURGE (although it actually works at my MUD).
 *
 * FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
 * though :)
 *
 * The command works by transporting the character to each of the rooms with
 * target in them. Private rooms are not violated.
 *
 */

/*
 * Expand the name of a character into a string that identifies THAT
 * character within a room. E.g. the second 'guard' -> 2. guard
 */
static const char *name_expand( CHAR_DATA *ch )
{
	int			count = 1;
	CHAR_DATA  *rch;
	char		name[MIL];		/* FORTUNATAMENTE nessun mob ha un nome lungo così tanto */
	static char outbuf[MIL];

	if ( IS_PG(ch) )
		return ch->name;

	one_argument( ch->name, name );		/* copy the first word into name */

	if ( !name[0] )						/* weird mob .. no keywords */
	{
		strcpy( outbuf, "" );			/* Do not return NULL, just an empty buffer */
		return outbuf;
	}

	/* ->people changed to ->first_person */
	for ( rch = ch->in_room->first_person;  rch && (rch != ch);  rch = rch->next_in_room )
	{
		if ( is_name(name, rch->name) )
			count++;
	}

	sprintf( outbuf, "%d.%s", count, name );
	return outbuf;
}


DO_RET do_for( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA   *room;
	ROOM_DATA   *old_room;
	CHAR_DATA   *p;
	CHAR_DATA   *p_prev;		/* p_next to p_prev */
	char		 range[MIL];
	int			 x;
	bool		 fAdmins = FALSE;
	bool		 fPgs	 = FALSE;
	bool		 fMobs	 = FALSE;
	bool		 fAll	 = FALSE;
	bool		 found;

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, range );
	if ( !VALID_STR(argument) || !VALID_STR(range) )	/* invalid usage? */
	{
		send_command( ch, "help FOR", CO );
		return;
	}

	if ( is_name_prefix(argument, "fine quit") )
	{
		send_to_char( ch, "Are you trying to crash the MUD or something?\r\n" );
		return;
	}

	if		( !str_cmp_all(range, FALSE) )								fAll	= TRUE;
	else if ( is_name(range, "amministratori admin admins amm"	) )		fAdmins	= TRUE;
	else if ( is_name(range, "pg pg players players giocatori"	) )		fPgs	= TRUE;
	else if ( is_name(range, "mob mobs mobile mobiles"			) )		fMobs	= TRUE;
	else
		send_command( ch, "help FOR", CO );	/* show syntax */

	/* do not allow # to make it easier */
	if ( fAll && strchr(argument, '#') )
	{
		send_to_char( ch, "Cannot use FOR EVERYWHERE with the # thingie.\r\n" );
		return;
	}

	set_char_color( AT_PLAIN, ch );
	if ( strchr(argument, '#') )		/* replace # ? */
	{
		for ( p = last_char;  p  ; p = p_prev )
		{
			p_prev = p->prev;
			found = FALSE;

			if ( !(p->in_room) || (p == ch) )
				continue;

			if		( IS_MOB(p)	&& fMobs				  )		found = TRUE;
			else if ( IS_PG(p)	&& fAdmins && IS_ADMIN(p) )		found = TRUE;
			else if ( IS_PG(p)	&& fPgs  && !IS_ADMIN(p)  )		found = TRUE;

			/* It looks ugly to me.. but it works :) */
			if ( found )	/* p is 'appropriate' */
			{
				char *pSource = argument;	/* head of buffer to be parsed */
				char  buf[MSL];
				char *pDest = buf;			/* parse into this */

				while ( *pSource )
				{
					if ( *pSource == '#' )	/* Replace # with name of target */
					{
						const char *namebuf = name_expand(p);

						if ( namebuf )		/* in case there is no mob name ?? */
						{
							while ( *namebuf )	/* copy name over */
								*(pDest++) = *(namebuf++);
						}
						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* chiude il while */
				*pDest = '\0';			/* Terminate */

				/* Execute */
				old_room = ch->in_room;
				char_from_room( ch );
				char_to_room( ch, p->in_room );

				/* Invia il comando secondo la lingua di chi lo manda */
				if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
					send_command( ch, pDest, IT );
				else
					send_command( ch, pDest, EN );

				char_from_room( ch );
				char_to_room( ch, old_room );

			} /* chiude l'if */
		} /* chiude il for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
		for ( x = 0;  x < MAX_KEY_HASH;  x++ )	/* run through all the buckets */
		{
			for ( room = room_index_hash[x];  room;  room = room->next )
			{
				found = FALSE;

				/* Anyone in here at all? */
				if ( fAll )	/* Everywhere executes always */
					found = TRUE;
				else if ( !room->first_person )	/* Skip it if room is empty */
					continue;
				/* ->people changed to first_person */

				/* Check if there is anyone here of the required type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				/* ->people to ->first_person */
				for ( p = room->first_person;  p && !found;  p = p->next_in_room )
				{
					/* do not execute on oneself */
					if ( p == ch )
						continue;

					if		( IS_MOB(p) && fMobs					)	found = TRUE;
					else if ( IS_PG(p)  && fAdmins &&  IS_ADMIN(p)	)	found = TRUE;
					else if ( IS_PG(p)  && fPgs    && !IS_ADMIN(p)	)	found = TRUE;
				}	/* for everyone inside the room */

				if ( found )
				{
					/* This may be ineffective. Consider moving character out of old_room
					 *  once at beginning of command then moving back at the end.
					 *  This however, is more safe? */
					old_room = ch->in_room;
					char_from_room( ch );
					char_to_room( ch, room );

					/* Invia il comando secondo la lingua di chi invia */
					if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
						send_command( ch, argument, IT );
					else
						send_command( ch, argument, EN );

					char_from_room( ch );
					char_to_room( ch, old_room );
				} /* if found */
			} /* for every room in a bucket */
		} /* chiude il for */
	} /* if strchr */
} /* do_for */


/*
 * Vnum search command
 */
DO_RET do_vsearch( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	char	  arg[MIL];
	bool	  found = FALSE;
	int		  obj_counter = 1;
	VNUM	  argvn;

	set_char_color( AT_PLAIN, ch );

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "&YSintassi&w:  vsearch <vnum>.\r\n" );
		return;
	}

	argvn = atoi( arg );
	if ( argvn < 1 && argvn >= MAX_VNUM )
	{
		send_to_char( ch, "Vnum out of range.\r\n" );
		return;
	}

	for ( obj = first_object;  obj != NULL;  obj = obj->next )
	{
		if ( argvn != obj->vnum )
			continue;

		found = TRUE;
		for ( in_obj = obj;  in_obj->in_obj != NULL;  in_obj = in_obj->in_obj )
			;

		if ( in_obj->carried_by != NULL )
		{
			pager_printf( ch, "[%2d] Level %d %s carried by %s.\r\n",
				obj_counter,
				obj->level,
				obj_short(obj),
				PERS(in_obj->carried_by, ch) );
		}
		else
		{
			pager_printf( ch, "[%2d] [%-5d] %s in %s.\r\n",
				obj_counter,
				(in_obj->in_room)  ?  in_obj->in_room->vnum  :  0,
				obj_short(obj),
				(in_obj->in_room == NULL)  ?  "somewhere"  :  in_obj->in_room->name );
		}
		obj_counter++;
	}

	if ( !found )
		send_to_char( ch, "Nothing like that in hell, earth, or heaven.\r\n" );
}


/* (RR) da togliere, oppure da dare come trust TRUST_BUILDER
 *
 * Quest point set
 * Syntax is: qpset char give/take amount
 */
DO_RET do_qpset( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg[MIL];
	char	   arg2[MIL];
	int		   amount;
	bool	   give;
	bool	   set;

	set_char_color( AT_ADMIN, ch );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Cannot qpset as an NPC.\r\n" );
		return;
	}

	argument = one_argument( argument, arg	);
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg) || !VALID_STR(arg2) || !VALID_STR(argument)
	  || arg[0] == '?' || !is_number(argument) )
	{
		send_to_char( ch, "&YSintassi&w: qpset <nomepg> <dai>     <valore>\r\n" );
		send_to_char( ch, "&YSintassi&w: qpset <nomepg> <togli>   <valore>\r\n" );
		send_to_char( ch, "&YSintassi&w: qpset <nomepg> <imposta> <valore>\r\n" );
		return;
	}

	victim = get_player_mud( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	set_char_color( AT_ADMIN, victim );
	if ( is_name_prefix(arg2, "dai give") )
	{
		give = TRUE;
		set	 = FALSE;
	}
	else if ( is_name_prefix(arg2, "togli take") )
	{
		give = FALSE;
		set	 = FALSE;
	}
	else if ( is_name_prefix(arg2, "imposta set") )
	{
		give = FALSE;
		set	 = TRUE;
	}
	else
	{
		send_command( ch, "qpset", CO );
		return;
	}

	amount = atoi( argument );
	if ( !set && amount <= 0 )
	{
		ch_printf( ch, "Valore di gloria da %s %s.\r\n",
			give ? "Dare" : "Togliere",
			amount == 0 ? "nullo" : "negativo" );
		send_command( ch, "qpset", CO );
	}

	if ( set )
	{
		if ( amount < -1000000 || amount > 1000000 )
		{
			send_to_char( ch, "Valore di gloria da impostare esagerato.\r\n" );
			return;
		}

		victim->pg->glory = amount;
	}
	else
	{
		if ( give )
		{
			if ( victim->pg->glory + amount > 1000000 )
			{
				send_to_char( ch, "No, è troppo.\r\n" );
				return;
			}
	
			victim->pg->glory += amount;
			victim->pg->glory_accum += amount;
			ch_printf( ch, "You have increased the glory of %s by %d.\r\n", victim->name, amount );
		}
		else
		{
			if ( victim->pg->glory - amount < 0 )
			{
				send_to_char( ch, "No, è troppo, la sua gloria andrebbe sotto lo zero.\r\n" );
				return;
			}
	
			victim->pg->glory -= amount;
			ch_printf( ch, "You have decreased the glory of %s by %d.\r\n", victim->name, amount );
		}
	}
}


/*
 * Visualizza il contenuto di un file testuale a video.
 */
DO_RET do_fshow( CHAR_DATA *ch, char *argument )
{
	char arg[MIL];

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		ch_printf	( ch, "&YSintassi&w:  fshow %s\r\n", IDEA_FILE );
		ch_printf	( ch, "&YSintassi&w:  fshow %s\r\n", PBUG_FILE );
		ch_printf	( ch, "&YSintassi&w:  fshow %s\r\n", TYPO_FILE );
		ch_printf	( ch, "&YSintassi&w:  fshow %s\r\n", COMMENT_FILE );
		ch_printf	( ch, "&YSintassi&w:  fshow %s\r\n", FIXED_FILE );
		send_to_char( ch, "&YSintassi&w:  fshow clan <clan>\r\n" );

		return;
	}

	one_argument( argument, arg );

	if		( !str_cmp(arg, PBUG_FILE)	  )	send_command( ch, "bug list", CO );
	else if ( !str_cmp(arg, IDEA_FILE)	  )	send_command( ch, "ideas list", CO );
	else if ( !str_cmp(arg, COMMENT_FILE) )	send_command( ch, "comments list", CO );
	else if ( !str_cmp(arg, TYPO_FILE)	  )	send_command( ch, "typo list", CO );
	else if ( !str_cmp(arg, FIXED_FILE)   )	send_command( ch, "fixed list", CO );
	else if ( !str_cmp(arg, "clan")  )
	{
		char buf[MIL];

		sprintf( buf, "victories %s", arg );
		send_command( ch, buf, CO );
	}
	else
	{
		ch_printf( ch, "Nessun file %s esistente.\r\n", arg );
	}
}


/*
 * Command to check for multiple ip addresses in the mud
 */
typedef struct ipcompare_data
{
	struct	ipcompare_data *prev;
	struct	ipcompare_data *next;
	char   *host;
	char   *name;
	int		connected;
	int		count;
	int		descriptor;
	int		idle;
	int		port;
	bool	printed;
} IPCOMPARE_DATA;

DO_RET do_ipcompare( CHAR_DATA *ch, char *argument )
{
	IPCOMPARE_DATA	 *first_ip = NULL;
	IPCOMPARE_DATA	 *last_ip  = NULL;
	IPCOMPARE_DATA	 *hmm;
	IPCOMPARE_DATA	 *hmm_next;
	DESCRIPTOR_DATA *d;
	CHAR_DATA		*victim;
	char			*addie = NULL;
	char			 arg[MIL];
	char			 arg1[MIL];
	char			 arg2[MIL];
	int				 count = 0;
	int				 times = -1;
	bool			 prefix = FALSE;
	bool			 suffix = FALSE;
	bool			 inarea = FALSE;
	bool			 inroom = FALSE;
	bool			 inworld = FALSE;
	bool			 fMatch;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg1);
	argument = one_argument( argument, arg2);

	set_char_color (AT_PLAIN, ch);

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, NOFOUND_MSG  );
		return;
	}

	if ( !VALID_STR(arg) )
	{
		send_to_pager( ch, "ipcompare\r\n" );
		send_to_pager( ch, "ipcompare <person> [room|area|world] [#]\r\n" );
		send_to_pager( ch, "ipcompare <site>   [room|area|world] [#]\r\n\r\n" );

		for ( d = first_descriptor;  d;  d = d->next )
		{
			fMatch = FALSE;
			for ( hmm = first_ip;  hmm;  hmm = hmm->next )
			{
				if ( !str_cmp(hmm->host, d->host) )
					fMatch = TRUE;
			}

			if ( !fMatch )
			{
				IPCOMPARE_DATA *temp;
				CREATE( temp, IPCOMPARE_DATA, 1);
				temp->host = str_dup ( d->host );
				LINK( temp, first_ip, last_ip, next, prev );
				count++;
			}
		}

		for ( hmm = first_ip;  hmm;  hmm = hmm_next )
		{
			hmm_next = hmm->next;
			UNLINK( hmm, first_ip, last_ip, next, prev );
			DISPOSE( hmm->host );
			DISPOSE( hmm );
		}
		ch_printf(ch, "There were %d unique ip addresses found.\r\n", count );
		return;
	}

	if ( VALID_STR(arg1) )
	{
		if ( is_number(arg1) )
		{
			times = atoi( arg1 );
		}
		else
		{
			if		( !str_cmp(arg1, "stanza") || !str_cmp(arg1, "room") )		inroom	= TRUE;
			else if ( !str_cmp(arg1, "area")							 )		inarea	= TRUE;
			else																inworld = TRUE;
		}

		if ( VALID_STR(arg2) )
		{
			if ( is_number(arg2) )
				times = atoi( arg2 );
			else
			{
				send_to_char( ch, "Please see help ipcompare for more info.\r\n" );
				return;
			}
		}
	} /* chiude l'if */

	victim = get_player_mud( ch, arg, TRUE );
	if ( victim && victim->desc )
		addie = victim->desc->host;
	else
	{
		addie = arg;

		if ( arg[0] == '*' )
		{
			prefix = TRUE;
			addie++;
		}

		if ( addie[strlen(addie) -1] == '*' )
		{
			suffix = TRUE;
			addie[strlen(addie)-1] = '\0';
		}
	}

	send_to_pager( ch, "\r\nDesc| Connected        |Idle|Port | Player     @HostIP          |\r\n" );
	send_to_pager( ch,     "----+------------------+----+-----+-----------------------------+\r\n" );

	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( inroom && ch->in_room != d->character->in_room )				continue;
		if ( inarea && ch->in_room->area != d->character->in_room->area )	continue;

		if ( times > 0 && count == (times-1) )
			break;

		if		( prefix && suffix && strstr(addie, d->host ) )		fMatch = TRUE;
		else if ( prefix && !str_suffix(addie , d->host)	  )		fMatch = TRUE;
		else if ( suffix && !str_prefix(addie , d->host)	  )		fMatch = TRUE;
		else if ( !str_cmp(d->host, addie)					  )		fMatch = TRUE;
		else														fMatch = FALSE;

		if ( fMatch )
		{
			count++;
			pager_printf( ch, " %3d|%-18.18s|%4d|%5d|%-12s@%-16s|\r\n",
				d->descriptor,
				code_name(NULL, d->connected, CODE_CONNECTED),
				d->idle / PULSE_IN_SECOND,
				d->port,
				d->original  ?  d->original->name  :
				d->character ? d->character->name  :  "(none)",
				d->host );
		}
	}

	pager_printf( ch, "%d user%s.\r\n", count, (count == 1)  ?  ""  :  "s" );
}


/*
 * Comando admin che permette di rinominare un pg
 */
DO_RET do_rename( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg1[MIL];
	char	   arg2[MIL];
	char	   newname[MSL];
	char	   oldname[MSL];
	char	   backname[MSL];

	argument = one_argument( argument, arg1 );
	one_argument( argument, arg2 );
	smash_tilde( arg2 );

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "&YSintassi&w: rename <victim> <new name>\r\n" );
		return;
	}

	if  ( !check_parse_name(arg2, 1) )
	{
		send_to_char( ch, "Illegal name.\r\n" );
		return;
	}

	/* Just a security precaution so you don't rename someone you don't mean too */
	victim = get_player_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "That person is not in the room.\r\n" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_to_char( ch, "You can't rename NPC's.\r\n" );
		return;
	}

	if ( get_trust(ch) < get_trust(victim) )
	{
		send_to_char( ch, "I don't think they would like that!\r\n" );
		return;
	}

	sprintf( newname, "%s%s", PLAYER_DIR, capitalize(arg2) );
	sprintf( oldname, "%s%s", PLAYER_DIR, victim->name );
	sprintf( backname,"%s%s", BACKUP_DIR, victim->name );

	if ( access(newname, F_OK) == 0 )
	{
		send_to_char( ch, "That name already exists.\r\n" );
		return;
	}

	/* Have to remove the old admin entry in the directories */
	if ( IS_ADMIN(victim) )
	{
		char adm_name[MSL];

		sprintf( adm_name, "%s%s", ADMIN_DIR, victim->name );
		remove( adm_name );
	}

	/* Remember to change the names of the areas */
	if ( victim->pg->build_area )
	{
		char filename[MSL];
		char newfilename[MSL];

		sprintf( filename, "%s%s.are", BUILD_DIR, victim->name );
		sprintf( newfilename, "%s%s.are", BUILD_DIR, capitalize(arg2) );
		if ( rename(filename, newfilename) != 0 )
			send_log( NULL, LOG_BUG, "do_rename: errore nel rinomare da %s a %s: %s", filename, newfilename, strerror(errno) );
		sprintf( filename, "%s%s.are.bak", BUILD_DIR, victim->name );
		sprintf( newfilename, "%s%s.are.bak", BUILD_DIR, capitalize(arg2) );
		if ( rename(filename, newfilename) != 0 )
			send_log( NULL, LOG_BUG, "do_rename: errore nel rinomare da %s a %s: %s", filename, newfilename, strerror(errno) );
	}

	rename_locker( victim, arg2 );		/* Rinomina l'eventuale locker */
	rename_account( victim, arg2 );	/* Rinomina l'eventuale account bancario */
	rename_notes( victim, arg2 );		/* Rinomina le note e le mail del pg */

	/* Rimuove il corrispettivo offline, dopo il renaming lo reinserà */
	remove_offline( victim );

	DISPOSE( victim->name );
	victim->name = str_dup( capitalize(arg2) );

	if ( remove(backname) != 0 )
		ch_printf( ch, "Couldn't delete the old backup file %s!\r\n", oldname );

	if ( remove(oldname) != 0 )
	{
		ch_printf( ch, "Couldn't delete the old file %s!\r\n", oldname );
		send_log( NULL, LOG_NORMAL, "Error: Couldn't delete file %s in do_rename. errno: %s", oldname, strerror(errno) );
	}

	/* Time to save to force the affects to take place */
	save_player( victim );
	add_offline( victim );

	send_to_char( ch, "Character was renamed.\r\n" );
}


/*
 * Info
 */
DO_RET do_info( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: info <messaggio>\r\n" );
		send_to_char( ch, "Il messaggio di informazione viene inviato a tutti i giocatori nel Mud.\r\n" );
		return;
	}

	for ( victim = first_player;  victim;  victim = victim->next_player )
	{
		if ( victim->desc && victim->desc->connected == CON_PLAYING )
			ch_printf( victim, "&W[&RINFO&W] &c%s\r\n", argument );
	}
}


/*
 * Saveall
 */
DO_RET do_saveall( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		*vch;
	AREA_DATA		*tarea;
	char			 filename[MIL];

	/* Salva i pg */
	for ( vch = first_player;  vch;  vch = vch->next_player )
	{
		if ( IS_PG(vch) )
			save_player( vch );
	}

	/* Salva le aree */
	for ( tarea = first_build;  tarea;  tarea = tarea->next )
	{
		if ( !HAS_BIT(tarea->flags, AREAFLAG_LOADED) )
			continue;
		sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
		fold_area( tarea, filename, FALSE );
	}

	send_to_char( ch, "Personaggi e aree sono stati salvati.\r\n" );
}


/*
 * Controlla i valori di un oggetto (indicizzato o non)
 *	a caricamento o tramite comando do_check
 */
void check_object( OBJ_DATA *obj, MUD_FILE *fp )
{
	char	buf[MSL];

	/*
	 * Controlla prima le variabili per gli oggetti indicizzati (FF) mancano dei campi
	 */
	if		( obj->prototype && obj->pObjProto )
		send_log( fp, LOG_CHECK, "check_object(%d): oggetto prototipo con puntatore a prototipo: %d", -1 , obj->vnum );
	else if ( !obj->prototype && !obj->pObjProto )
		send_log( fp, LOG_CHECK, "check_object(%d): oggetto non prototipo senza puntatore a prototipo: %d", -1, obj->vnum );

	/* (FF) Manca il check di alcune variabili */

	if ( obj->vnum <= 0 )
		send_log( fp, LOG_CHECK, "check_object(%d): vnum errato: %d", obj->vnum, -1 );
	if ( obj->vnum  >= MAX_VNUM )
		send_log( fp , LOG_CHECK, "check_object(%d): vnum #%d maggiore di MAX_VNUM", obj->vnum, -1 );
	if ( isupper(obj->short_descr[0]) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): short descr non in minuscolo", obj->vnum );
	obj->short_descr[0] = tolower( obj->short_descr[0] );
	if ( islower(obj->long_descr[0]) )
		send_log( fp, LOG_CHECK, "check_object(%d): long descr non in maiuscolo", obj->vnum );
	obj->long_descr[0]	= toupper( obj->long_descr[0] );
	if ( !nifty_is_name(obj->name, obj->long_descr) )
		send_log( fp, LOG_CHECK, "check_object(%d): keyword non trovate nella long descr", obj->vnum );

	if ( obj->type <= 0 || obj->type >= MAX_OBJTYPE )
		send_log( fp, LOG_CHECK, "check_object(%d): tipo di oggetto errato: %d", obj->vnum, obj->type );

	/* (TT) Bisognerà fare anche altri controlli all'hold, wield, missile_wield? */
	/* Controlla che non abbia la flag di dual wielding */
	if ( HAS_BIT(obj->wear_flags, OBJWEAR_DUAL) )
		send_log( fp, LOG_CHECK, "check_object(%d): errore, l'oggetto possiede la flag di dual wielding", obj->vnum );
	/* Controlla che abbia la flag di take se possiede un wear flag */
	if ( !IS_EMPTY(obj->wear_flags) && !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE) )
		send_log( fp, LOG_CHECK, "check_object(%d): non possiede la flag di take nonostante sia wereable", obj->vnum );
	/* (RR) che test mettere a layers? */

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_SECRET) )
		send_log( fp, LOG_CHECK, "check_object(%d): attenzione ex flag di prototipo", obj->vnum );

	if ( obj->weight < 1 || obj->weight > 1000000 )	/* > 1q */
		send_log( fp, LOG_CHECK, "check_object(%d): peso errato: %d", obj->vnum, obj->weight );

	if ( obj->cost < 0 || obj->cost > 100000 )
		send_log( fp, LOG_CHECK, "check_object(%d): costo errato: %d", obj->vnum, obj->cost );

	/* Nessun test per rent che tanto è inutilizzato */

	/* Controllo dei value suddiviso per tipi di oggetti., deve stare dopo la lettura
	 * Da tenere sotto la conversione in numero di tutti gli sn nei value in fread_object.
	 * (TT) (FF) son convinto che questi check presi dal doc area.txt non siano aggiornati */
	strcpy( buf, code_name(NULL, obj->type, CODE_OBJTYPE) );
	switch ( obj->type )
	{
	  default:
		send_log( fp, LOG_CHECK, "check_object(%d): controlli di value del tipo di oggetto %s non settati", obj->vnum, buf );
		break;

	  /* Qui vanno i tipi di oggetti senza struttura di value */
	  case OBJTYPE_TRASH:
	  case OBJTYPE_COOK:
	  case OBJTYPE_SCRAPS:
	  case OBJTYPE_FOUNTAIN:
	  case OBJTYPE_BLOOD:
	  case OBJTYPE_BLOODSTAIN:
	  case OBJTYPE_FIRE:
	  case OBJTYPE_PORTAL:
		break;

	  case OBJTYPE_LIGHT:
		/* (FF) Convertirlo in intensità di luce, da 1 a 100, se negativo, da -1 a -100 significa che è una ombra */
		if ( obj->light->intensity != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->light->intensity );
		/* (FF) Convertirlo a tipo di luce, magica, benedetta, naturale, artificiale, attivabile (se la fa solo impugnandola-indossandola, forse per questo ci vuole qualche flag?) */
		if ( obj->light->type != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->light->type );
		/* Ore di luce dell'oggetto, 0 è spento, -1 infinite, 32000 è il limite massimo di ore */
		if ( obj->light->hours < -1 || obj->light->hours > 32000 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->light->hours );
		break;

	  case OBJTYPE_SCROLL:
	  case OBJTYPE_POTION:
	  case OBJTYPE_PILL:
		/* Livello */
		if ( obj->scroll->level < 1 || obj->scroll->level > LVL_LEGEND )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->scroll->level );
		/* Spell 1 */
		if ( obj->scroll->sn1 < -1 || obj->scroll->sn1 >= top_sn )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->scroll->sn1 );
		/* Spell 2 */
		if ( obj->scroll->sn2 < -1 || obj->scroll->sn2 >= top_sn )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->scroll->sn2 );
		/* Spell 3 */
		if ( obj->scroll->sn3 < -1 || obj->scroll->sn3 >= top_sn )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->scroll->sn3 );
		break;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		/* Livello */
		if ( obj->wand->level < 1 || obj->wand->level > LVL_LEGEND )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->wand->level );
		/* Cariche massime */
		if ( obj->wand->max_charges < 1 || obj->wand->max_charges > 100 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->wand->max_charges );
		/* Cariche correnti */
		if ( obj->wand->curr_charges < 1 || obj->wand->curr_charges > 100 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->wand->curr_charges );
		/* Controlla che le cariche correnti non superino le massime */
		if ( obj->wand->curr_charges > obj->wand->max_charges )
			send_log( fp, LOG_CHECK, "check_object(%d): valore di cariche correnti maggiore di quelle massime: %d", obj->vnum, obj->wand->curr_charges );
		if ( obj->wand->sn < -1 || obj->wand->sn >= top_sn )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->wand->sn );
		break;

	  case OBJTYPE_WEAPON:
		/* Condition */
		if ( obj->weapon->condition < 0 || obj->weapon->condition > INIT_WEAPON_CONDITION )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->weapon->condition );
		/* Number dice	v1 and v2 are optional and will be */
		if ( obj->weapon->dice_number < 0 /*|| obj->value[1] > */ )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->weapon->dice_number );
		/* Size dice	autogenerated if set to 0 */
		if ( obj->weapon->dice_size < 0 /*|| obj->value[2] > */ )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->weapon->dice_size );
		/* Tipo di arma, suddivide ciò con le tipologie di danno */
		if ( obj->weapon->damage1 < 0 || obj->weapon->damage1 >= MAX_DAMAGE )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->weapon->damage1 );
		break;

	  case OBJTYPE_TREASURE:
		if ( obj->treasure->type != 0 )	/* (TT) test per vedere che oggetti sono ad avere valori diversi da 0 */
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->treasure->type );
		/* Condizione */
		if ( obj->treasure->condition < 0 /*|| obj->value[1] > INIT_WEAPON_CONDITION */ )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->treasure->condition );
		break;

	  case OBJTYPE_ARMOR:
#ifdef T2_ALFA
		/* AC adjust/current condition */
		if ( obj->armor->ac )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->armor->ac );
		/* Original AC */
		if ( obj->armor->ac_orig )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->armor->ac_orig );
		/* Tipo di pezzo di armatura */
		if ( obj->armor->type )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->armor->type );
#endif
		break;

	  case OBJTYPE_FURNITURE:
		if ( obj->furniture->regain[POINTS_LIFE] != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->furniture->regain[POINTS_LIFE] );
		if ( obj->furniture->regain[POINTS_MOVE] != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->furniture->regain[POINTS_MOVE] );
		if ( obj->furniture->regain[POINTS_MANA] != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->furniture->regain[POINTS_MANA] );
		if ( obj->furniture->regain[POINTS_SOUL] != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->furniture->regain[POINTS_SOUL] );
		break;

	  case OBJTYPE_CONTAINER:
		/* Capacità del contenitore */
		if ( obj->container->capacity < 1 || obj->container->capacity > 1000000 )	/* 1000 kili */
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->container->capacity );
		/* Vnum della chiave che apre il contenitore */
		if ( obj->container->key <= 0 || obj->container->key >= MAX_VNUM )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->container->key );
#ifdef T2_ALFA
		/* (FF) Durability, cioè condizione, da spostare in condizione */
		if ( obj->container->durability != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->container->durability );
#endif
		break;

	  case OBJTYPE_DRINK_CON:
		/* Capacità del contenitore */
		if ( obj->drinkcon->capacity < 1 || obj->drinkcon->capacity > 1000000 )	/* > 1q */
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->drinkcon->capacity );
		/* Current quantity */
		if ( obj->drinkcon->curr_quant < 1 || obj->drinkcon->curr_quant > 1000000 )	/* > 1q */
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->drinkcon->curr_quant );
		/* Numero del liquido, vedere const.c */
		if ( obj->drinkcon->liquid < 0 || obj->drinkcon->liquid >= MAX_LIQ )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->drinkcon->liquid );
		/* Se non è zero il liquido è avvelenato */
		if ( obj->drinkcon->poison < 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->drinkcon->poison );
		break;

	  case OBJTYPE_FOOD:
		/* Hours of food value */
		if ( obj->food->hours < 0 /*|| obj->food->hours > */ )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->food->hours );
		/* Condizione */
		if ( obj->food->condition < 0 /*|| obj->food->condition > INIT_WEAPON_CONDITION (TT)*/ )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->food->condition );
		/* if non-zero, food is poisoned (value 3 e non 2) */
		if ( obj->food->poison < 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->food->poison );
#ifdef T2_ALFA
		/* (FF) cook, burned, etc etc */
		if ( obj->food->flags != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 4 per tipo %s errato: %d", obj->vnum, buf, obj->food->flags );
#endif
		break;

	  case OBJTYPE_MONEY:
		/* value in gold pieces */
		if ( obj->money->qnt <= 0 || obj->money->qnt > 10000 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->money->qnt_recycle );
		if ( obj->money->qnt_recycle != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->money->qnt_recycle );
		if ( obj->money->type != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->money->type );
		break;

	  case OBJTYPE_BOAT:
		/* (FF) viaggi-room max */
		if ( obj->boat->rooms != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->boat->rooms );
		break;

	  case OBJTYPE_CORPSE_MOB:
	  case OBJTYPE_CORPSE_PG:
		/* Tempo, variabile ad uso online*/
		if ( obj->corpse->time != 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->corpse->time );
		break;

	  case OBJTYPE_TRAP:
		/* Cariche della trappola */
		if ( obj->trap->charges < 0 || obj->trap->charges > 100 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->trap->charges );
		/* Tipo di trappola */
		if ( obj->trap->type < 0 || obj->trap->type >= MAX_TRAPTYPE )	/* (TT) oppure < 1 ? */
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->trap->type );
		/* Livello della trappola */
		if ( obj->trap->level < 1 || obj->trap->level > LVL_LEGEND )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->trap->level );
		break;

	  case OBJTYPE_SALVE:
		/* Livello */
		if ( obj->salve->level < 1 || obj->salve->level > LVL_LEGEND )
			send_log( fp, LOG_CHECK, "check_object(%d): value 0 per tipo %s errato: %d", obj->vnum, buf, obj->salve->level );
		/* cariche attuali */
		if ( obj->salve->charges < 0 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 1 per tipo %s errato: %d", obj->vnum, buf, obj->salve->charges );
		/* cariche massime */
		if ( obj->salve->max_charges < 1 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 2 per tipo %s errato: %d", obj->vnum, buf, obj->salve->max_charges );
		/* controlla che le cariche attuali non superino le cariche massime */
		if ( obj->salve->charges > obj->salve->max_charges )
			send_log( fp, LOG_CHECK, "check_object(%d): errore per tipo %s: cariche attuali (%d) superano le cariche massime (%d)", obj->vnum, buf, obj->salve->charges, obj->salve->max_charges );
		/* delay di utilizzo della salve */
		if ( obj->salve->delay < 1 )
			send_log( fp, LOG_CHECK, "check_object(%d): value 3 per tipo %s errato: %d", obj->vnum, buf, obj->salve->delay );
		/* spell 1 */
		if ( obj->salve->sn1 < -1 || obj->salve->sn1 > top_sn )
			send_log( fp, LOG_CHECK, "check_object(%d): value 4 per tipo %s errato: %d", obj->vnum, buf, obj->salve->sn1 );
		/* spell 2 */
		if ( obj->salve->sn2 < -1 || obj->salve->sn2 > top_sn )
			send_log( fp, LOG_CHECK, "check_object(%d): value 5 per tipo %s errato: %d", obj->vnum, buf, obj->salve->sn2 );
		break;
	}

	/* (RR) bisogna creare il sistema di check delle keyword sulle extra degli oggetti */
#ifdef T2_ALFA
	if ( !xxx(ed->keyword, ed->description) )
		send_log( fp, LOG_CHECK, "fread_object: una o più keyword di extra descr non trovate nella descr", obj->vnum );
#endif
}


/*
 * Controlla i valori di una room index.
 */
void check_room( ROOM_DATA *pRoom, MUD_FILE *fp )
{
	EXIT_DATA *pexit;
	bool	   found_exit = FALSE;

	if ( pRoom->vnum <= 0 )
		send_log( fp, LOG_CHECK, "check_room(%d): vnum minore o uguale a 0: %d", -1, pRoom->vnum );

	if ( pRoom->vnum >= MAX_VNUM )
		send_log( fp, LOG_CHECK, "check_room(%d): vnum maggiore a MAX_VNUM (#%d) :%d", -1, MAX_VNUM, pRoom->vnum );

	if ( HAS_BIT(pRoom->flags, ROOM_PROTOTYPE) )
		send_log( fp, LOG_CHECK, "check_room(%d): flag di prototipo", pRoom->vnum );

	if ( pRoom->sector < 0 || pRoom->sector >= MAX_SECTOR )
		send_log( fp, LOG_CHECK, "check_room(%d): settore errato: %d.", pRoom->vnum, pRoom->sector );

	/* (TT) test per il tele delay */

	if ( pRoom->tele_vnum < 0 || pRoom->tele_vnum >= MAX_VNUM )
		send_log( fp, LOG_CHECK, "check_room(%d): tele vnum errato: %d", pRoom->vnum, pRoom->tele_vnum );

	/* (TT) mi serve per controllare le stanze teledelay e convertire il pulse in secondi */
	if ( pRoom->tele_vnum != 0 )
		send_log( fp, LOG_CHECK, "check_room(%d): teledelay attivo alla stanza #%d", pRoom->vnum, pRoom->tele_vnum );

	/* (CC) Da togliere */
	if ( pRoom->tunnel != 0 )
		send_log( fp, LOG_CHECK, "check_room(%d): lunghezza tunnel errata: %d", pRoom->vnum, pRoom->tunnel );

	/* Test per le uscite */
	for ( pexit = pRoom->first_exit;  pexit;  pexit = pexit->next )
	{
		found_exit = TRUE;

		/* (RR) Controllare che la keyword dell'uscita si trovi nella descrizione della room
		 *	oppure in quella della exit
		 *	oppure ancora in una extra descr della stanza */

		/* Direzione o portale */
		if ( pexit->vdir < 0 || pexit->vdir >= MAX_DIR )
			send_log( fp, LOG_CHECK, "check_room(%d): direzione dell'uscita errata: %d", pRoom->vnum, pexit->vdir );

		/* chiave dell'uscita */
		if ( pexit->key < -1 || pexit->key >= MAX_VNUM )
			send_log( fp, LOG_CHECK, "check_room(%d): vnum chiave errato: %d", pRoom->vnum, pexit->key );

		/* Vnum ove porta l'uscita */
		if ( pexit->vnum <= 0 || pexit->vnum >= MAX_VNUM )
			send_log( fp, LOG_CHECK, "check_room(%d): vnum di uscita errato: %d", pRoom->vnum, pexit->vnum );

		/* Se ha delle flag da porta e non ha la flag di porta avverte */
		if ( !HAS_BIT(pexit->flags, EXIT_ISDOOR)
		  && (HAS_BIT(pexit->flags, EXIT_CLOSED)
		  ||  HAS_BIT(pexit->flags, EXIT_LOCKED)
		  ||  HAS_BIT(pexit->flags, EXIT_NOPASSDOOR)
		  ||  HAS_BIT(pexit->flags, EXIT_BASHED)
		  ||  HAS_BIT(pexit->flags, EXIT_BASHPROOF)
		  ||  HAS_BIT(pexit->flags, EXIT_ISBOLT)
		  ||  HAS_BIT(pexit->flags, EXIT_BOLTED)) )
		{
			send_log( fp, LOG_CHECK, "check_room(%d): Flag da porta anche se manca la flag di porta alla direzione %d della stanza %d", pRoom->vnum, pexit->vdir, pRoom->vnum );
		}

		/* Distanza, (CC) futuro check < 1, */
		if ( pexit->distance < 0 || pexit->distance > 10 )
			send_log( fp, LOG_CHECK, "check_room(%d): distanza dell'uscita errata: %d", pRoom->vnum, pexit->distance );

		if ( pexit->pulltype < 0 || pexit->pulltype >= MAX_PULL )
			send_log( fp, LOG_CHECK, "check_room(%d): tipo di pull dell'uscita errato: %d", pRoom->vnum, pexit->pulltype );

		if ( pexit->pull < -100 || pexit->pull > 100 )
			send_log( fp, LOG_CHECK, "check_room(%d): potenza del pull errata: %d", pRoom->vnum, pexit->pull );
	}

	/* Se non ha trovato uscite, allora controlla che la stanza abbia la flag di DT */
	if ( !found_exit && !HAS_BIT(pRoom->flags, ROOM_DEATH) )
		send_log( NULL, LOG_CHECK, "check_room(%d): stanza senza uscite ma non è una DT", pRoom->vnum );

	/* (RR) Controllare che la keyword della extra descr stia nella descrizione della stanza
	 *	Oppure in altre extra? solo che qui bisognerebbe fare un controllo nesting... */
}


/*
 * Controlla i valori di una struttura mob index
 * CHAR_DATA servirà per fare dei testing online sui singoli mob
 */
void check_mobile( MOB_PROTO_DATA *pMobProto, MUD_FILE *fp )
{
	if ( pMobProto->vnum <= 0 )
		send_log( fp, LOG_CHECK, "check_mobile(%d): vnum errato: %d", -1, pMobProto->vnum );

	if ( pMobProto->vnum  >= MAX_VNUM )
		send_log( fp , LOG_CHECK, "check_mobile(%d): vnum #%d maggiore di MAX_VNUM", -1, pMobProto->vnum );

	if ( isupper(pMobProto->short_descr[0]) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): short descr non in minuscolo", pMobProto->vnum );

	if ( islower(pMobProto->long_descr[0]) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): long descr non in maiuscolo", pMobProto->vnum );

	/* Controlla che le keyword si trovino nella descrizione (FF) bisogna fare che identifichi il ì come apostrofo e legga le parole staccate nella long, altrimenti per esempio: potrebbe non trovare la key "avatar" nella long "dell'avatar" */
	if ( !nifty_is_name(pMobProto->name, pMobProto->long_descr) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): nessuna keyword trovata nella long descr", pMobProto->vnum );

	if ( islower(pMobProto->description[0]) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): description non in maiuscolo", pMobProto->vnum );

#ifdef T2_ALFA
	/* (GR) controllo che siano accenti reali e non muddosi */
	if ( mud_accent(pMobProto->short_descr) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): accenti muddosi nella short", pMobProto->vnum );
	if ( mud_accent(pMobProto->long_descr) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): accenti muddosi nella long", pMobProto->vnum );
	if ( mud_accent(pMobProto->description) )
		send_log( fp, LOG_CHECK, "check_mobile(%d): accenti muddosi nella description", pMobProto->vnum );
#endif

	if ( pMobProto->alignment < -1000 || pMobProto->alignment > 1000 )
		send_log( fp, LOG_CHECK, "check_mobile(%d): allineamento errato: %d", pMobProto->vnum, pMobProto->alignment );

	if ( pMobProto->level < 1 || pMobProto->level > LVL_LEGEND )
		send_log( fp, LOG_CHECK, "check_mobile(%d): livello errato: %d", pMobProto->vnum, pMobProto->level );

	if ( pMobProto->gold > pMobProto->level * 100 )
		send_log( fp, LOG_CHECK, "check_mobile(%d): forse un po' troppe monete: %d", pMobProto->vnum, pMobProto->gold );

	/* (FF) Aggiungere un check sul massimale della exp */
	if ( pMobProto->exp < 0 )
		send_log( fp, LOG_CHECK, "check_mobile(%d): exp minore di zero: %d", pMobProto->vnum, pMobProto->exp );

	if ( pMobProto->position < POSITION_SLEEP || pMobProto->position >= MAX_POSITION )
		send_log( fp, LOG_CHECK, "check_mobile(%d): posizione errata: %d", pMobProto->vnum, pMobProto->position );

	if ( pMobProto->defposition < POSITION_SLEEP || pMobProto->defposition >= MAX_POSITION )
		send_log( fp, LOG_CHECK, "check_mobile(%d): posizione errata: %d", pMobProto->vnum, pMobProto->position );

	if ( pMobProto->sex != SEX_MALE && pMobProto->sex != SEX_FEMALE )
		send_log( fp, LOG_CHECK, "check_mobile(%d): sesso errato: %d", pMobProto->vnum, pMobProto->sex );

	if ( pMobProto->race < 0 || pMobProto->race >= MAX_RACE )
		send_log( fp, LOG_CHECK, "check_mobile(%d): razza errata: %d", pMobProto->vnum, pMobProto->race );

	if ( pMobProto->class < 0 || pMobProto->class >= MAX_CLASS )
		send_log( fp, LOG_CHECK, "check_mobile(%d): classe errata: %d", pMobProto->vnum, pMobProto->class );

	if ( pMobProto->height < 0 || pMobProto->height > 100000 )	/* > 100m */
		send_log( fp, LOG_CHECK, "check_mobile(%d): altezza errata: %d", pMobProto->vnum, pMobProto->height );

	if ( pMobProto->weight < 0 || pMobProto->weight > 10000000 )	/* > 10q */
		send_log( fp, LOG_CHECK, "check_mobile(%d): altezza errata: %d", pMobProto->vnum, pMobProto->height );

	/* (FF) Controllo di altezza e peso sulla base razziale */

	if ( pMobProto->speaking < 0 || pMobProto->speaking >= MAX_LANGUAGE )
		send_log( fp, LOG_CHECK, "check_mobile(%d): speaking errata: %d", pMobProto->vnum, pMobProto->speaking );

#ifdef T2_ALFA
	/* Ogni mob ha un set di lingue con cui parlare (speaks), settare speaking per lo
	 *	più una razziale o il comune, fare la cosa dinamicamente in maniera che il mob
	 *	parli la lingua che il pg conosce meglio o la propria se non ne conosce */
	if ( pMobProto->speaking < 0 || pMobProto->speaking >= MAX_LANGUAGE )
		send_log( fp, LOG_CHECK, "check_mobile(%d): speaking errata: %d", pMobProto->vnum, pMobProto->speaking );
#endif

	/* (RR) test sull'hitroll, test sul damroll, numattack e altri numeri */
}


DO_RET do_check( CHAR_DATA *ch, char *argument )
{
	char	arg[MSL];
	VNUM	vnum = -1;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: check mob <vnum>\r\n" );
		send_to_char( ch, "          check mob <nome> <-(non supportato)\r\n" );
		send_to_char( ch, "          check obj <vnum>\r\n" );
		send_to_char( ch, "          check obj <nome>\r\n" );
		send_to_char( ch, "          check room <vnum>\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(argument) )
	{
		ch_printf( ch, "Quale cosa vorresti controllare con l'opzione %s?", arg );
		return;
	}

	if ( is_number(argument) )
		vnum = atoi( argument );

	if ( !str_cmp(arg, "mob") )
	{
		MOB_PROTO_DATA *pMobIndex;

		pMobIndex = get_mob_index( NULL, vnum );
		if ( !pMobIndex )
			ch_printf( ch, "Non esiste nessun mob index di vnum %d", vnum );

		check_mobile( pMobIndex, NULL );
	}
	else if ( !str_cmp(arg, "obj") )
	{
		OBJ_DATA *obj;	/* variabile sia per oggetto indicizzato che non */

		if ( vnum != -1 )
		{
			obj = get_obj_proto( NULL, vnum );
			if ( !obj )
				ch_printf( ch, "Non esiste nessun oggetto index di vnum %d", vnum );
		}
		else
		{
			obj = get_obj_world( ch, argument );
		}

		check_object( obj, NULL );
	}
	else if ( !str_cmp(arg, "room") )
	{
		ROOM_DATA *pRoom;

		pRoom = get_room_index( NULL, vnum );
		if ( !pRoom )
			ch_printf( ch, "Non esiste nessuna stanza index di vnum %d", vnum );

		check_room( pRoom, NULL );
	}
	else
	{
		send_command( ch, "check", CO );
	}
}


/*
 * DoAs command
 * Permette di eseguire un comando dal lato di un player normale
 * Se invece del nome si passa l'argomento giocatore l'admin invia il comando
 *	come un player normale.
 */
DO_RET do_doas( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA	*d;
	CHAR_DATA		*victim;
	char			 arg[MIL];

	if ( IS_MOB(ch) )
		return;

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "You must input the name of an online character.\n\r" );
		send_to_char( ch, "Per ora bisogna inviare i comandi con la lingua della vittima.\r\n" );
		return;
	}

	if ( is_name_prefix(argument, "fine quit cancella delete password") )
	{
		send_to_char( ch, "Non si può utilizzare questo comando.\r\n" );
		return;
	}

	if ( is_name(arg, "giocatore player utente user") )
	{
		int	trust;

		trust = ch->pg->trust;
		ch->pg->trust = TRUST_PLAYER;

		interpret( ch, argument );

		ch->pg->trust = trust;
		return;
	}

	victim = get_player_mud( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_pager( ch, "No such character online.\n\r" );
		return;
	}

	if ( !victim->desc )
	{
		send_to_pager( ch, "Non si può, è in idle.\r\n" );
		return;
	}

	if ( ch->desc->original || victim->desc->original )
	{
		send_to_pager( ch, "Non fino a che è switched.\r\n" );
		return;
	}

	if ( get_trust(ch) < get_trust(victim) )
	{
		send_to_char( ch, "I don't think so..\r\n" );
		return;
	}

	/* Penso che non ci sia controllo sulla trust del comando che ch invia, o evito di fare doas ad admin superiori a ch o controllo la trust del comando (che è meglio) */

	d = victim->desc;
	victim->desc = ch->desc;

	/* Invia il comando a seconda della lingua di chi lo invia */
	if ( HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
		send_command( victim, argument, IT );
	else
		send_command( victim, argument, EN );

	victim->desc = d;
	victim->desc->character = victim;
}


/*
 * This is call fremove which forces player to remove all their eq... :)
 * Written by takeda (takeda@mathlab.sunysb.edu)
 */
DO_RET do_freemove( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	OBJ_DATA	*obj_next;
	OBJ_DATA	*obj;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Syntax: fremove <char>\r\n" );
		return;
	}

	victim = get_player_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "That player is not here.\r\n" );
		return;
	}

	for ( obj = victim->first_carrying;  obj;  obj = obj_next )
	{
		obj_next = obj->next_content;

		if ( obj->wear_loc != WEARLOC_NONE )
			remove_obj( victim, obj->wear_loc, TRUE );
	}

	save_player( victim );
}


/*
 * Comando whois
 * (FF) volendo si può aggiungere un supporto per i pg offline
 */
DO_RET do_whois( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   buf[MSL];

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		send_to_pager( ch, "You must input the name of an online character.\r\n" );
		return;
	}

	victim = get_char_mud( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	score_handler( ch, victim, "" );

	/* Le informazioni sottostanti son quelle solo per pg e il mob victim non ci deve passare */
	if ( IS_MOB(victim) )
		return;

	set_char_color( AT_GREY, ch );

	ch_printf( ch, "Monete addosso: %d\r\n", victim->gold );
	show_player_accounts( ch, victim );

#ifdef T2_DETCLIENT
	if ( victim->desc && VALID_STR(victim->desc->client) )
		ch_printf( ch, "%s utilizza il Client %s\r\n", victim->name, victim->desc->client );
#endif

	if ( !VALID_STR(victim->pg->bio) )
		pager_printf( ch, "%s non ha ancora creato una biografia.\r\n", victim->name );
	else
		pager_printf( ch, "Biografia:\r\n%s", victim->pg->bio );

	if ( !VALID_STR(victim->pg->personality) )
		pager_printf( ch, "%s non ha ancora creato un proprio carattere.\r\n", victim->name );
	else
		pager_printf( ch, "Carattere:\r\n%s", victim->pg->personality );

	if ( !VALID_STR(victim->pg->objective) )
		pager_printf( ch, "%s non ha un obiettivo preciso.\r\n", victim->name );
	else
		pager_printf( ch, "Obiettivo: %s\r\n", victim->pg->objective );

	ch_printf( ch, "Output totale:           %d bytes\r\n", ch->desc->total_output );
#ifdef T2_MCCP
	ch_printf( ch, "Output totale compresso: %d bytes\r\n", ch->desc->mccp->total_output );
#endif

	if ( victim->pg->illegal_pk )
	{
		pager_printf(ch, "%s has committed %d illegal player kills.\r\n",
			victim->name, victim->pg->illegal_pk );
	}

	pager_printf( ch, "%s is %shelled at the moment.\r\n",
		victim->name, (victim->pg->release_date == 0)  ?  "not "  :  "" );

	if ( victim->pg->release_date != 0 )
	{
		pager_printf( ch, "%s was helled by %s, and will be released on %21.21s.\r\n",
			(victim->sex == SEX_FEMALE)  ?  "Ella"  :  "Egli",
			victim->pg->helled_by,
			friendly_ctime(&victim->pg->release_date) );
	}

	if ( HAS_BIT_PLR(victim, PLAYER_SILENCE)
	  || HAS_BIT_PLR(victim, PLAYER_NOEMOTE)
	  || HAS_BIT_PLR(victim, PLAYER_NOTELL)
	  || HAS_BIT_PLR(victim, PLAYER_THIEF)
	  || HAS_BIT_PLR(victim, PLAYER_KILLER) )
	{
		sprintf( buf, "This player has the following flags set:" );
		if ( HAS_BIT_PLR(victim, PLAYER_SILENCE) )		strcat( buf, " silence" );
		if ( HAS_BIT_PLR(victim, PLAYER_NOEMOTE) )		strcat( buf, " noemote" );
		if ( HAS_BIT_PLR(victim, PLAYER_NOTELL ) )		strcat( buf, " notell"  );
		if ( HAS_BIT_PLR(victim, PLAYER_THIEF  ) )		strcat( buf, " thief"	);
		if ( HAS_BIT_PLR(victim, PLAYER_KILLER ) )		strcat( buf, " killer"	);
		pager_printf( ch, "%s.\r\n", buf );
	}
}
