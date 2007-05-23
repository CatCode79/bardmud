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
 >						Modulo movimenti giovatore							<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "autoquest.h"
#include "command.h"
#include "db.h"
#include "dream.h"
#include "editor.h"
#include "fight.h"
#include "gramm.h"
#include "object_handler.h"
#include "interpret.h"
#include "make_obj.h"
#include "movement.h"
#include "mprog.h"
#include "msp.h"
#include "nanny.h"
#include "object_interact.h"
#include "room.h"
#include "skills.h"


TELEPORT_DATA	*first_teleport = NULL;
TELEPORT_DATA	*last_teleport	= NULL;


/*
 * Tabella con tutte le informazioni sulle direzioni
 */
const struct dir_type table_dir[] =
{
/*		code				 								eng 			from				to						rev_dir			trap_door */
	{ { DIR_NORTH,		"DIR_NORTH",		"nord"		},	"north",		"da nord",			"verso nord",			DIR_SOUTH,		TRAPFLAG_N		},
	{ { DIR_EAST,		"DIR_EAST",			"est"		},	"east",			"da est",			"verso est",			DIR_WEST,		TRAPFLAG_E		},
	{ { DIR_SOUTH,		"DIR_SOUTH",		"sud"		},	"south",		"da sud",			"verso sud",			DIR_NORTH,		TRAPFLAG_S		},
	{ { DIR_WEST,		"DIR_WEST",			"ovest"		},	"west",			"da ovest",			"verso ovest",			DIR_EAST,		TRAPFLAG_W		},
	{ { DIR_UP,			"DIR_UP",			"alto"		},	"up",			"dall'alto",		"verso l'alto",			DIR_DOWN,		TRAPFLAG_U		},
	{ { DIR_DOWN,		"DIR_DOWN",			"basso"		},	"down",			"dal basso",		"verso il basso",		DIR_UP,			TRAPFLAG_D		},
	{ { DIR_NORTHEAST,	"DIR_NORTHEAST",	"nordest"	},	"northeast",	"da nordest",		"verso nordest",		DIR_SOUTHWEST,	TRAPFLAG_NE		},
	{ { DIR_NORTHWEST,	"DIR_NORTHWEST",	"nordovest"	},	"northwest",	"da nordovest",		"verso nordovest",		DIR_SOUTHEAST,	TRAPFLAG_NW		},
	{ { DIR_SOUTHEAST,	"DIR_SOUTHEAST",	"sudest"	},	"southeast",	"da sudest",		"verso sudest",			DIR_NORTHWEST,	TRAPFLAG_SE		},
	{ { DIR_SOUTHWEST,	"DIR_SOUTHWEST",	"sudovest"	},	"southwest",	"da sudovest",		"verso sudovest",		DIR_NORTHEAST,	TRAPFLAG_SW		},
	{ { DIR_SOMEWHERE,	"DIR_SOMEWHERE",	"altrove"	},	"somewhere",	"da qualche parte",	"verso qualche parte",	DIR_SOMEWHERE,	-1				}
};
const int max_check_dir = sizeof(table_dir)/sizeof(struct dir_type);


/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.
 */
EXIT_DATA *get_exit( ROOM_DATA *room, int dir )
{
	EXIT_DATA *xit;

	if ( !room )
	{
		send_log( NULL, LOG_BUG, "get_exit: room è NULL" );
		return NULL;
	}

	for ( xit = room->first_exit;  xit;  xit = xit->next )
	{
		if ( xit->vdir == dir )
			return xit;
	}

	return NULL;
}


/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to( ROOM_DATA *room, int dir, VNUM vnum )
{
	EXIT_DATA *xit;

	if ( !room )
	{
		send_log( NULL, LOG_BUG, "get_exit_to: room è NULL" );
		return NULL;
	}

	for ( xit = room->first_exit;  xit;  xit = xit->next )
	{
		if ( xit->vdir == dir && xit->vnum == vnum )
			return xit;
	}

	return NULL;
}


/*
 * Function to get the nth exit of a room
 */
EXIT_DATA *get_exit_num( ROOM_DATA *room, int count )
{
	EXIT_DATA *xit;
	int		   cnt;

	if ( !room )
	{
		send_log( NULL, LOG_BUG, "get_exit_num: room è NULL" );
		return NULL;
	}

	for ( cnt = 0, xit = room->first_exit;  xit;  xit = xit->next )
	{
		if ( ++cnt == count )
			return xit;
	}

	return NULL;
}


/*
 * Modifica il movimento per l''ingombro.
 */
int encumbrance( CHAR_DATA *ch, int move )
{
	int cur, max;

	max = can_carry_weight( ch );
	cur = ch->carry_weight;

	if		( cur >= max		)	return move * 4;
	else if ( cur >= max * 0.95 )	return move * 3.5;
	else if ( cur >= max * 0.90 )	return move * 3;
	else if ( cur >= max * 0.85 )	return move * 2.5;
	else if ( cur >= max * 0.80 )	return move * 2;
	else if ( cur >= max * 0.75 )	return move * 1.5;
	else							return move;
}


/*
 * Controlla che il pg possa andare in quella direzione.
 */
bool can_go_char( CHAR_DATA *ch, int door )
{
	EXIT_DATA *ex;

	ex = get_exit( ch->in_room, door );

	if ( !ex )
		return FALSE;

	if ( ex->to_room == NULL )
		return FALSE;

	if ( HAS_BIT(ex->flags, EXIT_CLOSED) )
		return FALSE;

	return TRUE;
}


/*
 * Controlla che un oggetto possa andare in quella direzione.
 */
bool can_go_obj( OBJ_DATA *ch, int door )
{
	EXIT_DATA *ex;

	if ( (ex = get_exit(ch->in_room, door)) == NULL )
		return FALSE;

	if ( ex->to_room == NULL )
		return FALSE;

	if ( HAS_BIT(ex->flags, EXIT_CLOSED) )
		return FALSE;

	return TRUE;
}


/*
 * Check to see if a character can fall down, checks for looping
 */
bool will_fall( CHAR_DATA *ch, int fall )
{
	if ( (ch->in_room && HAS_BIT(ch->in_room->flags, ROOM_NOFLOOR))
	  && can_go_char(ch, DIR_DOWN)
	  && (!HAS_BIT(ch->affected_by, AFFECT_FLYING)
	  || (ch->mount && !HAS_BIT(ch->mount->affected_by, AFFECT_FLYING))) )
	{
		if ( fall > 80 )
		{
			send_log( NULL, LOG_BUG, "will_fall: (loop?) più di 80 stanze: vnum %d", ch->in_room->vnum );
			char_from_room(ch);
			char_to_room( ch, get_room_index(NULL, VNUM_ROOM_TEMPLE) );
			fall = 0;
			return TRUE;
		}
		set_char_color( AT_FALLING, ch );
		send_to_char( ch, "Sto precipitando!\r\n" );
		move_char( ch, get_exit(ch->in_room, DIR_DOWN), ++fall, FALSE );
		return TRUE;
	}

	return FALSE;
}


/*
 * Make objects in rooms that are nofloor fall
 */
void obj_fall( OBJ_DATA *obj, bool through )
{
	EXIT_DATA	*pexit;
	ROOM_DATA	*to_room;
	static int	 fall_count;
	static bool	 is_falling;	/* Stop loops from the call to obj_to_room() */

	if ( !obj->in_room || is_falling )
		return;

	if ( fall_count > 30 )
	{
		send_log( NULL, LOG_BUG, "obj_fal: l'oggetto sta cadendo in più di 30 stanze." );
		free_object( obj );
		fall_count = 0;
		return;
	}

	if ( HAS_BIT(obj->in_room->flags, ROOM_NOFLOOR)
	  && can_go_obj(obj, DIR_DOWN)
	  && !HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
	{

		pexit = get_exit( obj->in_room, DIR_DOWN );
		to_room = pexit->to_room;

		if ( through )
			fall_count++;
		else
			fall_count = 0;

		if ( obj->in_room == to_room )
		{
			send_log( NULL, LOG_BUG, "obj_fall: l'oggetto è caduto nella stessa stanza %d", to_room->vnum );
			free_object( obj );
			return;
		}

		if ( obj->in_room->first_person )
		{
			act( AT_PLAIN, "$p precipita verso il basso..", obj->in_room->first_person, obj, NULL, TO_ROOM );
			act( AT_PLAIN, "$p precipita verso il basso..", obj->in_room->first_person, obj, NULL, TO_CHAR );
		}
		obj_from_room( obj );
		is_falling = TRUE;
		obj = obj_to_room( obj, to_room );
		is_falling = FALSE;

		if ( obj->in_room->first_person )
		{
			act( AT_PLAIN, "$p precipita dall'alto.", obj->in_room->first_person, obj, NULL, TO_ROOM );
			act( AT_PLAIN, "$p precipita dall'alto.", obj->in_room->first_person, obj, NULL, TO_CHAR );
		}

		if ( !HAS_BIT(obj->in_room->flags, ROOM_NOFLOOR) && through )
		{
			int		dam;
			bool	destroy = FALSE;

			/* Calcola pseudo-realistico per emulare l'attrito nella caduta */
			dam = ((fall_count <= 4) ? fall_count : 4) * obj->weight/1000;

			/* Damage players */
			if ( obj->in_room->first_person && number_percent() > 15 )
			{
				CHAR_DATA *rch;
				CHAR_DATA *vch = NULL;
				int		   chcnt = 0;

				for ( rch = obj->in_room->first_person;  rch;  rch = rch->next_in_room, chcnt++ )
				{
					if ( number_range(0, chcnt) == 0 )
						vch = rch;
				}
				act( AT_WHITE, "$p precipita colpendo $n!", vch, obj, NULL, TO_ROOM );
				act( AT_WHITE, "$p precipita colpendomi!", vch, obj, NULL, TO_CHAR );

				if ( IS_MOB(vch) && HAS_BIT_ACT(vch, MOB_HARDHAT) )
					act( AT_WHITE, "$p rimbalza inoffensivo sul mio capo!", vch, obj, NULL, TO_CHAR );
				else
					damage( vch, vch, dam * get_level(vch)/2, TYPE_UNDEFINED );
			}

			/* Damage objects */
			switch ( obj->type )
			{
			  case OBJTYPE_WEAPON:
				if ( obj->weapon->condition - dam <= 0 )
					destroy = TRUE;
				else
					obj->weapon->condition -= dam;
				break;

			  case OBJTYPE_ARMOR:
			  case OBJTYPE_DRESS:
				if ( obj->armor->ac - dam <= 0 )
					destroy = TRUE;
				else
					obj->armor->ac -= dam;
				break;

			  default:
				if ( dam * 15 > get_obj_resistance(obj) )
					destroy = TRUE;
/*				else
					(FF) in futuro quando ci sarà la condition per tutti gli oggetti qui bisogna inserirci l'else */
				break;
			}

			if ( destroy )
			{
				if ( obj->in_room->first_person )
				{
					act( AT_PLAIN, "$p si è distrutto dalla caduta!",
						obj->in_room->first_person, obj, NULL, TO_ROOM );
					act( AT_PLAIN, "$p è distrutto dalla caduta!",
						obj->in_room->first_person, obj, NULL, TO_CHAR );
				}
				make_scraps( obj );
			}
		} /* chiude l'if */
		obj_fall( obj, TRUE );
	} /* chiude l'if */
}


/*
 * (FF) se si passa l'argomento cambia il tipo di ritorno, casa, cabala o pietra lunare
 */
DO_RET do_recall( CHAR_DATA *ch, char *argument )
{
#ifdef T2_ARENA
	if ( in_arena(ch) )
	{
		send_to_char( ch, "Mi è impossibile tornare in un luogo sicuro da un'arena.\r\n" );
		return;
	}
#endif

	if ( get_level(ch) > LVL_NEWBIE && !IS_ADMIN(ch) )
	{
		int cost;

		if ( ch->fighting )
		{
			send_to_char( ch, "Sto combattendo, mi è impossibile tornare in un luogo sicuro.\r\n" );
			return;
		}

		if ( IS_PG(ch) )
		{
			cost = get_oneirism_cost( ch, 20 );
			if ( ch->pg->glory < cost )
			{
				send_to_char( ch, "Non ho abbastanza gloria per poter ritornare in un luogo sicuro.\r\n" );
				return;
			}
	
			send_to_char( ch, "Entro in trance di delirio onirico astrale per tornare in un luogo sicuro.\r\n" );
			ch->pg->glory -= cost;
		}
	}

	WAIT_STATE( ch, PULSE_IN_SECOND * 2 );
	spell_word_of_recall( -1, -1, ch, NULL );
}


void update_dt( void )
{
	DESCRIPTOR_DATA *d;

	for ( d = first_descriptor;  d;  d = d->next )
	{
		CHAR_DATA *ch;

		ch = d->character;

		if ( d->connected != CON_PLAYING )	continue;
		if ( !ch )							continue;
		if ( IS_MOB(ch) )					continue;
		if ( IS_ADMIN(ch) )					continue;

		/* Calcola il tributo che un pg deve pagare per passare la DT */
		if ( HAS_BIT(ch->in_room->flags, ROOM_DEATH) )
		{
			act( AT_DEAD, "$n cade in una trappola senza uscita!", ch, NULL, NULL, TO_ROOM );
			set_char_color( AT_DEAD, ch );
			send_to_char( ch, "Dannazione! Sono in una trappola senza uscita!\r\n" );

			death_penalty( ch, DEATH_TRAP );

			spell_word_of_recall( -1, -1, ch, NULL );
		}
	}
}


/*
---------------------------------------------------------------------
 * Run command taken from DOTD codebase - Samson 2-25-99
 * Added argument to let players specify how far to run.
 * Fixed an infinite loop bug where somehow a closed door would cause problems.
 * Supressed display of rooms/terrain until you stop to prevent buffer overflows - Samson 4-16-01
---------------------------------------------------------------------
 * (FF) questo snippet aveva un supporto per la wild, se serve aggiungerlo
 */
DO_RET do_run( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*from_room;
	EXIT_DATA	*pexit;
	char		 arg[MIL];
	int			 amount = 0;
	int			 x;
    bool		 limited = FALSE;

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Devo correre verso dove?\r\n" );
		return;
	}

	if ( ch->position != POSITION_STAND && ch->position != POSITION_MOUNT )
	{
		send_to_char( ch, "Non posso certo correre in questa posizione!\r\n" );
		return;
	}

	if ( VALID_STR(argument) )
	{
		if ( is_number(argument) )
		{
			limited = TRUE;
			amount = atoi( argument );
		}
		else
		{
			send_to_char( ch, "Quanti scatti dovrei eseguire?\r\n" );
		}
	}

	from_room = ch->in_room;

	if ( limited )
	{
		for ( x = 1;  x <= amount;  x++ )
		{
			ch_printf( ch, "x %d\r\n", x );

			pexit = find_door( ch, arg, TRUE );
			if ( pexit )
			{
				if ( ch->points[POINTS_MOVE] < 1 )
				{
					send_to_char( ch, "Sono troppo esausto per correre qui e là.\r\n" );
#ifdef T2_MSP
					send_audio( ch->desc, "move0.wav", TO_CHAR );
#endif
					ch->points[POINTS_MOVE] = 0;
					break;
				}

				if ( move_char(ch, pexit, 0, TRUE) == rSTOP )
					break;
			}
		}
	}
	else
	{
		while ( (pexit = find_door(ch, arg, TRUE)) != NULL )
		{
			if ( ch->points[POINTS_MOVE] < 1 )
			{
				send_to_char( ch, "Sono troppo esausto per correre qui e là.\r\n" );
#ifdef T2_MSP
				send_audio( ch->desc, "move0.wav", TO_CHAR );
#endif
				ch->points[POINTS_MOVE] = 0;
				break;
			}

			if ( move_char(ch, pexit, 0, TRUE) == rSTOP )
				break;
		}
	}

	if ( ch->in_room == from_room )
	{
		send_to_char( ch, "Cerco di correre da qualche parte ma.. dove?\r\n" );
		act( AT_ACTION, "$n corre in giro senza una meta precisa pare.", ch, NULL, NULL, TO_ROOM );
		return;
	}

	send_to_char( ch, "Rallento, che corsa!\r\n" );
	act( AT_ACTION, "$n rallenta dopo aver corso per un po'.", ch, NULL, NULL, TO_ROOM );

	send_command( ch, "look", CO );
}


