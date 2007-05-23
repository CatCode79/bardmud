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


/*****************************************************************************\
 >							Shaddai's Polymorph								 <
\*****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "calendar.h"
#include "db.h"
#include "fread.h"
#include "interpret.h"
#include "morph.h"


/*
 * Variabili locali
 */
MORPH_DATA *first_morph = NULL;
MORPH_DATA *last_morph	= NULL;
int			top_morph	= 0;
VNUM		morph_vnum	= 1;		/* (TT) cambiato da 0 che era */


/*
 * Given the Morph's vnum, returns the pointer to the morph structure.
 */
MORPH_DATA *get_morph_vnum( VNUM vnum )
{
	MORPH_DATA *morph;

	if ( vnum < 1  )
		return NULL;

	for ( morph = first_morph;  morph;  morph = morph->next )
	{
		if ( morph->vnum == vnum )
			break;
	}

	return morph;
}


/*
 * Given the Morph's name, returns the pointer to the morph structure.
 */
MORPH_DATA *get_morph( char *arg )
{
	MORPH_DATA *morph;

	if ( !VALID_STR(arg) )
		return NULL;

	for ( morph = first_morph;  morph;  morph = morph->next )
	{
		if ( !str_cmp(morph->name, arg) )
			break;
	}

	return morph;
}


/*
 * Find an obj in player's inventory or wearing via a vnum
 */
OBJ_DATA *get_obj_vnum( CHAR_DATA *ch, VNUM vnum )
{
	OBJ_DATA *obj;

	for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if ( !can_see_obj(ch, obj, TRUE) )	continue;
		if ( obj->vnum != vnum )			continue;

		return obj;
	}

	return NULL;
}


/*
 * Find a morph you can use.
 */
MORPH_DATA *find_morph( CHAR_DATA *ch, char *target, bool is_cast)
{
	MORPH_DATA *morph;

	if ( !VALID_STR(target) )
		return NULL;

	for ( morph = first_morph;  morph;  morph = morph->next )
	{
		if ( str_cmp(morph->name, target) )
			continue;

		if ( can_morph(ch, morph, is_cast) )
			break;
	}

	return morph;
}


/*
 *  Shows morph info on a given morph.
 *  To see the description and help file, must use morphstat <morph> help
 */
