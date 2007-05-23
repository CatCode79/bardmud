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
 >							Modulo Spell handling							<
\****************************************************************************/


#include <ctype.h>
#include <dlfcn.h>		/* libdl */

#include "mud.h"
#include "affect.h"
#include "calendar.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "gramm.h"
#include "group.h"
#include "interpret.h"
#include "liquid.h"
#include "movement.h"
#include "magic.h"
#include "morph.h"
#include "room.h"
#include "save.h"
#include "timer.h"
#include "track.h"


/*
 * Tabella parsing sugli effetti di save
 */
const CODE_DATA code_spellsave[] =
{
	{ SPELLSAVE_NONE,		"SPELLSAVE_NONE",		"none"			},
	{ SPELLSAVE_NEGATE,		"SPELLSAVE_NEGATE",		"negate"		},
	{ SPELLSAVE_EIGHTHDAM,	"SPELLSAVE_EIGHTHDAM",	"eightdam"		},
	{ SPELLSAVE_QUARTERDAM,	"SPELLSAVE_QUARTERDAM",	"quarterdam"	},
	{ SPELLSAVE_HALFDAM,	"SPELLSAVE_HALFDAM",	"halfdam"		},
	{ SPELLSAVE_3QTRDAM,	"SPELLSAVE_3QTRDAM",	"3qtrdam"		},
	{ SPELLSAVE_REFLECT,	"SPELLSAVE_REFLECT",	"reflect"		},
	{ SPELLSAVE_ABSORB,		"SPELLSAVE_ABSORB",		"absorb"		}
};
const int max_check_spellsave = sizeof(code_spellsave)/sizeof(CODE_DATA);

/*
 * Tabella di parsing sul danno degli incantesimi.
 */
const CODE_DATA code_spelldamage[] =
{
	{ SPELLDAMAGE_NONE,			"SPELLDAMAGE_NONE",			"none"			},
	{ SPELLDAMAGE_FIRE,			"SPELLDAMAGE_FIRE",			"fire"			},
	{ SPELLDAMAGE_COLD,			"SPELLDAMAGE_COLD",			"cold"			},
	{ SPELLDAMAGE_ELECTRICITY,	"SPELLDAMAGE_ELECTRICITY",	"electricity"	},
	{ SPELLDAMAGE_ENERGY,		"SPELLDAMAGE_ENERGY",		"energy"		},
	{ SPELLDAMAGE_ACID,			"SPELLDAMAGE_ACID",			"acid"			},
	{ SPELLDAMAGE_POISON,		"SPELLDAMAGE_POISON",		"poison"		},
	{ SPELLDAMAGE_DRAIN,		"SPELLDAMAGE_DRAIN",		"drain"			}
};
const int max_check_spelldamage = sizeof(code_spelldamage)/sizeof(CODE_DATA);


/*
 * Tabella di parsing sulle azioni degli incantesimi.
 */
const CODE_DATA code_spellaction[] =
{
	{ SPELLACTION_NONE,		"SPELLACTION_NONE",		"none"		},
	{ SPELLACTION_CREATE,	"SPELLACTION_CREATE",	"create"	},
	{ SPELLACTION_DESTROY,	"SPELLACTION_DESTROY",	"destroy"	},
	{ SPELLACTION_RESIST,	"SPELLACTION_RESIST",	"resist"	},
	{ SPELLACTION_SUSCEPT,	"SPELLACTION_SUSCEPT",	"suscept"	},
	{ SPELLACTION_DIVINATE,	"SPELLACTION_DIVINATE",	"divinate"	},
	{ SPELLACTION_OBSCURE,	"SPELLACTION_OBSCURE",	"obscure"	},
	{ SPELLACTION_CHANGE,	"SPELLACTION_CHANGE",	"change"	}
};
const int max_check_spellaction = sizeof(code_spellaction)/sizeof(CODE_DATA);


/*
 * Tabella di parsing sul potere degli incantesimi.
 */
const CODE_DATA code_spellpower[] =
{
	{ SPELLPOWER_NONE,		"SPELLPOWER_NONE",		"none"		},
	{ SPELLPOWER_MINOR,		"SPELLPOWER_MINOR",		"minor"		},
	{ SPELLPOWER_GREATER,	"SPELLPOWER_GREATER",	"greater"	},
	{ SPELLPOWER_MAJOR,		"SPELLPOWER_MAJOR",		"major"		}
};
const int max_check_spellpower = sizeof(code_spellpower)/sizeof(CODE_DATA);


/*
 * Tabella di parsing sulle classi di incantesimi.
 */
const CODE_DATA code_spellsphere[] =
{
	{ SPELLSPHERE_NONE,		"SPELLSPHERE_NONE",		"none"		},
	{ SPELLSPHERE_LUNAR,	"SPELLSPHERE_LUNAR",	"lunar"		},
	{ SPELLSPHERE_SOLAR,	"SPELLSPHERE_SOLAR",	"solar"		},
	{ SPELLSPHERE_TRAVEL,	"SPELLSPHERE_TRAVEL",	"travel"	},
	{ SPELLSPHERE_SUMMON,	"SPELLSPHERE_SUMMON",	"summon"	},
	{ SPELLSPHERE_LIFE,		"SPELLSPHERE_LIFE",		"life"		},
	{ SPELLSPHERE_DEATH,	"SPELLSPHERE_DEATH",	"death"		},
	{ SPELLSPHERE_ILLUSION,	"SPELLSPHERE_ILLUSION",	"illusion"	}
};
const int max_check_spellsphere = sizeof(code_spellsphere)/sizeof(CODE_DATA);


void fread_spell( MUD_FILE *fp )
{
	fread_skill( fp, "SPELL" );
}


/*
------------------------------------------------------------------------------
 * DLSym Snippet.
 * Written by Trax of Forever's End
------------------------------------------------------------------------------
 */
SPELL_FUN *spell_function( const char *name )
{
	void		*funHandle;
	const char	*error;

	funHandle = dlsym( sysdata.dlHandle, name );
	if ( (error = dlerror( )) != NULL )
	{
		send_log( NULL, LOG_BUG, "Error locating %s in symbol table. %s", name, error );
		return spell_notfound;
	}

	return (SPELL_FUN *) funHandle;
}


/*
 * Carica e controlla tutti gli incantesimi
 */
void load_spells( void )
{
	fread_section( SPELL_FILE, "SPELL", fread_spell, FALSE );
}


/*
 * Controlla se una skill possiede la flag passata (ex-macro SPELL_FLAG)
 */
bool SPELL_FLAG( SKILL_DATA *skill, int flag )
{
	if ( !skill )
	{
		send_log( NULL, LOG_BUG, "SPELL_FLAG: skill è NULL" );
		return FALSE;
	}

	if ( flag < 0 && flag >= MAX_SKELL )
	{
		send_log( NULL, LOG_BUG, "SPELL_FLAG: flag errata: %d", flag );
		return FALSE;
	}

	if ( HAS_BITINT(skill->flags, flag) )
		return TRUE;

	return FALSE;
}


/*
 * Controlla se la classe può formulare incantesimo o no.
 * (FF) da generalizzare in maniera totalmente diversa.
 */
bool can_cast( CHAR_DATA *ch )
{
	if ( ch->class != CLASS_THIEF && ch->class != CLASS_FIGHTER )
		return TRUE;

	return FALSE;
}


int find_spell( CHAR_DATA *ch, const char *name, bool know )
{
	if ( IS_MOB(ch) || !know )
		return bsearch_skill( name, gsn_first_spell, gsn_first_skill-1 );
	else
		return ch_bsearch_skill( ch, name, gsn_first_spell, gsn_first_skill-1 );
}


/*
 * RIS by gsn lookups
 * Will need to add some || stuff for spells that need a special GSN
 * sn è uno spell
 * sd è un codice di SPELLDAMAGE_
 */
bool is_spelldamage( const int sn, const int sd )
{
	if ( !is_valid_sn(sn) )
	{
		send_log( NULL, LOG_BUG, "is_spelldamage: sn passato errato: %d", sn );
		return FALSE;
	}

	if ( sd < 0 || sd >= MAX_SPELLDAMAGE )
	{
		send_log( NULL, LOG_BUG, "is_spelldamage: sd passato errato: %d", sd );
		return FALSE;
	}

	if ( SPELL_DAMAGE(table_skill[sn]) == sd )
		return TRUE;

	return FALSE;
}


/*
 * Handler to tell the victim which spell is being affected.
 */
int dispel_casting( AFFECT_DATA *paf, CHAR_DATA *ch, CHAR_DATA *victim, int affect, bool dispel )
{
	SKILL_DATA *sktmp;
	char	   *spell;
	char		buf[MSL];
	bool 		is_mage	   = FALSE;
	bool		has_detect = FALSE;

	if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_MAGIC) )
		has_detect = TRUE;

	if ( IS_MOB(ch) || HAS_BIT(table_class[ch->class]->flags, CLASSFLAG_CASTER) || IS_ADMIN(ch) )
		is_mage = TRUE;

	if ( paf )
	{
		if ( (sktmp = get_skilltype(paf->type)) == NULL )
			return 0;
		spell = sktmp->name;
	}
	else
	{
		BITVECTOR *bv = meb( affect );
		spell = code_bit( NULL, bv, CODE_AFFECT );
		destroybv( bv );
	}

	set_char_color( AT_MAGIC, ch );
	set_char_color( AT_HITME, victim );

	if ( !can_see(ch, victim) )
	{
		strcpy( buf, "Qualcuno" );
	}
	else
	{
		strcpy( buf, (IS_MOB(victim)  ?  victim->short_descr  :  victim->name) );
		buf[0] = toupper( buf[0] );
	}

	if ( dispel )
	{
		ch_printf( victim, "Il mio %s svanisce.\r\n", spell );

		if ( is_mage && has_detect )
			ch_printf( ch, "L'incantesimo %s di %s svanisce.\r\n", buf, spell );
		else
			return 0;			/* Passa il messaggio Ok di default */
	}
	else
	{
		if ( is_mage && has_detect )
			ch_printf( ch, "L'incantesimo %s di %s vacilla un poco ma ancora resiste.\r\n", buf, spell );
		else
			return 0;			/* Passa il messaggio di fallimento */
	}

	return 1;
}


/*
 * Fancy message handling for a successful casting
 */
void successful_casting( SKILL_DATA *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	int chitroom = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_ACTION );
	int chit	 = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_HIT	 );
	int chitme	 = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_HITME	 );

	if ( skill->target != TARGET_CHAR_OFFENSIVE )
	{
		chit   = chitroom;
		chitme = chitroom;
	}

	if ( ch && ch != victim )
	{
		if		( VALID_STR(skill->hit_char) )
			act( AT_MAGIC, skill->hit_char, ch, obj, victim, TO_CHAR );
		else if ( skill->type == SKILLTYPE_SPELL )
			act( AT_MAGIC, "Ok.", ch, NULL, NULL, TO_CHAR );
	}

	if ( ch && VALID_STR(skill->hit_room) )
		act( AT_MAGIC, skill->hit_room, ch, obj, victim, TO_NOVICT );

	if ( ch && victim && VALID_STR(skill->hit_vict) )
	{
		if ( ch != victim )
			act( AT_MAGIC, skill->hit_vict, ch, obj, victim, TO_VICT );
		else
			act( AT_MAGIC, skill->hit_vict, ch, obj, victim, TO_CHAR );
	}
	else if ( ch && ch == victim && SKILLTYPE_SPELL == skill->type )
		act( AT_MAGIC, "Ok.", ch, NULL, NULL, TO_CHAR );
}


/*
 * Fancy message handling for a failed casting
 */
void failed_casting( SKILL_DATA *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	int chitroom = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_ACTION );
	int chit	 = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_HIT	   );
	int chitme   = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_HITME  );

	if ( skill->target != TARGET_CHAR_OFFENSIVE )
	{
		chit   = chitroom;
		chitme = chitroom;
	}

	if ( ch && ch != victim )
	{
		if ( VALID_STR(skill->miss_char) )
			act( AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR );
		else if ( skill->type == SKILLTYPE_SPELL )
			act( chitme, "Ho fallito.", ch, NULL, NULL, TO_CHAR );
	}

	/* Back Compat */
	if ( ch && VALID_STR(skill->miss_room) && str_cmp(skill->miss_room, "supress") )
		act( AT_MAGIC, skill->miss_room, ch, obj, victim, TO_NOVICT );

	if ( ch && victim && VALID_STR(skill->miss_vict) )
	{
		if ( ch != victim )
			act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_VICT );
		else
			act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_CHAR );
	}
	else if ( ch && ch == victim )
	{
		if ( VALID_STR(skill->miss_char) )
			act( AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR );
		else if ( skill->type == SKILLTYPE_SPELL )
			act( chitme, "Ho fallito.", ch, NULL, NULL, TO_CHAR );
	}
}


/*
 * Fancy message handling for being immune to something
 */
void immune_casting( SKILL_DATA *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	int chitroom = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_ACTION );
	int chit	 = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_HIT	 );
	int chitme   = ( (skill->type == SKILLTYPE_SPELL)  ?  AT_MAGIC  :  AT_HITME  );

	if ( skill->target != TARGET_CHAR_OFFENSIVE )
	{
		chit   = chitroom;
		chitme = chitroom;
	}

	if ( ch && ch != victim )
	{
		if ( VALID_STR(skill->imm_char) )
			act( AT_MAGIC, skill->imm_char, ch, obj, victim, TO_CHAR );
		else if ( VALID_STR(skill->miss_char) )
			act( AT_MAGIC, skill->hit_char, ch, obj, victim, TO_CHAR );
		else if ( skill->type == SKILLTYPE_SPELL || skill->type == SKILLTYPE_SPELL )
			act( chit, "Il sortilegio non sembra avere alcun effetto!", ch, NULL, NULL, TO_CHAR );
	}

	if ( ch && VALID_STR(skill->imm_room) )
		act( AT_MAGIC, skill->imm_room, ch, obj, victim, TO_NOVICT );
	else if ( ch && VALID_STR(skill->miss_room) )
		act( AT_MAGIC, skill->miss_room, ch, obj, victim, TO_NOVICT );

	if ( ch && victim && VALID_STR(skill->imm_vict) )
	{
		if ( ch != victim )
			act( AT_MAGIC, skill->imm_vict, ch, obj, victim, TO_VICT );
		else
			act( AT_MAGIC, skill->imm_vict, ch, obj, victim, TO_CHAR );
	}
	else if ( ch && victim && VALID_STR(skill->miss_vict) )
	{
		if ( ch != victim )
			act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_VICT );
		else
			act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_CHAR );
	}
	else if ( ch && ch == victim )
	{
		if ( VALID_STR(skill->imm_char) )
			act( AT_MAGIC, skill->imm_char, ch, obj, victim, TO_CHAR );
		else if ( VALID_STR(skill->miss_char) )
			act( AT_MAGIC, skill->hit_char, ch, obj, victim, TO_CHAR );
		else if ( skill->type == SKILLTYPE_SPELL || skill->type == SKILLTYPE_SKILL )
			act( chit, "Il sortilegio non sembra avere alcun effetto!", ch, NULL, NULL, TO_CHAR );
	}
}