ch_ret move_char( CHAR_DATA *ch, EXIT_DATA *pexit, int fall, bool running )
{
	ROOM_DATA	*in_room;
	ROOM_DATA	*to_room;
	ROOM_DATA	*from_room;
	OBJ_DATA	*boat;
	char		*txt;
	char		 buf[MSL];
	int			 rdir;		/* direzione inversa */
	int			 door;
	int			 distance;
	ch_ret		 retcode;
	bool		 drunk	  = FALSE;
	bool		 brief	  = FALSE;

	if ( IS_PG(ch) )
	{
		if ( is_drunk(ch, 2) && (ch->position != POSITION_SHOVE) && (ch->position != POSITION_DRAG) )
			drunk = TRUE;
	}

	if ( drunk && !fall )
	{
		door = number_door( );
		pexit = get_exit( ch->in_room, door );
	}

#ifdef DEBUG_MOVECHAR
	if ( pexit )
	{
		send_log( NULL, LOG_MONI, "move_char: %s va verso la direzione %d",
			ch->name, pexit->vdir );
	}
#endif

	retcode = rNONE;		/* (TT) o rSTOP? */
	txt = NULL;

	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_MOUNTED) )
		return retcode;

	in_room = ch->in_room;
	from_room = in_room;
	if ( !pexit || (to_room = pexit->to_room) == NULL )
	{
		if ( drunk && ch->position != POSITION_MOUNT
		  && ch->in_room->sector != SECTOR_WATER_SWIM
		  && ch->in_room->sector != SECTOR_WATER_NOSWIM
		  && ch->in_room->sector != SECTOR_UNDERWATER
		  && ch->in_room->sector != SECTOR_OCEANFLOOR )
		{
			switch ( number_bits(4) )
			{
			  default:
				act( AT_ACTION, "Ascidhenti! *Hic* Non riescio a manenermi in piedi.. per pocco non urtavo..", ch, NULL, NULL, TO_CHAR );
				act( AT_ACTION, "$n oscilla pericolosamente inciampando ovunque..", ch, NULL, NULL, TO_ROOM );
				break;
			  case 3:
				act( AT_ACTION, "Coscia? *Hic* Noo! Che cadutha! Insciampare così.. che rovjna!", ch, NULL, NULL, TO_CHAR );
				act( AT_ACTION, "$n incespica vistosamente.. inciampa e cade a terra!", ch, NULL, NULL, TO_ROOM );
				ch->position = POSITION_REST;
				break;
			  case 4:
				act( AT_HURT, "Argh! *Burp* Coscera? Ho urtato qualchosa.. che mal di testia.. sveng.. .. *THUD*", ch, NULL, NULL, TO_CHAR );
				act( AT_ACTION, "$n avanzando a tentoni si abbatte contro qualcosa..", ch, NULL, NULL, TO_ROOM );
				act( AT_ACTION, "$n cade a terra con un sordo 'THUD'.", ch, NULL, NULL, TO_ROOM );
				ch->position = POSITION_INCAP;
				break;
			}
		}
		else if ( drunk )
			act( AT_ACTION, "Cercho di farmi strada.. ma non è fascile inqueste condiscioni, ho bevu totropppo.", ch, NULL, NULL, TO_CHAR );
		else
			send_to_char( ch, "Qualcosa mi blocca il cammino.. non da questa parte.\r\n" );

		return rSTOP;
	}

	door = pexit->vdir;
	distance = pexit->distance;

	/* Se l'uscita è una finestra non c'è modo di oltrepassarla a meno che non sia una porta-finestra */
	if (  HAS_BIT(pexit->flags, EXIT_WINDOW)
	  && !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
	{
		send_to_char( ch, "Qualcosa mi blocca il cammino.. non da questa parte.\r\n" );
		return rSTOP;
	}

	if ( HAS_BIT(pexit->flags, EXIT_PORTAL) && IS_MOB(ch) )
	{
		act( AT_PLAIN, "No, non sono un Viaggiatore dei Piani, non posso passare.", ch, NULL, NULL, TO_CHAR );
		return rSTOP;
	}

	if ( HAS_BIT(pexit->flags, EXIT_NOMOB) && IS_MOB(ch) )
	{
		act( AT_PLAIN, "No, non posso passare da lì.", ch, NULL, NULL, TO_CHAR );
		return rSTOP;
	}

	/* Se la dir è chiusa
	 *	e se non si è affetti da passdoor e non si è spirito
	 * oppure la dir è nopassdoor */
	if ( HAS_BIT(pexit->flags, EXIT_CLOSED)
	  && ((!HAS_BIT(ch->affected_by, AFFECT_PASS_DOOR) && !(IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SPIRIT)))
	  || HAS_BIT(pexit->flags, EXIT_NOPASSDOOR)) )
	{
		if ( !HAS_BIT(pexit->flags, EXIT_SECRET) && !HAS_BIT(pexit->flags, EXIT_DIG) )
		{
			if ( drunk )
			{
				act( AT_PLAIN, "$n attraversa $d barcollando penosamente.", ch,
					NULL, pexit->keyword, TO_ROOM );
				act( AT_PLAIN, "Nonostante l'ubbriacatura attra versare $d non è coscì difficile..", ch,
					NULL, pexit->keyword, TO_CHAR );
			}
			else
			{
				act( AT_PLAIN, "Devo aprire $d se voglio passare.", ch, NULL, pexit->keyword, TO_CHAR );
			}
		}
		else
		{
			if ( drunk )
				send_to_char( ch, "Ho decisciamente bevutto troppo.. faccio fathica a rimanere inequi librio.\r\n" );
			else
				send_to_char( ch, "Qualcosa mi blocca il cammino.. non da questa parte.\r\n" );
		}

		return rSTOP;
	}

	/* (FF) Crazy virtual room idea convertita in sistema di distanza uscita,
	 *	probabilmente ciuccia maggiore movimento */
	if ( distance > 0 )
	{
		/* (FF) */
	}

	if ( !fall
	  && HAS_BIT(ch->affected_by, AFFECT_CHARM)
	  && ch->master
	  && in_room == ch->master->in_room )
	{
		send_to_char( ch, "No! Non voglio stare lontano dal mio Maestro.\r\n" );
		return rSTOP;
	}
#ifdef T2_MSP
	check_audio_location( ch );
#endif
#ifdef T2_ALFA
	if ( ch->in_room->area != to_room->area && !IS_ADMIN(ch) && IS_PG(ch) )
	{
		/* (RR) Ecco, qui devo aggiungere il sistema di enter di area senza il livelo hard, ma con messaggi che indichino se l'area è pericolosa o meno */
		set_char_color( AT_, ch );
		switch ( to_room->area->level_min - get_level(ch) )
		{
		  case 1:
			send_to_char( ch, "Non posso ancora camminare fin lì. Non ancora.\r\n" );
			break;
		  case 2:
			send_to_char( ch, "Presto o tardi camminerò lì. Ma non ancora, non ancora..\r\n" );
			break;
		  case 3:
			send_to_char( ch, "Non posso andare lì. Non è il mio posto.\r\n" );
			break;
		  default:
			send_to_char( ch, "Non posso andare lì. Non posso.\r\n" );
			break;
		}
	} /* chiude l'if */
#endif

	if ( !fall && IS_PG(ch) )
	{
		int move;

#ifdef T2_ALFA
		/* Prevent xxx from entering a xxx-flagged area (FF) tengo la traccia per possibili futuri utilizzi */
		if (  )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "\r\nQualcosa mi trattiene.. non posso andare li'!\r\n" );
			return rSTOP;
		}