DO_RET do_morphstat( CHAR_DATA *ch, char *argument )
{
	MORPH_DATA *morph;
	char		arg[MIL];
	char		buf[MSL];
	char		temp[MSL];
	int			count = 1,
				x;

	set_char_color( AT_CYAN, ch );

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Morphstat what?\r\n" );
		return;
	}

	if ( IS_MOB (ch) )
	{
		send_to_char( ch, "Mob's can't morphstat\r\n" );
		return;
	}

	if ( !str_cmp(arg, "list") || !str_cmp(arg, "lista") )
	{
		if ( !first_morph )
		{
			send_to_char( ch, "No morph's currently exist.\r\n" );
			return;
		}

		for ( morph = first_morph;  morph;  morph = morph->next )
		{
			pager_printf( ch, "&c[&C%2d&c]   Name:  &C%-13s	&cVnum:  &C%4d  &cUsed:  &C%3d\r\n",
				count, morph->name, morph->vnum, morph->used );
			count++;
		}

		return;
	}

	if ( !is_number( arg ) )
		morph = get_morph( arg );
	else
		morph = get_morph_vnum( atoi(arg) );

	if ( morph == NULL )
	{
		send_to_char( ch, "No such morph exists.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		pager_printf ( ch, "  &cMorph Name: &C%-20s  Vnum: %4d\r\n", morph->name, morph->vnum );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		send_to_pager( ch, "						   &BMorph Restrictions\r\n" );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		pager_printf ( ch, "  &cClassi Allowed   : &w%s\r\n", code_bit(NULL, morph->classes, CODE_CLASS) );
		pager_printf ( ch, "  &cRaces Not Allowed: &w%s\r\n", code_bit(NULL, morph->races, CODE_RACE) );
		pager_printf ( ch, "  &cSex:  &C%s   &cTime From:   &C%d   &cTime To:	&C%d\r\n",
			(morph->sex == SEX_FEMALE)  ?  "femmina"  :  "maschio",
			morph->timefrom, morph->timeto );
		pager_printf ( ch, "  &cDay From:  &C%d  &cDay To:  &C%d\r\n", morph->dayfrom, morph->dayto );
		pager_printf ( ch, "  &cLevel:  &C%d	   &cCasting Allowed   : &C%s\r\n", morph->level, (morph->no_cast)  ?  "NO"  :  "yes" );
		pager_printf ( ch, "  &cUSAGES:  Mana:  &C%d  &cMove:  &C%d  &cLife:  &C%d  Soul:  &C%d\r\n",
			morph->points_used[POINTS_MANA], morph->points_used[POINTS_MOVE], morph->points_used[POINTS_LIFE], morph->points_used[POINTS_SOUL] );
		pager_printf ( ch, "  &cGlory:  &C%d\r\n", morph->gloryused );
		pager_printf ( ch, "  &cObj1: &C%d  &cObjuse1: &C%s   &cObj2: &C%d  &cObjuse2: &C%s   &cObj3: &C%d  &cObjuse3: &c%s\r\n",
			morph->obj[0], (morph->objuse[0])  ?  "SI"  :  "no",
			morph->obj[1], (morph->objuse[1])  ?  "SI"  :  "no",
			morph->obj[2], (morph->objuse[2])  ?  "SI"  :  "no" );
		pager_printf ( ch, "  &cTimer: &w%d\r\n", morph->timer );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		send_to_pager( ch, "					   &BEnhancements to the Player\r\n" );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );

		pager_printf ( ch, "  &cStr: &C%2d&c )( Res: &C%2d&c )( Con: &C%2d&c )\r\n",
			morph->attr[ATTR_STR], morph->attr[ATTR_RES], morph->attr[ATTR_CON] );
		pager_printf ( ch, "  ( Int: &C%2d&c )( Coc: &C%2d&c )( Wil: &C%2d&c )\r\n",
			morph->attr[ATTR_INT], morph->attr[ATTR_COC], morph->attr[ATTR_WIL] );
		pager_printf ( ch, "  ( Agi: &C%2d&c )( Ref: &C%2d&c )( Spe: &C%2d&c )\r\n",
			morph->attr[ATTR_AGI], morph->attr[ATTR_REF], morph->attr[ATTR_SPE] );
		pager_printf ( ch, "  ( Spi: &C%2d&c )( Mig: &C%2d&c )( Abs: &C%2d&c )\r\n",
			morph->attr[ATTR_SPI], morph->attr[ATTR_MIG], morph->attr[ATTR_ABS] );
		pager_printf ( ch, "  ( Emp: &C%2d&c )( Bea: &C%2d&c )( Lea: &C%2d&c )\r\n",
			morph->attr[ATTR_EMP], morph->attr[ATTR_BEA], morph->attr[ATTR_LEA] );

		buf[0] = '\0';
		for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		{
			sprintf( temp, "%d ", morph->saving_throw[x] );
			strcat( buf, temp );
		}

		pager_printf ( ch, "  &cSave versus: &w%s\r\n", buf );
		pager_printf ( ch, "  &cDodge: &w%d  &cParry: &w%d  &cTumble: &w%d\r\n", morph->dodge, morph->parry, morph->tumble );
		pager_printf ( ch, "  &cLife	 : &w%s	&cMana   : &w%s	&cMove	  : &w%s\r\n", morph->points[POINTS_LIFE], morph->points[POINTS_MANA], morph->points[POINTS_MOVE] );	/* (RR) e soul? */
		pager_printf ( ch, "  &cDamroll : &w%s	&cHitroll: &w%s	&cAC	 : &w%d\r\n", morph->damroll, morph->hitroll, morph->ac );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		send_to_pager( ch, "						  &BAffects to the Player\r\n" );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		pager_printf ( ch, "  &cAffected by: &C%s\r\n", code_bit(NULL, morph->affected_by, CODE_AFFECT) );
		pager_printf ( ch, "  &cImmune	 : &w%s\r\n", code_bit(NULL, morph->immune, CODE_RIS) );
		pager_printf ( ch, "  &cSusceptible: &w%s\r\n", code_bit(NULL, morph->suscept, CODE_RIS) );
		pager_printf ( ch, "  &cResistant  : &w%s\r\n", code_bit(NULL, morph->resistant, CODE_RIS) );
		pager_printf ( ch, "  &cSkills	 : &w%s\r\n", morph->skills );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		send_to_pager( ch, "					 &BPrevented affects to the Player\r\n" );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		pager_printf ( ch, "  &cNot affected by: &C%s\r\n", code_bit(NULL, morph->no_affected_by, CODE_AFFECT) );
		pager_printf ( ch, "  &cNot Immune	 : &w%s\r\n", code_bit(NULL, morph->no_immune, CODE_RIS) );
		pager_printf ( ch, "  &cNot Susceptible: &w%s\r\n", code_bit(NULL, morph->no_suscept, CODE_RIS) );
		pager_printf ( ch, "  &cNot Resistant  : &w%s\r\n", code_bit(NULL, morph->no_resistant, CODE_RIS) );
		pager_printf ( ch, "  &cNot Skills	 : &w%s\r\n", morph->no_skills );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n\r\n" );

		return;
	}

	if ( !str_cmp(argument, "aiuto") || !str_cmp(argument, "help") || !str_cmp(argument, "desc") )
	{
		pager_printf ( ch, "  &cMorph Name  : &C%-20s\r\n", morph->name );
		pager_printf ( ch, "  &cDefault Pos : &w%d\r\n",	morph->defpos );
		pager_printf ( ch, "  &cKeywords	: &w%s\r\n",	morph->key_words );
		pager_printf ( ch, "  &cShortdesc   : &w%s\r\n",	(!VALID_STR(morph->short_desc))  ?  "(none set)"  :  morph->short_desc );
		pager_printf ( ch, "  &cLongdesc	: &w%s",		(!VALID_STR(morph->long_desc))  ?  "(none set)\r\n"  :  morph->long_desc );
		pager_printf ( ch, "  &cMorphself   : &w%s\r\n",	morph->morph_self );
		pager_printf ( ch, "  &cMorphother  : &w%s\r\n",	morph->morph_other );
		pager_printf ( ch, "  &cUnMorphself : &w%s\r\n",	morph->unmorph_self );
		pager_printf ( ch, "  &cUnMorphother: &w%s\r\n",	morph->unmorph_other );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		pager_printf ( ch, "								  &cHelp:\r\n&w%s\r\n", morph->help );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );
		pager_printf ( ch, "                               &cDescription:\r\n&w%s\r\n", morph->description );
		send_to_pager( ch, "&B[----------------------------------------------------------------------------]\r\n" );

		return ;
	}

	send_to_char( ch, "&YSintassi&w: morphstat <morph>\r\n" );
	send_to_char( ch, "&YSintassi&w: morphstat <morph> <help/desc>\r\n" );
}


