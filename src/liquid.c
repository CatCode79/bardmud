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


#include "mud.h"
#include "affect.h"
#include "command.h"
#include "db.h"
#include "group.h"
#include "herb.h"
#include "interpret.h"
#include "liquid.h"
#include "make_obj.h"
#include "mprog.h"
#include "room.h"


/*
 * Tabella dei liquidi.
 * Utilizzata nella sezione #OBJECT dei file di area.
 * (CC) convertirli nelle aree, da pensare che poter fare
 */
const struct liquid_data table_liquid[] =
{
	{ "water",				"trasparente",		{  0, 1, 10 }	},	/*  0 */
	{ "beer",				"ambra",			{  3, 2,  5 }	},
	{ "wine",				"rosato",			{  5, 2,  5 }	},
	{ "ale",				"rubino",			{  6, 2,  5 }	},
	{ "dark ale",			"scuro",			{  1, 2,  5 }	},

	{ "whisky",				"dorato",			{  6, 1,  4 }	},	/*  5 */
	{ "lemonade",			"giallo",			{  0, 1,  8 }	},
	{ "firebreather",		"fumante",			{ 10, 0,  0 }	},
	{ "local specialty",	"opaco",			{  3, 3,  3 }	},
	{ "slime mold juice",	"verde",			{  0, 4, -8 }	},

	{ "milk",				"bianco",			{  0, 3,  6 }	},	/* 10 */
	{ "tea",				"rossiccio",		{  0, 1,  6 }	},
	{ "coffee",				"nero",				{  0, 1,  6 }	},
	{ "blood",				"rosso",			{  0, 2, -1 }	},
	{ "salt water",			"trasparente",		{  0, 1, -2 }	},

	{ "cola",				"ciliegia",			{  0, 1,  5 }	},	/* 15 */
	{ "mead",				"ambrato",			{  4, 2,  5 }	},
	{ "grog",				"marrone scuro",	{  3, 2,  5 }	}	/* 17 */
};


