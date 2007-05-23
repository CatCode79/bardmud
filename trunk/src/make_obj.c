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
 >                    Modulo creazione Oggetti Specifici                    <
\****************************************************************************/


#include "mud.h"
#include "db.h"
#include "room.h"


#if 0
/*
 * Make a fire.
 * (FF) inutilizzato.
 */
void make_fire( ROOM_DATA *in_room, int timer )
{
	OBJ_DATA *fire;

	fire = make_object( get_obj_proto(NULL, VNUM_OBJ_FIRE), 0 );
	fire->timer = number_fuzzy( timer );
	obj_to_room( fire, in_room );
}
#endif


/*
 * Make a trap.
 */
OBJ_DATA *make_trap( int v0, int v1, int v2, int v3 )
{
	OBJ_DATA *obj;

	obj = make_object( get_obj_proto(NULL, VNUM_OBJ_TRAP), 0 );

	obj->timer = 0;
	obj->trap->charges	= v0;
	obj->trap->type		= v1;
	obj->trap->level	= v2;
	obj->trap->flags	= v3;

	return obj;
}


/*
 * Turn an object into scraps.
 */
void make_scraps( OBJ_DATA *obj )
{
	char		buf[MSL];
	OBJ_DATA   *scraps,
			   *tmpobj;
	CHAR_DATA  *ch = NULL;

	split_obj( obj, 1 );
	scraps	= make_object( get_obj_proto(NULL, VNUM_OBJ_SCRAPS), 0 );
	scraps->timer = number_range( 5, 15 );

	/* don't make scraps of scraps of scraps of ... */
	if ( obj->vnum == VNUM_OBJ_SCRAPS )
	{
		DISPOSE( scraps->short_descr );
		scraps->short_descr = str_dup( "some debris" );
		DISPOSE( scraps->long_descr );
		scraps->long_descr = str_dup( "Bits of debris lie on the ground here." );
	}
	else
	{
		sprintf( buf, scraps->short_descr, obj->short_descr );
		DISPOSE( scraps->short_descr );
		scraps->short_descr = str_dup( buf );
		sprintf( buf, scraps->long_descr, obj->short_descr );
		DISPOSE( scraps->long_descr );
		scraps->long_descr = str_dup( buf );
	}

	if ( obj->carried_by )
	{
		act( AT_OBJECT, "$p falls to the ground in scraps!",
			obj->carried_by, obj, NULL, TO_CHAR );

		if ( obj == get_eq_char(obj->carried_by, WEARLOC_WIELD)
		  && (tmpobj = get_eq_char(obj->carried_by, WEARLOC_DUAL)) != NULL )
		{
			tmpobj->wear_loc = WEARLOC_WIELD;
		}

		obj_to_room( scraps, obj->carried_by->in_room);
	}
	else if ( obj->in_room )
	{
		if ( (ch = obj->in_room->first_person ) != NULL )
		{
			act( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_ROOM );
			act( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_CHAR );
		}
		obj_to_room( scraps, obj->in_room);
	}
	if ( (obj->type == OBJTYPE_CONTAINER || obj->type == OBJTYPE_KEYRING
	  ||  obj->type == OBJTYPE_QUIVER || obj->type == OBJTYPE_CORPSE_PG)
	  && obj->first_content )
	{
		if ( ch && ch->in_room )
		{
			act( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_ROOM );
			act( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_CHAR );
		}

		if ( obj->carried_by )
			empty_obj( obj, NULL, obj->carried_by->in_room );
		else if ( obj->in_room )
			empty_obj( obj, NULL, obj->in_room );
		else if ( obj->in_obj )
			empty_obj( obj, obj->in_obj, NULL );
	}

	free_object( obj );
}


/*
 * Make some coinage
 */
OBJ_DATA *make_money( int amount )
{
	char buf[MSL];
	OBJ_DATA *obj;

	if ( amount <= 0 )
	{
		send_log( NULL, LOG_BUG, "make_money: zero or negative money %d.", amount );
		amount = 1;
	}

	if ( amount == 1 )
		obj = make_object( get_obj_proto(NULL, VNUM_OBJ_MONEY_ONE), 0 );
	else
	{
		obj = make_object( get_obj_proto(NULL, VNUM_OBJ_MONEY_SOME), 0 );
		sprintf( buf, obj->short_descr, amount );
		DISPOSE( obj->short_descr );
		obj->short_descr = str_dup( buf );
		obj->money->qnt = amount;
	}

	return obj;
}

