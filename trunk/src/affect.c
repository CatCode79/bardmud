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

/****************************************************************************************\
 >		Modulo di gestione degli Affect e dei RIS (Resistenze-Immunità-Suscettibilità)	<
\****************************************************************************************/


#include "mud.h"
#include "affect.h"
#include "db.h"
#include "morph.h"
#include "room.h"


/*
 * Variabili
 */
int	top_affect = 0;		/* Conta gli affect totali nel Mud */


/*
 * Tabella di codici affect
 */
const CODE_DATA code_affect[] =
{
	{ AFFECT_BLIND,				"AFFECT_BLIND",				"blind"				},
	{ AFFECT_INVISIBLE,			"AFFECT_INVISIBLE",			"invisible"			},
	{ AFFECT_DETECT_EVIL,		"AFFECT_DETECT_EVIL",		"detect_evil"		},
	{ AFFECT_DETECT_INVIS,		"AFFECT_DETECT_INVIS",		"detect_invis"		},
	{ AFFECT_DETECT_MAGIC,		"AFFECT_DETECT_MAGIC",		"detect_magic"		},
	{ AFFECT_DETECT_HIDDEN,		"AFFECT_DETECT_HIDDEN",		"detect_hidden"		},
	{ AFFECT_HOLD,				"AFFECT_HOLD",				"hold"				},
	{ AFFECT_SANCTUARY,			"AFFECT_SANCTUARY",			"sanctuary"			},
	{ AFFECT_FAERIE_FIRE,		"AFFECT_FAERIE_FIRE",	 	"faerie_fire"		},
	{ AFFECT_INFRARED,			"AFFECT_INFRARED",			"infrared"			},
	{ AFFECT_CURSE,				"AFFECT_CURSE",				"curse"				},
	{ AFFECT_FLAMING,			"AFFECT_FLAMING",			"_flaming"			},
	{ AFFECT_POISON,			"AFFECT_POISON",			"poison"			},
	{ AFFECT_PROTECT,			"AFFECT_PROTECT",			"protect"			},
	{ AFFECT_PARALYSIS,			"AFFECT_PARALYSIS",			"_paralysis"		},
	{ AFFECT_SNEAK,				"AFFECT_SNEAK",				"sneak"				},
	{ AFFECT_HIDE,				"AFFECT_HIDE",				"hide"				},
	{ AFFECT_SLEEP,				"AFFECT_SLEEP",				"sleep"				},
	{ AFFECT_CHARM,				"AFFECT_CHARM",				"charm"				},
	{ AFFECT_FLYING,			"AFFECT_FLYING",			"flying"			},
	{ AFFECT_PASS_DOOR,			"AFFECT_PASS_DOOR",			"pass_door"			},
	{ AFFECT_FLOATING,			"AFFECT_FLOATING",			"floating"			},
	{ AFFECT_TRUESIGHT,			"AFFECT_TRUESIGHT",			"truesight"			},
	{ AFFECT_DETECT_TRAPS,		"AFFECT_DETECT_TRAPS",		"detect_traps"		},
	{ AFFECT_SCRYING,			"AFFECT_SCRYING",			"scrying"			},
	{ AFFECT_FIRESHIELD,		"AFFECT_FIRESHIELD",		"fireshield"		},
	{ AFFECT_SHOCKSHIELD,		"AFFECT_SHOCKSHIELD",		"shockshield"		},
	{ AFFECT_HAUS1,				"AFFECT_HAUS1",				"r1"				},
	{ AFFECT_ICESHIELD,			"AFFECT_ICESHIELD",			"iceshield"			},
	{ AFFECT_POSSESS,			"AFFECT_POSSESS",			"possess"			},
	{ AFFECT_BERSERK,			"AFFECT_BERSERK",			"berserk"			},
	{ AFFECT_AQUA_BREATH,		"AFFECT_AQUA_BREATH",		"aqua_breath"		},
	{ AFFECT_RECURRING_SPELL,	"AFFECT_RECURRING_SPELL",	"recurring_spell"	},
	{ AFFECT_CONTAGIOUS,		"AFFECT_CONTAGIOUS",		"contagious"		},
	{ AFFECT_ACIDMIST,			"AFFECT_ACIDMIST",			"acidmist"			},
	{ AFFECT_VENOMSHIELD,		"AFFECT_VENOMSHIELD",		"venomshield"		},
	{ AFFECT_DETECT_GOOD,		"AFFECT_DETECT_GOOD",		"detect_good"		},
	{ AFFECT_DETECT_ALIGN,		"AFFECT_DETECT_ALIGN",		"detect_align"		}
};
const int max_check_affect = sizeof(code_affect)/sizeof(CODE_DATA);


/*
 * Tabella codici sui tipi di apply
 */
