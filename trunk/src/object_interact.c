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
 >						Modulo di manipolazione Oggetti						<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "gramm.h"
#include "group.h"
#include "information.h"
#include "interpret.h"
#include "herb.h"
#include "make_obj.h"
#include "movement.h"
#include "mprog.h"
#include "msp.h"
#include "room.h"
#include "save.h"
#include "shop.h"
#include "timer.h"


/*
 * How resistant an object is to damage
 * (FF) quando ci sarà il sistema dei materiali questa funzione sarà da modificare molto
 */
int get_obj_resistance( OBJ_DATA *obj )
{
	int resist;

	resist = number_fuzzy( MAX_ITEM_IMPACT );

	/* (FF) cambiare parte delle flag sotto con il sistema materiali */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC	 ) )	resist += number_fuzzy(12);	/* magical items are more resistant */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_METAL	 ) )	resist += number_fuzzy(5);	/* metal objects are definately stronger */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_ORGANIC	 ) )	resist -= number_fuzzy(5);	/* organic objects are most likely weaker */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_BLESS	 ) )	resist += number_fuzzy(5);	/* blessed objects should have a little bonus */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVENTORY) )	resist += 20;				/* lets make store inventory pretty tough */

	/* okay... let's add some bonus/penalty for item level... */
	resist += (obj->level / 20) - 2;

	/* and lasty... take armor or weapon's condition into consideration */
	if ( obj->type == OBJTYPE_ARMOR || obj->type == OBJTYPE_DRESS )
		resist += ( obj->armor->ac/2 ) - 2;
	if ( obj->type == OBJTYPE_WEAPON )
		resist += ( obj->weapon->condition/2 ) - 2;

	return URANGE( 10, resist, 99 );
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
	int		   weight;
	int		   amt;		/* gold per-race multipliers */

	if ( !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE) )
	{
		if ( IS_ADMIN(ch) )
		{
			send_to_char( ch, "L'oggetto non è TAKE ma gli admin lo prendono lo stesso.\r\n" );
		}
		else
		{
			send_to_char( ch, "Non posso prenderlo.\r\n" );
			return;
		}
	}

	if ( ch->carry_number + obj->count > can_carry_number(ch) )
	{
		act( AT_PLAIN, "$d: non posso portare con me tutte queste cose.", ch, NULL, obj->name, TO_CHAR );
		return;
	}

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_COVERING) )
		weight = obj->weight;
	else
		weight = get_obj_weight(obj);

	if ( ch->carry_weight + weight > can_carry_weight(ch) )
	{
		act( AT_PLAIN, "$d: non posso portare con me tutto questo peso.", ch, NULL, obj->name, TO_CHAR );
		return;
	}

	if ( container )
	{
		if ( container->type == OBJTYPE_KEYRING && !HAS_BIT(container->extra_flags, OBJFLAG_COVERING) )
		{
			act( AT_ACTION, "Tolgo $p da $P", ch, obj, container, TO_CHAR );
			act( AT_ACTION, "$n toglie $p da $P", ch, obj, container, TO_ROOM );
		}
		else
		{
			act( AT_ACTION, HAS_BIT(container->extra_flags, OBJFLAG_COVERING)  ?
				"Prendo $p da sotto $P."  :  "prendo $p da $P", ch, obj, container, TO_CHAR );
			act( AT_ACTION, HAS_BIT(container->extra_flags, OBJFLAG_COVERING)  ?
				"$n prende $p da sotto $P."  :  "$n prende $p da $P", ch, obj, container, TO_ROOM );
		}

		if ( HAS_BIT(container->extra_flags, OBJFLAG_CLANCORPSE) && IS_PG(ch) && str_cmp(container->name+22, ch->name) )
			container->corpse->looted++;

		obj_from_obj( obj );
	}
	else
	{
		act( AT_ACTION, "Prendo $p.", ch, obj, container, TO_CHAR );
		act( AT_ACTION, "$n prende $p.", ch, obj, container, TO_ROOM );
		obj_from_room( obj );
	}

	/* Storeroom checks */
	if ( HAS_BIT(ch->in_room->flags, ROOM_STOREROOM)
	  && (!container || !container->carried_by) )
	{
		fwrite_storage( ch );
	}

	if ( obj->type != OBJTYPE_CONTAINER )
		check_for_trap( ch, obj, TRAPFLAG_GET );

	if ( char_died(ch) )
		return;

	if ( obj->type == OBJTYPE_MONEY )
	{
		amt = obj->money->qnt;

#ifdef GOLD_MULT
		/*
		 *  The idea was to make some races more adroit at money handling,
		 *  however, this resulted in elves dropping 1M gps and picking
		 *  up 1.1M, repeating, and getting rich.  The only solution would
		 *  be to fuzzify the "drop coins" code, but that seems like it'd
		 *  lead to more confusion than it warrants.  -h
		 *
		 *  When you work on this again, make it so that amt is NEVER multiplied
		 *  by more than 1.0.  Use less than 1.0 for ogre, orc, troll, etc.
		 *  (Ie: a penalty rather than a bonus)
		 */
		/* (FF) controllare a vedere se aggiungere altre razze (o fare un campo per la razza apposta, oppure non utilizzare sta cosa */
		switch ( ch->race )
		{
		  case RACE_ELF		 :	amt *= 1.10;	break;
		  case RACE_DWARF	 :	amt *= 0.97;	break;
		  case RACE_HALFLING :	amt *= 1.02;	break;
		  case RACE_PIXIE	 :	amt *= 1.08;	break;
		  case RACE_HALFOGRE :	amt *= 0.92;	break;
		  case RACE_HALFORC	 :	amt *= 0.94;	break;
		  case RACE_HALFTROLL:	amt *= 0.90;	break;
		  case RACE_HALFELF	 :	amt *= 1.04;	break;
		  case RACE_GITH	 :	amt *= 1.06;	break;
		}
#endif

		/* Autosplit */
		if ( count_group(ch) > 0 && amt > 0 && HAS_BIT_PLR(ch, PLAYER_AUTOSPLIT) )
		{
			char	cmd[MIL];

			sprintf( cmd, "split %d", amt );
			send_command( ch, cmd, CO );
		}
#ifdef T2_MSP
		send_audio( ch->desc, "money.wav", TO_CHAR );
#endif
		ch->gold += amt;
		free_object( obj );
	}
	else
	{
		obj = obj_to_char( obj, ch );
	}

	if ( char_died(ch) || obj_extracted(obj) )
		return;

	oprog_get_trigger(ch, obj);
}


/*
 * Comando prendi
 */
DO_RET do_get( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA   *obj;
	OBJ_DATA   *obj_next;
	OBJ_DATA   *container;
	char		arg1[MIL];
	char		arg2[MIL];
	int			number;
	bool		found;

	argument = one_argument( argument, arg1 );
	if ( is_number(arg1) )
	{
		number = atoi( arg1 );
		if ( number < 1 )
		{
			send_to_char( ch, "Tsk, non mi sembra facile..\r\n" );
			return;
		}

		if ( (ch->carry_number + number) > can_carry_number(ch) )
		{
			send_to_char( ch, "Non posso trasportare così tanto.\r\n" );
			return;

		}
		argument = one_argument( argument, arg1 );
	}
	else
		number = 0;

	/* Get type. */
	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "Devo prendere cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	argument = one_argument( argument, arg2 );
	/* munch optional words */
	if ( (!str_cmp(arg2, "da") || !str_cmp(arg2, "from")) && !argument && VALID_STR(argument) )
		argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg2) )
	{
		if ( number <= 1 && str_cmp_all(arg1, ALL) )
		{
			/* 'get obj' */
			obj = get_obj_list( ch, arg1, ch->in_room->first_content, FALSE );
			if ( !obj )
			{
				act( AT_PLAIN, "Non c'è nessun $T qui.", ch, NULL, arg1, TO_CHAR );
				return;
			}

			split_obj( obj, 1 );
			get_obj( ch, obj, NULL );

			if ( char_died(ch) )
				return;

			save_player( ch );
		}
		else
		{
			int		cnt = 0;
			bool	fAll;
			char   *chk;

			if ( HAS_BIT(ch->in_room->flags, ROOM_DONATION) )
			{
				send_to_char( ch, "Meglio di no, questa mia azione è pura cupidigia!\r\n" );
				return;
			}

			if ( !str_cmp_all(arg1, FALSE) )
				fAll = TRUE;
			else
				fAll = FALSE;

			if ( number > 1 )
				chk = arg1;
			else
				chk = &arg1[ (arg1[0] == 'a') ? 4 : 6 ];	/* Se è all. punta al 4° se è tutto. tutti. tutte. a 6° carattere */

			/* 'get all' or 'get all.obj' */
			found = FALSE;
			for ( obj = ch->in_room->last_content;  obj;  obj = obj_next )
			{
				obj_next = obj->prev_content;

				if ( fAll == FALSE && !nifty_is_name(chk, obj->name) )	continue;
				if ( !can_see_obj(ch, obj, FALSE) )						continue;

				found = TRUE;

				if ( number && (cnt + obj->count) > number )
					split_obj( obj, number - cnt );

				cnt += obj->count;
				get_obj( ch, obj, NULL );

				if ( char_died(ch) )
					return;

				if ( ch->carry_number >= can_carry_number(ch)
				  || ch->carry_weight >= can_carry_weight(ch)
				  || (number && cnt >= number) )
				{
					save_player( ch );
					return;
				}
			} /* chiude il for */

			if ( !found )
			{
				if ( fAll )
					send_to_char( ch, "Non trovo nulla qui.\r\n" );
				else
					act( AT_PLAIN, "Non trovo nessun $T qui.", ch, NULL, chk, TO_CHAR );
			}
			else
				save_player( ch );
		} /* chiude l'else */
	} /* chiude l'if */
	else
	{
		char	buf[MSL];
		char	name[MIL];

		/* 'get ... container' */
		if ( !str_cmp_all(arg2, ALL) )
		{
			send_to_char( ch, "Non posso farlo.\r\n" );
			return;
		}

		container = get_obj_here( ch, arg2 );
		if ( !container )
		{
			act( AT_PLAIN, "Non trovo nessun $T qui.", ch, NULL, arg2, TO_CHAR );
			return;
		}

		switch ( container->type )
		{
		  default:
			if ( !HAS_BIT(container->extra_flags, OBJFLAG_COVERING) )
			{
				send_to_char( ch, "Non è un contenitore.\r\n" );
				return;
			}

			if ( ch->carry_weight + container->weight > can_carry_weight(ch) )
			{
				send_to_char( ch, "E' troppo pesante da sollevare per me.\r\n" );
				return;
			}
			break;

		  case OBJTYPE_CONTAINER:
		  case OBJTYPE_CORPSE_MOB:
		  case OBJTYPE_KEYRING:
		  case OBJTYPE_QUIVER:
			break;

		  case OBJTYPE_CORPSE_PG:
		  {
			CHAR_DATA *gch;

			if ( IS_MOB(ch) )
			{
				send_to_char( ch, "Non posso farlo.\r\n" );
				return;
			}

			/* Acquisisce il nome del cadavedere del pg */
			one_argument( container->name, name );

			if ( HAS_BIT(container->extra_flags, OBJFLAG_CLANCORPSE)
			  && IS_PG(ch)
			  && (get_timer(ch, TIMER_PKILLED) > 0 /*|| get_timer(ch, TIMER_KILLED) > 0*/)	/* (TT) da pensare */
			  && str_cmp(name, ch->name) )
			{
				send_to_char( ch, "Meglio non depredarlo ora.\r\n" );
				return;
			}

			/* Killer/owner loot only if die to pkill blow */
			/* Added check for admin so ADMS can get things out of corpses */
			if ( HAS_BIT(container->extra_flags, OBJFLAG_CLANCORPSE)
			  && IS_PG(ch) && !IS_ADMIN(ch)
			  && str_cmp(name, ch->name)
			  && VALID_STR(container->action_descr)
			  && str_cmp(container->action_descr, ch->name) )
			{
				send_to_char( ch, "Non sono stato io ad infliggere il colpo mortale su questo cadavere.\r\n" );
				return;
			}

			if ( HAS_BIT(container->extra_flags, OBJFLAG_CLANCORPSE)
			  && IS_PG(ch)
			  && str_cmp(name, ch->name)
			  && container->corpse->looted >= 3 )
			{
				send_to_char( ch, "Questo corpo sembra essere stato già saccheggiato.\r\n" );
				return;
			}

			if ( HAS_BIT(container->extra_flags, OBJFLAG_CLANCORPSE) && IS_PG(ch)
			  &&  container->corpse->level - get_level(ch) <  6
			  &&  container->corpse->level - get_level(ch) > -6 )
			{
				break;
			}

			if ( str_cmp(name, ch->name) && !IS_ADMIN(ch) )
			{
				bool fGroup;

				fGroup = FALSE;
				for ( gch = first_player;  gch;  gch = gch->next_player )
				{
					if ( is_same_group(ch, gch) && !str_cmp(name, gch->name) )
					{
						fGroup = TRUE;
						break;
					}
				}

				if ( !fGroup )
				{
					send_to_char( ch, "Questo cadavere è di qualcun'altro, non fa parte del nostro gruppo.\r\n" );
					return;
				}
			}
		  } /* chiude il case */
		} /* chiude lo switch */

		if ( !HAS_BIT(container->extra_flags, OBJFLAG_COVERING )
		  && HAS_BITINT(container->container->flags, CONTAINER_CLOSED) )
		{
			act( AT_PLAIN, "$d è chiuso.", ch, NULL, container->name, TO_CHAR );
			return;
		}

		if ( number <= 1 && str_cmp_all(arg1, ALL) )
		{
			/* 'get obj container' */
			obj = get_obj_list( ch, arg1, container->first_content, TRUE );
			if ( !obj )
			{
				if ( HAS_BIT(container->extra_flags, OBJFLAG_COVERING) )
					sprintf( buf, "Non trovo nulla del genere sotto %s.", container->short_descr );	/* (GR) */
				else
					sprintf( buf, "Non trovo nulla del genere in %s.", container->short_descr );	/* (GR) */

				act( AT_PLAIN, buf, ch, NULL, NULL, TO_CHAR );
				return;
			}

			split_obj( obj, 1 );
			get_obj( ch, obj, container );

			/* Oops no wonder corpses were duping oopsie did I do that */
			if ( container->type == OBJTYPE_CORPSE_PG )
				write_corpses( NULL, name, NULL );

			check_for_trap( ch, container, TRAPFLAG_GET );

			if ( char_died(ch) )
				return;

			save_player( ch );
		}
		else
		{
			int cnt = 0;
			bool fAll;
			char *chk;

			/* 'get all container' or 'get all.obj container' */
			if ( HAS_BIT(container->extra_flags, OBJFLAG_DONATION) )
			{
				send_to_char( ch, "No, è meglio per me non gettarmi in queste basse azioni di avidità!\r\n" );
				return;
			}

			if ( HAS_BIT(container->extra_flags, OBJFLAG_CLANCORPSE)
			  && !IS_ADMIN(ch)
			  && IS_PG(ch)
			  && str_cmp(ch->name, name) )
			{
				send_to_char( ch, "No, è meglio per me resistere alla bassa cupidigia!\r\n" );
				return;
			}

			if ( !str_cmp_all(arg1, FALSE) )
				fAll = TRUE;
			else
				fAll = FALSE;

			if ( number > 1 )
				chk = arg1;
			else
				chk = &arg1[ (arg1[0] == 'a') ? 4 : 5 ];

			found = FALSE;
			for ( obj = container->first_content;  obj;  obj = obj_next )
			{
				obj_next = obj->next_content;

				if ( fAll == FALSE && !nifty_is_name(chk, obj->name) )	continue;
				if ( !can_see_obj(ch, obj, TRUE) )						continue;

				found = TRUE;
				if ( number && (cnt + obj->count) > number )
					split_obj( obj, number - cnt );

				cnt += obj->count;
				get_obj( ch, obj, container );

				if ( char_died(ch) )
					return;

				if ( ch->carry_number >= can_carry_number(ch)
				  || ch->carry_weight >= can_carry_weight(ch)
				  || (number && cnt >= number) )
				{
					save_player( ch );
					return;
				}
			} /* chiude il for */

			if ( !found )
			{
				if ( fAll )
				{
					if ( container->type == OBJTYPE_KEYRING && !HAS_BIT(container->extra_flags, OBJFLAG_COVERING) )
						act( AT_PLAIN, "$T non ha chiavi.", ch, NULL, arg2, TO_CHAR );
					else
					{
						sprintf( buf, "Non trovo nulla del genere %s %s.",
							HAS_BIT(container->extra_flags, OBJFLAG_COVERING) ? "sotto" : "in",
							container->short_descr );	/* (GR) */
						act( AT_PLAIN, buf, ch, NULL, NULL, TO_CHAR );
					}
				}
				else
				{
					if ( container->type == OBJTYPE_KEYRING && !HAS_BIT(container->extra_flags, OBJFLAG_COVERING) )
						act( AT_PLAIN, "$T non possiede questa chiave.", ch, NULL, arg2, TO_CHAR );
					else
					{
						sprintf( buf, "Non trovo nulla del genere %s %s.",
							HAS_BIT(container->extra_flags, OBJFLAG_COVERING) ? "sotto" : "in",
							container->short_descr );	/* (GR) e dopo non dovrebbe servire più la act */
						act( AT_PLAIN, buf, ch, NULL, NULL, TO_CHAR );
					}
				}
			}
			else
				check_for_trap( ch, container, TRAPFLAG_GET );

			if ( char_died(ch) )
				return;

			/* Oops no wonder corpses were duping oopsie did I do that */
			if ( container->type == OBJTYPE_CORPSE_PG )
				write_corpses( NULL, name, NULL );

			if ( found )
				save_player( ch );
		} /* chiude l'else */
	} /* chiude l'else */
}


