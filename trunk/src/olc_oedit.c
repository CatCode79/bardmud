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
 *     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)         *
 *                                                                        *
 *   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin      *
 **************************************************************************/


/**************************************************************************\
 >						Object editing module (oedit.c)					  <
\**************************************************************************/

#include <ctype.h>

#include "mud.h"
#include "affect.h"
#include "build.h"
#include "db.h"
#include "editor.h"
#include "interpret.h"
#include "liquid.h"
#include "mprog.h"
#include "nanny.h"
#include "olc.h"


/*
 * Internal functions
 */
void oedit_disp_layer_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_container_flags_menu ( DESCRIPTOR_DATA *d );
void oedit_disp_lever_flags_menu	 ( DESCRIPTOR_DATA *d );
void oedit_disp_extradesc_menu		 ( DESCRIPTOR_DATA *d );
void oedit_disp_weapon_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_val1_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_val2_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_val3_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_val4_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_val5_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_val6_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_type_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_extra_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_wear_menu			 ( DESCRIPTOR_DATA *d );
void oedit_disp_menu				 ( DESCRIPTOR_DATA *d );
void oedit_disp_spells_menu			 ( DESCRIPTOR_DATA *d );
void oedit_liquid_type				 ( DESCRIPTOR_DATA *d );
/* void oedit_save_internally			( DESCRIPTOR_DATA *d );	non usata, commentata una chiamata sotto */


/*------------------------------------------------------------------------*/
void cleanup_olc( DESCRIPTOR_DATA *d )
{
	if ( d->olc )
	{
		if ( d->character )
		{
			d->character->dest_buf = NULL;
/*			act( AT_ACTION, "$n stops using OLC.", d->character, NULL, NULL, TO_ROOM ); */
		}

		d->connected = CON_PLAYING;
		DISPOSE( d->olc );
	}
}


/*
 * Starts it all off
 */
DO_RET do_ooedit( CHAR_DATA *ch, char *argument )
{
	char			 arg[MIL];
	DESCRIPTOR_DATA *d;
	OBJ_DATA		*obj;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I don't think so...\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "OEdit what?\r\n" );
		return;
	}

	if ( (obj = get_obj_world(ch, arg)) == NULL )
	{
		send_to_char( ch, "Nothing like that in hell, earth, or heaven.\r\n" );
		return;
	}

	/* Make sure the object isnt already being edited */
	for ( d = first_descriptor; d; d = d->next )
	{
		if ( d->connected == CON_OEDIT )
		{
			if ( d->olc && OLC_VNUM(d) == obj->vnum )
			{
				ch_printf( ch, "That object is currently being edited by %s.\r\n", d->character->name );
				return;
			}
		}
	}

	if ( !can_modify_obj(ch, obj) )
		return;

	d = ch->desc;
	CREATE( d->olc, OLC_DATA, 1 );
	OLC_VNUM(d) = obj->vnum;
	OLC_CHANGE(d) = FALSE;
	OLC_VAL(d) = 0;
	d->character->dest_buf = obj;
	d->connected = CON_OEDIT;
	oedit_disp_menu( d );

	act( AT_ACTION, "$n starts using OLC.", ch, NULL, NULL, TO_ADMIN );
}


DO_RET do_ocopy( CHAR_DATA *ch, char *argument )
{
	OBJ_PROTO_DATA	 *orig;
	OBJ_PROTO_DATA	 *copy;
	char			  arg1[MIL];
	char			  arg2[MIL];
	VNUM			  ovnum;
	VNUM			  cvnum;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Usage: ocopy <original> <new>\r\n" );
		return;
	}

	ovnum = atoi( arg1 );
	cvnum = atoi( arg2 );

	if ( get_trust(ch) < TRUST_BUILDER )
	{
		AREA_DATA *pArea;

		if ( !ch->pg || !(pArea = ch->pg->build_area) )
		{
			send_to_char( ch, "You must have an assigned area to copy objects.\n\r" );
			return;
		}
		if ( cvnum < pArea->vnum_low
		  || cvnum > pArea->vnum_high )
		{
			send_to_char( ch, "That number is not in your allocated range.\n\r" );
			return;
		}
	}

	if ( get_obj_proto(NULL, cvnum) )
	{
		send_to_char( ch, "That object already exists.\r\n" );
		return;
	}

	orig = get_obj_proto( NULL, ovnum );
	if ( !orig )
	{
		send_to_char( ch, "How can you copy something that doesnt exist?\r\n" );
		return;
	}

	if ( !orig->prototype )
	{
		send_to_char( ch, "L'oggetto non è un prototipo.\r\n" );
		return;
	}

	CREATE( copy, OBJ_PROTO_DATA, 1 );
	copy_object( orig, copy );

	copy->vnum = cvnum;
	copy->count = 0;	/* forse zero perché è un prototipo? (??) nella funzione copy_object c'è count = 1 cmq */

	LINK( copy, first_object_proto, last_obj_proto, next, prev );
	top_obj_proto++;

	set_char_color( AT_PLAIN, ch );
	send_to_char( ch, "Object copied.\r\n" );
}


/**************************************************************************
 Menu functions
 **************************************************************************/

void oedit_disp_progs( DESCRIPTOR_DATA *d )
{
	OBJ_DATA   *obj = d->character->dest_buf;
	MPROG_DATA *mprg;
	int			count;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( count = 0, mprg = obj->first_mudprog;  mprg;  mprg = mprg->next )
		ch_printf( d->character, "&g%2d&w) &c%s\r\n", ++count, code_name(NULL, mprg->type, CODE_MPTRIGGER) );

	if ( obj->first_mudprog )
		send_to_char( d->character, "\r\n" );

	send_to_char( d->character, "\r\nNon è possibile modificare i mobprog." );

	OLC_MODE(d) = OEDIT_MAIN_MENU;
}


void oedit_disp_prog_types( DESCRIPTOR_DATA *d )
{
	int count;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( count = 0;  count < MAX_MPTRIGGER;  count++ )
		ch_printf( d->character, "&g%2d&w) %s\r\n", count, code_name(NULL, count, CODE_MPTRIGGER) );

	send_to_char( d->character, "\r\nEnter type: " );
}


void oedit_disp_prog_choice( DESCRIPTOR_DATA *d )
{
	MPROG_DATA *mprg = d->character->spare_ptr;
	char		buf[MSL];

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	buf[0] = '\0';
	if ( mprg->first_mpcom )
	{
		MPROG_COM_LIST *mpcom;

		strcat( buf, "\r\n" );
		for ( mpcom = mprg->first_mpcom;  mpcom;  mpcom = mpcom->next )
			sprintf( buf+strlen(buf), "%s\r\n", mpcom->com );
	}

	ch_printf( d->character, "&gA&w) Type: &c%s\r\n", code_name(NULL, mprg->type, CODE_MPTRIGGER) );
	ch_printf( d->character, "&gB&w) Args: &c%s\r\n", mprg->arglist );
	ch_printf( d->character, "&gC&w) Prog: %s\r\n", buf );
	send_to_char( d->character, "&gQ&w) Quit\r\n" );

	send_to_char( d->character, "\r\nEnter choice: " );
}


