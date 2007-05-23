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
 >							Modulo inseguimento/caccia						<
\****************************************************************************/

#include "mud.h"
#include "command.h"
#include "db.h"
#include "gramm.h"
#include "interpret.h"
#include "movement.h"
#include "room.h"


#define BFS_ERROR			-1
#define BFS_ALREADY_THERE	-2
#define BFS_NO_PATH			-3


typedef struct bfs_queue_struct BFS_DATA;
struct bfs_queue_struct
{
	BFS_DATA	*next;
	ROOM_DATA	*room;
	char		 dir;
};


static BFS_DATA	*queue_head = NULL;
static BFS_DATA	*queue_tail = NULL;
static BFS_DATA	*room_queue = NULL;


/* Macro utilità */
#define MARK( room )		( SET_BIT   ((room)->flags, ROOM_BFS_MARK) )
#define UNMARK( room )		( REMOVE_BIT((room)->flags, ROOM_BFS_MARK) )
#define IS_MARKED( room )	( HAS_BIT   ((room)->flags, ROOM_BFS_MARK) )


bool valid_edge( EXIT_DATA *pexit )
{
	if ( sysdata.track_trough_door == FALSE )
		return FALSE;

	if ( !pexit->to_room )
		return FALSE;

	if ( IS_MARKED(pexit->to_room) )
		return FALSE;

	return TRUE;
}


void bfs_enqueue( ROOM_DATA *room, char dir )
{
	BFS_DATA *curr;

	curr = malloc( sizeof(BFS_DATA) );

	curr->room = room;
	curr->dir  = dir;
	curr->next = NULL;

	if ( queue_tail )
	{
		queue_tail->next = curr;
		queue_tail		 = curr;
	}
	else
	{
		queue_head = queue_tail = curr;
	}
}


void bfs_dequeue( void )
{
	BFS_DATA *curr;

	curr = queue_head;

	queue_head = queue_head->next;
	if ( !queue_head )
		queue_tail = NULL;

	free( curr );
}


void bfs_clear_queue( void )
{
	while ( queue_head )
		bfs_dequeue( );
}


void room_enqueue( ROOM_DATA *room )
{
	BFS_DATA *curr;

	curr = malloc( sizeof(BFS_DATA) );

	curr->room = room;
	curr->next = room_queue;

	room_queue = curr;
}


void clean_room_queue( void )
{
	BFS_DATA *curr,
			 *curr_next;

	for ( curr = room_queue;  curr;  curr = curr_next )
	{
		UNMARK( curr->room );
		curr_next = curr->next;
		free( curr );
	}

	room_queue = NULL;
}


int find_first_step( ROOM_DATA *src, ROOM_DATA *target, int maxdist )
{
	EXIT_DATA *pexit;
	int		   curr_dir;
	int		   count;

	if ( !src )
	{
		send_log( NULL, LOG_BUG, "find_first_step: stanza di partenza src NULL" );
		return BFS_ERROR;
	}

	if ( !target )
	{
		send_log( NULL, LOG_BUG, "find_first_step: stanza di arrivo target NULL" );
		return BFS_ERROR;
	}

	if ( src == target )
		return BFS_ALREADY_THERE;

	if ( src->area != target->area )
		return BFS_NO_PATH;

	room_enqueue( src );
	MARK( src );

	/* Prima, accoda il primo passo, salvando da quale direzione si era giunti. */
	for ( pexit = src->first_exit;  pexit;  pexit = pexit->next )
	{
		if ( valid_edge(pexit) )
		{
			curr_dir = pexit->vdir;
			MARK( pexit->to_room );
			room_enqueue( pexit->to_room );
			bfs_enqueue( pexit->to_room, curr_dir );
		}
	}

	count = 0;
	while ( queue_head )
	{
		if ( ++count > maxdist )
		{
			bfs_clear_queue( );
			clean_room_queue( );
			return BFS_NO_PATH;
		}

		if ( queue_head->room == target )
		{
			curr_dir = queue_head->dir;
			bfs_clear_queue( );
			clean_room_queue( );
			return curr_dir;
		}
		else
		{
			for ( pexit = queue_head->room->first_exit;  pexit;  pexit = pexit->next )
			{
				if ( valid_edge(pexit) )
				{
					curr_dir = pexit->vdir;
					MARK( pexit->to_room );
					room_enqueue( pexit->to_room );
					bfs_enqueue( pexit->to_room,queue_head->dir );
				}
			}
			bfs_dequeue( );
		}
	} /* chiude lo while */
	clean_room_queue( );

	return BFS_NO_PATH;
}