#endif

		if ( in_room->sector == SECTOR_AIR
			|| to_room->sector == SECTOR_AIR
			|| HAS_BIT(pexit->flags, EXIT_FLY) )
		{
			if ( ch->mount && !HAS_BIT(ch->mount->affected_by, AFFECT_FLYING) )
			{
				send_to_char( ch, "Questa cavalcatura non può volare!\r\n" );
				return rSTOP;
			}

			if ( !ch->mount && !HAS_BIT(ch->affected_by, AFFECT_FLYING) )
			{
				send_to_char( ch, "Dovrei avere delle ali, e funzionanti, per arrivare lì.\r\n" );
				return rSTOP;
			}
		}

		if ( in_room->sector == SECTOR_WATER_NOSWIM
		  || to_room->sector == SECTOR_WATER_NOSWIM )
		{
			if ( (ch->mount && !is_floating(ch->mount)) || !is_floating(ch) )
			{
				/*
				 * Look for a boat.
				 * We can use the boat obj for a more detailed description.
				 */
				boat = get_objtype_carry( ch, OBJTYPE_BOAT, TRUE, FALSE );
				if ( boat )
				{
					if ( drunk )
						txt = "naviga rollando";
					else
						txt = "naviga";
				}
				else
				{
					if ( ch->mount )
						send_to_char( ch, "La mia cavalcatura non può nuotare! Affogheremmo!\r\n" );
					else
						send_to_char( ch, "Ho bisogno di un mezzo.. una barca.. una canoa..\r\n" );
					return rSTOP;
				}
			}
		} /* chiude l'if */

		if ( HAS_BIT(pexit->flags, EXIT_CLIMB) )
		{
			bool found;

			found = FALSE;
			if		( ch->mount && HAS_BIT(ch->mount->affected_by, AFFECT_FLYING) )
				found = TRUE;
			else if ( HAS_BIT(ch->affected_by, AFFECT_FLYING) )
				found = TRUE;

			if ( !found && !ch->mount )
			{
				if ( (IS_PG(ch) && number_percent() > knows_skell(ch, gsn_climb))
					|| drunk || ch->mental_state < -90 )
				{
					send_to_char( ch, "Ho perso la presa! Cado!\r\n" );
					learn_skell( ch, gsn_climb, FALSE );
					if ( pexit->vdir == DIR_DOWN )	/* oppure mandarlo nella direzione opposta a cui stava andando, come se scivolasse (FF) */
						return move_char( ch, pexit, 1, running );

					set_char_color( AT_HURT, ch );
					send_to_char( ch, "OUCH! Che caduta..\r\n" );
					WAIT_STATE( ch, PULSE_IN_SECOND * 5 );
					return damage( ch, ch, (pexit->vdir == DIR_UP)  ?  10  :  5, TYPE_UNDEFINED );
				}
				found = TRUE;
				learn_skell( ch, gsn_climb, TRUE );
				WAIT_STATE( ch, table_skill[gsn_climb]->beats * 3 );	/* (WT) */
				txt = "scala";
			}

			if ( !found )
			{
				send_to_char( ch, "Non posso scalare nulla qui..\r\n" );
				return rSTOP;
			}
		} /* chiude l'if */

		if ( ch->mount )
		{
			switch ( ch->mount->position )
			{
			  case POSITION_DEAD:
				send_to_char( ch, "No! La mia cavalcatura! Povera bestia.. Pace all'anima sua.\r\n" );
				return rSTOP;
				break;

			  case POSITION_MORTAL:
			  case POSITION_INCAP:
				send_to_char( ch, "La mia cavalcatura non ce la fa.. troppe ferite.. fatica..\r\n" );
				return rSTOP;
				break;

			  case POSITION_STUN:
				send_to_char( ch, "Devo lasciare che la mia cavalcatura si riprenda, sembra stordita.\r\n" );
				return rSTOP;
				break;

			  case POSITION_SLEEP:
				send_to_char( ch, "La mia cavalcatura sta dormendo.. meglio che la svegli prima.\r\n" );
				return rSTOP;
				break;

			  case POSITION_REST:
				send_to_char( ch, "La mia cavalcatura sta riposando.. meglio che la faccia alzare.\r\n" );
				return rSTOP;
				break;

			  case POSITION_SIT:
				send_to_char( ch, "Dovrò far alzare in piedi la mia cavalcatura prima di procedere.\r\n" );
				return rSTOP;
				break;

			  default:
				break;
			}

			if ( !is_floating(ch->mount) )
				move = table_sector[in_room->sector].move_loss;
			else
				move = 1;

			/* Qui non è stato messo apposta la funzione stop_lowering_points */
			if ( ch->mount->points[POINTS_MOVE] < move )
			{
				send_to_char( ch, "La mia cavalcatura è esausta, non ce la fa.\r\n" );
				return rSTOP;
			}
		}
		else
		{
			if ( !is_floating(ch) )
			{
				move = encumbrance( ch, table_sector[in_room->sector].move_loss );
				if ( ch->class == CLASS_WANDERING
				  && (in_room->sector == SECTOR_FOREST
				  ||  in_room->sector == SECTOR_MOUNTAIN) )
				{
					move--;
				}
			}
			else
			{
				move = 1;
			}

			if ( ch->points[POINTS_MOVE] < move && !stop_lowering_points(ch, POINTS_MOVE) )
			{
#	ifdef T2_MSP
				send_audio( ch->desc, "move0.wav", TO_CHAR );
#	endif
				send_to_char( ch, "Non ce la faccio.. che stanchezza!\r\n" );
				return rSTOP;
			}
		}

		if ( running )
		{
			WAIT_STATE( ch, move * 3/2 );	/* (WT) */
			move *= 2;
		}
		else
			WAIT_STATE( ch, move * 3 );

		if ( ch->mount )
			ch->mount->points[POINTS_MOVE] -= move;
		else
			ch->points[POINTS_MOVE] -= move;
	} /* chiude l'if */

	/*
	 * Check if player can fit in the room
	 */
	if ( to_room->tunnel > 0 )
	{
		CHAR_DATA *ctmp;
		int count = ch->mount ? 1 : 0;

		for ( ctmp = to_room->first_person;  ctmp;  ctmp = ctmp->next_in_room )
		{
			if ( ++count >= to_room->tunnel )
			{
				if ( ch->mount && count == to_room->tunnel )
					send_to_char( ch, "Dovrò lasciare la mia cavalcatura qui se voglio entrare.\r\n" );
				else
					send_to_char( ch, "Non riesco ad entrare.. troppo poco spazio!\r\n" );
				return rSTOP;
			}
		}
	}

	if ( ch->mount )
	{
		bool unhorse = FALSE;

		switch ( ch->in_room->sector )
		{
		  default:
			send_log( NULL, LOG_BUG, "move_char: settore inesistente o errato per il controllo ch->mount: %d", ch->in_room->sector );
			break;

		  case SECTOR_INSIDE:
		  case SECTOR_CITY:
		  case SECTOR_FIELD:
		  case SECTOR_WATER_SWIM:
			if ( !can_use_skill(ch, number_percent(), gsn_mount) && number_range(0, 20) == 0 )
			{
				act( AT_SKILL, "Improvvisamente la mia cavalcatura, $N, si imbizzarrisce.", ch, NULL, ch->mount, TO_CHAR );
				act( AT_SKILL, "$N mi disarciona e cado dalla sella.", ch, NULL, ch->mount, TO_CHAR );
				act( AT_SKILL, "Improvvisamente la cavalcatura di $n, $N, si imbizzarrisce.", ch, NULL, ch->mount, TO_NOVICT );
				act( AT_SKILL, "$N disarciona $n.", ch, NULL, ch->mount, TO_NOVICT );
				act( AT_SKILL, "Improvvisamente mi vien voglia di imbizzarrirmi e disarcionare $n.", ch, NULL, ch->mount, TO_VICT );
				act( AT_SKILL, "Riesco a disarcionare quell'impedito del mio padrone.", ch, NULL, ch->mount, TO_VICT );
				unhorse = TRUE;
			}
			break;

		  /* Controlla solo con i settori più difficili da cavacare */
		  case SECTOR_FOREST:
		  case SECTOR_HILLS:
		  case SECTOR_MOUNTAIN:
		  case SECTOR_WATER_NOSWIM:
		  case SECTOR_UNDERWATER:
		  case SECTOR_AIR:
		  case SECTOR_DESERT:
		  case SECTOR_DUNNO:
		  case SECTOR_OCEANFLOOR:
		  case SECTOR_UNDERGROUND:
		  case SECTOR_LAVA:
		  case SECTOR_SWAMP:
			if ( !can_use_skill(ch, number_percent(), gsn_mount) && number_range(0, 10) == 0 )
			{
				act( AT_SKILL, "Il terreno è infido e cado da sopra $N.", ch, NULL, ch->mount, TO_CHAR );
				act( AT_SKILL, "$n fallisce nel tentativo di evitare le insidie del terreno e cade da sella.", ch, NULL, ch->mount, TO_NOVICT );
				act( AT_SKILL, "$n cerca di evitare le insidie del terreno ma io mica lo aiuto, è un somaro.", ch, NULL, ch->mount, TO_VICT );
				unhorse = TRUE;
			}
			break;
		};

		/* Disarciona */
		if ( unhorse )
		{
			ch->points[POINTS_LIFE] -= number_range( ch->mount->level/2, ch->mount->level*2 );
			REMOVE_BIT( ch->mount->act, MOB_MOUNTED );
			ch->mount = NULL;
			ch->position = number_range(0,1) ? POSITION_REST : POSITION_SIT;
			return rSTOP;
		}
	}

	/* Controlla se la lunghezza dei capelli del pg gli permettono di camminare (RR) farla come funzione a parte tanto per alleggerire la move_char */
	if ( IS_PG(ch) && !IS_ADMIN(ch) && ch->pg->hair && ch->pg->hair->length > ch->height )
	{
		int perc_len;

		/* Calcola la percentuale di lunghezza dei capelli rispetto l'altezza del pg */
		perc_len = ch->pg->hair->length * ch->height / 100;
		/* Il massimo è quando i capelli superano l'altezza del pg del 20% */
		if ( perc_len >= 120 )
			perc_len = 120;

		if ( number_range(perc_len, 130) == 130 )
		{
			char	*phrase_floor;

			if ( HAS_BIT(ch->affected_by, AFFECT_FLOATING) || HAS_BIT(ch->affected_by, AFFECT_FLYING) )
				send_to_char( ch, "I miei capelli ondeggiano leggermente nell'aria.\r\n" );
			else
			{
				if ( number_range(0, 1) == 0 )
				{
					ch_printf( ch, "Ahi!! Inciampo sui miei capelli e mi ritrovo distes%c.\r\n", gramm_ao(ch) );
					act( AT_ACTION, "$n inciampa sui suoi capelli e si ritrova per terra!", ch, NULL, NULL, TO_ROOM );
					ch->points[POINTS_LIFE] -= number_range( 0, get_level(ch) / 8 );
					ch->position = POSITION_REST;	/* (FF) funzioncina di caduta posizione random? rest, knee, sit etc etc */
					WAIT_STATE( ch, PULSE_IN_SECOND );
					return rSTOP;
				}

				/* Se non cade e se non vola crea dei messaggi appositi di strusciamento capelli per terra (FF) magari spostare queste frasi nella table_sector, visto che ogni settore ne ha una, mi sa che se continua così dovrò creare un file con il caricamento di tutti settori */
				switch ( ch->in_room->sector )
				{
				  default:
					send_log( NULL, LOG_BUG, "move_char: settore mancante nel check dei capelli lunghi: %d", ch->in_room->sector );
					phrase_floor = "strusciano per terra";
					break;
				  case SECTOR_INSIDE:
					phrase_floor = "strusciano sul pavimento";
					break;
				  case SECTOR_CITY:
					phrase_floor = "strusciano le vie cittadine";
					break;
				  case SECTOR_FIELD:
					phrase_floor = "strusciano sull'erba";
					break;
				  case SECTOR_FOREST:
					phrase_floor = "strusciano sulle foglie";
					break;
				  case SECTOR_HILLS:
					phrase_floor = "strusciano sulla terra";
					break;
				  case SECTOR_MOUNTAIN:
					phrase_floor = "strusciano sulla terra";
					break;
				  case SECTOR_WATER_SWIM:
				  case SECTOR_WATER_NOSWIM:
					phrase_floor = "s'inzuppano d'acqua";
					break;
				  case SECTOR_UNDERWATER:
				  case SECTOR_OCEANFLOOR:
					phrase_floor = "galleggiano liberi nell'acqua";
					break;
				  case SECTOR_AIR:
					phrase_floor = "vengono scompigliati dal vento";
					break;
				  case SECTOR_DESERT:
				  case SECTOR_DUNNO:
					phrase_floor = "si riempiono di sabbia";
					break;
				  case SECTOR_UNDERGROUND:
					phrase_floor = "strusciano per terra";
					break;
				  case SECTOR_LAVA:
					phrase_floor = "si bruciacchiano al contatto con la lava!";
					ch->points[POINTS_LIFE] -= number_range( 0, get_level(ch) / 16 );
					ch->pg->hair->length -= number_range( 0, 3 );
					break;
				  case SECTOR_SWAMP:
					phrase_floor = "si inzuppano di fanghiglia";
					break;
				}
				ch_printf( ch, "I miei capelli %s.\r\n", phrase_floor );
				act( AT_ACTION, "I capelli di $n $t.", ch, phrase_floor, NULL, TO_ROOM );
			}
		}
	}

	/* check for traps on exit - later */
	if ( IS_MOB(ch) || ch->pg->incognito == 0 )
	{
		if ( fall )
			txt = "cade";
		else if ( !txt )
		{
			if ( ch->mount )
			{
				if ( HAS_BIT(ch->mount->affected_by, AFFECT_FLOATING) )
					txt = "fluttua";
				else if ( HAS_BIT(ch->mount->affected_by, AFFECT_FLYING) )
					txt = "vola";
				else
					txt = "cavalca";
			}
			else
			{
				if ( HAS_BIT(ch->affected_by, AFFECT_FLOATING) )
				{
					if ( drunk )
						txt = "fluttua instabilmente";
					else
						txt = "vola";
				}
				else if ( HAS_BIT(ch->affected_by, AFFECT_FLYING) )
				{
					if ( drunk )
						txt = "vola storto";
					else
						txt = "vola";
				}
				else if ( ch->position == POSITION_SHOVE )
					txt = "è spintonato";
				else if ( ch->position == POSITION_DRAG )
					txt = "è trascinato";
				else
				{
					if ( drunk )
						txt = "barcolla";
					else
						txt = "va";
				}
			}
		}

		if ( !HAS_BIT(ch->affected_by, AFFECT_SNEAK) )
		{
			if ( ch->mount )
			{
				sprintf( buf, "$n %s %s in groppa a $N.", txt, table_dir[door].to );
				act( AT_ACTION, buf, ch, NULL, ch->mount, TO_NOVICT );
			}
			else
			{
				sprintf( buf, "$n %s %s.", txt, table_dir[door].to );
				act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
			}
		}
	}

	rprog_leave_trigger( ch );
	if ( char_died(ch) )
		return global_retcode;

	char_from_room( ch );

	if ( ch->mount )
	{
		rprog_leave_trigger( ch->mount );

		if ( char_died(ch) )	/* (TT) Codice associato alla modifica sopra */
			return global_retcode;

		if ( ch->mount )
		{
			char_from_room( ch->mount );
			char_to_room( ch->mount, to_room );
		}
	}

	char_to_room( ch, to_room );

	if ( IS_MOB(ch) || ch->pg->incognito == 0 )
	{
		if ( fall )
			txt = "cade";
		else if ( ch->mount )
		{
			if ( HAS_BIT(ch->mount->affected_by, AFFECT_FLOATING) )
				txt = "fluttuando";
			else if ( HAS_BIT(ch->mount->affected_by, AFFECT_FLYING) )
				txt = "volando";
			else
				txt = "cavalcando";
		}
		else
		{
			if ( HAS_BIT(ch->affected_by, AFFECT_FLOATING) )
			{
				if ( drunk )
					txt = "fluttua instabilmente";
				else
					txt = "fluttua";
			}
			else if ( HAS_BIT(ch->affected_by, AFFECT_FLYING) )
			{
				if ( drunk )
					txt = "vola storto";
				else
					txt = "vola";
			}
			else if ( ch->position == POSITION_SHOVE )
			{
				txt = "spintonato";
			}
			else if ( ch->position == POSITION_DRAG )
			{
				txt = "trascinato";
			}
			else
			{
				if ( drunk )
					txt = "barcolla";
				else
					txt = "arriva";
			}
		}

		rdir = table_dir[door].rev_dir;
	
		if ( !HAS_BIT(ch->affected_by, AFFECT_SNEAK) )
		{
			if ( ch->mount )
			{
				sprintf( buf, "$n arriva %s %s sopra $N.", txt, table_dir[rdir].from );
				act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
			}
			else
			{
				sprintf( buf, "$n %s %s.", txt, table_dir[rdir].from );
				act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
			}
		}
	}

	if ( IS_PG(ch) && !IS_ADMIN(ch)
	  && ch->in_room->area != to_room->area )
	{
		if ( get_level(ch) < to_room->area->level_low )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch,"Avverto uno strano sentore qui intorno.. non mi piace..\r\n" );
		}
		else if ( get_level(ch) > to_room->area->level_high )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Questo luogo non m'ispira niente di buono..\r\n" );
		}
	}

	/* Make sure everyone sees the room description of death traps */
	if ( IS_PG(ch) && HAS_BIT(ch->in_room->flags, ROOM_DEATH) )
	{
		if ( HAS_BIT_PLR(ch, PLAYER_BRIEF) )
			brief = TRUE;
		REMOVE_BIT( ch->pg->flags, PLAYER_BRIEF );
	}

	if ( !running )
		send_command( ch, "look", CO );

	if ( brief )
		SET_BIT( ch->pg->flags, PLAYER_BRIEF );

	/* BIG ugly looping problem here when the character is mptransed back to the starting room.
	 * To avoid this, check how many chars are in the room at the start and stop processing
	 *	followers after doing the right number of them.
	 */
	if ( !fall )
	{
		CHAR_DATA	*fch;
		CHAR_DATA	*nextinroom;
		int			 chars = 0;
		int			 count = 0;

		for ( fch = from_room->first_person;  fch;  fch = fch->next_in_room )
			chars++;

		for ( fch = from_room->first_person;  fch && ( count < chars );  fch = nextinroom )
		{
			nextinroom = fch->next_in_room;

			count++;
			if ( fch != ch && fch->master == ch
			  && (fch->position == POSITION_STAND || fch->position == POSITION_MOUNT) )	/* (TT) mount */
			{
				act( AT_ACTION, "Seguo $N.", fch, NULL, ch, TO_CHAR );
				move_char( fch, pexit, 0, running );
			}
		}
	}

	if ( ch->in_room->first_content )
		retcode = check_room_for_traps( ch, TRAPFLAG_ENTER_ROOM );

	if ( retcode != rNONE )	/* (TT) o rSTOP? */
		return retcode;

	if ( char_died(ch) )
		return retcode;

	mprog_entry_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	rprog_enter_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	mprog_greet_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	oprog_greet_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	if ( !will_fall(ch, fall) && fall > 0 )
	{
		if ( !HAS_BIT(ch->affected_by, AFFECT_FLOATING)
		  || (ch->mount && !HAS_BIT(ch->mount->affected_by, AFFECT_FLOATING)) )
		{
			set_char_color( AT_HURT, ch );
			send_to_char( ch, "Argh! Finalmente terra! Ma che botta..\r\n" );
			WAIT_STATE( ch, PULSE_IN_SECOND * 5 );
			retcode = damage( ch, ch, 20 * fall, TYPE_UNDEFINED );
		}
		else
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Scivoli lentamente giù posandoti a terra.\r\n" );
		}
	}

	return retcode;
}