/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
	CHAR_DATA	*rch;
	char		*pName;
	SKILL_DATA  *skill = get_skilltype( sn );
	char		 buf [MSL];
	char		 buf2[MSL];
	int			 iSyl;
	int			 length;

	struct syl_type
	{
		char *old;
		char *new;
	};

	/* (FF) prendere la parte dello xandra, e farlo come lingua poi */
	static const struct syl_type table_syl[] =
	{
		{ " ",			" "			},
		{ "ar",			"abra"		},
		{ "au",			"kada"		},
		{ "bless",		"fido"		},
		{ "blind",		"nose"		},
		{ "bur",		"mosa"		},
		{ "cu",			"judi"		},
		{ "de",			"oculo"		},
		{ "en",			"unso"		},
		{ "light",		"dies"		},
		{ "lo",			"hi"		},
		{ "mor",		"zak"		},
		{ "move",		"sido"		},
		{ "ness",		"lacri"		},
		{ "ning",		"illa"		},
		{ "per",		"duda"		},
		{ "polymorph",	"iaddahs"	},
		{ "ra",			"gru"		},
		{ "re",			"candus"	},
		{ "son",		"sabru"		},
		{ "tect",		"infra"		},
		{ "tri",		"cula"		},
		{ "ven",		"nofo"		},

		{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
		{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
		{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
		{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
		{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
		{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
		{ "y", "l" }, { "z", "k" },
		{ "", "" }
	};

	buf[0]	= '\0';
	for ( pName = skill->name;  VALID_STR(pName);  pName += length )
	{
		for ( iSyl = 0;  (length = strlen(table_syl[iSyl].old)) != 0;  iSyl++ )
		{
			if ( !str_prefix(table_syl[iSyl].old, pName) )
			{
				strcat( buf, table_syl[iSyl].new );
				break;
			}
		}
		if ( length == 0 )
			length = 1;
	}

	sprintf( buf2, "$n pronuncia le parole, '%s'.", buf		 );
	sprintf( buf,  "$n pronuncia le parole, '%s'.", skill->name );

	for ( rch = ch->in_room->first_person;  rch;  rch = rch->next_in_room )
	{
		if ( rch != ch )
			act( AT_MAGIC, (ch->class == rch->class)  ?  buf  :  buf2, ch, NULL, rch, TO_VICT );
	}
}


/*
 * Make adjustments to saving throw based in RIS
 */
int ris_save( CHAR_DATA *ch, int ris_chance, int ris )
{
	int modifier;

	modifier = 10;

	if ( HAS_BIT(ch->immune, ris) )
		modifier -= 10;

	if ( HAS_BIT(ch->resistant, ris) )
		modifier -= 2;

	if ( HAS_BIT(ch->susceptible, ris) )
	{
		if ( IS_MOB(ch) && HAS_BIT(ch->immune, ris) )
			modifier += 0;
		else
			modifier += 2;
	}

	if ( modifier <= 0 )
		return 1000;

	if ( modifier == 10 )
		return ris_chance;

	return (ris_chance * modifier) / 10;
}


/*
 * (FF) da rifare, per T2 le formule sono più complicate
 *
 * Fancy dice expression parsing complete with order of operations,
 *	simple exponent support, dice support as well as a few extra
 * variables: L = level, H = hp, M = mana, V = move, U = Soul, <-- (GG)
 *			  S = str, X = agi, I = int, W = wis, C = con, A = cha, Y = age
 *
 * Used for spell dice parsing, ie: 3d8+L-6
 */
int rd_parse( CHAR_DATA *ch, int level, char *experience )
{
	char *sexp[2];
	char  operation;
	int	  x;
	int	  lop = 0;
	int	  gop = 0;
	int	  eop = 0;
	int   total = 0;
	int	  len   = 0;

	/* take care of nulls coming in */
	if ( !VALID_STR(experience) || !strlen(experience) )
		return 0;

	/* get rid of brackets if they surround the entire expresion */
	if (
			(*experience == '(')
			&& !strchr(experience+1, '(')
			&& experience[strlen(experience)-1] == ')'
		)
	{
		experience[strlen(experience)-1] = '\0';
		experience++;
	}

	/* check if the expression is just a number */
	len = strlen( experience );

	if ( len == 1 && isalpha(experience[0]) )
	{
		switch ( experience[0] )
		{
		  case 'L': case 'l':		return level;
		  case 'H': case 'h':		return ch->points[POINTS_LIFE];
		  case 'M': case 'm':		return ch->points[POINTS_MANA];
		  case 'V': case 'v':		return ch->points[POINTS_MOVE];
/*		  case 'U': case 'u':		return ch->points[POINTS_SOUL]; (GG) */

		  case 'S': case 's':		return get_curr_attr( ch, ATTR_STR );	/* (RR) devo aggiungere gli altri */
		  case 'I': case 'i':		return get_curr_attr( ch, ATTR_INT );
		  case 'W': case 'w':		return get_curr_attr( ch, ATTR_WIL );
		  case 'X': case 'x':		return get_curr_attr( ch, ATTR_AGI );
		  case 'C': case 'c':		return get_curr_attr( ch, ATTR_CON );
		  case 'A': case 'a':		return get_curr_attr( ch, ATTR_BEA );

		  case 'Y': case 'y':		return get_age( ch, TIME_YEAR );
		}
	}

	for ( x = 0;  x < len;  ++x )
	{
		if ( !isdigit(experience[x]) && !isspace(experience[x]) )
			break;
	}

	if ( x == len )
		return atoi( experience );

	/* Break it into 2 parts */
	for ( x = 0;  x < (int)strlen(experience);  ++x )
	{
		switch ( experience[x] )
		{
		  case '^':
			if ( !total )
				eop = x;
			break;

		  case '-': case '+':
			if ( !total )
				lop = x;
			break;

		  case '*': case '/': case '%': case 'd': case 'D':
		  case '<': case '>': case '{': case '}': case '=':
			if ( !total )
				gop =  x;
			break;

		  case '(':
			++total;
			break;

		  case ')':
			--total;
			break;
		}
	}

	if (lop)
		x = lop;
	else if (gop)
		x = gop;
	else
		x = eop;

	operation = experience[x];
	experience[x] 	= '\0';
	sexp[0] = experience;
	sexp[1] = (char *)(experience + x + 1);

	/* work it out */
	total = rd_parse( ch, level, sexp[0] );
	switch ( operation )
	{
	  case '-':				total -=			  rd_parse( ch, level, sexp[1]  );		break;
	  case '+':				total +=			  rd_parse( ch, level, sexp[1]  );		break;
	  case '*':				total *=			  rd_parse( ch, level, sexp[1]  );		break;
	  case '/':				total /=			  rd_parse( ch, level, sexp[1]  );		break;
	  case '%':				total %=			  rd_parse( ch, level, sexp[1]  );		break;
	  case 'd': case 'D':	total  = dice( total, rd_parse( ch, level, sexp[1]) );		break;
	  case '<':				total  = ( total <	  rd_parse( ch, level, sexp[1]) );		break;
	  case '>':				total  = ( total >	  rd_parse( ch, level, sexp[1]) );		break;
	  case '=':				total  = ( total ==	  rd_parse( ch, level, sexp[1]) );		break;
	  case '{':				total  = UMIN( total, rd_parse( ch, level, sexp[1]) );		break;
	  case '}':				total  = UMAX( total, rd_parse( ch, level, sexp[1]) );		break;

	  case '^':
	  {
			int y = rd_parse( ch, level, sexp[1] ),
				z = total;

			for ( x = 1;  x < y;  ++x, z *= total )
				;

			total = z;

			break;
	  }
	} /* chiude lo switch */

	return total;
}


/*
 * Wrapper function so as not to destroy exprience
 * Il valore level passato deve essere già diviso per due
 */
int dice_parse( CHAR_DATA *ch, int level, char *experience )
{
	char buf[MIL];

	strcpy( buf, experience );
	return rd_parse( ch, level, buf );
}


/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 * Il valore level passato deve essere già diviso per due
 */
bool compute_savthrow( int level, CHAR_DATA *victim, int saving )
{
	int save;

	save = 50 + ( get_level(victim)/2 - level - victim->saving_throw[saving] ) * 5;
	save = URANGE( 5, save, 95 );

	return chance( victim, save );
}


/*
 * Process the spell's required components, if any
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */
bool process_components( CHAR_DATA *ch, int sn )
{
	SKILL_DATA	*skill	= get_skilltype(sn);
	char		*comp	= skill->components;
	char		*check;
	char		 arg[MIL];
	bool		 consume, fail, found;
	int			 val, value;
	OBJ_DATA	*obj;

	/* if no components necessary, then everything is cool */
	if ( !VALID_STR(comp) )
		return TRUE;

	while ( VALID_STR(comp) )
	{
		comp	= one_argument( comp, arg );
		consume = TRUE;
		fail	= found = FALSE;
		val		= -1;

		switch ( arg[1] )
		{
		  default:	check = arg+1;						break;
		  case '!':	check = arg+2;	fail = TRUE;		break;
		  case '+':	check = arg+2;	consume = FALSE;	break;
		  case '@':	check = arg+2;	val = 0;			break;
		  case '#':	check = arg+2;	val = 1;			break;
		  case '$':	check = arg+2;	val = 2;			break;
		  case '%':	check = arg+2;	val = 3;			break;
		  case '^':	check = arg+2;	val = 4;			break;
		  case '&':	check = arg+2;	val = 5;			break;
		  /*  reserve '*', '(' and ')' for v6, v7 and v8  */
		}
		value = atoi( check );
		obj = NULL;

		switch ( toupper(arg[0]) )
		{
		  case 'T':
			for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
			{
				if ( obj->type == value )
				{
					if ( fail )
					{
						if ( skill->type == SKILLTYPE_SKILL )
							send_to_char( ch, "Qualcosa m'impedisce l'utilizzo di questa abilità.\r\n" );
						else
							send_to_char( ch, "Al culmine del sortilegio un senso di vuoto mi circonda.. l'incantesimo è perduto.\r\n" );
						return FALSE;
					}
					found = TRUE;
					break;
				}
			}
			break;

		  case 'V':
			for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
			{
				if ( obj->pObjProto->vnum == value )
				{
					if ( fail )
					{
						if ( skill->type == SKILLTYPE_SKILL )
							send_to_char( ch, "Qualcosa m'impedisce l'utilizzo di questa abilità.\r\n" );
						else
							send_to_char( ch, "Un forte senso di disagio mi pervade.. la concentrazione sfuma e l'incantesimo è perduto.\r\n" );
						return FALSE;
					}
					found = TRUE;
					break;
				}
			}
			break;

		  case 'K':
			for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
			{
				if ( nifty_is_name(check, obj->name) )
				{
					if ( fail )
					{
						if ( skill->type == SKILLTYPE_SKILL )
							send_to_char( ch, "Qualcosa m'impedisce l'utilizzo di questa abilità.\r\n" );
						else
							send_to_char( ch, "Un brivido gelido mi attraversa la mente, l'incantesimo sfuma in un soffio..\r\n" );
						return FALSE;
					}
					found = TRUE;
					break;
				}
			}
			break;

		  case 'G':
			if ( ch->gold >= value )
			{
				if ( fail )
				{
					if ( skill->type == SKILLTYPE_SKILL )
						send_to_char( ch, "Qualcosa m'impedisce l'utilizzo di questa abilità.\r\n" );
					else
						send_to_char( ch, "Una forza misteriosa si oppone al ciclo del mio incantesimo.. che sfuma irrimediabilmente...\r\n" );
					return FALSE;
				}
				else
				{
					if ( consume )
					{
						set_char_color( AT_GOLD, ch );
						send_to_char( ch, "Una sottile luce attraversa la mia mente..\r\n" );
						ch->gold -= value;
					}
					continue;
				}
			}
			break;

		  case 'H':
			if ( ch->points[POINTS_LIFE] >= value )
			{
				if ( fail )
				{
					if ( skill->type == SKILLTYPE_SKILL )
						send_to_char( ch, "Qualcosa m'impedisce l'utilizzo di questa abilità.\r\n" );
					else
						send_to_char( ch, "Un'improvvisa mutazione nel campo di forza circostante interrompe il ciclo del mio incantesimo..\r\n" );
					return FALSE;
				}
				else
				{
					if ( consume )
					{
						set_char_color( AT_DRED, ch );
						send_to_char( ch, "Un diffuso senso di debolezza mi pervade lentamente..\r\n" );
						ch->points[POINTS_LIFE] -= value;
						update_position( ch );
					}
					continue;
				}
			}
			break;
		} /* chiude lo switch */

		/* Having this component would make the spell fail... if we get
			here, then the caster didn't have that component */
		if ( fail )
			continue;

		if ( !found )
		{
			if ( skill->type == SKILLTYPE_SKILL )
				send_to_char( ch, "Forse mi manca qualcosa per poterlo fare.\r\n" );
			else
				send_to_char( ch, "Non tutti gli elementi dell'incantesimo sono pronti..\r\n" );
			return FALSE;
		}

		if ( obj )
		{
			if ( val >= 0 && val < 6 )
			{
				split_obj( obj, 1 );

#ifdef T2_ALFA
				/* (RR) (FF) (BB) è la volta che per questi value io mi sparo, ok? */
				if ( obj->value[val] <= 0 )
				{
					if ( skill->type == SKILLTYPE_SKILL )
					{
						act( AT_DGREY, "$p va in frantumi!", ch, obj, NULL, TO_CHAR );
						act( AT_DGREY, "$p va in frantumi!", ch, obj, NULL, TO_ROOM );
					}
					else
					{
						act( AT_MAGIC, "$p svanisce senza lasciare tracce!", ch, obj, NULL, TO_CHAR );
						act( AT_MAGIC, "$p svanisce senza lasciare tracce!", ch, obj, NULL, TO_ROOM );
					}
					free_object( obj );
					return FALSE;
				}
				else if ( --obj->value[val] == 0 )
				{
					if ( skill->type == SKILLTYPE_SKILL )
					{
						act( AT_DGREY, "$p cade in terra in mille frammenti!", ch, obj, NULL, TO_CHAR );
						act( AT_DGREY, "$p cade in terra in mille frammenti!", ch, obj, NULL, TO_ROOM );
					}
					else
					{
						act( AT_MAGIC, "$p risplende debolmente.. per svanire improvvisamente appena la luce si ritira..", ch, obj, NULL, TO_CHAR );
						act( AT_MAGIC, "$p risplende debolmente.. per svanire improvvisamente appena la luce si ritira..", ch, obj, NULL, TO_ROOM );
					}
					free_object( obj );
				}
				else
#endif
				{
					if ( skill->type == SKILLTYPE_SKILL )
						act( AT_DGREY, "Piccoli frammenti si staccano da $p.", ch, obj, NULL, TO_CHAR );
					else
						act( AT_MAGIC, "$p risplende debolmente per un attimo.", ch, obj, NULL, TO_CHAR );
				}
			}
			else if ( consume )
			{
				split_obj( obj, 1 );
				if ( skill->type == SKILLTYPE_SKILL )
				{
					act( AT_DGREY, "$p cade in terra in mille frammenti!", ch, obj, NULL, TO_CHAR );
					act( AT_DGREY, "$p cade in terra in mille frammenti!", ch, obj, NULL, TO_ROOM );
				}
				else
				{
					act( AT_MAGIC, "$p risplende di luce propria.. ma vibra e si frammenta in mille pezzi!", ch, obj, NULL, TO_CHAR );
					act( AT_MAGIC, "$p risplende di luce propria.. ma vibra e si frammenta in mille pezzi!", ch, obj, NULL, TO_ROOM );
				}
				free_object( obj );
			}
			else
			{
				int count = obj->count;

				obj->count = 1;
				if ( skill->type == SKILLTYPE_SKILL )
					act( AT_DGREY, "Piccoli frammenti si staccano da $p.", ch, obj, NULL, TO_CHAR );
				else
					act( AT_MAGIC, "$p risplende di luce propria.", ch, obj, NULL, TO_CHAR );
				obj->count = count;
			}
		} /* chiude l'if */
	} /* chiude il while */

	return TRUE;
}


int pAbort;

/*
 * Locate targets.
 */
/* Turn off annoying message and just abort if needed */
bool silence_locate_targets;

void *locate_targets( CHAR_DATA *ch, char *arg, int sn, CHAR_DATA **victim, OBJ_DATA **obj )
{
	SKILL_DATA  *skill = get_skilltype( sn );
	void	   *vo	  = NULL;

	*victim	= NULL;
	*obj	= NULL;

	switch ( skill->target )
	{
	  default:
		send_log( NULL, LOG_BUG, "locate_targets: bad target for sn %d.", sn );
		return &pAbort;

	  case TARGET_IGNORE:
		break;

	  case TARGET_CHAR_OFFENSIVE:
		if ( !VALID_STR(arg) )
		{
			if ( (*victim = who_fighting(ch)) == NULL )
			{
				if ( !silence_locate_targets )
					send_to_char( ch, "Su chi vuoi lanciare l'incantesimo?\r\n" );
				return &pAbort;
			}
		}
		else
		{
			*victim = get_char_room( ch, arg, TRUE );
			if ( !*victim )
			{
				if ( !silence_locate_targets )
					send_to_char( ch, "Il bersaglio che chiude il ciclo del sortilegio non è qui.\r\n" );
				return &pAbort;
			}
		}

		if ( is_safe(ch, *victim, TRUE) )
			return &pAbort;

		if ( ch == *victim )
		{
			if ( SPELL_FLAG(get_skilltype(sn), SKELL_NOSELF) )
			{
				if ( !silence_locate_targets )
					send_to_char( ch, "Questo incantesimo non ha effetto se bersaglio e utente magico coincidono!\r\n" );
				return &pAbort;
			}
			if ( !silence_locate_targets )
				send_to_char( ch, "Dovrei proprio farlo su di me? Meglio di no..\r\n" );
		}

		if ( IS_PG(ch) )
		{
			if ( IS_PG(*victim) )
			{
				if ( get_timer(ch, TIMER_PKILLED) > 0 || get_timer(ch, TIMER_KILLED) > 0 )
				{
					if ( !silence_locate_targets )
						send_to_char( ch, "La mia anima è ritornata in questo corpo solo da poco tempo.\r\n" );
					return &pAbort;
				}

				if ( get_timer(*victim, TIMER_PKILLED) > 0 || get_timer(*victim, TIMER_KILLED) > 0 )
				{
					if ( !silence_locate_targets )
						send_to_char( ch, "La sua anima è ritornata in quel corpo solo da poco tempo.\r\n" );
					return &pAbort;
				}

#ifdef T2_ARENA
				if ( *victim != ch && !in_arena(ch) )
#else
				if ( *victim != ch )
#endif
				{
					if ( !silence_locate_targets )
						send_to_char( ch, "Non è un'azione consigliabile su altri avventurieri..\r\n" );
					else if ( who_fighting(*victim) != ch )
						return &pAbort;		/* Only auto-attack those that are hitting you. */
				}
			}

			if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) && ch->master == *victim )
			{
				if ( !silence_locate_targets )
					send_to_char( ch, "Non è affatto consigliabile farlo proprio sulla mia preda..\r\n" );
				return &pAbort;
			}
		} /* chiude l'if */

		is_illegal_pk( ch, *victim );
		vo = (void *) *victim;
		break;

	  case TARGET_CHAR_DEFENSIVE:
	  {
		if ( !VALID_STR(arg) )
		{
			*victim = ch;
		}
		else
		{
			*victim = get_char_room( ch, arg, TRUE );
			if ( !*victim )
			{
				if (!silence_locate_targets)
					send_to_char( ch, "Non è qui.\r\n" );
				return &pAbort;
			}
		}
	  }

		if ( ch == *victim && SPELL_FLAG(get_skilltype(sn), SKELL_NOSELF) )
		{
			if ( !silence_locate_targets )
				send_to_char( ch, "Non è un incantesimo che abbia una utilità se lanciato su me stesso.\r\n" );
			return &pAbort;
		}
		vo = (void *) *victim;
		break;

	  case TARGET_CHAR_SELF:
		if ( VALID_STR(arg) && !nifty_is_name(arg, ch->name) )
		{
			if (!silence_locate_targets)
				send_to_char( ch, "Il potere di questo incantesimo non può transitare su altri.\r\n" );
			return &pAbort;
		}
		vo = (void *) ch;
		break;

	  case TARGET_OBJ_INV:
	  {
		if ( !VALID_STR(arg) )
		{
			if ( !silence_locate_targets )
				send_to_char( ch, "Devo decidere il bersaglio del tuo incantesimo per chiudere il suo ciclo.\r\n" );
			return &pAbort;
		}

		if ( (*obj = get_obj_carry(ch, arg, TRUE)) == NULL )
		{
			if ( !silence_locate_targets )
				send_to_char( ch, "Non ne sono in possesso.\r\n" );
			return &pAbort;
		}
	  }
		vo = (void *) *obj;
		break;
	} /* chiude lo switch */
	return vo;
}


/*
 * Incantesimo null.
 */
SPELL_RET spell_null( int sn, int level, CHAR_DATA *ch, void *vo )
{
	send_to_char( ch, "Non si tratta di un incantesimo.\r\n" );
	return rNONE;
}

/* Don't remove, may look redundant, but is important! */
SPELL_RET spell_notfound( int sn, int level, CHAR_DATA *ch, void *vo )
{
	send_to_char( ch, "Non sembra il nome di un incantesimo che conosca.\r\n" );
	return rNONE;
}


/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
char *target_name_ranged = NULL;


/*
 * Cast a spell.  Multi-caster and component supported.
 */
DO_RET do_cast( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		*victim;
	OBJ_DATA		*obj;
	SKILL_DATA		*skill = NULL;
	struct timeval	 time_used;
	void			*vo = NULL;
	static char 	 staticbuf[MIL];
	char			 arg1[MIL];
	char			 arg2[MIL];
	int				 mana;
	int				 sn;
	ch_ret			 retcode;
	bool			 dont_wait = FALSE;

	retcode = rNONE;

	switch ( ch->substate )
	{
	  default:
		/* no ordering charmed mobs to cast spells */
		if ( IS_MOB(ch)
		  && (HAS_BIT(ch->affected_by, AFFECT_CHARM)
		  ||  HAS_BIT(ch->affected_by, AFFECT_POSSESS)) )
		{
			send_to_char( ch, "Non posso ordinare una cosa simile..\r\n" );
			return;
		}

		if ( HAS_BIT(ch->in_room->flags, ROOM_NOMAGIC) )
		{
			if ( IS_ADMIN(ch) )
			{
				send_to_char( ch, "[ADM] Stanza nomagic, ma gli admin castano comunque.\r\n" );
			}
			else
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( ch, "Qualcosa intorno a me sembra interrompere il ciclo arcano..\r\n" );
				return;
			}
		}

		if ( HAS_BIT(ch->in_room->flags, ROOM_SILENCE) && !IS_ADMIN(ch) )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Apro la bocca per pronunciare l'incantesimo ma non riesco ad emettere suoni.\r\n" );
			return;
		}

		if ( ch->in_room->sector == SECTOR_UNDERWATER && !IS_ADMIN(ch) )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Trovo piuttosto difficile pronunciare incantesimi sott'acqua.\r\n" );
			return;
		}

		target_name = one_argument( argument, arg1 );
		one_argument( target_name, arg2 );

		DISPOSE( target_name_ranged );
		target_name_ranged = str_dup( target_name );

		if ( !VALID_STR(arg1) )
		{
			send_to_char( ch, "Tre domande: chi? dove? cosa?\r\n" );
			return;
		}

		/* Regular pg spell casting */
		if ( !IS_ADMIN(ch) )
		{
			if ( (sn = find_spell(ch, arg1, TRUE)) < 0
			  || (IS_PG(ch) && get_level(ch) < table_skill[sn]->skill_level[ch->class]) )
			{
				send_to_char( ch, "Non sono in grado di farlo.\r\n" );
				return;
			}

			if ( (skill = get_skilltype(sn)) == NULL )
			{
				send_to_char( ch, "Non sono ancora in grado di farlo..\r\n" );
				return;
			}
		}
		else
		{
			/*
			 * Godly "spell builder" spell casting with debugging messages
			 */
			sn = skill_lookup( arg1 );
			if ( sn < 0 )
			{
				send_to_char( ch, "Mai nessuno ha teorizzato quest'incantesimo..\r\n" );
				return;
			}

			if ( sn >= MAX_SKILL )
			{
				send_to_char( ch, "Ups.. questo si che potrebbe fare male..\r\n" );
				return;
			}

			skill = get_skilltype( sn );
			if ( !skill )
			{
				send_to_char( ch, "Avverto una lieve perturbazione nel campo magico..\r\n" );
				return;
			}

			if ( skill->type != SKILLTYPE_SPELL )
			{
				send_to_char( ch, "Non esiste alcun incantesimo con tale nome.\r\n" );
				return;
			}

			if ( !skill->spell_fun )
			{
				send_to_char( ch, "Si tratta di un incantesimo ancora non teorizzato..\r\n" );
				return;
			}
		}

		/*
		 * Something else removed by Merc
		 */
		/* Band-aid alert!  IS_PG check */
		if ( ch->position < skill->min_position && IS_PG(ch) )
		{
			switch ( ch->position )
			{
			  default:
				send_to_char( ch, "Non riesco a concentrarmi abbastanza..\r\n" );
				break;

			  case POSITION_SIT:
				send_to_char( ch, "Da questa posizione non riesco ad invocare abbastanza energia.\r\n" );
				break;

			  case POSITION_REST:
				send_to_char( ch, "Da questa posizione non riesco a concentrarmi abbastanza.\r\n" );
				break;

			  case POSITION_FIGHT:
				if ( skill->min_position <= POSITION_EVASIVE )
					send_to_char( ch, "Questo stile di combattimento non mi permette di concentrarmi adeguatamente!\r\n" );
				else
					send_to_char( ch, "Non posso farlo mentre combatto!\r\n" );
				break;

			  case POSITION_DEFENSIVE:
				if ( skill->min_position <= POSITION_EVASIVE )
					send_to_char( ch, "Questo stile di combattimento non mi permette di concentrarmi adeguatamente!\r\n" );
				else
					send_to_char( ch, "Non posso farlo mentre combatto!\r\n" );
				break;

			  case POSITION_AGGRESSIVE:
				if ( skill->min_position <= POSITION_EVASIVE )
					send_to_char( ch, "Questo stile di combattimento non mi permette di concentrarmi adeguatamente!\r\n" );
				else
					send_to_char( ch, "Non posso farlo mentre combatto!\r\n" );
				break;

			  case POSITION_BERSERK:
				if ( skill->min_position <= POSITION_EVASIVE )
					send_to_char( ch, "Questo stile di combattimento non mi permette di concentrarmi adeguatamente!\r\n" );
				else
					send_to_char( ch, "Non posso farlo mentre combatti!\r\n" );
				break;

			  case POSITION_EVASIVE:
				send_to_char( ch, "Non posso farlo mentre combatto!\r\n" );
				break;

			  case POSITION_SLEEP:
				send_to_char( ch, "L'energia magica mi accarezza sussurrandomi nel sogno..\r\n" );
				break;
			} /* chiude lo switch */
			return;
		} /* chiude l'if */

		if ( skill->spell_fun == spell_null )
		{
			send_to_char( ch, "Non si tratta di un incantesimo!\r\n" );
			return;
		}

		if ( !skill->spell_fun )
		{
			send_to_char( ch, "Non sono in grado di lanciare quest'incantesimo.. ancora.\r\n" );
			return;
		}

		/* Added checks for spell sector type */
		if ( !ch->in_room
		  || (!IS_EMPTY(skill->spell_sector) && !HAS_BIT(skill->spell_sector, ch->in_room->sector)) )
		{
			send_to_char( ch, "Non posso lanciare qui questo incantesimo.\r\n" );
			return;
		}

		if ( IS_MOB(ch) )
			mana = 0;
		else
			mana = UMAX( skill->min_mana, 100 / UMAX(1, (2 + get_level(ch) - skill->skill_level[ch->class])) );

		/*
		 * Locate targets.
		 */
		vo = locate_targets( ch, arg2, sn, &victim, &obj );

		if ( vo == &pAbort )
			return;

		if ( victim && !can_pkill(ch, victim) && skill->intention == INTENT_AGGRESSIVE )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Una strana aura protettiva circonda la mia vittima impedendomi di lanciare l'incantesimo.\r\n" );
			return;
		}

		if ( ch->points[POINTS_MANA] < mana && !stop_lowering_points(ch, POINTS_MANA) )
		{
			send_to_char( ch, "Non ho abbastanza mana per quest'incantesimo.\r\n" );
			return;
		}

		if ( skill->participants <= 1 )
			break;

		/* multi-participant spells */
		add_timer( ch, TIMER_DO_FUN, PULSE_VIOLENCE * UMIN(skill->beats / 10, 3), do_cast, 1 );
		act( AT_MAGIC, "Modulo la voce assecondando il flusso magico..", ch, NULL, NULL, TO_CHAR );
		act( AT_MAGIC, "$n modula la voce in strane tonalità..", ch, NULL, NULL, TO_ROOM );
		sprintf( staticbuf, "%s %s", arg2, target_name );
		DISPOSE( ch->alloc_ptr );
		ch->alloc_ptr = str_dup( staticbuf );
		ch->tempnum = sn;
		return;

	  case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->alloc_ptr );

		if ( is_valid_sn( (sn = ch->tempnum) ) )
		{
			if ( (skill = get_skilltype(sn)) == NULL )
			{
				send_to_char( ch, "Qualcosa non è andato come doveva! Devo usare un ritmo diverso forse..\r\n" );
				send_log( NULL, LOG_BUG, "do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn );
				return;
			}

			if ( IS_MOB(ch) )
				mana = 0;
			else
				mana = UMAX( skill->min_mana, 100 / UMAX(1, (2 + get_level(ch) - skill->skill_level[ch->class])) );

			ch->points[POINTS_MANA] -= mana / 3;
		}
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "La mia voce si ferma..\r\n" );
		/* (RR) should add chance of backfire here */
		return;

	  case 1:
		sn = ch->tempnum;
		if ( (skill = get_skilltype(sn)) == NULL )
		{
			send_to_char( ch, "Qualcosa è andato storto!\r\n" );
			send_log( NULL, LOG_BUG, "do_cast: substate 1: bad sn %d", sn );
			return;
		}

		if ( !ch->alloc_ptr || !is_valid_sn(sn) || skill->type != SKILLTYPE_SPELL )
		{
			send_to_char( ch, "L'incantesimo viene interrotto brutalmente da una perturbazione nel flusso!\r\n" );
			send_log( NULL, LOG_BUG, "do_cast: ch->alloc_ptr NULL or bad sn (%d)", sn );
			return;
		}

		if ( IS_MOB(ch) )
			mana = 0;
		else
			mana = UMAX( skill->min_mana, 100 / UMAX(1, (2 + get_level(ch) - skill->skill_level[ch->class])) );

		strcpy( staticbuf, ch->alloc_ptr );
		target_name = one_argument( staticbuf, arg2 );
		DISPOSE( ch->alloc_ptr );
		ch->substate = SUB_NONE;

		if ( skill->participants > 1 )
		{
			CHAR_DATA *tmp;
			TIMER	  *t;
			int		   cnt = 1;

			for ( tmp = ch->in_room->first_person;  tmp;  tmp = tmp->next_in_room )
			{
				if ( tmp == ch )										continue;
				if ( (t = get_timerptr(tmp, TIMER_DO_FUN)) == NULL )	continue;
				if ( t->pulse < 1 )										continue;
				if ( t->do_fun != do_cast )								continue;
				if ( tmp->tempnum != sn )								continue;
				if ( !tmp->alloc_ptr )									continue;
				if ( can_pkill(ch, NULL) && !can_pkill(tmp, NULL)
				  && skill->intention == INTENT_AGGRESSIVE )			continue;	/* Se ch può pkillare e il partecipante no e lo skell è aggressivo allora salta */

				if ( !str_cmp(tmp->alloc_ptr, staticbuf) )
					++cnt;
			}

			if ( cnt >= skill->participants )
			{
				for ( tmp = ch->in_room->first_person;  tmp;  tmp = tmp->next_in_room )
				{
					if ( tmp == ch )										continue;
					if ( (t = get_timerptr(tmp, TIMER_DO_FUN)) == NULL )	continue;
					if ( t->pulse < 1 )										continue;
					if ( t->do_fun != do_cast )								continue;
					if ( tmp->tempnum != sn )								continue;
					if ( !tmp->alloc_ptr )									continue;
					if ( can_pkill(ch, NULL) && !can_pkill(tmp, NULL)
					  && skill->intention == INTENT_AGGRESSIVE )			continue;	/* Se ch può pkillare e il partecipante no e lo skell è aggressivo allora salta */


					if ( !str_cmp(tmp->alloc_ptr, staticbuf) )
					{
						extract_timer( tmp, t );
						act( AT_MAGIC, "Canalizzo l'energia verso $n sincronizzando il flusso di mana!",
							ch, NULL, tmp, TO_VICT );
						act( AT_MAGIC, "$N sincronizza il flusso di mana con il tuo nel lancio dell'incantesimo!", ch, NULL, tmp, TO_CHAR );
						act( AT_MAGIC, "$N sincronizza il flusso di mana con $n!", ch, NULL, tmp, TO_NOVICT );
						learn_skell( tmp, sn, TRUE );

						tmp->points[POINTS_MANA] -= mana;

						tmp->substate = SUB_NONE;
						tmp->tempnum = -1;
						DISPOSE( tmp->alloc_ptr );
					}
				}
				dont_wait = TRUE;
				send_to_char( ch, "Concentro le mie energie richiamando la formula dell'incantesimo!\r\n" );
				vo = locate_targets( ch, arg2, sn, &victim, &obj );
				if ( vo == &pAbort )
					return;
			}
			else
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( ch, "L'energia che ho richiamato è insufficiente per la riuscita dell'incantesimo..\r\n" );

				ch->points[POINTS_MANA] -= mana / 2;

				learn_skell( ch, sn, FALSE );
				return;
			}
		} /* chiude l'if */
	} /* chiude lo switch */

	/* uttering those magic words unless casting "ventriloquate" */
	if ( str_cmp(skill->name, "ventriloquate") )
		say_spell( ch, sn );

	if ( !dont_wait )
		WAIT_STATE( ch, skill->beats * 3 );	/* (WT) */

	/*
	 * Getting ready to cast... check for spell components
	 */
	if ( !process_components(ch, sn) )
	{
		ch->points[POINTS_MANA] -= mana / 2;
		learn_skell( ch, sn, FALSE );
		return;
	}

	/* (FF) In futuro qui se si alza la difficoltà degli incantesimi bisogna diminuire un po' il moltiplicatore * 5 */
	if ( IS_PG(ch) && (number_percent() + skill->difficulty * 4) > ch->pg->learned_skell[sn] )
	{
		/* Some more interesting loss of concentration messages */
		switch ( number_bits(2) )
		{
		  case 0:	/* too busy */
			if ( ch->fighting )
				send_to_char( ch, "La battaglia è troppo concitata perché io possa lanciare un incantesimo!\r\n" );
			else
				send_to_char( ch, "Non riesco a trovare concentrazione sufficiente!\r\n" );
			break;

		  case 1:	/* irritation */
			if ( number_bits(2) == 0 )
			{
				switch ( number_bits(2) )
				{
				  case 0: send_to_char( ch, "Un tremito nel flusso magico m'impedisce di mantenere la concentrazione.\r\n" );			break;
				  case 1: send_to_char( ch, "Una stretta alla gola m'impedisce di pronunciare correttamente la formula.\r\n" );			break;
				  case 2: send_to_char( ch, "Un bagliore improvviso mi distrae.. l'incantesimo è sprecato.\r\n" );						break;
				  case 3: send_to_char( ch, "Un improvviso peso grava per un istante nella mia testa.. perdo la concentrazione.\r\n" );	break;
				}
			}
			else
			{
				send_to_char( ch, "Un rumore distante mi distrae.. perdo la concentrazione.\r\n" );
			}
			break;

		  case 2:		/* not enough time */
			if ( ch->fighting )
				send_to_char( ch, "Non riesco a completare in tempo la formula magica!\r\n" );
			else
				send_to_char( ch, "Un lontano clamore mi fa perdere la concentrazione.\r\n" );
			break;

		  case 3:
			send_to_char( ch, "Un blocco mentale mi impedisce di ricordare la formula.. incantesimo sprecato!\r\n" );
			break;
		}

		ch->points[POINTS_MANA] -= mana / 2;

		learn_skell( ch, sn, FALSE );
		return;
	}
	else
	{
		ch->points[POINTS_MANA] -= mana;

		/*
		 * check for immunity to magic if victim is known...
		 * and it is a TARGET_CHAR_DEFENSIVE/SELF spell
		 * otherwise spells will have to check themselves
		 */
		if ( ((skill->target == TARGET_CHAR_DEFENSIVE
		  ||   skill->target == TARGET_CHAR_SELF)
		  && victim && HAS_BIT(victim->immune, RIS_MAGIC)) )
		{
			immune_casting( skill, ch, victim, NULL );
			retcode = rSPELL_FAILED;
		}
		else
		{
			start_timer(&time_used);
			retcode = (*skill->spell_fun) ( sn, get_level(ch)/2, ch, vo );
			end_timer( &time_used );
			update_userec( &time_used, &skill->use_count );
		}
	}

	if ( ch->in_room && HAS_BIT( ch->in_room->area->flags, AREAFLAG_SPELL_LIMIT ) )
		ch->in_room->area->curr_spell_count++;

	if ( retcode == rCHAR_DIED || retcode == rERROR || char_died(ch) )
		return;

	/* learning */
	if ( retcode != rSPELL_FAILED )
		learn_skell( ch, sn, TRUE );
	else
		learn_skell( ch, sn, FALSE );

	/*
	 * Fixed up a weird mess here, and added double safeguards
	 */
	if ( skill->target == TARGET_CHAR_OFFENSIVE
	  && victim
	  && !char_died(victim)
	  && victim != ch )
	{
		CHAR_DATA *vch, *vch_next;

		for ( vch = ch->in_room->first_person;  vch;  vch = vch_next )
		{
			vch_next = vch->next_in_room;

			if ( vch == victim )
			{
				if ( vch->master != ch && !vch->fighting )
					retcode = multi_hit( vch, ch, TYPE_UNDEFINED );

				break;
			}
		}
	}
}


