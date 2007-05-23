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
 >						Modulo delle "Procedure Speciali"					<
\****************************************************************************/

#include <dlfcn.h>		/* libdl */

#include "mud.h"
#include "affect.h"
#include "calendar.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "fread.h"
#include "interpret.h"
#include "movement.h"
#include "room.h"
#include "special.h"


/*
 * Definzione del file con l'elenco delle funzioni speciali
 */
#define SPECFUN_FILE	TABLES_DIR	"specfuns.dat"


/*
 * Variabili locali
 */
SPECFUN_DATA	*first_specfun	= NULL;
SPECFUN_DATA	*last_specfun	= NULL;
int				 top_specfun	= 0;


/* Snippet di Trax. Il commento non è da togliere o modificare,
 *	serve da credito a chi ha creato lo snippet.
------------------------------------------------------------------------------
 * DLSym Snippet.
 * Written by Trax of Forever's End
 * Simple load function - no OLC support for now.
 * This is probably something you DONT want builders playing with.
------------------------------------------------------------------------------
 * (RR) probabilmente da convertire in funzione fread_section
 */
void load_specfuns( void )
{
	SPECFUN_DATA	*specfun;
	MUD_FILE	*fp;

	fp = mud_fopen( "", SPECFUN_FILE, "r", TRUE );
	if ( !fp )
	{
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	for ( ; ; )
	{
		char	*word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_BUG, "load_specfuns: Premature end of file %s!", fp->path );
			MUD_FCLOSE( fp );
			return;
		}

		word = fread_word( fp );
		if( !str_cmp(word, "$") )
			break;

		CREATE( specfun, SPECFUN_DATA, 1 );
		specfun->name = str_dup( word );
		LINK( specfun, first_specfun, last_specfun, next, prev );
		top_specfun++;
	}

	MUD_FCLOSE( fp );
}


/*
 * Libera dalla memoria tutti le procedure speciali
 */
void free_all_specfuns( void )
{
	SPECFUN_DATA *specfun;

	for ( specfun = first_specfun;  specfun;  specfun = first_specfun )
	{
		UNLINK( specfun, first_specfun, last_specfun, next, prev );
		top_specfun--;

		DISPOSE( specfun->name );
		DISPOSE( specfun );
	}
}


