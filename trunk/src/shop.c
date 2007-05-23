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
 >								Modulo dei negozi							<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "bank.h"
#include "build.h"
#include "calendar.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "fread.h"
#include "group.h"
#include "interpret.h"
#include "magic.h"
#include "mprog.h"
#include "mxp.h"
#include "room.h"
#include "shop.h"


/*
 * Variabili esterne
 */
SHOP_DATA		*first_shop;
SHOP_DATA		*last_shop;

REPAIR_DATA		*first_repair;
REPAIR_DATA		*last_repair;

int				 top_shop;
int				 top_repair;	/* (CC) da inglobare con top_shop */


/*
 * Load a shop section.
 */
void load_shops( MUD_FILE *fp )
{
	SHOP_DATA *pShop;

	for ( ; ; )
	{
		MOB_PROTO_DATA *pMobIndex;
		int iTrade;

		CREATE( pShop, SHOP_DATA, 1 );
		pShop->keeper		= fread_number( fp );
		if ( pShop->keeper == 0 )
		{
			DISPOSE( pShop );
			break;
		}

		for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
			pShop->buy_type[iTrade]	= fread_number( fp );

		pShop->profit_buy	= fread_number( fp );
		pShop->profit_sell	= fread_number( fp );
		pShop->profit_buy	= URANGE( pShop->profit_sell+5, pShop->profit_buy, 500 );
		pShop->profit_sell	= URANGE( 5, pShop->profit_sell, pShop->profit_buy-5 );
		pShop->open_hour	= fread_number( fp );
		pShop->close_hour	= fread_number( fp );
							  fread_to_eol( fp );	/* Scarta così i vari commenti spesso presenti */

		pMobIndex			= get_mob_index( fp, pShop->keeper );
		pMobIndex->pShop	= pShop;

		if ( !first_shop )
			first_shop		= pShop;
		else
			last_shop->next	= pShop;

		pShop->next			= NULL;
		pShop->prev			= last_shop;
		last_shop			= pShop;
		top_shop++;
	}
}


/*
 * Load a repair shop section.
 */
void load_repairs( MUD_FILE *fp )
{
	REPAIR_DATA *rShop;

	for ( ; ; )
	{
		MOB_PROTO_DATA *pMobIndex;
		int iFix;

		CREATE( rShop, REPAIR_DATA, 1 );
		rShop->keeper		= fread_number( fp );

		if ( rShop->keeper == 0 )
		{
			DISPOSE( rShop );
			break;
		}

		for ( iFix = 0; iFix < MAX_FIX; iFix++ )
			rShop->fix_type[iFix] = fread_number( fp );

		rShop->profit_fix	= fread_number( fp );
		rShop->shop_type	= fread_number( fp );
		rShop->open_hour	= fread_number( fp );
		rShop->close_hour	= fread_number( fp );
							  fread_to_eol( fp );
		pMobIndex			= get_mob_index( fp, rShop->keeper );
		pMobIndex->rShop	= rShop;

		if ( !first_repair )
			first_repair		= rShop;
		else
			last_repair->next	= rShop;

		rShop->next		= NULL;
		rShop->prev		= last_repair;
		last_repair		= rShop;
		top_repair++;
	}
}


/*
 * Libera dalla memoria i negozi
 */
void free_all_shops( void )
{
	SHOP_DATA *shop;

	for ( shop = first_shop;  shop;  shop = first_shop )
	{
		UNLINK( shop, first_shop, last_shop, next, prev );
		top_shop--;

		DISPOSE( shop );
	}

	if ( top_shop != 0 )
		send_log( NULL, LOG_BUG, "free_all_shops: top_shop non è 0 dopo aver liberato i negozi: %d", top_shop );
}

/*
 * Libera tutti i negozi di riparazione
 */
void free_all_repairs( void )
{
	REPAIR_DATA *repair;

	for ( repair = first_repair;  repair;  repair = first_repair )
	{
		UNLINK( repair, first_repair, last_repair, next, prev );
		top_repair--;

		DISPOSE( repair );
	}

	if ( top_repair != 0 )
		send_log( NULL, LOG_BUG, "free_all_repairs: top_repair non è 0 dopo aver liberato i negozi di riparazione: %d", top_repair );
}


/*
 * Shopping commands
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch, const char *action )
{
	CHAR_DATA	*keeper;
	CHAR_DATA	*whof;
	SHOP_DATA	*pShop = NULL;
	REPAIR_DATA *rShop = NULL;
	char		 buf[MSL];
	int			 speakswell;

	for ( keeper = ch->in_room->first_person;  keeper;  keeper = keeper->next_in_room )
	{
		if ( IS_PG(keeper) )
			continue;

		if ( !str_cmp(action, "comprare") && (pShop = keeper->pIndexData->pShop) )
			break;
		if ( !str_cmp(action, "riparare") && (rShop = keeper->pIndexData->rShop) )
			break;
	}

	if ( keeper == NULL
	  || ((!str_cmp(action, "comprare") && pShop == NULL)
	  &&  (!str_cmp(action, "riparare") && rShop == NULL)) )
	{
		ch_printf( ch, "Non sono in un negozio in cui possa %s qualcosa.\r\n", action );
		return NULL;
	}

	/*
	 * Undesirables.
	 */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_KILLER) )
	{
		sprintf( buf, "say a %s Gli assassini non sono i benvenuti qui!", ch->name );
		send_command( keeper, buf, CO );
		send_command( keeper, "yell Guardie! Guardie!", CO );
		return NULL;
	}

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_THIEF) )
	{
		sprintf( buf, "say a %s I ladri non sono i benvenuti qui!", ch->name );
		send_command( keeper, buf, CO );
		send_command( keeper, "yell Guardie! Guardie!", CO );
		return NULL;
	}

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ATTACKER) )
	{
		sprintf( buf, "say a %s Le persone violente non sono le benvenute qui!", ch->name );
		send_command( keeper, buf, CO );
		send_command( keeper, "yell Guardie! Guardie! Un VIULENTO è qui!\r\n", CO );
		return NULL;
	}

	/*
	 * Disallow sales during battle
	 */
	whof = who_fighting( keeper );
	if ( whof )
	{
		if ( whof == ch )
		{
			sprintf( buf, "say a %s Mi affronti e vorresti i miei servizi?!", ch->name );
			send_command( keeper, buf, CO );
		}
		else
		{
			sprintf( buf, "say a %s Lasciami prima finire questo duello!", ch->name );
			send_command( keeper, buf, CO );
		}
		return NULL;
	}

	if ( who_fighting(ch) )
	{
		send_to_char( ch, "Devo prima terminare questo combattimento.\r\n" );
		return NULL;
	}

	/*
	 * Check to see if show is open
	 * Supports closing times after midnight
	 */
	if ( (!str_cmp(action, "comprare") && pShop && pShop->open_hour > pShop->close_hour)
	  || (!str_cmp(action, "riparare") && rShop && rShop->open_hour > rShop->close_hour) )
	{
		if ( (!str_cmp(action, "comprare") && pShop && calendar.hour < pShop->open_hour && calendar.hour > pShop->close_hour)
		  || (!str_cmp(action, "riparare") && rShop && calendar.hour < rShop->open_hour && calendar.hour > rShop->close_hour) )
		{
			sprintf( buf, "say a %s Spiacente, il negozio è chiuso. Provate più tardi!", ch->name );
			send_command( keeper, buf, CO );
			return NULL;
		}
	}
	else
	{
		if ( (!str_cmp(action, "comprare") && pShop && calendar.hour < pShop->open_hour)
		  || (!str_cmp(action, "riparare") && rShop && calendar.hour < rShop->open_hour) )
		{
			sprintf( buf, "say a %s Spiacente, il negozio è chiuso. Provate più tardi!", ch->name );
			send_command( keeper, buf, CO );
			return NULL;
		}

		if ( (!str_cmp(action, "comprare") && pShop && calendar.hour > pShop->close_hour)
		  || (!str_cmp(action, "riparare") && rShop && calendar.hour > rShop->close_hour) )
		{
			sprintf( buf, "say a %s Spiacente, il negozio è chiuso. Tornate domani!", ch->name );
			send_command( keeper, buf, CO );
			return NULL;
		}
	}

	if ( keeper->position == POSITION_SLEEP )
	{
		send_to_char( ch, "Dovrei contrattare con un dormiente?\r\n" );
		return NULL;
	}

	if ( keeper->position < POSITION_SLEEP )
	{
		send_to_char( ch, "Non credo che riesca a sentirmi..\r\n" );
		return NULL;
	}

	/*
	 * Invisible or hidden people.
	 */
	if ( !can_see(keeper, ch) )
	{
		sprintf( buf, "say Eh? Come? Fatevi vedere, prima di tutto!" );
		send_command( keeper, buf, CO );
		return NULL;
	}

	speakswell = UMIN( knows_language(keeper, ch->speaking, ch),
					   knows_language(ch, ch->speaking, keeper) );

	if ( (number_percent() % 65) > speakswell )
	{
		char buf2[MSL];

		if		( speakswell > 60 )
			sprintf( buf2, "Puoi ripeterlo.. più lentamente? Non ho capito bene.." );
		else if ( speakswell > 50 )
			sprintf( buf2, "Puoi ripetere per favore? Non ho capito l'ultima parola.." );
		else if ( speakswell > 40 )
			sprintf( buf2, "Come?! Non ho capito una parola!" );
		else
			sprintf( buf2, "Ma che lingua parlate? Non capisco nulla!" );

		sprintf( buf, "say a %s %s", ch->name, buf2 );
		send_command( keeper, buf, CO );
		return NULL;
	}

	return keeper;
}


