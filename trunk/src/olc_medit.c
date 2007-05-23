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


/**************************************************************************\
 *     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)         *
 *                                                                        *
 *   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin      *
 **************************************************************************
 *               Mobile/Player editing module (medit.c)                   *
\**************************************************************************/

#include <ctype.h>

#include "mud.h"
#include "affect.h"
#include "build.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "interpret.h"
#include "mprog.h"
#include "nanny.h"
#include "olc.h"
#include "save.h"


/*
 * Variabili globali.
 */
char		*olc_clan_list [MAX_CLAN];
int			 olc_top_clan;


/*-------------------------------------------------------------------*\
  initialization functions
\*-------------------------------------------------------------------*/


/*
 * Quite the hack here :P
 */
void medit_setup_arrays( void )
{
	CLAN_DATA	 *clan;
	int			  count;

	count = 0;
	for ( clan = first_clan;  clan;  clan = clan->next )
	{
		olc_clan_list[count] = clan->name;
		count++;
	}

	olc_top_clan = count-1;
}


/*
 * Display main menu for NPCs
 */
void medit_disp_npc_menu( DESCRIPTOR_DATA *d )
{
	CHAR_DATA	*ch;
	CHAR_DATA	*mob;
	int			 hitestimate;
	int			 damestimate;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "medit_disp_npc_menu: d è NULL" );
		return;
	}

	if ( !d->character )
	{
		send_log( NULL, LOG_BUG, "medit_disp_npc_menu: d->character è NULL, numero descrittore %d", d->descriptor );
		return;
	}

	ch  = d->character;
	mob = d->character->dest_buf;

	if ( !mob )
	{
		send_log( NULL, LOG_BUG, "medit_disp_npc_menu: mob è NULL" );
		return;
	}

	if ( !mob->pIndexData )
	{
		send_log( NULL, LOG_BUG, "medit_disp_npc_menu: struttura prototipo del mob %s NULL", mob->name );
		return;
	}

	/* (RR) (FF) (CC) (TT) è da ritarare secondo il nuovo sistema di livello */
	if ( !mob->pIndexData->hitnodice )
		hitestimate = get_level(mob)*4 + number_range(get_level(mob)/2 * get_level(mob)/8, get_level(mob)/2 * get_level(mob)/2 );
	else
		hitestimate = mob->pIndexData->hitnodice * number_range(1, mob->pIndexData->hitsizedice ) + mob->pIndexData->hitplus;

	damestimate = number_range( mob->pIndexData->damnodice,
		mob->pIndexData->damsizedice * mob->pIndexData->damnodice );

	damestimate += get_damroll(mob);

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	set_char_color( AT_PLAIN, d->character );
	ch_printf( ch, "-- Mob Number:  [&c%d&w]\r\n",										mob->pIndexData->vnum );
	ch_printf( ch, "&g1&w) Sex: &O%-7.7s          &g2&w) Name: &O%s\r\n",				(mob->sex == SEX_FEMALE)  ?  "femmina"  :  "maschio", mob->name );
	ch_printf( ch, "&g3&w) Shortdesc: &O%s\r\n",										(!VALID_STR(mob->short_descr))  ?  "(none set)"  :  mob->short_descr );
	ch_printf( ch, "&g4&w) Longdesc:-\r\n&O%s\r\n",										(!VALID_STR(mob->long_descr))   ?  "(none set)"  :  mob->long_descr );
	ch_printf( ch, "&g5&w) Description:-\r\n&O%-74.74s\r\n",							mob->description );
	ch_printf( ch, "&g6&w) Class: [&c%-11.11s&w], &g7&w) Race:   [&c%-11.11s&w]\r\n",	get_class(mob, FALSE), get_race(mob, FALSE) );
	ch_printf( ch, "&g8&w) Level:       [&c%5d&w], &g9&w) Alignment:    [&c%5d&w]\r\n",	mob->level, mob->alignment );

	ch_printf( ch, "&gA0&w) Forza:  [&c%5d&w]   &gA1&w) Resistenza:  [&c%5d&w]   &gA2&w) Costituzione:  [&c%5d&w]\r\n",
		get_curr_attr(mob, ATTR_STR), get_curr_attr(mob, ATTR_RES), get_curr_attr(mob, ATTR_CON) );
	ch_printf( ch, "&gA3&w) Intelligenza:  [&c%5d&w]   &gA4&w) Concentrazione:  [&c%5d&w]   &gA5&w) Volontà:  [&c%5d&w]\r\n",
		get_curr_attr(mob, ATTR_INT), get_curr_attr(mob, ATTR_COC), get_curr_attr(mob, ATTR_WIL) );
	ch_printf( ch, "&gA6&w) Agilità:  [&c%5d&w]   &gA7&w) Riflessi:  [&c%5d&w]   &gA8&w) Velocità:  [&c%5d&w]\r\n",
		get_curr_attr(mob, ATTR_AGI), get_curr_attr(mob, ATTR_REF), get_curr_attr(mob, ATTR_SPE) );
	ch_printf( ch, "&gA9&w) Spiritualità:  [&c%5d&w]   &gAA&w) Migrazione:  [&c%5d&w]   &gAB&w) Assorbimento:  [&c%5d&w]\r\n",
		get_curr_attr(mob, ATTR_SPI), get_curr_attr(mob, ATTR_MIG), get_curr_attr(mob, ATTR_ABS) );
	ch_printf( ch, "&gAC&w) Empatia:  [&c%5d&w]   &gAD&w) Bellezza:  [&c%5d&w]   &gAE&w) Comando:  [&c%5d&w]\r\n",
		get_curr_attr(mob, ATTR_EMP), get_curr_attr(mob, ATTR_BEA), get_curr_attr(mob, ATTR_LEA) );

	ch_printf( ch, "&gB0&w) Vista:  [&c%5d&w]   &gB1&w) Udito:  [&c%5d&w]   &gB2&w) Olfatto:  [&c%5d&w]\r\n",
		get_curr_sense(mob, SENSE_SIGHT), get_curr_sense(mob, SENSE_HEARING), get_curr_sense(mob, SENSE_SMELL) );
	ch_printf( ch, "&gB3&w) Gusto:  [&c%5d&w]   &gB4&w) Tatto:  [&c%5d&w]   &gB5&w) Sesto Senso:  [&c%5d&w]\r\n",
		get_curr_sense(mob, SENSE_TASTE), get_curr_sense(mob, SENSE_TOUCH), get_curr_sense(mob, SENSE_SIXTH) );

	ch_printf( ch, "&gC&w) DamNumDice:  [&c%5d&w], &gD&w) DamSizeDice:  [&c%5d&w], &gF&w) DamPlus:  [&c%5d&w]=[&c%5d&w]\r\n",
		mob->pIndexData->damnodice, mob->pIndexData->damsizedice, mob->pIndexData->damplus, damestimate );
	ch_printf( ch, "&gG&w) HitNumDice:  [&c%5d&w], &gH&w) HitSizeDice:  [&c%5d&w], &gI&w) HitPlus:  [&c%5d&w]=[&c%5d&w]\r\n",
		mob->pIndexData->hitnodice, mob->pIndexData->hitsizedice, mob->pIndexData->hitplus, hitestimate );
	ch_printf( ch, "&gJ&w) Gold:	 [&c%8d&w]", mob->gold );	/* Tolta la modifica per la spec_fun (FF) sarebbe da scalare il menù visto che la K non esiste più */

	ch_printf( ch, "&gL&w) Saving Throws\r\n" );
	ch_printf( ch, "&gM&w) Resistant  : &O%s\r\n",	code_bit(NULL, mob->resistant,	 CODE_RIS) );
	ch_printf( ch, "&gN&w) Immune     : &O%s\r\n",	code_bit(NULL, mob->immune,		 CODE_RIS) );
	ch_printf( ch, "&gO&w) Susceptible: &O%s\r\n",	code_bit(NULL, mob->susceptible,  CODE_RIS) );
	ch_printf( ch, "&gP&w) Position   : &O%s\r\n",	code_name(NULL, mob->position,	 CODE_POSITION) );
	ch_printf( ch, "&gQ&w) Attacks    : &c%s\r\n",	code_bit(NULL, mob->attacks,	 CODE_ATTACK) );
	ch_printf( ch, "&gR&w) Defenses   : &c%s\r\n",	code_bit(NULL, mob->defenses,	 CODE_DEFENSE) );
	ch_printf( ch, "&gS&w) Body Parts : &c%s\r\n",	code_bit(NULL, mob->xflags,		 CODE_PART) );
	ch_printf( ch, "&gT&w) Act Flags  : &c%s\r\n",	code_bit(NULL, mob->act,		 CODE_MOB) );
	ch_printf( ch, "&gU&w) Affected   : &c%s\r\n",	code_bit(NULL, mob->affected_by, CODE_AFFECT) );

	send_to_char( ch, "&gE&w) Esci\r\n" );
	send_to_char( ch, "Enter choice : " );

	OLC_MODE( d ) = MEDIT_NPC_MAIN_MENU;
}