/*
 * These functions wrap the sending of morph stuff, with morphing the code.
 * In most cases this is what should be called in the code when using spells,
 * obj morphing, etc...
 */
int do_morph_char( CHAR_DATA *ch, MORPH_DATA *morph )
{
	bool		canmorph = TRUE;
	OBJ_DATA   *obj;
	OBJ_DATA   *tmpobj;
	int			x;

	if ( ch->morph )
		canmorph = FALSE;

	for ( x = 0;  x < MAX_MORPH_OBJ;  x++ )
	{
		if ( morph->obj[x] )
		{
			if ( (obj = get_obj_vnum(ch, morph->obj[x])) == NULL )
				canmorph = FALSE;
			else if ( morph->objuse[x] )
			{
				act( AT_OBJECT, "$p disappears in a whisp of smoke!", obj->carried_by, obj, NULL, TO_CHAR );
				if ( obj == get_eq_char(obj->carried_by, WEARLOC_WIELD)
				  && (tmpobj=get_eq_char(obj->carried_by, WEARLOC_DUAL)) != NULL )
				{
					tmpobj->wear_loc = WEARLOC_WIELD;
				}
				split_obj( obj, 1 );
				free_object( obj );
			}
		}
	}

	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		if ( morph->points_used[x] )
		{
			if ( ch->points[x] < morph->points_used[x] )
				canmorph = FALSE;
			else
				ch->points[x] -= morph->points_used[x];
		}
	}

	if ( morph->gloryused )
	{
		if ( IS_PG(ch) && ch->pg->glory < morph->gloryused )
			canmorph = FALSE;
		else
			ch->pg->glory -= morph->gloryused;
	}

	if ( !canmorph )
	{
		send_to_char( ch, "You begin to transform, but something goes wrong.\r\n" );
		return FALSE;
	}
	send_morph_message ( ch, morph, TRUE );
	do_morph( ch, morph );

	return TRUE;
}


DO_RET do_unmorph_char( CHAR_DATA *ch )
{
	MORPH_DATA *temp;

	if ( !ch->morph )
		return;

	temp = ch->morph->morph;
	do_unmorph( ch );
	send_morph_message( ch, temp, FALSE );
}


/*
 * This function sends the morph/unmorph message to the people in the room.
 */
void send_morph_message( CHAR_DATA *ch, MORPH_DATA *morph, bool is_morph )
{
	if ( morph == NULL )
		return;

	if ( is_morph )
	{
		act( AT_MORPH, morph->morph_other, ch, NULL, NULL, TO_ROOM );
		act( AT_MORPH, morph->morph_self, ch, NULL, NULL, TO_CHAR );
	}
	else
	{
		act( AT_MORPH, morph->unmorph_other, ch, NULL, NULL, TO_ROOM );
		act( AT_MORPH, morph->unmorph_self, ch, NULL, NULL, TO_CHAR );
	}
}


/*
 * Checks to see if the char meets all the requirements to morph into
 * the provided morph.  Doesn't Look at NPC's class or race as they
 * are different from pc's, but still checks level and if they can
 * cast it or not.
 */
bool can_morph( CHAR_DATA *ch, MORPH_DATA *morph, bool is_cast )
{
	if ( !morph )
		return FALSE;

	/* Let administrators morph to anything. Also NPC can do any morph */
	if ( IS_ADMIN(ch) || IS_MOB(ch) )
		return TRUE;

	if ( morph->no_cast && is_cast )									return FALSE;
	if ( get_level(ch) < morph->level )									return FALSE;
	if ( morph->sex != -1 && morph->sex != ch->sex )					return FALSE;
	if ( morph->classes != 0 && !HAS_BIT(morph->classes, ch->class) )	return FALSE;
	if ( morph->races != 0 && HAS_BIT(morph->races, ch->race) )			return FALSE;
	if ( morph->timeto != -1 && morph->timefrom != -1 )
	{
		int 	tmp,
				x;
		bool	found = FALSE;
		/* x is a sanity check, just in case things go haywire so it doesn't
		 * loop forever here */
		for ( x = 0, tmp = morph->timefrom;  x < 25 && tmp != morph->timeto;  x++ )
		{
			if ( tmp == calendar.hour )
			{
				found = TRUE;
				break;
			}
			if ( tmp == 23 )
				tmp = 0;
			else
				tmp++;
		}
		if ( !found )
			return FALSE;
	}

	if ( morph->dayfrom != -1 && morph->dayto != -1
	  && (morph->dayto < (calendar.day+1) || morph->dayfrom > (calendar.day+1)) )
	{
		return FALSE;
	}

	return TRUE;
}


/*
 * Workhorse of the polymorph code, this turns the player into the proper
 * morph.  Doesn't check if you can morph or not so must use can_morph before
 * this is called.  That is so we can let people morph into things without
 * checking. Also, if anything is changed here, make sure it gets changed
 * in do_unmorph otherwise you will be giving your player stats for free.
 * This also does not send the message to people when you morph good to
 * use in save functions.
 */