DO_RET do_track( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *vict;
	char	   arg[MIL];
	int		   dir;
	int		   maxdist;

	if ( IS_PG(ch) && ch->pg->learned_skell[gsn_track] <= 0 )
	{
		send_to_char( ch, "Non conosci ancora questa tecnica.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Di chi le tracce vorresti seguire?\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_track]->beats * 3 );	/* (WT) */

	vict = get_char_mud( ch, arg, TRUE );
	if ( !vict )
	{
		send_to_char( ch, "Non trovi nessuna pista per raggiungere colui che cerchi.\r\n" );
		return;
	}

	maxdist = 100 + get_level(ch) * 15;

	if ( IS_PG(ch) )
		maxdist = ( maxdist * knows_skell(ch, gsn_track) ) / 100;

	dir = find_first_step( ch->in_room, vict->in_room, maxdist );

	switch ( dir )
	{
	  default:
		ch_printf( ch, "Scovo una pista che porta verso %s.\r\n", table_dir[dir].to );
		learn_skell( ch, gsn_track, TRUE );
		break;

	  case BFS_ERROR:
		send_to_char( ch, "Uhmm.. Qualcosa sembra sbagliato.\r\n" );
		break;

	  case BFS_ALREADY_THERE:
		send_to_char( ch, "Sono già stato in questo luogo!\r\n" );
		break;

	  case BFS_NO_PATH:
		send_to_char( ch, "Non riesco a scovare nessuna traccia qui.\r\n" );
		learn_skell( ch, gsn_track, FALSE );
		break;
	}
}


void found_prey( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char	victname[MSL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "found_prey: ch passato è NULL" );
		return;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "found_prey: victim passata è NULL" );
		return;
	}

	if ( !victim->in_room )
	{
		send_log( NULL, LOG_BUG, "found_prey: victim->in_room è NULL" );
		return;
	}

	strcpy( victname, IS_MOB(victim)  ?  victim->short_descr  :  victim->name );

	if ( !can_see(ch, victim) )
	{
		if ( number_percent( ) < 90 )
			return;

		switch ( number_bits(2) )
		{
		  case 0:
		    send_command( ch, "say non ti conviene farti trovare!", CO );
			break;

		  case 1:
			act( AT_ACTION, "$n annusa in giro alla ricerca di qualcuno.",  ch, NULL, victim, TO_NOVICT );
			act( AT_ACTION, "Annusi in giro alla ricerca di $N.",  ch, NULL, victim, TO_CHAR );
			act( AT_ACTION, "$n annusa in giro per trovarmi.", ch, NULL, victim, TO_VICT );
			send_command( ch, "say posso sentire l'odore del tuo sangue!", CO );
			break;

		  case 2:
			send_command( ch, "yell ti farò passare un brutto momento appena ti trovo!", CO );
			break;

		  case 3:
			send_command( ch, "say aspetta e vedrai..", CO );
			break;
		}
		return;
	}

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		char	buf[MSL];
		char	csex = gramm_ao( ch );

		if ( number_percent() < 90 )
			return;

		switch ( number_bits(2) )
		{
		  case 0:
			sprintf( buf, "say fatti vedere codard%c!", csex );
			send_command( ch, buf, CO );
			sprintf( buf, "yella maledett%c cordar%c!", csex, csex );
			send_command( ch, buf, CO );
			break;

		  case 1:
			send_command( ch, "say Risolviamo le cose fuori di qui", CO );
			break;

		  case 2:
			sprintf( buf, "yell sei un%s vigliacc%c!", ch->sex == SEX_FEMALE ? "a" : "", csex );	/* (bb) si ma qui se ci fosse un apply_sex non va'.. ci vuole una get_article su vigliaccX */
			send_command( ch, buf, CO );
			break;

		  case 3:
			act( AT_ACTION, "$n schiaffeggia $N con aria di sfida.", ch, NULL, victim, TO_NOVICT );
			act( AT_ACTION, "Schiaffeggio $N con aria di sfida.", ch, NULL, victim, TO_CHAR );
			act( AT_ACTION, "$n mi schiaffeggia con aria di sfida.", ch, NULL, victim, TO_VICT );
			break;
		}
		return;
	}

	switch ( number_bits(2) )
	{
	  case 0:
		send_command( ch, "yell Il tuo sangue sarà mio!", CO );
		break;

	  case 1:
		send_command( ch, "say Finalmente ci incontriamo nuovamente!", CO );
		break;

	  case 2:
		send_command( ch, "say Cosa vuoi che scriva sulla tua tomba?", CO );
		break;

	  case 3:
		act( AT_ACTION, "$n balza improvvisamente su $N!", ch, NULL, victim, TO_NOVICT );
		act( AT_ACTION, "Balzo improvvisamente su $N prendendol$x di sorpresa!", ch, NULL, victim, TO_CHAR );
		act( AT_ACTION, "$n mi balza addosso all'improvviso sbucando dal nulla!", ch, NULL, victim, TO_VICT );
	}
	stop_hunting( ch );
	set_fighting( ch, victim );
	multi_hit( ch, victim, TYPE_UNDEFINED );
}