/*
 * Cast spells at targets using a magical object.
 */
ch_ret obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	void	   *vo;
	ch_ret		retcode = rNONE;
	int			levdiff = get_level(ch)/2 - level;
	SKILL_DATA  *skill = get_skilltype( sn );
	struct timeval time_used;

	if ( sn == -1 )
		return retcode;

	if ( !skill || !skill->spell_fun )
	{
		send_log( NULL, LOG_BUG, "obj_cast_spell: bad sn %d.", sn );
		return rERROR;
	}

	if ( HAS_BIT( ch->in_room->flags, ROOM_NOMAGIC ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "Nulla sembra accadere..\r\n" );
		return rNONE;
	}

	if ( HAS_BIT( ch->in_room->flags, ROOM_SAFE ) && skill->target == TARGET_CHAR_OFFENSIVE )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "Nulla sembra accadere..\r\n" );
		return rNONE;
	}

	/*
	 * Basically this was added to cut down on level 5 players using level
	 * 40 scrolls in battle too often ;)
	 */
	if ( (skill->target == TARGET_CHAR_OFFENSIVE
	  || number_bits(7) == 1)	/* 1/128 chance if non-offensive */
	  && skill->type != SKILLTYPE_HERB
	  && !chance(ch, 95 + levdiff) )
	{
		switch ( number_bits(2) )
		{
		  case 0:
			failed_casting( skill, ch, victim, NULL );
			break;

		  case 1:
			act( AT_MAGIC, "Il flusso magico ha un rigurgito e l'incantesimo di $t genera una violenta implosione!", ch, skill->name, victim, TO_CHAR );
			if ( victim )
				act( AT_MAGIC, "$n viene ferit$x dal rigurgito del suo stesso incantesimo di $t!", ch, skill->name, victim, TO_VICT );

			act( AT_MAGIC, "$n viene ferit$x dal rigurgito del suo stesso incantesimo di $t!", ch, skill->name, victim, TO_NOVICT );
			return damage( ch, ch, number_range(1, level), TYPE_UNDEFINED );

		  case 2:
			failed_casting( skill, ch, victim, NULL );
			break;

		  case 3:
			act( AT_MAGIC, "Il flusso magico subisce un'alterazione! L'incantesimo di $t si tramuta in una fiammata!", ch, skill->name, victim, TO_CHAR );
			if ( victim )
				act( AT_MAGIC, "$n viene bruciat$x dalla fiama generata dal suo stesso incantesimo di $t!", ch, skill->name, victim, TO_VICT );

			act( AT_MAGIC, "$n viene bruciat$x dalla fiama generada dal suo stesso incantesimo di $t!", ch, skill->name, victim, TO_NOVICT );
			return damage( ch, ch, number_range(1, level), TYPE_UNDEFINED );
		}
		return rNONE;
	}

	target_name = "";
	switch ( skill->target )
	{
	  default:
		send_log( NULL, LOG_BUG, "obj_cast_spell: bad target for sn %d.", sn );
		return rERROR;

	  case TARGET_IGNORE:
		vo = NULL;
		if ( victim )
			target_name = victim->name;
		else if ( obj )
			target_name = obj->name;
		break;

	  case TARGET_CHAR_OFFENSIVE:
		if ( victim != ch )
		{
			if ( !victim )
				victim = who_fighting( ch );

			if ( !victim || (IS_PG(victim) && !can_pkill(ch, victim)) )
			{
				send_to_char( ch, "Non posso farlo.\r\n" );
				return rNONE;
			}
		}

		if ( ch != victim && is_safe( ch, victim, TRUE ) )
			return rNONE;

		vo = (void *) victim;
		break;

	  case TARGET_CHAR_DEFENSIVE:
		if ( victim == NULL )
			victim = ch;

		vo = (void *) victim;

		if ( skill->type != SKILLTYPE_HERB
		  && HAS_BIT(victim->immune, RIS_MAGIC) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rNONE;
		}
		break;

	  case TARGET_CHAR_SELF:
		vo = (void *) ch;

		if ( skill->type != SKILLTYPE_HERB
		  && HAS_BIT(ch->immune, RIS_MAGIC) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rNONE;
		}
		break;

	  case TARGET_OBJ_INV:
		if ( obj == NULL )
		{
			send_to_char( ch, "Non posso farlo.\r\n" );
			return rNONE;
		}
		vo = (void *) obj;
		break;
	}

	start_timer(&time_used);
	retcode = (*skill->spell_fun) ( sn, level, ch, vo );
	end_timer(&time_used);
	update_userec( &time_used, &skill->use_count );

	if ( retcode == rSPELL_FAILED )
		retcode = rNONE;

	if ( retcode == rCHAR_DIED || retcode == rERROR )
		return retcode;

	if ( char_died(ch) )
		return rCHAR_DIED;

	if ( skill->target == TARGET_CHAR_OFFENSIVE
	  && victim != ch
	  && !char_died(victim) )
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for ( vch = ch->in_room->first_person; vch; vch = vch_next )
		{
			vch_next = vch->next_in_room;

			if ( victim == vch && !vch->fighting && vch->master != ch )
			{
				retcode = multi_hit( vch, ch, TYPE_UNDEFINED );
				break;
			}
		}
	}
	return retcode;
}



/**-----------------------------------------------------*
 ** 	>>>		Funzioni degli Incantesimi.		<<<		*
 **-----------------------------------------------------*/

/*
 * Incantesimo di Acid Blast.
 */
SPELL_RET spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice( level, 6 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Blindness.
 */
SPELL_RET spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	AFFECT_DATA *aff;
	SKILL_DATA	*skill = get_skilltype( sn );
	int			tmp;

	if ( SPELL_FLAG(skill, SKELL_PKSENSITIVE) && IS_PG(ch) && IS_PG(victim) )
		tmp = level/2;
	else
		tmp = level;

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_BLIND) || compute_savthrow(tmp, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->location	= APPLY_HITROLL;
	aff->modifier	= -4;
	aff->duration	= ( level/3 + 1 ) * 23;
	aff->bitvector	= meb( AFFECT_BLIND );
	affect_to_char( victim, aff );

	set_char_color( AT_MAGIC, victim );
	send_to_char( victim, "Un bruciore improvviso m'investe il volto! Non vedo più nulla!\r\n" );

	if ( ch != victim )
	{
		act( AT_MAGIC, "Richiamo una fiamma oscura sul volto di $N, accecandol$X!", ch, NULL, victim, TO_CHAR );
		act( AT_MAGIC, "$n richiama un'ombra oscura sul volto di $N che sembra non distinguere più nulla!", ch, NULL, victim, TO_NOVICT );
	}

	return rNONE;
}


/*
 * Incantesimo di Burning Hands.
 */
SPELL_RET spell_burning_hands( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		 1,
		 3,  5,  7, 10, 14, 17, 20, 23, 26, 29,
		29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
		34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
		39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
		44, 44, 45, 45, 46, 46, 47, 47, 48, 48,
		49, 49, 50, 50, 51, 51, 52, 52, 53, 53,
		54, 54, 55, 55, 56, 56, 57, 57, 58, 58
	};
	int dam;

	level = UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level = UMAX( 0, level );
	dam	= number_range( dam_each[level] / 2, dam_each[level] * 2 );
	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Call Lightning.
 */
SPELL_RET spell_call_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int		   dam;
	bool	   ch_died;
	ch_ret	   retcode = rNONE;

	if ( !is_outside(ch) )
	{
		send_to_char( ch, "Devo trovarmi all'esterno per usufruire di questo incantesimo.\r\n" );
		return rSPELL_FAILED;
	}

	if ( ch->in_room->area->weather->precip <= 0 )
	{
		send_to_char( ch, "Richiamare fulmini a ciel sereno è impossibile..\r\n" );
		return rSPELL_FAILED;
	}

	dam = dice( level/2, 8 );

	set_char_color( AT_MAGIC, ch );
	send_to_char( ch, "Azzurri lampi richiamati dal cielo balenano contro i miei nemici!\r\n" );
	act( AT_MAGIC, "$n richiama dal cielo tuoni e lampi contro i suoi nemici!", ch, NULL, NULL, TO_ROOM );

	ch_died = FALSE;
	for ( vch = first_char;  vch;  vch = vch_next )
	{
		vch_next = vch->next;

		if ( !vch->in_room )
			continue;

		if ( vch->in_room == ch->in_room )
		{
			if ( IS_ADMIN(vch) )
				continue;

			if ( vch != ch && (IS_MOB(ch)) ? IS_PG(vch) : IS_MOB(vch) )
				retcode = damage( ch, vch, compute_savthrow(level, vch, SAVETHROW_SPELL_STAFF)  ?  dam/2  :  dam, sn );

			if ( retcode == rCHAR_DIED || char_died(ch) )
				ch_died = TRUE;

			continue;
		}

		if ( !ch_died
		  && vch->in_room->area == ch->in_room->area
		  && is_outside(vch)
		  && is_awake(vch) )
		{
			if ( number_bits(3) == 0 )
				send_to_char( vch, "&BIn lontananza chiari ed azzurri lampi riempiono il cielo.\r\n" );
		}
	}

	if ( ch_died )
		return rCHAR_DIED;
	else
		return rNONE;
}


/*
 * Incantesimi di Cause Light/Critical/Serious.
 */
SPELL_RET spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
	return damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn );
}

SPELL_RET spell_cause_critical( int sn, int level, CHAR_DATA *ch, void *vo )
{
	return damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn );
}

SPELL_RET spell_cause_serious( int sn, int level, CHAR_DATA *ch, void *vo )
{
	return damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn );
}


/*
 * Incantesimo di Change Sex.
 */
SPELL_RET spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	SKILL_DATA  *skill = get_skilltype(sn);
	AFFECT_DATA *aff;

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( is_affected(victim, sn) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= 10 * level * (20*3);
	aff->location	= APPLY_SEX;

	do
	{
		aff->modifier = number_range( 0, MAX_SEX-1 ) - victim->sex;
	} while ( aff->modifier == 0 );

	affect_to_char( victim, aff );

	successful_casting( skill, ch, victim, NULL );
	return rNONE;
}


/*
 * Funzione che controlla chi è immune allo charm
 *	oppure se riesce a farsi seguire da un mascotte o da altri,
 *	per gestire il gruppo di mob al seguito
 */
bool can_charm( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "can_charm: ch è NULL" );
		return FALSE;
	}

	if ( IS_MOB(ch) )
		return TRUE;

	if ( IS_ADMIN(ch) )
		return TRUE;

	if ( ch->pg->charmies < get_curr_attr(ch, ATTR_LEA)/25 + 1 )
		return TRUE;

	return FALSE;
}


/*
 * Incantesimo di Charm Person.
 */
SPELL_RET spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA   *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *aff;
	SKILL_DATA  *skill = get_skilltype( sn );
	int			 charm_chance;

	if ( victim == ch )
	{
		send_to_char( ch, "Si, mi ritengo veramente molto affascinante!\r\n" );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->immune, RIS_MAGIC)
	  || HAS_BIT(victim->immune, RIS_CHARM) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( IS_PG(victim) && IS_PG(ch) )
	{
		send_to_char( ch, "Meglio non farlo..\r\n" );
		return rSPELL_FAILED;
	}

	charm_chance = ris_save( victim, level, RIS_CHARM );

	if ( HAS_BIT(victim->affected_by, AFFECT_CHARM)
	  || charm_chance == 1000
	  || HAS_BIT(ch->affected_by, AFFECT_CHARM)
	  || level*2 < get_level(victim)
	  || circle_follow(victim, ch)
	  || !can_charm(ch)
	  || compute_savthrow(charm_chance, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( victim->master )
		stop_follower( victim );

	add_follower( victim, ch );

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= ( number_fuzzy((level + 1) / 5) + 1 ) * 69;
	aff->location	= 0;
	aff->modifier	= 0;
	aff->bitvector	= meb( AFFECT_CHARM );
	affect_to_char( victim, aff );

	successful_casting( skill, ch, victim, NULL );

	send_log( NULL, LOG_WARNING, "spell_charm_person: %s ha charmato %s.", ch->name, victim->name );

	/* C'è una probabilità casuale che la vittima poi non si ribelli al maestro */
	if ( number_range(0, 1) && IS_MOB(victim) )
	{
		start_hating( victim, ch );
		start_hunting( victim, ch );
	}

	return rNONE;
}


/*
 * Incantesimo di Chill Touch.
 */
SPELL_RET spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		 1,
		 2,  4,  6,  7,  8,  9, 12, 13, 13, 13,
		14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
		17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
		20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
		24, 24, 24, 25, 25, 25, 26, 26, 26, 27,
		27, 28, 28, 29, 29, 30, 30, 31, 31, 32,
		32, 33, 34, 34, 35, 35, 36, 37, 37, 38
	};
	AFFECT_DATA *aff;
	int dam;

	level = UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level = UMAX( 0, level );

	dam	= number_range( dam_each[level] / 2, dam_each[level] * 2 );

	if ( !compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		CREATE( aff, AFFECT_DATA, 1 );
		aff->type		= sn;
		aff->duration	= 14 * 3;
		aff->location	= APPLY_STR;
		aff->modifier	= -1;
		affect_join( victim, aff );
	}
	else
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Colour Spray.
 */
SPELL_RET spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		1,
		2,   3,  6,  9, 12, 13, 16, 19, 22, 25,
		30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
		58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
		65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
		73, 73, 74, 75, 76, 76, 77, 78, 79, 79,
		80, 80, 81, 82, 82, 83, 83, 84, 85, 85,
		86, 86, 87, 88, 88, 89, 89, 90, 91, 91
	};
	int dam;

	level = UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level = UMAX( 0, level );
	dam = number_range( dam_each[level] / 2,  dam_each[level] * 2 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Create Food.
 */
SPELL_RET spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *mushroom;

	mushroom = make_object( get_obj_proto(NULL, VNUM_OBJ_MUSHROOM), 0 );
	mushroom->food->hours = level + 5;
	act( AT_MAGIC, "$p appare dal nulla.", ch, mushroom, NULL, TO_ROOM );
	act( AT_MAGIC, "$p appare dal nulla.", ch, mushroom, NULL, TO_CHAR );
	mushroom = obj_to_room( mushroom, ch->in_room );

	return rNONE;
}


/*
 * Incantesimo di Create Water.
 */
SPELL_RET spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA	 *obj = (OBJ_DATA *) vo;
	WEATHER_DATA *weath;
	int			  water;

	if ( obj->type != OBJTYPE_DRINK_CON )
	{
		send_to_char( ch, "Non mi sembra un contenitore adatto all'acqua.\r\n" );
		return rSPELL_FAILED;
	}

	if ( obj->drinkcon->liquid != LIQ_WATER && obj->drinkcon->curr_quant != 0 )
	{
		send_to_char( ch, "C'è dell'altro liquido all'interno, devo svuotarlo prima.\r\n" );
		return rSPELL_FAILED;
	}

	weath = ch->in_room->area->weather;

	water = UMIN( level * (weath->precip >= 0) ? 40 : 20, obj->drinkcon->capacity - obj->drinkcon->curr_quant );

	if ( water > 0 )
	{
		char	qnt[MIL];

		split_obj( obj, 1 );
		obj->drinkcon->liquid = LIQ_WATER;
		obj->drinkcon->curr_quant += water;

		if ( !is_name("water", obj->name) )
		{
			char buf[MSL];

			sprintf( buf, "%s water", obj->name );
			DISPOSE( obj->name );
			obj->name = str_dup( buf );
		}

		if		( water < 8 )	strcpy( qnt, "poca " );
		else if ( water < 15 )	strcpy( qnt, "dell'" );
		else					strcpy( qnt, "molta " );

		if ( obj->drinkcon->curr_quant <= 0 )
			act( AT_MAGIC, "Creo $Tacqua dentro un $p.", ch, obj, qnt, TO_CHAR );
		else
			act( AT_MAGIC, "Aggiungo $Tacqua dentro un $p.", ch, obj, qnt, TO_CHAR );
	}

	return rNONE;
}


/*
 * Incantesimo di Blindness.
 */
SPELL_RET spell_cure_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype(sn);

	set_char_color( AT_MAGIC, ch );

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( !is_affected(victim, gsn_blindness) )
	{
		if ( ch != victim )
			send_to_char( ch, "Cerco di distogliere il velo oscuro.. ma è tutto inutile.\r\n" );
		else
			send_to_char( ch, "Attualmente non mi manca la vista.\r\n" );

		return rSPELL_FAILED;
	}

	affect_strip( victim, gsn_blindness );
	set_char_color( AT_MAGIC, victim);
	send_to_char( victim, "L'oscurità bruciante lascia finalmente i miei occhi, torno a vederci!\r\n" );

	if ( ch != victim )
		send_to_char( ch, "Riesco a distogliere il velo oscuro dagli occhi della vittima!\r\n" );

	return rNONE;
}


/*
 * Incantesimo di Sacral Divinity, del Paladino.
 */
SPELL_RET spell_sacral_divinity( int sn, int level, CHAR_DATA *ch, void *vo )
{
	AFFECT_DATA	*aff;
	SKILL_DATA	*skill = get_skilltype( sn );

	if ( ch->alignment < 350 )
	{
		act( AT_MAGIC, "Nessuno sembra voler ascoltare la mia preghiera.", ch, NULL, NULL, TO_CHAR );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(ch->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_SANCTUARY) )
		return rSPELL_FAILED;

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= level * 3 * 23;
	aff->location	= APPLY_AFFECT;
	aff->modifier	= 0;
	aff->bitvector	= meb( AFFECT_SANCTUARY );
	affect_to_char( ch, aff );

	act( AT_MAGIC, "Un mantello di sfolgorante luce eterea avvolge completamente $n.", ch, NULL, NULL, TO_ROOM);
	act( AT_MAGIC, "Un mantello di sfolgorante luce eterea mi avviluppa completamente.", ch, NULL, NULL, TO_CHAR );
	return rNONE;
}


/*
 * Incantesimo di Expurgation, del Paladino.
 */
SPELL_RET spell_expurgation( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( !is_affected(victim, gsn_poison) )
		return rSPELL_FAILED;

	affect_strip( victim, gsn_poison );
	act( AT_MAGIC, "Recito a bassa voce un'antica preghiera di purificazione.", ch, NULL, NULL, TO_CHAR );
	act( AT_MAGIC, "$n recita a bassa voce un'antica litania dai toni suadenti.", ch, NULL, NULL, TO_ROOM );
	return rNONE;
}


/*
 * Incantesimo di Bethsaidean Touch, del Paladino.
 */
SPELL_RET spell_bethsaidean_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( !is_affected(victim, gsn_blindness) )
		return rSPELL_FAILED;

	affect_strip( victim, gsn_blindness );
	set_char_color( AT_MAGIC, victim );
	send_to_char( victim, "La mia vista viene ristorata!\r\n" );

	if ( ch != victim )
	{
		act( AT_MAGIC, "$n sfiora con una mano le mie tempie e socchiude gli occhi mentre prega silenziosamente.", ch, NULL, victim, TO_VICT );
		act( AT_MAGIC, "$n sfiora con una mano le tempie di $N e socchiude gli occhi mentre prega silenziosamente.", ch, NULL, victim, TO_NOVICT );
		act( AT_MAGIC, "Sfioro con una mano le tempie di $N raccogliendomi in una silenziosa preghiera.", ch, NULL, victim, TO_CHAR );
	}
	return rNONE;
}


/*
 * Incantesimo di Cure Poison.
 */
SPELL_RET spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( is_affected(victim, gsn_poison) )
	{
		affect_strip( victim, gsn_poison );
		set_char_color( AT_MAGIC, victim);
		send_to_char( victim, "Un caldo tepore frena la fastidiosa morsa del veleno.\r\n" );
		victim->mental_state = URANGE( -100, victim->mental_state, -10 );

		if ( ch != victim )
		{
			act( AT_MAGIC, "La sofferenza di $N si affievolisce, l'effetto del veleno scompare.", ch, NULL, victim, TO_NOVICT );
			act( AT_MAGIC, "Scaccio il veleno dal corpo di $N.", ch, NULL, victim, TO_CHAR );
		}

		return rNONE;
	}
	else
	{
		set_char_color( AT_MAGIC, ch );

		if ( ch != victim )
			send_to_char( ch, "Il veleno sembra vincere la mia cura..\r\n" );
		else
			send_to_char( ch, "Non mi sembra che alcun veleno scorra nelle mie vene.\r\n" );

		return rSPELL_FAILED;
	}
}


/*
 * Incantesimo di Curse.
 */
SPELL_RET spell_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	AFFECT_DATA	*aff1;
	AFFECT_DATA	*aff2;
	SKILL_DATA	*skill = get_skilltype(sn);

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_CURSE) || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	CREATE( aff1, AFFECT_DATA, 1 );
	aff1->type		= sn;
	aff1->duration	= (level * 4) * 69;
	aff1->location	= APPLY_HITROLL;
	aff1->modifier	= -1;
	aff1->bitvector	= meb( AFFECT_CURSE );
	affect_to_char( victim, aff1 );

	CREATE( aff2, AFFECT_DATA, 1 );
	aff2->type		= sn;
	aff2->duration	= (level * 4) * 69;
	aff2->location	= APPLY_SAVE_SPELL;
	aff2->modifier	= 1;
	aff2->bitvector	= meb( AFFECT_CURSE );
	affect_to_char( victim, aff2 );

	set_char_color( AT_MAGIC, victim );
	send_to_char( victim, "Un forte senso di disagio mi pervade.\r\n" );

	if ( ch != victim )
	{
		act( AT_MAGIC, "Invoco una maledizione contro $N.", ch, NULL, victim, TO_CHAR );
		act( AT_MAGIC, "$n invoca su $N una maledizione!", ch, NULL, victim, TO_NOVICT );
	}

	return rNONE;
}


/*
 * Incantesimo di Detect Poison.
 */
SPELL_RET spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	set_char_color( AT_MAGIC, ch);

	if ( obj->type == OBJTYPE_DRINK_CON
	  || obj->type == OBJTYPE_FOOD
	  || obj->type == OBJTYPE_COOK )
	{
		if ( obj->type == OBJTYPE_COOK && obj->food->dam_cook == 0 )
			send_to_char( ch, "Non sembra cotto a dovere!\r\n" );
		else if ( obj->food->poison != 0 )
			send_to_char( ch, "Avverto la presenza del veleno.\r\n" );
		else
			send_to_char( ch, "Non sembra esserci traccia di veleno.\r\n" );
	}
	else
	{
		send_to_char( ch, "Non avverto la presenza di alcun veleno.\r\n" );
	}

	return rNONE;
}