DO_RET do_morph( CHAR_DATA *ch, MORPH_DATA *morph )
{
	CHAR_MORPH *ch_morph;
	int			x;

	if ( !morph )
		return;

	if ( ch->morph )
		do_unmorph_char( ch );

	ch_morph					 = make_char_morph ( morph );
	ch->armor					+= morph->ac;

	for ( x = 0;  x < MAX_ATTR;  x++ )
		ch->attr_mod[x]			+= morph->attr[x];

	for ( x = 0;  x < MAX_SENSE;  x++ )
		ch->sense_mod[x]		+= morph->sense[x];

	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		ch->saving_throw[x]		+= morph->saving_throw[x];

	ch_morph->hitroll			 = dice_parse( ch, morph->level/2, morph->hitroll );
	ch->hitroll					+= ch_morph->hitroll;
	ch_morph->damroll			 = dice_parse( ch, morph->level/2, morph->damroll );
	ch->damroll					+= ch_morph->damroll;

	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		ch_morph->points[x] = dice_parse( ch, morph->level/2, morph->points[x] );

		if ( ch->points_max[x] + ch_morph->points[x] > 32700 )
			ch_morph->points[x] = ( 32700 - ch->points_max[x] );
		ch->points_max[x] += ch_morph->points[x];

		if ( ch->points[x] > ch->points_max[x] )
			ch->points[x] = ch->points_max[x];
	}

	SET_BITS( ch->affected_by,	morph->affected_by );
	SET_BITS( ch->immune,		morph->immune );
	SET_BITS( ch->resistant,	morph->resistant );
	SET_BITS( ch->susceptible,	morph->suscept );

	REMOVE_BITS( ch->affected_by,	morph->no_affected_by );
	REMOVE_BITS( ch->immune,		morph->no_immune );
	REMOVE_BITS( ch->resistant,		morph->no_resistant );
	REMOVE_BITS( ch->susceptible,	morph->no_suscept );

	ch->morph = ch_morph;
	morph->used++;
}


/*
 * This makes sure to take all the affects given to the player by the morph
 * away.  Several things to keep in mind.  If you add something here make
 * sure you add it to do_morph as well (Unless you want to charge your players
 * for morphing ;) ).  Also make sure that their pfile saves with the morph
 * affects off, as the morph may not exist when they log back in.  This
 * function also does not send the message to people when you morph so it is
 * good to use in save functions and other places you don't want people to
 * see the stuff.
 */
DO_RET do_unmorph ( CHAR_DATA *ch )
{
	CHAR_MORPH *morph;
	int			x;

	if ( (morph = ch->morph) == NULL )
		return;

	ch->armor					-= morph->ac;

	for ( x = 0;  x < MAX_ATTR;  x++ )
		ch->attr_mod[x]			-= morph->attr[x];

	for ( x = 0;  x < MAX_SENSE;  x++ )
		ch->sense_mod[x]		-= morph->sense[x];

	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		ch->saving_throw[x]		-= morph->saving_throw[x];

	ch->hitroll					-= morph->hitroll;
	ch->damroll					-= morph->damroll;

	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		ch->points_max[x] -= morph->points[x];
		if ( ch->points[x] > ch->points_max[x] )
			ch->points[x] = ch->points_max[x];
	}

	REMOVE_BITS( ch->affected_by,	morph->affected_by );
	REMOVE_BITS( ch->immune,		morph->immune );
	REMOVE_BITS( ch->resistant,		morph->resistant );
	REMOVE_BITS( ch->susceptible,	morph->suscept );

	free_char_morph( morph );
	ch->morph = NULL;
	update_aris( ch );
}


/*
 * Molto semplice, ma meglio tenerla separata dal resto per future
 *	implementazioni e per evitare complicazioni.
 */
void free_char_morph( CHAR_MORPH *morph )
{
	DISPOSE( morph );
}


/*
 * Libera dalla memoria tutti i morph
 */
void free_all_morphs( void )
{
	MORPH_DATA *morph;

	for ( morph = first_morph;  morph;  morph = first_morph )
	{
		int	x;

		UNLINK( morph, first_morph, last_morph, next, prev );
		top_morph--;

		DISPOSE( morph->damroll		  );
		DISPOSE( morph->help		  );
		DISPOSE( morph->hitroll		  );
		DISPOSE( morph->key_words	  );
		DISPOSE( morph->long_desc	  );
		DISPOSE( morph->description	  );
		DISPOSE( morph->morph_other	  );
		DISPOSE( morph->morph_self	  );
		DISPOSE( morph->name		  );
		DISPOSE( morph->short_desc	  );
		DISPOSE( morph->skills		  );
		DISPOSE( morph->no_skills	  );
		DISPOSE( morph->unmorph_other );
		DISPOSE( morph->unmorph_self  );
		destroybv( morph->affected_by );
		destroybv( morph->classes );
		destroybv( morph->no_affected_by );
		destroybv( morph->no_immune );
		destroybv( morph->no_resistant );
		destroybv( morph->no_suscept );
		destroybv( morph->immune );
		destroybv( morph->resistant );
		destroybv( morph->suscept );
		destroybv( morph->races );
		for ( x = 0;  x < MAX_POINTS;  x++ )
			DISPOSE( morph->points[x] );
		DISPOSE( morph );
	}

	if ( top_morph != 0 )
		send_log( NULL, LOG_BUG, "free_all_morphs: top_morph diverso da 0 dopo aver liberato tutti i morph: %d", top_morph );
}


/*
 * This function set's up all the default values for a morph
 */