const CODE_DATA code_apply[] =
{
	{ APPLY_NONE,			"APPLY_NONE",			"none"				},	/* 0 */

	{ APPLY_STR,			"APPLY_STR",			"strength"			},
	{ APPLY_AGI,			"APPLY_AGI",			"dexterity"			},
	{ APPLY_INT,			"APPLY_INT",			"intelligence"		},
	{ APPLY_SPI,			"APPLY_SPI",			"spirituality"		},
	{ APPLY_CON,			"APPLY_CON",			"constitution"		},

	{ APPLY_SEX,			"APPLY_SEX",			"sex"				},
	{ APPLY_CLASS,			"APPLY_CLASS",			"class"				},
	{ APPLY_LEVEL,			"APPLY_LEVEL",			"level"				},
	{ APPLY_AGE,			"APPLY_AGE",			"age"				},
	{ APPLY_HEIGHT,			"APPLY_HEIGHT",			"height"			},	/* 10 */
	{ APPLY_WEIGHT,			"APPLY_WEIGHT",			"weight"			},
	{ APPLY_MANA,			"APPLY_MANA",			"mana"				},
	{ APPLY_LIFE,			"APPLY_LIFE",			"hit"				},
	{ APPLY_MOVE,			"APPLY_MOVE",			"move"				},
	{ APPLY_GOLD,			"APPLY_GOLD",			"gold"				},
	{ APPLY_EXP,			"APPLY_EXP",			"experience"		},
	{ APPLY_AC,				"APPLY_AC",				"armor"				},
	{ APPLY_HITROLL,		"APPLY_HITROLL",		"hitroll"			},
	{ APPLY_DAMROLL,		"APPLY_DAMROLL",		"damroll"			},
	{ APPLY_SAVE_POISON,	"APPLY_SAVE_POISON",	"save_poison"		},	/* 20 */
	{ APPLY_SAVE_ROD,		"APPLY_SAVE_ROD",		"save_rod"			},
	{ APPLY_SAVE_PARA,		"APPLY_SAVE_PARA",		"save_para"			},
	{ APPLY_SAVE_BREATH,	"APPLY_SAVE_BREATH",	"save_breath"		},
	{ APPLY_SAVE_SPELL,		"APPLY_SAVE_SPELL",		"save_spell"		},
	{ APPLY_EMP,			"APPLY_EMP",			"empatia"			},
	{ APPLY_AFFECT,			"APPLY_AFFECT",			"affected"			},
	{ APPLY_RESISTANT,		"APPLY_RESISTANT",		"resistant"			},
	{ APPLY_IMMUNE,			"APPLY_IMMUNE",			"immune"			},
	{ APPLY_SUSCEPTIBLE,	"APPLY_SUSCEPTIBLE",	"susceptible"		},
	{ APPLY_WEAPONSPELL,	"APPLY_WEAPONSPELL",	"weaponspell"		},	/* 30 */
	{ APPLY_LCK,			"APPLY_LCK",			"luck"				},
	{ APPLY_BACKSTAB,		"APPLY_BACKSTAB",		"backstab"			},
	{ APPLY_PICK,			"APPLY_PICK",			"pick"				},
	{ APPLY_TRACK,			"APPLY_TRACK",			"track"				},
	{ APPLY_STEAL,			"APPLY_STEAL",			"steal"				},
	{ APPLY_SNEAK,			"APPLY_SNEAK",			"sneak"				},
	{ APPLY_HIDE,			"APPLY_HIDE",			"hide"				},
	{ APPLY_PALM,			"APPLY_PALM",			"palm"				},
	{ APPLY_DETRAP,			"APPLY_DETRAP",			"detrap"			},
	{ APPLY_DODGE,			"APPLY_DODGE",			"dodge"				},	/* 40 */
	{ APPLY_PEEK,			"APPLY_PEEK",			"peek"				},
	{ APPLY_SCAN,			"APPLY_SCAN",			"scan"				},
	{ APPLY_GOUGE,			"APPLY_GOUGE",			"gouge"				},
	{ APPLY_SEARCH,			"APPLY_SEARCH",			"search"			},
	{ APPLY_MOUNT,			"APPLY_MOUNT",			"mount"				},
	{ APPLY_DISARM,			"APPLY_DISARM",			"disarm"			},
	{ APPLY_KICK,			"APPLY_KICK",			"kick"				},
	{ APPLY_PARRY,			"APPLY_PARRY",			"parry"				},
	{ APPLY_BASH,			"APPLY_BASH",			"bash"				},
	{ APPLY_STUN,			"APPLY_STUN",			"stun"				},	/* 50 */
	{ APPLY_PUNCH,			"APPLY_PUNCH",			"punch"				},
	{ APPLY_CLIMB,			"APPLY_CLIMB",			"climb"				},
	{ APPLY_GRIP,			"APPLY_GRIP",			"grip"				},
	{ APPLY_SCRIBE,			"APPLY_SCRIBE",			"scribe"			},
	{ APPLY_BREW,			"APPLY_BREW",			"brew"				},
	{ APPLY_WEARSPELL,		"APPLY_WEARSPELL",		"wearspell"			},
	{ APPLY_REMOVESPELL,	"APPLY_REMOVESPELL",	"removespell"		},
	{ APPLY_EMOTION,		"APPLY_EMOTION",		"emotion"			},
	{ APPLY_MENTALSTATE,	"APPLY_MENTALSTATE",	"mentalstate"		},
	{ APPLY_STRIPSN,		"APPLY_STRIPSN",		"stripsn"			},	/* 60 */
	{ APPLY_REMOVE,			"APPLY_REMOVE",			"remove"			},
	{ APPLY_DIG,			"APPLY_DIG",			"dig"				},
	{ APPLY_FULL,			"APPLY_FULL",			"full"				},
	{ APPLY_THIRST,			"APPLY_THIRST",			"thirst"			},
	{ APPLY_DRUNK,			"APPLY_DRUNK",			"drunk"				},
	{ APPLY_BLOOD,			"APPLY_BLOOD",			"blood"				},
	{ APPLY_COOK,			"APPLY_COOK",			"cook"				},
	{ APPLY_RECURRINGSPELL,	"APPLY_RECURRINGSPELL",	"recurring_spell"	},
	{ APPLY_CONTAGIOUS,		"APPLY_CONTAGIOUS",		"contagious"		},
	{ APPLY_EXT_AFFECT,		"APPLY_EXT_AFFECT",		"xaffected"			},	/* 70 */
	{ APPLY_ODOR,			"APPLY_ODOR",			"odor"				},
	{ APPLY_ROOMFLAG,		"APPLY_ROOMFLAG",		"roomflag"			},
	{ APPLY_SECTORTYPE,		"APPLY_SECTORTYPE",		"sectortype"		},
	{ APPLY_ROOMLIGHT,		"APPLY_ROOMLIGHT",		"roomlight"			},
	{ APPLY_TELEVNUM,		"APPLY_TELEVNUM",		"televnum"			},
	{ APPLY_TELEDELAY,		"APPLY_TELEDELAY",		"teledelay"			},
	{ APPLY_SOUL,			"APPLY_SOUL",			"soul"				},

	{ APPLY_SIGHT,			"APPLY_SIGHT",			"sight"				},
	{ APPLY_HEARING,		"APPLY_HEARING",		"hearing"			},
	{ APPLY_SMELL,			"APPLY_SMELL",			"smell"				},	/* 80 */
	{ APPLY_TASTE,			"APPLY_TASTE",			"taste"				},
	{ APPLY_TOUCH,			"APPLY_TOUCH",			"touch"				},
	{ APPLY_SIXTH,			"APPLY_SIXTH",			"sixth"				},

	{ APPLY_RES,			"APPLY_RES",			"resistance"		},
	{ APPLY_WIL,			"APPLY_WIL",			"willow"			},
	{ APPLY_COC,			"APPLY_COC",			"concentration"		},
	{ APPLY_REF,			"APPLY_REF",			"reflex"			},
	{ APPLY_SPE,			"APPLY_SPE",			"speed"				},
	{ APPLY_MIG,			"APPLY_MIG",			"migration"			},
	{ APPLY_ABS,			"APPLY_ABS",			"absorbation"		},	/* 90 */
	{ APPLY_BEA,			"APPLY_BEA",			"beauty"			},
	{ APPLY_LEA,			"APPLY_LEA",			"leadership"		}
};
const int max_check_apply = sizeof(code_apply)/sizeof(CODE_DATA);