/*
 * Funzione che gestisce tutti i 10 comandi di movimento subito sotto
 */
DO_RET direction_handler( CHAR_DATA *ch, char *argument, const int dir )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "direction_handler: ch è NULL" );
		return;
	}

	if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "direction_handler: stanza del ch %s è NULL",
			(IS_PG(ch)) ? ch->name : ch->short_descr );
		return;
	}

	/* i valori DIR_SOMEWHERE non li accetta */
	if ( dir < 0 || dir >= DIR_SOMEWHERE )
	{
		send_log( NULL, LOG_BUG, "direction_handler: direzione passata errata con ch %s: %d",
			(IS_PG(ch)) ? ch->name : ch->short_descr, dir );
		return;
	}

    /* Anche se il valore dell'uscita viene NULL è ok, lo gestisce poi la move_char() */
	move_char( ch, get_exit(ch->in_room, dir), 0, FALSE );
}

DO_RET do_north( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_NORTH );
}

DO_RET do_east( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_EAST );
}

DO_RET do_south( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_SOUTH );
}

DO_RET do_west( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_WEST );
}

DO_RET do_up( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_UP );
}

DO_RET do_down( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_DOWN );
}

DO_RET do_northeast( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_NORTHEAST );
}

DO_RET do_northwest( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_NORTHWEST );
}

DO_RET do_southeast( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_SOUTHEAST );
}

DO_RET do_southwest( CHAR_DATA *ch, char *argument )
{
	direction_handler( ch, argument, DIR_SOUTHWEST );
}


EXIT_DATA *find_door( CHAR_DATA *ch, char *arg, bool quiet )
{
	EXIT_DATA *pexit;
	int		   door;

	if ( !VALID_STR(arg) )
		return NULL;

	pexit = NULL;
	if		( !str_prefix(arg, "north"	  ) || !str_prefix(arg, "nord"	   ) )	door = DIR_NORTH;
	else if ( !str_prefix(arg, "east"	  ) || !str_prefix(arg, "est"	   ) )	door = DIR_EAST;
	else if ( !str_prefix(arg, "south"	  ) || !str_prefix(arg, "sud"	   ) )	door = DIR_SOUTH;
	else if ( !str_prefix(arg, "west"	  ) || !str_prefix(arg, "ovest"	   ) )	door = DIR_WEST;
	else if ( !str_prefix(arg, "up"		  ) || !str_prefix(arg, "alto"	   ) )	door = DIR_UP;
	else if ( !str_prefix(arg, "down"	  ) || !str_prefix(arg, "basso"	   ) )	door = DIR_DOWN;
	else if ( !str_prefix(arg, "northeast") || !str_prefix(arg, "nordest"  ) || !str_cmp(arg, "ne")						   )	door = DIR_NORTHEAST;
	else if ( !str_prefix(arg, "northwest") || !str_prefix(arg, "nordovest") || !str_cmp(arg, "nw")						   )	door = DIR_NORTHWEST;	/* Inutile fare il controllo su "no" tanto sarebbe inutile perché c'è prima il nord */
	else if ( !str_prefix(arg, "southeast") || !str_prefix(arg, "sudest"   ) || !str_cmp(arg, "se")						   )	door = DIR_SOUTHEAST;
	else if ( !str_prefix(arg, "southwest") || !str_prefix(arg, "sudovest" ) || !str_cmp(arg, "sw") || !str_cmp(arg, "so") )	door = DIR_SOUTHWEST;
	else
	{
		/* Ricerca esatta della keyword */
		for ( pexit = ch->in_room->first_exit;  pexit;  pexit = pexit->next )
		{
			if ( !quiet && !HAS_BIT(pexit->flags, EXIT_ISDOOR) )	continue;
			if ( !VALID_STR(pexit->keyword) )						continue;

			if ( nifty_is_name(arg, pexit->keyword) )
				return pexit;
		}

		/* Ricerca prefissa della keyword se l'argomento passato è abbastanza lungo */
		if ( strlen(arg) >= 2 )
		{
			for ( pexit = ch->in_room->first_exit;  pexit;  pexit = pexit->next )
			{
				if ( !quiet && !HAS_BIT(pexit->flags, EXIT_ISDOOR) )	continue;
				if ( !VALID_STR(pexit->keyword) )						continue;

				if ( nifty_is_name_prefix(arg, pexit->keyword) )
					return pexit;
			}
		}

		if ( !quiet )
			act( AT_PLAIN, "Non vedo $T qui.", ch, NULL, arg, TO_CHAR );

		return NULL;
	}

	pexit = get_exit( ch->in_room, door );
	if ( !pexit )
	{
		if ( !quiet )
			act( AT_PLAIN, "Non vedo $T qui.", ch, NULL, arg, TO_CHAR );
		return NULL;
	}

	if ( quiet )
		return pexit;

	if ( HAS_BIT(pexit->flags, EXIT_SECRET) )
	{
		act( AT_PLAIN, "Non vedo $T qui.", ch, NULL, arg, TO_CHAR );
		return NULL;
	}

	if ( !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
	{
		send_to_char( ch, "Non posso farlo.\r\n" );
		return NULL;
	}

	return pexit;
}


/*
 * Ritorna la direzione di una porta basandosi sul testo passato in arg.
 * Diverso da find_door, questa ultima cerca anche tra le keyword delle varie uscite.
 */
int get_door( const char *arg )
{
	if ( !VALID_STR(arg) )
		return -1;

	/* Ricerca esatta prima, permette di digitare no prima che venga interpretatao come nord */
	if		( !str_cmp(arg, "north"		) || !str_cmp(arg, "nord"		) )	return 0;
	else if ( !str_cmp(arg, "east"		) || !str_cmp(arg, "est"		) )	return 1;
	else if ( !str_cmp(arg, "south"		) || !str_cmp(arg, "sud"		) )	return 2;
	else if ( !str_cmp(arg, "west"		) || !str_cmp(arg, "ovest"		) )	return 3;
	else if ( !str_cmp(arg, "up"		) || !str_cmp(arg, "alto"		) )	return 4;
	else if ( !str_cmp(arg, "down"		) || !str_cmp(arg, "basso"		) )	return 5;
	else if ( !str_cmp(arg, "northeast"	) || !str_cmp(arg, "nordest"	) || !str_cmp(arg, "ne")						)	return 6;
	else if ( !str_cmp(arg, "northwest"	) || !str_cmp(arg, "nordovest"	) || !str_cmp(arg, "nw") || !str_cmp(arg, "no")	)	return 7;
	else if ( !str_cmp(arg, "southeast"	) || !str_cmp(arg, "sudest"		) || !str_cmp(arg, "se")						)	return 8;
	else if ( !str_cmp(arg, "southwest"	) || !str_cmp(arg, "sudovest"	) || !str_cmp(arg, "sw") || !str_cmp(arg, "so") )	return 9;

	/* Ricerca prefissa */
	if		( !str_prefix(arg, "north"	  ) || !str_prefix(arg, "nord"	   ) )	return 0;
	else if ( !str_prefix(arg, "east"	  ) || !str_prefix(arg, "est"	   ) )	return 1;
	else if ( !str_prefix(arg, "south"	  ) || !str_prefix(arg, "sud"	   ) )	return 2;
	else if ( !str_prefix(arg, "west"	  ) || !str_prefix(arg, "ovest"	   ) )	return 3;
	else if ( !str_prefix(arg, "up"		  ) || !str_prefix(arg, "alto"	   ) )	return 4;
	else if ( !str_prefix(arg, "down"	  ) || !str_prefix(arg, "basso"	   ) )	return 5;
	else if ( !str_prefix(arg, "northeast") || !str_prefix(arg, "nordest"  ) )	return 6;
	else if ( !str_prefix(arg, "northwest") || !str_prefix(arg, "nordovest") )	return 7;
	else if ( !str_prefix(arg, "southeast") || !str_prefix(arg, "sudest"   ) )	return 8;
	else if ( !str_prefix(arg, "southwest") || !str_prefix(arg, "sudovest" ) )	return 9;

	return -1;
}

/*
 * Controlla che la direzione passata sia almeno una direzione in inglese
 */
bool check_direction_arg( const char *dir )
{
	if ( !str_cmp(dir, "north")
	  || !str_cmp(dir, "east")
	  || !str_cmp(dir, "south")
	  || !str_cmp(dir, "west")
	  || !str_cmp(dir, "up")
	  || !str_cmp(dir, "down")
	  || !str_cmp(dir, "northeast")
	  || !str_cmp(dir, "northwest")
	  || !str_cmp(dir, "southeast")
	  || !str_cmp(dir, "southwest") )
	{
		return TRUE;
	}

	return FALSE;
}   
    
    
void set_bexit_flag( EXIT_DATA *pexit, int flag )
{   
	EXIT_DATA *pexit_rev;
    
	SET_BIT(pexit->flags, flag);
	if ( (pexit_rev = pexit->rexit) != NULL
		&& pexit_rev != pexit )
	{
		SET_BIT( pexit_rev->flags, flag );
	}
}


void remove_bexit_flag( EXIT_DATA *pexit, int flag )
{
	EXIT_DATA *pexit_rev;

	REMOVE_BIT(pexit->flags, flag);
	if ( (pexit_rev = pexit->rexit) != NULL
		&& pexit_rev != pexit )
	{
		REMOVE_BIT( pexit_rev->flags, flag );
	}
}


void toggle_bexit_flag( EXIT_DATA *pexit, int flag )
{
	EXIT_DATA *pexit_rev;

	TOGGLE_BIT(pexit->flags, flag);
	if ( (pexit_rev = pexit->rexit) != NULL
		&& pexit_rev != pexit )
	{
		TOGGLE_BIT( pexit_rev->flags, flag );
	}
}


DO_RET do_open( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	EXIT_DATA *pexit;
	int door;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Aspetta.. cosa devo aprire?\r\n" );
		return;
	}

	pexit = find_door( ch, argument, TRUE );	/* Le porte segrete le gestisce la find_door() */
	if ( pexit )
	{
		/* 'open door' */
		EXIT_DATA *pexit_rev;

		if ( !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
		{
			send_to_char( ch, "Non posso farlo.\r\n" );
			return;
		}
		if ( !HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "Ah! ma è già aperto.\r\n" );
			return;
		}
		if ( HAS_BIT(pexit->flags, EXIT_LOCKED) && HAS_BIT(pexit->flags, EXIT_BOLTED) )
		{
			send_to_char( ch, "La serratura è sprangata e pure chiusa a chiave.\r\n" );
#ifdef T2_MSP
			send_audio( ch->desc, "doorlocked.wav", TO_CHAR );
#endif
			return;
		}
		if ( HAS_BIT(pexit->flags, EXIT_BOLTED) )
		{
			send_to_char( ch, "E' sprangato.\r\n" );
#ifdef T2_MSP
			send_audio( ch->desc, "doorlocked.wav", TO_CHAR );
#endif
			return;
		}
		if (  HAS_BIT(pexit->flags, EXIT_LOCKED) )
		{
			send_to_char( ch, "E' chiuso a chiave.\r\n" );
#ifdef T2_MSP
			send_audio( ch->desc, "doorlocked.wav", TO_CHAR );
#endif
			return;
		}

		act( AT_ACTION, "$n apre $d.", ch, NULL, pexit->keyword, TO_ROOM );
		act( AT_ACTION, "Ecco, ho aperto $d.", ch, NULL, pexit->keyword, TO_CHAR );
		if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
		{
			CHAR_DATA *rch;

			for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
				act( AT_ACTION, "Una $d si apre.", rch, NULL, pexit_rev->keyword, TO_CHAR );	/* (GR) articolo della cosa che si apre da fare */
		}
		remove_bexit_flag( pexit, EXIT_CLOSED );
		door = pexit->vdir;
		if ( door >= DIR_NORTH && door < DIR_SOMEWHERE )
			check_room_for_traps( ch, table_dir[door].trap_door );
		return;
	} /* chiude l'if */

	if ( (obj = get_obj_here(ch, argument) ) != NULL )
	{
		/* 'open object' */
		if ( !obj->container )
		{
			ch_printf( ch, "%s non è un contenitore.\r\n", capitalize(obj->short_descr) );
			return;
		}

		if ( !HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			ch_printf( ch, "%s è già aperto.\r\n", capitalize(obj->short_descr) );
			return;
		}

		if ( !HAS_BITINT(obj->container->flags, CONTAINER_CLOSEABLE) )
		{
			ch_printf( ch, "Non posso aprire o chiudere %s.\r\n", capitalize(obj->short_descr) );
			return;
		}

		if ( HAS_BITINT(obj->container->flags, CONTAINER_LOCKED) )
		{
			ch_printf( ch, "%s è chiuso a chiave.\r\n", capitalize(obj->short_descr) );
			return;
		}

		REMOVE_BITINT( obj->container->flags, CONTAINER_CLOSED );
		act( AT_ACTION, "Ecco, ho aperto $p.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n apre $p.", ch, obj, NULL, TO_ROOM );
		check_for_trap( ch, obj, TRAPFLAG_OPEN );
		return;
	}

	ch_printf( ch, "Non vedo %s qui.\r\n", argument );
}