/* Simple validation function to be sure a function can be used on mobs */
bool validate_specfun( char *name )
{
	SPECFUN_DATA *specfun;

	for ( specfun = first_specfun;  specfun;  specfun = specfun->next )
	{
		if ( !str_cmp(specfun->name, name) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Given a name, return the appropriate spec_fun.
 */
SPEC_FUN *specfun_lookup( const char *name )
{
	void	   *funHandle;
	const char *error;

	funHandle = dlsym( sysdata.dlHandle, name );
	if ( (error = dlerror( ) ) != NULL)
	{
		send_log( NULL, LOG_BUG, "Error locating function %s in symbol table.", name );
		return NULL;
	}

	return (SPEC_FUN *) funHandle;
}


/*
 * Se un mob conosce la magia e odia qualcuno allora prova a evocarlo.
 */
void summon_if_hating( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	char	   buf[MSL];
	char	   name[MIL];
	bool	   found = FALSE;

	if ( ch->position <= POSITION_SLEEP )			return;
	if ( ch->fighting ) 							return;
	if ( ch->fearing )								return;
	if ( !ch->hating )								return;
	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )	return;
	if ( ch->hunting )								return;	/* Non evoca se un pg è vicino abbastanza da poterlo cacciare */

	one_argument( ch->hating->name, name );

	/* Si assicura che il giocatore esista, funziona anche se il pg esce dal gioco */
	for ( victim = first_char;  victim;  victim = victim->next )
	{
		if ( !str_cmp(victim->name, ch->hating->name) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
		return;

	if ( victim->in_room == ch->in_room )
		return;

	sprintf( buf, "cast summon %s", name );
	send_command( ch, buf, CO );
}


/*
 * Procedura centrale per i dragoni.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	int		   sn;

	if ( ch->position != POSITION_FIGHT
	  && ch->position != POSITION_EVASIVE
	  && ch->position != POSITION_DEFENSIVE
	  && ch->position != POSITION_AGGRESSIVE
	  && ch->position != POSITION_BERSERK )
	{
		return FALSE;
	}

	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( who_fighting(victim) != ch )	continue;
		if ( number_bits(2) )				continue;

		if ( IS_PG(victim) )
			break;
		if ( IS_MOB(victim) && HAS_BIT(victim->affected_by, AFFECT_CHARM) )
			break;
	}

	if ( !victim )
		return FALSE;

	sn = skill_lookup( spell_name );
	if ( sn < 0 )
		return FALSE;

	(*table_skill[sn]->spell_fun) ( sn, get_level(ch)/2, ch, victim );
	return TRUE;
}


/*
 * Procedure speciali per i soffi.
 */
SPEC_RET spec_breath_acid( CHAR_DATA *ch )
{
	return dragon( ch, "acid breath" );
}

SPEC_RET spec_breath_fire( CHAR_DATA *ch )
{
	return dragon( ch, "fire breath" );
}

SPEC_RET spec_breath_frost( CHAR_DATA *ch )
{
	return dragon( ch, "frost breath" );
}

SPEC_RET spec_breath_gas( CHAR_DATA *ch )
{
	int sn;

	if ( ch->position != POSITION_FIGHT
	  && ch->position != POSITION_EVASIVE
	  && ch->position != POSITION_DEFENSIVE
	  && ch->position != POSITION_AGGRESSIVE
	  && ch->position != POSITION_BERSERK )
	{
		return FALSE;
	}

	sn = skill_lookup( "gas breath" );
	if ( sn < 0 )
		return FALSE;

	(*table_skill[sn]->spell_fun) ( sn, get_level(ch)/2, ch, NULL );
	return TRUE;
}

SPEC_RET spec_breath_lightning( CHAR_DATA *ch )
{
	return dragon( ch, "lightning breath" );
}

SPEC_RET spec_breath_any( CHAR_DATA *ch )
{
	if ( ch->position != POSITION_FIGHT
	  && ch->position != POSITION_EVASIVE
	  && ch->position != POSITION_DEFENSIVE
	  && ch->position != POSITION_AGGRESSIVE
	  && ch->position != POSITION_BERSERK )
	{
		return FALSE;
	}

	switch ( number_bits(3) )
	{
	  case 0:	return spec_breath_fire		( ch );
	  case 1:
	  case 2:	return spec_breath_lightning( ch );
	  case 3:	return spec_breath_gas		( ch );
	  case 4:	return spec_breath_acid		( ch );
	  case 5:
	  case 6:
	  case 7:	return spec_breath_frost	( ch );
	}

	return FALSE;
}


/*
 * Procedure speciali per i Mob
 */
SPEC_RET spec_cast_adept( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;

	if ( !is_awake(ch) )	return FALSE;
	if ( ch->fighting )		return FALSE;

	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( victim == ch )				continue;
		if ( !can_see(ch, victim) )		continue;

		if ( number_bits(1) == 0 )
			break;
	}

	if ( !victim )
		return FALSE;

	switch ( number_bits(4) )
	{
	  case 0:
		act( AT_MAGIC, "$n pronuncia la parola 'ciroht'.", ch, NULL, NULL, TO_ROOM );
		spell_smaug( skill_lookup("armor"), get_level(ch)/2, ch, victim );
		return TRUE;

	  case 1:
		act( AT_MAGIC, "$n pronuncia la parola 'sunimod'.", ch, NULL, NULL, TO_ROOM );
		spell_smaug( skill_lookup("bless"), get_level(ch)/2, ch, victim );
		return TRUE;

	  case 2:
		act( AT_MAGIC, "$n pronuncia la parola 'suah'.", ch, NULL, NULL, TO_ROOM );
		spell_cure_blindness( skill_lookup("cure blindness"), get_level(ch)/2, ch, victim );
		return TRUE;

	  case 3:
		act( AT_MAGIC, "$n pronuncia la parola 'nran'.", ch, NULL, NULL, TO_ROOM );
		spell_smaug( skill_lookup("cure light"), get_level(ch)/2, ch, victim );
		return TRUE;

	  case 4:
		act( AT_MAGIC, "$n pronuncia la parola 'nyrcs'.", ch, NULL, NULL, TO_ROOM );
		spell_cure_poison( skill_lookup("cure poison"), get_level(ch)/2, ch, victim );
		return TRUE;

	  case 5:
		act( AT_MAGIC, "$n pronuncia la parola 'gartla'.", ch, NULL, NULL, TO_ROOM );
		spell_smaug( skill_lookup("refresh"), get_level(ch)/2, ch, victim );
		return TRUE;

	  case 6:
		act( AT_MAGIC, "$n pronuncia la parola 'naimad'.", ch, NULL, NULL, TO_ROOM );
		spell_smaug( skill_lookup("cure serious"), get_level(ch)/2, ch, victim );
		return TRUE;

	  case 7:
		act( AT_MAGIC, "$n pronuncia la parola 'gorog'.", ch, NULL, NULL, TO_ROOM );
		spell_remove_curse( skill_lookup("remove curse"), get_level(ch)/2, ch, victim );
		return TRUE;
	}

	return FALSE;
}


SPEC_RET spec_cast_cleric( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char	  *spell;
	int		   sn;

	summon_if_hating( ch );

	if ( ch->position != POSITION_FIGHT
	  && ch->position != POSITION_EVASIVE
	  && ch->position != POSITION_DEFENSIVE
	  && ch->position != POSITION_AGGRESSIVE
	  && ch->position != POSITION_BERSERK )
	{
		return FALSE;
	}

	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( who_fighting(victim) != ch )	continue;
		if ( number_bits(2) )				continue;

		if ( IS_PG(victim) )
			break;
		if ( IS_MOB(victim) && HAS_BIT(victim->affected_by, AFFECT_CHARM) )
			break;
	}

	if ( !victim  )		return FALSE;
	if ( victim == ch )	return FALSE;

	for ( ; ; )
	{
		int min_level;

		/* (FF) (LV) questi min_level forse sono da tarare */
		switch ( number_bits(4) )
		{
		  case  0:	min_level =  1;		spell = "cause light";		break;
		  case  1:	min_level =  6;		spell = "cause serious";	break;
		  case  2:	min_level = 12;		spell = "earthquake";		break;
		  case  3:	min_level = 14;		spell = "blindness";		break;
		  case  4:	min_level = 18;		spell = "cause critical";	break;
		  case  5:	min_level = 20;		spell = "dispel evil";		break;
		  case  6:	min_level = 24;		spell = "curse";			break;
		  case  7:	min_level = 26;		spell = "flamestrike";		break;
		  case  8:
		  case  9:
		  case 10:	min_level = 30;		spell = "harm";				break;
		  default:	min_level = 32;		spell = "dispel magic";		break;
		}

		if ( get_level(ch) >= min_level )
			break;
	}

	sn = skill_lookup( spell );
	if ( sn < 0 )
		return FALSE;

	(*table_skill[sn]->spell_fun) ( sn, get_level(ch)/2, ch, victim );
	return TRUE;
}


SPEC_RET spec_cast_mage( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char	  *spell;
	int		   sn;

	summon_if_hating( ch );

	if ( ch->position != POSITION_FIGHT
	  && ch->position != POSITION_EVASIVE
	  && ch->position != POSITION_DEFENSIVE
	  && ch->position != POSITION_AGGRESSIVE
	  && ch->position != POSITION_BERSERK )
	{
		return FALSE;
	}

	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( who_fighting(victim) != ch )	continue;
		if ( number_bits(2) )				continue;

		if ( IS_PG(victim) )
			break;
		if ( IS_MOB(victim) && HAS_BIT(victim->affected_by, AFFECT_CHARM) )
			break;
	}

	if ( !victim )		return FALSE;
	if ( victim == ch )	return FALSE;

	for ( ; ; )
	{
		int min_level;

		switch ( number_bits(4) )
		{
		  case  0:	min_level =  1;		spell = "magic missile";	break;
		  case  1:	min_level =  6;		spell = "chill touch";		break;
		  case  2:	min_level = 14;		spell = "weaken";			break;
		  case  3:	min_level = 16;		spell = "galvanic whip";	break;
		  case  4:	min_level = 22;		spell = "colour spray";		break;
		  case  5:	min_level = 24;		spell = "weaken";			break;
		  case  6:	min_level = 26;		spell = "energy drain";		break;
		  case  7:	min_level = 28;		spell = "spectral furor";	break;
		  case  8:
		  case  9:	min_level = 30;		spell = "fireball";			break;
		  default:	min_level = 40;		spell = "acid blast";		break;
		}

		if ( get_level(ch) >= min_level )
			break;
	}

	sn = skill_lookup( spell );
	if ( sn < 0 )
		return FALSE;

	(*table_skill[sn]->spell_fun) ( sn, get_level(ch)/2, ch, victim );
	return TRUE;
}


SPEC_RET spec_cast_undead( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char	  *spell;
	int		   sn;

	summon_if_hating( ch );

	if ( ch->position != POSITION_FIGHT
	  && ch->position != POSITION_EVASIVE
	  && ch->position != POSITION_DEFENSIVE
	  && ch->position != POSITION_AGGRESSIVE
	  && ch->position != POSITION_BERSERK )
	{
		return FALSE;
	}

	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( who_fighting(victim) != ch )	continue;
		if ( number_bits(2) == 0 )			continue;

		if ( IS_PG(victim) )
			break;
		if ( IS_MOB(victim) && HAS_BIT(victim->affected_by, AFFECT_CHARM) )
			break;
	}

	if ( !victim )		return FALSE;
	if ( victim == ch )	return FALSE;

	for ( ; ; )
	{
		int min_level;

		switch ( number_bits(4) )
		{
		  case  0:	min_level =  1;		spell = "chill touch";		break;
		  case  1:	min_level = 22;		spell = "weaken";			break;
		  case  2:	min_level = 24;		spell = "curse";			break;
		  case  3:	min_level = 26;		spell = "blindness";		break;
		  case  4:	min_level = 28;		spell = "poison";			break;
		  case  5:	min_level = 30;		spell = "energy drain";		break;
		  case  6:	min_level = 36;		spell = "harm";				break;
		  default:	min_level = 80;		spell = "gate";				break;
		}

		if ( get_level(ch) >= min_level )
			break;
	}

	sn = skill_lookup( spell );
	if ( sn < 0 )
		return FALSE;

	(*table_skill[sn]->spell_fun) ( sn, get_level(ch)/2, ch, victim );
	return TRUE;
}