/*
 * Tabella codici sulle resistenze-immunità-suscettibilità (RIS)
 */
const CODE_DATA code_ris[] =
{
	{ RIS_FIRE,			"RIS_FIRE",			"fire"			},
	{ RIS_COLD,			"RIS_COLD",			"cold"			},
	{ RIS_ELECTRICITY,	"RIS_ELECTRICITY",	"electricity"	},
	{ RIS_ENERGY,		"RIS_ENERGY",		"energy"		},
	{ RIS_BLUNT,		"RIS_BLUNT",		"blunt"			},
	{ RIS_PIERCE,		"RIS_PIERCE",		"pierce"		},
	{ RIS_SLASH,		"RIS_SLASH",		"slash"			},
	{ RIS_ACID,			"RIS_ACID",			"acid"			},
	{ RIS_POISON,		"RIS_POISON",		"poison"		},
	{ RIS_DRAIN,		"RIS_DRAIN",		"drain"			},
	{ RIS_SLEEP,		"RIS_SLEEP",		"sleep"			},
	{ RIS_CHARM,		"RIS_CHARM",		"charm"			},
	{ RIS_HOLD,			"RIS_HOLD",			"hold"			},
	{ RIS_NONMAGIC,		"RIS_NONMAGIC",		"nonmagic"		},
	{ RIS_PLUS1,		"RIS_PLUS1",		"plus1"			},
	{ RIS_PLUS2,		"RIS_PLUS2",		"plus2"			},
	{ RIS_PLUS3,		"RIS_PLUS3",		"plus3"			},
	{ RIS_PLUS4,		"RIS_PLUS4",		"plus4"			},
	{ RIS_PLUS5,		"RIS_PLUS5",		"plus5"			},
	{ RIS_PLUS6,		"RIS_PLUS6",		"plus6"			},
	{ RIS_MAGIC,		"RIS_MAGIC",		"magic"			},
	{ RIS_PARALYSIS,	"RIS_PARALYSIS",	"paralysis"		}
};
const int max_check_ris = sizeof(code_ris)/sizeof(CODE_DATA);