DO_RET do_close( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	EXIT_DATA *pexit;
	int door;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Cosa devo chiudere?\r\n" );
		return;
	}

	pexit = find_door( ch, argument, TRUE );
	if ( pexit )
	{
		/* 'close door' */
		EXIT_DATA *pexit_rev;

		if ( !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
		{
			send_to_char( ch, "No, non posso farlo.\r\n" );
			return;
		}
		if ( HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "E' già chiuso..\r\n" );
			return;
		}

		act( AT_ACTION, "$n chiude una $d.", ch, NULL, pexit->keyword, TO_ROOM );
		act( AT_ACTION, "Chiudo una $d.", ch, NULL, pexit->keyword, TO_CHAR );

		/* close the other side */
		if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
		{
			CHAR_DATA *rch;

			SET_BIT( pexit_rev->flags, EXIT_CLOSED );
			for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
				act( AT_ACTION, "Una $d si chiude.", rch, NULL, pexit_rev->keyword, TO_CHAR );
		}
		set_bexit_flag( pexit, EXIT_CLOSED );

		door = pexit->vdir;
		if ( door >= DIR_NORTH && door < DIR_SOMEWHERE )
			check_room_for_traps( ch, table_dir[door].trap_door );

		return;
	}

	obj = get_obj_here( ch, argument );
	if ( obj )
	{
		/* 'close object' */
		if ( !obj->container )
		{
			ch_printf( ch, "%s non è un contenitore.\r\n", capitalize(obj->short_descr) );
			return;
		}

		if ( HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			ch_printf( ch, "%s è già chiuso.\r\n", capitalize( obj->short_descr ) );
			return;
		}

		if ( !HAS_BITINT(obj->container->flags, CONTAINER_CLOSEABLE) )
		{
			ch_printf( ch, "Non posso aprire o chiudere %s.\r\n", capitalize( obj->short_descr ) );
			return;
		}

		SET_BITINT( obj->container->flags, CONTAINER_CLOSED );
		act( AT_ACTION, "Chiudo $p.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n chiude $p.", ch, obj, NULL, TO_ROOM );
		check_for_trap( ch, obj, TRAPFLAG_CLOSE );
		return;
	}

	ch_printf( ch, "Non trovo nessun %s qui.\r\n", argument );
}


/*
 * New: returns pointer to key/NULL instead of TRUE/FALSE
 *
 * (FF) If you want a feature like having admins always have a key... you'll
 * need to code in a generic key, and make sure free_object doesn't extract it
 * secondo me invece si può controllare prima di usare questa funzione in giro
 */
OBJ_DATA *has_key( CHAR_DATA *ch, VNUM key )
{
	OBJ_DATA *obj;
	OBJ_DATA *obj2;

	for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
	{
		if ( obj->vnum == key )
		{
			return obj;
		}
		else if ( obj->type == OBJTYPE_KEYRING )
		{
			for ( obj2 = obj->first_content;  obj2;  obj2 = obj2->next_content )
			{
				if ( obj2->vnum == key )
					return obj2;
			}
		}
	}

	return NULL;
}


DO_RET do_lock( CHAR_DATA *ch, char *argument )
{
	char arg[MIL];
	OBJ_DATA *obj, *key;
	EXIT_DATA *pexit;
	int count;

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Cosa devo chiudere a chiave?\r\n" );
		return;
	}

	pexit = find_door( ch, arg, TRUE );	/* Le porte segrete le gestisce la find_door() */
	if ( pexit )
	{
		/* 'lock door' */
		if ( !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
		{
			send_to_char( ch, "Non posso farlo.\r\n" );
			return;
		}
		if ( !HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "Non è chiuso.\r\n" );
			return;
		}
		if ( pexit->key < 0 )
		{
			send_to_char( ch, "Non riesco a chiudere..\r\n" );
			return;
		}
		if ( (key = has_key(ch, pexit->key)) == NULL )
		{
			send_to_char( ch, "Ho bisogno della chiave giusta..\r\n" );
			return;
		}
		if ( HAS_BIT(pexit->flags, EXIT_LOCKED) )
		{
			send_to_char( ch, "E' già chiuso a chiave.\r\n" );
			return;
		}

		send_to_char( ch, "*Click*\r\n" );
		count = key->count;	/* Imposta il count a 1 perchè altrimenti nella act_string la obj_short stampa un messaggio non consono */
		key->count = 1;
		act( AT_ACTION, "$n chiude $d con $p.", ch, key, pexit->keyword, TO_ROOM );
		key->count = count;
		set_bexit_flag( pexit, EXIT_LOCKED );
		return;
	}

	obj = get_obj_here( ch, arg );
	if ( obj )
	{
		/* 'lock object' */
		if ( !obj->container )
		{
			send_to_char( ch, "Non è un contenitore.\r\n" );
			return;
		}
		if ( !HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			send_to_char( ch, "Non è chiuso.\r\n" );
			return;
		}
		if ( obj->container->key <= 0 )
		{
			send_to_char( ch, "Non posso chiuderlo.\r\n" );
			return;
		}
		if ( (key = has_key(ch, obj->container->key)) == NULL )
		{
			send_to_char( ch, "Ho bisogno della chiave.\r\n" );
			return;
		}
		if ( HAS_BITINT(obj->container->flags, CONTAINER_LOCKED) )
		{
			send_to_char( ch, "E' già chiuso.\r\n" );
			return;
		}

		SET_BITINT( obj->container->flags, CONTAINER_LOCKED );
		send_to_char( ch, "*Click*\r\n" );
		count = key->count;	/* Imposta il count a 1 perchè altrimenti nella act_string la obj_short stampa un messaggio non consono */
		key->count = 1;
		act( AT_ACTION, "$n chiude $p con $P.", ch, obj, key, TO_ROOM );
		key->count = count;
		return;
	}

	ch_printf( ch, "Non vedo %s qui.\r\n", arg );
}


DO_RET do_unlock( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA	*obj;
	OBJ_DATA	*key;
	EXIT_DATA	*pexit;
	int			 count;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Cosa devo sbloccare?\r\n" );
		return;
	}

	pexit = find_door( ch, argument, TRUE );	/* Le porte secret le gestisce la find_door() */
	if ( pexit )
	{
		/* 'unlock door' */

		if ( !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
		{
			send_to_char( ch, "Non posso farlo.\r\n" );
			return;
		}

		if ( !HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "Non è chiuso.\r\n" );
			return;
		}

		if ( pexit->key < 0 )
		{
			send_to_char( ch, "Non può essere sbloccato.\r\n" );
			return;
		}

		if ( (key = has_key(ch, pexit->key)) == NULL )
		{
			send_to_char( ch, "Ho bisogno della chiave giusta.\r\n" );
			return;
		}

		if ( !HAS_BIT(pexit->flags, EXIT_LOCKED) )
		{
			send_to_char( ch, "E' già sbloccato.\r\n" );
			return;
		}

		send_to_char( ch, "*Click*\r\n" );
#ifdef T2_MSP
		send_audio( ch->desc, "unlock.wav", TO_CHAR );
#endif
		act( AT_ACTION, "$n sblocca $d con $p.", ch, key, pexit->keyword, TO_ROOM );

		count = key->count;
		key->count = 1;
		key->count = count;
		if ( HAS_BIT(pexit->flags, EXIT_EATKEY) )
		{
			split_obj( key, 1 );
			free_object( key );
		}
		remove_bexit_flag( pexit, EXIT_LOCKED );
			return;
	} /* chiude l'if */

	obj = get_obj_here( ch, argument );
	if ( obj )
	{
		/* 'unlock object' */
		if ( !obj->container )
		{
			send_to_char( ch, "Non è un contenitore.\r\n" );
			return;
		}
		if ( !HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			send_to_char( ch, "Non è chiuso.\r\n" );
			return;
		}
		if ( obj->container->key < 0 )
		{
			send_to_char( ch, "Non può essere sbloccato.\r\n" );
			return;
		}
		if ( (key = has_key(ch, obj->container->key)) == NULL )
		{
			send_to_char( ch, "Ho bisogno della chiave.\r\n" );
			return;
		}
		if ( !HAS_BITINT(obj->container->flags, CONTAINER_LOCKED) )
		{
			send_to_char( ch, "E' già sbloccato.\r\n" );
			return;
		}

		REMOVE_BITINT( obj->container->flags, CONTAINER_LOCKED );
		send_to_char( ch, "*Click*\r\n" );
		count = key->count;
		key->count = 1;
		act( AT_ACTION, "$n sblocca $p con $P.", ch, obj, key, TO_ROOM );
		key->count = count;
		if ( HAS_BITINT(obj->container->flags, CONTAINER_EATKEY) )
		{
			split_obj( key, 1 );
			free_object( key );
		}
		return;
	} /* chiude l'if */

	ch_printf( ch, "Non vedo %s qui.\r\n", argument );
}