void medit_disp_pc_menu( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch = d->character;
	CHAR_DATA *victim = d->character->dest_buf;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	ch_printf( ch, "&g1&w) Sex: &O%-7.7s		   &g2&w) Name: &O%s\r\n",					(victim->sex == SEX_FEMALE)  ?  "femmina"  :  "maschio", victim->name );
	ch_printf( ch, "&g3&w) Description:-\r\n&O%-74.74s\r\n", victim->description );
	ch_printf( ch, "&g4&w) Razza: [&c%-11.11s&w],  &g5&w) class:   [&c%-11.11s&w]\r\n",
		get_race(victim, FALSE), get_class(victim, FALSE) );
	ch_printf( ch, "&g6&w) Level:	   [&c%5d&w],  &g7&w) Alignment:	[&c%5d&w]\r\n",		victim->level, victim->alignment );
	ch_printf( ch, "&g8&w) Mentalstate:  [&c%5d&w],  &g9&w) Emotional: [&c%5d&w]\r\n",		victim->mental_state, victim->emotional_state );

	ch_printf( ch, "&gA0&w) Forza:  [&c%5d&w]   &gA1&w) Resistenza:  [&c%5d&w]   &gA2&w) Costituzione:  [&c%5d&w]\r\n",
		get_curr_attr(victim, ATTR_STR), get_curr_attr(victim, ATTR_RES), get_curr_attr(victim, ATTR_CON) );
	ch_printf( ch, "&gA3&w) Intelligenza:  [&c%5d&w]   &gA4&w) Concentrazione:  [&c%5d&w]   &gA5&w) Volontà:  [&c%5d&w]\r\n",
		get_curr_attr(victim, ATTR_INT), get_curr_attr(victim, ATTR_COC), get_curr_attr(victim, ATTR_WIL) );
	ch_printf( ch, "&gA6&w) Agilità:  [&c%5d&w]   &gA7&w) Riflessi:  [&c%5d&w]   &gA8&w) Velocità:  [&c%5d&w]\r\n",
		get_curr_attr(victim, ATTR_AGI), get_curr_attr(victim, ATTR_REF), get_curr_attr(victim, ATTR_SPE) );
	ch_printf( ch, "&gA9&w) Spiritualità:  [&c%5d&w]   &gAA&w) Migrazione:  [&c%5d&w]   &gAB&w) Assorbimento:  [&c%5d&w]\r\n",
		get_curr_attr(victim, ATTR_SPI), get_curr_attr(victim, ATTR_MIG), get_curr_attr(victim, ATTR_ABS) );
	ch_printf( ch, "&gAC&w) Empatia:  [&c%5d&w]   &gAD&w) Bellezza:  [&c%5d&w]   &gAE&w) Comando:  [&c%5d&w]\r\n",
		get_curr_attr(victim, ATTR_EMP), get_curr_attr(victim, ATTR_BEA), get_curr_attr(victim, ATTR_LEA) );

	ch_printf( ch, "&gB0&w) Vista:  [&c%5d&w]   &gB1&w) Udito:  [&c%5d&w]   &gB2&w) Olfatto:  [&c%5d&w]\r\n",
		get_curr_sense(victim, SENSE_SIGHT), get_curr_sense(victim, SENSE_HEARING), get_curr_sense(victim, SENSE_SMELL) );
	ch_printf( ch, "&gB3&w) Gusto:  [&c%5d&w]   &gB4&w) Tatto:  [&c%5d&w]   &gB5&w) Sesto Senso:  [&c%5d&w]\r\n",
		get_curr_sense(victim, SENSE_TASTE), get_curr_sense(victim, SENSE_TOUCH), get_curr_sense(victim, SENSE_SIXTH) );

	ch_printf( ch, "&gC0&w) Vita:   [&c%5d&w/&c%5d&w],  &gC1&w) Mana:   [&c%5d&w/&c%5d&w]\r\n",
		victim->points[POINTS_LIFE], victim->points_max[POINTS_LIFE], victim->points_max[POINTS_MANA], victim->points[POINTS_MANA] );
	ch_printf( ch, "&gC2&w) Movimento:[&c%5d&w/&c%-5d&w] &gC3&w) Anima:   [&c%5d&w/&c%5d&w]\r\n",
		victim->points[POINTS_MOVE], victim->points_max[POINTS_MOVE], victim->points[POINTS_SOUL], victim->points_max[POINTS_SOUL] );

	ch_printf( ch, "&gD&w) Gold:  [&c%11d&w]\r\n",		victim->gold );
	ch_printf( ch, "&gF&w) Thirst:      [&c%5d&w],  &gG&w) Full:       [&c%5d&w],  &gH&w) Drunk:     [&c%5d&w]\r\n",
		victim->pg->condition[CONDITION_THIRST], victim->pg->condition[CONDITION_FULL], victim->pg->condition[CONDITION_DRUNK] );

	ch_printf( ch, "&gJ&w) Saving Throws\r\n" );
	ch_printf( ch, "&gK&w) Resistant  : &O%s\r\n",	code_bit(NULL, victim->resistant,	 CODE_RIS) );
	ch_printf( ch, "&gL&w) Immune     : &O%s\r\n",	code_bit(NULL, victim->immune,		 CODE_RIS) );
	ch_printf( ch, "&gM&w) Susceptible: &O%s\r\n",	code_bit(NULL, victim->susceptible,	 CODE_RIS) );
	ch_printf( ch, "&gN&w) Position   : &O%s\r\n",	code_name(NULL, victim->position,		 CODE_POSITION) );
	ch_printf( ch, "&gP&w) Plr Flags  : &c%s\r\n",	code_bit(NULL, victim->pg->flags, CODE_PLAYER) );
	ch_printf( ch, "&gQ&w) Affected   : &c%s\r\n",	code_bit(NULL, victim->affected_by,	 CODE_AFFECT) );

	if ( victim->pg->clan )
		ch_printf( ch, "&gS&w) Clan       : &O%s\r\n", victim->pg->clan->name );
	else if ( !victim->pg->clan )
		ch_printf( ch, "&gS&w) Clan		: &ONone\r\n" );

	if ( get_trust(ch) >= TRUST_BUILDER )
	{
		send_to_char( ch, "&gE&w) Esci\r\n" );
		send_to_char( ch, "Enter choice : " );
	}

	OLC_MODE(d) = MEDIT_PC_MAIN_MENU;
}


void medit_disp_menu( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *victim = d->character->dest_buf;

	if ( IS_PG(victim) )
		medit_disp_pc_menu( d );
	else
		medit_disp_npc_menu( d );
}


/*
 * (TT) bisogna testarlo con i player
 */
DO_RET do_omedit( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA		*victim;
	char			 arg[MIL];

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I don't think so..\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "OEdit what?\r\n" );
		return;
	}

	victim = get_char_mud( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo mob non c'è.\r\n" );
		return;
	}

	/* Make sure the object isnt already being edited */
	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( d->connected == CON_MEDIT )
		{
			if ( IS_MOB(victim) && d->olc && OLC_VNUM(d) == victim->pIndexData->vnum )
			{
				ch_printf( ch, "That mobile is currently being edited by %s.\r\n", d->character->name );
				return;
			}
			else if ( IS_PG(victim) && d->character->dest_buf == victim )
			{
				ch_printf( ch, "That pg is currently being edited by %s.\r\n", d->character->name );
				return;
			}
		}
	}

	if ( !can_modify_mob(ch, victim) )
		return;

	d = ch->desc;
	CREATE( d->olc, OLC_DATA, 1 );
	if ( IS_MOB(victim) )
		OLC_VNUM(d) = victim->pIndexData->vnum;
	else
		medit_setup_arrays( );

	d->character->dest_buf = victim;
	d->connected = CON_MEDIT;
	OLC_CHANGE(d) = FALSE;
	medit_disp_menu( d );

	act( AT_ACTION, "$n starts using OLC.", ch, NULL, NULL, TO_ADMIN );
}