/*
 * Incantesimo di Dispel Evil.
 */
SPELL_RET spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype( sn );
	int dam;

	if ( IS_PG(ch) && is_align(ch, ALIGN_EVIL) )
		victim = ch;

	if ( is_align(victim, ALIGN_GOOD) )
	{
		act( AT_MAGIC, "L'incantesimo scivola via attraversando $N senza arrecare danno.", ch, NULL, victim, TO_CHAR );
		return rSPELL_FAILED;
	}

	if ( is_align(victim, ALIGN_NEUTRAL) )
	{
		act( AT_MAGIC, "$N non sembra subire minimamente questo incantesimo!", ch, NULL, victim, TO_CHAR );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	dam = dice( level, 4 );
	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Dispel Magic.
 * New version of dispel magic fixes alot of bugs, and allows players
 * to not lose thie affects if they have the spell and the affect.
 * Also prints a message to the victim, and does various other things :)
 */
SPELL_RET spell_dispel_magic (int sn, int level, CHAR_DATA * ch, void *vo)
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	SKILL_DATA	*skill = get_skilltype ( sn );
	AFFECT_DATA *paf;
	int			 cnt = 0;
	int			 affect_num;
	int			 affected_by = 0;
	int			 times = 0;
	int			 dispel_chance;
	bool		 is_mage = FALSE;
	bool		 found = FALSE;
	bool		 twice = FALSE;
	bool		 three = FALSE;

	set_char_color ( AT_MAGIC, ch );

	dispel_chance = get_curr_attr( ch, ATTR_WIL )/4 - get_curr_attr( victim, ATTR_ABS )/4;

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( IS_MOB(ch) || HAS_BIT(table_class[ch->class]->flags, CLASSFLAG_CASTER) || IS_ADMIN(ch) )
		is_mage = TRUE;

	if ( is_mage )
		dispel_chance += 5;
	else
		dispel_chance -= 15;

	/* Bug Fix to prevent possesed mobs from being dispelled */	/* era nei due if qui sotto (TT) */
	if ( IS_MOB(victim) && HAS_BIT(victim->affected_by, AFFECT_POSSESS) )
	{
		immune_casting ( skill, ch, victim, NULL );
		return rVICT_IMMUNE;
	}

	if ( ch == victim )
	{
		if ( ch->first_affect )
		{
			send_to_char( ch, "Non sembra succedere nulla di nuovo..\r\n" );

			while ( ch->first_affect )
				affect_remove ( ch, ch->first_affect );

			if ( IS_PG(ch) )		/* Stop the NPC bug */
				update_aris( victim );

			return rNONE;
		}
		else
		{
			send_to_char( ch, "Non sembra succedere nulla di nuovo..\r\n" );
			return rNONE;
		}
	}

	if ( !is_mage && !HAS_BIT(ch->affected_by, AFFECT_DETECT_MAGIC) )
	{
		send_to_char( ch, "Non avverto la presenza di alcuna aura magica da dissolvere.\r\n" );
		return rERROR;			/* You don't cast it so don't attack */
	}

	if ( number_percent() > (75 - dispel_chance) )
	{
		twice = TRUE;

		if ( number_percent () > (75 - dispel_chance) )
			three = TRUE;
	}


start_loop:		/* (RR) provare a rifare la funzione senza l'utilizzo delle etichette goto che odio */

	/* Grab affected_by from mobs first */
	if ( IS_MOB(victim) && !IS_EMPTY(victim->affected_by) )
	{
		for ( ; ; )
		{
			affected_by = number_range( 0, MAX_AFFECT-1 );

			if ( HAS_BIT(victim->affected_by, affected_by) )
			{
				found = TRUE;
				break;
			}

			if ( cnt++ > 30 )
			{
				found = FALSE;
				break;
			}
		}

		if ( found )		/* Ok lets see if it is a spell */
		{
			for ( paf = victim->first_affect; paf; paf = paf->next )
			{
				if ( HAS_BIT(paf->bitvector, affected_by) )
					break;
			}

			if ( paf )		/*It is a spell lets remove the spell too */
			{
				if ( level*2 < get_level(victim) || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
				{
					if ( !dispel_casting(paf, ch, victim, FALSE, FALSE) )
						failed_casting ( skill, ch, victim, NULL );

					return rSPELL_FAILED;
				}

				if ( SPELL_FLAG ( get_skilltype (paf->type), SKELL_NODISPEL))
				{
					if ( !dispel_casting ( paf, ch, victim, FALSE, FALSE ) )
						failed_casting ( skill, ch, victim, NULL );

					return rSPELL_FAILED;
				}

				if ( !dispel_casting ( paf, ch, victim, FALSE, TRUE ) && times == 0 )
				{
					successful_casting( skill, ch, victim, NULL );
				}

				affect_remove ( victim, paf );

				if ( (twice && times < 1) || (three && times < 2 ) )
				{
					times++;
					goto start_loop;
				}

				return rNONE;
			}
			else		/* Nope not a spell just remove the bit *For Mobs Only* */
			{
				if ( level*2 < get_level(victim) || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
				{
					if ( !dispel_casting (NULL, ch, victim, affected_by,FALSE) )
						failed_casting( skill, ch, victim, NULL );

					return rSPELL_FAILED;
				}

				if ( !dispel_casting(NULL, ch, victim, affected_by, TRUE) && times == 0 )
					successful_casting( skill, ch, victim, NULL );

				REMOVE_BIT( victim->affected_by, affected_by );

				if ( (twice && times < 1) || (three && times < 2) )
				{
					times++;
					goto start_loop;
				}
				return rNONE;
			}
		} /* chiude l'if */
	}

	/*
	 * Ok mob has no affected_by's or we didn't catch them lets go to first_affect.
	 */
	if ( !victim->first_affect )
	{
		failed_casting ( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}
	cnt = 0;

	/*
	 * Need to randomize the affects, yes you have to loop on average 1.5 times
	 * but dispel magic only takes at worst case 256 uSecs so who cares :)
	 */
	for ( paf = victim->first_affect;  paf;  paf = paf->next )
		cnt++;

	paf = victim->first_affect;

	for ( affect_num = number_range (0, (cnt - 1));affect_num > 0; affect_num-- )
		paf = paf->next;

	if ( level*2 < get_level(victim) || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		if ( !dispel_casting ( paf, ch, victim, FALSE, FALSE ) )
			failed_casting ( skill, ch, victim, NULL );

		return rSPELL_FAILED;
	}

	/* Need to make sure we have an affect and it isn't no dispel */
	if ( !paf || SPELL_FLAG(get_skilltype(paf->type), SKELL_NODISPEL) )
	{
		if ( !dispel_casting(paf, ch, victim, FALSE, FALSE) )
			failed_casting( skill, ch, victim, NULL );

		return rSPELL_FAILED;
	}

	if ( !dispel_casting(paf, ch, victim, FALSE, TRUE) && times == 0 )
		successful_casting ( skill, ch, victim, NULL );

	affect_remove ( victim, paf );

	if ( (twice && times < 1) || (three && times < 2 ) )
	{
		times++;
		goto start_loop;
	}

	/* Have to reset victim affects */
	if ( IS_PG(victim) )
		update_aris( victim );

	return rNONE;
}


/*
 * Incantesimo di Polymorph.
 */
SPELL_RET spell_polymorph( int sn, int level, CHAR_DATA *ch, void *vo )
{
	MORPH_DATA *morph;
	SKILL_DATA *skill = get_skilltype(sn);

	morph = find_morph( ch, target_name, TRUE );

	if ( !morph )
	{
		send_to_char( ch, "Non posso mutare il mio aspetto in nulla di simile!\r\n" );
		return rSPELL_FAILED;
	}

	if ( !do_morph_char(ch, morph) )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}
	return rNONE;
}


/*
 * Incantesimo di Earthquake.
 */
SPELL_RET spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	SKILL_DATA *skill = get_skilltype(sn);
	ch_ret	   retcode;
	bool	   ch_died;

	ch_died = FALSE;
	retcode = rNONE;

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}

	act( AT_MAGIC, "La terra sotto di me trema e si apre in profonde crepe!",  ch, NULL, NULL, TO_CHAR );
	act( AT_MAGIC, "$n richiama le forze della terra facendola aprire e tremare al suo comando!", ch, NULL, NULL, TO_ROOM );

	for ( vch = first_char;  vch;  vch = vch_next )
	{
		vch_next = vch->next;

		if ( !vch->in_room )
			continue;

		if ( vch->in_room == ch->in_room )
		{
			if ( IS_ADMIN(vch) )
				continue;

			if ( vch != ch && (IS_MOB(ch)) ? IS_PG(vch) : IS_MOB(vch)
			  && !HAS_BIT(vch->affected_by, AFFECT_FLYING)
			  && !HAS_BIT(vch->affected_by, AFFECT_FLOATING) )
			{
				retcode = damage( ch, vch, level + dice(2, 8), sn );
			}

			if ( retcode == rCHAR_DIED || char_died(ch) )
			{
				ch_died = TRUE;
				continue;
			}

			if ( char_died(vch) )
				continue;
		}

		if ( !ch_died && vch->in_room->area == ch->in_room->area )
		{
			if ( number_bits(3) == 0 )
				send_to_char( vch, "&BUn tremito scuote la terra.\r\n" );
		}
	}

	if ( ch_died )
		return rCHAR_DIED;

	return rNONE;
}


/*
 * Incantesimo di Enchant Weapon.
 */
SPELL_RET spell_enchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	if ( obj->type != OBJTYPE_WEAPON
		|| HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC)
		|| obj->first_affect )
	{
		act( AT_MAGIC, "Il flusso magico avvolge $p ma si frantuma in scintille abbandonando l'oggetto.", ch, obj, NULL, TO_CHAR );
		act( AT_MAGIC, "$n avvolge con un turbinio di scintille magiche $p.. ma senza ottenere nulla..", ch, obj, NULL, TO_NOVICT );
		return rSPELL_FAILED;
	}

	split_obj( obj, 1 );
	CREATE( paf, AFFECT_DATA, 1 );
	paf->type		= -1;
	paf->duration	= -1;
	paf->location	= APPLY_HITROLL;
	paf->modifier	= level / 15;
	LINK( paf, obj->first_affect, obj->last_affect, next, prev );

	CREATE( paf, AFFECT_DATA, 1 );
	paf->type		= -1;
	paf->duration	= -1;
	paf->location	= APPLY_DAMROLL;
	paf->modifier	= level / 15;
	LINK( paf, obj->first_affect, obj->last_affect, next, prev );

	SET_BIT( obj->extra_flags, OBJFLAG_MAGIC );

	if ( is_align(ch, ALIGN_GOOD) )
	{
		SET_BIT( obj->extra_flags, OBJFLAG_ANTI_EVIL );
		act( AT_BLUE, "$p riluce di opachi bagliori bluastri.", ch, obj, NULL, TO_ROOM );
		act( AT_BLUE, "$p riluce di opachi bagliori bluastri.", ch, obj, NULL, TO_CHAR );
	}
	else if ( is_align(ch, ALIGN_NEUTRAL) )
	{
		SET_BIT( obj->extra_flags, OBJFLAG_ANTI_EVIL );
		SET_BIT( obj->extra_flags, OBJFLAG_ANTI_GOOD );
		act( AT_DRED, "Stille di luce vermiglia scorrono attraverso $p.", ch, obj, NULL, TO_CHAR );
		act( AT_DRED, "Stille di luce vermiglia scorrono attraverso $p.", ch, obj, NULL, TO_ROOM );
	}
	else
	{
		SET_BIT( obj->extra_flags, OBJFLAG_ANTI_GOOD );
		act( AT_YELLOW, "$p risplende di un un sottile bagliore grigio nuvola..", ch, obj, NULL, TO_ROOM );
		act( AT_YELLOW, "$p brilla di luce inquietante.", ch, obj, NULL, TO_CHAR );
	}

	return rNONE;
}


/*
 * Incantesimo di Disenchant Weapon.
 */
SPELL_RET spell_disenchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;

	if ( obj->type != OBJTYPE_WEAPON )
	{
		send_to_char( ch, "Questo incantesimo è applicabile sole sulle armi.\r\n" );
		return rSPELL_FAILED;
	}

	if ( !HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) || !obj->first_affect )
	{
		send_to_char( ch, "Non sembra presente alcun tipo di incantamento su quest'arma.\r\n" );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(obj->pObjProto->extra_flags, OBJFLAG_MAGIC) )
	{
		send_to_char( ch, "L'incantamento su quest'arma è troppo radicato per poterlo rimuovere.\r\n" );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(obj->pObjProto->extra_flags, OBJFLAG_ANTI_GOOD)
	  || HAS_BIT(obj->pObjProto->extra_flags, OBJFLAG_ANTI_EVIL) )
	{
		send_to_char( ch, "Non posso disincantare un'arma la cui forgia è legata ad uno spirito d'allineamento.\r\n" );
		return rSPELL_FAILED;
	}

	split_obj( obj, 1 );
	for ( paf = obj->first_affect; paf; paf = paf->next)
	{
		if ( paf->location == APPLY_HITROLL || paf->location == APPLY_DAMROLL )
			UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
	}

	REMOVE_BIT( obj->extra_flags, OBJFLAG_MAGIC );

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_GOOD) )
	{
		REMOVE_BIT(obj->extra_flags, OBJFLAG_ANTI_GOOD);
		act( AT_BLUE, "$p assorbe tutte le radiazioni luminose blu nell'ambiente.", ch, obj, NULL, TO_CHAR );
	}

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_EVIL) )
	{
		REMOVE_BIT(obj->extra_flags, OBJFLAG_ANTI_EVIL);
		act( AT_RED, "$p assorbe tutte le radiazioni luminose rosse nell'ambiente.", ch, obj, NULL, TO_CHAR );
	}

	successful_casting( get_skilltype(sn), ch, NULL, obj );
	return rNONE;
}


/*
 * Incantesimo di Energy Drain, prosciuga HP, Mana e XP.
 * Il pronunciatore guadagna HP.
 */
SPELL_RET spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA  *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype(sn);
	int		    dam;
	int		    drain_chance;

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	drain_chance = ris_save( victim, get_level(victim)/2, RIS_DRAIN );
	if ( drain_chance == 1000 || compute_savthrow(drain_chance, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	ch->alignment = UMAX( -1000, ch->alignment - 200 );

	if ( get_level(victim) <= 2 )
		dam = ch->points[POINTS_LIFE] + 1;
	else
	{
		gain_exp( victim, 0 - number_range(level / 2, level * 3/2), FALSE );
		victim->points[POINTS_MANA] /= 2;
		victim->points[POINTS_MOVE] /= 2;
		dam	= dice( 1, level );
		ch->points[POINTS_LIFE] += dam;
	}

	if ( ch->points[POINTS_LIFE] > ch->points_max[POINTS_LIFE] )
		ch->points[POINTS_LIFE] = ch->points_max[POINTS_LIFE];

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Fireball.
 */
SPELL_RET spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		  1,
		  2,   3,   4,   5,   7,   9,  11,  13,  15,  17,
		 19,  21,  24,  27,  30,  35,  40,  45,  50,  55,
		 60,  65,  70,  75,  80,  82,  84,  86,  88,  90,
		 92,  94,  96,  98, 100, 102, 104, 106, 108, 110,
		112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
		132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
		152, 154, 156, 158, 160, 162, 164, 166, 168, 170
	};
	int dam;

	level = UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level = UMAX( 0, level );
	dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Flamestrike.
 */
SPELL_RET spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(6, 8);

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Faerie Fire.
 */
SPELL_RET spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	AFFECT_DATA	*aff;
	SKILL_DATA	*skill = get_skilltype(sn);

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_FAERIE_FIRE) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= level * 69;
	aff->location	= APPLY_AC;
	aff->modifier	= level * 2;
	aff->bitvector	= meb( AFFECT_FAERIE_FIRE );
	affect_to_char( victim, aff );

	act( AT_PINK, "Il mio corpo è ricoperto da una luminescente patina rosa.", victim, NULL, NULL, TO_CHAR );
	act( AT_PINK, "$n viene circondat$x da una luminscente patina rosa.", victim, NULL, NULL, TO_ROOM );

	return rNONE;
}


/*
 * Incantesimo di Faerie Fog.
 */
SPELL_RET spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *ich;

	act( AT_MAGIC, "$n evoca una fitta nube di polvere rosa fluorescente!", ch, NULL, NULL, TO_ROOM );
	act( AT_MAGIC, "Evoco dal nulla una fitta nube di polvere rosa fluorescente!", ch, NULL, NULL, TO_CHAR );

	for ( ich = ch->in_room->first_person; ich; ich = ich->next_in_room )
	{
		if ( ich == ch )											continue;
		if ( ich->pg && ich->pg->incognito >= ch->level )			continue;
		if ( compute_savthrow(level, ich, SAVETHROW_SPELL_STAFF) )	continue;

		affect_strip( ich, gsn_invis				  );
		affect_strip( ich, gsn_mass_invis			  );
		affect_strip( ich, gsn_sneak				  );
		REMOVE_BIT	( ich->affected_by, AFFECT_HIDE	  );
		REMOVE_BIT	( ich->affected_by, AFFECT_INVISIBLE );
		REMOVE_BIT	( ich->affected_by, AFFECT_SNEAK	  );

		act( AT_MAGIC, "$n viene rivelat$x da sbuffi di polvere!", ich, NULL, NULL, TO_ROOM );
		act( AT_MAGIC, "Gli sbuffi di polvere rosa mi rivelano!", ich, NULL, NULL, TO_CHAR );
	}

	return rNONE;
}


/*
 * Incantesimo di Gate.
 */
SPELL_RET spell_gate( int sn, int level, CHAR_DATA *ch, void *vo )
{
	MOB_PROTO_DATA *temp;

	temp = get_mob_index( NULL, VNUM_MOB_VAMPIRE );
	if ( !temp )
	{
		send_log( NULL, LOG_BUG, "spell_gate: il vnum del Vampiro %d non esiste.", VNUM_MOB_VAMPIRE );
		return rSPELL_FAILED;
	}

	char_to_room( make_mobile( temp ), ch->in_room );

	return rNONE;
}


/*
 * Incantesimo di Harm.
 */
SPELL_RET spell_harm( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	dam = UMAX( 20, victim->points[POINTS_LIFE] - dice(1, 4) );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam = UMIN( 50, dam / 4 );

	dam = UMIN( 100, dam );
	return damage( ch, victim, dam, sn );
}


/*
 * Funzione che visualizza l'identify di un oggetto
 */