DO_RET do_bashdoor( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;
	EXIT_DATA *pexit;
	char	   arg[MIL];

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_bashdoor]->skill_level[ch->class] )
	{
		send_to_char( ch, "Eh no, non ho mai imparato a buttar giù le porte..\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Cosa devo buttar giù?\r\n" );
		return;
	}

	if ( ch->fighting )
	{
		send_to_char( ch, "Devo prima finire il combattimento..\r\n" );
		return;
	}

	pexit = find_door( ch, arg, FALSE );	/* notare: quiet a FALSE */
	if ( pexit )
	{
		ROOM_DATA	*to_room;
		EXIT_DATA	*pexit_rev;
		int			 chance_bash;
		char		*keyword;

		if ( !HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "Calma, calma.. è già aperto.\r\n" );
			return;
		}

		WAIT_STATE( ch, table_skill[gsn_bashdoor]->beats * 3 );	/* (WT) */

		if ( HAS_BIT(pexit->flags, EXIT_SECRET) )
			keyword = "muro";
		else
			keyword = pexit->keyword;

		if ( IS_PG(ch) )
			chance_bash = knows_skell(ch, gsn_bashdoor) / 2;
		else
			chance_bash = 90;

		if ( HAS_BIT( pexit->flags, EXIT_LOCKED ) )
			chance_bash /= 3;

		if ( !HAS_BIT(pexit->flags, EXIT_BASHPROOF)
		  && ch->points[POINTS_MOVE] >= 20
		  && number_percent() < chance_bash + (get_curr_attr(ch, ATTR_STR)/4 - 19) * 4 )
		{
			REMOVE_BIT( pexit->flags, EXIT_CLOSED );
			if ( HAS_BIT(pexit->flags, EXIT_LOCKED) )
				REMOVE_BIT( pexit->flags, EXIT_LOCKED );
			SET_BIT( pexit->flags, EXIT_BASHED );

			act( AT_SKILL, "Bene! $d è giù!", ch, NULL, keyword, TO_CHAR );
			act( AT_SKILL, "$n butta giù con una poderosa spallata $d!", ch, NULL, keyword, TO_ROOM );
			learn_skell( ch, gsn_bashdoor, TRUE );

			if ( (to_room = pexit->to_room) != NULL
			  && (pexit_rev = pexit->rexit) != NULL
			  && pexit_rev->to_room == ch->in_room )
			{
				CHAR_DATA *rch;

				REMOVE_BIT( pexit_rev->flags, EXIT_CLOSED );
				if ( HAS_BIT(pexit_rev->flags, EXIT_LOCKED) )
					REMOVE_BIT( pexit_rev->flags, EXIT_LOCKED );
				SET_BIT( pexit_rev->flags, EXIT_BASHED );

				for ( rch = to_room->first_person;  rch;  rch = rch->next_in_room )
					act( AT_SKILL, "Con un boato $d va giù!", rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
			damage( ch, ch, ch->points_max[POINTS_LIFE]/20, gsn_bashdoor );
		}
		else
		{
			act( AT_SKILL, "SBAAAM!!! Maledizione $d è un ostacolo più solido di quanto pensassi.. non va giù!", ch, NULL, keyword, TO_CHAR );
			act( AT_SKILL, "SBAAAM!!!  $n colpisce con una spallata $d ma non ottiene nulla..", ch, NULL, keyword, TO_ROOM );
			damage( ch, ch, ch->points_max[POINTS_LIFE]/20 + 10, gsn_bashdoor );
			learn_skell( ch, gsn_bashdoor, FALSE );
		}
	}
	else
	{
		act( AT_SKILL, "WHAAAAM!!!  Maledizione! Questo muro è incrollabile! Riproviamo!", ch, NULL, NULL, TO_CHAR );
		act( AT_SKILL, "WHAAAAM!!!  $n colpisce con una poderosa spallata il muro.. ma senza successo..", ch, NULL, NULL, TO_ROOM );
		damage( ch, ch, ch->points_max[POINTS_LIFE]/20 + 10, gsn_bashdoor );
		learn_skell( ch, gsn_bashdoor, FALSE );
	}

	if ( !char_died(ch) )
	{
		for ( gch = ch->in_room->first_person;  gch;  gch = gch->next_in_room )
		{
			if ( !is_awake(gch) )						continue;
			if ( gch->fighting )						continue;
			if ( get_level(ch) - get_level(gch) > 4 )	continue;
			if ( number_bits(2) != 0 )					continue;

			if ( IS_MOB(gch) && !HAS_BIT(gch->affected_by, AFFECT_CHARM) )
				multi_hit( gch, ch, TYPE_UNDEFINED );
		}
	}
}


