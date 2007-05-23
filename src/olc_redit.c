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


/**************************************************************************
 *	   OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)		  *

 *	  Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin	  *
\**************************************************************************/


/**************************************************************************\
 >						Room editing module (medit.c)					 <
\**************************************************************************/

#include <ctype.h>
#include <stdarg.h>

#include "mud.h"
#include "build.h"
#include "db.h"
#include "editor.h"
#include "interpret.h"
#include "movement.h"
#include "mprog.h"
#include "nanny.h"
#include "olc.h"
#include "room.h"


/*
 * Il menu principale.
 */
void redit_disp_menu( DESCRIPTOR_DATA *d )
{
	char	 	 buf[MSL];
	ROOM_DATA	*room = d->character->dest_buf;
	char		*sect;


	switch ( room->sector )
	{
	  default:					sect = "None";			break;
	  case SECTOR_INSIDE:			sect = "Inside";		break;
	  case SECTOR_CITY:			sect = "City";			break;
	  case SECTOR_FIELD:			sect = "Field";			break;
	  case SECTOR_FOREST:			sect = "Forest";		break;
	  case SECTOR_HILLS:			sect = "Hills";			break;
	  case SECTOR_MOUNTAIN:		sect = "Mountains";		break;
	  case SECTOR_WATER_SWIM:		sect = "Swim";			break;
	  case SECTOR_WATER_NOSWIM:	sect = "Noswim";		break;
	  case SECTOR_UNDERWATER:		sect = "Underwater";	break;
	  case SECTOR_AIR:			sect = "Air";			break;
	  case SECTOR_DESERT:			sect = "Desert";		break;
	  case SECTOR_OCEANFLOOR:		sect = "Oceanfloor";	break;
	  case SECTOR_UNDERGROUND:	sect = "Underground";	break;
	}

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	sprintf( buf,
		"&w-- Room number : [&c%d&w]	  Room area: [&c%-30.30s&w]\r\n"
		"&g1&w) Name		: &O%s\r\n"
		"&g2&w) Description :\r\n&O%s"
		"&g3&w) Room flags  : &c%s\r\n"
		"&g4&w) Sector type : &c%s\r\n"
		"&g5&w) Tunnel	  : &c%d\r\n"
		"&g6&w) TeleDelay   : &c%d\r\n"
		"&g7&w) TeleVnum	: &c%d\r\n"
		"&gA&w) Exit menu\r\n"
		"&gB&w) Extra descriptions menu\r\n"
		"&gQ&w) Quit\r\n"
		"Enter choice : ",

		OLC_NUM(d),
		(room->area)  ?  room->area->name  :  "None???",
		room->name,
		room->description,
		code_bit(NULL, room->flags, CODE_ROOM),
		sect,
		room->tunnel,
		room->tele_delay,
		room->tele_vnum );

	set_char_color( AT_PLAIN, d->character );
	send_to_char( d->character, buf );

	OLC_MODE(d) = REDIT_MAIN_MENU;
}


DO_RET do_oredit( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA	*d;
	ROOM_DATA		*room;
	char			arg[MIL];

	if ( IS_MOB(ch) || !ch->desc )
	{
		send_to_char( ch, "I don't think so...\r\n" );
		return;
	}

	argument = one_argument( argument, arg );

	if ( !VALID_STR(argument) )
		room = ch->in_room;
	else
	{
		if ( is_number(arg) )
		{
			argument = one_argument( argument, arg );
			room = get_room_index( NULL, atoi(arg) );
		}
		else
		{
			send_to_char( ch, "Vnum must be specified in numbers!\r\n" );
			return;
		}
	}

	if ( !room )
	{
		send_to_char( ch, "That room does not exist!\r\n" );
		return;
	}

	/* Make sure the room isnt already being edited */
	for ( d = first_descriptor; d; d = d->next )
	{
		if ( d->connected == CON_REDIT )
		{
			if ( d->olc && OLC_VNUM(d) == room->vnum )
			{
				ch_printf( ch, "That room is currently being edited by %s.\r\n", d->character->name );
				return;
			}
		}
	}

	if ( !can_modify_room(ch, room) )
		return;

	d = ch->desc;
	CREATE( d->olc, OLC_DATA, 1 );
	OLC_VNUM(d)	= room->vnum;
	OLC_CHANGE(d) = FALSE;
	d->character->dest_buf = room;
	d->connected = CON_REDIT;
	redit_disp_menu( d );

	act( AT_ACTION, "$n starts using OLC.", ch, NULL, NULL, TO_ADMIN );
}