/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
DO_RET do_fill( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *source;
	char	  arg1[MIL];
	char	  arg2[MIL];
	int		  dest_item;
	int		  src_item1;
	int		  src_item2;
	int		  src_item3;
	int 	  diff = 0;
	bool 	  all = FALSE;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	/* munch optional words */
	if ( (!str_cmp(arg2, "da"  ) || !str_cmp(arg2, "con" )
	  ||  !str_cmp(arg2, "from") || !str_cmp(arg2, "with"))
	  && VALID_STR(argument) )
	{
		argument = one_argument( argument, arg2 );
	}

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "Devo riempire cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( (obj = get_obj_carry(ch, arg1, TRUE)) == NULL )
	{
		send_to_char( ch, "Non posseggo quest'oggetto.\r\n" );
		return;
	}
	else
	{
		dest_item = obj->type;
	}

	src_item1 = src_item2 = src_item3 = -1;
	switch ( dest_item )
	{
	  default:
		act( AT_ACTION, "$n tenta invano di riempire $p.", ch, obj, NULL, TO_ROOM );
		send_to_char( ch, "Non riesco a riempirlo.\r\n" );
		return;
	  /* place all fillable item types here */
	  case OBJTYPE_DRINK_CON:
		src_item1 = OBJTYPE_FOUNTAIN;
		src_item2 = OBJTYPE_BLOOD;
		break;
	  case OBJTYPE_HERB_CON:
		src_item1 = OBJTYPE_HERB;
		src_item2 = OBJTYPE_HERB_CON;
		break;
	  case OBJTYPE_PIPE:
		src_item1 = OBJTYPE_HERB;
		src_item2 = OBJTYPE_HERB_CON;
		break;
	  case OBJTYPE_CONTAINER:
		src_item1 = OBJTYPE_CONTAINER;
		src_item2 = OBJTYPE_CORPSE_MOB;
		src_item3 = OBJTYPE_CORPSE_PG;
		break;
	}

	if ( dest_item == OBJTYPE_CONTAINER || dest_item == OBJTYPE_PIPE )
	{
		if ( HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			act( AT_PLAIN, "$d è chiuso.", ch, NULL, obj->name, TO_CHAR );
			return;
		}
		if ( get_real_obj_weight(obj) / obj->count >= obj->container->capacity )
		{
			send_to_char( ch, "E' colmo fino all'orlo.\r\n" );
			return;
		}
	}
	else
	{
		diff = obj->drinkcon->capacity - obj->drinkcon->curr_quant;
		if ( diff < 1 || obj->drinkcon->curr_quant >= obj->drinkcon->capacity )
		{
			send_to_char( ch, "Sta traboccando fino all'orlo.\r\n" );
			return;
		}
	}

	if ( dest_item == OBJTYPE_PIPE && HAS_BITINT(obj->pipe->flags, PIPE_FULLOFASH) )
	{
		send_to_char( ch, "E' pieno di cenere, prima dovrei svuotarlo.\r\n" );
		return;
	}

	if ( VALID_STR(arg2) )
	{
		if ( dest_item == OBJTYPE_CONTAINER && !str_cmp_all(arg2, ALL) )
		{
			all = TRUE;
			source = NULL;
		}
		else if ( dest_item == OBJTYPE_PIPE )
		{
			/* This used to let you fill a pipe from an object on the ground.
			 * Seems to me you should be holding whatever you want to fill a pipe with.
			 * It's nitpicking, but I needed to change it to get a mobprog to work right.
			 * Check out Lord Fitzgibbon if you're curious.
			 */
			source = get_obj_carry( ch, arg2, TRUE );
			if ( !source )
			{
				send_to_char( ch, "Non ho quell'oggetto.\r\n" );
				return;
			}

			if ( source->type != src_item1
			  && source->type != src_item2
			  && source->type != src_item3 )
			{
				act( AT_PLAIN, "Non riesco a riempire $p con $P!", ch, obj, source, TO_CHAR );
				return;
			}
		}
		else
		{
			source =  get_obj_here( ch, arg2 );
			if ( !source )
			{
				send_to_char( ch, "Non trovo quell'oggetto.\r\n" );
				return;
			}
		}
	}
	else
	{
		source = NULL;
	}

	if ( !source && dest_item == OBJTYPE_PIPE )
	{
		send_to_char( ch, "Devo riempirlo con cosa?\r\n" );
		return;
	}

	if ( !source )
	{
		bool 	 found = FALSE;
		OBJ_DATA *src_next;

		found = FALSE;
		split_obj( obj, 1 );
		for ( source = ch->in_room->first_content;  source;  source = src_next )
		{
			src_next = source->next_content;
			if ( dest_item == OBJTYPE_CONTAINER )
			{
				if ( !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE) )							continue;
				if ( HAS_BIT(source->extra_flags, OBJFLAG_BURIED) )					continue;
				if ( ch->carry_weight + get_obj_weight(source) > can_carry_weight(ch) )	continue;

				if ( (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count) > obj->container->capacity )	/* (TT) uhmmm (bb) */
					continue;

				if ( all
				  && ((arg2[0] == 'a' && arg2[3] == '.' && !nifty_is_name(&arg2[4], source->name))		/* all. */
				  ||  (arg2[0] == 't' && arg2[4] == '.' && !nifty_is_name(&arg2[5], source->name))) )	/* tutto. tutti. tutte. */
				{
					continue;
				}

				obj_from_room( source );
				if ( source->type == OBJTYPE_MONEY )
				{
					ch->gold += source->money->qnt;
					free_object( source );
				}
				else
				{
					obj_to_obj( source, obj );
				}

				found = TRUE;
			}
			else if ( source->type == src_item1
			  ||	  source->type == src_item2
			  ||	  source->type == src_item3 )
			{
				found = TRUE;
				break;
			}
		} /* chiude il for */

		if ( !found )
		{
			switch ( src_item1 )
			{
			  default:
				send_to_char( ch, "Non c'è nulla che io possa utilizzare qui\r\n" );
				return;
			  case OBJTYPE_FOUNTAIN:
				send_to_char( ch, "Non vedo nessuna fontana o qualche liquido qui!\r\n" );
				return;
			  case OBJTYPE_BLOOD:
				send_to_char( ch, "Non trovo nessuna pozza di sangue qui!\r\n" );
				return;
			  case OBJTYPE_HERB_CON:
				send_to_char( ch, "Non trovo erbe qui!\r\n" );
				return;
			  case OBJTYPE_HERB:
				send_to_char( ch, "No, non trovo erbe da fumare.\r\n" );
				return;
			}
		}

		if ( dest_item == OBJTYPE_CONTAINER )
		{
			act( AT_ACTION, "Riempio $p.", ch, obj, NULL, TO_CHAR );
			act( AT_ACTION, "$n riempie $p.", ch, obj, NULL, TO_ROOM );
			return;
		}
	} /* chiude l'if */

	if ( dest_item == OBJTYPE_CONTAINER )
	{
		OBJ_DATA   *otmp,
				   *otmp_next;
		char	 	name[MIL];
		CHAR_DATA  *gch;
		char	   *pd;
		bool		found = FALSE;

		if ( source == obj )
		{
			send_to_char( ch, "Non posso riempire qualcosa con se stesso!\r\n" );
			return;
		}

		switch ( source->type )
		{
		  default:		/* put something in container */
			if ( !source->in_room		/* disallow inventory items */
			  || !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE)	/* (bb) forse source invece che obj? */
			  || ch->carry_weight + get_obj_weight(source) > can_carry_weight(ch)
			  || (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count) > obj->container->capacity )	/* (TT) uhmmm (bb) */
			{
				send_to_char( ch, "Non posso farlo.\r\n" );
				return;
			}
			split_obj( obj, 1 );
			act( AT_ACTION, "Prendo $P e lo infilo in $p.", ch, obj, source, TO_CHAR );
			act( AT_ACTION, "$n prende $P e lo mette dentro a $p.", ch, obj, source, TO_ROOM );
			obj_from_room(source);
			obj_to_obj(source, obj);
			break;

		  case OBJTYPE_MONEY:
			send_to_char( ch, "No, non posso farlo.. ancora.\r\n" );
			break;

		  case OBJTYPE_CORPSE_PG:
			if ( IS_MOB(ch) )
			{
				send_to_char( ch, "Spiacente ma non posso farlo.\r\n" );
				return;
			}

			if ( HAS_BIT(source->extra_flags, OBJFLAG_CLANCORPSE) && !IS_ADMIN(ch) )
			{
				send_to_char( ch, "Mi tremano le mani. Devo provare diversamente..\r\n" );
				return;
			}

			if ( !HAS_BIT(source->extra_flags, OBJFLAG_CLANCORPSE) )
			{
				pd = source->short_descr;
				pd = one_argument( pd, name );
				pd = one_argument( pd, name );
				pd = one_argument( pd, name );
				pd = one_argument( pd, name );

				if ( str_cmp(name, ch->name) && !IS_ADMIN(ch) )
				{
					bool fGroup;

					fGroup = FALSE;
					for ( gch = first_player;  gch;  gch = gch->next_player )
					{
						if ( !is_same_group(ch, gch) )
							continue;

						if ( !str_cmp(name, gch->name) )
						{
							fGroup = TRUE;
							break;
						}
					}

					if ( !fGroup )
					{
						send_to_char( ch, "Il cadavere di qualcun'altro..\r\n" );
						return;
					}
				}
			} /* chiude l'if */

		  case OBJTYPE_CONTAINER:
			if ( source->type == OBJTYPE_CONTAINER		/* don't remove */
			  && HAS_BITINT(source->container->flags, CONTAINER_CLOSED) )
			{
				act( AT_PLAIN, "$d è chiuso.", ch, NULL, source->name, TO_CHAR );
				return;
			}

		  case OBJTYPE_CORPSE_MOB:
			if ( (otmp = source->first_content) == NULL )
			{
				send_to_char( ch, "E' vuoto.\r\n" );
				return;
			}
			split_obj( obj, 1 );

			for (  ;  otmp;  otmp = otmp_next )
			{
				otmp_next = otmp->next_content;

				if ( !HAS_BIT( obj->wear_flags, OBJWEAR_TAKE)
				  || ch->carry_number + otmp->count > can_carry_number(ch)
				  || ch->carry_weight + get_obj_weight(otmp) > can_carry_weight(ch)
				  || (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count) > obj->container->capacity )	/* (TT) hmmm (bb) */
				{
					continue;
				}
				obj_from_obj( otmp );
				obj_to_obj( otmp, obj );
				found = TRUE;
			}

			if ( found )
			{
				act( AT_ACTION, "Riempio $p con $P.", ch, obj, source, TO_CHAR );
				act( AT_ACTION, "$n Riempie $p con $P.", ch, obj, source, TO_ROOM );
			}
			else
			{
				send_to_char( ch, "Lì dentro non trovo nulla di appropriato.\r\n" );
			}

			break;
		} /* chiude lo switch */
		return;
	} /* chiude l'if */

	if ( source->drinkcon && source->type != OBJTYPE_FOUNTAIN )
	{
		if ( source->drinkcon->curr_quant <= 0 )
		{
			send_to_char( ch, "Non è rimasto nulla con cui riempire!\r\n" );
			return;
		}

		if ( source->count > 1 )
			split_obj( source, 1 );
	}
	split_obj( obj, 1 );

	switch ( source->type )
	{
	  default:
		send_log( NULL, LOG_BUG, "do_fill: got bad item type: %d", source->type );
		send_to_char( ch, "Uhm qualcosa è andato storto..\r\n" );
		return;

	  case OBJTYPE_FOUNTAIN:
		if ( obj->drinkcon->curr_quant != 0 && obj->drinkcon->liquid != 0 )
		{
			send_to_char( ch, "C'è ancora liquido.\r\n" );
			return;
		}
		obj->drinkcon->liquid = 0;
		obj->drinkcon->curr_quant = obj->drinkcon->capacity;
		act( AT_ACTION, "Riempio $p da $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n riempie $p con $P.", ch, obj, source, TO_ROOM );
		return;

	  case OBJTYPE_BLOOD:
		if ( obj->drinkcon->curr_quant != 0 && obj->drinkcon->liquid != 13 )
		{
			send_to_char( ch, "E' presente ancora liquido.\r\n" );
			return;
		}
		obj->drinkcon->liquid = 13;
		if ( source->drinkcon->curr_quant < diff )
			diff = source->drinkcon->curr_quant;
		obj->drinkcon->curr_quant += diff;

		act( AT_ACTION, "Riempio $p con $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n riempie $p con $P.", ch, obj, source, TO_ROOM );

		if ( (source->drinkcon->curr_quant -= diff) < 1 )
		{
			free_object( source );
			make_bloodstain( ch );
		}
		return;

	  case OBJTYPE_HERB:
		if ( obj->container->capacity != 0 && obj->pipe->herb != source->herb->hnum )
		{
			send_to_char( ch, "Ci sono ancora erbe dentro.\r\n" );
			return;
		}
		obj->pipe->herb = source->herb->hnum;
		if ( source->herb->charges < diff )
			diff = source->herb->charges;
		obj->pipe->draws += diff;
		act( AT_ACTION, "Riempio $p con $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n riempie $p con $P.", ch, obj, source, TO_ROOM );
		if ( (source->herb->charges -= diff) < 1 )
			free_object( source );
		return;

	  case OBJTYPE_HERB_CON:
		if ( obj->drinkcon->curr_quant != 0 && obj->drinkcon->liquid != source->drinkcon->liquid )
		{
			send_to_char( ch, "Ci sono ancora sostanze erbacee all'interno.\r\n" );
			return;
		}
		obj->drinkcon->liquid = source->drinkcon->liquid;
		if ( source->drinkcon->curr_quant < diff )
			diff = source->drinkcon->curr_quant;
		obj->drinkcon->curr_quant += diff;
		source->drinkcon->curr_quant -= diff;
		act( AT_ACTION, "Riempio $p con $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n riempie $p con $P.", ch, obj, source, TO_ROOM );
		return;

	  case OBJTYPE_DRINK_CON:
		if ( obj->drinkcon->curr_quant != 0 && obj->drinkcon->liquid != source->drinkcon->liquid )
		{
			send_to_char( ch, "Noto che c'è ancora liquido.\r\n" );
			return;
		}
		obj->drinkcon->liquid = source->drinkcon->liquid;
		if ( source->drinkcon->curr_quant < diff )
			diff = source->drinkcon->curr_quant;
		obj->drinkcon->curr_quant += diff;
		source->drinkcon->curr_quant -= diff;

		act( AT_ACTION, "Riempio $p con $P.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n riempie $p con $P.", ch, obj, source, TO_ROOM );
		return;
	}
}