void identify_object( CHAR_DATA *ch, OBJ_DATA *obj )
{
	AFFECT_DATA *paf;
	SKILL_DATA	*sktmp;
	BITVECTOR	*temp = NULL;

	set_char_color( AT_LCYAN, ch );
	ch_printf( ch, "\r\nL'oggetto '%s' è %s",
		obj->short_descr,
		aoran(code_name(NULL, obj->type, CODE_OBJTYPE)) );

	SET_BITS( temp, obj->wear_flags );
	REMOVE_BIT( temp, OBJWEAR_TAKE );
	if ( obj->type != OBJTYPE_LIGHT && !IS_EMPTY(temp) )
		ch_printf( ch, ", locazione:  %s\r\n", code_bit(NULL, temp, CODE_OBJWEAR) );
	else
		send_to_char( ch, ".\r\n" );
	destroybv( temp );

	ch_printf( ch,
		"Proprietà particolari:  %s\r\nPeso %d, valore  %d, e livello %d.\r\n",
		code_bit(NULL, obj->extra_flags, CODE_OBJFLAG),
/*			code_bit(NULL, obj->magic_flags, CODE_OBJMAGIC),	unused for now */
		obj->weight,
		obj->cost,
		obj->level );

	set_char_color( AT_MAGIC, ch );
	switch ( obj->type )
	{
	  case OBJTYPE_CONTAINER:
		ch_printf( ch, "%s sembra essere %s.\r\n", capitalize(obj->short_descr),
			obj->container->capacity < 76  ? "di minima capacità"	:
			obj->container->capacity < 150 ? "di piccola capacità"	:
			obj->container->capacity < 300 ? "di media capacità"	:
			obj->container->capacity < 550 ? "di grande capacità"	:
			obj->container->capacity < 751 ? "di ottima capacità"	:
											 "di superba capacità" );
		break;

	  case OBJTYPE_SCROLL:
		ch_printf( ch, "%d livello dell'incantesimo di:", obj->scroll->level );

		if ( obj->scroll->sn1 >= 0 && (sktmp=get_skilltype(obj->scroll->sn1)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		if ( obj->scroll->sn2 >= 0 && (sktmp=get_skilltype(obj->scroll->sn2)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		if ( obj->scroll->sn3 >= 0 && (sktmp=get_skilltype(obj->scroll->sn3)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		send_to_char( ch, ".\r\n" );
		break;

	  case OBJTYPE_POTION:
		ch_printf( ch, "%d livello dell'incantesimo di:", obj->potion->level );

		if ( obj->potion->sn1 >= 0 && (sktmp=get_skilltype(obj->potion->sn1)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		if ( obj->potion->sn2 >= 0 && (sktmp=get_skilltype(obj->potion->sn2)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		if ( obj->potion->sn3 >= 0 && (sktmp=get_skilltype(obj->potion->sn3)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		send_to_char( ch, ".\r\n" );
		break;

	  case OBJTYPE_PILL:
		ch_printf( ch, "%d livello dell'incantesimo di:", obj->pill->level );

		if ( obj->pill->sn1 >= 0 && (sktmp=get_skilltype(obj->pill->sn1)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		if ( obj->pill->sn2 >= 0 && (sktmp=get_skilltype(obj->pill->sn2)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		if ( obj->pill->sn3 >= 0 && (sktmp=get_skilltype(obj->pill->sn3)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		send_to_char( ch, ".\r\n" );
		break;

	  case OBJTYPE_SALVE:
		ch_printf( ch, "Possiede %d(%d) applicazioni di livello %d",
			obj->salve->charges, obj->salve->max_charges, obj->salve->level );

		if ( obj->salve->sn1 >= 0 && (sktmp=get_skilltype(obj->salve->sn1)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		if ( obj->salve->sn2 >= 0 && (sktmp=get_skilltype(obj->salve->sn2)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		send_to_char( ch, ".\r\n" );
		break;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		ch_printf( ch, "Ha %d(%d) cariche di livello %d",
			obj->wand->curr_charges, obj->wand->max_charges, obj->wand->level );

		if ( obj->wand->sn >= 0 && (sktmp=get_skilltype(obj->wand->sn)) != NULL )
			ch_printf( ch, " '%s'", sktmp->name );

		send_to_char( ch, ".\r\n" );
		break;

	  case OBJTYPE_WEAPON:
	  {
		char	type[MIL];

		/* (FF) quando viene inserito il tipo di arma ricordarsi di aggiungere qui il codice relativo */

		set_char_color( AT_YELLOW, ch );
		switch ( obj->weapon->damage1 )
		{
		  default:				strcpy( type, "esotica" );					break;
		  case DAMAGE_HIT:		strcpy( type, "generica" );					break;
		  case DAMAGE_SLICE:	strcpy( type, "di tipo 'spada corta'" );	break;
		  case DAMAGE_STAB:		strcpy( type, "di tipo 'pugnale'" );		break;
		  case DAMAGE_SLASH:	strcpy( type, "di tipo 'spada lunga'" );	break;
		  case DAMAGE_WHIP:		strcpy( type, "di tipo 'frusta'" );			break;
		  case DAMAGE_CLAW:		strcpy( type, "di tipo 'uncino'" );			break;
		  case DAMAGE_BLAST:	strcpy( type, "magica" );					break;
		  case DAMAGE_POUND:	strcpy( type, "di tipo 'martello'" );		break;
		  case DAMAGE_CRUSH:	strcpy( type, "di tipo 'mazza'" );			break;
		  case DAMAGE_PIERCE:	strcpy( type, "di tipo 'lancia'" );			break;
		  case DAMAGE_BOLT:		strcpy( type, "di tipo 'balestra'" );		break;
		  case DAMAGE_ARROW:	strcpy( type, "di tipo 'arco'" );			break;
		  case DAMAGE_STONE:	strcpy( type, "di tipo 'fionda'" );			break;
		};

		ch_printf( ch, "E' un'arma %s.\n\r", type );
		ch_printf( ch, "Danno %d dadi da %d (media %d)%s\r\n",
			obj->weapon->dice_number, obj->weapon->dice_size,
			(obj->weapon->dice_number * obj->weapon->dice_size)/2,
			HAS_BIT(obj->extra_flags, OBJFLAG_POISONED)
				? ", e può causare avvelenamento."
				: "." );
		break;
	  }

	  case OBJTYPE_ARMOR:
	  case OBJTYPE_DRESS:
		ch_printf( ch, "Classe armatura %d.\r\n", obj->armor->ac );
		break;
	} /* chiude lo switch */

	for ( paf = obj->first_affect; paf; paf = paf->next )
		show_affect( ch, paf );
}


/*
 * Incantesimo di Identify, funziona su mobs/players/objs.
 */
SPELL_RET spell_identify( int sn, int level, CHAR_DATA *ch, void *vo )
{
	/* Made it show short descrs instead of keywords, seeing as you need
	   to know the keyword anyways, we may as well make it look nice */
	OBJ_DATA	*obj;
	CHAR_DATA	*victim;
	AFFECT_DATA *paf;
	SKILL_DATA	*skill = get_skilltype(sn);
	SKILL_DATA	*sktmp;
	char		*name;

	if ( !VALID_STR(target_name) )
	{
		send_to_char( ch, "Cosa dovrei identificare?\r\n" );
		return rSPELL_FAILED;
	}

	if ( (obj = get_obj_carry(ch, target_name, TRUE)) != NULL )	/* Massì qui TRUE visto che si parla di magia.. */
	{
		identify_object( ch, obj );
		return rNONE;
	}
	else if ( (victim = get_char_room(ch, target_name, TRUE)) != NULL )
	{
		if ( HAS_BIT(victim->immune, RIS_MAGIC) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		/*
		 * If they are morphed or a NPC use the appropriate short_desc otherwise use their name
		 */
		if ( victim->morph && victim->morph->morph )
			name = capitalize(victim->morph->morph->short_desc);
		else if ( IS_MOB(victim) )
			name = capitalize(victim->short_descr);
		else
			name = victim->name;

		ch_printf( ch,"%s sembra essere %s&w e seguire la via di %s&w.\r\n",
			name, aoran(get_race(victim, TRUE)), get_class(victim, TRUE) );

		if ( (chance(ch, 50) && get_level(ch) >= get_level(victim)+10 ) || IS_ADMIN(ch) )
		{
			ch_printf( ch, "%s ha su di sé gli incantesimi di: ", name );

			if ( !victim->first_affect )
			{
				send_to_char( ch, "nulla.\r\n" );
				return rNONE;
			}

			for ( paf = victim->first_affect; paf; paf = paf->next )
			{
				if (victim->first_affect != victim->last_affect)
				{
					if ( paf != victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )
						ch_printf( ch, "%s, ", sktmp->name );

					if ( paf == victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )
					{
						ch_printf( ch, "e %s.\r\n", sktmp->name );
						return rNONE;
					}
				}
				else
				{
					if ( (sktmp=get_skilltype(paf->type)) != NULL )
						ch_printf( ch, "%s.\r\n", sktmp->name );
					else
						send_to_char( ch, "\r\n" );
					return rNONE;
				}
			}
		} /* chiude l'if */
	} /* chiude l'else if */
	else
	{
		ch_printf( ch, "Non riesco a trovare %s!\r\n", target_name );
		return rSPELL_FAILED;
	}

	return rNONE;
}


/*
 * Incantesimo di Invisibility.
 */
SPELL_RET spell_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim;
	SKILL_DATA *skill = get_skilltype(sn);

	/* Modifications to work on player/object */
	if ( !VALID_STR(target_name) )
		victim = ch;
	else
		victim = get_char_room( ch, target_name, TRUE );

	if ( victim )
	{
		AFFECT_DATA *aff;

		if ( HAS_BIT(victim->immune, RIS_MAGIC) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		if ( HAS_BIT(victim->affected_by, AFFECT_INVISIBLE) )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		CREATE( aff, AFFECT_DATA, 1 );
		aff->type		 = sn;
		aff->duration  = ( level/4 + 12 ) * 69;
		aff->location  = APPLY_NONE;
		aff->modifier  = 0;
		aff->bitvector = meb( AFFECT_INVISIBLE );
		affect_to_char( victim, aff );

		act( AT_MAGIC, "$n sparisce dalla vista.", victim, NULL, NULL, TO_ROOM );
		act( AT_MAGIC, "Sparisco dalla vista.", victim, NULL, NULL, TO_CHAR );

		return rNONE;
	}
	else
	{
		OBJ_DATA *obj;

		obj = get_obj_carry( ch, target_name, TRUE );	/* TRUE anche se sarebbe assurdo rendere invisibili oggetti già eventualmente tali :P */
		if ( obj )
		{
			split_obj( obj, 1 );

			if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVIS)
			  || chance(ch, level/10 + 40) )
			{
				failed_casting( skill, ch, NULL, NULL );
				return rSPELL_FAILED;
			}

			SET_BIT( obj->extra_flags, OBJFLAG_INVIS );
			act( AT_MAGIC, "$p sparisce dalla vista.", ch, obj, NULL, TO_CHAR );
			return rNONE;
		}
	}

	ch_printf( ch, "Non riesco a trovare %s!\r\n", target_name );
	return rSPELL_FAILED;
}


/*
 * Incantesimo di Know Alignment.
 */
SPELL_RET spell_know_alignment( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	char *msg;
	int ap;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( !victim )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	ap = victim->alignment;

	if		( ap >  700 ) msg = "$N ha un'anima divinamente pura e candida.";
	else if ( ap >  350 ) msg = "$N ha un'anima tersa e libera dal male.";
	else if ( ap >  100 ) msg = "$N ha un'anima gentile e onesta.";
	else if ( ap > -100 ) msg = "$N ha un'anima inquieta in bilico tra bene e male.";
	else if ( ap > -350 ) msg = "$N ha un'anima nera e corrotta.";
	else if ( ap > -700 ) msg = "$N ha nell'anima solo la nera pece del maligno.";
	else				  msg = "Non riesco a comprendere molto riguardo $N.";

	act( AT_MAGIC, msg, ch, NULL, victim, TO_CHAR );
	return rNONE;
}


/*
 * Incantesimo di Lightning Bolt.
 */
SPELL_RET spell_lightning_bolt( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		 1,
		 2,  3,  4,  6,  8, 10, 15, 20, 25, 28,
		31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
		44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
		51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
		58, 58, 59, 60, 60, 61, 62, 62, 63, 64,
		64, 65, 65, 66, 66, 67, 68, 68, 69, 69,
		70, 71, 71, 72, 72, 73, 73, 74, 75, 75
	};
	int dam;

	level = UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level = UMAX( 0, level );

	dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Locate Object.
 */
SPELL_RET spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo )
{
	char	  buf[MIL];
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int		  cnt,
			  found = 0;

	for ( obj = first_object;  obj;  obj = obj->next )
	{
		if ( !can_see_obj(ch, obj, FALSE) )				continue;
		if ( !nifty_is_name(target_name, obj->name) )	continue;
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_NOLOCATE) && !IS_ADMIN(ch) )
			continue;

		found++;

		for ( cnt = 0, in_obj = obj;  in_obj->in_obj && cnt < 100;  in_obj = in_obj->in_obj, ++cnt )
			;

		if ( cnt >= MAX_NEST )
		{
			send_log( NULL, LOG_BUG, "spell_locate_obj: object [%d] %s is nested more than %d times!",
				obj->pObjProto->vnum, obj->short_descr, MAX_NEST );
			continue;
		}

		if ( in_obj->carried_by )
		{
			if ( IS_ADMIN(in_obj->carried_by) && IS_PG(in_obj->carried_by) )
			{
				found--;
				continue;
			}

			sprintf( buf, "%s è in possesso di %s.\r\n",
				obj_short(obj), PERS(in_obj->carried_by, ch) );
		}
		else
		{
			sprintf( buf, "%s in %s.\r\n",
				obj_short(obj),
				(in_obj->in_room == NULL) ? "percepisco l'oggetto ma non la sua ubicazione." : in_obj->in_room->name );
		}

/*		Gorog added this 98/09/02 but obj_short(obj) now nukes memory
		and crashes us when the resulting buffer is sent to the pager
		(not confined to just pager_printf).  Something else somewhere
		else must have changed recently to exacerbate this problem,
		as it is now a guaranteed crash cause.
*/
		buf[0] = toupper(buf[0]);
		set_char_color( AT_MAGIC, ch );

		send_to_pager( ch, buf );
#ifdef T2_ALFA
		/* (TT) se non funziona il pager_printf sopra utilizzare questo send_to_char sotto */
		send_to_char( ch, buf );
#endif
	}

	if ( !found )
	{
		send_to_char( ch, "Non riesco a percepire l'esistenza di nulla di simile.\r\n" );
		return rSPELL_FAILED;
	}

	return rNONE;
}


/*
 * Incantesimo di Magic Missile.
 */
SPELL_RET spell_magic_missile( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		 1,
		 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
		 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
		 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
		11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
		13, 13, 13, 13, 13, 14, 14, 14, 14, 14,
		15, 15, 15, 15, 15, 16, 16, 16, 16, 16,
		17, 17, 17, 17, 17, 18, 18, 18, 18, 18
	};
	int dam;

	level	= UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level	= UMAX( 0, level );
	dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
/*  What's this?  You can't save vs. magic missile!
	if ( saves_spell( level, victim ) )
		dam /= 2;
*/
	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Pass Door.
 */
SPELL_RET spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	AFFECT_DATA	*aff;
	SKILL_DATA	*skill = get_skilltype( sn );

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_PASS_DOOR) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		 = sn;
	aff->duration  = ( number_fuzzy((level+1) / 4) + 1 ) * 69;
	aff->location  = APPLY_NONE;
	aff->modifier  = 0;
	aff->bitvector = meb( AFFECT_PASS_DOOR );
	affect_to_char( victim, aff );

	act( AT_MAGIC, "Il corpo di $n sfuma in colori e forme translucenti e indefinite.", victim, NULL, NULL, TO_ROOM );
	act( AT_MAGIC, "Le molecole del mio corpo separano gli spazi tra di loro permettendomi di diventare translucente.", victim, NULL, NULL, TO_CHAR );

	return rNONE;
}


/*
 * Incantesimo di Poison.
 */
SPELL_RET spell_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	AFFECT_DATA *aff;
	int			poison_chance;
	bool		first = TRUE;

	poison_chance = ris_save( victim, level, RIS_POISON );

	if ( poison_chance == 1000 || compute_savthrow(poison_chance, victim, SAVETHROW_SPELL_STAFF) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "Il veleno non riesce a superare il sistema immunitario della mia vittima.\r\n" );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_POISON) )
		first = FALSE;

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= level * 69;
	aff->location	= APPLY_STR;
	aff->modifier	= -2;
	aff->bitvector	= meb( AFFECT_POISON );
	affect_join( victim, aff );

	set_char_color( AT_GREEN, victim );
	send_to_char( victim, "Lancinanti brividi mi assalgono, sento il veleno percorrere le mie vene.\r\n" );

	victim->mental_state = URANGE( 20, victim->mental_state + (first ? 5 : 0), 100 );

	if ( ch != victim )
	{
		act( AT_GREEN, "$N rabbrividisce agonizzando mentre il veleno si fa strada nel suo corpo.", ch, NULL, victim, TO_CHAR );
		act( AT_GREEN, "$N rabbrividisce agonizzando mentre il veleno si fa strada nel suo corpo.", ch, NULL, victim, TO_NOVICT );
	}

	return rNONE;
}


/*
 * Incantesimo di Remove Curse.
 */
SPELL_RET spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *obj;
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( is_affected(victim, gsn_curse) )
	{
		affect_strip( victim, gsn_curse );
		set_char_color( AT_MAGIC, victim );
		send_to_char( victim, "Il peso che gravava sulla mia anima si dissolve, finalmente.\r\n" );

		if ( ch != victim )
		{
			act( AT_MAGIC, "Riesco a sventare la maledizione che gravava su $N.", ch, NULL, victim, TO_CHAR );
			act( AT_MAGIC, "$N sembra rasserenarsi improvvisamente.", ch, NULL, victim, TO_NOVICT );
		}
	}
	else if ( victim->first_carrying )
	{
		for ( obj = victim->first_carrying; obj; obj = obj->next_content )
		{
			if ( !obj->in_obj
				&& (HAS_BIT(obj->extra_flags, OBJFLAG_NOREMOVE)
				|| HAS_BIT(obj->extra_flags, OBJFLAG_NODROP)) )
			{
				/* (RR) invece di togliere la flag deve gettarlo per terra */
				if ( HAS_BIT(obj->extra_flags, OBJFLAG_NOREMOVE) )
					REMOVE_BIT( obj->extra_flags, OBJFLAG_NOREMOVE );

				if ( HAS_BIT(obj->extra_flags, OBJFLAG_NODROP) )
					REMOVE_BIT( obj->extra_flags, OBJFLAG_NODROP );

				set_char_color( AT_MAGIC, victim );
				send_to_char( victim, "La nera presenza che mi affliggeva si dissolve nel nulla.\r\n" );

				if ( ch != victim )
				{
					act( AT_MAGIC, "Riesco a scacciare la maledizione che gravava su $N.", ch, NULL, victim, TO_CHAR );
					act( AT_MAGIC, "$N sembra essersi liberat$X di un grande peso.", ch, NULL, victim, TO_NOVICT );
				}

				return rNONE;
			}
		}
	}
	return rNONE;
}


/*
 * Incantesimo di Remove Trap.
 */
SPELL_RET spell_remove_trap( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA  *obj;
	OBJ_DATA  *trap;
	bool	  found;
	ch_ret	  retcode;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( !VALID_STR(target_name) )
	{
		send_to_char( ch, "Da dove vuoi rimuovere la trappola?\r\n" );
		return rSPELL_FAILED;
	}

	found = FALSE;

	if ( !ch->in_room->first_content )
	{
		send_to_char( ch, "Non riesco a trovare nulla di simile.\r\n" );
		return rNONE;
	}

	for ( obj = ch->in_room->first_content;  obj;  obj = obj->next_content )
	{
		if ( !can_see_obj(ch, obj, FALSE) )				continue;
		if ( !nifty_is_name(target_name, obj->name) )	continue;

		found = TRUE;
		break;
	}

	if ( !found )
	{
		send_to_char( ch, "Non riesco a trovare nulla di simile..\r\n" );
		return rSPELL_FAILED;
	}

	if ( (trap = get_trap(obj)) == NULL )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}

	if ( !chance(ch, 70 + (get_curr_attr(ch, ATTR_COC)/4) ) )
	{
		send_to_char( ch, "Ooops!\r\n" );
		retcode = spring_trap( ch, trap );

		if ( retcode == rNONE )
			retcode = rSPELL_FAILED;

		return retcode;
	}

	free_object( trap );
	successful_casting( skill, ch, NULL, NULL );

	return rNONE;
}


/*
 * Incantesimo di Shocking Grasp.
 */
SPELL_RET spell_shocking_grasp( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		 1,
		 3,  6,  9, 11, 14,	17, 20, 25, 29, 33,
		36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
		43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
		48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
		53, 53, 54, 54, 55, 55, 56, 56, 57, 57,
		58, 58, 59, 59, 60, 60, 61, 61, 62, 62,
		63, 63, 64, 64, 65, 65, 66, 66, 67, 67
	};
	int dam;

	level = UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level = UMAX( 0, level );

	dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Sleep.
 */
SPELL_RET spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo )
{
	AFFECT_DATA *aff;
	CHAR_DATA	*victim;
	SKILL_DATA	*skill = get_skilltype( sn );
	ch_ret		 retcode;
	int			 sleep_chance;
	int			 tmp;

	victim = get_char_room( ch, target_name, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non è qui.\r\n" );
		return rSPELL_FAILED;
	}

	if ( IS_PG(victim) && victim->fighting )
	{
		send_to_char( ch, "Scorre troppa adrenalina nelle vene di chi combatte, è impossibile da addormentare.\r\n" );
		return rSPELL_FAILED;
	}

	if ( is_safe(ch, victim, TRUE) )
		return rSPELL_FAILED;

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( SPELL_FLAG(skill, SKELL_PKSENSITIVE) && IS_PG(ch) && IS_PG(victim) )
		tmp = level / 2;
	else
		tmp = level;

	if ( HAS_BIT(victim->affected_by, AFFECT_SLEEP)
		|| (sleep_chance = ris_save(victim, tmp, RIS_SLEEP)) == 1000
		|| level*2 < get_level(victim)
		|| (victim != ch && HAS_BIT(victim->in_room->flags, ROOM_SAFE) )
		|| compute_savthrow(sleep_chance, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );

		if ( ch == victim )
			return rSPELL_FAILED;

		if ( !victim->fighting )
		{
			retcode = multi_hit( victim, ch, TYPE_UNDEFINED );

			if ( retcode == rNONE )
				retcode = rSPELL_FAILED;

			return retcode;
		}
	}

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= (level + 4) * 69;
	aff->location	= APPLY_NONE;
	aff->modifier	= 0;
	aff->bitvector	= meb( AFFECT_SLEEP );
	affect_join( victim, aff );

	if ( IS_PG(victim) )
		send_log( NULL, LOG_WARNING, "%s addormenta %s.", ch->name, victim->name );

	if ( is_awake(victim) )
	{
		act( AT_MAGIC, "Una violento torpore mi addormenta i sensi.. mi sento svenire.", victim, NULL, NULL, TO_CHAR );
		act( AT_MAGIC, "$n si accascia in preda ad un torpore mistico.", victim, NULL, NULL, TO_ROOM );
		victim->position = POSITION_SLEEP;
	}

	if ( IS_MOB(victim) )
		start_hating( victim, ch );

	return rNONE;
}


/*
 * Incantesimo di Summon.
 */
SPELL_RET spell_summon( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim;
	SKILL_DATA	*skill = get_skilltype( sn );

	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim
	  || victim == ch
	  || !victim->in_room
	  || HAS_BIT(ch->in_room->flags,	  ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_SAFE)
	  || HAS_BIT(victim->in_room->flags, ROOM_NOSUMMON)
	  || HAS_BIT(victim->in_room->flags, ROOM_NORECALL)
	  || get_level(victim) >= level*2 + 3
	  || victim->fighting
	  || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF)
	  || !can_pkill(ch, victim) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( ch->in_room->area != victim->in_room->area )
	{
		if ( ((IS_MOB(ch) != IS_MOB(victim)) && chance(ch, 30))
		  || ((IS_MOB(ch) == IS_MOB(victim)) && chance(ch, 60)) )
		{
			failed_casting( skill, ch, victim, NULL );
			set_char_color( AT_MAGIC, victim );
			send_to_char( victim, "Avverto per un istante una strana sensazione di peso opprimente.\r\n" );
			return rSPELL_FAILED;
		}
	}

	if ( IS_PG(ch) )
	{
		act( AT_MAGIC, "Un forte senso di nausea mi stordisce!", ch, NULL, NULL, TO_CHAR );
		act( AT_MAGIC, "$n collassa a terra, stordit$x!", ch, NULL, NULL, TO_ROOM );
		ch->position = POSITION_STUN;

		send_log( NULL, LOG_WARNING, "%s summoned %s to room %d.", ch->name, victim->name, ch->in_room->vnum );
	}

	act( AT_MAGIC, "$n scompare improvvisamente.", victim, NULL, NULL, TO_ROOM );
	char_from_room( victim );
	char_to_room( victim, ch->in_room );
	act( AT_MAGIC, "$n appare dal nulla.", victim, NULL, NULL, TO_ROOM );
	act( AT_MAGIC, "$N mi ha evocato!", victim, NULL, ch,   TO_CHAR );
	send_command( victim, "look", CO );

	return rNONE;
}


/*
 * (FF) Fare una funzione generica in futuro che checckerà tutti i tipi di teleporting
 *	passandogli un argomento per il tipo
 */
bool can_astral( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( victim == ch
	  || !victim->in_room
	  || HAS_BIT(victim->in_room->flags, ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_DEATH)
	  || HAS_BIT(ch->in_room->flags, ROOM_NORECALL)
	  || get_level(victim) >= get_level(ch) + 15
	  || !can_pkill(ch, victim)
	  || compute_savthrow(get_level(ch)/2, victim, SAVETHROW_SPELL_STAFF) )
	{
		return FALSE;
	}

	return TRUE;
}


/*
 * Incantesimo di Astral Walk.
 * Travel via the astral plains to quickly travel to desired location
 * Uses SMAUG spell messages is available to allow use as a SMAUG spell
 * (RR) controllare che non abbia bisogno di altri check oltre get_char_area() e il resto.
 */
SPELL_RET spell_astral_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim;
	SKILL_DATA	*skill = get_skilltype( sn );

	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim || !can_astral(ch, victim) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	/* (TT) Forse nel file spells.dat manca HitChar e HitRoom */
	act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
	act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );
	act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOVICT );

	char_from_room( ch );
	char_to_room  ( ch, victim->in_room );

	act( AT_MAGIC, skill->hit_dest, ch, NULL, victim, TO_NOVICT );

	send_command( ch, "look", CO );
	return rNONE;
}


/*
 * Incantesimo di Teleport.
 * (RR) pensare se inserire altre restrizioni, per esempio teletrasportarsi solo sulle aree del piano attuale
 */
SPELL_RET spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	ROOM_DATA	*pRoom;
	SKILL_DATA	*skill = get_skilltype(sn);

	if ( !victim->in_room
	  || HAS_BIT(victim->in_room->flags, ROOM_NORECALL)
	  || ( IS_PG(ch) && victim->fighting )
	  || ( victim != ch
	  && (compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) || compute_savthrow(level, victim, SAVETHROW_ROD_WANDS))) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	for ( ; ; )
	{
		pRoom = get_room_index( NULL, number_range(1, MAX_VNUM-1) );
		if ( pRoom )
		{
			if ( HAS_BIT(pRoom->flags, ROOM_NOASTRAL) )	continue;
			if ( HAS_BIT(pRoom->flags, ROOM_NORECALL) )	continue;
			if ( HAS_BIT(pRoom->area->flags, AREAFLAG_NOTELEPORT) )	continue;

			/* (FF) se vittima e ch si trovano nello stesso piano o nel piano wild allora breaka */
			break;
		}
	}

	act( AT_MAGIC, "$n scompare improvvisamente!", victim, NULL, NULL, TO_ROOM );
	char_from_room( victim );
	char_to_room( victim, pRoom );

	if ( IS_PG(victim) )
		act( AT_MAGIC, "$n appare dal nulla!", victim, NULL, NULL, TO_ROOM );

	send_command( victim, "look", CO );
	return rNONE;
}


/*
 * Incantesimo di Ventriloquate.
 */
SPELL_RET spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *vch;
	char	   buf1[MSL];
	char	   buf2[MSL];
	char	   speaker[MIL];

	target_name = one_argument( target_name, speaker );

	if ( !VALID_STR(target_name) )
	{
		send_to_char( ch, "Cosa dovrei dire tramite il ventriloquio?" );
		return rSPELL_FAILED;
	}

	sprintf( buf1, "%s dice '%s'.\r\n",				 speaker, target_name );
	sprintf( buf2, "%s sembra che stia dicendo '%s'.\r\n", speaker, target_name );
	buf1[0] = toupper( buf1[0] );

	for ( vch = ch->in_room->first_person;  vch;  vch = vch->next_in_room )
	{
		if ( !is_name(speaker, vch->name) )
		{
			set_char_color( AT_SAY, vch );
			send_to_char( vch, compute_savthrow(level, vch, SAVETHROW_SPELL_STAFF)  ?  buf2  :  buf1 );
		}
	}

	return rNONE;
}


/*
 * Incantesimo di Weaken.
 */
SPELL_RET spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	AFFECT_DATA	*aff;
	SKILL_DATA	*skill = get_skilltype( sn );

	set_char_color( AT_MAGIC, ch );

	if ( HAS_BIT(victim->immune, RIS_MAGIC) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( is_affected(victim, sn) || compute_savthrow(level, victim, SAVETHROW_ROD_WANDS) )
	{
		send_to_char( ch, "L'incantesimo non riesce a vincere la resistenza della mia vittima.\r\n" );
		return rSPELL_FAILED;
	}

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= level/2 * 69;
	aff->location	= APPLY_STR;
	aff->modifier	= -2;
	affect_to_char( victim, aff );

	set_char_color( AT_MAGIC, victim );
	send_to_char( victim, "Una devastante debolezza mi pervade!\r\n" );

	if ( ch != victim )
	{
		if ( get_curr_attr(victim, ATTR_STR) < 20 )	/* (bb) bisognerebbe invece fare un check sulla percentuale di forza che aveva prima */
		{
			act( AT_MAGIC, "$N ansima barcollando mentre la debolezza pervade i suoi muscoli.", ch, NULL, victim, TO_CHAR );
			act( AT_MAGIC, "$N ansima barcollando mentre la debolezza pervade i suoi muscoli.", ch, NULL, victim, TO_NOVICT );
		}
		else
		{
			act( AT_MAGIC, "$N sembra quasi non reggersi più in piedi grazie alla debolezza indotta dall'incantesimo.", ch, NULL, victim, TO_CHAR );
			act( AT_MAGIC, "$n induce una devastante debolezza nella muscolatura di $N che fatica a reggersi in piedi!", ch, NULL, victim, TO_NOVICT );
		}
	}
	return rNONE;
}


/*
 * A spell as it should be
 */
SPELL_RET spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo )
{
	ROOM_DATA	*location;
	CHAR_DATA	*opponent;

	location = NULL;

	if ( IS_PG(ch) && ch->pg->clan )
		location = ch->pg->clan->recall;

#ifdef T2_ALFA
	/* (FF) da fare in futuro con le nazioni, ci sono altri pezzi di codice simile a questo che necessiterebbero se si aggiunge questa feature */
	if ( !location )
		location = get_room_index(table_race[ch->race]->race_recall );
#endif

	if ( !location )
		location = get_room_index( NULL, VNUM_ROOM_TEMPLE );

	if ( !location )
	{
		send_to_char( ch, "Perdo completamente la cognizione dello spazio e del tempo.\r\n" );
		return rNONE;
	}

	if ( ch->in_room == location )
		return rNONE;

	if ( HAS_BIT(ch->in_room->flags, ROOM_NORECALL) )
	{
		send_to_char( ch, "Per chissà quale strana ragione non succede nulla..\r\n" );
		return rNONE;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_CURSE) && !HAS_BIT(ch->in_room->flags, ROOM_DEATH) )
	{
		send_to_char( ch, "Una maledizione pesa su di me impedendomi il ritorno!\r\n" );
		return rNONE;
	}

	if ( (opponent = who_fighting(ch)) != NULL )
	{
		int lose;

		if ( number_bits(1) == 0 || (IS_PG(opponent) && number_bits(3) > 1) )
		{
			WAIT_STATE( ch, PULSE_IN_SECOND );
			lose = get_level( ch ) * 2;
			gain_exp( ch, 0 - lose, FALSE );
			send_to_char( ch, "La mia preghiera per il ritorno non viene esaudita!\r\n" );
			return rNONE;
		}

		lose = get_level( ch ) * 4;
		gain_exp( ch, 0 - lose, FALSE );
		send_to_char( ch, "La preghiera mi salva dal combattimento!\r\n" );
		stop_fighting( ch, TRUE );
    }

	if ( HAS_BIT(ch->affected_by, AFFECT_INVISIBLE) )
		REMOVE_BIT( ch->affected_by, AFFECT_INVISIBLE );
	if ( HAS_BIT(ch->affected_by, AFFECT_SNEAK) )
		REMOVE_BIT( ch->affected_by, AFFECT_SNEAK );
	if ( HAS_BIT(ch->affected_by, AFFECT_HIDE) )
		REMOVE_BIT( ch->affected_by, AFFECT_HIDE );

	act( AT_ACTION, "$n svanisce in una nube di fumi colorati.", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, location );

	if ( ch->mount )
	{
		char_from_room( ch->mount );
		char_to_room( ch->mount, location );
	}
	act( AT_ACTION, "$n appare dal nulla.", ch, NULL, NULL, TO_ROOM );
	send_command( ch, "look", CO );

	return rNONE;
}


/**----------------------
 ** Incantesimi dei NPC.
 **/

/*
 * Incantesimo di Acid Breath.
 */
SPELL_RET spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA  *obj_lose;
	OBJ_DATA  *obj_next;
	int		   dam;
	int		   hpch;

	if ( chance(ch, level*2) && !compute_savthrow(level, victim, SAVETHROW_BREATH) )
	{
		for ( obj_lose = victim->first_carrying;  obj_lose;  obj_lose = obj_next )
		{
			int iWear;

			obj_next = obj_lose->next_content;

			if ( number_bits(2) != 0 )
				continue;

			switch ( obj_lose->type )
			{
			  case OBJTYPE_ARMOR:
			  case OBJTYPE_DRESS:
				if ( obj_lose->armor->ac > 0 )
				{
					split_obj( obj_lose, 1 );
					act( AT_DAMAGE, "$p si distrugge a causa dell'acido!", victim, obj_lose, NULL, TO_CHAR );

					if ( ( iWear = obj_lose->wear_loc ) != WEARLOC_NONE )
						victim->armor += apply_ac( obj_lose, iWear );
					obj_lose->armor->ac -= 1;
					obj_lose->cost		= 0;
					if ( iWear != WEARLOC_NONE )
						victim->armor -= apply_ac( obj_lose, iWear );
				}
				break;

			  case OBJTYPE_CONTAINER:
				split_obj( obj_lose, 1 );
				act( AT_DAMAGE, "$p si dissolve a causa dell'acido!",
					victim, obj_lose, NULL, TO_CHAR );
				act( AT_OBJECT, "Il contenuto di $p cade per terra intorno a $N.",
					victim, obj_lose, victim, TO_ROOM );
				act( AT_OBJECT, "Il contenuto di $p si riversa per terra.",
					victim, obj_lose, NULL, TO_CHAR );
				empty_obj( obj_lose, NULL, victim->in_room );
				free_object( obj_lose );
				break;
			}
		}
	} /* chiude l'if */

	hpch = UMAX( 10, ch->points[POINTS_LIFE] );
	dam  = number_range( hpch/16+1, hpch/8 );
	if ( compute_savthrow(level, victim, SAVETHROW_BREATH) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Fire Breath.
 */
SPELL_RET spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA  *obj_lose;
	OBJ_DATA  *obj_next;
	int		  dam;
	int		  hpch;

	if ( chance(ch, level*2) && !compute_savthrow(level, victim, SAVETHROW_BREATH) )
	{
		for ( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
		{
			char *msg;

			obj_next = obj_lose->next_content;

			if ( number_bits( 2 ) != 0 )
				continue;

			switch ( obj_lose->type )
			{
			  default:				continue;
			  case OBJTYPE_CONTAINER:	msg = "$p brucia e diventa polvere!";	break;
			  case OBJTYPE_POTION:		msg = "$p ribolle e schizza via!";		break;
			  case OBJTYPE_SCROLL:		msg = "$p brucia diventando cenere!";	break;
			  case OBJTYPE_STAFF:		msg = "$p si frantuma in polvere!";		break;
			  case OBJTYPE_WAND:		msg = "$p scoppietta e brucia!";		break;
			  case OBJTYPE_COOK:
			  case OBJTYPE_FOOD:		msg = "$p si carbonizza!";				break;
			  case OBJTYPE_PILL:		msg = "$p brucia e si scioglie!";		break;
			}

			split_obj( obj_lose, 1 );
			act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );

			if ( obj_lose->type == OBJTYPE_CONTAINER )
			{
				act( AT_OBJECT, "Il contenuto di $p cade per terra intorno a $N.",
					victim, obj_lose, victim, TO_ROOM );
				act( AT_OBJECT, "Il contenuto di $p si riversa per terra!",
					victim, obj_lose, NULL, TO_CHAR );
				empty_obj( obj_lose, NULL, victim->in_room );
			}
			free_object( obj_lose );
		}
	}

	hpch = UMAX( 10, ch->points[POINTS_LIFE] );
	dam  = number_range( hpch/16+1, hpch/8 );
	if ( compute_savthrow(level, victim, SAVETHROW_BREATH) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Frost Breath.
 */
SPELL_RET spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA  *obj_lose;
	OBJ_DATA  *obj_next;
	int		  dam;
	int		  hpch;

	if ( chance(ch, level*2) && !compute_savthrow(level, victim, SAVETHROW_BREATH) )
	{
		for ( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
		{
			char *msg;

			obj_next = obj_lose->next_content;

			if ( number_bits( 2 ) != 0 )
				continue;

			switch ( obj_lose->type )
			{
			  default:				continue;
			  case OBJTYPE_CONTAINER:
			  case OBJTYPE_DRINK_CON:
			  case OBJTYPE_POTION:		msg = "$p congela e si frantuma!";	break;
			}

			split_obj( obj_lose, 1 );
			act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );

			if ( obj_lose->type == OBJTYPE_CONTAINER )
			{
				act( AT_OBJECT, "Il contenuto di $p cade per terra attorno a $N.",
					victim, obj_lose, victim, TO_ROOM );
				act( AT_OBJECT, "Il contenuto di $p si riversa per terra!",
					victim, obj_lose, NULL, TO_CHAR );
				empty_obj( obj_lose, NULL, victim->in_room );
			}
			free_object( obj_lose );
		}
	}

	hpch = UMAX( 10, ch->points[POINTS_LIFE] );
	dam  = number_range( hpch/16+1, hpch/8 );
	if ( compute_savthrow(level, victim, SAVETHROW_BREATH) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Incantesimo di Gas Breath
 */
SPELL_RET spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int		   dam;
	int		   hpch;
	bool	   ch_died;

	ch_died = FALSE;

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "Non posso farlo qui.\r\n" );
		return rNONE;
	}

	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
		vch_next = vch->next_in_room;

		if ( IS_ADMIN(vch) )								continue;

		if ( IS_MOB(ch) ? IS_PG(vch) : IS_MOB(vch) )
		{
			hpch = UMAX( 10, ch->points[POINTS_LIFE] );
			dam  = number_range( hpch/16+1, hpch/8 );
			if ( compute_savthrow(level, vch, SAVETHROW_BREATH) )
				dam /= 2;

			if ( damage(ch, vch, dam, sn) == rCHAR_DIED || char_died(ch) )
				ch_died = TRUE;
		}
	}
	if ( ch_died )
		return rCHAR_DIED;
	else
		return rNONE;
}


/*
 * Incantesimo di Lightning Breath.
 */
SPELL_RET spell_lightning_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
	int hpch;

	hpch = UMAX( 10, ch->points[POINTS_LIFE] );
	dam = number_range( hpch/16+1, hpch/8 );
	if ( compute_savthrow(level, victim, SAVETHROW_BREATH) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/**
 ** Haus' Spell Additions
 **/

/* to do: portal		(like mpcreatepassage)
 *		enchant armour?	(say -1/-2/-3 ac )
 *		sharpness		(makes weapon of caster's level)
 *		repair			(repairs armor)
 *		blood burn		(offensive)		* name: net book of spells *
 *		spirit scream	(offensive)		* name: net book of spells *
 *		something about saltpeter or brimstone
 */

/*
 * Incantesimo di Transport. Working on DM's transport eq suggestion.
 */
SPELL_RET spell_transport( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim;
	char	  arg3[MSL];
	OBJ_DATA  *obj;
	SKILL_DATA *skill = get_skilltype(sn);

	target_name = one_argument( target_name, arg3 );

	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim
	  ||  victim == ch
	  || HAS_BIT(victim->in_room->flags, ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_DEATH)
	  || HAS_BIT(ch->in_room->flags, ROOM_NORECALL)
	  || get_level(victim) >= level*2 + 15
	  || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( victim->in_room == ch->in_room )
	{
		send_to_char( ch, "Sono proprio dietro di me!" );
		return rSPELL_FAILED;
	}

	if ( (obj = get_obj_carry(ch, arg3, TRUE)) == NULL
	  || (victim->carry_weight + get_obj_weight(obj)) > can_carry_weight(victim) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	split_obj( obj, 1 );		/* altrag shoots, haus alley-oops! */

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_NODROP) )
	{
		send_to_char( ch, "Non riesco ad abbandonare l'oggetto.\r\n" );
		return rSPELL_FAILED;	/* nice catch, caine */
	}

	act( AT_MAGIC, "$p svanisce nell'antimateria...", ch, obj, NULL, TO_CHAR );
	act( AT_MAGIC, "$p si smaterializza dalle mani di $n..", ch, obj, NULL, TO_ROOM );

	obj_from_char( obj );
	obj_to_char	 ( obj, victim );

	act( AT_MAGIC, "$p da $n appare nelle mie mani!", ch, obj, victim, TO_VICT );
	act( AT_MAGIC, "$p appare nelle mani di $n!", victim, obj, NULL, TO_ROOM );

	save_player( ch );
	save_player( victim );

	return rNONE;
}


/*
 * Incantesimo di Portal.
 * Syntax portal (mob/char)
 * opens a 2-way EXIT_PORTAL from caster's room to room inhabited by
 *  mob or character won't mess with existing exits
 * (RR) controllare anche qui i passaggi verso altre aree proibite, come quelle NOPOINTLOSS e magari ARENA
 *
 * do_mpopen_passage, combined with spell_astral
 */
SPELL_RET spell_portal( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim;
	ROOM_DATA	*targetRoom;
	ROOM_DATA	*fromRoom;
	OBJ_DATA	*portalObj;
	EXIT_DATA	*pexit;
	SKILL_DATA	*skill = get_skilltype(sn);
	char		 buf[MSL];
	int			 targetRoomVnum;

	/* No go if all kinds of things aren't just right, including the caster
		and victim are not both pkill or both peaceful.	*/
	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim
	  ||  victim == ch
	  || !victim->in_room
	  || HAS_BIT(victim->in_room->flags,	ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags,	ROOM_DEATH)
	  || HAS_BIT(victim->in_room->flags,	ROOM_NORECALL)
	  || HAS_BIT(ch->in_room->flags,		ROOM_NORECALL)
	  || HAS_BIT(ch->in_room->flags,		ROOM_NOASTRAL)
	  || get_level(victim) >= level*2 + 15
	  || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF)
	  || !can_pkill(ch, victim) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( victim->in_room == ch->in_room )
	{
		send_to_char( ch, "Sono proprio dietro di me!" );
		return rSPELL_FAILED;
	}

	targetRoomVnum = victim->in_room->vnum;
	fromRoom = ch->in_room;
	targetRoom = victim->in_room;

	/* Check if there already is a portal in either room. */
	for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
	{
		if ( HAS_BIT( pexit->flags, EXIT_PORTAL ) )
		{
			send_to_char( ch, "C'è già un portale qui.\r\n" );
			return rSPELL_FAILED;
		}

		if ( DIR_SOMEWHERE == pexit->vdir )
		{
			send_to_char( ch, "Non posso creare un portale in quella stanza.\r\n" );
			return rSPELL_FAILED;
		}
	}

	for ( pexit = targetRoom->first_exit; pexit; pexit = pexit->next )
	{
		if ( DIR_SOMEWHERE == pexit->vdir )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}
	}
	pexit = make_exit( fromRoom, targetRoom, DIR_SOMEWHERE );
	DISPOSE( pexit->keyword );
	DISPOSE( pexit->description );
	pexit->keyword		= str_dup( "portale" );
	pexit->description	= str_dup( "Attraverso il portale lasciandomi trasportare nel flusso spazio-temporale..\r\n" );
	pexit->key			= -1;
	pexit->flags		= multimeb( EXIT_PORTAL, EXIT_xENTER, EXIT_HIDDEN, EXIT_xLOOK, -1 );
	pexit->vnum			= targetRoomVnum;

	portalObj = make_object( get_obj_proto(NULL, VNUM_OBJ_PORTAL), 0 );
	portalObj->timer = 3;
	sprintf( buf, "a portal created by %s", ch->name );
	DISPOSE( portalObj->short_descr );
	portalObj->short_descr = str_dup( buf );

	act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
	act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );
	act( AT_MAGIC, skill->hit_vict, victim, NULL, victim, TO_ROOM );

	portalObj = obj_to_room( portalObj, ch->in_room );

	pexit = make_exit( targetRoom, fromRoom, DIR_SOMEWHERE );
	DISPOSE( pexit->keyword );
	DISPOSE( pexit->description );
	pexit->keyword		= str_dup( "portale" );
	pexit->description	= str_dup( "Attraverso il portale..\r\n" );
	pexit->key			= -1;
	pexit->flags		= multimeb( EXIT_PORTAL, EXIT_xENTER, EXIT_HIDDEN, -1 );
	pexit->vnum			= targetRoomVnum;

	portalObj = make_object( get_obj_proto(NULL, VNUM_OBJ_PORTAL), 0 );
	portalObj->timer = 3;
	DISPOSE( portalObj->short_descr );
	portalObj->short_descr = str_dup( buf );
	portalObj = obj_to_room( portalObj, targetRoom );