/*
 * For container flags
 */
void oedit_disp_container_flags_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA  *obj = d->character->dest_buf;
	int		   x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_CONTAINER;  x++ )
		ch_printf( d->character, "&g%d&w) %s\r\n", x+1, code_name(NULL, x, CODE_CONTAINER) );

	ch_printf( d->character, "Container flags: &c%s&w\r\n", code_bitint(NULL, obj->container->flags, CODE_CONTAINER) );
	send_to_char( d->character, "Enter flag, 0 to quit : " );
}


/*
 * Display lever flags menu
 */
void oedit_disp_lever_flags_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	int		  x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_TRIG;  x++ )
		ch_printf( d->character, "&g%2d&w) %s\r\n", x+1, code_name(NULL, x, CODE_TRIG) );

	ch_printf( d->character, "Lever flags: &c%s&w\r\nEnter flag, 0 to quit: ",
		code_bitint(NULL, obj->lever->flags, CODE_TRIG) );
}


/*
 * Fancy layering stuff, trying to lessen confusion :)
 */
void oedit_disp_layer_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;

	OLC_MODE(d) = OEDIT_LAYERS;
	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	send_to_char( d->character, "Choose which layer, or combination of layers fits best: \r\n\r\n" );
	ch_printf( d->character, "[&c%s&w] &g1&w) Nothing Layers\r\n",	IS_EMPTY(obj->layers	)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g2&w) Silk Shirt\r\n",		HAS_BIT(obj->layers,   1)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g3&w) Leather Vest\r\n",	HAS_BIT(obj->layers,   2)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g4&w) Light Chainmail\r\n",	HAS_BIT(obj->layers,   4)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g5&w) Leather Jacket\r\n",	HAS_BIT(obj->layers,   8)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g6&w) Light Cloak\r\n",		HAS_BIT(obj->layers,  16)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g7&w) Loose Cloak\r\n",		HAS_BIT(obj->layers,  32)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g8&w) Cape\r\n",			HAS_BIT(obj->layers,  64)  ?  "X"  :  " " );
	ch_printf( d->character, "[&c%s&w] &g9&w) Magical Effects\r\n",	HAS_BIT(obj->layers, 128)  ?  "X"  :  " " );
	send_to_char( d->character, "\r\nLayer or 0 to exit: " );
}


/*
 * For extra descriptions
 */
void oedit_disp_extradesc_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA  *obj = d->character->dest_buf;
	int		   count = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	if ( obj->first_extradescr )
	{

		EXTRA_DESCR_DATA *ed;

		for ( ed = obj->first_extradescr;  ed;  ed = ed->next )
			ch_printf( d->character, "&g%2d&w) Keyword: &O%s\r\n", ++count, ed->keyword );
	}

	if ( obj->first_extradescr )
		send_to_char( d->character, "\r\n" );

	send_to_char( d->character, "&gA&w) Add a new description\r\n" );
	send_to_char( d->character, "&gR&w) Remove a description\r\n" );
	send_to_char( d->character, "&gQ&w) Quit\r\n" );
	send_to_char( d->character, "\r\nEnter choice: " );

	OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}


void oedit_disp_extra_choice( DESCRIPTOR_DATA *d )
{
	EXTRA_DESCR_DATA *ed = d->character->spare_ptr;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	ch_printf( d->character, "&g1&w) Keyword: &O%s\r\n", ed->keyword );
	ch_printf( d->character, "&g2&w) Description: \r\n&O%s&w\r\n", ed->description );
	send_to_char( d->character, "\r\nChange which option? " );

	OLC_MODE(d) = OEDIT_EXTRADESC_CHOICE;
}


/*
 * Ask for *which* apply to edit and prompt for some other options
 */
void oedit_disp_prompt_apply_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA	*obj = d->character->dest_buf;
	AFFECT_DATA *paf;
	int			 counter = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( paf = obj->first_affect;  paf;  paf = paf->next )
	{
		ch_printf( d->character, " &g%2d&w) ", counter++ );
		show_affect( d->character, paf );
	}

	send_to_char( d->character, " \r\n &gA&w) Add an affect\r\n" );
	send_to_char( d->character, " &gR&w) Remove an affect\r\n"	  );
	send_to_char( d->character, " &gQ&w) Quit\r\n"				  );
	send_to_char( d->character, "\r\nEnter option or affect#: "  );
	OLC_MODE(d) = OEDIT_AFFECT_MENU;
}


/*
 * Ask for liquid type.
 */
void oedit_liquid_type( DESCRIPTOR_DATA *d )
{
	int		x;
	int		col = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_LIQ;  x++ )
	{
		ch_printf( d->character, " &w%2d&g) &c%-20.20s ", x, table_liquid[x].name );
		if ( ++col%3 == 0 )
			send_to_char( d->character, "\r\n" );
		if ( col%3 == 0 && x == MAX_LIQ-1 )
			send_to_char( d->character, "\r\n" );
	}
	send_to_char( d->character, "\r\n&wEnter drink type: " );
	OLC_MODE(d) = OEDIT_VALUE_3;
}


/*
 * Display the menu of apply types
 */
void oedit_disp_affect_menu( DESCRIPTOR_DATA *d )
{
	int		x;
	int		col = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
	for ( x = 0;  x < MAX_APPLY;  x++ )
	{
		/* Don't want people choosing these ones */
		if ( x == 0 || x == APPLY_EXT_AFFECT )
			continue;

		ch_printf( d->character, "&g%2d&w) %-20.20s ", x, code_name(NULL, x, CODE_APPLY) );

		if ( ++col%3 == 0 )
			send_to_char( d->character, "\r\n" );
		if ( col%3 == 0 && x == MAX_APPLY-1 )
			send_to_char( d->character, "\r\n" );
	}
	send_to_char( d->character, "\r\nEnter apply type (0 to quit): " );
	OLC_MODE(d) = OEDIT_AFFECT_LOCATION;
}


/*
 * Display menu of weapon types
 */