void morph_defaults( MORPH_DATA *morph )
{
	int		x;

	morph->damroll				= str_dup( "" );
	morph->help					= str_dup( "" );
	morph->hitroll				= str_dup( "" );
	morph->key_words			= str_dup( "" );
	morph->long_desc			= str_dup( "" );
	morph->description 			= str_dup( "" );
	morph->morph_other			= str_dup( "" );
	morph->morph_self			= str_dup( "" );
	morph->name					= str_dup( "" );
	morph->short_desc			= str_dup( "" );
	morph->skills				= str_dup( "" );
	morph->no_skills			= str_dup( "" );
	morph->unmorph_other		= str_dup( "" );
	morph->unmorph_self			= str_dup( "" );

	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		morph->points[x]		= str_dup ( "" );
		morph->points_used[x]	=  0;
	}

	morph->classes				=  0;
	morph->sex					= -1;
	morph->timefrom				= -1;
	morph->timeto				= -1;
	morph->dayfrom				= -1;
	morph->dayto				= -1;
	morph->gloryused			=  0;
	CLEAR_BITS( morph->affected_by );
	CLEAR_BITS( morph->immune );
	CLEAR_BITS( morph->resistant );
	CLEAR_BITS( morph->suscept );
	CLEAR_BITS( morph->no_affected_by );
	CLEAR_BITS( morph->no_immune );
	CLEAR_BITS( morph->no_resistant );
	CLEAR_BITS( morph->no_suscept );

	for ( x = 0;  x < MAX_MORPH_OBJ;  x++ )
	{
		morph->obj[x]			=  0;
		morph->objuse[x]		= FALSE;
	}

	morph->races				=  0;
	CLEAR_BITS( morph->resistant );
	CLEAR_BITS( morph->suscept );
	morph->used					=  0;
	morph->ac					=  0;
	morph->defpos				= POSITION_STAND;
	for ( x = 0;  x < MAX_ATTR;  x++ )
		morph->attr[x]			=  0;
	for ( x = 0;  x < MAX_SENSE;  x++ )
		morph->sense[x]			=  0;
	morph->dodge				=  0;
	morph->level				=  0;
	morph->parry				=  0;

	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		morph->saving_throw[x]	=  0;

	morph->tumble				=  0;
	morph->no_cast				= FALSE;
	morph->timer				= -1;
	morph->vnum					=  0;
}


/*
 * Questa funzione carica i dati dei Morph.  Note that this function MUST be
 * Essa deve venire chiamata dopo aver caricato le razze e le classi.
 */