DO_RET do_put( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA   *container;
	OBJ_DATA   *obj;
	OBJ_DATA   *obj_next;
	char		arg1[MIL];
	char		arg2[MIL];
	int			count;
	int			number;
	bool		save_char = FALSE;

	argument = one_argument( argument, arg1 );
	if ( is_number(arg1) )
	{
		number = atoi( arg1 );
		if ( number < 1 )
		{
			send_to_char( ch, "Troooppo facile..\r\n" );
			return;
		}
		argument = one_argument( argument, arg1 );
	}
	else
	{
		number = 0;
	}

	argument = one_argument( argument, arg2 );
	/* munch optional words */
	if ( (!str_cmp(arg2, "dentro") || !str_cmp(arg2, "into"	 )
	  ||  !str_cmp(arg2, "in"	 )
	  ||  !str_cmp(arg2, "sotto" ) || !str_cmp(arg2, "under" )
	  ||  !str_cmp(arg2, "sopra" ) || !str_cmp(arg2, "onto"	 )
	  ||  !str_cmp(arg2, "su"	 ) || !str_cmp(arg2, "on"	 ))
	  && !VALID_STR(argument) )
	{
		argument = one_argument( argument, arg2 );
	}

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Devo mettere cosa dentro che cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( !str_cmp_all(arg2, ALL) )
	{
		send_to_char( ch, "Non posso farlo.\r\n" );
		return;
	}

	container = get_obj_here( ch, arg2 );
	if ( !container )
	{
		act( AT_PLAIN, "Non trovo nessun $T qui.", ch, NULL, arg2, TO_CHAR );
		return;
	}

	if ( !container->carried_by )
		save_char = TRUE;

	if ( HAS_BIT(container->extra_flags, OBJFLAG_COVERING) )
	{
		if ( ch->carry_weight + container->weight > can_carry_weight(ch) )
		{
			send_to_char( ch, "E' troppo pesante da sollevare per me.\r\n" );
			return;
		}
	}
	else
	{
		if ( !container->container )
		{
			send_to_char( ch, "Non è un contenitore.\r\n" );
			return;
		}

		if ( HAS_BITINT(container->container->flags, CONTAINER_CLOSED) )
		{
			act( AT_PLAIN, "$d è chiuso.", ch, NULL, container->name, TO_CHAR );
			return;
		}
	}

	if ( number <= 1 && str_cmp_all(arg1, ALL) )
	{
		/* Se il numero di oggetti da mettere nel contenitore è uno
		 *	e se non si è passato l'argomento all
		 *	allora mette un solo oggetto nel contenitore
		 */

		obj = get_obj_carry( ch, arg1, TRUE );
		if ( !obj )
		{
			send_to_char( ch, "Non posseggo quell'oggetto.\r\n" );
			return;
		}

		if ( obj == container )
		{
			send_to_char( ch, "Non posso metterlo dentro se stesso.\r\n" );
			return;
		}

		if ( !can_drop_obj(ch, obj) )
		{
			send_to_char( ch, "Non vuole venire via.\r\n" );
			return;
		}

		if ( container->type == OBJTYPE_KEYRING && obj->type != OBJTYPE_KEY )
		{
			send_to_char( ch, "Non è una chiave.\r\n" );
			return;
		}

		if ( container->type == OBJTYPE_QUIVER && obj->type != OBJTYPE_PROJECTILE )
		{
			send_to_char( ch, "Non è un proiettile.\r\n" );
			return;
		}

		if ( (HAS_BIT(container->extra_flags, OBJFLAG_COVERING)
		  && (get_obj_weight(obj)/obj->count)
		  > ((get_obj_weight(container)/container->count)
		  -  container->weight)) )
		{
			send_to_char( ch, "Non riesco ad infilarlo lì.\r\n" );
			return;
		}

		/* Note: use of get_real_obj_weight() */
		if ( (get_real_obj_weight(obj)/obj->count)
		  +  (get_real_obj_weight(container)/container->count)
		  >  container->container->capacity )
		{
			send_to_char( ch, "Non riesco a infilarlo.\r\n" );
			return;
		}

		split_obj( obj, 1 );
		split_obj( container, 1 );
		obj_from_char( obj );
		obj = obj_to_obj( obj, container );
		check_for_trap ( ch, container, TRAPFLAG_PUT );

		if ( char_died(ch) )
			return;

		count = obj->count;
		obj->count = 1;

		if ( container->type == OBJTYPE_KEYRING && !HAS_BIT(container->extra_flags, OBJFLAG_COVERING) )
		{
			act( AT_ACTION, "$n infila $p su $P.", ch, obj, container, TO_ROOM );
			act( AT_ACTION, "Infilo $p su $P.", ch, obj, container, TO_CHAR );
		}
		else
		{
			act( AT_ACTION, HAS_BIT(container->extra_flags, OBJFLAG_COVERING)
				?  "$n nasconde $p sotto $P."  :  "$n mette $p in $P.",
				ch, obj, container, TO_ROOM );

			act( AT_ACTION, HAS_BIT(container->extra_flags, OBJFLAG_COVERING)
				?  "Nascondo $p sotto $P."  :  "Metto $p in $P.",
				ch, obj, container, TO_CHAR );
		}

		obj->count = count;

		/* Oops no wonder corpses were duping oopsie did I do that */
		if ( container->type == OBJTYPE_CORPSE_PG )
			write_corpses( NULL, container->short_descr+22, NULL );

		if ( save_char )
			save_player(ch);

		/* Storeroom check */
		if ( HAS_BIT(ch->in_room->flags, ROOM_STOREROOM) && !container->carried_by )
			fwrite_storage( ch );
	}
	else
	{
		char   *chk;
		int		cnt = 0;
		bool	fAll;
		bool	found = FALSE;
		bool	nofit = FALSE;		/* corregge un baco, prima appena trovava un oggetto che non stava nel contenitore smetteva il ciclo */
		bool	nodrop = FALSE;

		if ( !str_cmp_all(arg1, FALSE) )
			fAll = TRUE;
		else
			fAll = FALSE;

		if ( number > 1 )
			chk = arg1;
		else
			chk = &arg1[ arg1[0] == 'a' ? 4 : 5 ];

		split_obj( container, 1 );
		/* 'put all container' or 'put all.obj container' */
		for ( obj = ch->first_carrying;  obj;  obj = obj_next )
		{
			obj_next = obj->next_content;

			if ( fAll == FALSE && !nifty_is_name(chk, obj->name) )	continue;
			if ( obj->wear_loc != WEARLOC_NONE )						continue;
			if ( !can_see_obj(ch, obj, TRUE) )						continue;
			if ( obj == container )									continue;

			if ( container->type == OBJTYPE_KEYRING && obj->type != OBJTYPE_KEY )
				continue;
			if ( container->type == OBJTYPE_QUIVER  && obj->type != OBJTYPE_PROJECTILE )
				continue;

			if ( !can_drop_obj(ch, obj) )
			{
				nodrop = TRUE;
				act( AT_ACTION, "Non posso mettere $p dentro $P.", ch, obj, container, TO_CHAR );
			}
			else if ( get_obj_weight(obj) + get_obj_weight(container) > container->container->capacity )	/* il peso dell'oggetto più quello del contenitore */
			{
				nofit = TRUE;
			}
			else
			{
				found = TRUE;

				if ( number && (cnt + obj->count) > number )
					split_obj(obj, number - cnt);

				cnt += obj->count;
				obj_from_char( obj );

				if ( container->type == OBJTYPE_KEYRING)
				{
					act( AT_ACTION, "$n infila $p, in $P.", ch, obj, container, TO_ROOM );
					act( AT_ACTION, "Infilo $p, in $P.", ch, obj, container, TO_CHAR );
				}
				else
				{
					act( AT_ACTION, "$n mette $p in $P.", ch, obj, container, TO_ROOM );
					act( AT_ACTION, "Metto $p in $P.", ch, obj, container, TO_CHAR );
				}

				obj = obj_to_obj( obj, container );

				check_for_trap( ch, container, TRAPFLAG_PUT );

				if ( char_died(ch) )
					return;

				if ( number && cnt >= number )
					break;
			} /* chiude l'if */
		} /* chiude il for */

		if ( nofit == TRUE )
			act( AT_PLAIN, "Non riesco a mettere dentro qualche oggetto.", ch, NULL, NULL, TO_CHAR );

		/* Don't bother to save anything if nothing was dropped */
		if ( found == FALSE )
		{
			if ( nofit == FALSE && nodrop == FALSE )
			{
				if ( fAll )
					act( AT_PLAIN, "Non sto trasportando nulla.", ch, NULL, NULL, TO_CHAR );
				else
					act( AT_PLAIN, "Non sto trasportando $T.", ch, NULL, chk, TO_CHAR );
			}
			return;
		}

		if ( save_char )
			save_player(ch);

		/* Oops no wonder corpses were duping oopsie did I do that */
		if ( container->type == OBJTYPE_CORPSE_PG )
			write_corpses( NULL, container->short_descr+22, NULL );

		/* Storeroom check */
		if ( HAS_BIT(ch->in_room->flags, ROOM_STOREROOM) && !container->carried_by )
			fwrite_storage( ch );
	} /* chiude l'else */
}