DO_RET do_stand( CHAR_DATA *ch, char *argument )
{
	set_char_color( AT_PLAIN, ch );

	switch ( ch->position )
	{
	  case POSITION_SLEEP:
		if ( HAS_BIT(ch->affected_by, AFFECT_SLEEP) )
		{
			send_to_char( ch, "Maledizione.. non riesco a svegliarmi..!\r\n" );
			return;
		}

		send_to_char( ch, "In piedi.. subito.\r\n" );
		act( AT_ACTION, "$n si risveglia dal suo sonno.", ch, NULL, NULL, TO_ROOM );

		/* Se ci si sveglia resetta il sogno */
		dream_reset( ch );

		ch->position = POSITION_STAND;
		break;

	  case POSITION_REST:
		send_to_char( ch, "Ok, in piedi ora.\r\n" );
		act( AT_ACTION, "$n smette di riposare e si alza in piedi.", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_STAND;
		break;

	  case POSITION_SIT:
		send_to_char( ch, "Alziamoci.. forza..\r\n" );
		act( AT_ACTION, "$n si alza in piedi.", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_STAND;
		break;

	  case POSITION_STAND:
		send_to_char( ch, "Sono già in piedi.\r\n" );
		break;

	  case POSITION_FIGHT:
	  case POSITION_EVASIVE:
	  case POSITION_DEFENSIVE:
	  case POSITION_AGGRESSIVE:
	  case POSITION_BERSERK:
		send_to_char( ch, "Sto combattendo!\r\n" );
		break;
	}
}


DO_RET do_sit( CHAR_DATA *ch, char *argument )
{
	set_char_color( AT_PLAIN, ch );

	switch ( ch->position )
	{
	  case POSITION_SLEEP:
		if ( HAS_BIT(ch->affected_by, AFFECT_SLEEP) )
		{
			send_to_char( ch, "Maledizione! Non riesco a svegliarmi!\r\n" );
			return;
		}
		send_to_char( ch, "Mi siedo..\r\n" );
		act( AT_ACTION, "$n si sveglia e si siede.", ch, NULL, NULL, TO_ROOM );

		/* Se ci si sveglia resetta il sogno */
		dream_reset( ch );

		ch->position = POSITION_SIT;
		break;

	  case POSITION_REST:
		send_to_char( ch, "Si, mi siedo.\r\n" );
		act( AT_ACTION, "$n si mette a sedere.", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_SIT;
		break;

	  case POSITION_STAND:
		send_to_char( ch, "Mi siedo.\r\n" );
		act( AT_ACTION, "$n si siede.", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_SIT;
		break;

	  case POSITION_SIT:
		send_to_char( ch, "Più di così..\r\n" );
		break;

	  case POSITION_FIGHT:
	  case POSITION_EVASIVE:
	  case POSITION_DEFENSIVE:
	  case POSITION_AGGRESSIVE:
	  case POSITION_BERSERK:
		send_to_char( ch, "Sto combattendo! Non è il momento di pensare a sedersi\r\n" );
		break;

	  case POSITION_MOUNT:
		send_to_char( ch, "Sono già sulla mia cavalcatura..\r\n" );
		break;
	}
}


DO_RET do_rest( CHAR_DATA *ch, char *argument )
{
	set_char_color( AT_PLAIN, ch );

	switch ( ch->position )
	{
	  case POSITION_SLEEP:
		if ( HAS_BIT(ch->affected_by, AFFECT_SLEEP) )
		{
			send_to_char( ch, "Non.. non riesco a svegliarmi!\r\n" );
			return;
		}
		send_to_char( ch, "Fine del sogno.. rieccoci qui, vita.\r\n" );
		act( AT_ACTION, "$n si ridesta dal sonno.", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_REST;
		break;

	  case POSITION_REST:
		send_to_char( ch, "Sto già riposando.\r\n" );
		return;

	  case POSITION_STAND:
		send_to_char( ch, "Riposiamo un po'..\r\n" );
		act( AT_ACTION, "$n si mette comod$x e riposa..", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_REST;
		break;

	  case POSITION_SIT:
		send_to_char( ch, "Ah.. un po' di relax..\r\n" );
		act( AT_ACTION, "$n distende la schiena e riposa.", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_REST;
		break;

	  case POSITION_FIGHT:
	  case POSITION_EVASIVE:
	  case POSITION_DEFENSIVE:
	  case POSITION_AGGRESSIVE:
	  case POSITION_BERSERK:
		send_to_char( ch, "Sto combattendo! Non è il momento!\r\n" );
		return;

	  case POSITION_MOUNT:
		send_to_char( ch, "Sarà meglio scendere dalla cavalcatura prima..\r\n" );
		return;
	}
	rprog_rest_trigger(ch);
}


DO_RET do_sleep( CHAR_DATA *ch, char *argument )
{
	set_char_color( AT_PLAIN, ch );

	switch ( ch->position )
	{
	  case POSITION_SLEEP:
		send_to_char( ch, "Addormentarmi nel sogno?\r\n" );
		return;

	  case POSITION_REST:
		if ( ch->mental_state > 30 && (number_percent()+10) < ch->mental_state )
		{
			send_to_char( ch, "Ahhh non riuscirò mai ad addormentarmi così..\r\n" );
			act( AT_ACTION, "$n chiude gli occhi per qualche istante cercando di dormire.. ma è troppo irrequiet$x.", ch, NULL, NULL, TO_ROOM );
			return;
		}
		send_to_char( ch, "Sì.. è tempo di riposar le stanche membra.\r\n" );
		act( AT_ACTION, "$n chiude gli occhi e scivola nel sonno.", ch, NULL, NULL, TO_ROOM );
		break;

	  case POSITION_SIT:
		if ( ch->mental_state > 30 && (number_percent()+5) < ch->mental_state )
		{
			send_to_char( ch, "Ahhh non riuscirò mai ad addormentarmi così..\r\n" );
			act( AT_ACTION, "$n chiude gli occhi per qualche istante cercando di dormire.. ma è troppo irrequiet$x.", ch, NULL, NULL, TO_ROOM );
			return;
		}
		send_to_char( ch, "Si.. è tempo di dormire..\r\n" );
		act( AT_ACTION, "$n si distende, chiude gli occhi e scivola nel sonno.", ch, NULL, NULL, TO_ROOM );
		break;

	  case POSITION_STAND:
		if ( ch->mental_state > 30 && number_percent() < ch->mental_state )
		{
			send_to_char( ch, "Ahhh non riuscirò mai ad addormentarmi così..\r\n" );
			act( AT_ACTION, "$n chiude gli occhi per qualche istante cercando di dormire.. ma è troppo irrequiet$x.", ch, NULL, NULL, TO_ROOM );
			return;
		}
		send_to_char( ch, "Si.. è tempo di dormire..\r\n" );
		act( AT_ACTION, "$n si distende, chiude gli occhi e scivola nel sonno.", ch, NULL, NULL, TO_ROOM );
		break;

	  case POSITION_FIGHT:
	  case POSITION_EVASIVE:
	  case POSITION_DEFENSIVE:
	  case POSITION_AGGRESSIVE:
	  case POSITION_BERSERK:
		send_to_char( ch, "Sto combattendo! Penserò dopo a dormire!\r\n" );
		return;

	  case POSITION_MOUNT:
		send_to_char( ch, "Meglio scendere da qui prima di dormire..\r\n" );
		return;
	}

	ch->position = POSITION_SLEEP;
#ifdef T2_MSP
	if ( ch->sex == SEX_MALE )
		send_audio( ch->desc, "sleep.wav", TO_CHAR );
#endif
	rprog_sleep_trigger( ch );
}


DO_RET do_wake( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( !VALID_STR(argument) )
	{
		send_command( ch, "stand", CO );
		return;
	}

	if ( !is_awake(ch) )
	{
		send_to_char( ch, "Dovrei svegliare qualcuno mentre sto dormendo?\r\n" );
		return;
	}

	victim = get_char_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Chi dovrei svegliare? Non è qui.\r\n" );
		return;
	}

	if ( is_awake(victim) )
	{
		act( AT_PLAIN, "Non mi pare che $N stia dormendo..", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_SLEEP) || victim->position < POSITION_SLEEP )
	{
		act( AT_PLAIN, "Non riesco a svegliare $M!", ch, NULL, victim, TO_CHAR );
		return;
	}

	act( AT_ACTION, "Ecco.. ho svegliato $N.", ch, NULL, victim, TO_CHAR );
#ifdef T2_MSP
	if ( victim->sex == SEX_MALE )
		send_audio( victim->desc, "wake.wav", TO_CHAR );
#endif
	victim->position = POSITION_STAND;

	/* Se ci si sveglia resetta il sogno */
	dream_reset( victim );

	act( AT_ACTION, "$n ti ha svegliat$X.", ch, NULL, victim, TO_VICT );
}


/*
 * Teleport a character to another room
 */
void teleport_ch( CHAR_DATA *ch, ROOM_DATA *room, bool show )
{
#ifdef T2_ALFA
	if ( TELE_TRANSNOMSG )	/* flag, da aggiungere, per non fare vedere i msgs */
#endif
	act( AT_ACTION, "$n sparisce nel nulla!", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, room );
	act( AT_ACTION, "$n arriva dal nulla!", ch, NULL, NULL, TO_ROOM );

	if ( show )
		send_command( ch, "look", CO );
}


void teleport( CHAR_DATA *ch, VNUM room, int flags )
{
	CHAR_DATA	*nch;
	CHAR_DATA	*nch_next;
	ROOM_DATA	*start = ch->in_room;
	ROOM_DATA	*dest;
	bool		 show;

	dest = get_room_index( NULL, room );
	if ( !dest )
	{
		send_log( NULL, LOG_BUG, "teleport: bad room vnum %d", room );
		return;
	}

	if ( HAS_BITINT(flags, TELE_SHOWDESC) )
		show = TRUE;
	else
		show = FALSE;

	if ( !HAS_BITINT(flags, TELE_TRANSALL) )
	{
		teleport_ch( ch, dest, show );
		return;
	}

	/* teleport everybody in the room */
	for ( nch = start->first_person;  nch;  nch = nch_next )
	{
		nch_next = nch->next_in_room;

		teleport_ch( nch, dest, show );
	}

	/* teleport the objects on the ground too */
	if ( HAS_BITINT(flags, TELE_TRANSALLPLUS) )
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		for ( obj = start->first_content;  obj;  obj = obj_next )
		{
			obj_next = obj->next_content;

			obj_from_room( obj );
			obj_to_room( obj, dest );
		}
	}
}


void update_tele( void )
{
	TELEPORT_DATA *tele;
	TELEPORT_DATA *tele_next;

	if ( !first_teleport )
		return;

	for ( tele = first_teleport;  tele;  tele = tele_next )
	{
		tele_next = tele->next;

		if ( --tele->timer > 0 )
			continue;

		if ( tele->room->first_person )
		{
			int temp = 0;

			SET_BITINT( temp, TELE_TRANSALL );

			if ( HAS_BIT(tele->room->flags, ROOM_TELESHOWDESC) )
				SET_BITINT( temp, TELE_SHOWDESC );

			teleport( tele->room->first_person, tele->room->tele_vnum, temp );
		}

		UNLINK( tele, first_teleport, last_teleport, next, prev );
		DISPOSE( tele );
	}
}


/*
 * "Climb" in a certain direction.
 */
DO_RET do_climb( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA *pexit;
	bool found;

	found = FALSE;
	if ( !VALID_STR(argument) )
	{
		for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
		{
			if ( HAS_BIT( pexit->flags, EXIT_xCLIMB ) )
			{
				move_char( ch, pexit, 0, FALSE );
				return;
			}
		}
		send_to_char( ch, "Non trovo nulla da scalare.. qui.\r\n" );
		return;
	}

	if ( (pexit = find_door(ch, argument, TRUE)) != NULL
	  && HAS_BIT(pexit->flags, EXIT_xCLIMB) )
	{
		move_char( ch, pexit, 0, FALSE );
		return;
	}

	send_to_char( ch, "Non posso arrivare fin lì scalando..\r\n" );
}


/*
 * "enter" something (moves through an exit)
 */
DO_RET do_enter( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA  *pexit;
	bool		found;

	found = FALSE;
	if ( !VALID_STR(argument) )
	{
		/* Se tra le uscite della stanza ce ne è una con la flag di entrata allora entra */
		for ( pexit = ch->in_room->first_exit;  pexit;  pexit = pexit->next )
		{
			/* Se l'entrata è nascosta o segreta allora si necessita di keyword */
			if ( HAS_BIT(pexit->flags, EXIT_xENTER)
			  && (!HAS_BIT(pexit->flags, EXIT_HIDDEN) || !HAS_BIT(pexit->flags, EXIT_SECRET)) )	/* (TT) controllare che vada bene e che non abbisogni di altri tipologie al limite */
			{
				move_char( ch, pexit, 0, FALSE );
				return;
			}
		}

		/* Se il pg digita si trova all'esterno e intorno c'è una entrata ad una zona allora entra */
		if ( ch->in_room->sector != SECTOR_INSIDE && is_outside(ch) )
		{
			for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
			{
				if ( pexit->to_room && (pexit->to_room->sector == SECTOR_INSIDE
				  || HAS_BIT(pexit->to_room->flags, ROOM_INDOORS)) )
				{
					move_char( ch, pexit, 0, FALSE );
					return;
				}
			}
		}

		send_to_char( ch, "Non trovo nessuna entrata.\r\n" );
		return;
	}

	if ( (pexit = find_door(ch, argument, TRUE)) != NULL
	  && HAS_BIT(pexit->flags, EXIT_xENTER) )
	{
		move_char( ch, pexit, 0, FALSE );
		return;
	}

	send_to_char( ch, "Non posso entrare lì.\r\n" );
}


/*
 * Leave through an exit.
 */
DO_RET do_leave( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA *pexit;
	bool found;

	found = FALSE;
	if ( !VALID_STR(argument) )
	{
		for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
		{
			/* Se l'entrata è nascosta o segreta allora si necessita di keyword */
			if ( HAS_BIT(pexit->flags, EXIT_xLEAVE)
			  && (!HAS_BIT(pexit->flags, EXIT_HIDDEN) || !HAS_BIT(pexit->flags, EXIT_SECRET)) )	/* (TT) controllare che vada bene e che non abbisogni di altri tipologie al limite */
			{
				move_char( ch, pexit, 0, FALSE );
				return;
			}
		}
		if ( ch->in_room->sector == SECTOR_INSIDE || !is_outside(ch) )
		{
			for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
			{
				if ( pexit->to_room && pexit->to_room->sector != SECTOR_INSIDE
				  && !HAS_BIT(pexit->to_room->flags, ROOM_INDOORS) )
				{
					move_char( ch, pexit, 0, FALSE );
					return;
				}
			}
		}
		send_to_char( ch, "Non trovo nessuna uscita!\r\n" );
		return;
	}

	if ( (pexit = find_door(ch, argument, TRUE)) != NULL
	  && HAS_BIT(pexit->flags, EXIT_xLEAVE) )
	{
		move_char( ch, pexit, 0, FALSE );
		return;
	}

	send_to_char( ch, "Non posso uscire da lì!\r\n" );
}


/*
 * Tipi di uscita pull/push
 * suddivisi nelle 4 tabelle di misc, water, air, earth, fire.
 */
const CODE_DATA code_pull[] =
{
	{ PULL_UNDEFINED,	"PULL_UNDEFINED",	"undefined"		},
	{ PULL_VORTEX,		"PULL_VORTEX",		"vortex"		},
	{ PULL_VACUUM,		"PULL_VACUUM",		"vacuum"		},
	{ PULL_SLIP,		"PULL_SLIP",		"slip"			},
	{ PULL_ICE,			"PULL_ICE",			"ice"			},
	{ PULL_MYSTERIOUS,	"PULL_MYSTERIOUS",	"mysterious"	},
	/* acqua */
	{ PULL_CURRENT,		"PULL_CURRENT",		"current"		},
	{ PULL_WAVE,		"PULL_WAVE",		"wave"			},
	{ PULL_WHIRLPOOL,	"PULL_WHIRLPOOL",	"whirlpool"		},
	{ PULL_GEYSER,		"PULL_GEYSER",		"geyser"		},
	/* aria  */
	{ PULL_WIND,		"PULL_WIND",		"wind"			},
	{ PULL_STORM,		"PULL_STORM",		"storm"			},
	{ PULL_COLDWIND,	"PULL_COLDWIND",	"coldwind"		},
	{ PULL_BREEZE,		"PULL_BREEZE",		"breeze"		},
	/* terra */
	{ PULL_LANDSLIDE,	"PULL_LANDSLIDE",	"landslide"		},
	{ PULL_SINKHOLE,	"PULL_SINKHOLE",	"sinkhole"		},
	{ PULL_QUICKSAND,	"PULL_QUICKSAND",	"quicksand"		},
	{ PULL_EARTHQUAKE,	"PULL_EARTHQUAKE",	"earthquake"	},
	/* fuoco */
	{ PULL_LAVA,		"PULL_LAVA",		"lava"			},
	{ PULL_HOTAIR,		"PULL_HOTAIR",		"hotair"		}
};
const int max_check_pull = sizeof(code_pull)/sizeof(CODE_DATA);


/*
 * Check to see if an exit in the room is pulling (or pushing) players around.
 * Some types may cause damage.
 *
 * People kept requesting currents (like SillyMUD has), so I went all out
 * and added the ability for an exit to have a "pull" or a "push" force
 * and to handle different types much beyond a simple water current.
 *
 * This check is called by update_violence(). I'm not sure if this is the
 * best way to do it, or if it should be handled by a special queue.
 *
 * Future additions to this code may include equipment being blown away in
 * the wind (mostly headwear), and people being hit by flying objects
 *
 * TODO:
 *	handle more pulltypes
 *	give "entrance" messages for players and objects
 *	proper handling of player resistance to push/pulling
 */
ch_ret pullcheck( CHAR_DATA *ch, int pulse )
{
	ROOM_DATA	*room;
	EXIT_DATA	*xtmp;
	EXIT_DATA	*xit = NULL;
	OBJ_DATA	*obj;
	OBJ_DATA	*obj_next;
	bool		 move = FALSE;
	bool		 moveobj = TRUE;
	bool		 showroom = TRUE;
	int			 rdir;		/* direzione inversa */
	int			 pullfact;
	int			 pull;
	int			 resistance;
	char		*tochar = NULL;
	char		*toroom = NULL;
	char		*objmsg = NULL;
	char		*destrm = NULL;
	char		*destob = NULL;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "pullcheck: ch è NULL" );
		return rNONE;
	}

	room = ch->in_room;
	if ( !room )
	{
		send_log( NULL, LOG_BUG, "pullcheck: la room in cui si trova %s è NULL", ch->name );
		return rNONE;
	}

	/* Find the exit with the strongest force (if any) */
	for ( xtmp = room->first_exit;  xtmp;  xtmp = xtmp->next )
	{
		if ( !xtmp->pull )		continue;
		if ( !xtmp->to_room )	continue;

		if ( !xit || (abs(xtmp->pull) > abs(xit->pull)) )
			xit = xtmp;
	}

	if ( !xit )
		return rNONE;

	pull = xit->pull;

	/* strength also determines frequency */
	pullfact = URANGE( 1, 20-(abs(pull)/5), 20 );

	/* strongest pull not ready yet... check for one that is */
	if ( (pulse % pullfact) != 0 )
	{
		for ( xit = room->first_exit;  xit;  xit = xit->next )
		{
			if ( xit->pull && xit->to_room )
			{
				pull = xit->pull;
				pullfact = URANGE( 1, 20-(abs(pull)/5), 20 );
				if ( (pulse % pullfact) != 0 )
					break;
			}
		}

		if ( !xit )
			return rNONE;
	}

	/* negative pull = push... get the reverse exit if any */
	if ( pull < 0 )
	{
		xit = get_exit( room, table_dir[xit->vdir].rev_dir );
		if ( !xit )
			return rNONE;
	}

	rdir = table_dir[xit->vdir].rev_dir;

	/*
	 * First determine if the player should be moved or not
	 * Check various flags, spells, the players position and strength vs.
	 * the pull, etc... any kind of checks you like.
	 */
	switch ( xit->pulltype )
	{
	  case PULL_CURRENT:
	  case PULL_WHIRLPOOL:
		switch (room->sector)
		{
		/* allow whirlpool to be in any sector type */
		  default:
			if ( xit->pulltype == PULL_CURRENT )
				break;
		  case SECTOR_WATER_SWIM:
		  case SECTOR_WATER_NOSWIM:
			if ( (ch->mount && !is_floating(ch->mount))
			  || (!ch->mount && !is_floating(ch)) )
			{
				move = TRUE;
			}
			break;

		  case SECTOR_UNDERWATER:
		  case SECTOR_OCEANFLOOR:
			move = TRUE;
			break;
		}
		break;

	  case PULL_GEYSER:
	  case PULL_WAVE:
		move = TRUE;
		break;

	  case PULL_WIND:
	  case PULL_STORM:
		/* if not flying... check weight, position & strength */
		move = TRUE;
		break;

	  case PULL_COLDWIND:
		/* if not flying... check weight, position & strength
			also check for damage due to bitter cold */
		move = TRUE;
		break;

	  case PULL_HOTAIR:
		/* if not flying... check weight, position & strength
			also check for damage due to heat */
		move = TRUE;
		break;

	  /* light breeze -- very limited moving power */
	  case PULL_BREEZE:
		move = FALSE;
		break;

	  /*
	   * exits with these pulltypes should also be blocked from movement
	   * ie: a secret locked pickproof door with the name "_sinkhole_", etc
	   */
	  case PULL_EARTHQUAKE:
	  case PULL_SINKHOLE:
	  case PULL_QUICKSAND:
	  case PULL_LANDSLIDE:
	  case PULL_SLIP:
	  case PULL_LAVA:
		if ( (ch->mount && !is_floating(ch->mount))
			|| (!ch->mount && !is_floating(ch)) )
		{
			move = TRUE;
		}
		break;

	  /* as if player moved in that direction him/herself */
	  case PULL_UNDEFINED:
		return move_char( ch, xit, 0, FALSE );

	  /* all other cases ALWAYS move */
	  default:
		move = TRUE;
		break;
	}

	/* (TR) assign some nice text messages */
	switch ( xit->pulltype )
	{
	  case PULL_MYSTERIOUS:
		/* no messages to anyone */
		showroom = FALSE;
		break;

	  case PULL_WHIRLPOOL:
	  case PULL_VACUUM:
		tochar = "You are sucked $T!";
		toroom = "$n is sucked $T!";
		destrm = "$n is sucked in from $T!";
		objmsg = "$p is sucked $T.";
		destob = "$p is sucked in from $T!";
		break;

	  case PULL_CURRENT:
	  case PULL_LAVA:
		tochar = "You drift $T.";
		toroom = "$n drifts $T.";
		destrm = "$n drifts in from $T.";
		objmsg = "$p drifts $T.";
		destob = "$p drifts in from $T.";
		break;

	  case PULL_BREEZE:
		tochar = "You drift $T.";
		toroom = "$n drifts $T.";
		destrm = "$n drifts in from $T.";
		objmsg = "$p drifts $T in the breeze.";
		destob = "$p drifts in from $T.";
		break;

	  case PULL_GEYSER:
	  case PULL_WAVE:
		tochar = "You are pushed $T!";
		toroom = "$n is pushed $T!";
		destrm = "$n is pushed in from $T!";
		destob = "$p floats in from $T.";
		break;

	  case PULL_EARTHQUAKE:
		tochar = "The earth opens up and you fall $T!";
		toroom = "The earth opens up and $n falls $T!";
		destrm = "$n falls from $T!";
		objmsg = "$p falls $T.";
		destob = "$p falls from $T.";
		break;

	  case PULL_SINKHOLE:
		tochar = "The ground suddenly gives way and you fall $T!";
		toroom = "The ground suddenly gives way beneath $n!";
		destrm = "$n falls from $T!";
		objmsg = "$p falls $T.";
		destob = "$p falls from $T.";
		break;

	  case PULL_QUICKSAND:
		tochar = "You begin to sink $T into the quicksand!";
		toroom = "$n begins to sink $T into the quicksand!";
		destrm = "$n sinks in from $T.";
		objmsg = "$p begins to sink $T into the quicksand.";
		destob = "$p sinks in from $T.";
		break;

	  case PULL_LANDSLIDE:
		tochar = "The ground starts to slide $T, taking you with it!";
		toroom = "The ground starts to slide $T, taking $n with it!";
		destrm = "$n slides in from $T.";
		objmsg = "$p slides $T.";
		destob = "$p slides in from $T.";
		break;

	  case PULL_SLIP:
		tochar = "You lose your footing!";
		toroom = "$n loses su$x footing!";
		destrm = "$n slides in from $T.";
		objmsg = "$p slides $T.";
		destob = "$p slides in from $T.";
		break;

	  case PULL_VORTEX:
		tochar = "You are sucked into a swirling vortex of colors!";
		toroom = "$n is sucked into a swirling vortex of colors!";
		toroom = "$n appears from a swirling vortex of colors!";
		objmsg = "$p is sucked into a swirling vortex of colors!";
		objmsg = "$p appears from a swirling vortex of colors!";
		break;

	  case PULL_HOTAIR:
		tochar = "A blast of hot air blows you $T!";
		toroom = "$n is blown $T by a blast of hot air!";
		destrm = "$n is blown in from $T by a blast of hot air!";
		objmsg = "$p is blown $T.";
		destob = "$p is blown in from $T.";
		break;

	  case PULL_COLDWIND:
		tochar = "A bitter cold wind forces you $T!";
		toroom = "$n is forced $T by a bitter cold wind!";
		destrm = "$n is forced in from $T by a bitter cold wind!";
		objmsg = "$p is blown $T.";
		destob = "$p is blown in from $T.";
		break;

	  case PULL_WIND:
		tochar = "A strong wind pushes you $T!";
		toroom = "$n is blown $T by a strong wind!";
		destrm = "$n is blown in from $T by a strong wind!";
		objmsg = "$p is blown $T.";
		destob = "$p is blown in from $T.";
		break;

	  case PULL_STORM:
		tochar = "The raging storm drives you $T!";
		toroom = "$n is driven $T by the raging storm!";
		destrm = "$n is driven in from $T by a raging storm!";
		objmsg = "$p is blown $T.";
		destob = "$p is blown in from $T.";
		break;

	  default:
		if ( pull > 0 )
		{
			tochar = "You are pulled $T!";
			toroom = "$n is pulled $T.";
			destrm = "$n is pulled in from $T.";
			objmsg = "$p is pulled $T.";
			objmsg = "$p is pulled in from $T.";
		}
		else
		{
			tochar = "You are pushed $T!";
			toroom = "$n is pushed $T.";
			destrm = "$n is pushed in from $T.";
			objmsg = "$p is pushed $T.";
			objmsg = "$p is pushed in from $T.";
		}
		break;
	}


	/* Do the moving */
	if ( move )
	{
		/* display an appropriate exit message */
		if ( tochar )
		{
			act( AT_PLAIN, tochar, ch, NULL, code_name(NULL, xit->vdir, CODE_DIR), TO_CHAR );
			send_to_char( ch, "\r\n" );
		}
		if ( toroom )
			act(AT_PLAIN, toroom, ch, NULL, code_name(NULL, xit->vdir, CODE_DIR), TO_ROOM );

		/* display an appropriate entrance message */
		if ( destrm && xit->to_room->first_person )
		{
			act( AT_PLAIN, destrm, xit->to_room->first_person, NULL, code_name(NULL, rdir, CODE_DIR), TO_CHAR );
			act(AT_PLAIN, destrm, xit->to_room->first_person, NULL, code_name(NULL, rdir, CODE_DIR), TO_ROOM );
		}


		/* move the char */
		if ( xit->pulltype == PULL_SLIP )
			return move_char( ch, xit, 1, FALSE );
		char_from_room( ch );
		char_to_room( ch, xit->to_room );

		if ( showroom )
			send_command( ch, "look", CO );

		/* move the mount too */
		if ( ch->mount )
		{
			char_from_room( ch->mount );
			char_to_room( ch->mount, xit->to_room );
			if ( showroom )
				send_command( ch->mount, "look", CO );
		}
	} /* chiude l'if */

	/* move objects in the room */
	if ( moveobj )
	{
		for ( obj = room->first_content; obj; obj = obj_next )
		{
			obj_next = obj->next_content;

			if ( HAS_BIT(obj->extra_flags, OBJFLAG_BURIED) || !HAS_BIT( obj->wear_flags, OBJWEAR_TAKE) )
				continue;

			resistance = get_obj_weight( obj );
			if ( HAS_BIT(obj->extra_flags, OBJFLAG_METAL) )
				resistance = (resistance * 6) / 5;	/* (RR) da ritarare per il peso in kili */

			switch ( obj->type )
			{
			  case OBJTYPE_SCROLL:
			  case OBJTYPE_NOTE:
			  case OBJTYPE_TRASH:
				resistance >>= 2;
				break;

			  case OBJTYPE_SCRAPS:
			  case OBJTYPE_CONTAINER:
				resistance >>= 1;
				break;

			  case OBJTYPE_PEN:
			  case OBJTYPE_WAND:
				resistance = (resistance * 5) / 6;
				break;

			  case OBJTYPE_CORPSE_PG:
			  case OBJTYPE_CORPSE_MOB:
			  case OBJTYPE_FOUNTAIN:
				resistance <<= 2;
				break;
			}

			/* Is the pull greater than the resistance of the object? */
			if ( (abs(pull) * 10) > resistance )
			{
				if ( objmsg && room->first_person )
				{
					act( AT_PLAIN, objmsg, room->first_person, obj, code_name(NULL, xit->vdir, CODE_DIR), TO_CHAR );
					act( AT_PLAIN, objmsg, room->first_person, obj, code_name(NULL, xit->vdir, CODE_DIR), TO_ROOM );
				}
				if ( destob && xit->to_room->first_person )
				{
					act( AT_PLAIN, destob, xit->to_room->first_person, obj, code_name(NULL, rdir, CODE_DIR), TO_CHAR );
					act( AT_PLAIN, destob, xit->to_room->first_person, obj, code_name(NULL, rdir, CODE_DIR), TO_ROOM );
				}
				obj_from_room(obj);
				obj_to_room( obj, xit->to_room );
			}
		} /* chiude il for */
	} /* chiude l'if */

	return rNONE;
}


/*
 * This function bolts a door
 */
DO_RET do_bolt( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA	*pexit;
	char		 arg[MIL];

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Cosa devo chiudere con il catenaccio?\r\n" );
		return;
	}

	pexit = find_door ( ch, arg, TRUE );
	if ( pexit )	/* le porte segrete le gestisce la find_door() */
	{
		if ( !HAS_BIT (pexit->flags, EXIT_ISDOOR) )
		{
			send_to_char( ch, "Non posso farlo.\r\n" );
			return;
		}
		if ( !HAS_BIT (pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "Non è chiuso!\r\n" );
			return;
		}
		if ( !HAS_BIT (pexit->flags, EXIT_ISBOLT) )
		{
			send_to_char( ch, "Non c'è nessun catenaccio.\r\n" );
			return;
		}
		if ( HAS_BIT (pexit->flags, EXIT_BOLTED) )
		{
			send_to_char( ch, "L'uscita è già sprangata.\n" );
			return;
		}

		send_to_char( ch, "*Clunk*\r\n" );
		act( AT_ACTION,"$n chiude con il catenaccio $d.", ch, NULL, pexit->keyword, TO_ROOM );
		set_bexit_flag( pexit, EXIT_BOLTED );
		return;
	}

	ch_printf( ch, "Non vedo %s qui.\r\n", arg );
}


/*
 * This function unbolts a door
 */
DO_RET do_unbolt( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA	*pexit;
	char		 arg[MIL];

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Quale chiavistello devo sbloccare?\r\n" );
		return;
	}

	pexit = find_door( ch, arg, TRUE );	/* le porte segrete le gestisce la find_door() */
	if ( pexit )
	{
		if ( !HAS_BIT (pexit->flags, EXIT_ISDOOR) )
		{
			send_to_char( ch, "Non posso farlo.\r\n" );
			return;
		}
		if ( !HAS_BIT (pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "L'uscita non è chiusa.\r\n" );
			return;
		}
		if ( !HAS_BIT (pexit->flags, EXIT_ISBOLT) )
		{
			send_to_char( ch, "Non vedo nessun chiavistello.\r\n" );
			return;
		}
		if ( !HAS_BIT (pexit->flags, EXIT_BOLTED) )
		{
			send_to_char( ch, "Il chiavistello è già sbloccato.\r\n" );
			return;
		}

		send_to_char( ch, "*Clunk*\r\n" );
		act( AT_ACTION,"$n sblocca $d.", ch, NULL, pexit->keyword, TO_ROOM );
		remove_bexit_flag( pexit, EXIT_BOLTED );
		return;
	}

	ch_printf( ch, "Non vedo %s qui.\r\n", arg);
}


#ifdef T2_ALFA
/*
 * Ritorna vero se il pg sta in qualche modo volando
 * (bb) fa schifo, non ha senso e si confonde con is_floating,
 *	poi alla fin fine tra AFF_FLOATING e AFF_FLYING non c'è molta diff e questo non è bene
 */
bool is_flying( CHAR_DATA *ch )
{
	AFFECT_DATA *paf;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_flying: ch passato è NULL" );
		return FALSE;
	}

	for ( paf = ch->first_affect;  paf;  paf = paf_next )
	{
		if ( HAS_BIT(paf->bitvector, AFFECT_FLYING)
		  || HAS_BIT(paf->bitvector, AFFECT_FLOATING) )
			return TRUE;
	}

	return FALSE;
}