SPEC_RET spec_executioner( CHAR_DATA *ch )
{
	MOB_PROTO_DATA *cityguard;
	CHAR_DATA	   *victim;
	CHAR_DATA	   *v_next;
	char		   *crime;
	char			buf[MSL];

	if ( !is_awake(ch) )	return FALSE;
	if ( ch->fighting )		return FALSE;

	crime = "";
	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_KILLER) )
		{
			crime = "assassino";
			break;
		}

		if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_THIEF) )
		{
			crime = "ladro";
			break;
		}
	}

	if ( !victim )
		return FALSE;

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		sprintf( buf, "yell codardo di un %s!", crime );	/* (GR) articolo */
		send_command( ch, buf, CO );
		return TRUE;
	}

	sprintf( buf, "yell Proteggiamo l'innocente dal %s!!", crime );	/* (GR) articolo dal dall' */
	send_command( ch, buf, CO );
	multi_hit( ch, victim, TYPE_UNDEFINED );

	if ( char_died(ch) )
		return TRUE;

	/* Aggiunto il log nel caso che venga a mancare la guardia cittadina */
	cityguard = get_mob_index( NULL, VNUM_MOB_CITYGUARD );

	if ( !cityguard )
	{
		send_log( NULL, LOG_BUG, "spec_executioner: Guardia cittadina mancante - Vnum:[%d]", VNUM_MOB_CITYGUARD );
		return TRUE;
	}

	char_to_room( make_mobile(cityguard), ch->in_room );
	char_to_room( make_mobile(cityguard), ch->in_room );
	return TRUE;
}