/*
	send_log( NULL, LOG_WARNING, "%s has made a portal from room %d to room %d.",
		ch->name, fromRoom->vnum, targetRoomVnum );
*/
	return rNONE;
}


/*
 * Incantesimo di Farsight.
 */
SPELL_RET spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo )
{
	ROOM_DATA	*location;
	ROOM_DATA	*original;
	CHAR_DATA	*victim;
	SKILL_DATA	*skill = get_skilltype( sn );

	/* The spell fails if the victim isn't playing, the victim is the caster,
		the target room has private, solitary, noastral or death flags,
		the caster's room is norecall, the victim is too high in level, the
		victim makes the saving throw or the pkill flag on the caster is
		not the same as on the victim.  Got it?
	*/
	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim
	  ||  victim == ch
	  || !victim->in_room
	  || HAS_BIT(victim->in_room->flags, ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_DEATH)
	  || HAS_BIT(ch->in_room->flags,	  ROOM_NORECALL)
	  || get_level(victim) >= level*2 + 15
	  || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF)
	  || !can_pkill(ch, victim) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	location = victim->in_room;

	if ( !location )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	successful_casting( skill, ch, victim, NULL );
	original = ch->in_room;
	char_from_room( ch );
	char_to_room( ch, location );
	send_command( ch, "look", CO );
	char_from_room( ch );
	char_to_room( ch, original );
	return rNONE;
}


/*
 * Incantesimo di Recharge.
 */
SPELL_RET spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if ( obj->type == OBJTYPE_STAFF
	  || obj->type == OBJTYPE_WAND )
	{
		split_obj( obj, 1 );
		if ( obj->wand->curr_charges == obj->wand->curr_charges
		  || obj->wand->max_charges > (obj->pObjProto->wand->max_charges * 4) )
		{
			act( AT_FIRE, "$p scoppietta tra fiamme ardenti!", ch, obj, NULL, TO_CHAR );
			act( AT_FIRE, "$p scoppietta tra fiamme ardenti bruciando $n!", ch, obj, NULL, TO_ROOM);
			free_object( obj );

			if ( damage(ch, ch, obj->level, TYPE_UNDEFINED) == rCHAR_DIED || char_died(ch) )
				return rCHAR_DIED;
			else
				return rSPELL_FAILED;
		}

		if ( chance(ch, 2) )
		{
			act( AT_YELLOW, "$p risplende di un'accecante bagliore magico.",
				ch, obj, NULL, TO_CHAR );
			obj->wand->max_charges *= 2;
			obj->wand->curr_charges = obj->wand->max_charges;
			return rNONE;
		}
		else if ( chance(ch, 5) )
		{
			act( AT_YELLOW, "$p risplende di luce propria.. solo per qualche istante..",
				ch, obj, NULL, TO_CHAR);
			obj->wand->curr_charges = obj->wand->max_charges;
			return rNONE;
		}
		else if ( chance(ch, 10) )
		{
			act( AT_WHITE, "$p si disintegra in mille pezzi!", ch, obj, NULL, TO_CHAR );
			act( AT_WHITE, "$n disintegra $p nel tentativo di ristabilirne il flusso magico.",
				ch, obj, NULL, TO_ROOM );
			free_object( obj );
			return rSPELL_FAILED;
		}
		else if ( chance(ch, LVL_LEGEND/2 - get_level(ch)/4) )
		{
			send_to_char( ch, "Non accade nulla.\r\n" );
			return rSPELL_FAILED;
		}
		else
		{
			act( AT_MAGIC, "$p scotta!", ch, obj, NULL, TO_CHAR );
			--obj->wand->max_charges;
			obj->wand->curr_charges = obj->wand->max_charges;
			return rNONE;
		}
	}
	else
	{
		send_to_char( ch, "Non posso farlo!\r\n" );
		return rSPELL_FAILED;
	}
}


/*
 * Incantesimo di Plant Pass. Idea derivata dall' AD&D 2nd ed
 */
SPELL_RET spell_plant_pass( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim;
	SKILL_DATA *skill = get_skilltype( sn );

	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim
	  ||  victim == ch
	  || !victim->in_room
	  || HAS_BIT(victim->in_room->flags, ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_DEATH)
	  || (victim->in_room->sector	!= SECTOR_FOREST
	  &&  victim->in_room->sector	!= SECTOR_FIELD)
	  || (ch->in_room->sector		!= SECTOR_FOREST
	  &&  ch->in_room->sector		!= SECTOR_FIELD)
	  || HAS_BIT(ch->in_room->flags,	 ROOM_NORECALL)
	  || get_level(victim) >= level*2 + 15
	  || !can_pkill(ch, victim)
	  || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( ch->in_room->sector == SECTOR_FOREST )
		act( AT_MAGIC, "$n fonde il suo corpo con un albero qui vicino!", ch, NULL, NULL, TO_ROOM );
	else
		act( AT_MAGIC, "$n fonde il suo corpo con l'erba sul suolo!", ch, NULL, NULL, TO_ROOM );

	char_from_room( ch );
	char_to_room( ch, victim->in_room );

	if ( ch->in_room->sector == SECTOR_FOREST )
		act( AT_MAGIC, "$n appare da un albero qui vicino!", ch, NULL, NULL, TO_ROOM );
	else
		act( AT_MAGIC, "$n sbuca da un ciuffo d'erba nei paraggi..", ch, NULL, NULL, TO_ROOM );

	send_command( ch, "look", CO );
	return rNONE;
}


/*
 * Incantesimo di Mist Walk, versione vampirica della astral walk.
 */
SPELL_RET spell_mist_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA  *victim;
	SKILL_DATA *skill = get_skilltype(sn);
	bool		allowday = FALSE;

	set_char_color( AT_DGREEN, ch );

	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim || victim == ch )
	{
		failed_casting( skill, ch, victim, NULL );
		send_to_char( ch, "Non riesco a percepire la mia vittima..." );
		return rSPELL_FAILED;
	}

	if ( ch->pg->condition[CONDITION_BLOODTHIRST] > 22 )
		allowday = TRUE;

	if ( (calendar.hour < 21 && calendar.hour > 5 && !allowday )
		|| !victim->in_room
		|| HAS_BIT(victim->in_room->flags,	ROOM_NOASTRAL)
		|| HAS_BIT(victim->in_room->flags,	ROOM_DEATH)
		|| HAS_BIT(ch->in_room->flags,		ROOM_NORECALL)
		|| get_level(victim) >= level*2 + 15
		|| !can_pkill(ch, victim)
		|| compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		send_to_char( ch, "Non riesco a percepire la mia vittima..." );
		return rSPELL_FAILED;
	}

	/* Subtract 22 extra bp for mist walk from 0500 to 2100 */
	if ( calendar.hour < 21 && calendar.hour > 5 && IS_PG(ch) )
		gain_condition( ch, CONDITION_BLOODTHIRST, - 22 );

	act( AT_DGREEN, "$n muta il suo corpo in nebbia e svanisce nel nulla!",
		ch, NULL, NULL, TO_ROOM );

	char_from_room( ch );
	char_to_room( ch, victim->in_room );

	act( AT_DGREEN, "Una fitta nube nebbiosa invade l'ambiente per poi tramutarsi in un vortice di particelle che svelano la presenza di $n!",
		ch, NULL, NULL, TO_ROOM );

	send_command( ch, "look", CO );
	return rNONE;
}


/*
 * Incantesimo di Solar flight, versione clericale dell'Astral Walk.
 */
SPELL_RET spell_solar_flight( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	 *victim;
	SKILL_DATA	 *skill = get_skilltype(sn);
	WEATHER_DATA *weath = ch->in_room->area->weather;

	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim
	  ||  victim == ch
	  || (calendar.hour > 18 || calendar.hour < 8)
	  || !victim->in_room
	  || !is_outside(ch)
	  || !is_outside(victim)
	  || weath->precip >= 0
	  || HAS_BIT(victim->in_room->flags, ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_DEATH)
	  || HAS_BIT(ch->in_room->flags,	 ROOM_NORECALL)
	  || get_level(victim) >= level*2 + 15
	  || !can_pkill(ch, victim)
	  || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	act( AT_MAGIC, "$n svanisce in un'esplosione di luce!", ch, NULL, NULL, TO_ROOM );

	char_from_room( ch );
	char_to_room( ch, victim->in_room );

	act( AT_MAGIC, "$n appare in una sfolgorante esplosione di luci!",
		ch, NULL, NULL, TO_ROOM );

	send_command( ch, "look", CO );
	return rNONE;
}



/*
 * Incantesimo di Remove Invisibility.
 */
SPELL_RET spell_remove_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *obj;
	SKILL_DATA *skill = get_skilltype(sn);

	if ( !VALID_STR(target_name) )
	{
		send_to_char( ch, "Su cosa o chi dovrei lanciare l'incantesimo?\r\n" );
		return rSPELL_FAILED;
	}

	obj = get_obj_carry( ch, target_name, TRUE );	/* Sì, TRUE, può togliere l'invis anche se non lo vede */
	if ( obj )
	{
		if ( !HAS_BIT(obj->extra_flags, OBJFLAG_INVIS) )
		{
			send_to_char( ch, "Non è invisibile!\r\n" );
			return rSPELL_FAILED;
		}

		REMOVE_BIT(obj->extra_flags, OBJFLAG_INVIS);
		act( AT_MAGIC, "$p torna visibile.", ch, obj, NULL, TO_CHAR );

		send_to_char( ch, "Ok.\r\n" );
		return rNONE;
	}
	else
	{
		CHAR_DATA *victim;

		victim = get_char_room( ch, target_name, TRUE );
		if ( victim )
		{
			if ( !can_see(ch, victim) )
			{
				ch_printf( ch, "Non vedo %s.\r\n", target_name );
				return rSPELL_FAILED;
			}

			if ( !HAS_BIT(victim->affected_by, AFFECT_INVISIBLE) )
			{
				send_to_char( ch, "Non è invisibile!\r\n" );
				return rSPELL_FAILED;
			}

			if ( is_safe(ch, victim, TRUE) )
			{
				failed_casting( skill, ch, victim, NULL );
				return rSPELL_FAILED;
			}

			if ( HAS_BIT(victim->immune, RIS_MAGIC) )
			{
				immune_casting( skill, ch, victim, NULL );
				return rSPELL_FAILED;
			}

			if ( IS_PG(victim) )
			{
				if ( chance(ch, 50) && get_level(ch)/2 + 10 < get_level(victim)/2 )
				{
					failed_casting( skill, ch, victim, NULL );
					return rSPELL_FAILED;
				}
				else
				{
					is_illegal_pk(ch, victim);
				}
			}
			else
			{
				if ( chance(ch, 50) && get_level(ch)/2 + 15 < get_level(victim) )
				{
					failed_casting( skill, ch, victim, NULL );
					return rSPELL_FAILED;
				}
			}

			affect_strip( victim, gsn_invis					);
			affect_strip( victim, gsn_mass_invis			);
			REMOVE_BIT ( victim->affected_by, AFFECT_INVISIBLE);
			successful_casting( skill, ch, victim, NULL );
			return rNONE;
		}

		ch_printf( ch, "Non riesco a trovare %s.\r\n", target_name );
		return rSPELL_FAILED;
	} /* chiude l'else */
}