void oedit_disp_weapon_menu( DESCRIPTOR_DATA *d )
{
	int		x;
	int		col = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_ATTACK;  x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s ", x+1, code_name(NULL, x, CODE_ATTACK) );
		if ( ++col%2 == 0 )
			send_to_char( d->character, "\r\n" );
		if ( col%2 == 0 && x == MAX_ATTACK-1 )
			send_to_char( d->character, "\r\n" );
	}
	send_to_char( d->character, "\r\nEnter weapon type: " );
}


/*
 * Spell type
 */
void oedit_disp_spells_menu( DESCRIPTOR_DATA *d )
{
	send_to_char( d->character, "Enter the name of the spell: " );
}


/*
 * Object value 0
 */
void oedit_disp_val1_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	OLC_MODE(d) = OEDIT_VALUE_1;

	switch ( obj->type )
	{
	  case OBJTYPE_LIGHT:
		/* values 0 and 1 are unused.. jump to 2 */
		oedit_disp_val3_menu(d);
		break;

	  case OBJTYPE_SALVE:
	  case OBJTYPE_PILL:
	  case OBJTYPE_SCROLL:
	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
	  case OBJTYPE_POTION:
		send_to_char( d->character, "Spell level : " );
		break;

	  case OBJTYPE_MISSILE_WEAPON:
	  case OBJTYPE_WEAPON:
		send_to_char( d->character, "Condition : " );
		break;

	  case OBJTYPE_ARMOR:
	  case OBJTYPE_DRESS:
		send_to_char( d->character, "Current AC : " );
		break;

/*	  case OBJTYPE_QUIVER:
	  case OBJTYPE_KEYRING:
*/
	  case OBJTYPE_PIPE:
	  case OBJTYPE_CONTAINER:
	  case OBJTYPE_DRINK_CON:
	  case OBJTYPE_FOUNTAIN:
		send_to_char( d->character, "Capacity : " );
		break;

	  case OBJTYPE_FOOD:
		send_to_char( d->character, "Hours to fill stomach : " );
		break;

	  case OBJTYPE_MONEY:
		send_to_char( d->character, "Amount of gold coins : " );
		break;

	  case OBJTYPE_HERB:
		/* Value 0 unused, skip to 1 */
		oedit_disp_val2_menu( d );
		break;

	  case OBJTYPE_LEVER:
	  case OBJTYPE_SWITCH:
		oedit_disp_lever_flags_menu( d );
		break;

	  case OBJTYPE_TRAP:
		send_to_char( d->character, "Charges: " );
		break;

	  default:
		oedit_disp_menu( d );
	}
}


/*
 * object value 1
 */
void oedit_disp_val2_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	OLC_MODE(d) = OEDIT_VALUE_2;

	switch (obj->type)
	{
	  case OBJTYPE_PILL:
	  case OBJTYPE_SCROLL:
	  case OBJTYPE_POTION:
		oedit_disp_spells_menu(d);
		break;

	  case OBJTYPE_SALVE:
	  case OBJTYPE_HERB:
		send_to_char( d->character, "Charges: " );
		break;

	  case OBJTYPE_PIPE:
		send_to_char( d->character, "Number of draws: " );
		break;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		send_to_char( d->character, "Max number of charges : " );
		break;

	  case OBJTYPE_WEAPON:
		send_to_char( d->character, "Number of damage dice : " );
		break;

	  case OBJTYPE_FOOD:
		send_to_char( d->character, "Condition: " );
		break;

	  case OBJTYPE_CONTAINER:
		oedit_disp_container_flags_menu(d);
		break;

	  case OBJTYPE_DRINK_CON:
	  case OBJTYPE_FOUNTAIN:
		send_to_char( d->character, "Quantity : " );
		break;

	  case OBJTYPE_ARMOR:
		send_to_char( d->character, "Original AC: " );
		break;

	  case OBJTYPE_LEVER:
	  case OBJTYPE_SWITCH:
		if ( HAS_BITINT(obj->lever->flags, TRIG_CAST) )
			oedit_disp_spells_menu(d);
		else
			send_to_char( d->character, "Vnum: " );
		break;

	  default:
		oedit_disp_menu(d);
	}
}


/*
 * object value 2
 */
void oedit_disp_val3_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	OLC_MODE(d) = OEDIT_VALUE_3;

	switch ( obj->type )
	{
	  case OBJTYPE_LIGHT:
		send_to_char( d->character, "Number of hours (0 = burnt, -1 is infinite) : " );
		break;

	  case OBJTYPE_PILL:
	  case OBJTYPE_SCROLL:
	  case OBJTYPE_POTION:
		oedit_disp_spells_menu(d);
		break;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		send_to_char( d->character, "Number of charges remaining : " );
		break;

	  case OBJTYPE_WEAPON:
		send_to_char( d->character, "Size of damage dice : " );
		break;

	  case OBJTYPE_CONTAINER:
		send_to_char( d->character, "Vnum of key to open container (-1 for no key) : " );
		break;

	  case OBJTYPE_DRINK_CON:
	  case OBJTYPE_FOUNTAIN:
		oedit_liquid_type(d);
		break;

	  default:
		oedit_disp_menu(d);
	}
}


/*
 * object value 3
 */
void oedit_disp_val4_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	OLC_MODE(d) = OEDIT_VALUE_4;

	switch ( obj->type )
	{
	  case OBJTYPE_SCROLL:
	  case OBJTYPE_POTION:
	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		oedit_disp_spells_menu(d);
		break;

	  case OBJTYPE_WEAPON:
		oedit_disp_weapon_menu(d);
		break;

	  case OBJTYPE_DRINK_CON:
	  case OBJTYPE_FOUNTAIN:
	  case OBJTYPE_FOOD:
		ch_printf( d->character, "Poisoned (0 = not poisoned) : " );
		break;

	  default:
		oedit_disp_menu(d);
	}
}


/*
 * object value 4
 */
void oedit_disp_val5_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	OLC_MODE(d) = OEDIT_VALUE_5;

	switch ( obj->type )
	{
	  case OBJTYPE_SALVE:
		oedit_disp_spells_menu(d);
		break;

	  case OBJTYPE_FOOD:
		send_to_char( d->character, "Food value: " );
		break;

	  case OBJTYPE_MISSILE_WEAPON:
		send_to_char( d->character, "Range: " );
		break;

	  default:
		oedit_disp_menu(d);
	}
}


/*
 * object value 5
 */
void oedit_disp_val6_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	OLC_MODE(d) = OEDIT_VALUE_6;

	switch ( obj->type )
	{
	  case OBJTYPE_SALVE:
		oedit_disp_spells_menu(d);
		break;

	  default:
		oedit_disp_menu(d);
	}
}


/*
 * object type
 */