/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
	OBJ_DATA   *wield;
	int			mod;
	SKILL_DATA *skill;
	ch_ret		retcode;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "affect_modify: ch è NULL" );
		return;
	}

	if ( !paf )
	{
		send_log( NULL, LOG_BUG, "affect_modifiy: paf è NULL per il pg %s", ch->name );
		return;
	}

	mod = paf->modifier;

	if ( fAdd )
	{
		SET_BITS( ch->affected_by, paf->bitvector );
		if ( (paf->location % REVERSE_APPLY) == APPLY_RECURRINGSPELL )
		{
			mod = abs( mod );
			if ( is_valid_sn(mod) && (skill = table_skill[mod]) != NULL
			  && skill->type == SKILLTYPE_SPELL )
				SET_BIT( ch->affected_by, AFFECT_RECURRING_SPELL );
			else
				send_log( NULL, LOG_BUG, "affect_modify(%s) APPLY_RECURRINGSPELL con sn errato %d", ch->name, mod );

			return;
		}
	}
	else
	{
		REMOVE_BITS( ch->affected_by, paf->bitvector );
		/*
		 * Might be an idea to have a duration removespell which returns
		 * the spell after the duration... but would have to store
		 * the removed spell's information somewhere...
		 * (Though we could keep the affect, but disable it for a duration)
		 */
		if ( (paf->location % REVERSE_APPLY) == APPLY_RECURRINGSPELL )
		{
			mod = abs( mod );
			if ( !is_valid_sn(mod)
			  || (skill=table_skill[mod]) == NULL
			  || skill->type != SKILLTYPE_SPELL )
			{
				send_log( NULL, LOG_BUG, "affect_modify(%s) APPLY_RECURRINGSPELL con sn errato %d", ch->name, mod );
			}
			REMOVE_BIT( ch->affected_by, AFFECT_RECURRING_SPELL );
			return;
		}

		switch ( paf->location % REVERSE_APPLY )
		{
		  default:
			break;
		  case APPLY_AFFECT:		REMOVE_BITINT( *(ch->affected_by->flags+ch->affected_by->size-1), mod );	return;	/* (BB) questi qui tolgono la flag di BLIND, quasi sempre */
		  case APPLY_EXT_AFFECT:	REMOVE_BIT( ch->affected_by, mod );		return;
		  case APPLY_RESISTANT:		REMOVE_BIT( ch->resistant, mod );		return;
		  case APPLY_IMMUNE:		REMOVE_BIT( ch->immune, mod );			return;
		  case APPLY_SUSCEPTIBLE:	REMOVE_BIT( ch->susceptible, mod );		return;
		  case APPLY_REMOVE:
		  	if ( mod > 0 )
		  		SET_BIT( ch->affected_by, mod );	/* (BB) per evitare che inserisca sempre il blind */
		  		return;
		}

		mod = 0 - mod;

	} /* chiude l'else */

	send_log( NULL, LOG_APPLY, "affect_modify: apply %s a %s per %d",
		code_name(NULL, paf->location, CODE_APPLY), ch->name, mod );

	switch ( paf->location % REVERSE_APPLY )
	{
	  default:
		send_log( NULL, LOG_BUG, "Affect_modify: unknown location %d", paf->location );
		return;

	  case APPLY_NONE:													break;
	  case APPLY_STR:				ch->attr_mod[ATTR_STR]	+= mod;		break;
	  case APPLY_RES:				ch->attr_mod[ATTR_RES]	+= mod;		break;
	  case APPLY_CON:				ch->attr_mod[ATTR_CON]	+= mod;		break;
	  case APPLY_INT:				ch->attr_mod[ATTR_INT]	+= mod;		break;
	  case APPLY_COC:				ch->attr_mod[ATTR_COC]	+= mod;		break;
	  case APPLY_WIL:				ch->attr_mod[ATTR_WIL]	+= mod;		break;
	  case APPLY_AGI:				ch->attr_mod[ATTR_AGI]	+= mod;		break;
	  case APPLY_REF:				ch->attr_mod[ATTR_REF]	+= mod;		break;
	  case APPLY_SPE:				ch->attr_mod[ATTR_SPE]	+= mod;		break;
	  case APPLY_SPI:				ch->attr_mod[ATTR_SPI]	+= mod;		break;
	  case APPLY_MIG:				ch->attr_mod[ATTR_MIG]	+= mod;		break;
	  case APPLY_ABS:				ch->attr_mod[ATTR_ABS]	+= mod;		break;
	  case APPLY_EMP:				ch->attr_mod[ATTR_EMP]	+= mod;		break;
	  case APPLY_BEA:				ch->attr_mod[ATTR_BEA]	+= mod;		break;
	  case APPLY_LEA:				ch->attr_mod[ATTR_LEA]	+= mod;		break;
	  case APPLY_LCK:				break;	/* (RR) da togliere dopo la conversione */

	  case APPLY_SEX:
		if ( ch->sex == SEX_MALE )
			ch->sex = SEX_FEMALE;
		else
			ch->sex = SEX_MALE;
		break;

	  /* Inutilizzata per ora dato il richio collegato ad essa */
	  case APPLY_AGE:																break;

	  /* Regular apply types */
	  case APPLY_HEIGHT:			ch->height							+= mod;		break;
	  case APPLY_WEIGHT:			ch->weight							+= mod;		break;

	  case APPLY_LIFE:				ch->points_max[POINTS_LIFE]			+= mod;		break;
	  case APPLY_MANA:				ch->points_max[POINTS_MANA]			+= mod;		break;
	  case APPLY_MOVE:				ch->points_max[POINTS_MOVE]			+= mod;		break;
	  case APPLY_SOUL:				ch->points_max[POINTS_SOUL]			+= mod;		break;

	  case APPLY_SIGHT:				ch->sense_mod[SENSE_SIGHT]			+= mod;		break;
	  case APPLY_HEARING:			ch->sense_mod[SENSE_HEARING]		+= mod;		break;
	  case APPLY_SMELL:				ch->sense_mod[SENSE_SMELL]			+= mod;		break;
	  case APPLY_TASTE:				ch->sense_mod[SENSE_TASTE]			+= mod;		break;
	  case APPLY_TOUCH:				ch->sense_mod[SENSE_TOUCH]			+= mod;		break;
	  case APPLY_SIXTH:				ch->sense_mod[SENSE_SIXTH]			+= mod;		break;

	  case APPLY_AC:				ch->armor							+= mod;		break;
	  case APPLY_HITROLL:			ch->hitroll							+= mod;		break;
	  case APPLY_DAMROLL:			ch->damroll							+= mod;		break;

	  case APPLY_SAVE_POISON:		ch->saving_throw[SAVETHROW_BREATH]		+= mod;	break;
	  case APPLY_SAVE_ROD:			ch->saving_throw[SAVETHROW_PARA_PETRI]	+= mod;	break;
	  case APPLY_SAVE_PARA:			ch->saving_throw[SAVETHROW_POISON_DEATH]+= mod;	break;
	  case APPLY_SAVE_BREATH:		ch->saving_throw[SAVETHROW_SPELL_STAFF]	+= mod;	break;
	  case APPLY_SAVE_SPELL:		ch->saving_throw[SAVETHROW_ROD_WANDS]	+= mod;	break;

	  /* Bitvector modifying apply types */
	  case APPLY_AFFECT:
		if ( mod > 0 )
			SET_BITINT( *(ch->affected_by->flags+ch->affected_by->size-1), mod );	/* (BB) per evitare che inserisca sempre il BLIND */
			break;
	  case APPLY_EXT_AFFECT:
		if ( mod > 0 )
			SET_BIT( ch->affected_by, mod );	/* (BB) per evitare che inserisca sempre il BLIND */
			break;
	  case APPLY_RESISTANT:			SET_BIT( ch->resistant, mod );		break;
	  case APPLY_IMMUNE:			SET_BIT( ch->immune, mod );			break;
	  case APPLY_SUSCEPTIBLE:		SET_BIT( ch->susceptible, mod );	break;
	  case APPLY_WEAPONSPELL:		/* see fight.c */					break;
	  case APPLY_REMOVE:			REMOVE_BIT( ch->affected_by, mod );	break;	/* (BB) qui rimuove quasi sempre il bit BLIND */

	  /* Player condition modifiers */
	  case APPLY_FULL:
		if ( IS_PG(ch) )
			ch->pg->condition[CONDITION_FULL] = URANGE( 0, ch->pg->condition[CONDITION_FULL] + mod, 48 );
		break;

	  case APPLY_THIRST:
		if ( IS_PG(ch) )
			ch->pg->condition[CONDITION_THIRST] = URANGE( 0, ch->pg->condition[CONDITION_THIRST] + mod, 48 );
		break;

	  case APPLY_DRUNK:
		if ( IS_PG(ch) )
			ch->pg->condition[CONDITION_DRUNK] = URANGE( 0, ch->pg->condition[CONDITION_DRUNK] + mod, 48 );
		break;

	  case APPLY_BLOOD:
		if ( IS_PG(ch) )
			ch->pg->condition[CONDITION_BLOODTHIRST] = URANGE( 0, ch->pg->condition[CONDITION_BLOODTHIRST] + mod, get_level(ch)/2 + 10 );
		break;

	  case APPLY_MENTALSTATE:
		ch->mental_state	= URANGE(-100, ch->mental_state + mod, 100);
		break;

	  case APPLY_EMOTION:
		ch->emotional_state	= URANGE(-100, ch->emotional_state + mod, 100);
		break;

	  /* Specialty modifiers */
	  case APPLY_CONTAGIOUS:	/* (TT) Interessante, da pensare */
		break;

	  case APPLY_ODOR:			/* (TT) Interessante, da pensare */
		break;

	  case APPLY_STRIPSN:
		if ( is_valid_sn(mod) )
			affect_strip( ch, mod );
		else
			send_log( NULL, LOG_BUG, "affect_modify: APPLY_STRIPSN invalid sn %d", mod );
		break;

	  /* Spell cast upon wear/removal of an object */
	  case APPLY_WEARSPELL:
	  case APPLY_REMOVESPELL:
		if ( HAS_BIT(ch->in_room->flags, ROOM_NOMAGIC)
		  || HAS_BIT(ch->immune, RIS_MAGIC)
		  || ((paf->location%REVERSE_APPLY) == APPLY_WEARSPELL   && !fAdd)
		  || ((paf->location%REVERSE_APPLY) == APPLY_REMOVESPELL && !fAdd)
		  || saving_char == ch			/* so save/quit doesn't trigger */
		  || loading_char == ch )		/* so loading doesn't trigger   */
		{
			return;
		}

		mod = abs(mod);
		if ( is_valid_sn(mod)
		  && (skill=table_skill[mod]) != NULL
		  && skill->type == SKILLTYPE_SPELL )
		{
			if ( skill->target == TARGET_IGNORE || skill->target == TARGET_OBJ_INV )
			{
				send_log( NULL, LOG_BUG, "APPLY_WEARSPELL trying to apply bad target spell.  SN is %d.", mod );
				return;
			}
			if ( (retcode = (*skill->spell_fun) (mod, get_level(ch)/2, ch, ch)) == rCHAR_DIED || char_died(ch) )
				return;
		}
		break;

	/* Skill apply types */
	  case APPLY_PALM:			/* not implemented yet */							break;
	  case APPLY_TRACK:			modify_skill( ch, gsn_track,	mod, fAdd );		break;
	  case APPLY_HIDE:			modify_skill( ch, gsn_hide,		mod, fAdd );		break;
	  case APPLY_STEAL:			modify_skill( ch, gsn_steal,	mod, fAdd );		break;
	  case APPLY_SNEAK:			modify_skill( ch, gsn_sneak,	mod, fAdd );		break;
	  case APPLY_PICK:			modify_skill( ch, gsn_pick_lock,mod, fAdd );		break;
	  case APPLY_BACKSTAB:		modify_skill( ch, gsn_backstab,	mod, fAdd );		break;
	  case APPLY_DETRAP:		modify_skill( ch, gsn_detrap,	mod, fAdd );		break;
	  case APPLY_DODGE:			modify_skill( ch, gsn_dodge,	mod, fAdd );		break;
	  case APPLY_PEEK:			modify_skill( ch, gsn_peek,		mod, fAdd );		break;
	  case APPLY_SCAN:			modify_skill( ch, gsn_scan,		mod, fAdd );		break;
	  case APPLY_GOUGE:			modify_skill( ch, gsn_gouge,	mod, fAdd );		break;
	  case APPLY_SEARCH:		modify_skill( ch, gsn_search,	mod, fAdd );		break;
	  case APPLY_DIG:			modify_skill( ch, gsn_dig,		mod, fAdd );		break;
	  case APPLY_MOUNT:			modify_skill( ch, gsn_mount,	mod, fAdd );		break;
	  case APPLY_DISARM:		modify_skill( ch, gsn_disarm,	mod, fAdd );		break;
	  case APPLY_KICK:			modify_skill( ch, gsn_kick,		mod, fAdd );		break;
	  case APPLY_PARRY:			modify_skill( ch, gsn_parry,	mod, fAdd );		break;
	  case APPLY_BASH:			modify_skill( ch, gsn_bash,		mod, fAdd );		break;
	  case APPLY_STUN:			modify_skill( ch, gsn_stun,		mod, fAdd );		break;
	  case APPLY_PUNCH:			modify_skill( ch, gsn_punch,	mod, fAdd );		break;
	  case APPLY_CLIMB:			modify_skill( ch, gsn_climb,	mod, fAdd );		break;
	  case APPLY_GRIP:			modify_skill( ch, gsn_grip,		mod, fAdd );		break;
	  case APPLY_SCRIBE:		modify_skill( ch, gsn_scribe,	mod, fAdd );		break;
	  case APPLY_BREW:			modify_skill( ch, gsn_brew,		mod, fAdd );		break;
	  case APPLY_COOK:			modify_skill( ch, gsn_cook,		mod, fAdd );		break;

	  /* Room apply types */
	  case APPLY_ROOMFLAG:
	  case APPLY_SECTORTYPE:
	  case APPLY_ROOMLIGHT:
	  case APPLY_TELEVNUM:
		break;
	} /* chiude lo switch */

	/*
	 * Check for weapon wielding
	 */
	if ( IS_PG(ch)
	  && saving_char != ch
	  && (wield = get_eq_char(ch, WEARLOC_WIELD) ) != NULL
	  /* Per impugnare serve la forza, qui per mantenere impugnata l'arma serve la resistenza */
	  && get_obj_weight(wield)/1000 > get_curr_attr(ch, ATTR_RES)/5 )
	{
		static int depth;	/* Guard against recursion (for weapons with affects). */

		if ( depth == 0 )
		{
			depth++;
			act( AT_ACTION, "Sei troppo stanco per impugnare $p.", ch, wield, NULL, TO_CHAR );
			act( AT_ACTION, "$n smette di impugnare $p.", ch, wield, NULL, TO_ROOM );
			unequip_char( ch, wield );
			depth--;
		}
	}
}