DO_RET do_drink( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char	  arg[MIL];
	int		  amount;
	int		  liquid;

	if ( !VALID_STR(argument) )
	{
		for ( obj = ch->in_room->first_content;  obj;  obj = obj->next_content )
		{
			if ( obj->type == OBJTYPE_FOUNTAIN || obj->type == OBJTYPE_BLOOD )
				break;
		}

		if ( !obj )
		{
			send_to_char( ch, "Devo bere da dove o che cosa?\r\n" );
			return;
		}
	}
	else
	{
		argument = one_argument( argument, arg );

		/* munch optional words */
		if ( is_name(arg, "da from") && VALID_STR(argument) )	/* (TT) */
			argument = one_argument( argument, arg );

		if ( (obj = get_obj_here(ch, arg)) == NULL )
		{
			send_to_char( ch, "Non riesco a trovare ove poter bere.\r\n" );
			return;
		}
	}

	if ( obj->count > 1 && obj->type != OBJTYPE_FOUNTAIN )
		split_obj( obj, 1 );

	if ( IS_PG(ch) && ch->pg->condition[CONDITION_DRUNK] > 40 )
	{
		send_to_char( ch, "Ehehehe *Hic* Non riescio più a raggiu-ngiere la mia boccha. *Hic*\r\n" );
		return;
	}

	switch ( obj->type )
	{
	  default:
		if ( obj->carried_by == ch )
		{
			act( AT_ACTION, "$n solleva $p fino alla sua bocca e tenta di bere..", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Porto $p alla bocca e tento di bere.. Ma! Mi sento bene?", ch, obj, NULL, TO_CHAR );
		}
		else
		{
			act( AT_ACTION, "$n si sdraia a terra tendando di bere da $p..", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Mi sdraio a terra tentando di bere da $p.. Ma! Mi sento bene?", ch, obj, NULL, TO_CHAR );
		}
		break;

	  case OBJTYPE_POTION:
		if ( obj->carried_by == ch )
		{
			char	buf[MSL];

			sprintf( buf, "quaff %s", arg );
			send_command( ch, buf, CO );
		}
		else
		{
			send_to_char( ch, "Non sto trasportando ciò.\r\n" );
		}
		break;

	  case OBJTYPE_BLOOD:
#ifdef T2_VAMPIRE
		/* (FF) futura implementazione della malattia del vampirismo qui */
		if ( IS_VAMPIRE(ch) && !IS_NPC(ch) )
		{
			if ( obj->timer > 0		/* if timer, must be spilled blood */
			  && ch->pg->condition[CONDITION_BLOODTHIRST] > get_level(ch)/20 + 5 )
			{
				send_to_char( "It is beneath you to stoop to drinking blood from the ground!\n\r", ch );
				send_to_char( "Unless in dire need, you'd much rather have blood from a victim's neck!\n\r", ch );
				return;
			}
			if ( ch->pg->condition[CONDITION_BLOODTHIRST] < (get_level(ch)/2 + 10) )
			{
				if ( ch->pg->condition[CONDITION_FULL] >= 48
				  || ch->pg->condition[CONDITION_THIRST] >= 48 )
				{
					send_to_char( "You are too full to drink any blood.\n\r", ch );
					return;
				}

				if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
				{
					act( AT_BLOOD, "$n drinks from the spilled blood.", ch, NULL, NULL, TO_ROOM );
					set_char_color( AT_BLOOD, ch );
					send_to_char( "You relish in the replenishment of this vital fluid...\n\r", ch );
					if ( obj->drinkcon->curr_quant <= 1 )
					{
						set_char_color( AT_BLOOD, ch );
						send_to_char( "You drink the last drop of blood from the spill.\n\r", ch );
						act( AT_BLOOD, "$n drinks the last drop of blood from the spill.", ch, NULL, NULL, TO_ROOM );
					}
				}

				gain_condition( ch, CONDITION_BLOODTHIRST, 1 );
				gain_condition( ch, CONDITION_FULL, 1 );
				gain_condition( ch, CONDITION_THIRST, 1 );
				if ( --obj->drinkcon->curr_quant <= 0 )
				{
					if ( obj->serial == cur_obj )
						global_objcode = rOBJ_DRUNK;
					free_object( obj );
					make_bloodstain( ch );
				}
			}
			else
			{
				send_to_char( "Alas... you cannot consume any more blood.\n\r", ch );
			}
		}
		else
#endif
		send_to_char( ch, "Non è nella mia natura bere del sangue.\r\n" );
		break;

	  case OBJTYPE_FOUNTAIN:
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n beve dalla fontana.", ch, NULL, NULL, TO_ROOM );
			send_to_char( ch, "Bevo qualche sorso dalla fontana.\r\n" );
		}

		if ( IS_PG(ch) )
			ch->pg->condition[CONDITION_THIRST] = 40;
		break;

	  case OBJTYPE_DRINK_CON:
		if ( obj->drinkcon->curr_quant <= 0 )
		{
			send_to_char( ch, "E' già vuoto.\r\n" );
			return;
		}

		if ( (liquid = obj->drinkcon->liquid) >= MAX_LIQ )
		{
			send_log( NULL, LOG_BUG, "do_drink: bad liquid number %d.", liquid );
			liquid = obj->drinkcon->liquid = 0;
		}

		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n beve $T da $p.", ch, obj, table_liquid[liquid].name, TO_ROOM );
			act( AT_ACTION, "Bevo $T da $p.", ch, obj, table_liquid[liquid].name, TO_CHAR );
		}

		amount = 1;

		if ( IS_PG(ch) )
		{
			gain_condition( ch, CONDITION_DRUNK,  amount * table_liquid[liquid].affect[CONDITION_DRUNK]	);
			gain_condition( ch, CONDITION_FULL,	 amount * table_liquid[liquid].affect[CONDITION_FULL]	);
			gain_condition( ch, CONDITION_THIRST, amount * table_liquid[liquid].affect[CONDITION_THIRST]	);

			if		( ch->pg->condition[CONDITION_DRUNK]  > 24 )	act( AT_ACTION, "Sono disintegrat$x.. quasi in coma etilico.", ch, NULL, NULL, TO_CHAR );
			else if ( ch->pg->condition[CONDITION_DRUNK]  > 18 )	act( AT_ACTION, "Sono molto ubriac$x.", ch, NULL, NULL, TO_CHAR );
			else if ( ch->pg->condition[CONDITION_DRUNK]  > 12 )	act( AT_ACTION, "Sono ubriaco.", ch, NULL, NULL, TO_CHAR );
			else if ( ch->pg->condition[CONDITION_DRUNK]  >  8 )	act( AT_ACTION, "Mi sento leggermente brill$x.", ch, NULL, NULL, TO_CHAR );
			else if ( ch->pg->condition[CONDITION_DRUNK]  >  5 )	act( AT_ACTION, "Sono con la mente lucida.", ch, NULL, NULL, TO_CHAR );

			if		( ch->pg->condition[CONDITION_FULL]   > 40 )	act( AT_ACTION, "Sono pien$x!", ch, NULL, NULL, TO_CHAR );

			if		( ch->pg->condition[CONDITION_THIRST] > 40 )	act( AT_ACTION, "Sono dissetat$x.", ch, NULL, NULL, TO_CHAR );
			else if ( ch->pg->condition[CONDITION_THIRST] > 36 )	act( AT_ACTION, "Ho una leggera sete.", ch, NULL, NULL, TO_CHAR );
			else if ( ch->pg->condition[CONDITION_THIRST] > 30 )	act( AT_ACTION, "Ho la gola secca.", ch, NULL, NULL, TO_CHAR );
		}

		if ( obj->drinkcon->poison )
		{
			/* The drink was poisoned! */
			AFFECT_DATA *aff;

			act( AT_POISON, "$n spalanca gli occhi, contorcendo il volto dal disgusto..", ch, NULL, NULL, TO_ROOM );
			act( AT_POISON, "Che schifo! Bleahh.. sarà veleno?", ch, NULL, NULL, TO_CHAR );

			ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );

			CREATE( aff, AFFECT_DATA, 1 );
			aff->type 		= gsn_poison;
			aff->duration	= obj->drinkcon->poison * 10;
			aff->location	= APPLY_NONE;
			aff->modifier	= 0;
			aff->bitvector	= meb( AFFECT_POISON );
			affect_join( ch, aff );
		}

		obj->drinkcon->curr_quant -= once_to_gram( amount );	/* Sì amount sarebbe 1 e veniva considerata un'oncia, bisogna convertirla dunque */
		if ( obj->drinkcon->curr_quant <= 0 )
		{
			send_to_char( ch, "Il contenitore sparisce..\r\n" );
			if ( cur_obj == obj->serial )
				global_objcode = rOBJ_DRUNK;
			free_object( obj );
		}
		break;
	} /* chiude lo switch */

	WAIT_STATE( ch, PULSE_IN_SECOND );
}