/*
 * (RR) wow...
 */
DO_RET do_rcopy( CHAR_DATA *ch, char *argument )
{
	return;
}


bool is_in_olc( DESCRIPTOR_DATA *d )
{
	if ( !d )
		return FALSE;

	if ( !d->character || IS_MOB(d->character) )
		return FALSE;

	/* objs */
	if ( d->connected == CON_OEDIT )
		return TRUE;
	/* mobs */
	if ( d->connected == CON_MEDIT  )
		return TRUE;
	/* rooms */
	if ( d->connected == CON_REDIT )
		return TRUE;

	return FALSE;
}


/*
 * Log all changes to catch those sneaky bastards =)
 */
void olc_log( DESCRIPTOR_DATA *d, char *format, ... )
{
	ROOM_DATA	*room	= d->character->dest_buf;
	OBJ_DATA	*obj	= d->character->dest_buf;
	CHAR_DATA	*victim = d->character->dest_buf;
	char		 logline[MSL];
	char		 log_buf[MSL];
	va_list		 args;

	if ( !d )
	{
		send_log( NULL, LOG_OLC, "olc_log: called with null descriptor" );
		return;
	}

	va_start( args, format );
	vsprintf( logline, format, args );
	va_end( args );

	if		( d->connected == CON_REDIT )
		sprintf( log_buf, "%s ROOM(%d): ", d->character->name, room->vnum );
	else if ( d->connected == CON_OEDIT )
		sprintf( log_buf, "%s OBJ(%d): ", d->character->name, obj->vnum );
	else if ( d->connected == CON_MEDIT )
	{
		if ( IS_MOB(victim) )
			sprintf( log_buf, "%s MOB(%d): ", d->character->name, victim->pIndexData->vnum );
		else
			sprintf( log_buf, "%s PLR(%s): ", d->character->name, victim->name );
	}
	else
	{
		send_log( NULL, LOG_OLC, "olc_log: called with a bad connected state: %d", d->connected );
		return;
	}

	send_log( NULL, LOG_OLC, "olc_log: %s%s", log_buf, logline );
}


/**************************************************************************
  Menu functions
 **************************************************************************/

/*
 * Nice fancy redone Extra Description stuff :)
 */
void redit_disp_extradesc_prompt_menu( DESCRIPTOR_DATA *d )
{
	EXTRA_DESCR_DATA *ed;
	ROOM_DATA		 *room = d->character->dest_buf;
	int				  counter = 0;

	for ( ed = room->first_extradescr;  ed;  ed = ed->next )
		ch_printf( d->character, "&g%2d&w) %-40.40s\r\n", counter++, ed->keyword );

	send_to_char( d->character, "\r\nWhich extra description do you want to edit? " );
}


void redit_disp_extradesc_menu( DESCRIPTOR_DATA *d )
{
	ROOM_DATA	*room = d->character->dest_buf;
	int			 count = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	if ( room->first_extradescr )
	{
		EXTRA_DESCR_DATA *ed;

		for ( ed = room->first_extradescr;  ed;  ed = ed->next )
			ch_printf( d->character, "&g%2d&w) Keyword: &O%s\r\n", ++count, ed->keyword );

		send_to_char( d->character, "\r\n" );
	}

	send_to_char( d->character, "&gA&w) Add a new description\r\n" );
	send_to_char( d->character, "&gR&w) Remove a description\r\n" );
	send_to_char( d->character, "&gQ&w) Quit\r\n" );
	send_to_char( d->character, "\r\nEnter choice: " );

	OLC_MODE(d) = REDIT_EXTRADESC_MENU;
}