DO_RET do_empty( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA		   *obj;
	char				arg1[MIL];
	char				arg2[MIL];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( (!str_cmp(arg2, "dentro") || !str_cmp(arg2, "in") || !str_cmp(arg2, "into"))
	  && VALID_STR(argument) )
	{
		argument = one_argument( argument, arg2 );
	}

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "Cosa dovrei svuotare?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( (obj = get_obj_carry(ch, arg1, TRUE)) == NULL )
	{
		send_to_char( ch, "Non ho nulla del genere.\r\n" );
		return;
	}

	if ( obj->count > 1 )
		split_obj( obj, 1 );

	switch ( obj->type )
	{
	  default:
		act( AT_ACTION, "Agito $p tentando di svuotarlo..", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n inizia ad agitare $p cercando di svuotare il contenuto..", ch, obj, NULL, TO_ROOM );
		return;

	  case OBJTYPE_PIPE:
		act( AT_ACTION, "Colpisco gentilmente $p e lo svuoto.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n da colpetti al fondo di $p svuotando il contenuto.", ch, obj, NULL, TO_ROOM );
		REMOVE_BITINT( obj->pipe->flags, PIPE_FULLOFASH );
		REMOVE_BITINT( obj->pipe->flags, PIPE_LIT );
		obj->pipe->draws = 0;
		return;

	  case OBJTYPE_DRINK_CON:
		if ( obj->drinkcon->curr_quant < 1 )
		{
			send_to_char( ch, "E' vuoto.\r\n" );
			return;
		}
		act( AT_ACTION, "Svuoto $p.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n svuota $p.", ch, obj, NULL, TO_ROOM );
		obj->drinkcon->curr_quant = 0;
		return;

	  case OBJTYPE_CONTAINER:
	  case OBJTYPE_QUIVER:
		if ( HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			act( AT_PLAIN, "$d è chiuso.", ch, NULL, obj->name, TO_CHAR );
			return;
		}
		/* non ci vuole il break */

	  case OBJTYPE_KEYRING:
		if ( !obj->first_content )
		{
			send_to_char( ch, "E' gia vuoto.\r\n" );
			return;
		}

		if ( !VALID_STR(arg2) )
		{
			if ( HAS_BIT(ch->in_room->flags, ROOM_NODROP)
			  || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_LITTERBUG)) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( ch, "E' meglio non disordinare questo luogo\r\n" );
				return;
			}

			if ( HAS_BIT(ch->in_room->flags, ROOM_NODROPALL)
			  || HAS_BIT(ch->in_room->flags, ROOM_STOREROOM) )
			{
				send_to_char( ch, "Uhm non posso farlo qui..\r\n" );
				return;
			}

			if ( empty_obj(obj, NULL, ch->in_room) )
			{
				act( AT_ACTION, "Svuoto $p.", ch, obj, NULL, TO_CHAR );
				act( AT_ACTION, "$n svuota $p.", ch, obj, NULL, TO_ROOM );

				save_player( ch );
			}
			else
			{
				send_to_char( ch, "Hmmm.. non funziona.\r\n" );
			}
		}
		else
		{
			OBJ_DATA *dest = get_obj_here(ch, arg2);

			if ( !dest )
			{
				send_to_char( ch, "Non lo trovo.\r\n" );
				return;
			}

			if ( dest == obj )
			{
				send_to_char( ch, "Non posso svuotarlo dentro se stesso!\r\n" );
				return;
			}

			if ( dest->type != OBJTYPE_CONTAINER
			  && dest->type != OBJTYPE_KEYRING
			  && dest->type != OBJTYPE_QUIVER )
			{
				send_to_char( ch, "Non credo sia un contenitore.\r\n" );
				return;
			}

			if ( HAS_BITINT(dest->container->flags, CONTAINER_CLOSED) )
			{
				act( AT_PLAIN, "$d è chiuso.", ch, NULL, dest->name, TO_CHAR );
				return;
			}

			split_obj( dest, 1 );
			if ( empty_obj( obj, dest, NULL ) )
			{
				act( AT_ACTION, "Svuoto $p dentro $P.", ch, obj, dest, TO_CHAR );
				act( AT_ACTION, "$n svuota $p dentro $P.", ch, obj, dest, TO_ROOM );
				if ( !dest->carried_by )
					save_player( ch );
			}
			else
			{
				act( AT_ACTION, "$P è pieno fino all'orlo.", ch, obj, dest, TO_CHAR );
			}
		} /* chiude l'else */
		return;

	} /* chiude lo switch */
}


DO_RET do_drop( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA		   *obj;
	OBJ_DATA		   *obj_next;
	char				arg[MIL];
	int					number;
	bool				found;

	argument = one_argument( argument, arg );
	if ( is_number(arg) )
	{
		number = atoi( arg );
		if ( number < 1 )
		{
			send_to_char( ch, "Ehehe, facile..\r\n" );
			return;
		}
		argument = one_argument( argument, arg );
	}
	else
	{
		number = 0;
	}

	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Devo posare che cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_LITTERBUG) )
	{
		set_char_color( AT_YELLOW, ch );
		send_to_char( ch, "Una strana Forza mi intorpidisce e mi impedisce di posarlo..\r\n" );
		return;
	}

	if ( HAS_BIT(ch->in_room->flags, ROOM_NODROP) && ch != supermob )
	{
		if ( IS_ADMIN(ch) )
			send_to_char( ch, "[ADM] In questa stanza i pg normali non potrebbero droppare oggetti.\r\n" );
		else
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Una misteriosa Forza, forse del luogo, mi blocca!\r\n" );
			return;
		}
	}

	if ( number > 0 )
	{
		/* 'drop NNNN coins' */

		if ( is_name_prefix(arg, "monete moneta coins coin oro") )
		{
			if ( ch->gold < number )
			{
				send_to_char( ch, "Non ho tutte quelle monete.\r\n" );
				return;
			}

			ch->gold -= number;

			for ( obj = ch->in_room->first_content;  obj;  obj = obj_next )
			{
				obj_next = obj->next_content;

				switch ( obj->vnum )
				{
				  case VNUM_OBJ_MONEY_ONE:
					number += 1;
					free_object( obj );
					break;

				  case VNUM_OBJ_MONEY_SOME:
					number += obj->money->qnt;
					free_object( obj );
					break;
				}
			}
			act( AT_ACTION, "$n lascia cadere dell'oro.", ch, NULL, NULL, TO_ROOM );
			obj_to_room( make_money(number), ch->in_room );
			send_to_char( ch, "Lascio scivolare dell'oro per terra.\r\n" );
#ifdef T2_MSP
			send_audio( ch->desc, "money.wav", TO_CHAR );
#endif
			save_player( ch );

			return;
		} /* chiude l'if */
	} /* chiude l'if */

	if ( number <= 1 && str_cmp_all(arg, ALL) )
	{
		/* 'drop obj' */
		if ( (obj = get_obj_carry(ch, arg, TRUE)) == NULL )
		{
			send_to_char( ch, "Non posseggo quell'oggetto.\r\n" );
			return;
		}

		if ( !can_drop_obj(ch, obj) )
		{
			send_to_char( ch, "Non si vuole staccare dal mio corpo.\r\n" );
			return;
		}

		split_obj( obj, 1 );
		act( AT_ACTION, "$n posa $p.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Poso $p.", ch, obj, NULL, TO_CHAR );

		obj_from_char( obj );
		obj = obj_to_room( obj, ch->in_room );
		oprog_drop_trigger( ch, obj );	/* mudprogs */

		if ( char_died(ch) || obj_extracted(obj) )
			return;

		/* Storeroom saving */
		if ( HAS_BIT(ch->in_room->flags, ROOM_STOREROOM) )
			fwrite_storage( ch );
	}
	else
	{
		int	  cnt = 0;
		char *chk;
		bool  fAll;

		if ( !str_cmp_all(arg, FALSE) )
			fAll = TRUE;
		else
			fAll = FALSE;

		if ( number > 1 )
			chk = arg;
		else
			chk = &arg[ arg[0] == 'a' ? 4 : 5 ];

		/* 'drop all' or 'drop all.obj' */
		if ( HAS_BIT(ch->in_room->flags, ROOM_NODROPALL)
		  || HAS_BIT(ch->in_room->flags, ROOM_STOREROOM) )
		{
			send_to_char( ch, "Non posso farlo qui..\r\n" );
			return;
		}

		found = FALSE;
		for ( obj = ch->first_carrying;  obj;  obj = obj_next )
		{
			obj_next = obj->next_content;

			if ( fAll == FALSE && !nifty_is_name(chk, obj->name) )	continue;
			if ( obj->wear_loc != WEARLOC_NONE )						continue;
			if ( !can_see_obj(ch, obj, TRUE) )						continue;
			if ( !can_drop_obj(ch, obj) )							continue;

			found = TRUE;
			if ( has_trigger(obj->first_mudprog, MPTRIGGER_DROP) && obj->count > 1 )
			{
				++cnt;
				split_obj( obj, 1 );
				obj_from_char( obj );
				if ( !obj_next )
					obj_next = ch->first_carrying;
			}
			else
			{
				if ( number && (cnt + obj->count) > number )
					split_obj( obj, number - cnt );
				cnt += obj->count;
				obj_from_char( obj );
			}
			act( AT_ACTION, "$n posa $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Poso $p.", ch, obj, NULL, TO_CHAR );
			obj = obj_to_room( obj, ch->in_room );
			oprog_drop_trigger( ch, obj );		/* mudprogs */
			if ( char_died(ch) )
				return;
			if ( number && cnt >= number )
				break;
		} /* chiude il for */

		if ( !found )
		{
			if ( fAll )
				act( AT_PLAIN, "Non stai trasportando nulla.", ch, NULL, NULL, TO_CHAR );
			else
				act( AT_PLAIN, "Non sto trasportando $T.", ch, NULL, chk, TO_CHAR );
		}
	}

	save_player( ch );	/* duping protector */
}


/*
 * Gestisce il give di N monete a un mob o pg
 * E' scaccato da do_give perché viene chiamata anche da do_money
 */
DO_RET give_money( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	char		arg1[MIL];
	char		arg2[MIL];
	char		buf [MIL];
	int			amount;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Devo dare quante monete a chi?\r\n" );
		return;
	}

	amount = atoi( arg1 );
	if ( amount <= 0 || !is_name_prefix(arg2, "monete moneta coins coin oro") )
	{
		send_to_char( ch, "No, non posso dare della moneta così.\r\n" );
		return;
	}

	argument = one_argument( argument, arg2 );
	if ( is_name(arg2, "a to") && !VALID_STR(argument) )
		argument = one_argument( argument, arg2 );

	victim = get_char_room( ch, arg2, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non lo vedo qui.\r\n" );
		return;
	}

	if ( ch->gold < amount )
	{
		send_to_char( ch, "Molto generoso da parte mia, ma non ho tutto quell'oro.\r\n" );
		return;
	}

	ch->gold	 -= amount;
	victim->gold += amount;
	strcpy( buf, "$n ti dà " );
	strcat( buf, arg1 );
	strcat( buf, (amount > 1) ? " monete." : " moneta." );

	act( AT_ACTION, buf, ch, NULL, victim, TO_VICT	);
	act( AT_ACTION, "$n consegna a $N dell'oro.",  ch, NULL, victim, TO_NOVICT );
	act( AT_ACTION, "Dò a $N dell'oro.",  ch, NULL, victim, TO_CHAR	);
#ifdef T2_MSP
	send_audio( victim->desc, "money.wav", TO_CHAR );
#endif
	mprog_bribe_trigger( victim, ch, amount );

	if ( !char_died(ch) )
		save_player( ch );
	if ( !char_died(victim) )
		save_player( victim );
}