void fread_morph( MUD_FILE *fp )
{
	MORPH_DATA	*morph = NULL;

	CREATE ( morph, MORPH_DATA, 1 );
	morph_defaults( morph );

	for ( ; ; )
	{
		char	*word;
		int		 x;

		word = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'			  )								fread_to_eol( fp );
		else if	( !strcmp(word, "Armor"		) )		morph->ac		   =	fread_number( fp );
		else if	( !strcmp(word, "Affected"	) )		morph->affected_by =	fread_code_bit( fp, CODE_AFFECT );
		else if ( !strcmp(word, "Attribute" ) )
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
				morph->attr[x] =											fread_number( fp );
		}
		else if ( !strcmp(word, "Class") )			morph->classes =		fread_code_bit( fp, CODE_CLASS );
		else if	( !strcmp(word, "Damroll"	 ) )
		{
			if ( morph->damroll )
				DISPOSE( morph->damroll );
			morph->damroll = fread_string( fp );
		}
		else if	( !strcmp(word, "DayFrom"	 ) )	morph->dayfrom		  =	fread_number( fp );
		else if	( !strcmp(word, "DayTo"		 ) )	morph->dayto		  =	fread_number( fp );
		else if	( !strcmp(word, "Defpos"	 ) )	morph->defpos		  =	fread_number( fp );
		else if	( !strcmp(word, "Description") )
		{
			if ( morph->description )
				DISPOSE( morph->description );
			morph->description =											fread_string( fp );
		}
		else if	( !strcmp(word, "Dodge"		 ) )	morph->dodge		  =	fread_number( fp );
		else if ( !strcmp(word, "End"		 ) )	break;
/*		else if	( !strcmp(word, "GloryUsed"  ) )	morph->gloryused	  =	fread_number( fp ); */
		else if	( !strcmp(word, "Help"		 ) )
		{
			if ( morph->help )
				DISPOSE( morph->help );
			morph->help = fread_string( fp );
		}
		else if	( !strcmp(word, "Hitroll"	 ) )
		{
			if ( morph->hitroll )
				DISPOSE( morph->hitroll );
			morph->hitroll = fread_string( fp );
		}
		else if	( !strcmp(word, "Immune"	 ) )	morph->immune		  =	fread_code_bit( fp, CODE_RIS );
		else if	( !strcmp(word, "Keywords"	 ) )
		{
			if ( morph->key_words )
				DISPOSE( morph->key_words );
			morph->key_words = fread_string( fp );
		}
		else if	( !strcmp(word, "Level"		 ) )	morph->level		  =	fread_number( fp );
		else if	( !strcmp(word, "Longdesc"	 ) )
		{
			if ( morph->long_desc )
				DISPOSE( morph->long_desc );
			morph->long_desc	  =	fread_string( fp );
		}
		else if	( !strcmp(word, "MorphOther" ) )
		{
			if ( morph->morph_other )
				DISPOSE( morph->morph_other );
			morph->morph_other = fread_string( fp );
		}
		else if	( !strcmp(word, "MorphSelf"  ) )
		{
			if ( morph->morph_self )
				DISPOSE( morph->morph_self );
			morph->morph_self = fread_string( fp );
		}
		else if	( !strcmp(word, "Name"		 ) )	morph->name			  =	fread_string( fp );
		else if	( !strcmp(word, "NoAffected" ) )	morph->no_affected_by =	fread_code_bit( fp, CODE_AFFECT );
		else if	( !strcmp(word, "NoImmune"	 ) )	morph->no_immune	  =	fread_code_bit( fp, CODE_RIS );
		else if	( !strcmp(word, "NoResistant") )	morph->no_resistant	  =	fread_code_bit( fp, CODE_RIS );
		else if	( !strcmp(word, "NoSkills"	 ) )
		{
			if ( morph->no_skills )
				DISPOSE( morph->no_skills );
			morph->no_skills =												fread_string( fp );
		}
		else if	( !strcmp(word, "NoSuscept"	 ) )	morph->no_suscept	  =	fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "NoCast"	 ) )	morph->no_cast		  =	fread_number( fp );
		else if ( !strcmp(word, "Objs"		 ) )
		{
			for ( x = 0;  x < MAX_MORPH_OBJ;  x++ )
				morph->obj[x] =												fread_number( fp );
		}
		else if ( !strcmp(word, "ObjUse") )
		{
			for ( x = 0;  x < MAX_MORPH_OBJ;  x++ )
				morph->objuse[x] =											fread_number( fp );
		}
		else if ( !strcmp(word, "Point") )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
				morph->points[x] =											str_dup( fread_word(fp) );
		}
		else if ( !strcmp(word, "PointUsed") )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
				morph->points_used[x] =										fread_number( fp );
		}
		else if ( !strcmp(word, "Parry") )			morph->parry =			fread_number( fp );
		else if ( !strcmp(word, "Race" ) )			morph->races =			fread_code_bit( fp, CODE_RACE );
		else if ( !strcmp(word, "Resistant"	  ) )	morph->resistant =		fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "SavingThrow" ) )
		{
			for ( x = 0;  x < MAX_SAVETHROW;  x++ )
				morph->saving_throw[x] =									fread_number( fp );
		}
		else if ( !strcmp(word, "Sense") )
		{
			for ( x = 0;  x < MAX_SENSE;  x++ )
				morph->sense[x] =											fread_number( fp );
		}
		else if ( !strcmp(word, "Sex"		  ) )	morph->sex			 =	fread_number( fp );
		else if ( !strcmp(word, "ShortDesc"	  ) )
		{
			if ( morph->short_desc )
				DISPOSE( morph->short_desc );
			morph->short_desc =	fread_string( fp );
		}
		else if ( !strcmp(word, "Skills"	  ) )
		{
			if ( morph->skills )
				DISPOSE( morph->skills );
			morph->skills		 =	fread_string( fp );
		}
		else if ( !strcmp(word, "Suscept"	  ) )	morph->suscept		 =	fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "TimeFrom"	  ) )	morph->timefrom		 =	fread_number( fp );
		else if ( !strcmp(word, "Timer"		  ) )	morph->timer		 =	fread_number( fp );
		else if ( !strcmp(word, "TimeTo"	  ) )	morph->timeto		 =	fread_number( fp );
		else if ( !strcmp(word, "Tumble"	  ) )	morph->tumble		 =	fread_number( fp );
		else if ( !strcmp(word, "UnmorphOther") )
		{
			if ( morph->unmorph_other )
				DISPOSE( morph->unmorph_other );
			morph->unmorph_other =	fread_string( fp );
		}
		else if ( !strcmp(word, "UnmorphSelf" ) )
		{
			if ( morph->unmorph_self )
				DISPOSE( morph->unmorph_self );
			morph->unmorph_self  =	fread_string( fp );
		}
		else if ( !strcmp(word, "Used"		  ) )	morph->used			 =	fread_number( fp );
		else if ( !strcmp(word, "Vnum"		  ) )	morph->vnum			 =	fread_number( fp );
		else
		{
			send_log( fp, LOG_FREAD, "fread_morph: no match: %s", word );
			exit( EXIT_FAILURE );
		}
	} /* chiude il for */

	LINK( morph, first_morph, last_morph, next, prev );
	top_morph++;
}


void setup_morph_vnum( void )
{
	MORPH_DATA *morph;
	VNUM	   vnum;

	vnum = morph_vnum;

	for ( morph = first_morph;  morph;  morph = morph->next )
	{
		if ( morph->vnum > vnum )
			vnum = morph->vnum;
	}
	if ( vnum < 1000 )
		vnum = 1000;
	else
		vnum++;

	for ( morph = first_morph;  morph;  morph = morph->next )
	{
		if ( morph->vnum == 1 )	/* (TT) cambiato da 0 che era */
		{
			morph->vnum = vnum;
			vnum++;
		}
	}

	morph_vnum = vnum;
}


/*
 * Carica tutti i Morph
 */
void load_morphs( void )
{
	fread_list( MORPH_LIST, "MORPH", fread_morph, TRUE );
	setup_morph_vnum( );
}


/*
 * Create new player morph, a scailed down version of original morph
 * so if morph gets changed stats don't get messed up.
 */