void hunt_victim( CHAR_DATA *ch )
{
	CHAR_DATA *tmp;
	EXIT_DATA *pexit;
	int		   ret;
	bool	   found;

	if ( !ch )							return;
	if ( !ch->hunting )					return;
	if ( ch->position <= POSITION_SLEEP )	return;

	found = FALSE;
	for ( tmp = first_char;  tmp;  tmp = tmp->next )
	{
		if ( ch->hunting->who == tmp )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_command( ch, "say Dannazione! La preda mi è sfuggita!!", CO );
		stop_hunting( ch );
		return;
	}

	if ( ch->in_room == ch->hunting->who->in_room )
	{
		if ( ch->fighting )
			return;

		found_prey( ch, ch->hunting->who );
		return;
	}

	ret = find_first_step( ch->in_room, ch->hunting->who->in_room, 500 + get_level(ch)*12 );
	if ( ret < 0 )
	{
		send_command( ch, "say Dannazione! La preda mi è sfuggita!!", CO );
		stop_hunting( ch );
		return;
	}

	if ( (pexit = get_exit(ch->in_room, ret)) == NULL )
	{
		send_log( NULL, LOG_BUG, "hunt_victim: uscita perduta?" );
		return;
	}

	move_char( ch, pexit, FALSE, FALSE );

	if ( char_died(ch) )
		return;

	if ( !ch->hunting )
	{
		if ( !ch->in_room )
		{
			send_log( NULL, LOG_BUG, "hunt_victim: nessun ch->in_room!  Mob #%d, nome: %s.  Posizionato il mob nel limbo.",
				ch->pIndexData->vnum, ch->name );
			char_to_room( ch, get_room_index(NULL, VNUM_ROOM_LIMBO) );
			return;
		}
		send_command( ch, "say Dannazione! La preda mi è sfuggita!!", CO );
		return;
	}

	if ( ch->in_room == ch->hunting->who->in_room )
	{
		found_prey( ch, ch->hunting->who );
	}
	else
	{
		CHAR_DATA *vch;

		vch = scan_for_victim( ch, pexit, ch->hunting->name );

		/* Cerca di effettuare un attacco a distanza se possibile */
		if ( vch && !mob_fire(ch, ch->hunting->who->name) )
			; /* (FF) codice relativo agli attacchi a distanza, spell o armi da lancio che sia */
	}
}