/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "affect_to_char: ch è NULL con tipo di paf: %d", paf  ?  paf->type  :  0 );
		return;
	}

	if ( !paf )
	{
		send_log( NULL, LOG_BUG, "affect_to_char: paf è NULL per il pg %s", ch->name );
		return;
	}

	LINK( paf, ch->first_affect, ch->last_affect, next, prev );
	top_affect++;

	affect_modify( ch, paf, TRUE );
}


/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "affect_remove: ch è NULL" );
		return;
	}

	if ( !paf )
	{
		send_log( NULL, LOG_BUG, "affect_remove: paf è NULL per il pg %s", ch->name );
		return;
	}

	if ( !ch->first_affect )
	{
		send_log( NULL, LOG_BUG, "affect_remove: nessun affect per il pg %s, tipo di paf: %d", ch->name, paf  ?  paf->type  :  0 );
		return;
	}

	affect_modify( ch, paf, FALSE );

	UNLINK( paf, ch->first_affect, ch->last_affect, next, prev );
	top_affect--;

	destroybv( paf->bitvector );
	DISPOSE( paf );
}


/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "affect_strip: ch è NULL" );
		return;
	}

	if ( sn < 0 || sn >= top_sn )
	{
		send_log( NULL, LOG_BUG, "affect_strip: sn errato: %d", sn );
		return;
	}

	for ( paf = ch->first_affect;  paf;  paf = paf_next )
	{
		paf_next = paf->next;
		if ( paf->type == sn )
			affect_remove( ch, paf );
	}
}