CHAR_MORPH *make_char_morph ( MORPH_DATA *morph )
{
	CHAR_MORPH *ch_morph;
	int			x;

	if ( morph == NULL )
		return NULL;

	CREATE ( ch_morph, CHAR_MORPH, 1);
	ch_morph->morph					= morph;
	ch_morph->ac					= morph->ac;
	for ( x = 0;  x < MAX_ATTR;  x++ )
		ch_morph->attr[x]			= morph->attr[x];
	for ( x = 0;  x < MAX_SENSE;  x++ )
		ch_morph->sense[x]			= morph->sense[x];
	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		ch_morph->saving_throw[x]	= morph->saving_throw[x];
	ch_morph->timer					= morph->timer;
	ch_morph->hitroll				= 0;
	ch_morph->damroll				= 0;
	for ( x = 0;  x < MAX_POINTS;  x++ )
		ch_morph->points[x]			= 0;
	SET_BITS( ch_morph->affected_by, morph->affected_by );
	SET_BITS( ch_morph->immune, morph->immune );
	SET_BITS( ch_morph->resistant, morph->resistant );
	SET_BITS( ch_morph->suscept, morph->suscept );
	SET_BITS( ch_morph->no_affected_by, morph->no_affected_by );
	SET_BITS( ch_morph->no_immune, morph->no_immune );
	SET_BITS( ch_morph->no_resistant, morph->no_resistant );
	SET_BITS( ch_morph->no_suscept, morph->no_suscept );

	return ch_morph;
}


void fwrite_morph_data( CHAR_DATA *ch, MUD_FILE *fp )
{
	CHAR_MORPH *morph;
	char		buf[MSL];
	char		temp[MSL];
	int			x;

	if ( ch->morph == NULL )
		return;

	morph = ch->morph;

	fprintf( fp->file, "#MorphData\n" );

	/* Only Print Out what is necessary */
	if ( morph->morph != NULL )
	{
		fprintf( fp->file, "Vnum          %d\n",	morph->morph->vnum );
		fprintf( fp->file, "Name          %s~\n\n",	morph->morph->name );
	}

	fprintf( fp->file, "Affect        %s\n",	code_bit(fp, morph->affected_by, CODE_AFFECT) );
	fprintf( fp->file, "Armor         %d\n",	morph->ac );
	buf[0] = '\0';
	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		sprintf( temp, "%d ", morph->attr[x] );
		strcat( buf, temp );
	}
	fprintf( fp->file, "Attribute     %s\n",	buf );
	fprintf( fp->file, "Damroll       %d\n",	morph->damroll );
	fprintf( fp->file, "Dodge         %d\n",	morph->dodge );
	fprintf( fp->file, "Hitroll       %d\n",	morph->hitroll );
	fprintf( fp->file, "Immune        %s\n",	code_bit(fp, morph->immune, CODE_RIS) );
	fprintf( fp->file, "NoAffect      %s\n",	code_bit(fp, morph->no_affected_by, CODE_AFFECT) );
	fprintf( fp->file, "NoImmune      %s\n",	code_bit(fp, morph->no_immune, CODE_RIS) );
	fprintf( fp->file, "NoResistant   %s\n",	code_bit(fp, morph->no_resistant, CODE_RIS) );
	fprintf( fp->file, "NoSuscept     %s\n",	code_bit(fp, morph->no_suscept, CODE_RIS) );
	fprintf( fp->file, "Parry         %d\n",	morph->parry );
	buf[0] = '\0';
	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		sprintf( temp, "%d ", morph->points[x] );
		strcat( buf, temp );
	}
	fprintf( fp->file, "Point         %s\n",	buf );
	fprintf( fp->file, "Resistant     %s\n",	code_bit(fp, morph->resistant, CODE_RIS) );
	fprintf( fp->file, "Suscept       %s\n",	code_bit(fp, morph->suscept, CODE_RIS) );

	buf[0] = '\0';
	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
	{
		sprintf( temp, "%d ", morph->saving_throw[x] );
		strcat( buf, temp );
	}
	fprintf( fp->file, "SaveElements  %s\n",	buf );

	buf[0] = '\0';
	for ( x = 0;  x < MAX_SENSE;  x++ )
	{
		sprintf( temp, "%d ", morph->sense[x] );
		strcat( buf, temp );
	}

	fprintf( fp->file, "Sense         %s\n",	buf );
	fprintf( fp->file, "Timer         %d\n",	morph->timer );
	fprintf( fp->file, "Tumble        %d\n",	morph->tumble );
	fprintf( fp->file, "End\n" );
}


void clear_char_morph( CHAR_MORPH *morph )
{
	int		x;

	morph->timer				= -1;
	CLEAR_BITS( morph->affected_by );
	CLEAR_BITS( morph->no_affected_by );
	CLEAR_BITS( morph->immune );
	CLEAR_BITS( morph->no_immune );
	CLEAR_BITS( morph->no_resistant );
	CLEAR_BITS( morph->no_suscept );
	CLEAR_BITS( morph->resistant );
	CLEAR_BITS( morph->suscept );
	morph->ac					= 0;
	for ( x = 0;  x < MAX_ATTR;  x++ )
		morph->attr[x]			= 0;
	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		morph->saving_throw[x]	= 0;
	morph->damroll				= 0;
	morph->dodge				= 0;
	for (x = 0; x < MAX_POINTS;  x++ )
		morph->points[x]			= 0;
	morph->hitroll				= 0;
	morph->parry				= 0;
	morph->tumble				= 0;
	morph->morph				= NULL;
}