/*
 * Incantesimo di Animate Dead.
 */
SPELL_RET spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA		*mob;
	OBJ_DATA		*corpse;
	OBJ_DATA		*corpse_next;
	OBJ_DATA		*obj;
	OBJ_DATA		*obj_next;
	MOB_PROTO_DATA	*pMobIndex;
	SKILL_DATA		*skill = get_skilltype(sn);
	bool			 found;
	char			 buf[MSL];

	found = FALSE;

	for ( corpse = ch->in_room->first_content;  corpse;  corpse = corpse_next )
	{
		corpse_next = corpse->next_content;

		if ( corpse->type == OBJTYPE_CORPSE_MOB && corpse->cost != -5 )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char( ch, "Non riesco a trovare un corpo adatto allo scopo.\r\n" );
		return rSPELL_FAILED;
	}

	if ( get_mob_index(NULL, VNUM_MOB_ANIMATED_CORPSE) == NULL )
	{
		send_log( NULL, LOG_BUG, "spell_animate_dead: Vnum 5 not found for spell_animate_dead!" );
		return rNONE;
	}

	if ( (pMobIndex = get_mob_index(NULL, (int) abs(corpse->cost))) == NULL )
	{
		send_log( NULL, LOG_BUG, "spell_animate_dead: Can not find mob for cost of corpse, spell_animate_dead" );
		return rSPELL_FAILED;
	}

	if ( IS_PG(ch) )
	{
		if ( ch->points[POINTS_MANA] - pMobIndex->level*2 < 0 && !stop_lowering_points(ch, POINTS_MANA) )
		{
			send_to_char( ch, "Non ho sufficiente mana per rianimare.\r\n" );
			return rSPELL_FAILED;
		}
		else
		{
			ch->points[POINTS_MANA] -= pMobIndex->level * 2;
		}
	} /* chiude l'if */

	if ( IS_ADMIN(ch) || (chance(ch, 75) && pMobIndex->level/2 - get_level(ch)/2 < 10) )
	{
		AFFECT_DATA		*aff;

		mob			= make_mobile( get_mob_index(NULL, VNUM_MOB_ANIMATED_CORPSE) );
		char_to_room( mob, ch->in_room );
		mob->level	= UMIN( get_level(ch) / 2, pMobIndex->level );
		mob->race	= pMobIndex->race;			/* should be undead */

		/* Fix so mobs wont have 0 hps and crash mud */
		if ( !pMobIndex->hitnodice )
		{
			mob->points_max[POINTS_LIFE] = pMobIndex->level * 4 + number_range(
				pMobIndex->level/2 * pMobIndex->level / 8,
				pMobIndex->level/2 * pMobIndex->level / 2 );
		}
		else
			mob->points_max[POINTS_LIFE] = dice(pMobIndex->hitnodice, pMobIndex->hitsizedice) + pMobIndex->hitplus;

		mob->points_max[POINTS_LIFE] = UMAX(
			URANGE(mob->points_max[POINTS_LIFE]/4, (mob->points_max[POINTS_LIFE] * corpse->corpse->tick) / 100, get_level(ch)/2 * dice(20,10)),
			1 );

		mob->points[POINTS_LIFE] = mob->points_max[POINTS_LIFE];
		mob->damroll	  = ch->level / 16;
		mob->hitroll	  = ch->level / 12;
		mob->alignment	  = ch->alignment;

		act( AT_MAGIC, "$n richiama $T dal letto di morte!", ch, NULL, pMobIndex->short_descr, TO_ROOM );
		act( AT_MAGIC, "Richiamo $T dal letto di morte!", ch, NULL, pMobIndex->short_descr, TO_CHAR );

		sprintf( buf, "animated corpse %s", pMobIndex->name );
		DISPOSE( mob->name );
		mob->name = str_dup( buf );

		sprintf( buf, "Il corpo rianimato di %s", pMobIndex->short_descr );
		DISPOSE( mob->short_descr );
		mob->short_descr = str_dup( buf );

		sprintf( buf, "Leggi l'orrore negli occhi di questa creatura che ha visto la morte.. è il corpo di %s.\r\n", pMobIndex->short_descr );
		DISPOSE( mob->long_descr );
		mob->long_descr = str_dup( buf );
		add_follower( mob, ch );

		CREATE( aff, AFFECT_DATA, 1 );
		aff->type		 = sn;
		aff->duration  = ( number_fuzzy((level+1)/4) + 1 ) * 69;
		aff->location  = 0;
		aff->modifier  = 0;
		aff->bitvector = meb(AFFECT_CHARM);
		affect_to_char( mob, aff );

		if ( corpse->first_content )
		{
			for ( obj = corpse->first_content;  obj;  obj = obj_next )
			{
				obj_next = obj->next_content;
				obj_from_obj( obj );
				obj_to_room( obj, corpse->in_room );
			}
		}
		split_obj( corpse, 1 );
		free_object( corpse );
		return rNONE;
	}
	else
	{
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}
}


/*
 * Incantesimo di Possess.
 */
SPELL_RET spell_possess( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim;
	AFFECT_DATA *aff;
	SKILL_DATA	*skill = get_skilltype(sn);
	int			 attempt;

	if ( !ch->desc || ch->desc->original )
	{
		send_to_char( ch, "Devo essere nel mio stato originario.\r\n" );
		return rSPELL_FAILED;
	}

	victim = get_char_room( ch, target_name, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non è qui.\r\n" );
		return rSPELL_FAILED;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Possedermi?!\r\n" );
		return rSPELL_FAILED;
	}

	if ( IS_PG(victim) )
	{
		send_to_char( ch, "Non posso possedere un avventuriero!\r\n" );
		return rSPELL_FAILED;
	}

	if ( victim->desc )
	{
		ch_printf( ch, "%s è già sotto il possesso di qualcuno.\r\n", victim->short_descr );
		return rSPELL_FAILED;
	}

	if ( HAS_BIT(victim->immune, RIS_MAGIC)
	  || HAS_BIT(victim->immune, RIS_CHARM) )
	{
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	attempt = ris_save( victim, level, RIS_CHARM );

	if ( HAS_BIT(victim->affected_by, AFFECT_POSSESS)
	  || HAS_BIT(ch->affected_by, AFFECT_CHARM)
	  || level*2 < get_level(victim)
	  || victim->desc
	  || compute_savthrow(attempt, victim, SAVETHROW_SPELL_STAFF)
	  || !chance(ch, 25) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		 = sn;
	aff->duration  = ( ((get_level(ch)/2 - get_level(victim)/2) / 2 ) * 3 ) + 20;
	aff->location  = 0;
	aff->modifier  = 0;
	aff->bitvector = meb( AFFECT_POSSESS );
	affect_to_char( victim, aff );

	ch_printf( ch, "Riesco a entrare in possesso di %s!\r\n", victim->short_descr );

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc		= ch->desc;
	ch->desc			= NULL;
	ch->switched		= victim;

	return rNONE;
}


/*
 * Incantesimo di Knock. Ignores pickproofs, but can't unlock containers.
 */
SPELL_RET spell_knock( int sn, int level, CHAR_DATA *ch, void *vo )
{
	EXIT_DATA *pexit;
	SKILL_DATA *skill = get_skilltype(sn);

	set_char_color(AT_MAGIC, ch);

	/* Shouldn't know why it didn't work, and shouldn't work on pickproof exits. */
	if ( !(pexit = find_door(ch, target_name, FALSE))
		|| !HAS_BIT(pexit->flags, EXIT_CLOSED)
		|| !HAS_BIT(pexit->flags, EXIT_LOCKED)
		|| HAS_BIT(pexit->flags, EXIT_PICKPROOF) )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}

	REMOVE_BIT(pexit->flags, EXIT_LOCKED);
	send_to_char( ch, "*Click*\r\n" );

	if ( pexit->rexit && pexit->rexit->to_room == ch->in_room )
		REMOVE_BIT( pexit->rexit->flags, EXIT_LOCKED );

	if ( pexit->vdir >= DIR_NORTH && pexit->vdir < DIR_SOMEWHERE )
		check_room_for_traps( ch, TRAPFLAG_UNLOCK | table_dir[pexit->vdir].trap_door );
	return rNONE;
}


/*
 * Incantesimo di Dream. Una specie di CHANNEL di comunicazione agli addormentati dell'area.
 */
SPELL_RET spell_dream( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim;
	char arg[MIL];

	target_name = one_argument( target_name, arg );
	set_char_color( AT_MAGIC, ch );

	victim = get_char_area( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non sento la sua presenza qui nei d'intorni.\r\n" );
		return rSPELL_FAILED;
	}

	if ( victim->position != POSITION_SLEEP )
	{
		send_to_char( ch, "Non sta dormendo.\r\n" );
		return rSPELL_FAILED;
	}

	if ( !target_name )
	{
		send_to_char( ch, "Quale messaggio dovrei inviare nel suo sonno?\r\n" );
		return rSPELL_FAILED;
	}

	set_char_color( AT_TELL, victim );
	ch_printf( victim, "La voce di %s ti giunge nel sogno comunicandoti '%s'.\r\n", PERS(ch, victim), target_name );
	successful_casting( get_skilltype(sn), ch, victim, NULL );
/*	send_to_char( ch, "Ok.\r\n" ); */
	return rNONE;
}


/*
 * Incantesimo di Spiral Blast. Dell'Augurer.
 */
SPELL_RET spell_spiral_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int		  dam;
	int		  hpch;
	bool	  ch_died;

	ch_died = FALSE;

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "Non puoi farlo qui.\r\n" );
		return rNONE;
	}

	for ( vch = ch->in_room->first_person;  vch;  vch = vch_next )
	{
		vch_next = vch->next_in_room;

		if ( IS_ADMIN(vch) )								continue;

		if ( IS_MOB(ch) ? IS_PG(vch) : IS_MOB(vch) )
		{
			act( AT_MAGIC, "$n irradia una spirale di gas colorati che trapassa $N.",
				ch, ch, vch,  TO_ROOM );
			act( AT_MAGIC, "Richiami una spirale di gas colorati che trapassa $N!",
				ch, ch, vch , TO_CHAR );

			hpch = UMAX( 10, ch->points[POINTS_LIFE] );
			dam  = number_range( hpch/14+1, hpch/7 );

			if ( compute_savthrow(level, vch, SAVETHROW_BREATH) )
				dam /= 2;

			if ( damage(ch, vch, dam, sn) == rCHAR_DIED || char_died(ch) )
				ch_died = TRUE;
		}
	}

	if ( ch_died )
		return rCHAR_DIED;

	return rNONE;
}


/*
 * Incantesimo di Scorching Surge. Dell'Augurer.
 */
SPELL_RET spell_scorching_surge( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const int dam_each[] =
	{
		  1,
		  2,   3,   7,   9,  11,  13,  15,  17,  19,  21,
		 19,  22,  25,  28,  30,  35,  40,  45,  50,  55,
		 60,  65,  70,  75,  80,  82,  84,  86,  88,  90,
		 92,  94,  96,  98, 100, 102, 104, 106, 108, 110,
		112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
		132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
		152, 154, 156, 158, 160, 162, 164, 166, 168, 170
	};
	int dam;

	level = UMIN( level, (int)(sizeof(dam_each)/sizeof(dam_each[0])) - 1 );
	level = UMAX( 0, level );

	dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	act( AT_MAGIC, "Una lancinante sferza di dolore attraversa il corpo di $n!", ch, NULL, NULL, TO_ROOM );
	act( AT_MAGIC, "Una lancinante sferza di dolore mi attraversa il corpo!", ch, NULL, NULL, TO_CHAR );

	return damage( ch, victim, (dam*1.4), sn );
}


/*
 * Incantesimo di Helical Flow. Dell'Augurer.
 */
SPELL_RET spell_helical_flow( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim;
	SKILL_DATA *skill = get_skilltype(sn);

	victim = get_char_area( ch, target_name, TRUE );
	if ( !victim
	  ||  victim == ch
	  || !victim->in_room
	  || HAS_BIT(victim->in_room->flags, ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_DEATH)
	  || HAS_BIT(ch->in_room->flags,	  ROOM_NORECALL)
	  || get_level(victim) >= level*2 + 15
	  || !can_pkill(ch, victim)
	  || compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	act( AT_MAGIC, "Il corpo di $n sfuma in lente spirali di colore e svanisce in una colonna di luce!",
		ch, NULL, NULL, TO_ROOM );

	char_from_room( ch );
	char_to_room( ch, victim->in_room );

	act( AT_MAGIC, "Una colonna di luce e spirali colorate scivola giù dal cielo.. svelando la presenza di $n!",
		ch, NULL, NULL, TO_ROOM );

	send_command( ch, "look", CO );
	return rNONE;
}



	/************************************************************************
	 *		Tutto ciò dopo questo punto fa parte dell'incantesimo SMAUG		*
	 ************************************************************************/

/*
 * Saving throw check
 */
bool check_save( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim )
{
	SKILL_DATA *skill = get_skilltype(sn);
	bool	   saved = FALSE;

	if ( SPELL_FLAG(skill, SKELL_PKSENSITIVE) && IS_PG(ch) && IS_PG(victim) )
		level /= 2;

	if ( skill->saves )
		saved = compute_savthrow( level, victim, skill->saves );

	return saved;
}


ch_ret spell_affect_char( int sn, int level, CHAR_DATA *ch, void *vo )
{
	AFFECT_DATA	*aff;
	SMAUG_AFF	*saf;
	SKILL_DATA	*skill	= get_skilltype(sn);
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	int			affect_chance;
	ch_ret		retcode = rNONE;

	if ( SPELL_FLAG(skill, SKELL_RECASTABLE) )
		affect_strip( victim, sn );

	for ( saf = skill->first_affect;  saf;  saf = saf->next )
	{
		if ( saf->location >= REVERSE_APPLY )
			victim = ch;
		else
			victim = (CHAR_DATA *) vo;

		CREATE( aff, AFFECT_DATA, 1 );
		/* Check if char has this bitvector already */
		aff->bitvector = meb( saf->bitvector );

		if ( saf->bitvector >= 0
		  && HAS_BIT(victim->affected_by, saf->bitvector)
		  && !SPELL_FLAG(skill, SKELL_ACCUMULATIVE) )
		{
			continue;
		}

		/* necessary for affect_strip to work properly... */
		switch ( saf->bitvector )
		{
		  default:
			aff->type = sn;
			break;

		  case AFFECT_POISON:
			aff->type = gsn_poison;
			affect_chance = ris_save( victim, level, RIS_POISON );

			if ( affect_chance == 1000 )
			{
				retcode = rVICT_IMMUNE;
				if ( SPELL_FLAG(skill, SKELL_STOPONFAIL) )
					return retcode;
				continue;
			}

			if ( compute_savthrow(affect_chance, victim, SAVETHROW_POISON_DEATH) )
			{
				if ( SPELL_FLAG(skill, SKELL_STOPONFAIL) )
					return retcode;
				continue;
			}

			victim->mental_state = URANGE( 30, victim->mental_state + 2, 100 );
			break;

		  case AFFECT_BLIND:		aff->type = gsn_blindness;	break;
		  case AFFECT_CURSE:		aff->type = gsn_curse;		break;
		  case AFFECT_INVISIBLE:	aff->type = gsn_invis;		break;

		  case AFFECT_SLEEP:
			aff->type = gsn_sleep;
			affect_chance = ris_save( victim, level, RIS_SLEEP );
			if ( affect_chance == 1000 )
			{
				retcode = rVICT_IMMUNE;
				if ( SPELL_FLAG(skill, SKELL_STOPONFAIL) )
					return retcode;
				continue;
			}
			break;

		  case AFFECT_CHARM:
			aff->type = gsn_charm_person;
			affect_chance = ris_save( victim, level, RIS_CHARM );
			if ( affect_chance == 1000 )
			{
				retcode = rVICT_IMMUNE;

				if ( SPELL_FLAG(skill, SKELL_STOPONFAIL) )
					return retcode;
				continue;
			}
			break;

		  case AFFECT_POSSESS:
			aff->type = gsn_possess;
			break;
		}

		aff->duration = dice_parse( ch, level, saf->duration ) * 3;
		aff->modifier = dice_parse( ch, level, saf->modifier );
		aff->location = saf->location % REVERSE_APPLY;

		if ( aff->duration == 0 )
		{
			int xp_gain;

			switch ( aff->location )
			{
			  case APPLY_LIFE:
				victim->points[POINTS_LIFE] = URANGE( 0, victim->points[POINTS_LIFE] + aff->modifier, victim->points_max[POINTS_LIFE] );
				update_position( victim );

				if ( (aff->modifier > 0 && ch->fighting && ch->fighting->who == victim)
				  || (aff->modifier > 0 && victim->fighting && victim->fighting->who == ch) )
				{
					int xp = ch->fighting  ?  ch->fighting->xp  :  victim->fighting->xp;

					xp_gain = (int) (xp * aff->modifier*2) / victim->points_max[POINTS_LIFE];
					gain_exp( ch, 0 - xp_gain, FALSE );
				}

				if ( IS_MOB(victim) && victim->points[POINTS_LIFE] <= 0 )
					damage( ch, victim, 5, TYPE_UNDEFINED );
				break;

			  case APPLY_MANA:
				victim->points[POINTS_MANA] = URANGE( 0, victim->points[POINTS_MANA] + aff->modifier, victim->points_max[POINTS_MANA] );
				update_position( victim );
				break;

			  case APPLY_MOVE:
				victim->points[POINTS_MOVE] = URANGE( 0, victim->points[POINTS_MOVE] + aff->modifier, victim->points_max[POINTS_MOVE] );
				update_position( victim );
				break;

			  case APPLY_SOUL:
				victim->points[POINTS_SOUL] = URANGE( 0, victim->points[POINTS_SOUL] + aff->modifier, victim->points_max[POINTS_SOUL] );
				update_position( victim );
				break;

			  default:
				affect_modify( victim, aff, TRUE );
				break;
			}
		}
		else if ( SPELL_FLAG( skill, SKELL_ACCUMULATIVE ) )
			affect_join( victim, aff );
		else
			affect_to_char( victim, aff );
	} /* chiude il for */

	update_position( victim );
	return retcode;
}


/*
 * Is immune to a damage type
 */
bool is_immune( CHAR_DATA *ch, int damtype )
{
	switch ( damtype )
	{
	  default:
/*		send_log( NULL, LOG_BUG, "is_immune: tipologia di danno SD non dichiarato: %d", damtype ); (TT) Per ora disattivo che fa spam, quando ho tempo la rivedo */
		break;

	  case SPELLDAMAGE_FIRE:				if ( HAS_BIT(ch->immune, RIS_FIRE		) )		return TRUE;	break;
	  case SPELLDAMAGE_COLD:				if ( HAS_BIT(ch->immune, RIS_COLD		) )		return TRUE;	break;
	  case SPELLDAMAGE_ELECTRICITY:		if ( HAS_BIT(ch->immune, RIS_ELECTRICITY	) )		return TRUE;	break;
	  case SPELLDAMAGE_ENERGY:			if ( HAS_BIT(ch->immune, RIS_ENERGY		) )		return TRUE;	break;
	  case SPELLDAMAGE_ACID:				if ( HAS_BIT(ch->immune, RIS_ACID		) )		return TRUE;	break;
	  case SPELLDAMAGE_POISON:			if ( HAS_BIT(ch->immune, RIS_POISON		) )		return TRUE;	break;
	  case SPELLDAMAGE_DRAIN:			if ( HAS_BIT(ch->immune, RIS_DRAIN		) )		return TRUE;	break;
	}

	return FALSE;
}


/*
 * Generic spell affect
 */
ch_ret spell_affect( int sn, int level, CHAR_DATA *ch, void *vo )
{
	SMAUG_AFF	*saf;
	SKILL_DATA	*skill = get_skilltype(sn);
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	bool		 groupsp;
	bool		 areasp;
	bool		 hitchar = FALSE;
	bool		 hitroom = FALSE;
	bool		 hitvict = FALSE;
	ch_ret		 retcode;

	if ( !skill->first_affect )
	{
		send_log( NULL, LOG_BUG, "spell_affect: nessun affects al sn %d(%s) per il pg %s",
			sn, table_skill[sn]->name, ch->name );
		return rNONE;
	}

	if ( SPELL_FLAG(skill, SKELL_GROUPSPELL) )
		groupsp = TRUE;
	else
		groupsp = FALSE;

	if ( SPELL_FLAG(skill, SKELL_AREA) )
		areasp = TRUE;
	else
		areasp = FALSE;

	if ( !groupsp && !areasp )
	{
		/* Can't find a victim */
		if ( !victim )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		if ( (skill->type != SKILLTYPE_HERB
		  && HAS_BIT(victim->immune, RIS_MAGIC))
		  || is_immune(victim, SPELL_DAMAGE(skill)) )
		{
			immune_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		/* Spell is already on this guy */
		if ( is_affected(victim, sn)
		  && !SPELL_FLAG(skill, SKELL_ACCUMULATIVE)
		  && !SPELL_FLAG(skill, SKELL_RECASTABLE) )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		saf = skill->first_affect;
		if ( saf && !saf->next
		  && saf->location == APPLY_STRIPSN
		  && !is_affected(victim, dice_parse(ch, level, saf->modifier)) )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}

		if ( check_save(sn, level, ch, victim) )
		{
			failed_casting( skill, ch, victim, NULL );
			return rSPELL_FAILED;
		}
	}
	else
	{
		if ( VALID_STR(skill->hit_char) )
		{
			if ( strstr(skill->hit_char, "$N") )
				hitchar = TRUE;
			else
				act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
		}

		if ( VALID_STR(skill->hit_room) )
		{
			if ( strstr(skill->hit_room, "$N") )
				hitroom = TRUE;
			else
				act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );
		}

		if ( VALID_STR(skill->hit_vict) )
			hitvict = TRUE;

		if ( victim )
			victim = victim->in_room->first_person;
		else
			victim = ch->in_room->first_person;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "spell_affect: could not find victim: sn %d", sn );
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	for ( ;  victim;  victim = victim->next_in_room )
	{
		if ( groupsp || areasp )
		{
			if ( (groupsp && !is_same_group(victim, ch))
			  || HAS_BIT(victim->immune, RIS_MAGIC)
			  || is_immune(victim, SPELL_DAMAGE(skill))
			  || check_save(sn, level, ch, victim)
			  || (!SPELL_FLAG(skill, SKELL_RECASTABLE) && is_affected(victim, sn)) )
			{
				continue;
			}

			if ( hitvict && ch != victim )
			{
				act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );

				if ( hitroom )
				{
					act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOVICT );
					act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_CHAR );
				}
			}
			else if ( hitroom )
				act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );

			if ( ch == victim )
			{
				if ( hitvict )
					act( AT_MAGIC, skill->hit_vict, ch, NULL, ch, TO_CHAR );
				else if ( hitchar )
					act( AT_MAGIC, skill->hit_char, ch, NULL, ch, TO_CHAR );
			}
			else if ( hitchar )
				act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );

		} /* chiude l'if */

		retcode = spell_affect_char( sn, level, ch, victim );

		if ( !groupsp && !areasp )
		{
			if ( retcode == rVICT_IMMUNE )
				immune_casting( skill, ch, victim, NULL );
			else
				successful_casting( skill, ch, victim, NULL );
			break;
		}
	} /* chiude il for */
	return rNONE;
}


/*
 * Generic offensive spell damage attack
 */