/*
 * Add or enhance an affect.
 * Limitations they may be high... but at least they're there :)
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	AFFECT_DATA *paf_old;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "affect_join: ch è NULL" );
		return;
	}

	if ( !paf )
	{
		send_log( NULL, LOG_BUG, "affect_join: paf è NULL per il pg %s", ch->name );
		return;
	}

	for ( paf_old = ch->first_affect;  paf_old;  paf_old = paf_old->next )
	{
		if ( paf_old->type == paf->type )
		{
			paf->duration = UMIN( 100000, paf->duration + paf_old->duration );

			if ( paf->modifier )
				paf->modifier = UMIN( 5000, paf->modifier + paf_old->modifier );
			else
				paf->modifier = paf_old->modifier;

			affect_remove( ch, paf_old );
			break;
		}
	}

	affect_to_char( ch, paf );
}


/*
 * Ritorna TRUE se i due affect sono identici
 * Se check_duration è TRUE effettu anche un controllo sulla durata dell'affect
 */
bool is_same_affect( AFFECT_DATA *aff1, AFFECT_DATA *aff2, bool check_duration )
{
	if ( !aff1 )
	{
		send_log( NULL, LOG_BUG, "is_same_affect: aff1 è NULL" );
		return FALSE;
	}

	if ( !aff2 )
	{
		send_log( NULL, LOG_BUG, "is_same_affect: aff2 è NULL" );
		return FALSE;
	}

	if ( aff1->type != aff2->type )
		return FALSE;

	if ( check_duration && aff1->duration != aff2->duration )
		return FALSE;

	if ( aff1->location != aff2->location )
		return FALSE;

	if ( aff1->modifier != aff2->modifier )
		return FALSE;

	if ( !SAME_BITS(aff1->bitvector, aff2->bitvector) )
		return FALSE;

	return TRUE;
}


/*
 * Copia l'affect passato ritornandolo
 */
AFFECT_DATA *copy_affect( AFFECT_DATA *source )
{
	AFFECT_DATA *dest;

	if ( !source )
	{
		send_log( NULL, LOG_BUG, "copy_affect: affect source passato è NULL" );
		return NULL;
	}

	CREATE( dest, AFFECT_DATA, 1 );

	dest->type		= source->type;
	dest->duration	= source->duration;
	dest->location	= source->location;
	dest->modifier	= source->modifier;
	dest->bitvector	= source->bitvector;

	return dest;
}