DO_RET do_give( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	OBJ_DATA   *obj;
	char		str[MSL];		/* Copia di argument originale ad uso della give_money */
	char		arg1[MIL];
	char		arg2[MIL];

	if ( ms_find_obj(ch) )
		return;

	strcpy( str, argument );	/* copia argument */

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Devo dare cosa a chi?\r\n" );
		return;
	}

	if ( is_number(arg1) )
	{
		give_money( ch, str );
		return;
	}

	if ( is_name(arg2, "a to") && !VALID_STR(argument) )
		argument = one_argument( argument, arg2 );

	if ( (obj = get_obj_carry(ch, arg1, TRUE)) == NULL )
	{
		send_to_char( ch, "Non ho quell'oggetto.\r\n" );
		return;
	}

	if ( obj->wear_loc != WEARLOC_NONE )
	{
		send_to_char( ch, "Devo rimuoverlo prima.\r\n" );
		return;
	}

	victim = get_char_room( ch, arg2, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non lo vedo qui.\r\n" );
		return;
	}

	if ( !can_drop_obj(ch, obj) )
	{
		send_to_char( ch, "Non vuole venire via.\r\n" );
		return;
	}

	if ( victim->carry_number+1 > can_carry_number( victim ) )
	{
		act( AT_PLAIN, "$N ha le mani impegnate.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( victim->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_weight( victim ) )
	{
		act( AT_PLAIN, "Sembra che $N non possa portare con sè tutto quel peso.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( !can_see_obj(victim, obj, FALSE) )
	{
		act( AT_PLAIN, "$N non lo può vedere.", ch, NULL, victim, TO_CHAR );
		return;
	}

	split_obj( obj, 1 );
	obj_from_char( obj );
	act( AT_ACTION, "$n dà $p a $N.", ch, obj, victim, TO_NOVICT );
	act( AT_ACTION, "$n ti dà $p.",   ch, obj, victim, TO_VICT );
	act( AT_ACTION, "Dò $p a $N.", ch, obj, victim, TO_CHAR );
	obj = obj_to_char(obj, victim);
	mprog_give_trigger(victim, ch, obj);

	if ( !char_died(ch) )
		save_player(ch);
	if ( !char_died(victim) )
		save_player( victim );
}


/*
 * Damage an object.
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */
obj_ret damage_obj( OBJ_DATA *obj )
{
	CHAR_DATA	*ch;
	obj_ret		 objcode;

	ch = obj->carried_by;
	objcode = rNONE;

	split_obj( obj, 1 );

	act( AT_OBJECT, "($p è danneggiato)", ch, obj, NULL, TO_CHAR );		/* (TT) c'era la flag di gag qui */
	if ( obj->in_room && (ch = obj->in_room->first_person) != NULL )
	{
		act( AT_OBJECT, "($p è danneggiato)", ch, obj, NULL, TO_ROOM );
		act( AT_OBJECT, "($p è danneggiato)", ch, obj, NULL, TO_CHAR );
		ch = NULL;
	}

	if ( obj->type != OBJTYPE_LIGHT )
		oprog_damage_trigger(ch, obj);
#ifdef T2_ARENA
	else if ( !in_arena(ch) )
		oprog_damage_trigger(ch, obj);
#endif

	if ( obj_extracted(obj) )
		return global_objcode;

	switch ( obj->type )
	{
	  default:
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
		break;

	  case OBJTYPE_CONTAINER:
	  case OBJTYPE_KEYRING:
	  case OBJTYPE_QUIVER:
		if ( --obj->container->durability <= 0 )	/* (FF) condition */
		{
#ifdef T2_ARENA
			if ( !in_arena(ch) )
			{
				make_scraps( obj );
				objcode = rOBJ_SCRAPPED;
			}
			else
#endif
			{
				obj->container->durability = 1;
			}
		}
		break;

	  /* Le luci non vengono distrutte ma rimangono inutilizzabili */
	  /*(FF) fare una flag per i value di luce che indichi che la luce svanisca quando ha finito le ore */
	  case OBJTYPE_LIGHT:
		if ( --obj->light->hours <= 0 )
			obj->light->hours = 0;
		break;

	  case OBJTYPE_ARMOR:
	  case OBJTYPE_DRESS:
		if ( ch && obj->armor->ac >= 1 )
			ch->armor += apply_ac( obj, obj->wear_loc );
		if ( --obj->armor->ac <= 0 )
		{
#ifdef T2_ARENA
			/* (TT) vita dura se si attiva questo, bisognerebbe invece renderli poco utilizzabili */
			if ( !in_arena(ch) )
			{
				make_scraps(obj);
				objcode = rOBJ_SCRAPPED;
			}
			else
#endif
			{
				obj->armor->ac = 1;
				ch->armor -= apply_ac( obj, obj->wear_loc );
			}
		}
		else if ( ch && obj->armor->ac >= 1 )
		{
			ch->armor -= apply_ac( obj, obj->wear_loc );
		}
		break;

	  case OBJTYPE_WEAPON:
		if ( --obj->weapon->condition <= 0 )
		{
#ifdef T2_ARENA
			/* (TT) vita dura se si attiva questo, bisognerebbe invece renderli poco utilizzabili */
			if ( !in_arena(ch) )
			{
				make_scraps( obj );
				objcode = rOBJ_SCRAPPED;
			}
			else
#endif
				obj->weapon->condition = 1;
		}
		break;
	}

    if ( ch != NULL )
       save_player( ch );

	return objcode;
}


/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
	OBJ_DATA *obj, *tmpobj;

	if ( (obj = get_eq_char(ch, iWear)) == NULL )
		return TRUE;

	if ( !fReplace
	  && ch->carry_number + obj->count > can_carry_number(ch) )
	{
		act( AT_PLAIN, "$d: Non posso portare con me tutte quelle cose.", ch, NULL, obj->name, TO_CHAR );
		return FALSE;
	}

	if ( !fReplace )
		return FALSE;

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_NOREMOVE) )
	{
		act( AT_PLAIN, "Non posso rimuovere $p.", ch, obj, NULL, TO_CHAR );
		return FALSE;
	}

	if ( obj == get_eq_char(ch, WEARLOC_WIELD)
	  && ( tmpobj = get_eq_char(ch, WEARLOC_DUAL)) != NULL )
	{
		tmpobj->wear_loc = WEARLOC_WIELD;
	}

	unequip_char( ch, obj );

	act( AT_ACTION, "$n si sfila $p.", ch, obj, NULL, TO_ROOM );
	act( AT_ACTION, "Smetto di usare $p.", ch, obj, NULL, TO_CHAR );
	oprog_remove_trigger( ch, obj );
	return TRUE;
}


/*
 * See if char could be capable of dual-wielding
 * (FF) farlo libero, però se il pg porta due armi e non potrebbe vi saranno forti penalità nel danno e nei colpi maldestri
 */
bool could_dual( CHAR_DATA *ch )
{
	if ( IS_MOB(ch) || ch->pg->learned_skell[gsn_dual_wield] != 0 )
		return TRUE;

	return FALSE;
}


/*
 * See if char can dual wield at this time
 */
bool can_dual( CHAR_DATA *ch )
{
	if ( !could_dual(ch) )
		return FALSE;

	if ( get_eq_char(ch, WEARLOC_MISSILE) )
	{
		send_to_char( ch, "Non posso impugnare due armi quando utilizzo armi a gittata.\n\r" );
		return FALSE;
	}

	if ( get_eq_char(ch, WEARLOC_DUAL) )
	{
		send_to_char( ch, "Sto già impugnando due armi!\r\n" );
		return FALSE;
	}

	if ( get_eq_char(ch, WEARLOC_SHIELD) )
	{
		send_to_char( ch, "Ho già un braccio impegnato dallo scudo.\r\n" );
		return FALSE;
	}

	if ( get_eq_char(ch, WEARLOC_HOLD) )
	{
		send_to_char( ch, "Non posso impugnare due cose, quando sto già tenendo qualcosa!\r\n" );
		return FALSE;
	}

	return TRUE;
}


/*
 * Check to see if there is room to wear another object on this location
 * (RR) da rifare con i dress
 */
bool can_layer( CHAR_DATA *ch, OBJ_DATA *obj, int wear_loc )
{
	OBJ_DATA *otmp;

	for ( otmp = ch->first_carrying;  otmp;  otmp = otmp->next_content )
	{
		if ( otmp->wear_loc == wear_loc )
			return FALSE;
	}

	return TRUE;
}