void oedit_disp_type_menu( DESCRIPTOR_DATA *d )
{
	int		x;
	int		col = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0; x < MAX_OBJTYPE; x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s ", x+1, code_name(NULL, x, CODE_OBJTYPE) );

		if ( ++col%3 == 0 )
			send_to_char( d->character, "\r\n" );
		if ( col%3 == 0 && x == MAX_OBJTYPE-1 )
			send_to_char( d->character, "\r\n" );
	}
	send_to_char( d->character, "\r\nEnter object type: " );
}


/*
 * object extra flags
 */
void oedit_disp_extra_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	int		  x;
	int		  col = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_OBJFLAG;  x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s ", x+1, code_name(NULL, x, CODE_OBJFLAG) );

		if ( ++col%2 == 0 )
			send_to_char( d->character, "\r\n" );
		if ( col%2 == 0 && x == MAX_OBJFLAG-1 )
			send_to_char( d->character, "\r\n" );
	}
	ch_printf( d->character, "\r\nObject flags: &c%s&w\r\nEnter object extra flag (0 to quit): ",
		code_bit(NULL, obj->extra_flags, CODE_OBJFLAG) );
}


/*
 * Display wear flags menu
 */
void oedit_disp_wear_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;
	int		  x;
	int		  col = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_OBJWEAR;  x++ )
	{
		if ( x == OBJWEAR_DUAL )
			continue;

		ch_printf( d->character, "&g%2d&w) %-20.20s ", x+1, code_name(NULL, x, CODE_WEARFLAG) );
		if ( ++col%2 == 0 )
			send_to_char( d->character, "\r\n" );
		if ( col%2 == 0 && x == MAX_OBJWEAR-1 )
			send_to_char( d->character, "\r\n" );
	}

	ch_printf( d->character, "\r\nWear flags: &c%s&w\r\nEnter wear flag, 0 to quit:  ",
		code_bit(NULL, obj->wear_flags, CODE_WEARFLAG) );
}


/*
 * Display main menu
 */
void oedit_disp_menu( DESCRIPTOR_DATA *d )
{
	OBJ_DATA *obj = d->character->dest_buf;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	/* Build first half of menu */
	set_char_color( AT_PLAIN, d->character );
	ch_printf( d->character,
		"-- Item number    : [&c%d&w]\r\n"
		"&g1&w) Name       : &O%s\r\n"
		"&g2&w) S-Desc     : &O%s\r\n"
		"&g3&w) L-Desc     :-\r\n&O%s\r\n"
		"&g4&w) A-Desc     :-\r\n&O%s\r\n"
		"&g5&w) Type       : &c%s\r\n"
		"&g6&w) Extra flags: &c%s\r\n",
		obj->vnum,
		obj->name,
		obj->short_descr,
		obj->long_descr,
		obj->action_descr  ?  obj->action_descr  :  "<not set>\r\n",
		code_name(NULL,obj->type, CODE_OBJTYPE),
		code_bit(NULL, obj->extra_flags, CODE_OBJFLAG) );

	/* Build second half of the menu */
	ch_printf( d->character,
		"&g7&w) Wear flags  : &c%s\r\n"
		"&g8&w) Weight      : &c%d\r\n"
		"&g9&w) Cost        : &c%d\r\n"
		"&gB&w) Timer       : &c%d\r\n"
		"&gC&w) Level       : &c%d\r\n"		/* -- Object level. */
		"&gD&w) Layers      : &c%d\r\n"
		"&gE&w) Values      : &c(RR) in rifacimento, non attivi\r\n"
		"&gF&w) Affect menu\r\n"
		"&gG&w) Extra descriptions menu\r\n"
		"&gQ&w) Quit\r\n"
		"Enter choice : ",
		code_bit(NULL, obj->wear_flags, CODE_WEARFLAG),
		obj->weight,
		obj->cost,
		obj->timer,
		obj->level,
		print_bitvector(obj->layers) );

	OLC_MODE( d ) = OEDIT_MAIN_MENU;
}


/***************************************************************************
  Object affect editing/removing functions
 ***************************************************************************/

void edit_object_affect( DESCRIPTOR_DATA *d, int number )
{
	OBJ_DATA	*obj = d->character->dest_buf;
	int			 count = 0;
	AFFECT_DATA *paf;

	for ( paf = obj->first_affect;  paf;  paf = paf->next )
	{
		if ( count == number )
		{
			d->character->spare_ptr = paf;
			OLC_VAL(d) = TRUE;
			oedit_disp_affect_menu(d);
			return;
		}
		count++;
	}
	send_to_char( d->character, "Affect not found.\r\n" );
}


void remove_affect_from_obj( OBJ_DATA *obj, int number )
{
	int			 count = 0;
	AFFECT_DATA *paf;

	if ( obj->first_affect )
	{
		for ( paf = obj->first_affect;  paf;  paf = paf->next )
		{
			if ( count == number )
			{
				UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
				DISPOSE( paf );
				--top_affect;
				return;
			}
			count++;
		}
	}
}


EXTRA_DESCR_DATA *oedit_find_extradesc( OBJ_DATA *obj, int number )
{
	EXTRA_DESCR_DATA *ed;
	int				  count = 0;

	for ( ed = obj->first_extradescr; ed; ed = ed->next )
	{
		if ( ++count == number )
			return ed;
	}

	return NULL;
}


MPROG_DATA *oedit_find_prog( OBJ_DATA *obj, int number )
{
	MPROG_DATA *mprg;
	int			count;

	for ( count = 0, mprg = obj->first_mudprog; mprg; mprg = mprg->next )
	{
		if ( ++count == number )
			return mprg;
	}

	return NULL;
}


/*
 * Bogus command for resetting stuff
 * Non bisogna inserirlo nel file di commands.dat
 */