/* For exits */
void redit_disp_exit_menu( DESCRIPTOR_DATA *d )
{
	ROOM_DATA	*room = d->character->dest_buf;
	EXIT_DATA	*pexit;
	int			 cnt;

	OLC_MODE(d) = REDIT_EXIT_MENU;
	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( cnt = 0, pexit = room->first_exit;  pexit;  pexit = pexit->next )
	{
		ch_printf( d->character,
			"&g%2d&w) %-10.10s to %-5d.  Key: %d  Keywords: %s  Flags: %s.\r\n",
			++cnt,
			code_name(NULL, pexit->vdir, CODE_DIR),
			(pexit->to_room)  ?  pexit->to_room->vnum  :  0,
			pexit->key,
			code_bit(NULL, pexit->flags, CODE_EXIT),
			(VALID_STR(pexit->keyword))  ?  pexit->keyword  :  "(none)" );
	}

	if ( room->first_exit )
		send_to_char( d->character, "\r\n" );

	send_to_char( d->character, "&gA&w) Add a new exit\r\n" );
	send_to_char( d->character, "&gR&w) Remove an exit\r\n" );
	send_to_char( d->character, "&gQ&w) Quit\r\n" );

	send_to_char( d->character, "\r\nEnter choice: " );
}


void redit_disp_exit_edit( DESCRIPTOR_DATA *d )
{
	char				flags[MSL];
	EXIT_DATA		   *pexit = d->character->spare_ptr;
	int					x;

	flags[0] = '\0';
	for ( x = 0;  x < MAX_EXIT;  x++ )
	{
		if ( pexit->flags && HAS_BIT(pexit->flags, x) )
		{
			strcat( flags, code_name(NULL, x, CODE_EXIT) );
			strcat( flags, " " );
		}
	}

	OLC_MODE(d) = REDIT_EXIT_EDIT;
	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	ch_printf( d->character, "&g1&w) Direction  : &c%s\r\n",	code_name(NULL, pexit->vdir, CODE_DIR) );
	ch_printf( d->character, "&g2&w) To Vnum	: &c%d\r\n",	(pexit->to_room)  ?  pexit->to_room->vnum  :  -1 );
	ch_printf( d->character, "&g3&w) Key		: &c%d\r\n",	pexit->key );
	ch_printf( d->character, "&g4&w) Keyword	: &c%s\r\n",	(VALID_STR(pexit->keyword))  ?  pexit->keyword  :  "(none)" );
	ch_printf( d->character, "&g5&w) Flags      : &c%s\r\n",	(VALID_STR(flags))  ?  flags  :  "(none)" );
	ch_printf( d->character, "&g6&w) Description: &c%s\r\n",	(VALID_STR(pexit->description))  ?  pexit->description  :  "(none)" );
	send_to_char( d->character, "&gQ&w) Quit\r\n" );
	send_to_char( d->character, "\r\nEnter choice: " );
}


void redit_disp_exit_dirs( DESCRIPTOR_DATA *d )
{
	int		x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_DIR;  x++ )
		ch_printf( d->character, "&g%2d&w) %s\r\n", x, code_name(NULL, x, CODE_DIR) );

	send_to_char( d->character, "\r\nChoose a direction: " );
}


/* For exit flags */
void redit_disp_exit_flag_menu( DESCRIPTOR_DATA *d )
{
	EXIT_DATA		   *pexit = d->character->spare_ptr;
	char				buf[MSL];
	int					x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_EXIT;  x++ )
	{
		if ( (x == EXIT_RES1) || (x == EXIT_RES2) || (x == EXIT_PORTAL) )
			continue;
		ch_printf( d->character, "&g%2d&w) %-20.20s\r\n", x+1, code_name(NULL, x, CODE_EXIT) );
	}

	buf[0] = '\0';
	for ( x = 0;  x < MAX_EXIT;  x++ )
	{
		if ( HAS_BIT(pexit->flags, x) )
		{
			strcat( buf, code_name(NULL, x, CODE_EXIT) );
			strcat( buf, " " );
		}
	}

	ch_printf( d->character, "\r\nExit flags: &c%s&w\r\n"
		"Enter room flags, 0 to quit: ", buf );
	OLC_MODE(d) = REDIT_EXIT_FLAGS;
}