int get_cost( CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
	SHOP_DATA	*pShop;
	int			 cost;
	bool		 richcustomer;
	int			 profitmod;

	if ( !obj )
		return 0;

	pShop = keeper->pIndexData->pShop;
	if ( !pShop )
		return 0;

	if ( ch->gold > (get_level(ch)/2 * get_level(ch)/2 * 100000) )
		richcustomer = TRUE;
	else
		richcustomer = FALSE;

	if ( fBuy )
	{
		profitmod = 13 - ( get_curr_attr(ch, ATTR_EMP)/4 + get_curr_attr(ch, ATTR_BEA)/4) + (richcustomer)  ?  15  :  0
			+ ((URANGE(5, get_level(ch)/2, LVL_LEGEND/2)-20)/2 );

		cost = (int) (obj->cost
			* UMAX( (pShop->profit_sell+1), (pShop->profit_buy+profitmod)) )
			/ 100;

		cost = (int) (cost * (80 + UMIN(get_level(ch)/2, LVL_LEGEND/2))) / 100;
		cost = compute_cost( ch, keeper, cost );	/* calcola il costo sulla base delle azioni ed altri fattori */
	}
	else
	{
		OBJ_DATA *obj2;
		int itype;

		/* Se è un oggetto che keeper vende ritorna zero */
		for ( obj2 = keeper->first_carrying;  obj2;  obj2 = obj2->next_content )
		{
			if ( obj->pObjProto == obj2->pObjProto )
				return 0;
		}

		profitmod = (get_curr_attr(ch, ATTR_EMP)/4 + get_curr_attr(ch, ATTR_BEA)/4) - 13 - (richcustomer)  ?  15  :  0;

		cost = 0;
		for ( itype = 0;  itype < MAX_TRADE;  itype++ )
		{
			if ( obj->type == pShop->buy_type[itype] )
			{
				cost = (int) (obj->cost
					* UMIN( (pShop->profit_buy-1),
					(pShop->profit_sell+profitmod)) ) / 100;
				break;
			}
		}

		if ( cost > 0 )
			cost = compute_cost( ch, keeper, cost );	/* calcola il costo sulla base dell'andamento del mercato e altri fattori */
	}

	/* Fix to stop crashses when selling bad wands/staves */
	if ( obj->type == OBJTYPE_STAFF || obj->type == OBJTYPE_WAND )
	{
		if ( obj->wand->max_charges == 0 )
			return 0;

		cost = (int) ( cost * obj->wand->curr_charges / obj->wand->max_charges );
	}

	return cost;
}


int get_repaircost( CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj )
{
	REPAIR_DATA *rShop;
	int			 cost;
	int			 itype;
	bool		 found;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_repaircost: ch è NULL" );
		return 0;
	}

	if ( !keeper )
	{
		send_log( NULL, LOG_BUG, "get_repaircost: keeper è NULL" );
		return 0;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "get_repaircost: obj è NULL" );
		return 0;
	}

	rShop = keeper->pIndexData->rShop;
	if ( !rShop )
		return 0;

	cost = 0;
	found = FALSE;
	for ( itype = 0; itype < MAX_FIX; itype++ )
	{
		if ( obj->type == rShop->fix_type[itype] )
		{
			cost = (int) (obj->cost * rShop->profit_fix / 1000);
			found = TRUE;
			break;
		}
	}

	if ( found == FALSE )
		cost = -1;

	if ( cost == 0 )
		cost = 1;

	if ( found && cost > 0 )
	{
		switch (obj->type)
		{
		  case OBJTYPE_ARMOR:
		  case OBJTYPE_DRESS:
			if ( obj->armor->ac >= obj->armor->ac_orig )
				cost = -2;
			else
				cost *= ( obj->armor->ac_orig - obj->armor->ac );
			break;

		  case OBJTYPE_WEAPON:
			if ( obj->weapon->condition == INIT_WEAPON_CONDITION )
				cost = -2;
			else
				cost *= ( INIT_WEAPON_CONDITION - obj->weapon->condition );
			break;

		  case OBJTYPE_WAND:
		  case OBJTYPE_STAFF:
			if ( obj->wand->curr_charges >= obj->wand->max_charges )
				cost = -2;
			else
				cost *= ( obj->wand->curr_charges - obj->wand->curr_charges );
		}
	}

	/* Se le azioni vanno male aumenta, se vanno bene sconta */
	if ( cost > 1 )
	{
		switch ( get_share_band() )
		{
		  case 1:	(int) ( cost *= 1.02 );		break;
		  case 2:	(int) ( cost *= 1.01 );		break;
		  case 4:	(int) ( cost /= 1.01 );		break;
		  case 5:	(int) ( cost /= 1.02 );		break;
		}
	}

	/* Sconto di 4% se il negoziante e il pg hanno la stessa razza */
	if ( cost > 1 && ch->race == keeper->race )
		(int) ( cost /= 1.04 );

	/* (FF) Aggiungerci anche un aumento o altro per le razze che i pg odiano,
	 *	magari non li servono proprio e poi la skill haggle */

	return cost;
}