ch_ret spell_attack( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	SKILL_DATA *skill = get_skilltype(sn);
	bool	   saved = check_save( sn, level, ch, victim );
	int		   dam;
	ch_ret	   retcode = rNONE;

	if ( saved && SPELL_SAVE(skill) == SPELLSAVE_NEGATE )
	{
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	}

	if ( skill->dice )
		dam = UMAX( 0, dice_parse(ch, level, skill->dice) );
	else
		dam = dice( 1, level/2 );

	if ( saved )
	{
		switch ( SPELL_SAVE(skill) )
		{
		  case SPELLSAVE_3QTRDAM:		dam   = (dam * 3) / 4;	break;
		  case SPELLSAVE_HALFDAM:		dam >>= 1;				break;
		  case SPELLSAVE_QUARTERDAM:	dam >>= 2;				break;
		  case SPELLSAVE_EIGHTHDAM:	dam >>= 3;				break;

		  case SPELLSAVE_ABSORB:			/* victim absorbs spell for hp's */
			act( AT_MAGIC, "$N mi assorbe $t!", ch,	  skill->noun_damage, victim, TO_CHAR	 );
			act( AT_MAGIC, "Assorbo a $N $t!", victim, skill->noun_damage, ch,	  TO_CHAR	 );
			act( AT_MAGIC, "$N assorbe a $n $t!", ch,	  skill->noun_damage, victim, TO_NOVICT );
			victim->points[POINTS_LIFE] = URANGE( 0, victim->points[POINTS_LIFE] + dam, victim->points_max[POINTS_LIFE] );
			update_position( victim );

			if ( (dam > 0 && ch->fighting && ch->fighting->who == victim)
				|| (dam > 0 && victim->fighting && victim->fighting->who == ch) )
			{
				int xp = ch->fighting  ?  ch->fighting->xp  :  victim->fighting->xp;
				int xp_gain = (int) (xp * dam * 2) / victim->points_max[POINTS_LIFE];

				gain_exp( ch, 0 - xp_gain, FALSE );
			}

			if ( skill->first_affect )
				retcode = spell_affect_char( sn, level, ch, victim );

			return retcode;

		  case SPELLSAVE_REFLECT:			/* reflect the spell to the caster */
			return spell_attack( sn, level, victim, ch );
		}
	}
	retcode = damage( ch, victim, dam, sn );

	if ( retcode == rNONE && skill->first_affect
	  && !char_died(ch) && !char_died(victim)
	  && (!is_affected(victim, sn)
	  || SPELL_FLAG(skill, SKELL_ACCUMULATIVE)
	  || SPELL_FLAG(skill, SKELL_RECASTABLE)) )
	{
		retcode = spell_affect_char( sn, level, ch, victim );
	}
	return retcode;
}


/*
 * Generic area attack
 */
ch_ret spell_area_attack( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA  *vch;
	CHAR_DATA  *vch_next;
	SKILL_DATA *skill = get_skilltype(sn);
	int			dam;
	bool		saved;
	bool		affects;
	bool		ch_died = FALSE;
	ch_ret		retcode = rNONE;

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	}

	affects = (skill->first_affect ? TRUE : FALSE);

	if ( VALID_STR(skill->hit_char) )
		act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );

	if ( VALID_STR(skill->hit_room) )
		act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );

	for ( vch = ch->in_room->first_person;  vch;  vch = vch_next )
	{
		vch_next = vch->next_in_room;

		if ( IS_ADMIN(vch) )								continue;
		if ( vch == ch )									continue;
		if ( is_safe( ch, vch, FALSE) )						continue;

		saved = check_save( sn, level, ch, vch );

		if ( saved && SPELL_SAVE(skill) == SPELLSAVE_NEGATE )
		{
			failed_casting( skill, ch, vch, NULL );
			continue;
		}
		else if ( skill->dice )
		{
			dam = dice_parse( ch, level, skill->dice );
		}
		else
		{
			dam = dice( 1, level/2 );
		}

		if ( saved )
		{
			switch ( SPELL_SAVE(skill) )
			{
			  case SPELLSAVE_3QTRDAM:		dam   = (dam * 3) / 4;	break;
			  case SPELLSAVE_HALFDAM:		dam >>= 1;				break;
			  case SPELLSAVE_QUARTERDAM:	dam >>= 2;				break;
			  case SPELLSAVE_EIGHTHDAM:	dam >>= 3;				break;

			  case SPELLSAVE_ABSORB:			/* victim absorbs spell for hp's */
				act( AT_MAGIC, "$N mi assorbe $t!", ch,	skill->noun_damage, vch, TO_CHAR	);
				act( AT_MAGIC, "Assorbo $N's $t!", vch,	skill->noun_damage, ch,  TO_CHAR	);
				act( AT_MAGIC, "$N assorbe $n's $t!", ch,	skill->noun_damage, vch, TO_NOVICT );
				vch->points[POINTS_LIFE] = URANGE( 0, vch->points[POINTS_LIFE] + dam, vch->points_max[POINTS_LIFE] );
				update_position( vch );

				if ( (dam > 0 && ch->fighting && ch->fighting->who == vch)
					|| (dam > 0 && vch->fighting && vch->fighting->who == ch) )
				{
					int xp = ch->fighting
						? ch->fighting->xp
						: vch->fighting->xp;

					int xp_gain = (int) (xp * dam * 2) / vch->points_max[POINTS_LIFE];
					gain_exp( ch, 0 - xp_gain, FALSE );
				}
				continue;

			  case SPELLSAVE_REFLECT:		/* reflect the spell to the caster */
				retcode = spell_attack( sn, level, vch, ch );
				if ( char_died(ch) )
				{
					ch_died = TRUE;
					break;
				}
				continue;
			}
		}

		retcode = damage( ch, vch, dam, sn );

		if ( retcode == rNONE && affects && !char_died(ch) && !char_died(vch)
		  && (!is_affected(vch, sn)
		  || SPELL_FLAG(skill, SKELL_ACCUMULATIVE)
		  || SPELL_FLAG(skill, SKELL_RECASTABLE)) )
		{
			retcode = spell_affect_char( sn, level, ch, vch );
		}

		if ( retcode == rCHAR_DIED || char_died(ch) )
		{
			ch_died = TRUE;
			break;
		}
	} /* chiude il for */
	return retcode;
}


/*
 * Generic inventory object spell
 */
ch_ret spell_obj_inv( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA   *obj = (OBJ_DATA *) vo;
	SKILL_DATA  *skill = get_skilltype(sn);

	if ( !obj )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rNONE;
	}

	switch ( SPELL_ACTION(skill) )
	{
	  default:
	  case SPELLACTION_NONE:
		return rNONE;

	  case SPELLACTION_CREATE:
		if ( SPELL_FLAG(skill, SKELL_WATER) )	/* create water */
		{
			int water;
			WEATHER_DATA *weath = ch->in_room->area->weather;

			if ( obj->type != OBJTYPE_DRINK_CON )
			{
				send_to_char( ch, "Non è un contenitore adatto all'acqua.\r\n" );
				return rSPELL_FAILED;
			}

			if ( obj->drinkcon->liquid != LIQ_WATER && obj->drinkcon->curr_quant != 0 )
			{
				send_to_char( ch, "Vi è già dell'altro liquido, devo svuotarlo prima.\r\n" );
				return rSPELL_FAILED;
			}

			water = UMIN( (skill->dice ? dice_parse(ch, level, skill->dice) : level)
				* (weath->precip >= 0 ? 2 : 1),
				obj->drinkcon->capacity - obj->drinkcon->curr_quant );

			if ( water > 0 )
			{
				split_obj( obj, 1 );
				obj->drinkcon->liquid  = LIQ_WATER;
				obj->drinkcon->curr_quant += water;

				if ( !is_name( "water", obj->name ) )
				{
					char buf[MSL];

					sprintf( buf, "%s water", obj->name );
					DISPOSE( obj->name );
					obj->name = str_dup( buf );
				}
			}
			successful_casting( skill, ch, NULL, obj );
			return rNONE;
		} /* chiude l'if */

		if ( SPELL_DAMAGE(skill) == SPELLDAMAGE_FIRE )	/* burn object */
        {
/*			return rNONE; */		/* (??) da guardare meglio a tempo debito */
        }

		if ( SPELL_DAMAGE(skill) == SPELLDAMAGE_POISON	/* poison object */
		  || SPELL_CLASS(skill)  == SPELLSPHERE_DEATH )
		{
			switch ( obj->type )
			{
			  default:
				failed_casting( skill, ch, NULL, obj );
				break;
			  case OBJTYPE_COOK:
			  case OBJTYPE_FOOD:
			  case OBJTYPE_DRINK_CON:
				split_obj( obj, 1 );
				obj->food->poison = 1;
				successful_casting( skill, ch, NULL, obj );
				break;
			}
			return rNONE;
		}

		if ( SPELL_CLASS(skill) == SPELLSPHERE_LIFE	/* purify food/water */
		  && (obj->type == OBJTYPE_FOOD
		  ||  obj->type == OBJTYPE_DRINK_CON
		  ||  obj->type == OBJTYPE_COOK) )
		{
			switch ( obj->type )
			{
			  default:
				failed_casting( skill, ch, NULL, obj );
				break;
			  case OBJTYPE_COOK:
			  case OBJTYPE_FOOD:
			  case OBJTYPE_DRINK_CON:
				split_obj( obj, 1 );
				obj->food->poison = 0;
				successful_casting( skill, ch, NULL, obj );
				break;
			}
			return rNONE;
		}

		if ( SPELL_CLASS(skill) != SPELLSPHERE_NONE )
		{
			failed_casting( skill, ch, NULL, obj );
			return rNONE;
		}

		switch ( SPELL_POWER(skill) )	/* clone object */
		{
			OBJ_DATA *clone;

		  default:
		  case SPELLPOWER_NONE:
			if ( get_level(ch)/2 - obj->level/2 < 10
			  || obj->cost > get_level(ch)/2 * get_curr_attr(ch, ATTR_COC)/4 )	/* Concentrazione perché per farlo al meglio deve dare il massimo, e più dà il meglio e più vale */
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			break;

		  case SPELLPOWER_MINOR:
			if ( get_level(ch)/2 - obj->level/2 < 20
			  || obj->cost > get_level(ch)/2 * get_curr_attr(ch, ATTR_COC)/20 )
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			break;

		  case SPELLPOWER_GREATER:
			if ( get_level(ch)/2 - obj->level/2 < 5
			  || obj->cost > get_level(ch)/2 * 10 * get_curr_attr(ch, ATTR_COC)/4 )
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			break;

	  case SPELLPOWER_MAJOR:
			if ( get_level(ch)/2 - obj->level/2 < 0
			  || obj->cost > get_level(ch)/2 * 50 * get_curr_attr(ch, ATTR_COC)/4 )
			{
				failed_casting( skill, ch, NULL, obj );
				return rNONE;
			}
			break;

			clone = clone_object(obj);
			clone->timer = skill->dice  ?  dice_parse(ch, level, skill->dice)  :  0;
			obj_to_char( clone, ch );
			successful_casting( skill, ch, NULL, obj );
		} /* chiude lo switch */

		return rNONE;

	  case SPELLACTION_DESTROY:
	  case SPELLACTION_RESIST:
	  case SPELLACTION_SUSCEPT:
	  case SPELLACTION_DIVINATE:
		if ( SPELL_DAMAGE(skill) == SPELLDAMAGE_POISON )		/* detect poison */
		{
			if ( obj->type == OBJTYPE_DRINK_CON
			  || obj->type == OBJTYPE_FOOD
			  || obj->type == OBJTYPE_COOK)
			{
				if ( obj->type == OBJTYPE_COOK && obj->food->dam_cook == 0 )
					send_to_char( ch, "Qualche altro minuto al fuoco non guasterebbe..\r\n" );
				else if ( obj->food->poison != 0 )
					send_to_char( ch, "Avverto la presenza di tracce di veleno.\r\n" );
				else
					send_to_char( ch, "Perfettamente commestibile!\r\n" );
			}
			else
				send_to_char( ch, "Non rilevo alcuna traccia di veleno.\r\n" );
			return rNONE;
		}
		return rNONE;

	  case SPELLACTION_OBSCURE:			/* make obj invis */
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVIS)
			|| chance(ch, skill->dice  ?  dice_parse(ch, level, skill->dice)  :  20))
		{
			failed_casting( skill, ch, NULL, NULL );
			return rSPELL_FAILED;
		}
		successful_casting( skill, ch, NULL, obj );
		SET_BIT( obj->extra_flags, OBJFLAG_INVIS );
		return rNONE;

	  case SPELLACTION_CHANGE:
		return rNONE;
	}

	return rNONE;
}


/*
 * Generic object creating spell
 */
ch_ret spell_create_obj( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA	   *obj;
	OBJ_PROTO_DATA *oi;
	SKILL_DATA	   *skill = get_skilltype( sn );
	int			    lvl;
	VNUM		    vnum = skill->value;

	switch ( SPELL_POWER(skill) )
	{
	  default:
	  case SPELLPOWER_NONE:		lvl = 10;		break;
	  case SPELLPOWER_MINOR:	lvl = 0;		break;
	  case SPELLPOWER_GREATER:	lvl = level/2;	break;
	  case SPELLPOWER_MAJOR:	lvl = level;	break;
	}

	/*
	 * Add predetermined objects here
	 */
	if ( vnum == 0 )
	{
		if ( !str_cmp(target_name, "spada") || !str_cmp(target_name, "sword") )
			vnum = VNUM_OBJ_SCHOOL_SWORD;

		if ( !str_cmp(target_name, "scudo") || !str_cmp(target_name, "shield") )
			vnum = VNUM_OBJ_SCHOOL_SHIELD;
	}

	if ( (oi = get_obj_proto(NULL, vnum)) == NULL
	  || (obj = make_object(oi, lvl)) == NULL )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rNONE;
	}

	obj->timer = skill->dice  ?  dice_parse( ch, level, skill->dice )  :  0;

	successful_casting( skill, ch, NULL, obj );

	/* Particolarità per il globo di luce */
	if ( vnum == VNUM_OBJ_LIGHT_BALL )
		obj->light->intensity = UMAX( LVL_NEWBIE, get_level(ch)/2 );

	if ( HAS_BIT( obj->wear_flags, OBJWEAR_TAKE) )
		obj_to_char( obj, ch );
	else
		obj_to_room( obj, ch->in_room );

	return rNONE;
}


/*
 * Generic mob creating spell
 */
ch_ret spell_create_mob( int sn, int level, CHAR_DATA *ch, void *vo )
{
	SKILL_DATA	   *skill = get_skilltype(sn);
	CHAR_DATA	   *mob;
	MOB_PROTO_DATA *mi;
	AFFECT_DATA	   *aff;
	int			    lvl;
	VNUM		    vnum = skill->value;

	/* set maximum mob level */
	switch ( SPELL_POWER(skill) )
	{
	  default:
	  case SPELLPOWER_NONE:		lvl = 20;		break;
	  case SPELLPOWER_MINOR:	lvl = 5;		break;
	  case SPELLPOWER_GREATER:	lvl = level/2;	break;
	  case SPELLPOWER_MAJOR:	lvl = level;	break;
	}

	/*
	 * Add predetermined mobiles here
	 */
	if ( vnum == 0 )
	{
		if ( !str_cmp(target_name, "guardia") || !str_cmp(target_name, "cityguard") )
			vnum = VNUM_MOB_CITYGUARD;

		if ( !str_cmp(target_name, "vampiro") || !str_cmp(target_name, "vampire") )
			vnum = VNUM_MOB_VAMPIRE;
	}

	/* Controlla se il pg ha raggiunto il massimo possibile di seguaci charmati */
	if ( IS_PG(ch) && !IS_ADMIN(ch) && !can_charm(ch) )
	{
		send_to_char( ch, "Ho già il massimo dei seguaci che mi è possibile controllare.\r\n" );
		return rERROR;
	}

	if ( (mi = get_mob_index(NULL, vnum)) == NULL
	  || (mob = make_mobile(mi)) == NULL )
	{
		failed_casting( skill, ch, NULL, NULL );
		return rNONE;
	}

	mob->level   = UMIN( lvl, skill->dice  ?  dice_parse(ch, level, skill->dice)  :  get_level(mob) );
	mob->armor	 = interpolate( get_level(mob)/2, 100, -100 );
	mob->points_max[POINTS_LIFE] = get_level(mob) * 4 +
		number_range( get_level(mob)/2 * get_level(mob)/8, get_level(mob)/2 * get_level(mob)/2 );
	mob->points[POINTS_LIFE] = mob->points_max[POINTS_LIFE];
	mob->gold	 = 0;

	successful_casting( skill, ch, mob, NULL );
	char_to_room( mob, ch->in_room );
	add_follower( mob, ch );

	CREATE( aff, AFFECT_DATA, 1 );
	aff->type		= sn;
	aff->duration	= ( number_fuzzy((level+1) / 6) + 1 ) * 69;
	aff->location	= 0;
	aff->modifier	= 0;
	aff->bitvector	= meb( AFFECT_CHARM );
	affect_to_char( mob, aff );

	return rNONE;
}


/*
 * Generic handler for new "SMAUG" spells
 */
SPELL_RET spell_smaug( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA	*victim;
	SKILL_DATA	*skill = get_skilltype( sn );

	/* Put this check in to prevent crashes from this getting a bad skill */
	if ( !skill )
	{
		send_log( NULL, LOG_BUG, "spell_smaug: called with a null skill for sn %d", sn );
		return rERROR;
	}

	switch ( skill->target )
	{
	  case TARGET_IGNORE:
		/* offensive area spell */
		if ( SPELL_FLAG(skill, SKELL_AREA)
		  && ((SPELL_ACTION(skill) == SPELLACTION_DESTROY && SPELL_CLASS(skill) == SPELLSPHERE_LIFE)
		  ||  (SPELL_ACTION(skill) == SPELLACTION_CREATE	 && SPELL_CLASS(skill) == SPELLSPHERE_DEATH)) )
		{
			return spell_area_attack( sn, level, ch, vo );
		}

		/* Crea un oggetto */
		if ( SPELL_ACTION(skill) == SPELLACTION_CREATE )
		{
			if ( SPELL_FLAG(skill, SKELL_OBJECT) )	/* create object */
				return spell_create_obj( sn, level, ch,  vo );

			if ( SPELL_CLASS(skill) == SPELLSPHERE_LIFE )	/* create mob */
				return spell_create_mob( sn, level, ch,  vo );
		}

		/* affect a distant player */
		victim = get_char_area( ch, target_name, TRUE );
		if ( SPELL_FLAG(skill, SKELL_DISTANT)
		  && victim
		  && !HAS_BIT( victim->in_room->flags, ROOM_NOASTRAL)
		  && SPELL_FLAG(skill, SKELL_CHARACTER) )
		{
			return spell_affect( sn, level, ch, get_char_area(ch, target_name, TRUE) );
		}

		/* affect a player in this room (should have been TARGET_CHAR_XXX) */
		if ( SPELL_FLAG(skill, SKELL_CHARACTER) )
			return spell_affect(sn, level, ch, get_char_room(ch, target_name, TRUE) );

		/* Attacco a distanza */
		if ( skill->range > 0
		  && ((SPELL_ACTION(skill) == SPELLACTION_DESTROY  && SPELL_CLASS(skill) == SPELLSPHERE_LIFE)
		  ||  (SPELL_ACTION(skill) == SPELLACTION_CREATE	  && SPELL_CLASS(skill) == SPELLSPHERE_DEATH)) )
		{
			return ranged_attack( ch, target_name_ranged, NULL, NULL, sn, skill->range );
		}

		/* will fail, or be an area/group affect */
		return spell_affect( sn, level, ch, vo );

	  case TARGET_CHAR_OFFENSIVE:
		/* a regular damage inflicting spell attack */
		if ( (SPELL_ACTION(skill) == SPELLACTION_DESTROY	&& SPELL_CLASS(skill) == SPELLSPHERE_LIFE )
		  || (SPELL_ACTION(skill) == SPELLACTION_CREATE	&& SPELL_CLASS(skill) == SPELLSPHERE_DEATH) )
		{
			return spell_attack( sn, level, ch, vo );
		}
		/* a nasty spell affect */
		return spell_affect( sn, level, ch, vo );

	  case TARGET_CHAR_DEFENSIVE:
	  case TARGET_CHAR_SELF:
		if ( SPELL_FLAG(skill, SKELL_NOFIGHT )
		  && (ch->position == POSITION_FIGHT
		  ||  ch->position == POSITION_EVASIVE
		  ||  ch->position == POSITION_DEFENSIVE
		  ||  ch->position == POSITION_AGGRESSIVE
		  ||  ch->position == POSITION_BERSERK) )
		{
			send_to_char( ch, "Non riesco a concentrarmi abbastanza per farlo!\r\n" );
			return rNONE;
		}

		if ( vo && SPELL_ACTION(skill) == SPELLACTION_DESTROY )
		{
			victim = (CHAR_DATA *) vo;

			/* cure poison */
			if ( SPELL_DAMAGE(skill) == SPELLDAMAGE_POISON )
			{
				if ( is_affected(victim, gsn_poison) )
				{
					affect_strip( victim, gsn_poison );
					victim->mental_state = URANGE( -100, victim->mental_state, -10 );
					successful_casting( skill, ch, victim, NULL );
					return rNONE;
				}
				failed_casting( skill, ch, victim, NULL );
				return rSPELL_FAILED;
			}

			/* cure blindness */
			if ( SPELL_CLASS(skill) == SPELLSPHERE_ILLUSION )
			{
				if ( is_affected(victim, gsn_blindness) )
				{
					affect_strip( victim, gsn_blindness );
					successful_casting( skill, ch, victim, NULL );
					return rNONE;
				}
				failed_casting( skill, ch, victim, NULL );
				return rSPELL_FAILED;
			}
		}
		return spell_affect( sn, level, ch, vo );

	  case TARGET_OBJ_INV:
		return spell_obj_inv( sn, level, ch, vo );
	}
	return rNONE;
}


/* Haus' new, new mage spells follow */
/*
 * 4 Energy Spells
 */
SPELL_RET spell_ethereal_fist( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 35, level );
	dam		= 1.3 * ( level * number_range(1, 6) ) - 31;
	dam		= UMAX( 0,dam );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam/=4;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_spectral_furor( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	level	= UMAX( 0, level );
	level	= UMIN( 16, level );
	dam		= 1.3 * ( level * number_range(1, 7) + 7 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_hand_of_chaos( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 18, level );
	dam		= 1.3 * ( level * number_range(1, 7) + 9 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam/=4;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_disruption( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 14, level );
	dam		= 1.3 * ( level * number_range(1, 6) + 8 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam = 0;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_sonic_resonance( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 23, level );
	dam		= 1.3 * ( level * number_range(1, 8) );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam = dam * 3/4;

	return damage( ch, victim, dam, sn );
}


/*
 * 3 Mentalstate spells
 */
SPELL_RET spell_mind_wrack( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	/* decrement mentalstate by up to 50 */
	level	= UMAX( 0, level );
	dam		= number_range( 0, 0 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_mind_wrench( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	/* increment mentalstate by up to 50 */
	level	= UMAX( 0, level );
	dam		= number_range( 0, 0 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Non-offensive spell!
 */
SPELL_RET spell_revive( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	/* set mentalstate to mentalstate/2 */
	level	= UMAX( 0, level );
	dam		= number_range( 0, 0 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * n Acid Spells
 */
SPELL_RET spell_sulfurous_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 19, level );
	dam		= 1.3 * ( 2*level*number_range(1, 7) + 11 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 4;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_caustic_fount( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 42, level );
	dam		= 1.3 * ( 2*level*number_range(1, 6) ) - 31;
	dam		= UMAX(0,dam);

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam = dam * 1/2;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_acetum_primus( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	dam		= 1.3 * ( 2*level*number_range(1, 4) + 7 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam = dam * 3/4;

	return damage( ch, victim, dam, sn );
}


/*
 *  Electrical
 */
SPELL_RET spell_galvanic_whip( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 10, level );
	dam		= 1.3 * ( level*number_range(1, 6) + 5 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_magnetic_thrust( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level  );
	level	= UMIN( 29, level );
	dam		= 0.65 * ( (5*level*number_range(1, 6)) + 16 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 3;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_quantum_spike( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam,
			   l;

	level	= UMAX( 0, level );
	l		= 2 * ( level/10 );
	dam		= 1.3 * ( l*number_range(1,40) + 145 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 2;

	return damage( ch, victim, dam, sn );
}


/*
 * Black-magicish guys
 */

/* L2 Mage Spell */
SPELL_RET spell_black_hand( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 5, level );
	dam		= 1.3 * ( level*number_range(1, 6) + 3 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 4;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_black_fist( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam;

	level	= UMAX( 0, level );
	level	= UMIN( 30, level );
	dam		= 1.3 * ( level*number_range(1, 9) + 4 );

	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 4;

	return damage( ch, victim, dam, sn );
}


/*
 * (FF) questo andrebbe bene per l'incantesimo frusta d'ombra di Tane
 */
SPELL_RET spell_black_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int		   dam,
			   l;

	level	= UMAX( 0, level );
	l		= 2 * level/10;
	dam		= 1.3 * ( l*number_range(1,50) + 135 );


	if ( compute_savthrow(level, victim, SAVETHROW_SPELL_STAFF) )
		dam /= 4;

	return damage( ch, victim, dam, sn );
}


SPELL_RET spell_midas_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
	int		   val;
	OBJ_DATA  *obj = (OBJ_DATA *) vo;

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_NODROP) )
	{
		send_to_char( ch, "Non riesco a liberarmene!\r\n" );
		return rSPELL_FAILED;
	}

	if ( !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE)
	  || (obj->type == OBJTYPE_CORPSE_MOB)
	  || (obj->type == OBJTYPE_CORPSE_PG) )
	{
		send_to_char( ch, "Non mi sembra di essere in grado di trasformare tale oggetto in oro!\r\n" );
		return rNONE;
	}

	split_obj( obj, 1 );

	val = obj->cost / 2;
	val = UMAX( 0, val );

	ch->gold += val;

	if ( obj_extracted(obj) )
		return rNONE;
	if ( cur_obj == obj->serial )
		global_objcode = rOBJ_SACCED;

	free_object( obj );
	send_to_char( ch, "Agendo sulla struttura molecolare riesco a trasmutare in oro l'oggetto!\r\n" );

	return rNONE;
}