/* For room flags */
void redit_disp_flag_menu( DESCRIPTOR_DATA *d )
{
	ROOM_DATA	*room = d->character->dest_buf;
	int			 x;
	int			 columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_ROOM;  x++ )
	{
		if ( x == 28 || x == 29 || x == 31 )	/* (FF) se per caso venissero convertiti i flag di room in ext cambiare questi numeri con le rispettive etichette di enumerazione */
			continue;

		ch_printf( d->character, "&g%2d&w) %-20.20s ", x+1, code_name(NULL, x, CODE_ROOM) );
		if ( !(++columns % 2) )
			send_to_char( d->character, "\r\n" );
	}
	ch_printf( d->character, "\r\nRoom flags: &c%s&w\r\nEnter room flags, 0 to quit : ",
		code_bit(NULL, room->flags, CODE_ROOM) );
	OLC_MODE( d ) = REDIT_FLAGS;
}


/* for sector type */
void redit_disp_sector_menu( DESCRIPTOR_DATA *d )
{
	int		x,
			columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_SECTOR;  x++ )
	{
		if ( x == SECTOR_DUNNO )
			continue;

		ch_printf( d->character, "&g%2d&w) %-20.20s ", x+1, code_name(NULL, x, CODE_SECTOR) );

		if ( !(++columns % 2) )
			send_to_char( d->character, "\r\n" );

	}
	send_to_char( d->character, "\r\nEnter sector type : " );
	OLC_MODE(d) = REDIT_SECTOR;
}


EXTRA_DESCR_DATA *redit_find_extradesc( ROOM_DATA *room, int number )
{
	int				  count = 0;
	EXTRA_DESCR_DATA *ed;

	for ( ed = room->first_extradescr; ed; ed = ed->next )
	{
		if ( ++count == number )
			return ed;
	}

	return NULL;
}


DO_RET do_redit_reset( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA		 *room = ch->dest_buf;
	EXTRA_DESCR_DATA *ed = ch->spare_ptr;

	switch ( ch->substate )
	{
	  case SUB_ROOM_DESCR:
		if ( !ch->dest_buf )
		{
			/* If theres no dest_buf, theres no object, so stick em back as playing */
			ch_printf( ch, "Fatal error: report to Coder.\r\n" );
			send_log( NULL, LOG_OLC, "do_redit_reset: sub_obj_extra: NULL ch->dest_buf" );
			ch->substate = SUB_NONE;
			ch->desc->connected = CON_PLAYING;
			return;
		}
		DISPOSE( room->description );
		room->description = copy_buffer( ch );
		stop_editing( ch );
		ch->dest_buf = room;
		ch->desc->connected = CON_REDIT;
		ch->substate = SUB_NONE;

		olc_log( ch->desc, "Edited room description" );
		redit_disp_menu( ch->desc );
		return;

	  case SUB_ROOM_EXTRA:
		DISPOSE( ed->description );
		ed->description = copy_buffer( ch );
		stop_editing( ch );
		ch->dest_buf = room;
		ch->spare_ptr = ed;
		ch->substate = SUB_NONE;
		ch->desc->connected = CON_REDIT;
		oedit_disp_extra_choice( ch->desc );
		OLC_MODE(ch->desc) = REDIT_EXTRADESC_CHOICE;
		olc_log( ch->desc, "Edit description for exdesc %s", ed->keyword );

		return;
	}
}


/**************************************************************************
  The main loop
 **************************************************************************/