/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 *
 * Restructured a bit to allow for specifying body location
 * & Added support for layering on certain body locations
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, int wear_bit, const char *verb )
{
	OBJ_DATA *tmpobj = NULL;
	char	  buf[MSL];
	int		  bit;
	int		  tmp;

	split_obj( obj, 1 );

	if ( get_level(ch) < obj->level )
	{
		if ( IS_ADMIN(ch))
			ch_printf( ch, "[ADM] Per %s questo oggetto bisogna essere di livello %d.\r\n", verb, obj->level );
		sprintf( buf, "Tento di %s $p, ma sono troppo inespert%c.", verb, gramm_ao(ch) );
		act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
		sprintf( buf, "$n tenta di %s $p, ma non ci riesce.", verb );
		act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
		return;
	}

	/* (RR) alcune da integrare con altre classi dopo la (CC) e magri creare una variabile apposta che sia bitvector e dia la possibilità di scegliere più ANTI_ per un oggetto (ma magari è già così) */
	if ( !IS_ADMIN(ch)
	  && (( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_WARRIOR	)
	  && ch->class == CLASS_FIGHTER						)
	  || ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_WARRIOR	)
	  && ch->class == CLASS_PALADIN						)
	  || ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_MAGE		)
	  && ch->class == CLASS_MAGE						)
	  || ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_THIEF	)
	  && ch->class == CLASS_THIEF						)
	  || ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_DRUID	)
	  && ch->class == CLASS_DRUID						)
	  || ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_WARRIOR	)
	  && ch->class == CLASS_WANDERING					)
	  || ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_CLERIC	)
	  && ch->class == CLASS_PRIEST						)) )
	{
		sprintf( buf, "Non posso %s $p.", verb );
		act( AT_MAGIC, buf, ch, obj, NULL, TO_CHAR );
		sprintf( buf, "$n tenta di %s $p, ma non ci riesce.", verb );
		act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
		return;
	}

	if ( wear_bit > -1 )
	{
		bit = wear_bit;
		if ( !HAS_BIT(obj->wear_flags,  bit) )
		{
			if ( fReplace )
			{
				switch ( bit )
				{
				  case OBJWEAR_HOLD:
					send_to_char( ch, "Non posso tenerlo.\r\n" );
					break;
				  case OBJWEAR_WIELD:
				  case OBJWEAR_MISSILE:
					send_to_char( ch, "Non posso impugnarlo.\r\n" );
					break;
				  default:
					ch_printf( ch, "Non posso indossarlo nella posizione %s.\r\n", code_name(NULL, bit, CODE_WEARFLAG) );
				}
			}
			return;
		}
	}
	else
	{
		for ( bit = -1, tmp = 1;  tmp < MAX_OBJWEAR;  tmp++ )
		{
			if ( HAS_BIT(obj->wear_flags,  tmp) )
			{
				bit = tmp;
				break;
			}
		}
	}

	/* Currently cannot have a light in non-light position */
	if ( obj->type == OBJTYPE_LIGHT )
	{
		if ( !remove_obj(ch, WEARLOC_LIGHT, fReplace) )
			return;

		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n sprigiona una luce con $p irradiando i dintorni.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Impugno $p come fonte luminosa.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_LIGHT );
		oprog_wear_trigger( ch, obj );
		return;
	}

	if ( bit == -1 )
	{
		if ( fReplace )
			ch_printf( ch, "Non posso %s quest'oggetto.\r\n", verb );
		return;
	}

	switch ( bit )
	{
	  case OBJWEAR_FINGER:
		if ( get_eq_char(ch, WEARLOC_FINGER_L)
		  && get_eq_char(ch, WEARLOC_FINGER_R)
		  && !remove_obj(ch, WEARLOC_FINGER_L, fReplace)
		  && !remove_obj(ch, WEARLOC_FINGER_R, fReplace) )
		{
			return;
		}

		if ( !get_eq_char(ch, WEARLOC_FINGER_L) )
		{
			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
			{
				act( AT_ACTION, "$n infila $p su un dito della mano sinistra.",	ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Infilo $p su un dito della mano sinistra.",  ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEARLOC_FINGER_L );
			oprog_wear_trigger( ch, obj );
			return;
		}

		if ( !get_eq_char(ch, WEARLOC_FINGER_R) )
		{
			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
			{
				act( AT_ACTION, "$n infila $p su un dito della mano destra.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Infilo $p su un dito della mano destra.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEARLOC_FINGER_R );
			oprog_wear_trigger( ch, obj );
			return;
		}

		send_log( NULL, LOG_BUG, "wear_obj: no free finger." );
		send_to_char( ch, "Sto già indossando qualcosa sulle dita.\r\n" );
		return;

	  case OBJWEAR_NECK:
		if ( get_eq_char(ch, WEARLOC_NECK) != NULL
		  && !remove_obj(ch, WEARLOC_NECK, fReplace) )
		{
			return;
		}

		if ( !get_eq_char(ch, WEARLOC_NECK) )
		{
			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
			{
				act( AT_ACTION, "$n indossa $p attorno al collo.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Indosso $p attorno al collo.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEARLOC_NECK );
			oprog_wear_trigger( ch, obj );
			return;
		}

		send_log( NULL, LOG_BUG, "wear_obj: no free neck." );
		send_to_char( ch, "Sto già indossando qualcosa attorno al collo.\r\n" );
		return;

	  case OBJWEAR_BODY:
/*		if ( !remove_obj(ch, WEARLOC_BODY, fReplace) )
		return;
*/
		if ( !can_layer(ch, obj, WEARLOC_BODY) )
		{
			send_to_char( ch, "Non riesco a infilarlo, sto già indossando qualcosa.\r\n" );
			return;
		}

		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n indossa $p al torso.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p sul torace.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_BODY );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_HEAD:
		if ( !remove_obj(ch, WEARLOC_HEAD, fReplace) )
			return;
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n si mette $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Proteggo il mio capo con $p.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_HEAD );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_EYES:
		if ( !remove_obj(ch, WEARLOC_EYES, fReplace) )
			return;
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n stringendo leggermente le palpebre, mette $p davanti a gli occhi.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Metto agli occhi $p.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_EYES );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_FACE:
		if ( !remove_obj(ch, WEARLOC_FACE, fReplace) )
			return;
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n indossa $p, nascondendogli in parte il volto.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso sul viso $p.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_FACE );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_EARS:
		if ( !remove_obj(ch, WEARLOC_EARS, fReplace) )
			return;
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n infila $p sulle orecchie.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p sulle orecchie.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_EARS );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_LEGS:
/*		if ( !remove_obj(ch, WEARLOC_LEGS, fReplace) )
			return;
*/
		if ( !can_layer(ch, obj, WEARLOC_LEGS) )
		{
			send_to_char( ch, "Non riesco a infilarli, sto già indossando qualcosa sulle gambe.\r\n" );
			return;
		}
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n indossa $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p alle gambe.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_LEGS );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_FEET:
/*		if ( !remove_obj(ch, WEARLOC_FEET, fReplace) )
			return;
*/
		if ( !can_layer(ch, obj, WEARLOC_FEET) )
		{
			send_to_char( ch, "Sto già indossando qualcosa ai piedi.\r\n" );
			return;
		}
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n reggendosi in qualche modo, indossa ai piedi $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p ai piedi.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_FEET );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_HANDS:
/*		if ( !remove_obj(ch, WEARLOC_HANDS, fReplace) )
			return;
*/
		if ( !can_layer(ch, obj, WEARLOC_HANDS) )
		{
			send_to_char( ch, "Non riesco a farlo, sto già indossando qualcosa alle mani.\r\n" );
			return;
		}
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n distende le dita, e infila $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p alle mani.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_HANDS );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_ARMS:
/*		if ( !remove_obj(ch, WEARLOC_ARMS, fReplace) )
			return;
*/
		if ( !can_layer(ch, obj, WEARLOC_ARMS) )
		{
			send_to_char( ch, "Sto già indossando qualcosa alle braccia.\r\n" );
			return;
		}
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n indossa $p sulle braccia.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p alle braccia.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_ARMS );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_ABOUT:
/*		if ( !remove_obj(ch, WEARLOC_ABOUT, fReplace) )
			return;
*/
		if ( !can_layer(ch, obj, WEARLOC_ABOUT) )
		{
			send_to_char( ch, "Sto già indossando qualcosa.\r\n" );
			return;
		}
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n circonda il corpo con $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p attorno al corpo.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_ABOUT );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_BACK:
		if ( !remove_obj( ch, WEARLOC_BACK, fReplace ) )
			return;
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n si infila $p sulla schiena.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p alla schiena.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_BACK );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_WAIST:
/*		if ( !remove_obj(ch, WEARLOC_WAIST, fReplace) )
			return;
*/
		if ( !can_layer(ch, obj, WEARLOC_WAIST) )
		{
			send_to_char( ch, "Non posso, sto già indossando qualcosa intorno alla vita.\r\n" );
			return;
		}
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n si cinge $p alla vita.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Indosso $p alla vita.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_WAIST );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_WRIST:
		if ( get_eq_char(ch, WEARLOC_WRIST_L)
		  && get_eq_char(ch, WEARLOC_WRIST_R)
		  && !remove_obj(ch, WEARLOC_WRIST_L, fReplace)
		  && !remove_obj(ch, WEARLOC_WRIST_R, fReplace) )
		{
			return;
		}

		if ( !get_eq_char(ch, WEARLOC_WRIST_L) )
		{
			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
			{
				act( AT_ACTION, "$n indossa $p al polso sinistro.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Indosso $p al polso sinistro.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEARLOC_WRIST_L );
			oprog_wear_trigger( ch, obj );
			return;
		}

		if ( !get_eq_char(ch, WEARLOC_WRIST_R) )
		{
			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
			{
				act( AT_ACTION, "$n indossa $p al polso destro.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Indosso $p al polso destro.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEARLOC_WRIST_R );
			oprog_wear_trigger( ch, obj );
			return;
		}

		send_log( NULL, LOG_BUG, "wear_obj: no free wrist." );
		send_to_char( ch, "Sto già indossando due oggetti ai polsi.\r\n" );
		return;

	  case OBJWEAR_ANKLE:
		if ( get_eq_char(ch, WEARLOC_ANKLE_L)
		  && get_eq_char(ch, WEARLOC_ANKLE_R)
		  && !remove_obj(ch, WEARLOC_ANKLE_L, fReplace)
		  && !remove_obj(ch, WEARLOC_ANKLE_R, fReplace) )
		{
			return;
		}

		if ( !get_eq_char(ch, WEARLOC_ANKLE_L) )
		{
			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
			{
				act( AT_ACTION, "$n indossa $p intorno alla caviglia sinistra.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Indosso $p intorno alla caviglia sinistra.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEARLOC_ANKLE_L );
			oprog_wear_trigger( ch, obj );
			return;
		}

		if ( !get_eq_char(ch, WEARLOC_ANKLE_R) )
		{
			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
			{
				act( AT_ACTION, "$n indossa $p intorno alla caviglia destra.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Indosso $p intorno alla caviglia destra.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEARLOC_ANKLE_R );
			oprog_wear_trigger( ch, obj );
			return;
		}

		send_log( NULL, LOG_BUG, "wear_obj: no free ankle." );
		send_to_char( ch, "Sto già indossando due oggetti sulle caviglie.\r\n" );
		return;

	  case OBJWEAR_FLOAT:
		if ( !remove_obj(ch, WEARLOC_FLOAT, fReplace) )
			return;

		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n lascia fluttuare $p accanto a sè.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Lascio fluttuare $p accanto a me.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_FLOAT );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_SHIELD:
		if ( get_eq_char(ch, WEARLOC_DUAL)
		  || (get_eq_char(ch, WEARLOC_WIELD) && get_eq_char(ch, WEARLOC_MISSILE)) )
		{
			send_to_char( ch, "E come farei ad usare uno scudo e due armi?!\r\n" );
			return;
		}

		if ( !remove_obj(ch, WEARLOC_SHIELD, fReplace) )
			return;

		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			act( AT_ACTION, "$n utilizza $p come scudo.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Utilizzo $p come scudo.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_SHIELD );
		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_MISSILE:
	  case OBJWEAR_WIELD:
		if ( !could_dual(ch) )
		{
			if ( !remove_obj(ch, WEARLOC_MISSILE, fReplace) )
				return;
			if ( !remove_obj(ch, WEARLOC_WIELD, fReplace) )
				return;
			tmpobj = NULL;
		}
		else
		{
			OBJ_DATA *mw, *dw, *hd;

			tmpobj = get_eq_char( ch, WEARLOC_WIELD );
			mw = get_eq_char( ch, WEARLOC_MISSILE );
			dw = get_eq_char( ch, WEARLOC_DUAL );
			if ( tmpobj && (mw || dw) )
			{
				send_to_char( ch, "Sto già impugnando due armi.\r\n" );
				return;
			}

			hd = get_eq_char( ch, WEARLOC_HOLD );
			if ( (mw && hd) || (tmpobj && hd) )
			{
				send_to_char( ch, "Sto già impugnando un'arma e tenendo qualcosa.\r\n" );
				return;
			}
		}

		if ( tmpobj )
		{
			if ( can_dual(ch) )	/* (FF) sarebbe da togliere questo test, tutti possono provare se son forti abbastanza, poi è la resistenza a decidere */
			{
				/* Per alzare ed impugnare un'arma serve la forza */
				if ( (get_obj_weight(obj) + get_obj_weight(tmpobj))/1000 > get_curr_attr(ch, ATTR_STR)/4 )
				{
					send_to_char( ch, "E' troppo pesante da alzare per le mie forze.\r\n" );
					return;
				}

				if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
				{
					if ( ch->right_hand == TRUE )
					{
						act( AT_ACTION, "$n impugna $p nella mano sinistra.", ch, obj, NULL, TO_ROOM );
						act( AT_ACTION, "Impugno $p nella mano sinistra.", ch, obj, NULL, TO_CHAR );
					}
					else
					{
						act( AT_ACTION, "$n impugna $p nella mano destra.", ch, obj, NULL, TO_ROOM );
						act( AT_ACTION, "Impugno $p nella mano destra.", ch, obj, NULL, TO_CHAR );
					}
				}

				if ( bit == OBJWEAR_MISSILE )
					equip_char( ch, obj, WEARLOC_MISSILE );
				else
					equip_char( ch, obj, WEARLOC_DUAL );
				oprog_wear_trigger( ch, obj );
			}
			return;
		}

		/* Per alzare ed impugnare un'arma serve la forza */
		if ( get_obj_weight(obj)/1000 > get_curr_attr(ch, ATTR_STR)/5 )
		{
			send_to_char( ch, "E' troppo pesante da alzare per le mie forze.\r\n" );
			return;
		}

		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			if ( ch->right_hand == TRUE )
			{
				act( AT_ACTION, "$n impugna $p nella mano destra.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Impugno $p nella mano destra.", ch, obj, NULL, TO_CHAR );
			}
			else
			{
				act( AT_ACTION, "$n impugna $p nella mano sinistra.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Impugno $p nella mano sinistra.", ch, obj, NULL, TO_CHAR );
			}
		}
		if ( bit == OBJWEAR_MISSILE )
		{
			equip_char( ch, obj, WEARLOC_MISSILE );
		}
		else
		{
			equip_char( ch, obj, WEARLOC_WIELD );
		}

		oprog_wear_trigger( ch, obj );
		return;

	  case OBJWEAR_HOLD:
		if ( get_eq_char(ch, WEARLOC_DUAL)
		  || (get_eq_char(ch, WEARLOC_WIELD) && get_eq_char(ch, WEARLOC_MISSILE)) )
		{
			send_to_char( ch, "Non posso tenere qualcosa e due armi!\r\n" );
			return;
		}

		if ( !remove_obj(ch, WEARLOC_HOLD, fReplace) )
			return;

		if (  obj->type == OBJTYPE_WAND
		  ||  obj->type == OBJTYPE_STAFF
		  ||  obj->type == OBJTYPE_FOOD
		  ||  obj->type == OBJTYPE_COOK
		  ||  obj->type == OBJTYPE_PILL
		  ||  obj->type == OBJTYPE_POTION
		  ||  obj->type == OBJTYPE_SCROLL
		  ||  obj->type == OBJTYPE_DRINK_CON
		  ||  obj->type == OBJTYPE_BLOOD
		  ||  obj->type == OBJTYPE_PIPE
		  ||  obj->type == OBJTYPE_HERB
		  ||  obj->type == OBJTYPE_KEY
		  || !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "$n impugna $p.",   ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Tengo $p in mano.", ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEARLOC_HOLD );
		oprog_wear_trigger( ch, obj );
		return;

	  default:
		send_log( NULL, LOG_BUG, "wear_obj: uknown/unused item_wear bit %d", bit );
		if ( fReplace )
			send_to_char( ch, "Non posso indossarlo né impugnarlo o tenerlo.\r\n" );
		return;
	}
}


void wear_wield_hold( CHAR_DATA *ch, char *argument, const char *verb )
{
	OBJ_DATA *obj;
	char	  arg1[MIL];
	char	  arg2[MIL];
	int		  wear_bit;

	if ( !VALID_STR(argument) )
	{
		ch_printf( ch, "Devo %s che cosa?\r\n", verb );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( VALID_STR(argument) && is_name(arg2, "su sopra attorno on upon around") )
		argument = one_argument( argument, arg2 );

	if ( ms_find_obj(ch) == TRUE )
		return;

	if ( !str_cmp_all(arg1, FALSE) )
	{
		OBJ_DATA *obj_next;

		for ( obj = ch->first_carrying;  obj;  obj = obj_next )
		{
			obj_next = obj->next_content;

			if ( obj->wear_loc != WEARLOC_NONE )	continue;
			if ( !can_see_obj(ch, obj, TRUE) )		continue;

			wear_obj( ch, obj, FALSE, -1, verb );
			if ( char_died(ch) )
				return;
		}
		return;
	}
	else
	{
		obj = get_obj_carry( ch, arg1, TRUE );
		if ( !obj )
		{
			send_to_char( ch, "Non posseggo quest'oggetto.\r\n" );
			return;
		}

		if ( VALID_STR(arg2) )
		{
			int		x;
			bool	found = FALSE;

			for ( x = 0;  x < MAX_OBJWEAR; x++ )
			{
				if ( !str_cmp(arg2, code_wearflag[x].name) )	/* (BB) si vabbe'... solito casino tra wearflag e objwear... è tutto da rifare in una sola table_ così siamo tutti più felici */
				{
					found = TRUE;
					break;
				}
			}

			if ( !found )
			{
				send_to_char( ch, "Su che parte del corpo dovrei indossare l'oggetto?\r\n" );
				return;
			}

			wear_bit = code_num( NULL, arg2, CODE_WEARFLAG );
		}
		else
			wear_bit = -1;

		wear_obj( ch, obj, TRUE, wear_bit, verb );
	}
}


DO_RET do_wear( CHAR_DATA *ch, char *argument )
{
	wear_wield_hold( ch, argument, "indossare" );
}

DO_RET do_wield( CHAR_DATA *ch, char *argument )
{
	wear_wield_hold( ch, argument, "impugnare" );
}

DO_RET do_hold( CHAR_DATA *ch, char *argument )
{
	wear_wield_hold( ch, argument, "tenere" );
}


DO_RET do_remove( CHAR_DATA *ch, char *argument )
{
	char	  arg[MIL];
	char	  arg2[MIL];
	OBJ_DATA *obj,
			 *obj_next;

	one_argument( argument, arg );
	one_argument( argument, arg2 );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Devo rimuovere che cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( !str_cmp_all(arg, FALSE) )
	{
		for ( obj = ch->first_carrying;  obj != NULL;  obj = obj_next )
		{
			obj_next = obj->next_content;

			/* Se l'oggetto è un vestito e non si è passato l'argomento corretto lo salta */
			if ( obj->type == OBJTYPE_DRESS && is_name_prefix(arg2, "vestito vestiti veste vesti abito abiti") )
				continue;

			if ( obj->wear_loc == WEARLOC_NONE )	continue;
			if ( !can_see_obj(ch, obj, TRUE) )	continue;

			remove_obj( ch, obj->wear_loc, TRUE );
		}
		return;
	}

	if ( (obj = get_obj_wear(ch, arg, TRUE)) == NULL )
	{
		send_to_char( ch, "Non sto usando quell'oggetto.\r\n" );
		return;
	}

	if ( (obj_next = get_eq_char(ch, obj->wear_loc)) != obj )
	{
		act( AT_PLAIN, "Devo rimuovere $p prima.", ch, obj_next, NULL, TO_CHAR );
		return;
	}

	remove_obj( ch, obj->wear_loc, TRUE );
}


DO_RET do_bury( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	int		  move;
	bool	  shovel;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Cosa dovrei seppellire?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	shovel = FALSE;
	for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
	{
		if ( obj->type == OBJTYPE_SHOVEL )
		{
			shovel = TRUE;
			break;
		}
	}

	obj = get_obj_list_rev( ch, argument, ch->in_room->last_content, FALSE );
	if ( !obj )
	{
		send_to_char( ch, "Non lo trovo.\r\n" );
		return;
	}

	split_obj( obj, 1 );

	if ( !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE)
	  && (!HAS_BIT(obj->extra_flags, OBJFLAG_CLANCORPSE) || IS_MOB(ch)) )
	{
		act( AT_PLAIN, "Non posso sotterrare $p.", ch, obj, 0, TO_CHAR );
		return;
	}

	switch ( ch->in_room->sector )
	{
	  case SECTOR_CITY:
	  case SECTOR_INSIDE:
		send_to_char( ch, "Il suolo è troppo consistente per aprire una cavità.\r\n" );
		return;
	  case SECTOR_WATER_SWIM:
	  case SECTOR_WATER_NOSWIM:
	  case SECTOR_UNDERWATER:
		send_to_char( ch, "Non posso sotterare qualcosa qui.\r\n" );
		return;
	  case SECTOR_AIR:
		send_to_char( ch, "EH!? Nel'aria?!\r\n" );
		return;
	}

	/* Confronta prendendo il massimo tra il minimo trasportabile dal pg (con forza 1)
	 *	ed il suo peso */
	if ( obj->weight > (UMAX(2000, (can_carry_weight(ch)/15)) ) && !shovel )
	{
		send_to_char( ch, "Necessito di una pala per seppellire qualcosa di così grosso.\r\n" );
		return;
	}

	/* Valori di esempio con pala, senza pala basta moltiplicarli per 5.
	 *	oggetto di 25 kg,	pg con forza 25:	move = 50
	 *	oggetto di 50 kg,	pg con forza 25:	move = 100
	 *	oggetto di 25 kg,	pg con forza 50:	move = 25
	 *	oggetto di 50 kg,	pg con forza 50:	move = 50
	 *	oggetto di 25 kg,	pg con forza 75:	move = 16
	 *	oggetto di 50 kg,	pg con forza 75:	move = 33 */
	move = ( obj->weight * 100 * (shovel  ?  1  :  5)) / UMAX(2000, can_carry_weight(ch) );
	move = URANGE( 10, move, ch->points_max[POINTS_MOVE] );
	if ( move > ch->points[POINTS_MOVE] && !stop_lowering_points(ch, POINTS_MOVE) )
	{
		send_to_char( ch, "Non ho la forza neccessaria per seppellire una cosa cosi grossa.\r\n" );
		return;
	}

	ch->points[POINTS_MOVE] -= move;

	if ( obj->type == OBJTYPE_CORPSE_MOB
	  || obj->type == OBJTYPE_CORPSE_PG )
	{
		act( AT_ACTION, "$n sotterra solennemente $p...", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Sotterro con solennità $p..", ch, obj, NULL, TO_CHAR );
	}
	else
	{
		act( AT_ACTION, "$n sotterra $p...", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Sotterro $p..", ch, obj, NULL, TO_CHAR );
	}

	SET_BIT( obj->extra_flags, OBJFLAG_BURIED );
	WAIT_STATE( ch, URANGE(PULSE_IN_SECOND*3, move *3/2, PULSE_IN_SECOND*30) );
}


/*
---------------------------------------------------------------------
 * Junk command installed by Samson 1-13-98
 * Code courtesy of Stu, from the mailing list.
 * Allows player to destroy an item in their inventory.
---------------------------------------------------------------------
 */
DO_RET do_junk( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA	*obj;
	OBJ_DATA	*obj_next;
	bool		 found = FALSE;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Cosa dovrei rompere?\r\n" );
		return;
	}

	/* Parte del comando riguardante i mob per i mudprog
	 *	- supporta l'"all."
	 *	- se trova l'oggetto come indossato distrugge anche quello
	 *	- non invia i messaggi
	 * (FF) devo trovare un modo per unirlo alla parte di sotto in maniera elegante, mi fa schifo così
	 */
	if ( IS_MOB(ch) )
	{
		char	arg[MIL];

		argument = one_argument( argument, arg );
		if ( !str_cmp_all(arg, ALL) )
		{
			for ( obj = ch->first_carrying;  obj;  obj = obj_next )
			{
				obj_next = obj->next_content;

				if ( !VALID_STR(argument) || is_name(&arg[arg[0] == 'a' ? 4 : 5], obj->name) )
				{
					if ( obj->wear_loc != WEARLOC_NONE)
						unequip_char( ch, obj );
					free_object( obj );
				}
			}
		}
		else
		{
			obj = get_obj_wear( ch, arg, TRUE );
			if ( obj )
			{
				unequip_char( ch, obj );
				free_object( obj );
				return;
			}

			obj = get_obj_carry( ch, arg, TRUE );
			if ( !obj )
				return;

			free_object( obj );
		}

		return;
	}

	/* Parte del comando junk riguardante i pg */
	for ( obj = ch->first_carrying;  obj;  obj = obj_next )
	{
		obj_next = obj->next_content;

		if ( !nifty_is_name(argument, obj->name) )	continue;
		if ( obj->wear_loc != WEARLOC_NONE )			continue;
		if ( can_see_obj(ch, obj, TRUE) )
		{
			found = TRUE;
			break;
		}
	}

	if ( found )
	{
		if ( !can_drop_obj(ch, obj) && !IS_ADMIN(ch) )
		{
			send_to_char( ch, "Non posso rompere questo oggetto, è maledetto!\n\r" );
			return;
		}

		if ( (obj->type == OBJTYPE_CORPSE_PG || obj->type == OBJTYPE_CORPSE_MOB)
		  && !IS_ADMIN(ch) )
		{
			send_to_char( ch, "Più buttato di così, si muore..\r\n" );
			return;
		}

		/* Non può farlo se è un contenitore pieno */
		if ( obj->type == OBJTYPE_CONTAINER && obj->first_content )
		{
			send_to_char( ch, "Non butto via qualcosa che contiene altri oggetti.\r\n" );
			return;
		}

		split_obj( obj, 1 );
		obj_from_char( obj );
		free_object( obj );

		act( AT_ACTION, "$n rompe $p.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Rompo $p.", ch, obj, NULL, TO_CHAR );

		return;
	}

	ch_printf( ch, "Non trovo nessun %s\r\n", argument );
}


DO_RET do_sacrifice( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Cosa dovrei sacrificare?\r\n" );
		return;
	}

	if ( !str_cmp(argument, ch->name) )
	{
		send_to_char( ch, "Sacrificare me stesso? Non mi facevo così pio. Non mi pare comunque il caso.\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	obj = get_obj_list_rev( ch, argument, ch->in_room->last_content, FALSE );
	if ( !obj )
	{
		send_to_char( ch, "Non lo trovo.\r\n" );
		return;
	}

	split_obj( obj, 1 );
	if ( !HAS_BIT( obj->wear_flags, OBJWEAR_TAKE) )
	{
		act( AT_PLAIN, "$p non è un sacrificio ammissibile.", ch, obj, 0, TO_CHAR );
		return;
	}

	if ( IS_PG(ch) && obj->weapon && HAS_BIT(obj->magic_flags, OBJMAGIC_PKDISARMED) )
    {
		if ( get_level(ch)			 - obj->weapon->lvl_disarm > 5
		  || obj->weapon->lvl_disarm - get_level(ch)		   > 5 )
		{
			send_to_char( ch, "\n\r&bA godly force freezes your outstretched hand.\n\r" );
			return;
		}
	}

	if ( obj->type == OBJTYPE_CORPSE_PG && obj->first_content )
		send_to_char( ch, "Non posso sacrificare questo cadavere.\r\n" );

	if ( ch->points[POINTS_SOUL] < ch->points_max[POINTS_SOUL]/3 )
		ch->points[POINTS_SOUL] += 1;

	send_to_char( ch, "Spero che le Divinità dei Piani sorrideranno.\r\n" );
	act( AT_ACTION, "$n offre in sacrificio $p.", ch, obj, NULL, TO_ROOM );		/* (GG) AT_SAC */

	oprog_sac_trigger( ch, obj );
	if ( obj_extracted(obj) )
		return;

	if ( cur_obj == obj->serial )
		global_objcode = rOBJ_SACCED;

	split_obj( obj, 1 );
	free_object( obj );
}


DO_RET do_brandish( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*vch;
	CHAR_DATA	*vch_next;
	OBJ_DATA	*staff;
	ch_ret		 retcode;
	int			 sn;

	if ( (staff = get_eq_char(ch, WEARLOC_HOLD)) == NULL )
	{
		send_to_char( ch, "Non sto tenendo nulla nelle mie mani.\r\n" );
		return;
	}

	if ( staff->type != OBJTYPE_STAFF )
	{
		send_to_char( ch, "Posso brandire solo dei bastoni.\r\n" );
		return;
	}

	sn = staff->wand->sn;
	if ( sn < 0 || sn >= top_sn || table_skill[sn]->spell_fun == NULL )
	{
		send_log( NULL, LOG_BUG, "do_brandish: sn errato %d.", sn );
		return;
	}

	WAIT_STATE( ch, PULSE_VIOLENCE * 2 );

	if ( staff->wand->curr_charges > 0 )
	{
		if ( !oprog_use_trigger( ch, staff, NULL, NULL, NULL ) )
		{
			act( AT_MAGIC, "$n brandisce $p.", ch, staff, NULL, TO_ROOM );
			act( AT_MAGIC, "Brandisco $p.",  ch, staff, NULL, TO_CHAR );
		}

		for ( vch = ch->in_room->first_person;  vch;  vch = vch_next )
		{
			vch_next = vch->next_in_room;

			if ( IS_ADMIN(vch) )
				continue;

			switch ( table_skill[sn]->target )
			{
			  default:
				send_log( NULL, LOG_BUG, "do_brandish: target errato per sn %d.", sn );
				return;

			  case TARGET_IGNORE:
				if ( vch != ch )
					continue;
				break;

			  case TARGET_CHAR_OFFENSIVE:
				if ( IS_MOB(ch)  ?  IS_MOB(vch)  :  IS_PG(vch) )
					continue;
				break;

			  case TARGET_CHAR_DEFENSIVE:
				if ( IS_MOB(ch)  ?  IS_PG(vch)  :  IS_MOB(vch) )
					continue;
				break;

			  case TARGET_CHAR_SELF:
				if ( vch != ch )
					continue;
				break;
			}

			retcode = obj_cast_spell( staff->wand->sn, staff->wand->level, ch, vch, NULL );
			if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
			{
				send_log( NULL, LOG_BUG, "do_brandish: pg morto" );
				return;
			}
		} /* chiude il for */
	} /* chiude l'if */

	if ( --staff->wand->curr_charges <= 0 )
	{
		act( AT_MAGIC, "$p si circonda di venature di luce color azzurro sempre piu intenso e scompare dalle mani di $n!", ch, staff, NULL, TO_ROOM );
		act( AT_MAGIC, "$p scompare dalle mie mani!", ch, staff, NULL, TO_CHAR );
		if ( staff->serial == cur_obj )
			global_objcode = rOBJ_USED;
		free_object( staff );
	}
}


DO_RET do_zap( CHAR_DATA *ch, char *argument )
{
	char	   arg[MIL];
	CHAR_DATA *victim;
	OBJ_DATA  *wand;
	OBJ_DATA  *obj;
	ch_ret	   retcode;

	if ( (wand = get_eq_char(ch, WEARLOC_HOLD)) == NULL )
	{
		send_to_char( ch, "Non sto tenendo nulla nella mie mano.\r\n" );
		return;
	}

	if ( wand->type != OBJTYPE_WAND )
	{
		send_to_char( ch, "Posso puntare solo con una bacchetta.\r\n" );
		return;
	}

	obj = NULL;

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		if ( ch->fighting )
			victim = who_fighting( ch );
		else
		{
			send_to_char( ch, "Devo puntare a chi o cosa?\r\n" );
			return;
		}
	}
	else
	{
		victim = get_char_room( ch, arg, TRUE );
		obj = get_obj_here( ch, arg );
		if ( !victim && !obj )
		{
			send_to_char( ch, "Non lo trovo qui.\r\n" );
			return;
		}
	}

	WAIT_STATE( ch, PULSE_VIOLENCE );

	if ( wand->wand->curr_charges > 0 )
	{
		if ( victim )
		{
			if ( !oprog_use_trigger( ch, wand, victim, NULL, NULL ) )
			{
				act( AT_MAGIC, "$n punta $p nella direzione di $N.", ch, wand, victim, TO_ROOM );
				act( AT_MAGIC, "Punto $p nella direzione di $N.", ch, wand, victim, TO_CHAR );
			}
		}
		else
		{
			if ( !oprog_use_trigger( ch, wand, NULL, obj, NULL ) )
			{
				act( AT_MAGIC, "$n punta $p contro $P.", ch, wand, obj, TO_ROOM );
				act( AT_MAGIC, "Punti $p contro $P.", ch, wand, obj, TO_CHAR );
			}
		}

		retcode = obj_cast_spell( wand->wand->sn, wand->wand->level, ch, victim, obj );
		if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
		{
			send_log( NULL, LOG_BUG, "do_zap: ch morto" );
			return;
		}
	}

	if ( --wand->wand->curr_charges <= 0 )
	{
		act( AT_MAGIC, "$p si vaporizza in particelle fluttuanti nell'aria.", ch, wand, NULL, TO_ROOM );
		act( AT_MAGIC, "$p esplode in frammenti.", ch, wand, NULL, TO_CHAR );
		if ( wand->serial == cur_obj )
			global_objcode = rOBJ_USED;
		free_object( wand );
	}
}


DO_RET do_eat( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA   *obj;
	char		buf[MSL];
	ch_ret		retcode;
	int			foodcond;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo mangiare cosa?\r\n" );
		return;
	}

	if ( IS_MOB(ch) || ch->pg->condition[CONDITION_FULL] > 5 )
	{
		if ( ms_find_obj(ch) )
			return;
	}

	obj = find_obj( ch, argument, TRUE );
	if ( !obj )
		return;

	if ( !IS_ADMIN(ch) )
	{
		if ( obj->type != OBJTYPE_FOOD
		  && obj->type != OBJTYPE_PILL
		  && obj->type != OBJTYPE_COOK )
		{
			act( AT_ACTION, "$n mordicchia $p. Uhmm..",  ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Tento di mordicchiare $p..", ch, obj, NULL, TO_CHAR );
			return;
		}

		if ( IS_PG(ch) && ch->pg->condition[CONDITION_FULL] > 40 )
		{
			send_to_char( ch, "Ho la pancia troppo piena per ingurgitare altro.\r\n" );
			return;
		}
	}

	/* required due to object grouping */
	split_obj( obj, 1 );
	if ( obj->in_obj )
	{
		act( AT_PLAIN, "Prendo $p da $P.", ch, obj, obj->in_obj, TO_CHAR );
		act( AT_PLAIN, "$n prende $p da $P.", ch, obj, obj->in_obj, TO_ROOM );
	}

	/* If fighting, chance of dropping food */
	if ( ch->fighting && number_percent() > get_curr_attr(ch, ATTR_AGI)*2/4 + 47 )
	{
		if ( ch->in_room->sector == SECTOR_UNDERWATER
		  || ch->in_room->sector == SECTOR_WATER_SWIM
		  || ch->in_room->sector == SECTOR_WATER_NOSWIM )
		{
			strcpy( buf, "sparisce nell'acqua" );
		}
		else if ( ch->in_room->sector == SECTOR_AIR || HAS_BIT(ch->in_room->flags, ROOM_NOFLOOR) )
			strcpy( buf, "cade giu" );
		else
			strcpy( buf, "è calpestato" );

		act( AT_MAGIC, "$n getta $p, e il suo $T.", ch, obj, buf, TO_ROOM );
		act( AT_MAGIC, "Oops, $p scivola dalla mia mano e $T!", ch, obj, buf, TO_CHAR );
	}
	else
	{
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			if ( !VALID_STR(obj->action_descr) )
			{
				act( AT_ACTION, "$n mangia $p.",  ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Mangio $p.", ch, obj, NULL, TO_CHAR );
			}
			else
			{
				actiondesc( ch, obj );
			}
		}

		switch ( obj->type )
		{
		  case OBJTYPE_COOK:
		  case OBJTYPE_FOOD:
			if ( ch->fighting )
				WAIT_STATE( ch, PULSE_IN_SECOND * 2 );
			else
				WAIT_STATE( ch, PULSE_IN_SECOND / 2 );

			if ( obj->timer > 0 && obj->food->condition > 0 )
				foodcond = (obj->timer * 10) / obj->food->condition;
			else
				foodcond = 10;

			if ( IS_PG(ch) )
			{
				int condition;

				condition = ch->pg->condition[CONDITION_FULL];
				gain_condition( ch, CONDITION_FULL, (obj->food->hours * foodcond) / 10 );
				if ( condition <= 1 && ch->pg->condition[CONDITION_FULL] > 1 )
					act( AT_ACTION, "Sono sazi$x.", ch, NULL, NULL, TO_CHAR );
				else if ( ch->pg->condition[CONDITION_FULL] > 40 )
					act( AT_ACTION, "Sono pien$x come un cinghiale.", ch, NULL, NULL, TO_CHAR );
			}

			if ( obj->food->poison != 0
			  || (foodcond < 4 && number_range(0, foodcond + 1) == 0)
			  || (obj->type == OBJTYPE_COOK && obj->food->dam_cook == 0 ) )
			{
				/* The food was poisoned! */
				AFFECT_DATA *aff;

				if ( obj->food->poison != 0 )
				{
					act( AT_POISON, "$n tossisce e sputa.", ch, NULL, NULL, TO_ROOM );
					act( AT_POISON, "Argh! *Coff* E' avvelenato!", ch, NULL, NULL, TO_CHAR );
					ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
				}
				else
				{
					act( AT_POISON, "$n tossisce su $p.", ch, obj, NULL, TO_ROOM );
					act( AT_POISON, "Argh! *Coff* E' avvelenato, sto tossendo su $p.", ch, obj, NULL, TO_CHAR );
					ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
				}

				CREATE( aff, AFFECT_DATA, 1 );
				aff->type 	 = gsn_poison;
				aff->duration  = obj->food->hours * 8 * UMAX( 1, obj->food->poison );	/* l'UMAX è per evitare valori 0 a poison */
				aff->location  = APPLY_NONE;
				aff->modifier  = 0;
				aff->bitvector = meb( AFFECT_POISON );
				affect_join( ch, aff );
			}
			break;

		  case OBJTYPE_PILL:
			mudstat.upill_val += obj->cost/100;

			if ( ch->fighting )
				WAIT_STATE( ch, PULSE_IN_SECOND * 2 );
			else
				WAIT_STATE( ch, PULSE_IN_SECOND / 2 );

			retcode = obj_cast_spell( obj->pill->sn1, obj->pill->level, ch, ch, NULL );
			if ( retcode == rNONE )
				retcode = obj_cast_spell( obj->pill->sn2, obj->pill->level, ch, ch, NULL );
			if ( retcode == rNONE )
				retcode = obj_cast_spell( obj->pill->sn3, obj->pill->level, ch, ch, NULL );
			break;
		} /* chiude lo switch */

	} /* chiude l'else */
#ifdef T2_MSP
	send_audio( ch->desc, "eatfruit.wav", TO_CHAR );
#endif
	if ( obj->serial == cur_obj )
		global_objcode = rOBJ_EATEN;

	free_object( obj );
}


DO_RET do_quaff( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA		   *obj;
	ch_ret				retcode;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Dovrei traccanare cosa?\r\n" );
		return;
	}

	obj = find_obj( ch, argument, TRUE );
	if ( !obj )
		return;

	if ( IS_PG(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
		return;

	if ( obj->type != OBJTYPE_POTION )
	{
		char	buf[MSL];

		if ( obj->type == OBJTYPE_DRINK_CON )
		{
			sprintf( buf, "beve %s", obj->name );
			send_command( ch, buf, CO );
		}
		else
		{
			act( AT_ACTION, "$n avvicina $p alle sue labbra e tenta di bere il contenuto..", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Avvicino $p alle tue labbra e provo a bere il contenuto..", ch, obj, NULL, TO_CHAR );
		}
		return;
	}

	/*
	 * Empty container check
	 */
	if ( obj->potion->sn1 == -1 && obj->potion->sn2 == -1 && obj->potion->sn3 == -1 )
	{
		send_to_char( ch, "Uhm, buono il vuoto..\r\n" );
		return;
	}

	/*
	 * Fullness checking
	 */
	if ( IS_PG(ch)
		&& (ch->pg->condition[CONDITION_FULL]   >= 48
		||  ch->pg->condition[CONDITION_THIRST] >= 48) )
	{
		send_to_char( ch, "Il mio stomaco non vuole nient'altro.\r\n" );
		return;
	}

	split_obj( obj, 1 );

	if ( obj->in_obj )
	{
		act( AT_PLAIN, "Prendo $p da $P.", ch, obj, obj->in_obj, TO_CHAR );
		act( AT_PLAIN, "$n prende $p da $P.", ch, obj, obj->in_obj, TO_ROOM );
	}

	/* If fighting, chance of dropping potion */
	if ( ch->fighting && number_percent() > get_curr_attr(ch, ATTR_AGI)*2/4 + 48 )
	{
		act( AT_MAGIC, "$n maneggia in modo maldestro $p che si disintegra in piccoli pezzi.", ch, obj, NULL, TO_ROOM );
		act( AT_MAGIC, "Oops.. $p mi è caduto dalle mani, si è rotto!", ch, obj, NULL ,TO_CHAR );
	}
	else
	{
		if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL) )
		{
			if ( obj->in_obj )
			{
				act( AT_ACTION, "$n beve d'un fiato, $p da $P.", ch, obj, obj->in_obj, TO_ROOM );
				act( AT_ACTION, "Bevo $p da $P.", ch, obj, obj->in_obj, TO_CHAR );
			}
			else
			{
				act( AT_ACTION, "$n beve d'un fiato, $p.",  ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "Bevo $p tutto d'un fiato.", ch, obj, NULL, TO_CHAR );
			}
		}

		if ( ch->fighting )
			WAIT_STATE( ch, PULSE_IN_SECOND * 2 );
		else
			WAIT_STATE( ch, PULSE_IN_SECOND / 2 );

		gain_condition( ch, CONDITION_THIRST, 1 );
		if ( IS_PG(ch) && ch->pg->condition[CONDITION_THIRST] > 43 )
			act( AT_ACTION, "Ho lo stomaco quasi pieno.", ch, NULL, NULL, TO_CHAR );

		retcode = obj_cast_spell( obj->potion->sn1, obj->potion->level, ch, ch, NULL );
		if ( retcode == rNONE )
			retcode = obj_cast_spell( obj->potion->sn2, obj->potion->level, ch, ch, NULL );
		if ( retcode == rNONE )
			retcode = obj_cast_spell( obj->potion->sn3, obj->potion->level, ch, ch, NULL );
	}
	if ( obj->vnum == VNUM_OBJ_FLASK_BREWING )
		mudstat.used_brewed++;
	else
		mudstat.upotion_val += obj->cost/100;

	if ( cur_obj == obj->serial )
		global_objcode = rOBJ_QUAFFED;

	free_object( obj );
}


DO_RET do_recite( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		   *victim;
	OBJ_DATA		   *scroll;
	OBJ_DATA		   *obj;
	char				arg1[MIL];
	char				arg2[MIL];
	ch_ret				retcode;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "Dovrei recitare cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( (scroll = get_obj_carry(ch, arg1, TRUE)) == NULL )
	{
		send_to_char( ch, "Non ho quella pergamena.\r\n" );
		return;
	}

	if ( scroll->type != OBJTYPE_SCROLL )
	{
		act( AT_ACTION, "$n impugna $p e concentrandosi tenta di recitare la formula su essa scritta..",  ch, scroll, NULL, TO_ROOM );
		act( AT_ACTION, "Impugno $p e mi concentro tentando di recitarla..", ch, scroll, NULL, TO_CHAR );
		return;
	}

	if ( IS_MOB(ch) && (scroll->vnum == VNUM_OBJ_SCROLL_SCRIBING) )
	{
		send_to_char( ch, "Non conosco questo dialetto.\r\n" );
		return;
	}

	if ( scroll->vnum == VNUM_OBJ_SCROLL_SCRIBING && get_level(ch) + 10 < scroll->scroll->level )
	{
		send_to_char( ch, "Questa pergamena è troppo complessa.\r\n" );
		return;
	}

	obj = NULL;
	if ( !VALID_STR(arg2) )
	{
		victim = ch;
	}
	else
	{
		victim = get_char_room( ch, arg2, TRUE );
		obj	 = get_obj_here ( ch, arg2 );
		if ( !victim && !obj )
		{
			send_to_char( ch, "Non trovo nulla.\r\n" );
			return;
		}
	}

	if ( scroll->vnum == VNUM_OBJ_SCROLL_SCRIBING )
		mudstat.used_scribed++;

	split_obj( scroll, 1 );
	act( AT_MAGIC, "$n recita $p.", ch, scroll, NULL, TO_ROOM );
	act( AT_MAGIC, "Tento di recitare $p.", ch, scroll, NULL, TO_CHAR );

	if ( victim != ch || ch->fighting )
		WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
	else
		WAIT_STATE( ch, PULSE_IN_SECOND / 2 );

	retcode = obj_cast_spell( scroll->scroll->sn1, scroll->scroll->level, ch, victim, obj );
	if ( retcode == rNONE )
		retcode = obj_cast_spell( scroll->scroll->sn2, scroll->scroll->level, ch, victim, obj );
	if ( retcode == rNONE )
		retcode = obj_cast_spell( scroll->scroll->sn3, scroll->scroll->level, ch, victim, obj );

	if ( scroll->serial == cur_obj )
		global_objcode = rOBJ_USED;

	free_object( scroll );
}


DO_RET do_rap( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA	*pexit;
	ROOM_DATA	*to_room;
	EXIT_DATA	*pexit_rev;
	char		*keyword;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo battere cosa?\r\n" );
		return;
	}

	if ( ch->fighting )
	{
		send_to_char( ch, "Ho altre cose da fare con le mani per il momento..\r\n" );
		return;
	}

	pexit = find_door( ch, argument, FALSE );
	if ( !pexit )
	{
		act( AT_ACTION, "Mimo il bussare una porta..", ch, NULL, NULL, TO_CHAR );
		act( AT_ACTION, "$n mima il bussare una porta.", ch, NULL, NULL, TO_ROOM );
		return;
	}

	if ( !HAS_BIT(pexit->flags, EXIT_CLOSED) )
	{
		send_to_char( ch, "Perché dovrei bussare?  E' aperta.\r\n" );
		return;
	}

	if ( HAS_BIT(pexit->flags, EXIT_SECRET) )
		keyword = "muro";		/* (GR) vorrebbe "sul" nell'act sotto */
	else
		keyword = pexit->keyword;

	act( AT_ACTION, "Batto con decisione su $d.", ch, NULL, keyword, TO_CHAR );
	act( AT_ACTION, "$n batte con decisione su $d.", ch, NULL, keyword, TO_ROOM );

	if ( (to_room = pexit->to_room) != NULL
	  && (pexit_rev = pexit->rexit) != NULL
	  && pexit_rev->to_room == ch->in_room )
	{
		CHAR_DATA *rch;

		for ( rch = to_room->first_person;  rch;  rch = rch->next_in_room )
		{
			act( AT_ACTION, "Qualcuno bussa fortemente dall'altro lato di $d.",
				rch, NULL, pexit_rev->keyword, TO_CHAR );
		}
	}
}


/*
 * Apply a salve/ointment
 * Support for applying to others.  Pkill concerns dealt with elsewhere.
 */
DO_RET do_apply( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *salve;
	OBJ_DATA  *obj;
	char	   arg1[MIL];
	char	   arg2[MIL];
	ch_ret	   retcode;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "Dovrei applicare cosa?\r\n" );
		return;
	}

	if ( ch->fighting )
	{
		send_to_char( ch, "No, in questo momento devo pensare alla mia pellaccia!\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( (salve = get_obj_carry(ch, arg1, TRUE)) == NULL )
	{
		send_to_char( ch, "Non ho nulla di tutto ciò.\r\n" );
		return;
	}

	obj = NULL;
	if ( !VALID_STR(arg2) )
	{
		victim = ch;
	}
	else
	{
		victim = get_char_room( ch, arg2, TRUE );
		obj  = get_obj_here( ch, arg2 );
		if ( !victim && !obj )
		{
			send_to_char( ch, "Devo applicare cosa a chi o cosa?\r\n" );
			return;
		}
	}

	/* (FF) apply salve to another object, probabilmente serve per applicare olii sulle armi */
	if ( obj )
	{
		send_to_char( ch, "Non lo posso fare.\r\n" );
		return;
	}

	if ( victim->fighting )
	{
		send_to_char( ch, "Negativo, sto combattendo!\r\n" );
		return;
	}

	if ( salve->type != OBJTYPE_SALVE )
	{
		if ( victim == ch )
		{
			act( AT_ACTION, "$n inizia a sfregare $p su se stess$x...", ch, salve, NULL, TO_ROOM );
			act( AT_ACTION, "Provo a sfregare $p su me stess$x...", ch, salve, NULL, TO_CHAR );
		}
		else
		{
			act( AT_ACTION, "$n inizia a sfregare $p su $N.", ch, salve, victim, TO_NOVICT );
			act( AT_ACTION, "$n inizia a sfregare $p su di me", ch, salve, victim, TO_VICT );
			act( AT_ACTION, "Tento di sfregare $p su $N.", ch, salve, victim, TO_CHAR );
		}
		return;
	}

	split_obj( salve, 1 );
	salve->salve->charges--;

	if ( !oprog_use_trigger(ch, salve, NULL, NULL, NULL) )
	{
		if ( !VALID_STR(salve->action_descr) )
		{
			if ( salve->salve->charges < 1 )
			{
				if ( victim != ch )
				{
					act( AT_ACTION, "$n sfrega $p adosso a $N.", ch, salve, victim, TO_NOVICT );
					act( AT_ACTION, "$n sfrega $p adosso a me.", ch, salve, victim, TO_VICT );
					act( AT_ACTION, "Sfrego $p adosso a $N.", ch, salve, victim, TO_CHAR );
				}
				else
				{
					act( AT_ACTION, "Sfrego $p su me stess$x.", ch, salve, NULL, TO_CHAR );
					act( AT_ACTION, "$n sfrega $p su se stess$x.", ch, salve, NULL, TO_ROOM );
				}
			}
			else
			{
				if ( victim != ch )
				{
					act( AT_ACTION, "$n sfrega $p su $N.", ch, salve, victim, TO_NOVICT );
					act( AT_ACTION, "$n sfrega $p adosso a me.", ch, salve, victim, TO_VICT );
					act( AT_ACTION, "Sfrego $p su $N.", ch, salve, victim, TO_CHAR );
				}
				else
				{
					act( AT_ACTION, "Sfrego $p contro me stess$x.", ch, salve, NULL, TO_CHAR );
					act( AT_ACTION, "$n sfrega $p contro se stess$x.", ch, salve, NULL, TO_ROOM );
				}
			}
		}
		else
			actiondesc( ch, salve );

	} /* chiude l'if */

	WAIT_STATE( ch, salve->salve->delay * 3 );	/* (WT) (TT) */
	retcode = obj_cast_spell( salve->salve->sn1, salve->salve->level, ch, victim, NULL );
	if ( retcode == rNONE )
		retcode = obj_cast_spell( salve->salve->sn2, salve->salve->level, ch, victim, NULL );
	if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
	{
		send_log( NULL, LOG_BUG, "do_apply: char died" );
		return;
	}

	if ( !obj_extracted(salve) && salve->salve->charges <= 0 )
		free_object( salve );
}


/*
 * Function to handle the state changing of a triggerobject (lever)
 */
void pullorpush( CHAR_DATA *ch, OBJ_DATA *obj, bool pull )
{
	ROOM_DATA	*room;
	ROOM_DATA	*to_room;
	EXIT_DATA	*pexit;
	EXIT_DATA	*pexit_rev;
	CHAR_DATA	*rch;
	char		*txt;
	char		 buf[MSL];
	int			 edir;
	bool		 isup;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "pullorpush: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "pullorpush: obj è NULL" );
		return;
	}

	if ( !obj->lever )
	{
		send_log( NULL, LOG_BUG, "pullorpush: l'oggetto %d non è una leva", obj->vnum );
		return;
	}

	if ( HAS_BITINT(obj->lever->flags, TRIG_UP) )
		isup = TRUE;
	else
		isup = FALSE;

	switch ( obj->type )
	{
	  default:
		ch_printf( ch, "Non posso %s!\r\n", pull  ?  "tirare"  :  "spingere" );
		return;
		break;

	  case OBJTYPE_SWITCH:
	  case OBJTYPE_LEVER:
	  case OBJTYPE_PULLCHAIN:
		if ( (!pull && isup) || (pull && !isup) )
		{
			ch_printf( ch, "E' già %s.\r\n", isup  ?  "sù"  :  "giù" );
			return;
		}

	  case OBJTYPE_BUTTON:
		if ( (!pull && isup) || (pull && !isup) )
		{
			ch_printf( ch, "E' già %s.\r\n", isup  ?  "attivo"  :  "disattivo" );
			return;
		}
		break;
	} /* chiude lo switch */

	if ( (pull) && has_trigger(obj->first_mudprog, MPTRIGGER_PULL) )
	{
		if ( !HAS_BITINT(obj->lever->flags, TRIG_AUTORETURN) )
			REMOVE_BITINT( obj->lever->flags, TRIG_UP );
		oprog_pull_trigger( ch, obj );
		return;
	}

	if ( (!pull) && has_trigger(obj->first_mudprog, MPTRIGGER_PUSH) )
	{
		if ( !HAS_BITINT(obj->lever->flags, TRIG_AUTORETURN) )
			SET_BITINT( obj->lever->flags, TRIG_UP );
		oprog_push_trigger( ch, obj );
		return;
	}

	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
		sprintf( buf, "$n %s $p.", pull ? "spinge" : "preme" );
		act( AT_ACTION, buf,  ch, obj, NULL, TO_ROOM );
		sprintf( buf, "%s $p.", pull ? "Spingi" : "Premi" );
		act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
	}

	if ( !HAS_BITINT(obj->lever->flags, TRIG_AUTORETURN) )
	{
		if ( pull )
			REMOVE_BITINT( obj->lever->flags, TRIG_UP );
		else
			SET_BITINT( obj->lever->flags, TRIG_UP );
	}

	if ( HAS_BITINT(obj->lever->flags, TRIG_TELEPORT)
	  || HAS_BITINT(obj->lever->flags, TRIG_TELEPORTALL)
	  || HAS_BITINT(obj->lever->flags, TRIG_TELEPORTPLUS) )
	{
		int flags = 0;

		room = get_room_index( NULL, obj->lever->vnum_sn );
		if ( !room )
		{
			send_log( NULL, LOG_BUG, "pullorpush: obj punta ad una stanza non valida %d", obj->lever->vnum_sn );
			return;
		}

		if ( HAS_BITINT(obj->lever->flags, TRIG_SHOWROOMDESC) )	SET_BITINT( flags, TELE_SHOWDESC );
		if ( HAS_BITINT(obj->lever->flags, TRIG_TELEPORTALL ) )	SET_BITINT( flags, TELE_TRANSALL );
		if ( HAS_BITINT(obj->lever->flags, TRIG_TELEPORTPLUS) )	SET_BITINT( flags, TELE_TRANSALLPLUS );

		teleport( ch, obj->lever->vnum_sn, flags );
		return;
	}

	if ( HAS_BITINT(obj->lever->flags, TRIG_RAND4)
	  || HAS_BITINT(obj->lever->flags, TRIG_RAND6) )
	{
		int maxd;

		room = get_room_index( NULL, obj->lever->vnum_sn );
		if ( !room )
		{
			send_log( NULL, LOG_BUG, "pullorpush: obj punta ad una stanza non valida %d", obj->lever->vnum_sn );
			return;
		}

		if ( HAS_BITINT(obj->lever->flags, TRIG_RAND4) )
			maxd = 3;
		else
			maxd = 5;

		randomize_exits( room, maxd );
		for ( rch = room->first_person;  rch;  rch = rch->next_in_room )
		{
			send_to_char( rch, "Avverto un forte suono simile a un rombo.\r\n" );
			send_to_char( rch, "Qualcosa è cambiato..\r\n" );
		}
	}

	if ( HAS_BITINT(obj->lever->flags, TRIG_DOOR) )
	{
		room = get_room_index( NULL, obj->lever->vnum_sn );
		if ( !room )
			room = obj->in_room;

		if ( !room )
		{
			send_log( NULL, LOG_BUG, "pullorpush: obj punta ad una stanza non valida %d", obj->lever->vnum_sn );
			return;
		}

		if		( HAS_BITINT(obj->lever->flags, TRIG_D_NORTH) )
		{
			edir = DIR_NORTH;
			txt = "verso nord";
		}
		else if ( HAS_BITINT(obj->lever->flags, TRIG_D_SOUTH) )
		{
			edir = DIR_SOUTH;
			txt = "verso sud";
		}
		else if ( HAS_BITINT(obj->lever->flags, TRIG_D_EAST) )
		{
			edir = DIR_EAST;
			txt = "verso est";
		}
		else if ( HAS_BITINT(obj->lever->flags, TRIG_D_WEST) )
		{
			edir = DIR_WEST;
			txt = "verso ovest";
		}
		else if ( HAS_BITINT(obj->lever->flags, TRIG_D_UP) )
		{
			edir = DIR_UP;
			txt = "verso l'alto";
		}
		else if ( HAS_BITINT(obj->lever->flags, TRIG_D_DOWN) )
		{
			edir = DIR_DOWN;
			txt = "verso il basso";
		}
		else
		{
			send_log( NULL, LOG_BUG, "pullorpush: door: nessuna direzione-flag impostata." );
			return;
		}

		pexit = get_exit( room, edir );
		if ( !pexit )
		{
			if ( !HAS_BITINT(obj->lever->flags, TRIG_PASSAGE) )
			{
				send_log( NULL, LOG_BUG, "pullorpush: obj punta ad una non-uscita %d", obj->lever->vnum_sn );
				return;
			}
			to_room = get_room_index( NULL, obj->lever->vnum );

			if ( !to_room )
			{
				send_log( NULL, LOG_BUG, "pullorpush: dest punta ad una stanza non valida %d", obj->lever->vnum );
				return;
			}

			pexit = make_exit( room, to_room, edir );
			DISPOSE( pexit->keyword );
			pexit->keyword		= str_dup( "" );
			DISPOSE( pexit->description );
			pexit->description	= str_dup( "" );
			pexit->key			= -1;
			pexit->flags	= 0;
			top_exit++;

			act( AT_PLAIN, "Noto che si apre un passaggio!", ch, NULL, NULL, TO_CHAR );
			act( AT_PLAIN, "Un passaggio si apre!", ch, NULL, NULL, TO_ROOM );
			return;
		} /* chiude l'if */

		if ( HAS_BITINT(obj->lever->flags, TRIG_UNLOCK)
		  && HAS_BIT(pexit->flags, EXIT_LOCKED) )
		{
			REMOVE_BIT(pexit->flags, EXIT_LOCKED);

			act( AT_PLAIN, "Odo dei deboli scatti $T.", ch, NULL, txt, TO_CHAR );
			act( AT_PLAIN, "Odi dei debolissimi scatti $T.", ch, NULL, txt, TO_ROOM );

			if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
				REMOVE_BIT( pexit_rev->flags, EXIT_LOCKED );

			return;
		}

		if ( HAS_BITINT(obj->lever->flags, TRIG_LOCK)
		  && !HAS_BIT( pexit->flags, EXIT_LOCKED) )
		{
			SET_BIT( pexit->flags, EXIT_LOCKED) ;

			act( AT_PLAIN, "Avverto dei deboli scatti $T.", ch, NULL, txt, TO_CHAR );
			act( AT_PLAIN, "Avverti dei debolissimi scatti $T.", ch, NULL, txt, TO_ROOM );

			if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
				SET_BIT( pexit_rev->flags, EXIT_LOCKED );

			return;
		}
		if ( HAS_BITINT( obj->lever->flags, TRIG_OPEN   )
		  && HAS_BIT( pexit->flags, EXIT_CLOSED) )
		{
			REMOVE_BIT(pexit->flags, EXIT_CLOSED);

			for ( rch = room->first_person;  rch;  rch = rch->next_in_room )
				act( AT_ACTION, "Un $d si apre.", rch, NULL, pexit->keyword, TO_CHAR );

			if ( (pexit_rev = pexit->rexit) != NULL  && pexit_rev->to_room == ch->in_room )
			{
				REMOVE_BIT( pexit_rev->flags, EXIT_CLOSED );
				for ( rch = pexit->to_room->first_person;  rch;  rch = rch->next_in_room )
					act( AT_ACTION, "Una $d si apre.", rch, NULL, pexit_rev->keyword, TO_CHAR );	/* (GR) articolo della cosa che si apre */
			}

			if ( edir >= DIR_NORTH && edir < DIR_SOMEWHERE )
				check_room_for_traps( ch, table_dir[edir].trap_door );
			return;
		}
		if ( HAS_BITINT( obj->lever->flags, TRIG_CLOSE   )
		  && !HAS_BIT( pexit->flags, EXIT_CLOSED) )
		{
			SET_BIT(pexit->flags, EXIT_CLOSED);

			for ( rch = room->first_person;  rch;  rch = rch->next_in_room )
				act( AT_ACTION, "$d si chiude.", rch, NULL, pexit->keyword, TO_CHAR );

			if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
			{
				SET_BIT( pexit_rev->flags, EXIT_CLOSED );
				for ( rch = pexit->to_room->first_person;  rch;  rch = rch->next_in_room )
					act( AT_ACTION, "$d si chiude.", rch, NULL, pexit_rev->keyword, TO_CHAR );
			}

			if ( edir >= DIR_NORTH && edir < DIR_SOMEWHERE )
				check_room_for_traps( ch, table_dir[edir].trap_door );
			return;
		}
	} /* chiude l'if */
}


DO_RET do_pull( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo tirare cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	obj = get_obj_here( ch, argument );
	if ( !obj )
	{
		act( AT_PLAIN, "Non c'è nessun $T qui.", ch, NULL, argument, TO_CHAR );
		return;
	}

	pullorpush( ch, obj, TRUE );
}


DO_RET do_push( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo spingere cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	obj = get_obj_here( ch, argument );
	if ( !obj )
	{
		act( AT_PLAIN, "Non c'è nessun $T qui.", ch, NULL, argument, TO_CHAR );
		return;
	}

	pullorpush( ch, obj, FALSE );
}