DO_RET do_buy( CHAR_DATA *ch, char *argument )
{
	int	maxgold;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Cosa dovrei comprare?\r\n" );
		return;
	}

	if ( HAS_BIT(ch->in_room->flags, ROOM_MASCOTTESHOP) )
	{
		CHAR_DATA	*mascotte;
		ROOM_DATA	*pRoomNext;
		ROOM_DATA	*in_room;
		char		 buf[MSL];

		if ( IS_MOB(ch) )
		{
			send_to_char( ch, "I mob non possono comprare cuccioli.\r\n" );
			return;
		}

		pRoomNext = get_room_index( NULL, ch->in_room->vnum+1 );
		if ( !pRoomNext )
		{
			send_log( NULL, LOG_BUG, "do_buy: mascotte shop errato al vnum %d.", ch->in_room->vnum );

			send_to_char( ch, "Non è un articolo qui in vendita.\r\n" );
/*			sprintf( buf, "say a %s Non è un articolo qui in vendita.", ch->name );
			send_command( keeper, buf, CO ); */
			return;
		}

		if ( ch->pg->mascotte )
		{
			send_to_char( ch, "Posseggo già un cucciolo con me.\r\n" );
			return;
		}

		in_room 	= ch->in_room;
		ch->in_room = pRoomNext;
		mascotte 		= get_mob_room( ch, argument, TRUE );
		ch->in_room = in_room;

		if ( !mascotte || !HAS_BIT_ACT(mascotte, MOB_MASCOTTE) )
		{
			send_to_char( ch, "Spiacente, non è un articolo qui in vendita.\r\n" );
/*			sprintf( buf, "say a %s Spiacente, non è un articolo qui in vendita.", ch->name );
			send_command( keeper, buf, CO );*/
			return;
		}

		if ( ch->gold < 10 * get_level(mascotte)/2 * get_level(mascotte)/2 )
		{
			send_to_char( ch, "Mi mancano un po' di monete!\r\n" );
			return;
		}

		/* (FF) Lo può comprare, ma solo con la skill di addestramento è alta, oppure lo compra e poi son cazzi suoi */
		if ( get_level(ch) < get_level(mascotte) )
		{
			send_to_char( ch, "Non mi sembri pronto per tenerlo a bada.\r\n" );
/*			sprintf( buf, "say a %s Non mi sembri pronto per tenerlo a bada.", ch->name );
			send_command( keeper, buf, CO ); */
			return;
		}

		if ( !can_charm(ch) )
		{
			send_to_char( ch, "Non posso portarmi dietro questo cucciolo, ho troppo poco Comando.\r\n" );
			return;
		}

		maxgold = 10 * get_level(mascotte)/2 * get_level(mascotte)/2;
		ch->gold = maxgold;
		boost_economy( ch->in_room->area, maxgold );

		mascotte = make_mobile( mascotte->pIndexData );

		SET_BIT( mascotte->act, MOB_MASCOTTE );
		SET_BIT( mascotte->affected_by, AFFECT_CHARM );
/*		This isn't needed anymore since you can order your mascottes
		SET_BIT( mascotte->affected_by, AFFECT_CHARM );
*/
		if ( VALID_STR(argument) )
		{
			sprintf( buf, "%s %s", mascotte->name, argument );
			DISPOSE( mascotte->name );
			mascotte->name = str_dup( buf );
		}

		sprintf( buf, "%s Una targhetta sul collo reca l'iscrizione 'Appartengo a %s'.\r\n", mascotte->description, ch->name );
		DISPOSE( mascotte->description );
		mascotte->description = str_dup( buf );

		char_to_room( mascotte, ch->in_room );
		add_follower( mascotte, ch );

		send_to_char( ch, "Ecco il tuo cucciolo!\r\n" );
/*		sprintf( buf, "say a %s Ecco il tuo cucciolo!", ch->name );
		send_command( keeper, buf, CO );*/
		act( AT_ACTION, "$n compra $N.", ch, NULL, mascotte, TO_ROOM );

		return;
	}
	else
	{
		CHAR_DATA *keeper;
		OBJ_DATA  *obj;
		char	   buf[MSL];
		char	   arg[MIL];
		int		   cost;
		int		   noi  = 1;	/* Number of items */
		int		   mnoi = 10;	/* Max number of items to be bought at once */

		keeper = find_keeper( ch, "comprare" );
		if ( !keeper )
			return;

		maxgold = get_level(keeper)/2 * get_level(keeper)/2 * 50000;

		argument = one_argument( argument, arg );
		if ( is_number(arg) )
		{
			noi = atoi( arg );

			if ( !VALID_STR(argument) )
			{
				sprintf( buf, "say a %s Quale cosa vorresti vendere in quantità di %d?", ch->name, noi );
				send_command( keeper, buf, CO );
				return;
			}

			if ( noi > mnoi )
			{
				sprintf( buf, "say a %s Non te ne posso vendere in quantità così elevate in una volta sola!", ch->name );
				send_command( keeper, buf, CO );
				return;
			}

			/* Se c'era il numero cerca l'oggetto nella stringa argument */
			obj = get_obj_carry( keeper, argument, TRUE );
		}
		else
			obj = get_obj_carry( keeper, arg, TRUE );	/* Se non c'era il numero cerca l'oggetto nella stringa arg */

		if ( !obj )
		{
			sprintf( buf, "say a %s Non ho nulla di simile, consulta la lista.", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		cost = get_cost( ch, keeper, obj, TRUE ) * noi;
		if ( cost <= 0 )
		{
			sprintf( buf, "say a %s Oh scusate, temo che non possa vendere quest'oggetto, ormai non ha più valore.", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		if ( !can_see_obj(ch, obj, FALSE) )	/* FALSE perché se non lo può vedere che compra a fare? */
		{
			sprintf( buf, "say a %s Non lo potete vedere? Peccato, sarebbe stato un eventuale ottimo acquisto.", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		if ( !HAS_BIT(obj->extra_flags, OBJFLAG_INVENTORY) && noi > 1 )
		{
			sprintf( buf, "say a %s Mi dispiace.. non ne ho abbastanza in magazzino da venderne più di una alla volta!", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		if ( get_level(ch) < obj->level )
		{
			sprintf( buf, "say a %s Non mi sembri esperto abbastanza per poterlo utilizzare.", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		if ( ch->gold < cost )
		{
			sprintf( buf, "say a %s Eh si, ma vi manca un po' di moneta per comprare %s, giusto?", ch->name, obj->short_descr );
			send_command( keeper, buf, CO );
			return;
		}

		if ( ch->carry_number + obj->count > can_carry_number(ch) )
		{
			send_to_char( ch, "Non posso portare così tanti oggetti.\r\n" );
			return;
		}

		if ( ch->carry_weight + (get_obj_weight(obj) * noi)
			+ (noi > 1  ?  2  :  0) > can_carry_weight(ch) )
		{
			send_to_char( ch, "Non posso portare così tanti oggetti!\r\n" );
			return;
		}

		if ( noi == 1 )
		{
			act( AT_ACTION, "$n compra $p.", ch, obj, NULL, TO_ROOM );
			sprintf( buf, "Compro $p per %d monet%c d'oro.", cost, (cost == 1) ? 'a' : 'e' );
			act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
		}
		else
		{
			sprintf( arg, "$n compra %d $p.", noi );
			act( AT_ACTION, arg, ch, obj, NULL, TO_ROOM );
			sprintf( arg, "Compro %d $p per %d monet%c d'oro.", noi, cost, (cost == 1) ? 'a' : 'e' );
			act( AT_ACTION, arg, ch, obj, NULL, TO_CHAR );
			act( AT_ACTION, "$N li mette in un borsello e me li porge.", ch, NULL, keeper, TO_CHAR );
		}

		ch->gold	 -= cost;
		keeper->gold += cost;

		if ( keeper->gold > maxgold )
		{
			boost_economy( keeper->in_room->area, keeper->gold - maxgold/2 );
			keeper->gold = maxgold/2;
			act( AT_ACTION, "$n mette qualche moneta in un sacchettino.", keeper, NULL, NULL, TO_ROOM );
		}

		if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVENTORY) )
		{
			OBJ_DATA *buy_obj;

			buy_obj = make_object( obj->pObjProto, obj->level );
			REMOVE_BIT( buy_obj->extra_flags, OBJFLAG_INVENTORY );

			/*
			 * Due to grouped objects and carry limitations in SMAUG
			 * The shopkeeper gives you a bag with multiple-buy,
			 * and also, only one object needs be created with a count
			 * set to the number bought.
			 */
			if ( noi > 1 )
			{
				OBJ_DATA *bag;

				bag = make_object( get_obj_proto(NULL, VNUM_OBJ_SHOPPING_BAG), 1 );
				SET_BIT( bag->extra_flags, OBJFLAG_GROUNDROT );
				bag->timer = 10;
				/* perfect size bag ;) */
				bag->container->capacity = bag->weight + (buy_obj->weight * noi);
				buy_obj->count = noi;
				obj->pObjProto->count += (noi - 1);
				objects_loaded += (noi - 1);
				obj_to_obj( buy_obj, bag );
				obj_to_char( bag, ch );
			}
			else
				obj_to_char( buy_obj, ch );
		}
		else
		{
			obj_from_char( obj );
			obj_to_char( obj, ch );
		}
		return;
	} /* chiude l'else */
}


/*
 * This code relies heavily on the items held by the shopkeeper
 *	being sorted in descending order by level.
 * obj_to_char in handler.c was modified to achieve this.
 */
DO_RET do_list( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];
	char	buf_lvl[MIL];
	char	buf_vnum[MIL];
	bool	found_lowlev = FALSE;	/* indica se ha trovato qualcosa in vendita di livello minore al quello di ch */
	bool	found_upplev = FALSE;	/* indica se ha trovato qualcosa in vendita di livello maggiore a quello di ch */

	if ( HAS_BIT(ch->in_room->flags, ROOM_MASCOTTESHOP) )
	{
		ROOM_DATA	*pRoomNext;
		CHAR_DATA	*mascotte;

		/* Questo check secondo me è orrendo, da rifare con qualcos'altra cosa (FF) */
		pRoomNext = get_room_index( NULL, ch->in_room->vnum+1 );
		if ( !pRoomNext )
		{
			send_log( NULL, LOG_BUG, "do_list: mascotte shop errato at vnum %d.", ch->in_room->vnum );
			send_to_char( ch, "Non posso farlo qui.\r\n" );
			return;
		}

		/* (FF) non se po' fa perché non punta al keeper */
/*		if ( IS_ADMIN(ch) )
		{
			pager_printf( ch, "\r\n[ADM] profit buy: %d  profit sell: %s  apre alle: %d  chiude alle: %d\r\n",
				ch->in_room->xxx->profit_buy, ch->in_room->xxx->profit_sell,
				ch->in_room->xxx->open_hour, ch->in_room->xxx->close_hour );
		}
*/

		/* Indica un aumento o una diminuizione di prezzi a seconda dell'economia */
		switch ( get_share_band() )
		{
		  case 1:
			send_to_char( ch, "Purtroppo hanno aumentato i prezzi, l'economia sta andando male\r\n" );
			break;
		  case 5:
			send_to_char( ch, "Hanno diminuito i prezzi di questi cuccioli, l'economia è propizia.\r\n" );
			break;
		}


		for ( mascotte = pRoomNext->first_person;  mascotte;  mascotte = mascotte->next_in_room )
		{
			if ( IS_PG(mascotte) )						continue;
			if ( !HAS_BIT_ACT(mascotte, MOB_MASCOTTE) )	continue;

			/* Suddivide la lista in due parti, le creature sotto il livello del pg e le creature sopra al livello del pg */
			if ( !found_lowlev || !found_upplev )
			{
				if ( !found_upplev && get_level(ch) >= get_level(mascotte) )
				{
					send_to_pager( ch, "\r\nHo abbastanza esperienza per:\r\n" );
					if ( IS_ADMIN(ch) )
						send_to_pager( ch, "Liv Prezzo Creatura\r\n" );
					else
						send_to_pager( ch, "    Prezzo Creatura\r\n" );
					found_upplev = TRUE;
				}
				if ( !found_lowlev && get_level(ch) < get_level(mascotte) )
				{
					send_to_pager( ch, "Non ho abbastanza esperienza per:\r\n" );
					if ( IS_ADMIN(ch) )
						send_to_pager( ch, "Liv Prezzo Creatura\r\n" );
					else
						send_to_pager( ch, "    Prezzo Creatura\r\n" );
					found_lowlev = TRUE;
				}
			}

			buf_lvl[0] = '\0';
			if ( IS_ADMIN(ch) )
				sprintf( buf_lvl, "%3d", get_level(mascotte) );

			pager_printf( ch, "%3s %6d %s\r\n",
				buf_lvl,
				get_level(mascotte) * get_level(mascotte) * 11,
				mascotte->short_descr );
		}

		if ( found_lowlev == FALSE && found_upplev == FALSE )
			send_to_char( ch, "Stiamo aspettando che ci arrivino i nuovi esemplari dall'estero.\r\n" );

		return;
	}
	else
	{
		CHAR_DATA	*keeper;
		OBJ_DATA	*obj;
		int			 cost;

		if ( (keeper = find_keeper(ch, "comprare")) == NULL )
			return;

		if ( IS_ADMIN(ch) )
		{
			pager_printf( ch, "\r\n[ADM] profit buy: %d  profit sell: %d  apre alle: %d  chiude alle: %d\r\n",
				keeper->pIndexData->pShop->profit_buy, keeper->pIndexData->pShop->profit_sell,
				keeper->pIndexData->pShop->open_hour, keeper->pIndexData->pShop->close_hour );
		}

		buf[0] = '\0';
		/* Indica un aumento o una diminuizione di prezzi a seconda dell'economia */
		switch ( get_share_band() )
		{
		  case 1:
			sprintf( buf, "say a %s Purtroppo sono stato obbligato ad aumentare i prezzi, l'economia sta andando male", ch->name );
			break;
		  case 5:
			sprintf( buf, "say a %s Ho diminuito i prezzi della mia merce, l'economia è propizia.", ch->name );
			break;
		}
		if ( VALID_STR(buf) )
			send_command( keeper, buf, CO );

		/* Loop until you see an object higher level than char
		   Note that this depends on the keeper having a sorted list */
		for ( obj = keeper->first_carrying;  obj;  obj = obj->next_content )
		{
			if ( VALID_STR(argument) && !nifty_is_name(argument, obj->name) )
				continue;
			if ( obj->wear_loc != WEARLOC_NONE )					continue;
			if ( !can_see_obj(ch, obj, FALSE) )						continue;
			if ( (cost = get_cost(ch, keeper, obj, TRUE)) <= 0 )	continue;

			/* Suddivide la lista in due parti, gli oggetti sotto il livello del pg e gli oggetti sopra al livello del pg */
			if ( !found_lowlev || !found_upplev )
			{
				if ( !found_upplev && get_level(ch) >= obj->level )
				{
					send_to_pager( ch, "\r\nHo abbastanza esperienza per:\r\n" );
					if ( IS_ADMIN(ch) )
						send_to_pager( ch, "Liv Prezzo Oggetto\r\n" );
					else
						send_to_pager( ch, "    Prezzo Oggetto\r\n" );
					found_upplev = TRUE;
				}
				if ( !found_lowlev && get_level(ch) < obj->level )
				{
					send_to_pager( ch, "Non ho abbastanza esperienza per:\r\n" );
					if ( IS_ADMIN(ch) )
						send_to_pager( ch, "Liv Prezzo Oggetto\r\n" );
					else
						send_to_pager( ch, "    Prezzo Oggetto\r\n" );
					found_lowlev = TRUE;
				}
			}

			if ( IS_ADMIN(ch) )
			{
				sprintf( buf_lvl, "%3d", obj->level );
				if ( HAS_BIT_PLR(ch, PLAYER_VNUM) )
					sprintf( buf_vnum, " (#%d)", obj->pObjProto->vnum );
			}
			else
			{
				buf_lvl[0] = '\0';
				buf_vnum[0] = '\0';
			}
#ifdef T2_MXP
			pager_printf( ch, "%3s %6d " MXPTAG("lst \"%s\" \"%s\"") "%s" MXPTAG("/lst") ".%s\r\n",
				buf_lvl,
				cost,
				obj->name,
				obj->short_descr,
				capitalize(obj->short_descr),
				buf_vnum );
#else
			pager_printf( ch, "%3s %6d %s.%s\r\n",
				buf_lvl,
				cost,
				capitalize(obj->short_descr),
				buf_vnum );
#endif
		}

		if ( !found_lowlev && !found_upplev )
		{
			if ( !VALID_STR(argument) )
				send_to_char( ch, "Non posso comprare nulla qui.\r\n" );
			else
				send_to_char( ch, "Non posso comprarlo qui.\r\n" );
		}
	} /* chiude l'else */
}


DO_RET do_sell( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *keeper;
	OBJ_DATA   *obj;
	char		buf[MSL];
	int			cost;

	keeper = find_keeper( ch, "comprare" );
	if ( !keeper )
		return;

	if ( !VALID_STR(argument) )
	{
		sprintf( buf, "say a %s Cosa desideri vendere?", ch->name );
		send_command( keeper, buf, CO );
		return;
	}


	obj = get_obj_carry( ch, argument, TRUE );
	if ( !obj )
	{
		sprintf( buf, "say a %s Cosa? Ma l'avete con voi?", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	if ( !can_see_obj(keeper, obj, FALSE) )
	{
		sprintf( buf, "say a %s Eh? Umh.. non riesco a vedere nulla del genere.", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	if ( !can_drop_obj(ch, obj) )
	{
		send_to_char( ch, "Accidenti! Non riesco a liberarmi dell'oggetto!\r\n" );
		return;
	}

	if ( obj->timer > 0 )
	{
		sprintf( buf, "say a %s %s si svaluta molto in fretta.", ch->name, obj->short_descr );
		send_command( keeper, buf, CO );
		return;
	}

	cost = get_cost( ch, keeper, obj, FALSE );
	if ( cost <= 0 )
	{
		act( AT_ACTION, "$n guarda senza interesse $p.", keeper, obj, ch, TO_VICT );
		return;
	}

	if ( cost >= keeper->gold )
	{
		sprintf( buf, "say a %s Oh meraviglia! %s però vale più di quanto mi possa permettere.", ch->name, obj->short_descr );
		send_command( keeper, buf, CO );
		return;
	}

	split_obj( obj, 1 );
	act( AT_ACTION, "$n vende $p.", ch, obj, NULL, TO_ROOM );
	sprintf( buf, "Vendo $p per %d monet%c d'oro.", cost, (cost == 1)  ?  'a'  :  'e' );
	act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
	ch->gold += cost;
	keeper->gold -= cost;
	if ( keeper->gold < 0 )
		keeper->gold = 0;

	if ( obj->type == OBJTYPE_TRASH )
		free_object( obj );
	else
	{
		obj_from_char( obj );
		obj_to_char( obj, keeper );
	}
}


DO_RET do_value( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*keeper;
	OBJ_DATA	*obj;
	char		 buf[MSL];
	int			 cost;

	keeper = find_keeper( ch, "comprare" );
	if ( !keeper )
		return;

	if ( !VALID_STR(argument) )
	{
		sprintf( buf, "say a %s Cosa vorresti valutare?", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	obj = get_obj_carry( ch, argument, TRUE );
	if ( !obj )
	{
		sprintf( buf, "say a %s Avete con voi quell'oggetto?", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	if ( !can_see_obj(keeper, obj, FALSE) )
	{
		sprintf( buf, "say a %s Eh? Umh.. non riesco a vedere nulla del genere.", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	if ( !can_drop_obj(ch, obj) )
	{
		ch_printf( ch, "Accidenti! Non riesco a liberarmi di %s!\r\n", obj->short_descr );
		return;
	}

	cost = get_cost( ch, keeper, obj, FALSE );
	if ( cost <= 0 )
	{
		act( AT_ACTION, "$n guarda senza interesse $p.", keeper, obj, ch, TO_VICT );
		return;
	}

	sprintf( buf, "say a %s Umh posso darti %d monete d'oro per %s.", ch->name, cost, obj->short_descr );
	send_command( keeper, buf, CO );
}


/*
 * Repair a single object. Used when handling "repair all"
 */
void repair_one_obj( CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, char *arg, int maxgold, char *fixstr, char*fixstr2 )
{
	char buf[MSL];
	int cost;

	if ( !can_drop_obj(ch, obj) )
	{
		ch_printf( ch, "Non riesco a liberarmi di %s!\r\n", obj->name );
		return;
	}

	cost = get_repaircost( ch, keeper, obj );
	if ( cost < 0 )
	{
		if ( cost != -2 )
			sprintf( buf, "say a %s E' un oggetto senza valore, non vale la pena ripararlo.", ch->name );
		else
			sprintf( buf, "say a %s Non mi sembra che abbia bisogno di riparazioni!", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	/* "repair all" gets a 10% surcharge */
	if ( !str_cmp_all(arg, FALSE) )
		cost = (cost * 11) / 10;
	else
		cost = cost;

	sprintf( buf, "say a %s Ti costerà %d monet%c d'oro %s %s..'",
		ch->name,
		cost,
		(cost == 1)  ?  'a'  :  'e',
		fixstr,
		obj->name );
	send_command( keeper, buf, CO );

	if ( cost > ch->gold )
	{
		sprintf( buf, "say a %s  Ma vedo che non te lo puoi permettere..", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	sprintf( buf, "$n da $p a $N, che in poco tempo %s sistema.", fixstr2 );
	act( AT_ACTION, buf, ch, obj, keeper, TO_ROOM );
	sprintf( buf, "$N mi fa pagare la riparazione %d monet%c d'oro per %s $p.",
		cost, (cost == 1)  ?  'a'  :  'e', fixstr );
	act( AT_ACTION, buf, ch, obj, keeper, TO_CHAR );
	ch->gold 	-= cost;
	keeper->gold += cost;
	if ( keeper->gold < 0 )
	{
		keeper->gold = 0;
	}
	else if ( keeper->gold > maxgold )
	{
		boost_economy( keeper->in_room->area, keeper->gold - maxgold/2 );
		keeper->gold = maxgold/2;
		act( AT_ACTION, "$n depone le monete in un sacchettino.", keeper,
			NULL, NULL, TO_ROOM );
	}

	switch ( obj->type )
	{
	  default:
		send_to_char( ch, "Per qualche strana ragione sento un particolare senso di vuoto.. nelle mie tasche.\r\n" );
		break;
	  case OBJTYPE_ARMOR:
		obj->armor->ac = obj->armor->ac_orig;
		break;
	  case OBJTYPE_WEAPON:
		obj->weapon->condition = INIT_WEAPON_CONDITION;
		break;
	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		obj->wand->curr_charges = obj->wand->max_charges;
		break;
	}

	oprog_repair_trigger( ch, obj );
}


DO_RET do_repair( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*keeper;
	OBJ_DATA	*obj;
	char		*fixstr;
	char		*fixstr2;
	char		 buf[MSL];
	int			 maxgold;

	keeper = find_keeper( ch, "riparare" );
	if ( !keeper )
		return;

	/* (TT) ma servirà questo test? non ci pensa la find_keeper? */
	if ( keeper->pIndexData->rShop == NULL )
	{
		send_to_char( ch, "Non mi trovo da un negoziante adatto per questo.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		sprintf( buf, "say a %s Cosa vorresti far riparare?", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	maxgold = get_level(keeper)/2 * get_level(keeper)/2 * 100000;

	switch ( keeper->pIndexData->rShop->shop_type )
	{
	  default:
		send_log( NULL, LOG_BUG, "do_reapir: tipo di shop errato: %d", keeper->pIndexData->rShop->shop_type );
		/* senza break, continua in SHOP_FIX */
	  case SHOP_FIX:
		fixstr  = "ripara";
		fixstr2 = "ti ripara";
		break;
	  case SHOP_RECHARGE:
		fixstr  = "ricarica";
		fixstr2 = "ti ricarica";
		break;
	}

	if ( !str_cmp_all(argument, FALSE) )
	{
		for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
		{
			if ( obj->wear_loc != WEARLOC_NONE )		continue;
			if ( !can_see_obj(ch, obj, TRUE) )		continue;
			if ( !can_see_obj(keeper, obj, FALSE) )	continue;

			if ( (obj->type == OBJTYPE_ARMOR
			  ||  obj->type == OBJTYPE_WEAPON
			  ||  obj->type == OBJTYPE_WAND
			  ||  obj->type == OBJTYPE_STAFF) )
			{
				repair_one_obj( ch, keeper, obj, argument, maxgold, fixstr, fixstr2 );
			}
		}
		return;
	}

	obj = get_obj_carry( ch, argument, TRUE );
	if ( !obj )
	{
		sprintf( buf, "say a %s Non mi sembra che abbiate quest'oggetto.", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	if ( !can_see_obj(keeper, obj, FALSE) )
	{
		sprintf( buf, "say a %s Eh? Umh.. non riesco a vedere nulla del genere.", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	repair_one_obj( ch, keeper, obj, argument, maxgold, fixstr, fixstr2 );
}


void appraise_all( CHAR_DATA *ch, CHAR_DATA *keeper, char *fixstr )
{
	OBJ_DATA *obj;
	char	  buf[MSL];
	int 	  cost = 0;
	int		  total = 0;

	for ( obj = ch->first_carrying;  obj != NULL;  obj = obj->next_content )
	{
		if ( obj->wear_loc  != WEARLOC_NONE )		continue;
		if ( !can_see_obj(ch, obj, TRUE) )		continue;
		if ( (obj->type != OBJTYPE_ARMOR
		  &&  obj->type != OBJTYPE_WEAPON
		  &&  obj->type != OBJTYPE_WAND
		  &&  obj->type != OBJTYPE_STAFF) )		continue;

		if ( !can_drop_obj(ch, obj) )
		{
			ch_printf( ch, "Non riesco a liberarti di %s.\r\n", obj->name );
		}
		else if ( (cost = get_repaircost(ch, keeper, obj)) < 0 )
		{
			if ( cost != -2 )
				sprintf( buf, "say a %s Spiacente, non vale la pena ripararlo.", ch->name );
			else
				sprintf( buf, "say a %s Quest'oggetto mi sembra che sia in perfette condizioni!", ch->name );
			send_command( keeper, buf, CO );
		}
		else
		{
			sprintf( buf, "say a %s Ti costerà %d monet%c d'oro  %s %s'",
				ch->name,
				cost,
				(cost == 1)  ?  'a'  :  'e',
				fixstr, obj->name );
			send_command( keeper, buf, CO );
			total += cost;
		}
	} /* chiude il for */

	if ( total > 0 )
	{
		send_to_char( ch, "\r\n" );

		sprintf( buf, "say a %s Ti costerà %d monet%c d'oro in tutto.",
			ch->name,
			total,
			(cost == 1)  ?  'a'  :  'e' );
		send_command( keeper, buf, CO );

		sprintf( buf, "say a %s Ricorda che sulle riparazioni complete c'è la tassa del 10%% in più sul totale!", ch->name );
		send_command( keeper, buf, CO );
	}
}


DO_RET do_appraise( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*keeper;
	OBJ_DATA	*obj;
	char		*fixstr;
	char		 buf[MSL];
	int			 cost;

	keeper = find_keeper( ch, "riparare" );
	if ( !keeper )
		return;

	if ( !VALID_STR(argument) )
	{
		sprintf( buf, "say a %s Cosa vuoi far analizzare?", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	switch ( keeper->pIndexData->rShop->shop_type )
	{
	  default:
		send_log( NULL, LOG_BUG, "do_appraise: tipo di negozio errato: %d", keeper->pIndexData->rShop->shop_type );
		/* senza break, continua in SHOP_FIX */
	  case SHOP_FIX:
		fixstr  = "repair";
		break;
	  case SHOP_RECHARGE:
		fixstr  = "recharge";
		break;
	}

	if ( !str_cmp_all(argument, FALSE) )
	{
		appraise_all( ch, keeper, fixstr );
		return;
	}

	obj = get_obj_carry( ch, argument, TRUE );
	if ( !obj )
	{
		sprintf( buf, "say a %s Ah si? E dov'è?", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	if ( !can_see_obj(keeper, obj, FALSE) )
	{
		sprintf( buf, "say a %s Eh? Umh.. non riesco a vedere nulla del genere.", ch->name );
		send_command( keeper, buf, CO );
		return;
	}

	if ( !can_drop_obj(ch, obj) )
	{
		send_to_char( ch, "Non riesco a liberarmene!\r\n" );
		return;
	}

	cost = get_repaircost( ch, keeper, obj );
	if ( cost < 0 )
	{
		if ( cost != -2 )
		{
			sprintf( buf, "say a %s Spiacente.. non riuscirei proprio a fare nulla con %s",
				ch->name, obj->short_descr );
		}
		else
		{
			sprintf( buf, "say a %s %s mi sembra in perfette condizioni.",
				ch->name, obj->short_descr );
		}
		send_command( keeper, buf, CO );
		return;
	}

	sprintf( buf, "say a %s Ti costerà %d monet%c d'oro %s quell'oggetto..",
		ch->name,
		cost,
		(cost == 1)  ?  'a'  :  'e',
		fixstr );
	send_command( keeper, buf, CO );

	if ( cost > ch->gold )
	{
		sprintf( buf, "say a %s E non mi sembra che tu te lo possa permettere.", ch->name );
		send_command( keeper, buf, CO );
	}
}


/*																		  *
 * ------------------ Shop Building and Editing Section ----------------- *
 *																		  */

DO_RET do_makeshop( CHAR_DATA *ch, char *argument )
{
	SHOP_DATA	   *shop;
	MOB_PROTO_DATA *mob;
	VNUM			vnum;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Sintassi: makeshop <mobvnum>\r\n" );
		return;
	}

	vnum = atoi( argument );
	mob = get_mob_index( NULL, vnum );
	if ( !mob )
	{
		send_to_char( ch, "Mob non trovato.\r\n" );
		return;
	}

	if ( !can_edit_mob(ch, mob) )
		return;

	if ( mob->pShop )
	{
		send_to_char( ch, "Questo mob è già proprietario di un negozio.\r\n" );
		return;
	}

	CREATE( shop, SHOP_DATA, 1 );

	LINK( shop, first_shop, last_shop, next, prev );
	shop->keeper		= vnum;
	shop->profit_buy	= 120;
	shop->profit_sell	= 90;
	shop->open_hour		= 0;
	shop->close_hour	= 23;
	mob->pShop			= shop;

	send_to_char( ch, "Fatto.\r\n" );
}


DO_RET do_shopset( CHAR_DATA *ch, char *argument )
{
	SHOP_DATA	   *shop;
	MOB_PROTO_DATA *mob;
	char			arg1[MIL];
	char			arg2[MIL];
	VNUM			vnum;
	int				value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Sintassi: shopset <mob vnum> <field> value\r\n" );
		send_to_char( ch, "\r\nField being one of:\r\n" );
		send_to_char( ch, "  buy0 buy1 buy2 buy3 buy4 buy sell open close keeper\r\n" );
		return;
	}

	vnum = atoi( arg1 );
	mob = get_mob_index( NULL, vnum );
	if ( !mob )
	{
		send_to_char( ch, "Mob non trovato.\r\n" );
		return;
	}

	if ( !can_edit_mob(ch, mob) )
		return;

	if ( !mob->pShop )
	{
		send_to_char( ch, "Questo mob non è proprietario di alcun negozio.\r\n" );
		return;
	}
	shop = mob->pShop;
	value = atoi( argument );

	/* (RR) rifarlo con la str_cmp_exc e compra# e raggruppare quindi tutti i 5 i compra# */
	if ( !str_cmp(arg2, "compra0") || !str_cmp( arg2, "buy0" ) )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		shop->buy_type[0] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "compra1") || !str_cmp(arg2, "buy1") )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		shop->buy_type[1] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "compra2") || !str_cmp(arg2, "buy2") )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		shop->buy_type[2] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "compra3") || !str_cmp(arg2, "buy3") )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		shop->buy_type[3] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "compra4") || !str_cmp(arg2, "buy4") )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		shop->buy_type[4] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "compra") || !str_cmp(arg2, "buy") )
	{
		if ( value <= (shop->profit_sell+5) || value > 1000 )
		{
			send_to_char( ch, "Fuori dal valore massimo.\r\n" );
			return;
		}
		shop->profit_buy = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "vendi") || !str_cmp(arg2, "sell") )
	{
		if ( value < 0 || value >= (shop->profit_buy-5) )
		{
			send_to_char( ch, "Fuori dal valore massimo.\r\n" );
			return;
		}
		shop->profit_sell = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "aperto") || !str_cmp(arg2, "open") )
	{
		if ( value < 0 || value > 23 )
		{
			send_to_char( ch, "Fuori dal valore massimo.\r\n" );
			return;
		}
		shop->open_hour = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "chiuso") || !str_cmp(arg2, "close") )
	{
		if ( value < 0 || value > 23 )
		{
			send_to_char( ch, "Fuori dal valore massimo.\r\n" );
			return;
		}
		shop->close_hour = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "custode") || !str_cmp(arg2, "keeper") )
	{
		MOB_PROTO_DATA *mob2;

		mob2 = get_mob_index( NULL, vnum );
		if ( !mob2 )
		{
			send_to_char( ch, "Mob non trovato.\r\n" );
			return;
		}

		/* Se non può editare il mob originale esce */
		if ( !can_edit_mob(ch, mob) )
			return;

		if ( mob2->pShop )
		{
			send_to_char( ch, "Questo mob ha già un negozio.\r\n" );
			return;
		}
		mob->pShop  = NULL;
		mob2->pShop = shop;
		shop->keeper = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	send_command( ch, "shopset", CO );
}


/*
 * Comando admin che visualizza le statistiche di un negozio
 */
DO_RET do_shopstat( CHAR_DATA *ch, char *argument )
{
	SHOP_DATA	   *shop;
	MOB_PROTO_DATA *mob;
	VNUM			vnum;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Sintassi: shopstat <keeper vnum>\r\n" );
		return;
	}

	vnum = atoi( argument );
	mob = get_mob_index( NULL, vnum );
	if ( !mob )
	{
		send_to_char( ch, "Mob non trovato.\r\n" );
		return;
	}

	if ( mob->pShop == NULL )
	{
		send_to_char( ch, "Questo mob non ha un negozio.\r\n" );
		return;
	}

	shop = mob->pShop;

	ch_printf( ch, "Negoziante: #%d  %s  Razza: %s\r\n", shop->keeper, mob->short_descr, code_name(NULL, mob->race, CODE_RACE) );
	ch_printf( ch, "buy0 [%s]  buy1 [%s]  buy2 [%s]  buy3 [%s]  buy4 [%s]\r\n",
		code_name(NULL, shop->buy_type[0], CODE_OBJTYPE),
		code_name(NULL, shop->buy_type[1], CODE_OBJTYPE),
		code_name(NULL, shop->buy_type[2], CODE_OBJTYPE),
		code_name(NULL, shop->buy_type[3], CODE_OBJTYPE),
		code_name(NULL, shop->buy_type[4], CODE_OBJTYPE) );
	ch_printf( ch, "Profit:  buy %3d%%  sell %3d%%\r\n", shop->profit_buy, shop->profit_sell );
	ch_printf( ch, "Hours:   open %2d  close %2d\r\n", shop->open_hour, shop->close_hour );
}


DO_RET do_shops( CHAR_DATA *ch, char *argument )
{
	SHOP_DATA *shop;

	if ( !first_shop )
	{
		send_to_char( ch, "Non ci sono negozi\r\n" );
		return;
	}

	set_char_color( AT_NOTE, ch );
	for ( shop = first_shop;  shop;  shop = shop->next )
	{
		ch_printf( ch, "Keeper: %5d Buy: %3d Sell: %3d Open: %2d Close: %2d Buy: %2d %2d %2d %2d %2d\r\n",
			shop->keeper,
			shop->profit_buy,	shop->profit_sell,
			shop->open_hour,	shop->close_hour,
			shop->buy_type[0],
			shop->buy_type[1],
			shop->buy_type[2],
			shop->buy_type[3],
			shop->buy_type[4] );
	}
}


/*																		  *
 * -------------- Repair Shop Building and Editing Section -------------- *
 *																		  */

DO_RET do_makerepair( CHAR_DATA *ch, char *argument )
{
	REPAIR_DATA	   *repair;
	VNUM			vnum;
	MOB_PROTO_DATA *mob;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Sintassi: makerepair <mobvnum>\r\n" );
		return;
	}

	vnum = atoi( argument );
	mob = get_mob_index( NULL, vnum );
	if ( !mob )
	{
		send_to_char( ch, "Mob non trovato.\r\n" );
		return;
	}

	if ( !can_edit_mob(ch, mob) )
		return;

	if ( mob->rShop )
	{
		send_to_char( ch, "Questo mob ha già un negozio di riparazioni.\r\n" );
		return;
	}

	CREATE( repair, REPAIR_DATA, 1 );

	LINK( repair, first_repair, last_repair, next, prev );
	repair->keeper		= vnum;
	repair->profit_fix	= 100;
	repair->shop_type	= SHOP_FIX;
	repair->open_hour	= 0;
	repair->close_hour	= 23;
	mob->rShop			= repair;

	send_to_char( ch, "Fatto.\r\n" );
}


DO_RET do_repairset( CHAR_DATA *ch, char *argument )
{
	REPAIR_DATA	   *repair;
	MOB_PROTO_DATA *mob;
	MOB_PROTO_DATA *mob2;
	char			arg1[MIL];
	char			arg2[MIL];
	VNUM			vnum;
	int				value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Sintassi: repairset <mob vnum> <field> value\r\n"		);
		send_to_char( ch, "\r\nField being one of:\r\n"						);
		send_to_char( ch, "  fix0 fix1 fix2 profit type open close keeper\r\n"	);
		return;
	}

	vnum = atoi( arg1 );

	if ( (mob = get_mob_index(NULL, vnum)) == NULL )
	{
		send_to_char( ch, "Mob non trovato..\r\n" );
		return;
	}

	if ( !can_edit_mob(ch, mob) )
		return;

	if ( !mob->rShop )
	{
		send_to_char( ch, "Questo mob ha già un repair shop.\r\n" );
		return;
	}
	repair = mob->rShop;
	value = atoi( argument );

	/* (RR) Stessa cosa del compra# sopra */
	if ( !str_cmp(arg2, "ripara0") || !str_cmp(arg2, "fix0") )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		repair->fix_type[0] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "ripara1") || !str_cmp(arg2, "fix1") )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		repair->fix_type[1] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "ripara2") || !str_cmp(arg2, "fix2") )
	{
		if ( !is_number(argument) )
			value = code_num( NULL, argument, CODE_OBJTYPE );
		if ( value < 0 || value >= MAX_OBJTYPE )
		{
			send_to_char( ch, "Tipo d'oggetto non valido!\r\n" );
			return;
		}
		repair->fix_type[2] = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "profitto") || !str_cmp(arg2, "profit") )
	{
		if ( value < 1 || value > 1000 )
		{
			send_to_char( ch, "Fuori dai valori massimi.\r\n" );
			return;
		}
		repair->profit_fix = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "tipo") || !str_cmp(arg2, "type") )
	{
		if ( value < 1 || value > 2 )
		{
			send_to_char( ch, "Fuori dai valori limite.\r\n" );
			return;
		}
		repair->shop_type = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "aperto") || !str_cmp(arg2, "open") )
	{
		if ( value < 0 || value > 23 )
		{
			send_to_char( ch, "Fuori dai valori.\r\n" );
			return;
		}
		repair->open_hour = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "chiuso") || !str_cmp(arg2, "close") )
	{
		if ( value < 0 || value > 23 )
		{
			send_to_char( ch, "Fuori dai limiti massimi d'orario.\r\n" );
			return;
		}
		repair->close_hour = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "custode") || !str_cmp(arg2, "keeper") )
	{
		if ( (mob2 = get_mob_index(NULL, vnum)) == NULL )
		{
			send_to_char( ch, "Mob non trovato..\r\n" );
			return;
		}
		if ( !can_edit_mob(ch, mob) )
			return;
		if ( mob2->rShop )
		{
			send_to_char( ch, "Questo mob ha già un negozio.\r\n" );
			return;
		}
		mob->rShop  = NULL;
		mob2->rShop = repair;
		repair->keeper = value;
		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	send_command( ch, "repairset", CO );
}


DO_RET do_repairstat( CHAR_DATA *ch, char *argument )
{
	REPAIR_DATA	   *repair;
	MOB_PROTO_DATA *mob;
	VNUM			vnum;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Sintassi: repairstat <keeper vnum>\r\n" );
		return;
	}

	vnum = atoi( argument );

	if ( (mob = get_mob_index(NULL, vnum)) == NULL )
	{
		send_to_char( ch, "Mob non trovato..\r\n" );
		return;
	}

	if ( !mob->rShop )
	{
		send_to_char( ch, "Questo mob non ha nessun repair shop.\r\n" );
		return;
	}
	repair = mob->rShop;

	ch_printf( ch, "Keeper: %d  %s\r\n", repair->keeper, mob->short_descr );
	ch_printf( ch, "fix0 [%s]  fix1 [%s]  fix2 [%s]\r\n",
		code_name(NULL, repair->fix_type[0], CODE_OBJTYPE),
		code_name(NULL, repair->fix_type[1], CODE_OBJTYPE),
		code_name(NULL, repair->fix_type[2], CODE_OBJTYPE) );
	ch_printf( ch, "Profit: %3d%%  Type: %d\r\n", repair->profit_fix, repair->shop_type );
	ch_printf( ch, "Hours:   open %2d  close %2d\r\n", repair->open_hour, repair->close_hour );
}


DO_RET do_repairshops( CHAR_DATA *ch, char *argument )
{
	REPAIR_DATA *repair;

	if ( !first_repair )
	{
		send_to_char( ch, "Non ci sono repair shop.\r\n" );
		return;
	}

	set_char_color( AT_NOTE, ch );

	for ( repair = first_repair;  repair;  repair = repair->next )
	{
		ch_printf( ch, "Keeper: %5d Profit: %3d Type: %d Open: %2d Close: %2d Fix: %2d %2d %2d\r\n",
			repair->keeper,
			repair->profit_fix,	repair->shop_type,
			repair->open_hour,	repair->close_hour,
			repair->fix_type[0],
			repair->fix_type[1],
			repair->fix_type[2] );
	}
}
