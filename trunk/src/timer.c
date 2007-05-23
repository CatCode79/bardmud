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
#include "db.h"
#include "editor.h"
#include "timer.h"


/*
 * Add a timer to ch
 * Support for "call back" time delayed commands
 * (FF) anche qui fare una funzione di lag rpg-speed come l'altra
 */
void add_timer( CHAR_DATA *ch, int type, int pulse, DO_FUN *fun, int value )
{
	TIMER *timer;

	for ( timer = ch->first_timer;  timer;  timer = timer->next )
	{
		if ( timer->type == type )
		{
			timer->pulse	= pulse;
			timer->do_fun	= fun;
			timer->value	= value;
			break;
		}
	}

	if ( !timer )
	{
		CREATE( timer, TIMER, 1 );

		timer->pulse	= pulse;
		timer->type		= type;
		timer->do_fun	= fun;
		timer->value	= value;

		LINK( timer, ch->first_timer, ch->last_timer, next, prev );
	}
}


TIMER *get_timerptr( CHAR_DATA *ch, int type )
{
	TIMER *timer;

	for ( timer = ch->first_timer;  timer;  timer = timer->next )
	{
		if ( timer->type == type )
			return timer;
	}

	return NULL;
}


int get_timer( CHAR_DATA *ch, int type )
{
	TIMER *timer;

	timer = get_timerptr( ch, type );
	if ( timer )
		return timer->pulse;

	return 0;
}


void extract_timer( CHAR_DATA *ch, TIMER *timer )
{
	if ( !timer )
	{
		send_log( NULL, LOG_BUG, "extract_timer: NULL timer" );
		return;
	}

	UNLINK( timer, ch->first_timer, ch->last_timer, next, prev );
	DISPOSE( timer );
}


void remove_timer( CHAR_DATA *ch, int type )
{
	TIMER *timer;

	for ( timer = ch->first_timer;  timer;  timer = timer->next )
	{
		if ( timer->type == type )
			break;
	}

	if ( timer )
		extract_timer( ch, timer );
}


void update_timer( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	TIMER	  *timer;
	TIMER	  *timer_next;

	for ( ch = first_char;  ch;  ch = ch_next )
	{
		set_cur_char( ch );
		ch_next = ch->next;

		for ( timer = ch->first_timer;  timer;  timer = timer_next )
		{
			timer_next = timer->next;

			timer->pulse--;
			if ( timer->pulse > 0 )
				continue;

			if ( timer->type == TIMER_ASUPRESSED )
			{
				if ( timer->value == -1 )
				{
					timer->pulse = 50000;
					continue;
				}
			}

			if ( timer->type == TIMER_DO_FUN )
			{
				int tempsub;

				tempsub = ch->substate;
				ch->substate = timer->value;
				(timer->do_fun)( ch, "" );
				if ( char_died(ch) )
					break;
				ch->substate = tempsub;
			}

			extract_timer( ch, timer );
		} /* chiude il for */
	}

	tail_chain( );
}


/*
 * Check to see if player's attacks are (still?) suppressed
 */
bool is_attack_supressed( CHAR_DATA *ch )
{
	TIMER *timer;

	if ( IS_MOB(ch) )
		return FALSE;

	timer = get_timerptr( ch, TIMER_ASUPRESSED );

	if ( !timer )
		return FALSE;

	/* perma-supression -- bard? (can be reset at end of fight, or spell, etc) */
	if ( timer->value == -1 )
		return TRUE;

	/* this is for timed supressions */
	if ( timer->pulse >= 1 )
		return TRUE;

	return FALSE;
}


/*
 * Check for a timer delayed command (search, dig, detrap, etc)
 */
bool check_delayed_command( CHAR_DATA *ch )
{
	TIMER	*timer;

	timer = get_timerptr( ch, TIMER_DO_FUN );
	if ( timer )
	{
		int		tempsub;

		tempsub = ch->substate;
		ch->substate = SUB_TIMER_DO_ABORT;
		(timer->do_fun) ( ch, "" );

		if ( char_died(ch) )
			return FALSE;

		if ( ch->substate != SUB_TIMER_CANT_ABORT )
		{
			ch->substate = tempsub;
			extract_timer( ch, timer );
		}
		else
		{
			ch->substate = tempsub;
			return FALSE;
		}
	}

	return TRUE;
}