DO_RET do_oedit_reset( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA		 *obj  = ch->dest_buf;
	EXTRA_DESCR_DATA *ed   = ch->spare_ptr;
	MPROG_DATA		 *mprg = ch->spare_ptr;
	int				  mode = OLC_MODE( ch->desc );

	switch ( ch->substate )
	{
	  default:
		return;

	  case SUB_OBJ_EXTRA:
		if ( !ch->dest_buf )
		{
			send_to_char( ch, "Fatal error, report to Coder.\r\n" );
			send_log( NULL, LOG_OLC, "do_oedit_reset: sub_obj_extra: NULL ch->dest_buf" );
			ch->substate = SUB_NONE;
			return;
		}
/*		OLC_DESC(ch->desc) = ch->spare_ptr; */
		DISPOSE( ed->description );
		ed->description = copy_buffer( ch );
		stop_editing( ch );
		ch->dest_buf = obj;
		ch->spare_ptr = ed;
		ch->substate = SUB_NONE;
		ch->desc->connected = CON_OEDIT;
		OLC_MODE(ch->desc) = OEDIT_EXTRADESC_CHOICE;
		oedit_disp_extra_choice( ch->desc );
		return;

	  case SUB_OBJ_LONG:
		if ( !ch->dest_buf )
		{
			send_to_char( ch, "Fatal error, report to Coder.\r\n" );
			send_log( NULL, LOG_OLC, "do_oedit_reset: sub_obj_long: NULL ch->dest_buf" );
			ch->substate = SUB_NONE;
			return;
		}

		DISPOSE( obj->long_descr );
		obj->long_descr = copy_buffer( ch );

		stop_editing( ch );
		ch->dest_buf = obj;
		ch->desc->connected = CON_OEDIT;
		ch->substate = SUB_NONE;
		OLC_MODE( ch->desc ) = OEDIT_MAIN_MENU;
		oedit_disp_menu( ch->desc );
		return;

	  case SUB_MPROG_EDIT:
		if ( mprg->first_mpcom )
		{
			MPROG_COM_LIST	*mpcom;

			for ( mpcom = mprg->first_mpcom;  mpcom;  mpcom = mpcom->next )
			{
				DISPOSE( mpcom->com );
				DISPOSE( mpcom );
			}
		}
		mprg->first_mpcom->com = copy_buffer( ch );	/* (BB) (FF) (RR) farlo bene, devo spezzettare ogni riga un comando */
#ifdef T2_ALFA
		/* Used to get sequential lines of a multi line string (separated by "\r\n")
		 * Thus its like one_argument(), but a trifle different. It is destructive
		 *  to the multi line string argument, and thus clist must not be shared.
		 */
		char *mprog_next_command( char *clist )
		{
			char *pointer = clist;

			while ( *pointer != '\r' && *pointer != '\0' )
				pointer++;

			if ( *pointer == '\r' )		*pointer++ = '\0';
			if ( *pointer == '\n' )		*pointer++ = '\0';

			return ( pointer );
		}
#endif
		stop_editing( ch );
		ch->dest_buf = obj;
		ch->desc->connected = ch->tempnum;
		ch->substate = SUB_NONE;
		OLC_MODE( ch->desc ) = mode;
		oedit_disp_prog_choice( ch->desc );
		return;
	}
}


/*
 * This function interprets the arguments that the character passed
 *  to it based on which OLC mode you are in at the time.
 */