DO_RET do_mcopy( CHAR_DATA *ch, char *argument )
{
	MOB_PROTO_DATA *orig;
	MOB_PROTO_DATA *copy;
	char			arg1[MIL];
	char			arg2[MIL];
	int				x;
	VNUM			ovnum;
	VNUM			cvnum;
	MPROG_DATA	   *mprog;
	MPROG_DATA	   *mprog_dest = NULL;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Usage: mcopy <original> <new>\r\n" );
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

	if ( get_mob_index(NULL, cvnum) )
	{
		send_to_char( ch, "That mob already exists.\r\n" );
		return;
	}

	orig = get_mob_index( NULL, ovnum );
	if ( !orig )
	{
		send_to_char( ch, "How can you copy something that doesnt exist?\r\n" );
		return;
	}

	CREATE( copy, MOB_PROTO_DATA, 1 );
	copy->vnum					= cvnum;
	copy->name					= str_dup( orig->name );
	copy->short_descr			= str_dup( orig->short_descr );
	copy->long_descr			= str_dup( orig->long_descr  );
	copy->description	        = str_dup( orig->description );
	SET_BITS( copy->act, orig->act );
	SET_BITS( copy->affected_by, orig->affected_by );
	copy->pShop					= NULL;
	copy->rShop					= NULL;
	copy->spec_fun				= orig->spec_fun;
	copy->first_mudprog				= NULL;
	copy->alignment				= orig->alignment;
	copy->level					= orig->level;
	copy->ac					= orig->ac;
	copy->hitnodice				= orig->hitnodice;
	copy->hitsizedice			= orig->hitsizedice;
	copy->hitplus				= orig->hitplus;
	copy->damnodice				= orig->damnodice;
	copy->damsizedice			= orig->damsizedice;
	copy->damplus				= orig->damplus;
	copy->gold					= orig->gold;
	copy->exp					= orig->exp;
	copy->position				= orig->position;
	copy->defposition			= orig->defposition;
	copy->sex					= orig->sex;
	for ( x = 0;  x < MAX_ATTR;  x++ )
		copy->attr_perm[x]		= orig->attr_perm[x];
	for ( x = 0;  x < MAX_SENSE;  x++ )
		copy->sense_perm[x]		= orig->sense_perm[x];
	copy->race					= orig->race;
	copy->class					= orig->class;
	SET_BITS( copy->xflags, orig->xflags );
	SET_BITS( copy->resistant, orig->resistant );
	SET_BITS( copy->immune, orig->immune );
	SET_BITS( copy->susceptible, orig->susceptible );
	copy->numattacks			= orig->numattacks;
	SET_BITS( copy->attacks, orig->attacks );
	SET_BITS( copy->defenses, orig->defenses );
	copy->height				= orig->height;
	copy->weight				= orig->weight;
	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		copy->saving_throw[x]	= orig->saving_throw[x];

	copy->first_mudprog = mprog_dest;
	for ( mprog = orig->first_mudprog;  mprog;  mprog = mprog->next )
	{
		mprog_dest = copy_mudprog( mprog );
		LINK( mprog_dest, copy->first_mudprog, copy->last_mudprog, next, prev );

		mprog_dest = mprog_dest->next;
	}

	copy->count			= 0;
	LINK( copy, first_mob_proto, last_mob_proto, next, prev );
	top_mob_proto++;

	set_char_color( AT_PLAIN, ch );
	send_to_char( ch, "Mobile copied.\r\n" );
}


/**************************************************************************
 Menu Displaying Functions
 **************************************************************************/

/*
 * Display poistions (sitting, standing etc), same for pos and defpos
 */
void medit_disp_positions( DESCRIPTOR_DATA *d )
{
	int		x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	/* La posizione deve essere minore di POSITION_MOUNT, quelle sucessive sono posizioni modificabili solo dal gioco */
	for ( x = 0;  x < POSITION_MOUNT;  x++ )
		ch_printf( d->character, "&g%2d&w) %s\r\n", x+1, code_name(NULL, x, CODE_POSITION) );

	send_to_char( d->character, "Enter position number : " );
}


/*
 * Display mobile sexes, this is hard coded cause it just works that way :)
 */
void medit_disp_sex( DESCRIPTOR_DATA *d )
{
	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	send_to_char( d->character, " &g0&w) Maschio\r\n"			);
	send_to_char( d->character, " &g1&w) Femmina\r\n"			);
	send_to_char( d->character, "\r\nEnter gender number : "	);
}


/*
 * Used for both mob affected_by and object affect bitvectors
 */
void medit_disp_ris( DESCRIPTOR_DATA *d )
{
	CHAR_DATA	*victim = d->character->dest_buf;
	int			 x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_RIS;  x++ )
		ch_printf( d->character, "&g%2d&w) %-20.20s\r\n", x+1, code_name(NULL, x, CODE_RIS) );

	switch ( OLC_MODE(d) )
	{
	  case OEDIT_AFFECT_MODIFIER:
	  {
		BITVECTOR	*bits;

		bits = convert_bitvector( d->character->tempnum );
		ch_printf( d->character, "\r\nCurrent flags: &c%s&w\r\n", code_bit(NULL, bits, CODE_RIS) );
		destroybv( bits );
	  }
	  break;

	  case MEDIT_RESISTANT:
		ch_printf( d->character, "\r\nCurrent flags: &c%s&w\r\n", code_bit(NULL, victim->resistant, CODE_RIS) );
		break;

	  case MEDIT_IMMUNE:
		ch_printf( d->character, "\r\nCurrent flags: &c%s&w\r\n", code_bit(NULL, victim->immune, CODE_RIS) );
		break;

	  case MEDIT_SUSCEPTIBLE:
		ch_printf( d->character, "\r\nCurrent flags: &c%s&w\r\n", code_bit(NULL, victim->susceptible, CODE_RIS) );
		break;
	}

	send_to_char( d->character, "Enter flag (0 to quit): " );
}


/*
 * Mobile attacks
 */
void medit_disp_attack_menu( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *victim = d->character->dest_buf;
	int		   x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_ATTACK;  x++ )
		ch_printf( d->character, "&g%2d&w) %-20.20s\r\n", x+1, code_name(NULL, x, CODE_ATTACK) );

	ch_printf( d->character, "Current flags: &c%s&w\r\nEnter attack flag (0 to exit): ",
		code_bit(NULL, victim->attacks, CODE_ATTACK) );
}


/*
 * Display menu of NPC defense flags
 */
void medit_disp_defense_menu( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *victim = d->character->dest_buf;
	int		   x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_DEFENSE;  x++ )
		ch_printf( d->character, "&g%2d&w) %-20.20s\r\n", x+1, code_name(NULL, x, CODE_DEFENSE) );

	ch_printf( d->character, "Current flags: &c%s&w\r\nEnter defense flag (0 to exit): ",
		code_bit(NULL, victim->defenses, CODE_DEFENSE) );
}


/*-------------------------------------------------------------------*/
/*. Display mob-flags menu .*/

void medit_disp_mob_flags( DESCRIPTOR_DATA *d )
{
	CHAR_DATA		   *victim = d->character->dest_buf;
	int					x,
						columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_MOB;  x++)
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s  ", x+1, code_name(NULL, x, CODE_MOB) );
		if ( !(++columns % 2) )
			ch_printf( d->character, "\r\n" );
	}

	ch_printf( d->character, "\r\nCurrent flags : &c%s&w\r\n"
		"Enter mob flags (0 to quit) : ",
		code_bit(NULL, victim->act, CODE_MOB) );
}


/*
 * Special handing for PC only flags
 */
void medit_disp_plr_flags( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *victim = d->character->dest_buf;
	int		   x;
	int		   columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_PLAYER;  x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s   ", x+1, code_name(NULL, x, CODE_PLAYER) );
		if ( ++columns%2 == 0 )
			ch_printf( d->character, "\r\n" );
	}

	ch_printf( d->character, "\r\nCurrent flags: &c%s&w\r\nEnter flags (0 to quit): ",
		code_bit(NULL, victim->pg->flags, CODE_PLAYER) );
}


/*-------------------------------------------------------------------*/
/*
 * Display aff-flags menu
 */
void medit_disp_aff_flags( DESCRIPTOR_DATA *d )
{
	char	   buf[MSL];
	CHAR_DATA *victim = d->character->dest_buf;
	int		   x,
			   columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_AFFECT;  x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s  ", x+1, code_name(NULL, x, CODE_AFFECT) );
		if ( !(++columns % 2) )
			send_to_char( d->character, "\r\n" );
	}

	if ( OLC_MODE(d) == OEDIT_AFFECT_MODIFIER )
	{
		buf[0] = '\0';

		for ( x = 0;  x < MAX_AFFECT;  x++ )
		{
			BITVECTOR *bits;

			bits = convert_bitvector( d->character->tempnum );
			if ( HAS_BIT(bits, x) )
			{
				strcat( buf, " " );
				strcat( buf, code_name(NULL, x, CODE_AFFECT) );
			}
			destroybv( bits );
		}
		ch_printf( d->character, "\r\nCurrent flags   : &c%s&w\r\n", buf );
	}
	else
	{
		ch_printf( d->character, "\r\nCurrent flags   : &c%s&w\r\n",
			code_bit(NULL, victim->affected_by, CODE_AFFECT) );
	}

	send_to_char( d->character, "Enter affected flags (0 to quit) : " );
}