/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer )
{
	OBJ_DATA		   *corpse;
	OBJ_DATA		   *obj;
	OBJ_DATA		   *obj_next;
	char			   *name_short;
	char			    name_keyword[MIL];
	char				buf[MSL];

	if ( IS_MOB(ch) )
	{
		one_argument( ch->name, name_keyword );
		name_short	= ch->short_descr;
		corpse		= make_object( get_obj_proto(NULL, VNUM_OBJ_CORPSE_NPC), 0 );
		corpse->timer	= 6;

		/* Crea i value per funzionare come un contenitore */
		init_values( corpse, OBJTYPE_CONTAINER );
		corpse->container->capacity		= can_carry_weight( ch );
		corpse->container->key			= 0;
		corpse->container->durability	= 6;
		corpse->container->type			= OBJTYPE_NONE;
		corpse->container->flags		= 0;

		if ( ch->gold > 0 )
		{
			if ( ch->in_room )
			{
				ch->in_room->area->gold_looted += ch->gold;
				mudstat.global_looted += ch->gold/100;
			}
			obj_to_obj( make_money(ch->gold), corpse );
			ch->gold = 0;
		}

/*		Cannot use these!  They are used. (FF) magari pensarci se usarlo o meno
		corpse->value[0] = (int) ch->pIndexData->vnum;
*/

		/*	Using corpse cost to cheat, since corpses not sellable (FF) bleah... fare una variabile apposita */
		corpse->cost = ( -(int) ch->pIndexData->vnum );
		corpse->corpse->time = corpse->timer;

		corpse->corpse->pg = FALSE;
	}
	else
	{
		strcpy( name_keyword, ch->name );
		name_short = ch->name;
		corpse	= make_object( get_obj_proto(NULL, VNUM_OBJ_CORPSE_PC), 0 );

		/* Crea i value per funzionare come un contenitore */
		init_values( corpse, OBJTYPE_CONTAINER );
		corpse->container->capacity		= can_carry_weight( ch );
		corpse->container->key			= 0;
		corpse->container->durability	= 6;
		corpse->container->type			= OBJTYPE_NONE;
		corpse->container->flags		= 0;

#ifdef T2_ARENA
		if ( in_arena(ch) )
			corpse->timer = 0;
		else
#endif
			corpse->timer = 40;

		corpse->corpse->time = (int) ( corpse->timer/8 );
		corpse->corpse->level = get_level( ch );	/* Niente livello /2 qui */

		if ( can_pkill(ch, NULL) && sysdata.pk_loot )
			SET_BIT( corpse->extra_flags, OBJFLAG_CLANCORPSE );

		/* Pkill corpses get save timers, in ticks (approx 70 seconds)
		   This should be anough for the killer to type 'get all corpse'. */
		if ( IS_PG(ch) && IS_PG(killer) )
			corpse->corpse->tick = 1;
		else
			corpse->corpse->tick = 0;

		corpse->corpse->pg = TRUE;
	}

	if ( can_pkill(ch, NULL) && can_pkill(killer, NULL) && ch != killer )
	{
		strcpy( buf, killer->name );
		DISPOSE( corpse->action_descr );
		corpse->action_descr = str_dup( buf );
	}

	/* Added corpse name - make locate easier , other skills */
	sprintf( buf, "%s cadavere corpo resti putrefatti", name_keyword );
	DISPOSE( corpse->name );
	corpse->name = str_dup( buf );

	sprintf( buf, corpse->short_descr, name_short );
	DISPOSE( corpse->short_descr );
	corpse->short_descr = str_dup( buf );

	sprintf( buf, corpse->long_descr, name_short );
	DISPOSE( corpse->long_descr );
	corpse->long_descr = str_dup( buf );

	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
		obj_next = obj->next_content;

		obj_from_char( obj );
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVENTORY)
		  || HAS_BIT(obj->extra_flags, OBJFLAG_DEATHROT) )
			free_object( obj );
		else
			obj_to_obj( obj, corpse );
	}

	obj_to_room( corpse, ch->in_room );
}


void make_blood( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj				= make_object( get_obj_proto(NULL, VNUM_OBJ_BLOOD), 0 );

	obj->timer				  = number_range( 2, 4 );
	obj->drinkcon->curr_quant = number_range( 3, UMIN(5, get_level(ch)/2) );

	obj_to_room( obj, ch->in_room );
}


void make_bloodstain( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj			= make_object( get_obj_proto(NULL, VNUM_OBJ_BLOODSTAIN), 0 );
	obj->timer	= number_range( 1, 2 );
	obj_to_room( obj, ch->in_room );
}