void oedit_parse( DESCRIPTOR_DATA *d, char *arg )
{
	OBJ_DATA		 *obj = d->character->dest_buf;
	AFFECT_DATA		 *paf = d->character->spare_ptr;
	AFFECT_DATA		 *npaf;
	EXTRA_DESCR_DATA *ed = d->character->spare_ptr;
	char			  arg1[MIL];
	int				  number = 0, max_val, min_val, value;
/*	bool			  found; */

	switch ( OLC_MODE(d) )
	{
	  case OEDIT_CONFIRM_SAVESTRING:
		switch ( *arg )
		{
		  case 'y':
		  case 'Y':
			send_to_char( d->character, "Saving object to memory.\r\n" );
/*			oedit_save_internally(d); */

		  case 'n':
		  case 'N':
			cleanup_olc( d );
			return;

		  default:
			send_to_char( d->character, "Invalid choice!\r\n" );
			send_to_char( d->character, "Do you wish to save this object internally?\r\n" );

			return;
		}

	  case OEDIT_MAIN_MENU:
		/* switch to whichever mode the user selected, display prompt or menu */
		switch ( toupper(arg[0]) )
		{
		  case 'Q':
/*			send_to_char( d->character, "Do you wish to save this object internally?: " );
			 OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING;
*/
			cleanup_olc( d );
			return;

		  case '1':
			send_to_char( d->character, "Enter namelist : " );
			OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
			break;

		  case '2':
			send_to_char( d->character, "Enter short desc : " );
			OLC_MODE(d) = OEDIT_SHORTDESC;
			break;

		  case '3':
			send_to_char( d->character, "Enter long desc :-\r\n| " );
			OLC_MODE(d) = OEDIT_LONGDESC;
			break;

		  case '4':
			/* lets not */
			send_to_char( d->character, "Enter action desc :-\r\n" );
			OLC_MODE(d) = OEDIT_ACTDESC;
			break;

		  case '5':
			oedit_disp_type_menu(d);
			OLC_MODE(d) = OEDIT_TYPE;
			break;

		  case '6':
			oedit_disp_extra_menu(d);
			OLC_MODE(d) = OEDIT_EXTRAS;
			break;

		  case '7':
			oedit_disp_wear_menu(d);
			OLC_MODE(d) = OEDIT_WEAR;
			break;

		  case '8':
			send_to_char( d->character, "Enter weight : " );
			OLC_MODE(d) = OEDIT_WEIGHT;
			break;

		  case '9':
			send_to_char( d->character, "Enter cost : " );
			OLC_MODE(d) = OEDIT_COST;
			break;

		  case 'B':
			send_to_char( d->character, "Enter timer : " );
			OLC_MODE(d) = OEDIT_TIMER;
			break;

		  case 'C':
			send_to_char( d->character, "Enter level : " );
			OLC_MODE(d) = OEDIT_LEVEL;
			break;

		  case 'D':
			if ( HAS_BIT( obj->wear_flags, OBJWEAR_BODY  )
			  || HAS_BIT( obj->wear_flags, OBJWEAR_ABOUT )
			  || HAS_BIT( obj->wear_flags, OBJWEAR_ARMS  )
			  || HAS_BIT( obj->wear_flags, OBJWEAR_FEET  )
			  || HAS_BIT( obj->wear_flags, OBJWEAR_HANDS )
			  || HAS_BIT( obj->wear_flags, OBJWEAR_LEGS  )
			  || HAS_BIT( obj->wear_flags, OBJWEAR_WAIST ) )
			{
				oedit_disp_layer_menu(d);
				OLC_MODE(d) = OEDIT_LAYERS;
			}
			else
			{
				send_to_char( d->character, "The wear location of this object is not layerable.\r\n" );
			}
			break;

		  case 'E':
			oedit_disp_val1_menu(d);
			break;

		  case 'F':
			oedit_disp_prompt_apply_menu(d);
			break;

		  case 'G':
			oedit_disp_extradesc_menu(d);
			break;

		  case 'H':
			oedit_disp_progs(d);
			break;

		  default:
			oedit_disp_menu(d);
			break;
		}
		return;			/* end of OEDIT_MAIN_MENU */

	  case OEDIT_EDIT_NAMELIST:
		DISPOSE( obj->name );
		obj->name = str_dup( arg );
		olc_log( d, "Changed name to %s", obj->name );
		break;

	  case OEDIT_SHORTDESC:
		DISPOSE( obj->short_descr );
		obj->short_descr = str_dup( arg );
		olc_log( d, "Changed short to %s", obj->short_descr );
		break;

	  case OEDIT_LONGDESC:
		DISPOSE( obj->long_descr );
		obj->long_descr = str_dup( arg );
		olc_log( d, "Changed longdesc to %s", obj->long_descr );
		break;

	  case OEDIT_ACTDESC:
		DISPOSE( obj->action_descr );
		obj->action_descr = str_dup( arg );
		olc_log( d, "Changed actiondesc to %s", obj->action_descr );
		break;

	  case OEDIT_TYPE:
		if ( is_number(arg) )
			number = atoi( arg );
		else
			number = code_num( NULL, arg, CODE_OBJTYPE );

		if ( number < 1 || number >= MAX_OBJTYPE )
		{
			send_to_char( d->character, "Invalid choice, try again : " );
			return;
		}

		obj->type = (int) number;
		olc_log( d, "Changed object type to %s", code_name(NULL, number, CODE_OBJTYPE) );
		break;

	  case OEDIT_EXTRAS:
		while ( VALID_STR(arg) )
		{
			arg = one_argument( arg, arg1 );
			if ( is_number(arg1) )
			{
				number = atoi( arg1 );

				if ( number == 0 )
				{
					oedit_disp_menu(d);
					return;
				}

				number -= 1; /* Offset for 0 */
				if ( number < 0 || number >= MAX_OBJFLAG )
				{
					oedit_disp_extra_menu( d );
					return;
				}
			}
			else
			{
				number = code_num( NULL, arg1, CODE_OBJFLAG );
				if ( number < 0 || number >= MAX_OBJFLAG )
				{
					oedit_disp_extra_menu(d);
					return;
				}
			}

			TOGGLE_BIT( obj->extra_flags, number );
			olc_log( d, "%s the flag %s",
				HAS_BIT(obj->extra_flags, number)  ?  "Added"  :  "Removed",
				code_name(NULL, number, CODE_OBJFLAG) );

			/* If you used a number, you can only do one flag at a time */
			if ( is_number(arg) )
				break;
		} /* chiude il while */

		oedit_disp_extra_menu( d );
		return;

	  case OEDIT_WEAR:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
				break;
			else
			if ( number < 0 || number >= MAX_OBJWEAR )
			{
				send_to_char( d->character, "Invalid flag, try again: " );
				return;
			}
			else
			{
				number -= 1; /* Offset to accomodate 0 */
				TOGGLE_BIT( obj->wear_flags, number );
				olc_log( d, "%s the wearloc %s",
					HAS_BIT(obj->wear_flags, number)  ?  "Added"  :  "Removed",
					code_name(NULL, number, CODE_WEARFLAG) );
			}
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_WEARFLAG );
				if ( number != -1 )
				{
					TOGGLE_BIT( obj->wear_flags, number );
					olc_log( d, "%s the wearloc %s",
						HAS_BIT(obj->wear_flags, number) ? "Added" : "Removed",
						code_name(NULL, number, CODE_WEARFLAG) );
				}
			}
		}

		oedit_disp_wear_menu(d);
		return;

	  case OEDIT_WEIGHT:
		number = atoi(arg);
		obj->weight = number;
		olc_log( d, "Changed weight to %d", obj->weight );
		break;

	  case OEDIT_COST:
		number = atoi(arg);
		obj->cost = number;
		olc_log( d, "Changed cost to %d", obj->cost );
		break;

	  case OEDIT_TIMER:
		number = atoi(arg);
		obj->timer = number;
		olc_log( d, "Changed timer to %d", obj->timer );
		break;

	  case OEDIT_LEVEL:
		number = atoi(arg);
		obj->level = URANGE( 0, number, LVL_LEGEND );
		olc_log( d, "Changed object level to %d", obj->level );
		break;

	  case OEDIT_LAYERS:
		number = atoi( arg );
		switch ( number )
		{
		  case 0:	oedit_disp_menu( d );				return;
		  case 1:	CLEAR_BITS( obj->layers );			break;
		  case 2:	TOGGLE_BIT( obj->layers,   1 );		break;
		  case 3:	TOGGLE_BIT( obj->layers,   2 );		break;
		  case 4:	TOGGLE_BIT( obj->layers,   4 );		break;
		  case 5:	TOGGLE_BIT( obj->layers,   8 );		break;
		  case 6:	TOGGLE_BIT( obj->layers,  16 );		break;
		  case 7:	TOGGLE_BIT( obj->layers,  32 );		break;
		  case 8:	TOGGLE_BIT( obj->layers,  64 );		break;
		  case 9:	TOGGLE_BIT( obj->layers, 128 );		break;
		  default:
			send_to_char( d->character, "Invalid selection, try again: " );
			return;
		}
		olc_log( d, "Changed layers to %d", print_bitvector(obj->layers) );
		oedit_disp_layer_menu( d );
		return;

	  case OEDIT_VALUE_1:
		number = atoi(arg);
		switch ( obj->type )
		{
		  default:
			/* (FF) (RR) rifare tutto il sistema di editing dei value */
/*			obj->value[0] = number;*/
			oedit_disp_val2_menu( d );

		  case OBJTYPE_LEVER:
		  case OBJTYPE_SWITCH:
			if ( number < 0 || number > 29 )
			{
				oedit_disp_lever_flags_menu(d);
			}
			else
			{
				if ( number != 0 )
				{
					TOGGLE_BITINT( obj->lever->flags, (number - 1) );
					oedit_disp_val1_menu( d );
				}
				else
				{
					oedit_disp_val2_menu( d );
				}
			}
			break;
		}
/*		olc_log( d, "Changed v0 to %d", obj->value[0] );*/
		return;

	  case OEDIT_VALUE_2:
		number = atoi(arg);
		switch (obj->type)
		{
		  default:
			/* (FF) (RR) anche qui da rifare il sistema dei value */
/*			obj->value[1] = number; */
			oedit_disp_val3_menu( d );
			break;

		  case OBJTYPE_PILL:
		  case OBJTYPE_SCROLL:
		  case OBJTYPE_POTION:
			if ( !is_number(arg) )
				number = skill_lookup( arg );
			obj->scroll->sn1 = number;
			oedit_disp_val3_menu( d );
			break;

		  case OBJTYPE_LEVER:
		  case OBJTYPE_SWITCH:
			if ( HAS_BITINT(obj->lever->flags, TRIG_CAST) )
				number = skill_lookup( arg );
			obj->lever->vnum_sn = number;
			oedit_disp_val3_menu( d );
			break;

		  case OBJTYPE_CONTAINER:
			number = atoi( arg );
			if ( number < 0 || number >= MAX_CONTAINER )
				oedit_disp_container_flags_menu( d );
			else
			{
				/* if 0, quit */
				if ( number != 0 )
				{
					number = (number - 1);
					TOGGLE_BITINT( obj->container->flags, number );
					oedit_disp_val2_menu(d);
				}
				else
				{
					oedit_disp_val3_menu(d);
				}
			}
			break;
		}