/*
 * Libera dalla memoria una struttura di affect
 */
void free_affect( AFFECT_DATA *aff )
{
	destroybv( aff->bitvector );
	DISPOSE( aff );
}


/*
 * Libera dalla memoria una struttura di smaug affect
 */
void free_smaug_affect( SMAUG_AFF *saff )
{
	DISPOSE( saff->duration );
	DISPOSE( saff->modifier );
	DISPOSE( saff );
}


/*
 * Ritorna il numero attaule di affect
 */
int get_top_affect( void )
{
	return top_affect;
}


/*
 * Ritorna la grandezza stimata con il sizeof di un affect
 */
int get_sizeof_affect( void )
{
	return top_affect * sizeof( AFFECT_DATA );
}


/*
 * Apply only affected and RIS on a char
 */
static void aris_affect( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "aris_affect: ch è NULL" );
		return;
	}

	if ( !paf )
	{
		send_log( NULL, LOG_BUG, "aris_affect: paf è NULL per il pg %s", ch->name );
		return;
	}

	SET_BITS( ch->affected_by, paf->bitvector );

	if ( paf->modifier != 0 )	/* (BB) per evitare la cecità non inserisce il BLIND */
	{
		switch ( paf->location % REVERSE_APPLY )
		{
		  case APPLY_AFFECT:		SET_BITINT( *(ch->affected_by->flags+ch->affected_by->size-1), paf->modifier );	break;
		  case APPLY_RESISTANT:		SET_BIT( ch->resistant, paf->modifier );				break;
		  case APPLY_IMMUNE:		SET_BIT( ch->immune, paf->modifier );					break;
		  case APPLY_SUSCEPTIBLE:	SET_BIT( ch->susceptible, paf->modifier );				break;
		}
	}
}


/*
 * Update affecteds and RIS for a character in case things get messed.
 * This should only really be used as a quick fix until the cause
 * of the problem can be hunted down.
 *
 * Quick fix?  Looks like a good solution for a lot of problems.
 *
 */
void update_aris( CHAR_DATA *ch )
{
	AFFECT_DATA *paf;
	OBJ_DATA	*obj;
	int			 hiding;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "update_aris: ch è NULL" );
		return;
	}

	/* Temp mod to bypass admins so they can keep their mset affects,
	 * just a band-aid until we get more time to look at it */
	if ( IS_MOB(ch) || IS_ADMIN(ch) )
		return;

	/* So chars using hide skill will continue to hide */
	hiding = HAS_BIT( ch->affected_by, AFFECT_HIDE );

	CLEAR_BITS( ch->affected_by );
	CLEAR_BITS( ch->resistant );
	CLEAR_BITS( ch->immune );
	CLEAR_BITS( ch->susceptible );

	CLEAR_BITS( ch->no_affected_by );
	CLEAR_BITS( ch->no_resistant );
	CLEAR_BITS( ch->no_immune );
	CLEAR_BITS( ch->no_susceptible );

	/* Add in effects from race */
	SET_BITS( ch->affected_by, table_race[ch->race]->affected );
	SET_BITS( ch->resistant, table_race[ch->race]->resist );
	SET_BITS( ch->susceptible, table_race[ch->race]->suscept );

	/* Add in effect from spells */
	for ( paf = ch->first_affect;  paf;  paf = paf->next )
		aris_affect( ch, paf );

	/* Add in effects from equipment */
	for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
	{
		if ( obj->wear_loc != WEARLOC_NONE )
		{
			for ( paf = obj->first_affect;  paf;  paf= paf->next )
				aris_affect( ch, paf );
		}
	}

	/* Add in effects from the room */
	if ( ch->in_room )	/* non-existant char booboo-fix */
	{
		for ( paf = ch->in_room->first_affect;  paf;  paf = paf->next )
			aris_affect( ch, paf );
	}

	/* Add in effects for polymorph */
	if ( ch->morph )
	{
		SET_BITS( ch->affected_by, ch->morph->affected_by );
		SET_BITS( ch->immune, ch->morph->immune );
		SET_BITS( ch->resistant, ch->morph->resistant );
		SET_BITS( ch->susceptible, ch->morph->suscept );

		/* Right now only morphs have no_ things */
		SET_BITS( ch->no_affected_by, ch->morph->no_affected_by );
		SET_BITS( ch->no_immune, ch->morph->no_immune );
		SET_BITS( ch->no_resistant, ch->morph->no_resistant );
		SET_BITS( ch->no_susceptible, ch->morph->no_suscept );
	}

	/* If they were hiding before, make them hiding again */
	if ( hiding )
		SET_BIT( ch->affected_by, AFFECT_HIDE );
}


void update_affect( void )
{
	CHAR_DATA	*ch;
	CHAR_DATA	*ch_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for ( ch = first_char;  ch;  ch = ch_next )
	{
		set_cur_char( ch );
		ch_next = ch->next;

		for ( paf = ch->first_affect;  paf;  paf = paf_next )
		{
			paf_next = paf->next;

			if ( paf->duration > 0 )
			{
				paf->duration--;
				continue;
			}

			if ( paf->duration < 0 )
				continue;

			if ( !paf_next
			  || paf_next->type != paf->type
			  || paf_next->duration > 0 )
			{
				SKILL_DATA *skill;

				skill = get_skilltype( paf->type );

				if ( paf->type > 0 && skill && VALID_STR(skill->msg_off) )
				{
					set_char_color( AT_WEAROFF, ch );
					ch_printf( ch, "%s\r\n", skill->msg_off );
				}
			}

			if ( paf->type == gsn_possess )
			{
				ch->desc->character			  = ch->desc->original;
				ch->desc->original			  = NULL;
				ch->desc->character->desc	  = ch->desc;
				ch->desc->character->switched = NULL;
				ch->desc					  = NULL;
			}

			affect_remove( ch, paf );
		}
	}

	tail_chain( );
}