SPEC_RET spec_fido( CHAR_DATA *ch )
{
	OBJ_DATA *corpse;
	OBJ_DATA *c_next;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_awake(ch) )
		return FALSE;

	for ( corpse = ch->in_room->first_content;  corpse;  corpse = c_next )
	{
		c_next = corpse->next_content;
		if ( corpse->type != OBJTYPE_CORPSE_MOB )
			continue;

		act( AT_ACTION, "$n divora selvaggiamente un cadavere.", ch, NULL, NULL, TO_ROOM );

		for ( obj = corpse->first_content;  obj;  obj = obj_next )
		{
			obj_next = obj->next_content;
			obj_from_obj( obj );
			obj_to_room( obj, ch->in_room );
		}

		free_object( corpse );
		return TRUE;
	}

	return FALSE;
}


SPEC_RET spec_guard( CHAR_DATA *ch )
{
	CHAR_DATA	*victim;
	CHAR_DATA	*v_next;
	CHAR_DATA	*ech;
	char		*crime;
	char		 buf[MSL];
	int			 max_evil;

	if ( !is_awake(ch) )	return FALSE;
	if ( ch->fighting )		return FALSE;

	max_evil = 300;
	ech		 = NULL;
	crime	 = "";

	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( !victim->fighting )			continue;
		if ( who_fighting(victim) == ch )	continue;

		if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_KILLER) )
		{
			crime = "assassino";
			break;
		}

		if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_THIEF) )
		{
			crime = "ladro";
			break;
		}

		if ( victim->alignment < max_evil )
		{
			max_evil = victim->alignment;
			ech	  = victim;
		}
	}

	if ( victim && HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		sprintf( buf, "yell codardo di un %s!", crime );	/* (GR) articolo */
		send_command( ch, buf, CO );
		return TRUE;
	}

	if ( victim )
	{
		sprintf( buf, "yell Proteggiamo l'innocente dal %s!!", crime );	/* (GR) articolo dal dall' */
		send_command( ch, buf, CO );
		multi_hit( ch, victim, TYPE_UNDEFINED );
		return TRUE;
	}

	if ( ech )
	{
		act( AT_YELL, "$n urla 'Proteggiamo l'innocente!'", ch, NULL, NULL, TO_ROOM );
		multi_hit( ch, ech, TYPE_UNDEFINED );
		return TRUE;
	}

	return FALSE;
}