void fread_morph_data( CHAR_DATA *ch, MUD_FILE *fp )
{
	CHAR_MORPH *morph;
	char	   *word;
	int			x;

	CREATE( morph, CHAR_MORPH, 1);
	clear_char_morph( morph );
	ch->morph = morph;

	for ( ; ; )
	{
		word = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'			  )								fread_to_eol( fp );
		else if ( !strcmp(word, "Affect"	) )		morph->affected_by =	fread_code_bit( fp, CODE_AFFECT );
		else if ( !strcmp(word, "Armor"		) )		morph->ac		   =	fread_number( fp );
		else if ( !strcmp(word, "Attribute" ) )
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
				morph->attr[x] =											fread_number( fp );
		}
		else if ( !strcmp(word, "Damroll") )		morph->damroll =		fread_number( fp );
		else if ( !strcmp(word, "Dodge"	 ) )		morph->dodge   =		fread_number( fp );
		else if ( !strcmp(word, "End"	 ) )								return;
		else if ( !strcmp(word, "Hitroll") )		morph->hitroll =		fread_number( fp );
		else if ( !strcmp(word, "Immune" ) )		morph->immune  =		fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "Name"	 ) )
		{
			if ( morph->morph )
			{
				if ( str_cmp(morph->morph->name, fread_string(fp)) )
					send_log( fp, LOG_FREAD, "fread_morph_data: morph name doesn't match vnum %d.", morph->morph->vnum );
			}
		}
		else if ( !strcmp(word, "NoAffect"	 ) )	morph->no_affected_by =	fread_code_bit( fp, CODE_AFFECT );
		else if ( !strcmp(word, "NoImmune"	 ) )	morph->no_immune	  =	fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "NoResistant") )	morph->no_resistant   =	fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "NoSuscept"	 ) )	morph->no_suscept	  =	fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "Parry"		 ) )	morph->parry		  =	fread_number( fp );
		else if ( !strcmp(word, "Point"		 ) )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
				morph->points[x] =											fread_number( fp );
		}
		else if ( !strcmp(word, "Resistant"	  ) )	morph->resistant =		fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "SavingThrow" ) )
		{
			for ( x = 0;  x < MAX_SAVETHROW;  x++ )
				morph->saving_throw[x] =									fread_number( fp );
		}
		else if ( !strcmp(word, "Sense") )
		{
			for ( x = 0;  x < MAX_SENSE;  x++ )
				morph->sense[x] =											fread_number( fp );
		}
		else if ( !strcmp(word, "Suscept") )		morph->suscept =		fread_code_bit( fp, CODE_RIS );
		else if ( !strcmp(word, "Timer"	 ) )		morph->timer   =		fread_number( fp );
		else if ( !strcmp(word, "Tumble" ) )		morph->tumble  =		fread_number( fp );
		else if ( !strcmp(word, "Vnum"	 ) )		morph->morph   =		get_morph_vnum( fread_number(fp) );
		else
			send_log( fp, LOG_FREAD, "fread_morph_data: no match: %s", word );
	} /* chiude il for */
}


void unmorph_all( MORPH_DATA *morph )
{
	CHAR_DATA *vch;

	for ( vch = first_player;  vch;  vch = vch->next_player )
	{
		if ( !vch->morph )					continue;
		if ( !vch->morph->morph )			continue;
		if ( vch->morph->morph != morph )	continue;

		do_unmorph_char( vch );
	}
}


/*
 * Following functions are for administrator testing purposes.
 */
DO_RET do_adm_morph( CHAR_DATA *ch, char *argument )
{
	MORPH_DATA *morph;
	CHAR_DATA  *victim = NULL;
	char		arg[MIL];
	char		arg2[MIL];
	VNUM		vnum;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Solo i giocatori possono usare questo comando.\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	one_argument( argument, arg2 );

	if ( !is_number(arg) )
	{
		send_to_char( ch, "Syntax: morph <vnum>\r\n" );
		return;
	}

	vnum = atoi( arg );
	morph = get_morph_vnum( vnum );

	if ( morph == NULL )
	{
		ch_printf( ch, "Nessun morph %d esistente.\r\n", vnum );
		return;
	}

	if ( !VALID_STR(arg2) )
	{
		do_morph_char( ch, morph );
	}
	else
	{
		victim = get_player_mud( ch, arg2, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "Nessuno con questo nome da nessuna parte.\r\n" );
			return;
		}
	}

	if ( victim )
	{
		if ( get_trust(ch) < get_trust(victim) )
		{
			send_to_char( ch, "Non puoi farlo!\r\n" );
			return;
		}
		do_morph_char( victim, morph );
	}

	send_to_char( ch, "Fatto.\r\n" );
}


/*
 * This is just a wrapper.
 */
DO_RET do_adm_unmorph( CHAR_DATA *ch , char *argument )
{
	CHAR_DATA	*victim = NULL;

	if ( !VALID_STR(argument) )
	{
		do_unmorph_char( ch );
	}
	else
	{
		victim = get_player_mud( ch, argument, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "Nessuno con questo nome da nessuna parte.\r\n" );
			return;
		}
	}

	if ( victim )
	{
		if ( get_trust(ch) < get_trust(victim) )
		{
			send_to_char( ch, "Non puoi fare questo!\r\n" );
			return;
		}

		do_unmorph_char( victim );
	}

	send_to_char( ch, "Fatto.\r\n" );
}