void room_affect( ROOM_DATA *pRoom, AFFECT_DATA *paf, bool fAdd )
{
	if ( fAdd )
	{
		switch ( paf->location )
		{
		  case APPLY_ROOMFLAG:
		  case APPLY_SECTORTYPE:
			break;
		  case APPLY_ROOMLIGHT:
			pRoom->light += paf->modifier;
			break;
		  case APPLY_TELEVNUM:
		  case APPLY_TELEDELAY:
			break;
		}
	}
	else
	{
		switch ( paf->location )
		{
		  case APPLY_ROOMFLAG:
		  case APPLY_SECTORTYPE:
			break;
		  case APPLY_ROOMLIGHT:
			pRoom->light -= paf->modifier;
			break;
		  case APPLY_TELEVNUM:
		  case APPLY_TELEDELAY:
			break;
		}
	}
}


/*
 * Show an affect verbosely to a character
 */
void show_affect( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	char buf[MSL];
	int x;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "show_affect: ch è NULL" );
		return;
	}

	if ( !paf )
	{
		send_log( NULL, LOG_BUG, "show_affect: NULL paf" );
		return;
	}

	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
		switch ( paf->location )
		{
		  default:
			sprintf( buf, "Affects %s by %d.\r\n",
				code_name(NULL, paf->location, CODE_APPLY), paf->modifier );
			break;

		  case APPLY_AFFECT:
			sprintf( buf, "Affects %s by", code_name(NULL, paf->location, CODE_APPLY) );
			for ( x = 0;  x < 32;  x++ )	/* (FF) da mettere a posto */
			{
				if ( HAS_BITINT(paf->modifier, x) )
				{
					strcat( buf, " " );
					strcat( buf, code_name(NULL, x, CODE_AFFECT) );
				}
			}
			strcat( buf, "\r\n" );
			break;

		  case APPLY_WEAPONSPELL:
		  case APPLY_WEARSPELL:
		  case APPLY_REMOVESPELL:
			sprintf( buf, "Casts spell '%s'\r\n", is_valid_sn(paf->modifier)  ?
				table_skill[paf->modifier]->name  :  "unknown" );
			break;

		  case APPLY_RESISTANT:
		  case APPLY_IMMUNE:
		  case APPLY_SUSCEPTIBLE:
			sprintf( buf, "Affects %s by", code_name(NULL, paf->location, CODE_APPLY) );
			for ( x = 0;  x < 32;  x++ )	/* (FF) da mettere a posto */
			{
				if ( HAS_BITINT(paf->modifier, x) )
				{
					strcat( buf, " " );
					strcat( buf, code_name(NULL, x, CODE_RIS) );
				}
			}
			strcat( buf, "\r\n" );
			break;
		}
		send_to_char( ch, buf );
	}
}


/*
 * Ritorna TRUE se il pg è affetto da un incantesimo o da una skill
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;

	for ( paf = ch->first_affect;  paf;  paf = paf->next )
	{
		if ( paf->type == sn )
			return TRUE;
	}

	return FALSE;
}


/*
 * Affects-at-a-glance
 */
DO_RET do_affected( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *aff;

	if ( IS_MOB(ch) )
		return;

	set_char_color( AT_SCORE, ch );

	if ( is_name(argument, "da by") )
	{
		send_to_char( ch, "\r\n&BImpregnato da" );
		if ( !IS_EMPTY(ch->affected_by) )
			ch_printf( ch, ":\r\n&C%s\r\n", code_bit(NULL, ch->affected_by, CODE_AFFECT) );
		else
			send_to_char( ch, " nulla\r\n" );

		if ( IS_ADMIN(ch) )
		{
			send_to_char( ch, "\r\n" );

			if ( !IS_EMPTY(ch->resistant)	)	ch_printf( ch, "&BResisteze:      &C%s\r\n", code_bit(NULL, ch->resistant, CODE_RIS) );
			if ( !IS_EMPTY(ch->immune)		)	ch_printf( ch, "&BImmunità:       &C%s\r\n", code_bit(NULL, ch->immune, CODE_RIS) );
			if ( !IS_EMPTY(ch->susceptible) )	ch_printf( ch, "&BSuscettibilità: &C%s\r\n", code_bit(NULL, ch->susceptible, CODE_RIS) );
		}
		return;
	}

	if ( !ch->first_affect )
	{
		send_to_char( ch, "&CNon sento nessuna affezione su di me.\r\n&w" );
		return;
	}

	send_to_char( ch, "&g-----------------------------------------------\r\n" );
	for ( aff = ch->first_affect;  aff;  aff = aff->next )
	{
		SKILL_DATA	*skill;

		skill = get_skilltype( aff->type );
		if ( skill )
		{
			set_char_color( AT_SCORE, ch );
			send_to_char( ch, "| &wAffetto da:  " );

			if ( IS_ADMIN(ch) && aff->duration >= 0 )
			{
				if ( aff->duration < 75 )		set_char_color( AT_WHITE, ch );
				if ( aff->duration < 18 )		set_char_color( AT_WHITE + AT_BLINK, ch );

				ch_printf( ch, "(%5d)   ", aff->duration );
			}

			set_char_color( AT_SCORE, ch );
			ch_printf( ch, "%-20s &g|\r\n", skill->name );
		}
	}
	send_to_char( ch, "-----------------------------------------------\r\n" );
}