/*		olc_log( d, "Changed v1 to %d", obj->value[1] );*/
		return;

	  case OEDIT_VALUE_3:
		number = atoi(arg);
		/* Some error checking done here */
		switch ( obj->type )
		{
		  default:
			/* Would require modifying if you have bvnum */
			min_val = -32000;
			max_val = MAX_VNUM;
/*			obj->value[2] = URANGE( min_val, number, max_val );*/
			break;

		  case OBJTYPE_SCROLL:
		  case OBJTYPE_POTION:
		  case OBJTYPE_PILL:
			min_val = -1;
			max_val = top_sn-1;
			if ( !is_number(arg) )
				number = skill_lookup( arg );
			obj->scroll->sn2 = URANGE( min_val, number, max_val );
			break;

		  case OBJTYPE_WEAPON:
			min_val = 0;
			max_val = 100;
			obj->weapon->dice_size = URANGE( min_val, number, max_val );
			break;

		  case OBJTYPE_DRINK_CON:
		  case OBJTYPE_FOUNTAIN:
			min_val = 0;
			max_val = MAX_LIQ - 1;
			obj->drinkcon->liquid = URANGE( min_val, number, max_val );
			break;
		}
/*		olc_log( d, "Changed v2 to %d", obj->value[2] ); */
		oedit_disp_val4_menu(d);
		return;

	  case OEDIT_VALUE_4:
		number = atoi( arg );
		switch ( obj->type )
		{
		  default:
			min_val = -32000;
			max_val = 32000;
/*			obj->value[3] = URANGE( min_val, number, max_val ); */
			break;

		  case OBJTYPE_PILL:
		  case OBJTYPE_SCROLL:
		  case OBJTYPE_POTION:
			min_val = -1;
			max_val = top_sn - 1;
			if ( !is_number(arg) )
				number = skill_lookup( arg );
			obj->scroll->sn3 = URANGE( min_val, number, max_val );

		  case OBJTYPE_WAND:
		  case OBJTYPE_STAFF:
			min_val = -1;
			max_val = top_sn - 1;
			if ( !is_number(arg) )
				number = skill_lookup( arg );
			obj->wand->sn = URANGE( min_val, number, max_val );
			break;

		  case OBJTYPE_WEAPON:
			min_val = 0;
			max_val = MAX_ATTACK -1;
			if ( number < min_val || number > max_val )
			{
				oedit_disp_val4_menu( d );
				return;
			}
			obj->weapon->damage1 = URANGE( min_val, number, max_val );
			break;
		}
/*		olc_log( d, "Changed v3 to %d", obj->value[3] );*/
		oedit_disp_val5_menu( d );
		break;

	  case OEDIT_VALUE_5:
		number = atoi( arg );
		switch ( obj->type )
		{
		  default:
			min_val = -32000;
			max_val = 32000;
/*			obj->value[4] = URANGE( min_val, number, max_val );*/
			break;

		  case OBJTYPE_SALVE:
			if ( !is_number(arg) )
				number = skill_lookup( arg );
			min_val = -1;
			max_val = top_sn-1;
			obj->salve->sn1 = URANGE( min_val, number, max_val );
			break;

		  case OBJTYPE_FOOD:
			min_val = 0;
			max_val = 32000;
			obj->food->init_cond = URANGE( min_val, number, max_val );
			break;
		}
/*		olc_log( d, "Changed v4 to %d", obj->value[4] );*/
		oedit_disp_val6_menu(d);
		break;

	  case OEDIT_VALUE_6:
		number = atoi( arg );
		switch ( obj->type )
		{
		  default:
			min_val = -32000;
			max_val = 32000;
/*			obj->value[5] = URANGE( min_val, number, max_val );*/
			break;

		  case OBJTYPE_SALVE:
			if ( !is_number(arg) )
				number = skill_lookup( arg );
			min_val = -1;
			max_val = top_sn-1;
			obj->salve->sn2 = URANGE( min_val, number, max_val );
			break;
		}