SPEC_RET spec_janitor( CHAR_DATA *ch )
{
	OBJ_DATA *trash;
	OBJ_DATA *trash_next;

	if ( !is_awake(ch) )
		return FALSE;

	for ( trash = ch->in_room->first_content;  trash;  trash = trash_next )
	{
		trash_next = trash->next_content;

		if ( !HAS_BIT(trash->wear_flags, OBJWEAR_TAKE) )	continue;
		if ( HAS_BIT(trash->extra_flags, OBJFLAG_BURIED) )	continue;

		if ( trash->type == OBJTYPE_DRINK_CON
		  || trash->type == OBJTYPE_TRASH
		  || trash->cost < 10
		  || (trash->pObjProto->vnum == VNUM_OBJ_SHOPPING_BAG && !trash->first_content) )
		{
			act( AT_ACTION, "$n raccoglie della spazzatura.", ch, NULL, NULL, TO_ROOM );
			obj_from_room( trash );
			obj_to_char( trash, ch );
			return TRUE;
		}
	}

	return FALSE;
}


SPEC_RET spec_mayor( CHAR_DATA *ch )
{
	static const char  open_path[]  = "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
	static const char  close_path[] = "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";
	static const char *path;
	static int		   pos;
	static bool		   move;

	if ( !move )
	{
		if ( calendar.hour == 6 )
		{
			path = open_path;
			move = TRUE;
			pos  = 0;
		}

		if ( calendar.hour == 20 )
		{
			path = close_path;
			move = TRUE;
			pos  = 0;
		}
	}

	if ( ch->fighting )
		return spec_cast_cleric( ch );

	if ( !move || ch->position < POSITION_SLEEP )
		return FALSE;

	/* (TR) */
	switch ( path[pos] )
	{
	  case '0':
	  case '1':
	  case '2':
	  case '3':
		move_char( ch, get_exit(ch->in_room, path[pos] - '0'), 0, FALSE );
		break;

	  case 'W':
		ch->position = POSITION_STAND;
		act( AT_ACTION, "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'S':
		ch->position = POSITION_SLEEP;
		act( AT_ACTION, "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'a':
		act( AT_SAY, "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'b':
		act( AT_SAY, "$n says 'What a view!  I must do something about that dump!'", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'c':
		act( AT_SAY, "$n says 'Vandals!  Youngsters have no respect for anything!'", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'd':
		act( AT_SAY, "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'e':
		act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven open!'", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'E':
		act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven closed!'", ch, NULL, NULL, TO_ROOM );
		break;

	  case 'O':
	  	send_command( ch, "unlock gate", CO );
		send_command( ch, "open gate", CO );
		break;

	  case 'C':
	  	send_command( ch, "close gate", CO );
	  	send_command( ch, "lock gate", CO );
		break;

	  case '.' :
		move = FALSE;
		break;
	}

	pos++;
	return FALSE;
}


SPEC_RET spec_poison( CHAR_DATA *ch )
{
	CHAR_DATA *victim;

	if ( ch->position != POSITION_FIGHT
	  && ch->position != POSITION_EVASIVE
	  && ch->position != POSITION_DEFENSIVE
	  && ch->position != POSITION_AGGRESSIVE
	  && ch->position != POSITION_BERSERK )
	{
		return FALSE;
	}

	victim = who_fighting( ch );
	if ( !victim )								return FALSE;
	if ( number_percent() > get_level(ch)/2 )	return FALSE;

	act( AT_HIT,	"Mordi $N!",	ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n morde $N!", ch, NULL, victim, TO_NOVICT );
	act( AT_POISON, "$n ti morde!", ch, NULL, victim, TO_VICT );

	spell_poison( gsn_poison, get_level(ch)/2, ch, victim );
	return TRUE;
}


SPEC_RET spec_thief( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	int		   gold;
	int		   maxgold;

	if ( ch->position != POSITION_STAND )
		return FALSE;

	for ( victim = ch->in_room->first_person;  victim;  victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( IS_MOB(victim) )			continue;
		if ( IS_ADMIN(victim) )			continue;
		if ( number_bits(2) != 0 )		continue;
		if ( !can_see(ch, victim) )		continue;

		if ( is_awake(victim) && number_range(0, get_level(ch)/2) == 0 )
		{
			act( AT_ACTION, "$n mi sta derubando! Dannato ladro!",
				ch, NULL, victim, TO_VICT	 );
			if ( get_curr_sense(ch, SENSE_SIXTH) > 75 )
			{
				act( AT_ACTION, "Mi accorgo che $n sta frugando nel suo sacco di monete di $N!",
					ch, NULL, victim, TO_NOVICT );
			}
		}
		else
		{
			maxgold = get_level(ch)/2 * get_level(ch)/2 * 1000;
			gold = victim->gold * number_range( 1, URANGE(2, get_level(ch)/8, 10) ) / 100;
			ch->gold += 9 * gold / 10;
			victim->gold -= gold;

			if ( ch->gold > maxgold )
			{
				boost_economy( ch->in_room->area, ch->gold - maxgold/2 );
				ch->gold = maxgold/2;
			}
		}
		return TRUE;
	} /* chiude il for */

	return FALSE;
}