void medit_disp_clans( DESCRIPTOR_DATA *d )
{
	int		x;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	send_to_char( d->character, "Clans:\r\n" );
	for ( x = 0;  x <= olc_top_clan;  x++ )
		ch_printf( d->character, "&g%2d&w) %-20.20s\r\n", x+1, olc_clan_list[x] );

	send_to_char( d->character, "Enter choice (0 for none): " );
}



void medit_disp_parts( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *victim = d->character->dest_buf;
	int		   x,
			   columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_PART;  x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s	", x+1, code_name(NULL, x, CODE_PART) );
		if ( ++columns%2 == 0 )
			send_to_char( d->character, "\r\n" );
	}

	ch_printf( d->character, "\r\nCurrent flags: %s\r\nEnter flag or 0 to exit: ",
		code_bit(NULL, victim->xflags, CODE_PART) );
}


void medit_disp_classes( DESCRIPTOR_DATA *d )
{
	int		   x,
			   columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s	 \r\n", x+1, code_name(NULL, x, CODE_CLASS) );
		if ( ++columns%2 == 0 )
			send_to_char( d->character, "\r\n" );
	}

	send_to_char( d->character, "\r\nEnter class: " );
}


void medit_disp_races( DESCRIPTOR_DATA *d )
{
	int		   x,
			   columns = 0;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		ch_printf( d->character, "&g%2d&w) %-20.20s      \r\n", x+1, code_name(NULL, x, CODE_RACE) );
		if ( ++columns%2 == 0 )
			send_to_char( d->character, "\r\n" );
	}

	send_to_char( d->character, "\r\nEnter race: " );
}


void medit_disp_saving_menu( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *victim = d->character->dest_buf;

	write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

	ch_printf( d->character, "&g1&w) %-30.30s: %2d\r\n", "Saving vs. breath",			victim->saving_throw[SAVETHROW_BREATH] );
	ch_printf( d->character, "&g2&w) %-30.30s: %2d\r\n", "Saving vs. para & petri",		victim->saving_throw[SAVETHROW_PARA_PETRI] );
	ch_printf( d->character, "&g3&w) %-30.30s: %2d\r\n", "Saving vs. poison & death",	victim->saving_throw[SAVETHROW_POISON_DEATH] );
	ch_printf( d->character, "&g4&w) %-30.30s: %2d\r\n", "Saving vs. spell & staff",	victim->saving_throw[SAVETHROW_SPELL_STAFF] );
	ch_printf( d->character, "&g4&w) %-30.30s: %2d\r\n", "Saving vs. wands & rod",		victim->saving_throw[SAVETHROW_ROD_WANDS] );
	ch_printf( d->character, "\r\nModify which saving throw: " );

	OLC_MODE(d) = MEDIT_SAVE_MENU;
}


/*
 * Bogus command for resetting stuff
 */
DO_RET do_medit_reset( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim = ch->dest_buf;

	switch ( ch->substate )
	{
	  default:
		return;

	  case SUB_MOB_DESCR:
		if ( !ch->dest_buf )
		{
			send_to_char( ch, "do_medit_reset: informare il Coder.\r\n" );
			send_log( NULL, LOG_OLC, "do_medit_reset: sub_mob_desc: NULL ch->dest_buf" );
			cleanup_olc( ch->desc );
			ch->substate = SUB_NONE;
			return;
		}
	    DISPOSE( victim->description );
	    victim->description = copy_buffer( ch );

		stop_editing( ch );
		ch->dest_buf = victim;
		ch->substate = SUB_NONE;
		ch->desc->connected = CON_MEDIT;
		medit_disp_menu( ch->desc );

		return;
	}
}


/**************************************************************************
  The GARGANTAUN event handler
 **************************************************************************/