/* (FF) fino a che gli affect fan così schifo non mi va' di farlo sto comando */
/*
 * Comando per cominciare a volare oppure atterrare
 * Non toglie gli affect di volo o altro
 */
DO_RET do_fly( CHAR_DATA *ch, char *argument )
{
	bool up;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_fly: ch passato è NULL" );
		return;
	}

	if ( !VALID_STR(argument) )
		up = TRUE;
	else
	{
		if ( is_name(argument, "sù su' su sopra alto up") )
			up = TRUE;
		else if ( is_name(argument, "giù giu' giusotto basso down") )
			up = FALSE;
		else
		{
			send_to_char( ch, "Dove dovrei volare? Sù o giù?\r\n" );
			return;
		}
	}

	if ( up )
	{
		if ( is_flying(ch) )
		{
			if ( HAS_BIT(ch->enflags, ENFLAG_FLYCHANGED) )
			{
				REMOVE_BIT( ch->enflags, ENFLAG_FLYCHANGED );
				send_to_char( "Rincomincio a volare.\r\n", ch );
			}
			else
				send_to_char( "Sto già volando.\r\n", ch );
		}
		else
		{
			if ( HAS_BIT(ch->enflags, ENFLAG_FLYCHANGED) )
			{
				send_log( NULL, LOG_BUG, "do_fly: (up) flag di entità flychanged attiva anche quando non dovrebbe per %s",
					ch->name );
				REMOVE_BIT( ch->enflags, ENFLAG_FLYCHANGED );
			}
			send_to_char( ch, "Per volare dovrei trovare una pozione o delle ali.\r\n" );
		}
	}
	else
	{
		if ( is_flying(ch) )
		{
			if ( HAS_BIT(ch->enflags, ENFLAG_FLYCHANGED) )
			{
				send_log( NULL, LOG_BUG, "do_fly: (down) flag di entità flychanged attiva anche quando non dovrebbe per %s",
					ch->name );
			}
			SET_BIT( ch->enflags, ENFLAG_FLYCHANGED );
			act( AT_PLAIN, "Lentamente tocco il terreno.", ch, NULL, NULL, TO_CHAR );
			act( AT_PLAIN, "$n lentamente atterra.", ch, NULL, NULL, TO_ROOM );
		}
		else
		{
			if ( HAS_BIT(ch->enflags, ENFLAG_FLYCHANGED) )
			{
				send_log( NULL, LOG_BUG, "do_fly: (down) flag di entità flychanged attiva anche quando non dovrebbe per %s",
					ch->name );
				REMOVE_BIT( ch->enflags, ENFLAG_FLYCHANGED );
			}
			send_to_char( ch, "Sono già con i piedi per terra.\r\n" );
		}
	}
}


/* Land Command
 * This command allows a player who is floating or flying, to cease,
 *	no matter how or why they're doing it.
 */
DO_RET do_land( CHAR_DATA *ch, char *argument )
{
	send_command( ch, "fly down", CO );
}
#else
DO_RET do_land( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	bool		 found = FALSE;

	for ( paf = ch->first_affect;  paf;  paf = paf_next )
	{
		paf_next = paf->next;
		if ( HAS_BIT(paf->bitvector, AFFECT_FLYING)
		  || HAS_BIT(paf->bitvector, AFFECT_FLOATING) )
		{
			affect_remove( ch, paf );
			found = TRUE;
		}
	}

	REMOVE_BIT( ch->affected_by, AFFECT_FLYING	);
	REMOVE_BIT( ch->affected_by, AFFECT_FLOATING	);

	if ( found )
	{
		act( AT_PLAIN, "Atterro.", ch, NULL, NULL, TO_CHAR );
		act( AT_PLAIN, "$n atterra.", ch, NULL, NULL, TO_ROOM );
	}
	else
	{
		send_to_char( ch, "Sono già con i piedi per terra.\r\n" );
	}
}
#endif