void redit_parse( DESCRIPTOR_DATA *d , char *arg )
{
	ROOM_DATA		 *room  = d->character->dest_buf;
	ROOM_DATA		 *tmp;
	EXIT_DATA		 *pexit = d->character->spare_ptr;
	EXTRA_DESCR_DATA *ed	  = d->character->spare_ptr;
	char			  arg1[MIL];
	char			  buf[MSL];
	int				  number = 0;

	switch ( OLC_MODE(d) )
	{
	  case REDIT_CONFIRM_SAVESTRING:
		switch ( *arg )
		{
		  case 'y':
		  case 'Y':
			/* redit_save_internally(d); */
			send_log( NULL, LOG_OLC, "OLC: %s edits room %d", d->character->name, OLC_NUM(d) );
			cleanup_olc( d );
			send_to_char( d->character, "Room saved to memory.\r\n" );
			break;
		  case 'n':
		  case 'N':
			cleanup_olc( d );
			break;
		  default:
			send_to_char( d->character, "Invalid choice!\r\n" );
			send_to_char( d->character, "Do you wish to save this room internally? : " );
			break;
		}
		return;

	  case REDIT_MAIN_MENU:
		switch ( *arg )
		{
		  case 'q':
		  case 'Q':
/*			if ( OLC_CHANGE(d) )
			{ *. Something has been modified .*
				send_to_char( d->character, "Do you wish to save this room internally? : " );
				OLC_MODE(d) = REDIT_CONFIRM_SAVESTRING;
			}
			else */
			cleanup_olc( d );
			return;
		  case '1':
			send_to_char( d->character, "Enter room name:-\r\n| " );
			OLC_MODE(d) = REDIT_NAME;
			break;
		  case '2':
			OLC_MODE(d) = REDIT_DESC;
			d->character->substate = SUB_ROOM_DESCR;
			d->character->last_cmd = do_redit_reset;

			send_to_char( d->character, "Enter room description:-\r\n" );
			if ( !room->description )
				room->description = str_dup( "" );
			start_editing( d->character, room->description );
			break;
		  case '3':
			redit_disp_flag_menu(d);
			break;
		  case '4':
			redit_disp_sector_menu(d);
			break;
		  case '5':
			send_to_char( d->character, "How many people can fit in the room? " );
			OLC_MODE(d) = REDIT_TUNNEL;
			break;
		  case '6':
			send_to_char( d->character, "How long before people are teleported out? " );
			OLC_MODE(d) = REDIT_TELEDELAY;
			break;
		  case '7':
			send_to_char( d->character, "Where are they teleported to? " );
			OLC_MODE(d) = REDIT_TELEVNUM;
			break;
		  case 'a':
		  case 'A':
			redit_disp_exit_menu(d);
			break;
		  case 'b':
		  case 'B':
			redit_disp_extradesc_menu(d);
			break;

		  default:
			send_to_char( d->character, "Invalid choice!" );
			redit_disp_menu(d);
			break;
		}
		return;

	  case REDIT_NAME:
		DISPOSE( room->name );
		room->name = str_dup( arg );
		olc_log( d, "Changed name to %s", room->name );
		break;

	  case REDIT_DESC:
		/* we will NEVER get here */
		send_log( NULL, LOG_OLC, "redit_parse: reached REDIT_DESC case in redit_parse" );
		break;

	  case REDIT_FLAGS:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
				break;
			else if ( number < 0 || number >= MAX_ROOM )
			{
				send_to_char( d->character, "Invalid flag, try again: " );
				return;
			}
			else
			{
				number--;	/* Offset for 0 */
				TOGGLE_BIT( room->flags, number );
				olc_log( d, "%s the room flag %s",
				HAS_BIT( room->flags, number ) ? "Added" : "Removed",
				code_name(NULL, number, CODE_ROOM) );
			}
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_ROOM );
				if ( number > 0 )
				{
					TOGGLE_BIT( room->flags, number );
					olc_log( d, "%s the room flag %s",
						HAS_BIT( room->flags, number ) ? "Added" : "Removed",
						code_name( NULL, number, CODE_ROOM) );
				}
			}
		}
		redit_disp_flag_menu(d);
		return;

	  case REDIT_SECTOR:
		number = atoi( arg );
		if ( number < 0 || number >= MAX_SECTOR )
		{
			send_to_char( d->character, "Invalid choice!" );
			redit_disp_sector_menu(d);
			return;
		}
		else
			room->sector = number;

		olc_log( d, "Changed sector to %s", code_name(NULL, number, CODE_SECTOR) );
		break;

	  case REDIT_TUNNEL:
		number = atoi( arg );
		room->tunnel = URANGE( 0, number, 1000 );
		olc_log( d, "Changed tunnel amount to %d", room->tunnel );
		break;

	  case REDIT_TELEDELAY:
		number = atoi( arg );
		room->tele_delay = number;
		olc_log( d, "Changed teleportation delay to %d", room->tele_delay );
		break;

	  case REDIT_TELEVNUM:
		number = atoi( arg );
		room->tele_vnum = URANGE( 1, number, MAX_VNUM-1 );
		olc_log( d, "Changed teleportation vnum to %d", room->tele_vnum );
		break;

	  case REDIT_EXIT_MENU:
		switch ( toupper(arg[0]) )
		{
		  default:
			if ( is_number(arg) )
			{
				number = atoi( arg );
				pexit = get_exit_num( room, number );
				d->character->spare_ptr = pexit;
				redit_disp_exit_edit(d);
				return;
			}
			redit_disp_exit_menu( d );
			return;
		  case 'A':
			OLC_MODE(d) = REDIT_EXIT_ADD;
			redit_disp_exit_dirs( d );
			return;
		  case 'R':
			OLC_MODE(d) = REDIT_EXIT_DELETE;
			send_to_char( d->character, "Delete which exit? " );
			return;
		  case 'Q':
			d->character->spare_ptr = NULL;
			break;
		}
		break;

	  case REDIT_EXIT_EDIT:
		switch ( toupper(arg[0]) )
		{
		  case 'Q':
			d->character->spare_ptr = NULL;
			redit_disp_exit_menu(d);
			return;
		  case '1':
			/* OLC_MODE(d) = REDIT_EXIT_DIR;
			redit_disp_exit_dirs(d); */
			send_to_char( d->character, "This option can only be changed by remaking the exit.\r\n" );
			break;
		  case '2':
			OLC_MODE(d) = REDIT_EXIT_VNUM;
			send_to_char( d->character, "Which room does this exit go to? " );
			return;
		  case '3':
			OLC_MODE(d) = REDIT_EXIT_KEY;
			send_to_char( d->character, "What is the vnum of the key to this exit? " );
			return;
		  case '4':
			OLC_MODE(d) = REDIT_EXIT_KEYWORD;
			send_to_char( d->character, "What is the keyword to this exit? " );
			return;
		  case '5':
			OLC_MODE(d) = REDIT_EXIT_FLAGS;
			redit_disp_exit_flag_menu(d);
			return;
		  case '6':
			OLC_MODE(d) = REDIT_EXIT_DESC;
			send_to_char( d->character, "Description:\r\n] " );
			return;
		}
		redit_disp_exit_edit(d);
		return;

	  case REDIT_EXIT_DESC:
		if ( !VALID_STR(arg) )
		{
			DISPOSE( pexit->description );
			pexit->description = str_dup( "" );
		}
		else
		{
			sprintf( buf, "%s\r\n", arg );
			DISPOSE( pexit->description );
			pexit->description = str_dup( buf );
		}
		olc_log( d, "Changed %s description to %s", code_name( NULL, pexit->vdir, CODE_DIR), arg  ?  arg  :  "none" );
		redit_disp_exit_edit(d);
		return;

	  case REDIT_EXIT_ADD:
		if ( is_number( arg ) )
		{
			number = atoi( arg );
			if ( number < 0 || number >= MAX_DIR )
			{
				send_to_char( d->character, "Invalid direction, try again: " );
				return;
			}
			d->character->tempnum = number;
		}
		else
		{
			number = get_dir(arg);
			pexit = get_exit( room, number );
			if ( pexit )
			{
				send_to_char( d->character, "An exit in that direction already exists.\r\n" );
				redit_disp_exit_menu(d);
				return;
			}
			d->character->tempnum = number;
		}
		OLC_MODE(d) = REDIT_EXIT_ADD_VNUM;
		send_to_char( d->character, "Which room does this exit go to? " );
		return;

	  case REDIT_EXIT_ADD_VNUM:
		number = atoi( arg );
		if ( (tmp = get_room_index(NULL, number)) == NULL )
		{
			send_to_char( d->character, "Non-existant room.\r\n" );
			OLC_MODE(d) = REDIT_EXIT_MENU;
			redit_disp_exit_menu(d);
			return;
		}
		pexit = make_exit( room, tmp, d->character->tempnum );
		DISPOSE( pexit->keyword );
		DISPOSE( pexit->description );
		pexit->keyword		= str_dup( "" );
		pexit->description	= str_dup( "" );
		pexit->key			= -1;
		pexit->flags	= 0;
		act( AT_ADMIN, "$n reveals a hidden passage!", d->character, NULL, NULL, TO_ROOM );
		d->character->spare_ptr = pexit;

		olc_log( d, "Added %s exit to %d", code_name( NULL, pexit->vdir, CODE_DIR), pexit->vnum );

		OLC_MODE(d) = REDIT_EXIT_EDIT;
		redit_disp_exit_edit(d);
		return;

	  case REDIT_EXIT_DELETE:
		if ( !is_number( arg ) )
		{
			send_to_char( d->character, "Exit must be specified in a number.\r\n" );
			redit_disp_exit_menu(d);
		}
		number = atoi( arg );
		pexit = get_exit_num( room, number );

		if ( !pexit )
		{
			send_to_char( d->character, "That exit does not exist.\r\n" );
			redit_disp_exit_menu(d);
		}
		olc_log( d, "Removed %s exit", code_name( NULL, pexit->vdir, CODE_DIR) );
		extract_exit( room, pexit );
		redit_disp_exit_menu( d );
		return;

	  case REDIT_EXIT_VNUM:
		number = atoi( arg );
		if ( number < 1 || number >= MAX_VNUM )
		{
			send_to_char( d->character, "Invalid room number, try again : " );
			return;
		}

		if ( get_room_index(NULL, number) == NULL )
		{
			send_to_char( d->character, "That room does not exist, try again: " );
			return;
		}
		pexit->vnum = number;
		olc_log( d, "%s exit vnum changed to %d", code_name( NULL, pexit->vdir, CODE_DIR), pexit->vnum );
		redit_disp_exit_menu( d );
		return;

	  case REDIT_EXIT_KEYWORD:
		DISPOSE( pexit->keyword );
		pexit->keyword = str_dup( arg );
		olc_log( d, "Changed %s keyword to %s", code_name( NULL, pexit->vdir, CODE_DIR), pexit->keyword );
		redit_disp_exit_edit( d );
		return;

	  case REDIT_EXIT_KEY:
		number = atoi( arg );
		if ( number < 1 || number >= MAX_VNUM )
			send_to_char( d->character, "Invalid vnum, try again: " );
		else
		{
			pexit->key = number;
			redit_disp_exit_edit( d );
		}
		olc_log( d, "%s key vnum is now %d", code_name( NULL, pexit->vdir, CODE_DIR), pexit->key );
		return;

	  case REDIT_EXIT_FLAGS:
		number = atoi( arg );
		if ( number == 0 )
		{
			redit_disp_exit_edit( d );
			return;
		}

		if ( number < 0 || number >= MAX_EXIT
		  || ((number-1) == EXIT_RES1)
		  || ((number-1) == EXIT_RES2)
		  || ((number-1) == EXIT_PORTAL) )
		{
			send_to_char( d->character, "That's not a valid choice!\r\n" );
			redit_disp_exit_flag_menu( d );
		}
		number -= 1;
		TOGGLE_BIT( pexit->flags, number );
		olc_log( d, "%s %s to %s exit",
			HAS_BIT(pexit->flags, number)  ?  "Added"  :  "Removed",
			code_name(NULL, number, CODE_EXIT),
			code_name(NULL, pexit->vdir, CODE_DIR) );
		redit_disp_exit_flag_menu( d );
		return;

	  case REDIT_EXTRADESC_DELETE:
		ed = redit_find_extradesc( room, atoi(arg) );
		if ( !ed )
		{
			send_to_char( d->character, "Not found, try again: " );
			return;
		}
		olc_log( d, "Deleted exdesc %s", ed->keyword );
		UNLINK( ed, room->first_extradescr, room->last_extradescr, next, prev );
		DISPOSE( ed->keyword );
		DISPOSE( ed->description );
		DISPOSE( ed );
		top_ed--;
		redit_disp_extradesc_menu(d);
		return;

	  case REDIT_EXTRADESC_CHOICE:
		switch ( toupper( arg[0] ) )
		{
		  case 'Q':
			if ( !ed->keyword || !ed->description )
			{
				send_to_char( d->character, "No keyword and/or description, junking..." );
				UNLINK( ed, room->first_extradescr, room->last_extradescr, next, prev );
				DISPOSE( ed->keyword );
				DISPOSE( ed->keyword );
				DISPOSE( ed );
				top_ed--;
			}
			d->character->spare_ptr = NULL;
			redit_disp_extradesc_menu(d);
			return;
		  case '1':
			OLC_MODE(d) = REDIT_EXTRADESC_KEY;
			send_to_char( d->character, "Keywords, seperated by spaces: " );
			return;
		  case '2':
			OLC_MODE(d) = REDIT_EXTRADESC_DESCRIPTION;
			d->character->substate = SUB_ROOM_EXTRA;
			d->character->last_cmd = do_redit_reset;
			send_to_char( d->character, "Enter new extradesc description: \r\n" );
			start_editing( d->character, ed->description );
			return;
		}
		break;

	  case REDIT_EXTRADESC_KEY:
/*		if ( SetRExtra( room, arg ) )
		{
			send_to_char( d->character, "A extradesc with that keyword already exists.\r\n" );
			redit_disp_extradesc_menu(d);
			return;
		} */
		olc_log( d, "Changed exkey %s to %s", ed->keyword, arg );
		DISPOSE( ed->keyword );
		ed->keyword = str_dup( arg );
		oedit_disp_extra_choice(d);
		OLC_MODE(d) = REDIT_EXTRADESC_CHOICE;
		return;

	  case REDIT_EXTRADESC_MENU:
		switch ( toupper( arg[0] ) )
		{
		  case 'Q':
			break;
		  case 'A':
			CREATE( ed, EXTRA_DESCR_DATA, 1 );
			LINK( ed, room->first_extradescr, room->last_extradescr, next, prev );
			ed->keyword = str_dup( "" );
			ed->description = str_dup( "" );
			top_ed++;
			d->character->spare_ptr = ed;
			olc_log( d, "Added new exdesc" );
			oedit_disp_extra_choice(d);
			OLC_MODE(d) = REDIT_EXTRADESC_CHOICE;
			return;
		  case 'R':
			OLC_MODE(d) = REDIT_EXTRADESC_DELETE;
			send_to_char( d->character, "Delete which extra description? " );
			return;
		  default:
			if ( is_number(arg) )
			{
				ed = redit_find_extradesc( room, atoi(arg) );
				if ( !ed )
				{
					send_to_char( d->character, "Not found, try again: " );
					return;
				}
				d->character->spare_ptr = ed;
				oedit_disp_extra_choice(d);
				OLC_MODE(d) = REDIT_EXTRADESC_CHOICE;
			}
			else
				redit_disp_extradesc_menu(d);

			return;
		}
		break;

	  default:
		/* we should never get here */
		send_log( NULL, LOG_OLC, "redit_parse: reached default case in parse_redit" );
		break;
	} /* chiude lo switch */
	/* Log the changes, so we can keep track of those sneaky bastards */
	/* Don't log on the flags cause it does that above */
/*	if ( OLC_MODE(d) != REDIT_FLAGS )
		olc_log( d, arg );
*/
	/*. If we get this far, something has be changed .*/
	OLC_CHANGE(d) = TRUE;
	redit_disp_menu(d);
}