/*		olc_log( d, "Changed v5 to %d", obj->value[5] );*/
		break;

	  case OEDIT_AFFECT_MENU:
		number = atoi( arg );

		switch ( arg[0] )
		{
		  default: /* if its a number, then its prolly for editing an affect */
			if ( is_number( arg ) )
				edit_object_affect( d, number );
			else
				oedit_disp_prompt_apply_menu( d );
			return;

		  case 'r':
		  case 'R':
			/* Chop off the 'R', if theres a number following use it, otherwise prompt for input */
			arg = one_argument( arg, arg1 );
			if ( VALID_STR(arg) )
			{
				number = atoi( arg );
				remove_affect_from_obj( obj, number );
				oedit_disp_prompt_apply_menu(d);
			}
			else
			{
				send_to_char( d->character, "Remove which affect? " );
				OLC_MODE(d) = OEDIT_AFFECT_REMOVE;
			}
			return;

		  case 'a':
		  case 'A':
			CREATE( paf, AFFECT_DATA, 1 );
			d->character->spare_ptr = paf;
			oedit_disp_affect_menu(d);
			return;

		  case 'q':
		  case 'Q':
			d->character->spare_ptr = NULL;
			break;
		}
		break;	/* If we reach here, we're done */

	  case OEDIT_AFFECT_LOCATION:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
			{
				/* Junk the affect */
				DISPOSE( paf );
				d->character->spare_ptr = NULL;
				oedit_disp_prompt_apply_menu( d );
				return;
			}
		}
		else
		{
			number = code_num( NULL, arg, CODE_APPLY );
		}

		if ( number < 0 || number >= MAX_APPLY || number == APPLY_EXT_AFFECT )
		{
			send_to_char( d->character, "Invalid location, try again: " );
			return;
		}
		paf->location = number;
		OLC_MODE(d) = OEDIT_AFFECT_MODIFIER;
		/* Insert all special affect handling here ie: non numerical stuff */
		/* And add the appropriate case statement below */
		if ( number == APPLY_AFFECT )
		{
			d->character->tempnum = 0;
			medit_disp_aff_flags( d );
		}
		else if ( number == APPLY_RESISTANT || number == APPLY_IMMUNE || number == APPLY_SUSCEPTIBLE )
		{
			d->character->tempnum = 0;
			medit_disp_ris( d );
		}
		else if ( number == APPLY_WEAPONSPELL || number == APPLY_WEARSPELL || number == APPLY_REMOVESPELL )
			oedit_disp_spells_menu( d );
		else
			send_to_char( d->character, "\r\nModifier: ");
		return;

	  case OEDIT_AFFECT_MODIFIER:
		switch ( paf->location )
		{
		  case APPLY_AFFECT:
		  case APPLY_RESISTANT:
		  case APPLY_IMMUNE:
		  case APPLY_SUSCEPTIBLE:
			if ( is_number(arg) )
			{
				BITVECTOR *temp;

				number = atoi( arg );
				if ( number == 0 )
				{
					value = d->character->tempnum;
					break;
				}
				temp = convert_bitvector( d->character->tempnum );
				TOGGLE_BIT( temp, (number-1) );
				destroybv( temp );
			}
			else
			{
				while ( VALID_STR(arg) )
				{
					arg = one_argument( arg, arg1 );
					if ( paf->location == APPLY_AFFECT )
						number = code_num( NULL, arg1, CODE_AFFECT );
					else
						number = code_num( NULL, arg1, CODE_RIS );

					if ( number < 0 )
						ch_printf( d->character, "Invalid flag: %s\r\n", arg1 );
					else
					{
						BITVECTOR *temp;

						temp = convert_bitvector( d->character->tempnum );
						TOGGLE_BIT( temp, number );
						destroybv( temp );
					}
				}
			}
			if ( paf->location == APPLY_AFFECT )
				medit_disp_aff_flags(d);
			else
				medit_disp_ris(d);
			return;

		  case APPLY_WEAPONSPELL:
		  case APPLY_WEARSPELL:
		  case APPLY_REMOVESPELL:
			if ( is_number( arg ) )
			{
				number = atoi( arg );
				if ( is_valid_sn( number ) )
					value = number;
				else
				{
					send_to_char( d->character, "Invalid sn, try again: " );
					return;
				}
			}
			else
			{
				value = bsearch_skill_exact( arg, gsn_first_spell, gsn_first_skill - 1 );
				if ( value < 0 )
				{
					ch_printf( d->character, "Invalid spell %s, try again: ", arg );
					return;
				}
			}
			break;

		  default:
			value = atoi( arg );
			break;
		}

		/* Link it in */
		if ( !value || OLC_VAL(d) == TRUE )
		{
			paf->modifier = value;
			olc_log( d, "Modified affect to: %s by %d", code_name(NULL, paf->location, CODE_APPLY), value );
			OLC_VAL( d ) = FALSE;
			oedit_disp_prompt_apply_menu( d );
			return;
		}

		CREATE( npaf, AFFECT_DATA, 1 );
		npaf->type = -1;
		npaf->duration = -1;
		npaf->location = URANGE( 0, paf->location, MAX_APPLY );
		npaf->modifier = value;

		LINK( npaf, obj->first_affect, obj->last_affect, next, prev );
		++top_affect;

		olc_log( d, "Added new affect: %s by %d", code_name(NULL, npaf->location, CODE_APPLY), npaf->modifier );

		DISPOSE( paf );
		d->character->spare_ptr = NULL;
		oedit_disp_prompt_apply_menu(d);
		return;

	  case OEDIT_AFFECT_RIS:
		/* Unnecessary atm */
		number = atoi( arg );
		if ( number < 0 || number >= MAX_RIS )
		{
			send_to_char( d->character, "Unknown flag, try again: " );
			return;
		}
		return;

	  case OEDIT_AFFECT_REMOVE:
		number = atoi( arg );
		remove_affect_from_obj( obj, number );
		olc_log( d, "Removed affect #%d", number );
		oedit_disp_prompt_apply_menu(d);
		return;

	  case OEDIT_EXTRADESC_KEY:
		olc_log( d, "Changed exdesc %s to %s", ed->keyword, arg );
		DISPOSE( ed->keyword );
		ed->keyword = str_dup( arg );
		oedit_disp_extra_choice( d );
		return;

	  case OEDIT_EXTRADESC_DESCRIPTION:
		/* Should never reach this */
		break;

	  case OEDIT_EXTRADESC_CHOICE:
		number = atoi( arg );
		switch ( number )
		{
		  case 0:
			OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
			oedit_disp_extradesc_menu(d);
			return;
		  case 1:
			OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
			send_to_char( d->character, "Enter keywords, speperated by spaces: " );
			return;
		  case 2:
			OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
			d->character->substate = SUB_OBJ_EXTRA;
			d->character->last_cmd = do_oedit_reset;

			send_to_char( d->character, "Enter new extra description - :\r\n" );
			if ( !ed->description )
				ed->description = str_dup( "" );
			start_editing( d->character, ed->description );
			return;
		}
		break;

	  case OEDIT_EXTRADESC_DELETE:
		ed = oedit_find_extradesc( obj, atoi(arg) );
		if ( !ed )
		{
			send_to_char( d->character, "Extra description not found, try again: " );
			return;
		}
		olc_log( d, "Deleted exdesc %s", ed->keyword );

		UNLINK( ed, obj->first_extradescr, obj->last_extradescr, next, prev );

		DISPOSE( ed->keyword );
		DISPOSE( ed->description );
		DISPOSE( ed );
		top_ed--;
		oedit_disp_extradesc_menu(d);
		return;

	  case OEDIT_EXTRADESC_MENU:
		switch ( toupper( arg[0] ) )
		{
		  case 'Q':
			break;

		  case 'A':
			CREATE( ed, EXTRA_DESCR_DATA, 1 );

			LINK( ed, obj->first_extradescr, obj->last_extradescr, next, prev );

			ed->keyword		= str_dup( "" );
			ed->description	= str_dup( "" );
			top_ed++;
			d->character->spare_ptr = ed;
			olc_log( d, "Added new exdesc" );

			oedit_disp_extra_choice(d);
			return;

		  case 'R':
			OLC_MODE(d) = OEDIT_EXTRADESC_DELETE;
			send_to_char( d->character, "Delete which extra description?" );
			return;

		  default:
			if ( is_number(arg) )
			{
				ed = oedit_find_extradesc( obj, atoi(arg) );
				if ( !ed )
				{
					send_to_char( d->character, "Not found, try again: " );
					return;
				}
				d->character->spare_ptr = ed;
				oedit_disp_extra_choice(d);
			}
			else
				oedit_disp_extradesc_menu(d);
			return;
		}
		break;

	  default:
		send_log( NULL, LOG_OLC, "oedit_parse: Reached default case!" );
		break;
	}

	/*. If we get here, we have changed something .*/
	OLC_CHANGE( d ) = TRUE;	/*. Has changed flag .*/
	oedit_disp_menu(d);
}