void medit_parse( DESCRIPTOR_DATA *d, char *arg )
{
	CHAR_DATA		   *victim = d->character->dest_buf;
	int					number = 0;
	int					minattr;
	int					maxattr;
	char				arg1[MIL];
	char				buf[MSL];
	CLAN_DATA		   *clan;
	AREA_DATA		   *tarea;
	bool				found = FALSE;

	minattr = DEF_MIN_ATTR;
	maxattr = DEF_MAX_ATTR;

	switch ( OLC_MODE(d) )
	{
	  case MEDIT_CONFIRM_SAVESTRING:
		switch ( toupper(*arg) )
		{
		  case 'Y':
			/* If its a mob, fold_area otherwise save_player */
			send_to_char( d->character, "Saving...\r\n" );
			if ( IS_MOB(victim) )
			{
					send_log( NULL, LOG_OLC, "medit_parse: %s edits mob %d(%s)",
					d->character->name,
					victim->pIndexData->vnum,
					victim->name );

				for ( tarea = first_area;  tarea;  tarea = tarea->next )
				{
					if ( OLC_VNUM(d) >= tarea->vnum_low
					  && OLC_VNUM(d) <= tarea->vnum_high )
					{
						sprintf( buf, "%s%s", AREA_DIR, tarea->filename );
						if ( get_trust(d->character) >= TRUST_BUILDER )
							fold_area( tarea, buf, FALSE );
						found = TRUE;
					}
				}
				/* I'm assuming that if it isn't an installed area, its the char's */
				if ( !found
				  && (tarea = d->character->pg->build_area) != NULL
				  && get_trust(d->character) >= TRUST_BUILDER
				  && HAS_BIT(tarea->flags, AREAFLAG_LOADED) )
				{
					tarea = d->character->pg->build_area;
					sprintf( buf, "%s%s", BUILD_DIR, tarea->filename );
					fold_area( tarea, buf, FALSE );
				}
			}
			else
			{
				send_log( NULL, LOG_OLC, "medit_parse: %s edits %s", d->character->name, victim->name );
				save_player( victim );
			}
			cleanup_olc( d );
			return;

		  case 'N':
			cleanup_olc( d );
			return;

		  default:
			send_to_char( d->character, "Invalid choice!\r\n" );
			send_to_char( d->character, "Do you wish to save to disk? : " );
			return;
		}
		break;

	  case MEDIT_NPC_MAIN_MENU:
		switch ( toupper(arg[0]) )
		{
		  case 'E':
			cleanup_olc( d );
			return;
		  case '1':
			OLC_MODE(d) = MEDIT_SEX;
			medit_disp_sex(d);
			return;
		  case '2':
			OLC_MODE(d) = MEDIT_NAME;
			send_to_char( d->character, "\r\nEnter name: " );
			return;
		  case '3':
			OLC_MODE(d) = MEDIT_S_DESC;
			send_to_char( d->character, "\r\nEnter short description: " );
			return;
		  case '4':
			OLC_MODE(d) = MEDIT_L_DESC;
			send_to_char( d->character, "\r\nEnter long description: " );
			return;
		  case '5':
			OLC_MODE(d) = MEDIT_D_DESC;
			d->character->substate = SUB_MOB_DESCR;
			d->character->last_cmd = do_medit_reset;

			send_to_char( d->character, "Enter new mob description:\r\n" );
			if ( !victim->description )
				victim->description = str_dup( "" );
			start_editing( d->character, victim->description );
			return;
		  case '6':
			OLC_MODE(d) = MEDIT_CLASS;
			medit_disp_classes(d);
			return;
		  case '7':
			OLC_MODE(d) = MEDIT_RACE;
			medit_disp_races(d);
			return;
		  case '8':
			OLC_MODE(d) = MEDIT_LEVEL;
			send_to_char( d->character, "\r\nEnter level: " );
			return;
		  case '9':
			OLC_MODE(d) = MEDIT_ALIGNMENT;
			send_to_char( d->character, "\r\nEnter alignment: " );
			return;

		  case 'A':
			switch ( toupper(arg[1]) )
			{
			  case '0':
			  	OLC_MODE(d) = MEDIT_STR;
				send_to_char( d->character, "\r\nInserire la forza: " );
				return;
			  case '1':
			  	OLC_MODE(d) = MEDIT_RES;
				send_to_char( d->character, "\r\nInserire la resistenza: " );
				return;
			  case '2':
			  	OLC_MODE(d) = MEDIT_CON;
				send_to_char( d->character, "\r\nInserire la costituzione: " );
				return;

			  case '3':
			  	OLC_MODE(d) = MEDIT_INT;
				send_to_char( d->character, "\r\nInserire l'intelligenza: " );
				return;
			  case '4':
			  	OLC_MODE(d) = MEDIT_COC;
				send_to_char( d->character, "\r\nInserire la concentrazione: " );
				return;
			  case '5':
			  	OLC_MODE(d) = MEDIT_WIL;
				send_to_char( d->character, "\r\nInserire la volontà: " );
				return;

			  case '6':
			  	OLC_MODE(d) = MEDIT_AGI;
				send_to_char( d->character, "\r\nInserire l'agilità: " );
				return;
			  case '7':
			  	OLC_MODE(d) = MEDIT_REF;
				send_to_char( d->character, "\r\nInserire i riflessi: " );
				return;
			  case '8':
			  	OLC_MODE(d) = MEDIT_SPE;
				send_to_char( d->character, "\r\nInserire la velocità: " );
				return;

			  case '9':
			  	OLC_MODE(d) = MEDIT_SPI;
				send_to_char( d->character, "\r\nInserire la spiritualità: " );
				return;
			  case 'A':
			  	OLC_MODE(d) = MEDIT_MIG;
				send_to_char( d->character, "\r\nInserire la migrazione: " );
				return;
			  case 'B':
			  	OLC_MODE(d) = MEDIT_ABS;
				send_to_char( d->character, "\r\nInserire l'assorbimento: " );
				return;

			  case 'C':
			  	OLC_MODE(d) = MEDIT_EMP;
				send_to_char( d->character, "\r\nInserire l'empatia: " );
				return;
			  case 'D':
			  	OLC_MODE(d) = MEDIT_BEA;
				send_to_char( d->character, "\r\nInserire la bellezza: " );
				return;
			  case 'E':
			  	OLC_MODE(d) = MEDIT_LEA;
				send_to_char( d->character, "\r\nInserire il comando: " );
				return;
			}

		  case 'B':
			switch ( toupper(arg[1]) )
			{
			  case '0':
				OLC_MODE(d) = MEDIT_SIGHT;
				send_to_char( d->character, "\r\nInserire la vista: " );
				return;
			  case '1':
				OLC_MODE(d) = MEDIT_HEARING;
				send_to_char( d->character, "\r\nInserire l'udito: " );
				return;
			  case '2':
				OLC_MODE(d) = MEDIT_SMELL;
				send_to_char( d->character, "\r\nInserire l'olfatto: " );
				return;
			  case '3':
				OLC_MODE(d) = MEDIT_TASTE;
				send_to_char( d->character, "\r\nInserire il gusto: " );
				return;
			  case '4':
				OLC_MODE(d) = MEDIT_TOUCH;
				send_to_char( d->character, "\r\nInserire il tatto: " );
				return;
			  case '5':
				OLC_MODE(d) = MEDIT_SIXTH;
				send_to_char( d->character, "\r\nInserire il sesto senso: " );
				return;
			}

		  case 'C':
			OLC_MODE(d) = MEDIT_DAMNUMDIE;
			send_to_char( d->character, "\r\nEnter number of damage dice: " );
			return;
		  case 'D':
			OLC_MODE(d) = MEDIT_DAMSIZEDIE;
			send_to_char( d->character, "\r\nEnter size of damage dice: " );
			return;
		  case 'F':
			OLC_MODE(d) = MEDIT_DAMPLUS;
			send_to_char( d->character, "\r\nEnter amount to add to damage: " );
			return;
		  case 'G':
			OLC_MODE(d) = MEDIT_HITNUMDIE;
			send_to_char( d->character, "\r\nEnter number of hitpoint dice: " );
			return;
		  case 'H':
			OLC_MODE(d) = MEDIT_HITSIZEDIE;
			send_to_char( d->character, "\r\nEnter size of hitpoint dice: " );
			return;
		  case 'I':
			OLC_MODE(d) = MEDIT_HITPLUS;
			send_to_char( d->character, "\r\nEnter amount to add to hitpoints: " );
			return;
		  case 'J':
			OLC_MODE(d) = MEDIT_GOLD;
			send_to_char( d->character, "\r\nEnter amount of gold mobile carries: " );
			return;
		  case 'L':
			OLC_MODE(d) = MEDIT_SAVE_MENU;
			medit_disp_saving_menu(d);
			return;
		  case 'M':
			OLC_MODE(d) = MEDIT_RESISTANT;
			medit_disp_ris(d);
			return;
		  case 'N':
			OLC_MODE(d) = MEDIT_IMMUNE;
			medit_disp_ris(d);
			return;
		  case 'O':
			OLC_MODE(d) = MEDIT_SUSCEPTIBLE;
			medit_disp_ris(d);
			return;
		  case 'P':
			OLC_MODE(d) = MEDIT_POS;
			medit_disp_positions(d);
			return;
		  case 'Q':
			OLC_MODE(d) = MEDIT_ATTACK;
			medit_disp_attack_menu(d);
			return;
		  case 'R':
			OLC_MODE(d) = MEDIT_DEFENSE;
			medit_disp_defense_menu(d);
			return;
		  case 'S':
			OLC_MODE(d) = MEDIT_PARTS;
			medit_disp_parts(d);
			return;
		  case 'T':
			OLC_MODE(d) = MEDIT_NPC_FLAGS;
			medit_disp_mob_flags(d);
			return;
		  case 'U':
			OLC_MODE(d) = MEDIT_AFFECT_FLAGS;
			medit_disp_aff_flags(d);
			return;

		  default:
			medit_disp_npc_menu( d );
			return;
		}
		break;

	  case MEDIT_PC_MAIN_MENU:
		switch ( toupper(arg[0]) )
		{
		  case 'E':
			if ( OLC_CHANGE(d) )
			{
				send_to_char( d->character, "Do you wish to save changes to disk? (y/n): " );
				OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
			}
			else
				cleanup_olc( d );
			return;

		  case '1':
			OLC_MODE(d) = MEDIT_SEX;
			medit_disp_sex(d);
			return;
		  case '2':
			if ( IS_PG(victim) && get_trust(d->character) < TRUST_BUILDER )
				break;
			OLC_MODE(d) = MEDIT_NAME;
			return;
		  case '3':
			OLC_MODE(d) = MEDIT_D_DESC;
			d->character->substate = SUB_MOB_DESCR;
			d->character->last_cmd = do_medit_reset;

			send_to_char( d->character, "Enter new player description:\r\n" );
			if ( !victim->description )
				victim->description = str_dup( "" );
			start_editing( d->character, victim->description );
			return;
		  case '4':
			OLC_MODE(d) = MEDIT_CLASS;
			medit_disp_classes(d);
			return;
		  case '5':
			OLC_MODE(d) = MEDIT_RACE;
			medit_disp_races(d);
			return;
		  case '6':
			send_to_char( d->character, "\r\nNPC Only!!" );
			break;
		  case '7':
			OLC_MODE(d) = MEDIT_ALIGNMENT;
			send_to_char( d->character, "\r\nEnter alignment: " );
			return;
		  case '8':
			OLC_MODE(d) = MEDIT_MENTALSTATE;
			send_to_char( d->character, "\r\nEnter players mentalstate: " );
			return;
		  case '9':
			OLC_MODE(d) = MEDIT_EMOTIONAL;
			send_to_char( d->character, "\r\nEnter players emotional state: " );
			return;

		  case 'A':
			switch ( toupper(arg[1]) )
			{
			  case '0':
			  	OLC_MODE(d) = MEDIT_STR;
				send_to_char( d->character, "\r\nInserire la forza: " );
				return;
			  case '1':
			  	OLC_MODE(d) = MEDIT_RES;
				send_to_char( d->character, "\r\nInserire la resistenza: " );
				return;
			  case '2':
			  	OLC_MODE(d) = MEDIT_CON;
				send_to_char( d->character, "\r\nInserire la costituzione: " );
				return;

			  case '3':
			  	OLC_MODE(d) = MEDIT_INT;
				send_to_char( d->character, "\r\nInserire l'intelligenza: " );
				return;
			  case '4':
			  	OLC_MODE(d) = MEDIT_COC;
				send_to_char( d->character, "\r\nInserire la concentrazione: " );
				return;
			  case '5':
			  	OLC_MODE(d) = MEDIT_WIL;
				send_to_char( d->character, "\r\nInserire la volontà: " );
				return;

			  case '6':
			  	OLC_MODE(d) = MEDIT_AGI;
				send_to_char( d->character, "\r\nInserire l'agilità: " );
				return;
			  case '7':
			  	OLC_MODE(d) = MEDIT_REF;
				send_to_char( d->character, "\r\nInserire i riflessi: " );
				return;
			  case '8':
			  	OLC_MODE(d) = MEDIT_SPE;
				send_to_char( d->character, "\r\nInserire la velocità: " );
				return;

			  case '9':
			  	OLC_MODE(d) = MEDIT_SPI;
				send_to_char( d->character, "\r\nInserire la spiritualità: " );
				return;
			  case 'A':
			  	OLC_MODE(d) = MEDIT_MIG;
				send_to_char( d->character, "\r\nInserire la migrazione: " );
				return;
			  case 'B':
			  	OLC_MODE(d) = MEDIT_ABS;
				send_to_char( d->character, "\r\nInserire l'assorbimento: " );
				return;

			  case 'C':
			  	OLC_MODE(d) = MEDIT_EMP;
				send_to_char( d->character, "\r\nInserire l'empatia: " );
				return;
			  case 'D':
			  	OLC_MODE(d) = MEDIT_BEA;
				send_to_char( d->character, "\r\nInserire la bellezza: " );
				return;
			  case 'E':
			  	OLC_MODE(d) = MEDIT_LEA;
				send_to_char( d->character, "\r\nInserire il comando: " );
				return;
			}

		  case 'B':
			switch ( toupper(arg[1]) )
			{
			  case '0':
				OLC_MODE(d) = MEDIT_SIGHT;
				send_to_char( d->character, "\r\nInserire la vista: " );
				return;
			  case '1':
				OLC_MODE(d) = MEDIT_HEARING;
				send_to_char( d->character, "\r\nInserire l'udito: " );
				return;
			  case '2':
				OLC_MODE(d) = MEDIT_SMELL;
				send_to_char( d->character, "\r\nInserire l'olfatto: " );
				return;
			  case '3':
				OLC_MODE(d) = MEDIT_TASTE;
				send_to_char( d->character, "\r\nInserire il gusto: " );
				return;
			  case '4':
				OLC_MODE(d) = MEDIT_TOUCH;
				send_to_char( d->character, "\r\nInserire il tatto: " );
				return;
			  case '5':
				OLC_MODE(d) = MEDIT_SIXTH;
				send_to_char( d->character, "\r\nInserire il sesto senso: " );
				return;
			}

		  case 'C':
			switch ( toupper(arg[1]) )
			{
			  case '0':
				OLC_MODE(d) = MEDIT_LIFE;
				send_to_char( d->character, "\r\nInserire la vita: " );
				return;
			  case '1':
				OLC_MODE(d) = MEDIT_MANA;
				send_to_char( d->character, "\r\nInserire il mana: " );
				return;
			  case '2':
				OLC_MODE(d) = MEDIT_MOVE;
				send_to_char( d->character, "\r\nInserire il movimento: " );
				return;
			  case '3':
				OLC_MODE(d) = MEDIT_SOUL;
				send_to_char( d->character, "\r\nInserire l'anima: " );
				return;
			}

		  case 'D':
			OLC_MODE(d) = MEDIT_GOLD;
			send_to_char( d->character, "\r\nEnter amount of gold player carries: " );
			return;
		  case 'F':
			OLC_MODE(d) = MEDIT_THIRST;
	 		send_to_char( d->character, "\r\nEnter player's thirst (0 = dehydrated): " );
			return;
		  case 'G':
			OLC_MODE(d) = MEDIT_FULL;
			send_to_char( d->character, "\r\nEnter player's fullness (0 = starving): " );
			return;
		  case 'H':
			OLC_MODE(d) = MEDIT_DRUNK;
			send_to_char( d->character, "\r\nEnter player's drunkeness (0 = sober): " );
			return;
		  case 'J':
			OLC_MODE(d) = MEDIT_SAVE_MENU;
			medit_disp_saving_menu(d);
			return;
		  case 'K':
			OLC_MODE(d) = MEDIT_RESISTANT;
			medit_disp_ris(d);
			return;
		  case 'L':
			OLC_MODE(d) = MEDIT_IMMUNE;
			medit_disp_ris(d);
			return;
		  case 'M':
			OLC_MODE(d) = MEDIT_SUSCEPTIBLE;
			medit_disp_ris(d);
			return;
		  case 'N':
			send_to_char( d->character, "NPCs Only!!\r\n" );
			break;
		  case 'O':
			OLC_MODE(d) = MEDIT_PLAYER_FLAGS;
			medit_disp_plr_flags(d);
			return;
		  case 'Q':
			OLC_MODE(d) = MEDIT_AFFECT_FLAGS;
			medit_disp_aff_flags(d);
			return;
		  case 'S':
			if ( get_trust(d->character) < TRUST_BUILDER )
				break;
			OLC_MODE(d) = MEDIT_CLAN;
			medit_disp_clans(d);
			return;

		  default:
			medit_disp_pc_menu( d );
			return;
		}
		break;

	  case MEDIT_NAME:
		if ( IS_PG(victim) )
		{
			sprintf( buf, "pcrename %s %s", victim->name, arg );
			send_command( d->character, buf, CO );
			olc_log( d, "Changes name to %s", arg );
			return;
		}

		DISPOSE( victim->name );
		victim->name = str_dup( arg );
		olc_log( d, "Changed name to %s", arg );
		break;

	  case MEDIT_S_DESC:
		DISPOSE( victim->short_descr );
		victim->short_descr = str_dup( arg );
		olc_log( d, "Changed short desc to %s", arg );
		break;

	  case MEDIT_L_DESC:
		strcpy( buf, arg );
		strcat( buf, "\r\n" );
		DISPOSE( victim->long_descr );
		victim->long_descr = str_dup( buf );
		olc_log( d, "Changed long desc to %s", arg );
		break;

	  case MEDIT_D_DESC:
		/*. We should never get here .*/
		cleanup_olc( d );
		send_log( NULL, LOG_OLC, "medit_parse: Reached D_DESC case!" );
		break;

	  case MEDIT_NPC_FLAGS:
		/* REDONE, again, then again */
		if ( is_number(arg) )
		{
			if ( atoi(arg) == 0 )
				break;
		}

		while ( VALID_STR(arg) )
		{
			arg = one_argument( arg, arg1 );

			if ( is_number(arg1) )
			{
				number = atoi( arg1 );

				number--;
				if ( number < 0 || number >= MAX_MOB )
				{
					send_to_char( d->character, "Invalid flag, try again: " );
					return;
				}
			}
			else
			{
				number = code_num( NULL, arg1, CODE_MOB );
				if ( number < 0 )
				{
					send_to_char( d->character, "Invalid flag, try again: " );
					return;
				}
			}

			TOGGLE_BIT( victim->act, number );
		}
		medit_disp_mob_flags( d );
		return;

	  case MEDIT_PLAYER_FLAGS:
		if ( is_number(arg) )
		{
			number = atoi( arg );

			if ( number == 0 )
				break;

			number--;
			if ( number < 0 || number >= MAX_PLAYER )
			{
				TOGGLE_BIT( victim->pg->flags, number );
				olc_log( d, "%s the flag %s", HAS_BIT(victim->act, number)  ?  "Added"  :  "Removed",
					code_name(NULL, number, CODE_PLAYER) );
			}
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_PLAYER );

				if ( number > 0 )
				{
					TOGGLE_BIT( victim->act, number );
					olc_log( d, "%s the flag %s", HAS_BIT(victim->act, number) ? "Added" : "Removed",
						code_name(NULL, number, CODE_PLAYER) );
				}
			}
		}
		medit_disp_plr_flags( d );
		return;

	  case MEDIT_AFFECT_FLAGS:
		if ( is_number(arg) )
		{
			number = atoi( arg );

			if ( number == 0 )
				break;

			number--;
			if ( number < 0 || number >= MAX_AFFECT )
			{
				TOGGLE_BIT( victim->affected_by, number );
				olc_log( d, "%s the affect %s", HAS_BIT(victim->affected_by, number) ? "Added" : "Removed",
					code_name(NULL, number, CODE_AFFECT) );
			}
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_AFFECT );
				if ( number > 0 )
				{
					TOGGLE_BIT( victim->affected_by, number );
					olc_log( d, "%s the affect %s", HAS_BIT(victim->affected_by, number) ? "Added" : "Removed",
						code_name(NULL, number, CODE_AFFECT) );
				}
			}
		}
		medit_disp_aff_flags(d);
		return;


/*-------------------------------------------------------------------*/
	  /*
	   * Numerical responses
	   */
	  case MEDIT_LIFE:
		victim->points_max[POINTS_LIFE] = URANGE( 1, atoi(arg), 30000 );
		olc_log( d, "Vita cambiata in %d", victim->points_max[POINTS_LIFE] );
		break;

	  case MEDIT_MANA:
		victim->points_max[POINTS_MANA] = URANGE( 1, atoi(arg), 30000 );
		olc_log( d, "Mana cambiati in %d", victim->points_max[POINTS_MANA] );
		break;

	  case MEDIT_MOVE:
		victim->points_max[POINTS_MOVE] = URANGE( 1, atoi(arg), 30000 );
		olc_log( d, "Movimento cambiato in %d", victim->points_max[POINTS_MOVE] );
		break;

	  case MEDIT_SOUL:
		victim->points_max[POINTS_SOUL] = URANGE( 1, atoi(arg), 30000 );
		olc_log( d, "Anima cambiata in %d", victim->points_max[POINTS_SOUL] );
		break;

	  case MEDIT_PRACTICE:
		victim->pg->practice = URANGE( 1, atoi(arg), 300 );
		olc_log( d, "Changed practives to %d", victim->pg->practice );
		break;

	  case MEDIT_PASSWORD:
	  {
		char	*pwdnew;

		pwdnew = check_password( d->character, arg );
		if ( !pwdnew )
			return;

		DISPOSE( victim->pg->pwd );
		victim->pg->pwd = str_dup( pwdnew );

		save_player( victim );
		olc_log( d, "Password modifica." );

		break;
	  }

	  case MEDIT_SAV1:
		victim->saving_throw[SAVETHROW_BREATH] = URANGE( -30, atoi(arg), 30 );
		medit_disp_saving_menu(d);
		olc_log( d, "Changed save_poison_death to %d", victim->saving_throw[SAVETHROW_BREATH] );
		return;

	  case MEDIT_SAV2:
		victim->saving_throw[SAVETHROW_PARA_PETRI] = URANGE( -30, atoi(arg), 30 );
		medit_disp_saving_menu(d);
		olc_log( d, "Changed save_wand to %d", victim->saving_throw[SAVETHROW_PARA_PETRI] );
		return;

	  case MEDIT_SAV3:
		victim->saving_throw[SAVETHROW_POISON_DEATH] = URANGE( -30, atoi(arg), 30 );
		medit_disp_saving_menu(d);
		olc_log( d, "Changed save_paralysis_petrification to %d", victim->saving_throw[SAVETHROW_POISON_DEATH] );
		return;

	  case MEDIT_SAV4:
		victim->saving_throw[SAVETHROW_SPELL_STAFF] = URANGE( -30, atoi(arg), 30 );
		medit_disp_saving_menu(d);
		olc_log( d, "Changed save_breath to %d", victim->saving_throw[SAVETHROW_SPELL_STAFF] );
		return;

	  case MEDIT_SAV5:
		victim->saving_throw[SAVETHROW_ROD_WANDS] = URANGE( -30, atoi(arg), 30 );
		medit_disp_saving_menu(d);
		olc_log( d, "Changed save_breath to %d", victim->saving_throw[SAVETHROW_ROD_WANDS] );
		return;

	  case MEDIT_STR:
		victim->attr_perm[ATTR_STR] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo forza cambiato a %d", victim->attr_perm[ATTR_STR] );
		break;

	  case MEDIT_RES:
		victim->attr_perm[ATTR_RES] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo resistenza cambiato a %d", victim->attr_perm[ATTR_RES] );
		break;

	  case MEDIT_CON:
		victim->attr_perm[ATTR_CON] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo costituzione cambiato a %d", victim->attr_perm[ATTR_CON] );
		break;

	  case MEDIT_INT:
		victim->attr_perm[ATTR_INT] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo intelligenza cambiato a %d", victim->attr_perm[ATTR_INT] );
		break;

	  case MEDIT_COC:
		victim->attr_perm[ATTR_COC] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo concentrazione cambiato a %d", victim->attr_perm[ATTR_COC] );
		break;

	  case MEDIT_WIL:
		victim->attr_perm[ATTR_WIL] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo volontà cambiato a %d", victim->attr_perm[ATTR_WIL] );
		break;

	  case MEDIT_AGI:
		victim->attr_perm[ATTR_AGI] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo agilità cambiato a %d", victim->attr_perm[ATTR_AGI] );
		break;

	  case MEDIT_REF:
		victim->attr_perm[ATTR_REF] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo riflessi cambiato a %d", victim->attr_perm[ATTR_REF] );
		break;

	  case MEDIT_SPE:
		victim->attr_perm[ATTR_SPE] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo velocità cambiato a %d", victim->attr_perm[ATTR_SPE] );
		break;

	  case MEDIT_SPI:
		victim->attr_perm[ATTR_SPI] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo spiritualità cambiato a %d", victim->attr_perm[ATTR_SPI] );
		break;

	  case MEDIT_MIG:
		victim->attr_perm[ATTR_MIG] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo migrazione cambiato a %d", victim->attr_perm[ATTR_MIG] );
		break;

	  case MEDIT_ABS:
		victim->attr_perm[ATTR_ABS] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo assorbimento cambiato a %d", victim->attr_perm[ATTR_ABS] );
		break;

	  case MEDIT_EMP:
		victim->attr_perm[ATTR_EMP] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo empatia cambiato a %d", victim->attr_perm[ATTR_EMP] );
		break;

	  case MEDIT_BEA:
		victim->attr_perm[ATTR_BEA] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo bellezza cambiato a %d", victim->attr_perm[ATTR_BEA] );
		break;

	  case MEDIT_LEA:
		victim->attr_perm[ATTR_LEA] = URANGE( minattr, atoi(arg), maxattr );
		olc_log( d, "Attributo comando cambiato a %d", victim->attr_perm[ATTR_LEA] );
		break;

	  case MEDIT_SEX:
		victim->sex = URANGE( 0, atoi( arg ), 2 );
		olc_log( d, "Changed sex to %s", (victim->sex == SEX_FEMALE)  ?  "Femmina"  :  "Maschio" );
		break;

	  case MEDIT_HITROLL:
		victim->hitroll = URANGE( 0, atoi(arg), 85);
		olc_log( d, "Changed hitroll to %d", victim->hitroll );
		break;

	  case MEDIT_DAMROLL:
		victim->damroll = URANGE( 0, atoi(arg), 65 );
		olc_log( d, "Changed damroll to %d", victim->damroll );
		break;

	  case MEDIT_DAMNUMDIE:
		olc_log( d, "Changed damnumdie to %d", victim->pIndexData->damnodice );
		break;

	  case MEDIT_DAMSIZEDIE:
		olc_log( d, "Changed damsizedie to %d", victim->pIndexData->damsizedice );
		break;

	  case MEDIT_DAMPLUS:
		olc_log( d, "Changed damplus to %d", victim->pIndexData->damplus );
		break;

	  case MEDIT_HITNUMDIE:
		olc_log( d, "Changed hitnumdie to %d", victim->pIndexData->hitnodice );
		break;

	  case MEDIT_HITSIZEDIE:
		olc_log( d, "Changed hitsizedie to %d", victim->pIndexData->hitsizedice );
		break;

	  case MEDIT_HITPLUS:
		olc_log( d, "Changed hitplus to %d", victim->pIndexData->hitplus );
		break;

	  case MEDIT_AC:
		victim->armor = URANGE( -300, atoi(arg), 300 );
		olc_log( d, "Changed armor to %d", victim->armor );
		break;

	  case MEDIT_GOLD:
		victim->gold = UMAX( 0, atoi(arg) );
		olc_log( d, "Changed gold to %d", victim->gold );
		break;

	  case MEDIT_POS:
		victim->position = URANGE( 0, atoi(arg), POSITION_STAND );
		olc_log( d, "Changed position to %d", victim->position );
		break;

	  case MEDIT_DEFAULT_POS:
		victim->defposition = URANGE( 0, atoi(arg), POSITION_STAND );
		olc_log( d, "Changed default position to %d", victim->defposition );
		break;

	  case MEDIT_MENTALSTATE:
		victim->mental_state = URANGE( -100, atoi(arg), 100 );
		olc_log( d, "Changed mental state to %d", victim->mental_state );
		break;

	  case MEDIT_EMOTIONAL:
		victim->emotional_state = URANGE( -100, atoi(arg), 100 );
		olc_log( d, "Changed emotional state to %d", victim->emotional_state );
		break;

	  case MEDIT_THIRST:
		victim->pg->condition[CONDITION_THIRST] = URANGE( 0, atoi(arg), 100 );
		olc_log( d, "Changed thirst to %d", victim->pg->condition[CONDITION_THIRST] );
		break;

	  case MEDIT_FULL:
		victim->pg->condition[CONDITION_FULL] = URANGE( 0, atoi(arg), 100 );
		olc_log( d, "Changed hunger to %d", victim->pg->condition[CONDITION_FULL] );
		break;

	  case MEDIT_DRUNK:
		victim->pg->condition[CONDITION_DRUNK] = URANGE( 0, atoi(arg), 100 );
		olc_log( d, "Changed drunkness to %d", victim->pg->condition[CONDITION_DRUNK] );
		break;

	  case MEDIT_SAVE_MENU:
		number = atoi( arg );
		switch ( number )
		{
		  default:
			send_to_char( d->character, "Invalid saving throw, try again: " );
			return;
		  case 0:
			break;
		  case 1:
			OLC_MODE(d) = MEDIT_SAV1;
			send_to_char( d->character, "\r\nEnter throw (-30 to 30): " );
			return;
		  case 2:
			OLC_MODE(d) = MEDIT_SAV2;
			send_to_char( d->character, "\r\nEnter throw (-30 to 30): " );
			return;
		  case 3:
			OLC_MODE(d) = MEDIT_SAV3;
			send_to_char( d->character, "\r\nEnter throw (-30 to 30): " );
			return;
		  case 4:
			OLC_MODE(d) = MEDIT_SAV4;
			send_to_char( d->character, "\r\nEnter throw (-30 to 30): " );
			return;
		  case 5:
			OLC_MODE(d) = MEDIT_SAV5;
			send_to_char( d->character, "\r\nEnter throw (-30 to 30): " );
			return;
		}
		/* If we reach here, we are going back to the main menu */
		break;

	  case MEDIT_CLASS:
		number = atoi( arg );
		if ( IS_MOB(victim) )
		{
			victim->class = URANGE( 0, number, MAX_CLASS-1 );
			break;
		}
		victim->class = URANGE( 0, number, MAX_CLASS-1 );
		olc_log( d, "Changed class to %s", get_class(victim, FALSE) );
		break;

	  case MEDIT_RACE:
		number = atoi( arg );
		if ( IS_MOB(victim ) )
		{
			victim->race = URANGE( 0, number, MAX_RACE-1 );
			break;
		}
		victim->race = URANGE( 0, number, MAX_RACE-1 );
		olc_log( d, "Changed race to %s", get_race(victim, FALSE) );
		break;

	  case MEDIT_PARTS:
		number = atoi( arg );
		if ( number == 0 )
			break;

		number--;
		if ( number < 0 || number >= MAX_PART )
		{
			send_to_char( d->character, "Invalid part, try again: " );
			return;
		}
		else
		{
			if ( number == 0 )
				break;
			else
			{
				number -= 1;
				TOGGLE_BIT( victim->xflags, number );
			}
		}
		olc_log( d, "%s the body part %s", HAS_BIT(victim->xflags, (number - 1))  ?  "Added"  :  "Removed",
			code_name(NULL, number, CODE_PART) );
		medit_disp_parts(d);
		return;

	  case MEDIT_ATTACK:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
				break;

			number--;	/* offset */
			if ( number < 0 || number >= MAX_ATTACK )
			{
				send_to_char( d->character, "Invalid flag, try again: " );
				return;
			}

			TOGGLE_BIT( victim->attacks, number );
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_ATTACK );
				if ( number < 0 )
				{
					send_to_char( d->character, "Invalid flag, try again: " );
					return;
				}

				TOGGLE_BIT( victim->attacks, number );
			}
		}
		medit_disp_attack_menu(d);
		olc_log( d, "%s the attack %s", HAS_BIT(victim->attacks,number)? "Added" : "Removed",
			code_name(NULL, number, CODE_ATTACK) );
		return;

	  case MEDIT_DEFENSE:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
				break;

			number--;	/* offset */
			if ( number < 0 || number >= MAX_DEFENSE )
			{
				send_to_char( d->character, "Invalid flag, try again: " );
				return;
			}
			else
				TOGGLE_BIT( victim->defenses, number );
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_DEFENSE );
				if ( number < 0 )
				{
					send_to_char( d->character, "Invalid flag, try again: " );
					return;
				}
				TOGGLE_BIT( victim->defenses, number );
			}
		}
		medit_disp_defense_menu( d );
		olc_log( d, "%s the defense %s", HAS_BIT(victim->defenses,number) ? "Added" : "Removed",
			code_name(NULL, number, CODE_DEFENSE) );
		return;

	  case MEDIT_LEVEL:
		victim->level = URANGE( 1, atoi(arg), LVL_LEGEND );
		olc_log( d, "Changed level to %d", victim->level );
		break;

	  case MEDIT_ALIGNMENT:
		victim->alignment = URANGE( -1000, atoi(arg), 1000 );
		olc_log( d, "Changed alignment to %d", victim->alignment );
		break;

	  case MEDIT_RESISTANT:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
				break;

			number--;
			if ( number < 0 || number >= MAX_RIS )
			{
				send_to_char( d->character, "Invalid flag, try again: " );
				return;
			}
			TOGGLE_BIT( victim->resistant, number );
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_RIS );
				if ( number < 0 )
				{
					send_to_char( d->character, "Invalid flag, try again: " );
					return;
				}
				TOGGLE_BIT( victim->resistant, number );
			}
		}
		medit_disp_ris(d);
		olc_log( d, "%s the resistant %s", HAS_BIT( victim->resistant, number ) ? "Added" : "Removed",
			code_name(NULL, number, CODE_RIS) );
		return;

	  case MEDIT_IMMUNE:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
				break;

			number--;
			if ( number < 0 || number >= MAX_RIS )
			{
				send_to_char( d->character, "Invalid flag, try again: " );
				return;
			}
			TOGGLE_BIT( victim->immune, number );
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_RIS );
				if ( number < 0 )
				{
					send_to_char( d->character, "Invalid flag, try again: " );
					return;
				}
				TOGGLE_BIT( victim->immune, number );
			}
		}
		medit_disp_ris(d);
		olc_log( d, "%s the immune %s", HAS_BIT( victim->immune, number ) ? "Added" : "Removed",
			code_name(NULL, number, CODE_RIS) );
		return;

	  case MEDIT_SUSCEPTIBLE:
		if ( is_number(arg) )
		{
			number = atoi( arg );
			if ( number == 0 )
				break;

			number--;
			if ( number < 0 || number >= MAX_RIS )
			{
				send_to_char( d->character, "Invalid flag, try again: " );
				return;
			}
			TOGGLE_BIT( victim->susceptible, number );
		}
		else
		{
			while ( VALID_STR(arg) )
			{
				arg = one_argument( arg, arg1 );
				number = code_num( NULL, arg1, CODE_RIS );
				if ( number < 0 )
				{
					send_to_char( d->character, "Invalid flag, try again: " );
					return;
				}
				TOGGLE_BIT( victim->susceptible, number );
			}
		}
		medit_disp_ris( d );
		olc_log( d, "%s the suscept %s", HAS_BIT(victim->susceptible, number) ? "Added" : "Removed",
			code_name(NULL, number, CODE_RIS) );
		return;

	  case MEDIT_CLAN:
		if ( get_trust(d->character) < TRUST_BUILDER )
			break;
		number = atoi( arg );
		if ( number < 0 || number > olc_top_clan+1 )
		{
			send_to_char( d->character, "Invalid choice, try again: " );
			return;
		}

		if ( number == 0 )
		{
			if ( !IS_ADMIN(victim) )
			{
				--victim->pg->clan->members;
				fwrite_clan( victim->pg->clan );
			}

			victim->pg->clan = NULL;
			break;
		}

		clan = get_clan( olc_clan_list[number-1], TRUE );
		if ( !clan )
		{
			send_log( NULL, LOG_OLC, "medit_parse: non-existant clan linked into olc_clan_list." );
			break;
		}

		if ( victim->pg->clan != NULL && !IS_ADMIN(victim) )
		{
			--victim->pg->clan->members;
			fwrite_clan( victim->pg->clan );
		}

		victim->pg->clan = clan;

		if ( !IS_ADMIN(victim) )
		{
			++victim->pg->clan->members;
			fwrite_clan( victim->pg->clan );
		}
		olc_log( d, "Clan changed to %s", clan->name );
		break;

/*-------------------------------------------------------------------*/
	  default:
		/*. We should never get here .*/
		send_log( NULL, LOG_OLC, "medit_parse: Reached default case!" );
		cleanup_olc(d);
		return;;
	}
/*-------------------------------------------------------------------*/
/*. END OF CASE
	If we get here, we have probably changed something, and now want to
	return to main menu.  Use OLC_CHANGE as a 'has changed' flag
*/
	OLC_CHANGE(d) = TRUE;
	medit_disp_menu(d);
}
/*. End of medit_parse() .*/
