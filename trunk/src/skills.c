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
 >							Player skills module						    <
\****************************************************************************/

#include <ctype.h>
#include <dlfcn.h>		/* libdl */
#include <limits.h>

#include "mud.h"
#include "affect.h"
#include "build.h"
#include "calendar.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "editor.h"
#include "fread.h"
#include "gramm.h"
#include "group.h"
#include "herb.h"
#include "information.h"
#include "interpret.h"
#include "magic.h"
#include "movement.h"
#include "msp.h"
#include "mxp.h"
#include "morph.h"
#include "room.h"
#include "save.h"
#include "timer.h"


/*
 * Variabili globali
 */
int				 top_sn;
SKILL_DATA		*table_skill[MAX_SKILL];
#ifdef T2_ALFA
SKILL_DATA		*table_disease [MAX_DISEASE];
#endif


const CODE_DATA code_skilltype[] =
{
	{ SKILLTYPE_UNKNOWN,	"SKILLTYPE_UNKNOWN",	"Unknown"	},
	{ SKILLTYPE_SPELL,		"SKILLTYPE_SPELL",		"Spell"		},
	{ SKILLTYPE_SKILL,		"SKILLTYPE_SKILL",		"Skill"		},
	{ SKILLTYPE_WEAPON,		"SKILLTYPE_WEAPON",		"Weapon"	},
	{ SKILLTYPE_TONGUE,		"SKILLTYPE_TONGUE",		"Tongue"	},	/* (CC) che in realtà non esiste più */
	{ SKILLTYPE_HERB,		"SKILLTYPE_HERB",		"Herb"		},
	{ SKILLTYPE_RACIAL,		"SKILLTYPE_RACIAL",		"Racial"	},
	{ SKILLTYPE_DISEASE,	"SKILLTYPE_DISEASE",	"Disease"	}
};
const int max_check_skilltype = sizeof(code_skilltype)/sizeof(CODE_DATA);


/*
 * Tabella di codici di intenzione
 */
const CODE_DATA code_intent[] =
{
	{ INTENT_NEUTRAL,		"INTENT_NEUTRAL",		"neutral"	 },
	{ INTENT_FRIENDLY,		"INTENT_FRIENDLY",		"friedly"	 },
	{ INTENT_AGGRESSIVE,	"INTENT_AGGRESSIVE",	"aggressive" }
};
const int max_check_intent = sizeof(code_intent)/sizeof(CODE_DATA);


/*
 * Tabella di codici sulle flag di skill e spell
 */
const CODE_DATA code_skell[] =
{
	{ SKELL_WATER,			"SKELL_WATER",			"water"			},
	{ SKELL_EARTH,			"SKELL_EARTH",			"earth"			},
	{ SKELL_AIR,			"SKELL_AIR",			"air"			},
	{ SKELL_ASTRAL,			"SKELL_ASTRAL",			"astral"		},
	{ SKELL_AREA,			"SKELL_AREA",			"area"			},
	{ SKELL_DISTANT,		"SKELL_DISTANT",		"distant"		},
	{ SKELL_REVERSE,		"SKELL_REVERSE",		"reverse"		},
	{ SKELL_NOSELF,			"SKELL_NOSELF",			"noself"		},
	{ SKELL_UNUSED2,		"SKELL_UNUSED2",		"_unused2_"		},
	{ SKELL_ACCUMULATIVE,	"SKELL_ACCUMULATIVE",	"accumulative"	},
	{ SKELL_RECASTABLE,		"SKELL_RECASTABLE",		"recastable"	},
	{ SKELL_NOSCRIBE,		"SKELL_NOSCRIBE",		"noscribe"		},
	{ SKELL_NOBREW,			"SKELL_NOBREW",			"nobrew"		},
	{ SKELL_GROUPSPELL,		"SKELL_GROUPSPELL",		"group"			},
	{ SKELL_OBJECT,			"SKELL_OBJECT",			"object"		},
	{ SKELL_CHARACTER,		"SKELL_CHARACTER",		"character"		},
	{ SKELL_SECRETSKILL,	"SKELL_SECRETSKILL",	"secretskill"	},
	{ SKELL_PKSENSITIVE,	"SKELL_PKSENSITIVE",	"pksensitive"	},
	{ SKELL_STOPONFAIL,		"SKELL_STOPONFAIL",		"stoponfail"	},
	{ SKELL_NOFIGHT,		"SKELL_NOFIGHT",		"nofight"		},
	{ SKELL_NODISPEL,		"SKELL_NODISPEL",		"nodispel"		},
	{ SKELL_RANDOMTARGET,	"SKELL_RANDOMTARGET",	"randomtarget"	},
	{ SKELL_SILENT,			"SKELL_SILENT",			"silent"		}
};
const int max_check_skell = sizeof(code_skell)/sizeof(CODE_DATA);


/*
------------------------------------------------------------------------------
 * DLSym Snippet.
 * Written by Trax of Forever's End
------------------------------------------------------------------------------
 */
DO_FUN *skill_function( char *name )
{
   void		  *funHandle;
   const char *error;

	funHandle = dlsym( sysdata.dlHandle, name );
	if ( (error = dlerror()) != NULL )
	{
		send_log( NULL, LOG_BUG, "Error locating %s in symbol table. %s", name, error );
		return skill_notfound;
	}

	return (DO_FUN *) funHandle;
}


/*
 * Funzioni relative al caricamento e lettura delle abilità
 */
void fread_skill( MUD_FILE *fp, const char *section )
{
	SKILL_DATA	*skill;
	char		*word;
	int			 x;
	bool		 got_info = FALSE;

	CREATE( skill, SKILL_DATA, 1 );
	skill->intention = INTENT_NONE;		/* Per weapon e skills, gli incantesimi devono averlo */
	skill->slot = 0;
	skill->min_mana = 0;

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		skill->skill_level[x] = LVL_LEGEND;
		skill->skill_adept[x] = 95;
	}

	for ( x = 0;  x < MAX_RACE;  x++ )
		skill->race_mod[x] = 0;

	skill->target = 0;
	skill->skill_fun = NULL;
	skill->spell_fun = NULL;
	CLEAR_BITS( skill->spell_sector );

	for ( ; ; )
	{
		word = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'		  )									fread_to_eol( fp );
		else if ( !str_cmp(word, "Affect") )
		{
			SMAUG_AFF *saf;

			CREATE( saf, SMAUG_AFF, 1 );
			saf->duration =													str_dup( fread_word(fp) );
			saf->location =													fread_number( fp );
			saf->modifier =													str_dup( fread_word(fp) );
			saf->bitvector =												fread_number( fp );

			/* Bisogna controllare che qui il mod non sia 0, altrimenti nella affect_modify() viene applicato l'AFFECT_BLIND */
			if ( saf->modifier == 0 )
				send_log( fp, LOG_BUG, "fread_skill: modifier 0 alla skill %s", skill->name );

			if ( !got_info )
			{
				for ( x = 0;  x < 32;  x++ )
				{
					if ( HAS_BITINT(saf->bitvector, x) )
					{
						saf->bitvector = x;
						break;
					}
				}
				if ( x == 32 )
					saf->bitvector = -1;
			}
			LINK( saf, skill->first_affect, skill->last_affect, next, prev );
		}
		else if ( !str_cmp(word, "FunName") )
		{
			DO_FUN		*dofun;
			SPELL_FUN	*spellfun;

			skill->fun_name = str_dup( fread_word(fp) );
			if		( !str_prefix("do_", skill->fun_name) && (dofun = skill_function(skill->fun_name)) != skill_notfound )
			{
				skill->skill_fun = dofun;
				skill->spell_fun = NULL;
			}
			else if ( str_prefix("do_", skill->fun_name) && (spellfun = spell_function(skill->fun_name)) != spell_notfound )
			{
				skill->spell_fun = spellfun;
				skill->skill_fun = NULL;
			}
			else
			{
				send_log( NULL, LOG_FREAD, "fread_skill: unknown skill/spell %s", skill->fun_name );
				skill->spell_fun = spell_null;
			}
		}
		else if ( !str_cmp(word, "Components"  ) )	skill->components =		fread_string( fp );
		else if ( !str_cmp(word, "DamMsg"	   ) )	skill->noun_damage =	fread_string( fp );
		else if ( !str_cmp(word, "Dice"		   ) )	skill->dice =			fread_string( fp );
		else if ( !str_cmp(word, "DieArea"	   ) )	skill->die_area =		fread_string( fp );	/* (GG) e anche TO_AREA */
		else if ( !str_cmp(word, "DieAround"   ) )	skill->die_around =		fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "DieChar"	   ) )	skill->die_char =		fread_string( fp );
		else if ( !str_cmp(word, "DieRoom"	   ) )	skill->die_room =		fread_string( fp );
		else if ( !str_cmp(word, "DieVict"	   ) )	skill->die_vict =		fread_string( fp );
		else if ( !str_cmp(word, "Difficulty"  ) )	skill->difficulty =		fread_number( fp );	/* (RR) (SK) */
		else if ( !str_cmp(word, "End"		   ) )	break;
		else if ( !str_cmp(word, "Flags"	   ) )	/* (FF) non posso cambiare le flags di skill in extended per colpa di questa parte, troppo ostica per me, prima convertirla liscia e poi cambiare il tutto */
		{
			skill->flags = fread_number( fp );

			/* Convert to new style */
			if ( !got_info )
			{
				skill->info = skill->flags & ((1<<11) - 1);
				if ( HAS_BITINT(skill->flags, OLD_SF_SAVE_NEGATES) )
				{
					if ( HAS_BITINT(skill->flags, OLD_SF_SAVE_HALF_DAMAGE) )
					{
						SET_SSAV( skill, SPELLSAVE_QUARTERDAM );
						REMOVE_BITINT( skill->flags, OLD_SF_SAVE_HALF_DAMAGE );
					}
					else
						SET_SSAV( skill, SPELLSAVE_NEGATE );

					REMOVE_BITINT( skill->flags, OLD_SF_SAVE_NEGATES );	/* (BB) */
				}
				else if ( HAS_BITINT(skill->flags, OLD_SF_SAVE_HALF_DAMAGE) )
				{
					SET_SSAV( skill, SPELLSAVE_HALFDAM );
					REMOVE_BITINT( skill->flags, OLD_SF_SAVE_HALF_DAMAGE );
				}
				skill->flags >>= 11;
			}
		}
		else if ( !str_cmp(word, "HitArea"	   ) )	skill->hit_area =		fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "HitAround"   ) )	skill->hit_around =		fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "HitChar"	   ) )	skill->hit_char =		fread_string( fp );
		else if ( !str_cmp(word, "HitDest"	   ) )	skill->hit_dest =		fread_string( fp );
		else if ( !str_cmp(word, "HitRoom"	   ) )	skill->hit_room =		fread_string( fp );
		else if ( !str_cmp(word, "HitVict"	   ) )	skill->hit_vict =		fread_string( fp );
		else if ( !str_cmp(word, "MastArea"	   ) )	skill->mast_area =		fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "MastAround"  ) )	skill->mast_around =	fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "MastChar"	   ) )	skill->mast_char =		fread_string( fp );
		else if ( !str_cmp(word, "MastRoom"	   ) )	skill->mast_room =		fread_string( fp );
		else if ( !str_cmp(word, "MastVict"	   ) )	skill->mast_vict =		fread_string( fp );
		else if ( !str_cmp(word, "ImmArea"	   ) )	skill->imm_area =		fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "ImmAround"   ) )	skill->imm_around =		fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "ImmChar"	   ) )	skill->imm_char =		fread_string( fp );
		else if ( !str_cmp(word, "ImmRoom"	   ) )	skill->imm_room =		fread_string( fp );
		else if ( !str_cmp(word, "ImmVict"	   ) )	skill->imm_vict =		fread_string( fp );
		else if ( !str_cmp(word, "Info"		   ) )
		{
			skill->info =													fread_number( fp );
			got_info = TRUE;
		}
		else if ( !str_cmp(word, "Intention"  ) )	skill->intention =		code_code( fp, fread_word(fp), CODE_INTENT );
		else if ( !str_cmp(word, "Mana"		  ) )	skill->min_mana	=		fread_number( fp );
		else if ( !str_cmp(word, "MinPosition") )	skill->min_position =	code_code( fp, fread_word(fp), CODE_POSITION );
		else if ( !str_cmp(word, "MissArea"	   ) )	skill->miss_area =		fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "MissAround"  ) )	skill->miss_around =	fread_string( fp );	/* (GG) */
		else if ( !str_cmp(word, "MissChar"	   ) )	skill->miss_char =		fread_string( fp );
		else if ( !str_cmp(word, "MissRoom"	   ) )	skill->miss_room =		fread_string( fp );
		else if ( !str_cmp(word, "MissVict"	   ) )	skill->miss_vict =		fread_string( fp );
		else if ( !str_cmp(word, "Name"		   ) )	skill->name =			fread_string( fp );
		else if ( !str_cmp(word, "Participants") )	skill->participants	=	fread_number( fp );
#ifdef T2_SKILL
		else if ( !str_cmp(word, "Preskill1"   ) )	skill->pre_skill1 =		fread_string( fp );
		else if ( !str_cmp(word, "Preskill2"   ) )	skill->pre_skill1 =		fread_string( fp );
		else if ( !str_cmp(word, "Preskill3"   ) )	skill->pre_skill1 =		fread_string( fp );
#endif
		else if ( !str_cmp(word, "RaceMod"	   ) )	skill->race_mod[fread_number(fp)] = fread_number( fp );	/* (GG) */
		else if ( !str_cmp(word, "Range"	   ) )	skill->range =			fread_number( fp );
		else if ( !str_cmp(word, "Rounds"	   ) )	skill->beats =			fread_number( fp );
		else if ( !str_cmp(word, "Saves"	   ) )	skill->saves =			fread_number( fp );
		else if ( !str_cmp(word, "Slot"		   ) )	skill->slot =			fread_number( fp );
		else if ( !str_cmp(word, "SpellSector" ) )	skill->spell_sector =	fread_old_bitvector( fp, CODE_SECTOR );
		else if ( !str_cmp(word, "Target"	   ) )	skill->target =			fread_number( fp );
		else if ( !str_cmp(word, "Teachers"	   ) )	skill->teachers =		fread_string( fp );
		else if ( !str_cmp(word, "Type"		   ) )	skill->type =			get_skill( fread_word(fp) );
		else if ( !str_cmp(word, "Value"	   ) )	skill->value =			fread_number( fp );
		else if ( !str_cmp(word, "WearOff"	   ) )	skill->msg_off =		fread_string( fp );
		else
			send_log( fp, LOG_FREAD, "fread_skill: nessuna etichetta trovata per la parola %s", word );
	}

	/*
	 * Controlla la skill
	 */

	if ( skill->saves != 0 && SPELL_SAVE(skill) == SPELLSAVE_NONE )
	{
		send_log( fp, LOG_FREAD, "fread_skill(%s):  Has saving throw (%d) with no saving effect.",
			skill->name, skill->saves );
		SET_SSAV( skill, SPELLSAVE_NEGATE );
	}

	if ( skill->min_position < 0 || skill->min_position >= MAX_POSITION )
	{
		send_log( fp, LOG_BUG, "fread_skill: posizione minima della skill errata: %d", skill->min_position );
		exit( EXIT_FAILURE );
	}

	if ( !str_cmp(section, "SKILL") || !str_cmp(section, "SPELL") || !str_cmp(section, "WEAPON") )
	{
		if ( top_sn >= MAX_SKILL )
		{
			send_log( fp, LOG_FREAD, "fread_skill: numero skill al limite di MAX_SKILL %d", MAX_SKILL );
			exit( EXIT_FAILURE );
		}

		table_skill[top_sn] = skill;
		top_sn++;

#ifdef T2_CNV_SMAUG
		if		( skill->type == SKILL_UNKNOWN )
			send_log( fp, LOG_FREAD, "fread_skill: tipo di skill 0: %s", skill->name );
		else if ( skill->type == SKILLTYPE_SPELL )
			convert_skill_from_smaug( "spells.cnv", skill );
		else if ( skill->type == SKILL_SKILL )
			convert_skill_from_smaug( "skills.cnvt", skill );
		else if ( skill->type == SKILL_WEAPON )
			convert_skill_from_smaug( "weapons.cnvt", skill );
		else if ( skill->type == SKILL_HERB )
			send_log( fp, LOG_FREAD, "fread_skill: tipo di erba in sezione errata %s", skil->name );
		else if ( skill->type == SKILL_RACIAL )
			convert_skill_from_smaug( "racial.cnv", skill );
		else if ( skill->type == SKILL_DISEASE )
			convert_skill_from_smaug( "disease.cnv", skill );
#endif
	}
	else if ( !str_cmp(section, "HERB") )
	{
		if ( top_herb >= MAX_HERB )
		{
			send_log( fp, LOG_FREAD, "fread_skill: numero erbe al limite di MAX_HERB %d", MAX_HERB );
			exit( EXIT_FAILURE );
		}
		table_herb[top_herb] = skill;
		top_herb++;
#ifdef T2_CNV_SMAUG
		convert_skill_from_smaug( "herb_cnv.dat", skill );
#endif
	}
	else
	{
		send_log( NULL, LOG_FREAD, "fread_skill: tipo di sezione passato alla lettura skill errata: %s", section );
		exit( EXIT_FAILURE );
	}
}


/*
 * Libera dalla memoria una skill
 */
void free_skill( SKILL_DATA *skill )
{
	SMAUG_AFF *saf;

	if ( !skill )
	{
		send_log( NULL, LOG_BUG, "free_skill: skill passata è NULL" );
		return;
	}

	DISPOSE( skill->name );
	DISPOSE( skill->fun_name );
	DISPOSE( skill->noun_damage );
	DISPOSE( skill->msg_off );
	DISPOSE( skill->hit_area );
	DISPOSE( skill->hit_around );
	DISPOSE( skill->hit_char );
	DISPOSE( skill->hit_vict );
	DISPOSE( skill->hit_room );
	DISPOSE( skill->hit_dest );
	DISPOSE( skill->mast_area );
	DISPOSE( skill->mast_around );
	DISPOSE( skill->mast_char );
	DISPOSE( skill->mast_vict );
	DISPOSE( skill->mast_room );
	DISPOSE( skill->mast_dest );
	DISPOSE( skill->miss_area );
	DISPOSE( skill->miss_around );
	DISPOSE( skill->miss_char );
	DISPOSE( skill->miss_vict );
	DISPOSE( skill->miss_room );
	DISPOSE( skill->die_area );
	DISPOSE( skill->die_around );
	DISPOSE( skill->die_char );
	DISPOSE( skill->die_vict );
	DISPOSE( skill->die_room );
	DISPOSE( skill->imm_area );
	DISPOSE( skill->imm_around );
	DISPOSE( skill->imm_char );
	DISPOSE( skill->imm_vict );
	DISPOSE( skill->imm_room );
	DISPOSE( skill->dice );
	destroybv( skill->spell_sector );
#ifdef T2_SKILL
	DISPOSE( skill->preskill1 );
	DISPOSE( skill->preskill2 );
	DISPOSE( skill->preskill3 );
#endif
	for ( saf = skill->first_affect;  saf;  saf = skill->first_affect )
	{
		UNLINK( saf, skill->first_affect, skill->last_affect, next, prev );
		free_smaug_affect( saf );
	}
	DISPOSE( skill->components );
	DISPOSE( skill->teachers );
	DISPOSE( skill );
}


/*
 * Libera dalla memoria tutte le skill
 */
void free_all_skills( void )
{
	int sn;

	for ( sn = 0;  sn < MAX_SKILL;  sn++ )
	{
		if ( !table_skill[sn] )
			continue;

		free_skill( table_skill[sn] );
		top_sn--;
	}

	if ( top_sn != 0 )
		send_log( NULL, LOG_BUG, "free_all_skells: top_sn non è 0 dopo aver liberato le skill: %d", top_sn );
}


int get_skill( const char *skilltype )
{
	if ( !str_cmp( skilltype, "Race"  ) )	return SKILLTYPE_RACIAL;
	if ( !str_cmp( skilltype, "Spell" ) )	return SKILLTYPE_SPELL;
	if ( !str_cmp( skilltype, "Skill" ) )	return SKILLTYPE_SKILL;
	if ( !str_cmp( skilltype, "Weapon") )	return SKILLTYPE_WEAPON;
	if ( !str_cmp( skilltype, "Herb"  ) )	return SKILLTYPE_HERB;

	return SKILLTYPE_UNKNOWN;
}


/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILL_DATA **sk1, SKILL_DATA **sk2 )
{
	SKILL_DATA *skill_one = (*sk1);
	SKILL_DATA *skill_two = (*sk2);

	if ( !skill_one &&  skill_two )				return  1;
	if (  skill_one && !skill_two )				return -1;
	if ( !skill_one && !skill_two )				return  0;
	if (  skill_one->type < skill_two->type )	return -1;
	if (  skill_one->type > skill_two->type )	return  1;

	return strcmp( skill_one->name, skill_two->name );
}


/*
 * Sort the skill table with qsort
 */
void sort_skill_table( void )
{
	send_log( NULL, LOG_LOAD, "Sorting skill table..." );
	qsort( &table_skill[1], top_sn-1, sizeof(SKILL_DATA *), (int(*) (const void *, const void *)) skill_comp );
}


/*
 * Remap slot numbers to sn values
 */
void remap_slot_numbers( void )
{
	SKILL_DATA  *skill;
	SMAUG_AFF	*saf;
	char		 tmp[32];
	int			 sn;

	send_log( NULL, LOG_LOAD, "Remapping slots to sns" );

	for ( sn = 0;  sn <= top_sn;  sn++ )
	{
		if ( (skill = table_skill[sn]) == NULL )
			continue;

		for ( saf = skill->first_affect;  saf;  saf = saf->next )
		{
			if ( saf->location == APPLY_WEAPONSPELL
			  || saf->location == APPLY_WEARSPELL
			  || saf->location == APPLY_REMOVESPELL
			  || saf->location == APPLY_STRIPSN
			  || saf->location == APPLY_RECURRINGSPELL )
			{
				sprintf( tmp, "%d", slot_lookup(atoi(saf->modifier)) );
				DISPOSE( saf->modifier );
				saf->modifier = str_dup( tmp );
			}
		}
	}
}


void fread_sskill( MUD_FILE *fp )
{
	fread_skill( fp, "SKILL" );
}


/*
 * Carica e controlla tutte le skill
 */
void load_skills( void )
{
	fread_section( SKILL_FILE, "SKILL", fread_sskill, FALSE );

	/* (FF) da eseguire il controllo, come con il load_social, per cui le skill non debbano essere uguale di nome ai comandi */

	/* (FF) da fare l'ordine alfabetico */
}


#if T2_ALFA
/*
 * Lookup a skill by name, only stopping at skills the player has.
 * (FF) mai utilizzata.
 */
int ch_slookup( CHAR_DATA *ch, const char *name )
{
	int sn;

	if ( IS_MOB(ch) )
		return skill_lookup( name );

	for ( sn = 0;  sn < top_sn;  sn++ )
	{
		if ( !VALID_STR(table_skill[sn]->name) )
			break;

		if ( ch->pg->learned_skell[sn] <= 0 )							continue;
		if ( get_level(ch) < table_skill[sn]->skill_level[ch->class] )	continue;

		if ( !str_prefix(name, table_skill[sn]->name) )
			return sn;
	}

	return -1;
}

int find_weapon( CHAR_DATA *ch, const char *name, bool know )
{
	if ( IS_MOB(ch) || !know )
		return bsearch_skill( name, gsn_first_weapon, gsn_top_sn-1 );

	return ch_bsearch_skill( ch, name, gsn_first_weapon, gsn_top_sn-1 );
}
#endif


/*
 * Perform a binary search on a section of the skill table
 * Check for prefix matches
 */
int bsearch_skill_prefix( const char *name, int first, int top )
{
	int sn;

	for ( ; ; )
	{
		sn = (first + top) >> 1;

		if ( !is_valid_sn(sn) )
			return -1;

		if ( !str_prefix(name, table_skill[sn]->name) )
			return sn;

		if ( first >= top )
			return -1;

		if ( strcmp(name, table_skill[sn]->name) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}

	return -1;
}


/*
 * Perform a binary search on a section of the skill table
 * Check for exact matches only
 */
int bsearch_skill_exact( const char *name, int first, int top )
{
	int sn;

	for ( ; ; )
	{
		sn = (first + top) >> 1;

		if ( !is_valid_sn(sn) )
			return -1;

		if ( !str_cmp(name, table_skill[sn]->name) )
			return sn;

		if ( first >= top )
			return -1;

		if ( strcmp(name, table_skill[sn]->name) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}

	return -1;
}


/*
 * Perform a binary search on a section of the skill table
 * Check exact match first, then a prefix match
 */
int bsearch_skill( const char *name, int first, int top )
{
	int sn;

	sn = bsearch_skill_exact( name, first, top );

	if ( sn == -1 )
		return bsearch_skill_prefix( name, first, top );

	return sn;
}


/*
 * Perform a binary search on a section of the skill table
 * Only match skills player knows
 */
int ch_bsearch_skill_prefix( CHAR_DATA *ch, const char *name, int first, int top )
{
	int sn;

	for ( ; ; )
	{
		sn = (first + top) >> 1;

		if ( !str_prefix(name, table_skill[sn]->name)
		  && ch->pg->learned_skell[sn] > 0
		  && get_level(ch) >= table_skill[sn]->skill_level[ch->class] )
		{
			return sn;
		}

		if ( first >= top )
			return -1;

		if ( strcmp( name, table_skill[sn]->name) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}

	return -1;
}


int ch_bsearch_skill_exact( CHAR_DATA *ch, const char *name, int first, int top )
{
	int sn;

	for ( ; ; )
	{
		sn = (first + top) >> 1;

		if ( !str_cmp(name, table_skill[sn]->name) && ch->pg->learned_skell[sn] > 0
		  && get_level(ch) >= table_skill[sn]->skill_level[ch->class] )
			return sn;

		if ( first >= top )
			return -1;

		if ( strcmp(name, table_skill[sn]->name) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}

	return -1;
}


int ch_bsearch_skill( CHAR_DATA *ch, const char *name, int first, int top )
{
	int sn;

	sn = ch_bsearch_skill_exact(ch, name, first, top);

	if ( sn == -1 )
		return ch_bsearch_skill_prefix( ch, name, first, top );

	return sn;
}


/*
 * Lookup a skill by name
 */
int skill_lookup( const char *name )
{
	int sn;

	if ( !VALID_STR(name) )
	{
/*		send_log( NULL, LOG_BUG, "skill_lookup: name passato di skill non valido" );*/
		return -1;
	}

	/* Da considerare un pochinello nidificati */
	if ( (sn = bsearch_skill_exact	  (name, gsn_first_spell,  gsn_first_skill-1 )) == -1 )
	if ( (sn = bsearch_skill_exact	  (name, gsn_first_skill,  gsn_first_weapon-1)) == -1 )
	if ( (sn = bsearch_skill_exact	  (name, gsn_first_weapon, gsn_top_sn-1		 )) == -1 )
	if ( (sn = bsearch_skill_prefix	  (name, gsn_first_spell,  gsn_first_skill-1 )) == -1 )
	if ( (sn = bsearch_skill_prefix	  (name, gsn_first_skill,  gsn_first_weapon-1)) == -1 )
	if ( (sn = bsearch_skill_prefix	  (name, gsn_first_weapon, gsn_top_sn-1		 )) == -1
		&&	gsn_top_sn < top_sn )
	{
		for ( sn = gsn_top_sn;  sn < top_sn;  sn++ )
		{
			if ( !table_skill[sn] )
				return -1;

			if ( !VALID_STR(table_skill[sn]->name) )
				return -1;

			if ( !str_prefix(name, table_skill[sn]->name) )
				return sn;
		}
		return -1;
	}

	return sn;
}


/*
 * Return a skilltype pointer based on sn
 * Returns NULL if bad, unused or personal sn
 */
SKILL_DATA *get_skilltype( int sn )
{
	if ( sn >= TYPE_RACIAL )
		return NULL;

	if ( sn >= TYPE_HERB )
		return is_valid_herb(sn-TYPE_HERB)  ?  table_herb[sn-TYPE_HERB]  :  NULL;

	if ( sn >= TYPE_HIT )
		return NULL;

	if ( is_valid_sn(sn) )
		return table_skill[sn];

	return NULL;
}


/*
 * Lookup a skill by slot number
 * Used for object loading
 * (FF) in futuro è da togliere, quando tutti gli slot saranno convertiti in nomi di skill o spell
 */
int slot_lookup( int slot )
{
	int sn;

	if ( slot <= 0 )
		return -1;

	for ( sn = 0;  sn < top_sn;  sn++ )
	{
		if ( slot == table_skill[sn]->slot )
			return sn;
	}

	if ( fBootDb )
	{
		send_log( NULL, LOG_BUG, "slot_lookup: bad slot %d", slot );
		exit( EXIT_FAILURE );
	}

	return -1;
}


/*
 * Controlla se il sn passato è una skill.
 */
bool is_valid_sn( int sn )
{
	if ( sn < 0 )
		return FALSE;

	if ( sn >= MAX_SKILL )
		return FALSE;

	if ( !table_skill[sn] )
		return FALSE;

	if( !VALID_STR(table_skill[sn]->name) )
		return FALSE;

	return TRUE;
}


bool is_legal_kill( CHAR_DATA *ch, CHAR_DATA *vch )
{
	if ( IS_MOB(ch) )
		return TRUE;

	if ( IS_MOB(vch) )
		return TRUE;

	/* (FF) Se il pg si trova in un'area con flag di città non è legale */

	/* Se il pg fa parte dello stesso clan della vittima allora non è legale */
	if ( ch->pg->clan && ch->pg->clan == vch->pg->clan )
		return FALSE;

	return TRUE;
}


/*
 * New check to see if you can use skills to support morphs
 */
bool can_use_skill( CHAR_DATA *ch, int percent, int gsn )
{
	if ( IS_MOB(ch) && percent < 85 )
		return TRUE;

	if ( IS_PG(ch) && percent < knows_skell(ch, gsn) )
		return TRUE;

	if ( ch->morph && ch->morph->morph )
	{
		if ( VALID_STR(ch->morph->morph->skills)
		  && is_name(table_skill[gsn]->name, ch->morph->morph->skills)
		  && percent < 85 )
		{
			return TRUE;
		}

		if ( VALID_STR(ch->morph->morph->no_skills)
		  && is_name(table_skill[gsn]->name, ch->morph->morph->no_skills) )
		{
			return FALSE;
		}
	}

	return FALSE;
}


/*
 * Ritorna il massimo che il pg può imparare per quella skill/spell
 */
int get_adept_skell( CHAR_DATA *ch, int sn )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_adept_skell: ch è NULL" );
		return 75;
	}

	if ( sn < 0 || sn >= top_sn )
	{
		send_log( NULL, LOG_BUG, "get_adept_skell: sn errato: %d", sn );
		return 0;
	}

	return URANGE( 0, table_skill[sn]->skill_adept[ch->class], MAX_LEARN_SKELL );
}


/*
 * Ritorna ciò che il pg ha imparato per quella skill/spell
 */
int knows_skell( CHAR_DATA *ch, int sn )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "knows_skell: ch è NULL" );
		return 0;
	}

	if ( sn < 0 || sn >= top_sn )
	{
		send_log( NULL, LOG_BUG, "knows_skell: sn errato: %d", sn );
		return 0;
	}

	if ( IS_MOB(ch) )
		return number_range( 70, 85 );

	if ( ch->pg->learned_skell[sn] < 0 || ch->pg->learned_skell[sn] > MAX_LEARN_SKELL )
	{
		send_log( NULL, LOG_BUG, "knows_skell: valore appreso dal pg %s per la skill %s errato: %d",
			ch->name, table_skill[sn]->name, ch->pg->learned_skell[sn] );
	}

	return ( URANGE(0, ch->pg->learned_skell[sn], MAX_LEARN_SKELL) );
}


/*
 * Modify a skill (hopefully) properly
 *
 * On "adding" a skill modifying affect, the value set is unimportant
 * upon removing the affect, the skill it enforced to a proper range.
 */
void modify_skill( CHAR_DATA *ch, int sn, int mod, bool fAdd )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "modify_skill: ch è NULL" );
		return;
	}

	if ( sn < 0 || sn >= top_sn )
	{
		send_log( NULL, LOG_BUG, "modify_skill: sn errato: %d", sn );
		return;
	}

	/* (RR) aggiungere altri controlli sui valori passati */

	if ( IS_MOB(ch) )
		return;

	if ( fAdd )
		ch->pg->learned_skell[sn] += mod;
	else
		ch->pg->learned_skell[sn] = URANGE( 0, ch->pg->learned_skell[sn] + mod, get_adept_skell(ch, sn) );
}


/*
 * Si impara dai sucessi ottenuti o dai fallimenti.
 * bool success appunto indica se è un successo o fallimento
 * skell significa skill & spell, giochino di parole
 */
void learn_skell( CHAR_DATA *ch, int sn, bool success )
{
	int		adept;			/* Livello massimo con cui il pg imparerà la skill	*/
	int		gain;			/* Px guadagnati nell'apprendimento della skill		*/
	int		sklvl;			/* Livello della skill che il pg sta apprendendo	*/
	int		learn = 0;		/* Quanto guadagna di apprendimento					*/
	int		percent;		/* Percentuale casuale								*/
	int		learn_chance;	/* Valore di successo								*/

	if ( IS_MOB(ch) )									return;
	if ( number_range(0, 8+(get_level(ch)/10)) != 0 )	return;		/* Rallenta l'apprendimento */
	if ( ch->pg->learned_skell[sn] <= 1 )				return;

	/* Calcola l'adept */
	adept = get_adept_skell( ch, sn );
#ifdef T2_SKILL
	/* (FF) Diminuisco l'adept della percentuale dell'intelligenza, così il resto devono impararselo dai maestri */
	adept = adept * ch->attr[ATTR_INT] / 100
#endif

	/* Livello della skill */
	sklvl = table_skill[sn]->skill_level[ch->class];
	if ( sklvl == 0 )
		sklvl = get_level( ch );

	/* Se già arrivato all'adept esce */
	if ( ch->pg->learned_skell[sn] >= adept )
		return;

	/* più è alta succ_chance e meno imparano */
	learn_chance = ch->pg->learned_skell[sn] + ( table_skill[sn]->difficulty * 5 );
	percent = number_percent( );
	if ( percent >= learn_chance )
	{
		if ( success )
		{
			learn = number_range( 1, 2 );
			send_log( NULL, LOG_LEARN, "learn_skell: success high percent (1, 2)= %d per skell %s (%d%%) al pg %s",
				learn, table_skill[sn]->name, ch->pg->learned_skell[sn], ch->name );
		}
		else
		{
			learn = number_range( 0, 1 );
			send_log( NULL, LOG_LEARN, "learn_skell: failure high percent (0, 1)= %d per skell %s (%d%%) al pg %s",
				learn, table_skill[sn]->name, ch->pg->learned_skell[sn], ch->name );
		}
	}
	else if ( learn_chance - percent <= 25 )
	{
		if ( success )
		{
			learn = number_range( 0, 1 );
			send_log( NULL, LOG_LEARN, "learn_skell: success (0, 1)= %d per skell %s (%d%%) al pg %s",
				learn, table_skill[sn]->name, ch->pg->learned_skell[sn], ch->name );
		}
		else
		{
			learn = number_range( -1, 0 );
			/* Diminuisce la perdita di skell */
			if ( learn == -1 && number_range(0,1) == 1 )
				learn = 0;
			send_log( NULL, LOG_LEARN, "learn_skell: failure (-1, 0)= %d per skell %s (%d%%) al pg %s",
				learn, table_skill[sn]->name, ch->pg->learned_skell[sn], ch->name );
		}
	}
	else
	{
		return;
	}

	ch->pg->learned_skell[sn] = URANGE( 0, (ch->pg->learned_skell[sn] + learn), adept );

	/* Se si ha avuto successo nell'apprendimento si guadagna px */
	if ( success )
	{
		gain = sklvl/4 + 1;
		gain_exp( ch, gain, FALSE );
	}

	/* Se si è amministratori visualizza l'esperienza acquisita */
	if ( IS_ADMIN(ch) )
	{
		set_char_color( AT_ADMIN, ch );
		ch_printf( ch, "[ADM] Impari %d riguardo %s.\r\n", learn, table_skill[sn]->name );
	}
}


/*
 * Ritorna la skill se la trova tra i nomi di skill
 */
SKILL_DATA *find_skill( CHAR_DATA *ch, const char *command, bool exact, bool ita, const int first, const int top )
{
	int sn;

	if ( exact == TRUE || exact == ALL )
	{
		if ( ch && (ita == TRUE || ita == ALL) )
		{
			for ( sn = first;  sn < top;  sn++ )
			{
				if ( !table_skill[sn] )							continue;
				if ( !table_skill[sn]->skill_fun
				  && table_skill[sn]->spell_fun == spell_null )	continue;
				if ( !can_use_skill(ch, 0, sn) )				continue;

				if ( !str_cmp(command, table_skill[sn]->name) )
					return table_skill[sn];
			}
		}

		if ( ita == FALSE || ita == ALL )
		{
			for ( sn = first;  sn < top;  sn++ )
			{
				if ( !table_skill[sn] )							continue;
				if ( !table_skill[sn]->skill_fun
				  && table_skill[sn]->spell_fun == spell_null )	continue;
				if ( ch && !can_use_skill(ch, 0, sn) )			continue;

				if ( !str_cmp(command, table_skill[sn]->name) )	/* (FF) quando saranno tradotte le skill da cambiare */
					return table_skill[sn];
			}
		}
	}

	if ( ch && (exact == FALSE || exact == ALL) )
	{
		if ( ch && (ita == TRUE || ita == ALL) )
		{
			for ( sn = first;  sn < top;  sn++ )
			{
				if ( !table_skill[sn] )							continue;
				if ( !table_skill[sn]->skill_fun
				  && table_skill[sn]->spell_fun == spell_null )	continue;
				if ( !can_use_skill(ch, 0, sn) )				continue;

				if ( !str_prefix(command, table_skill[sn]->name) )
					return table_skill[sn];
			}
		}

		if ( ita == FALSE || ita == ALL )
		{
			for ( sn = first;  sn < top;  sn++ )
			{
				if ( !table_skill[sn] )							continue;
				if ( !table_skill[sn]->skill_fun
				  && table_skill[sn]->spell_fun == spell_null )	continue;
				if ( !can_use_skill(ch, 0, sn) )				continue;

				if ( !str_prefix(command, table_skill[sn]->name) )	/* (FF) quando saranno tradotte le skill da cambiare */
					return table_skill[sn];
			}
		}
	}

	return NULL;
}


/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows.
 */
bool check_skill( CHAR_DATA *ch, const char *command, char *argument, bool exact, bool ita )
{
	SKILL_DATA	  *skill;
	struct timeval time_used;
	int 		   mana;
	int			   blood;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_skill: ch è NULL" );
		return FALSE;
	}

	if ( !VALID_STR(command) )
	{
		send_log( NULL, LOG_BUG, "check_skill: argomento skill passato non è valido" );
		return FALSE;
	}

	skill = find_skill( ch, command, exact, ita, gsn_first_skill, gsn_first_weapon-1 );
	if ( !skill )
		return FALSE;

	if ( !check_position(ch, skill->min_position) )
		return TRUE;

	if ( IS_MOB(ch) &&  (HAS_BIT(ch->affected_by, AFFECT_CHARM) || HAS_BIT(ch->affected_by, AFFECT_POSSESS)) )
	{
		send_to_char( ch, "Un blocco mentale ti impedisce di farlo..\r\n" );
		act( AT_GREY,"$n strizza gli occhi nervosamente.", ch, NULL, NULL, TO_ROOM );
		return TRUE;
	}

	/* check if mana is required */
	if ( skill->min_mana )
	{
		if ( IS_MOB(ch) )
			mana = 0;
		else
			mana = UMAX( skill->min_mana, 100 / UMAX(1, (2 + get_level(ch) - skill->skill_level[ch->class])) );

		blood = UMAX(1, (mana+4) / 8);		/* NPCs don't have PCDatas. */

		/* Controlla che si abbiano abbastanza mana oppure si abbia la fiducia sufficiente
		 *	oppure si ci si trovi in un luogo ove poter castare in quantità senza perdere punti */
		if ( ch->points[POINTS_MANA] < mana && !stop_lowering_points(ch, POINTS_MANA) )
		{
			send_to_char( ch, "Non hai abbastanza mana.\r\n" );
			return TRUE;
		}
	}
	else
	{
		mana = 0;
		blood = 0;
	}

	/*
	 * Is this a real do-fun, or a really a spell?
	 */
	if ( !skill->skill_fun )
	{
		ch_ret	   retcode = rNONE;
		void	  *vo	   = NULL;
		CHAR_DATA *victim  = NULL;
		OBJ_DATA  *obj	   = NULL;

		target_name = "";

		switch ( skill->target )
		{
		  default:
			send_log( NULL, LOG_BUG, "check_skill: bad target for %s", skill->name );
			send_to_char( ch, "Qualcosa è andato storto..\r\n" );
			return TRUE;

		  case TARGET_IGNORE:
			vo = NULL;
			if ( !VALID_STR(argument) )
			{
				if ( (victim = who_fighting(ch)) != NULL )
					target_name = victim->name;
			}
			else
				target_name = argument;
			break;

		  case TARGET_CHAR_OFFENSIVE:
			if ( !VALID_STR(argument) && (victim = who_fighting(ch)) == NULL )
			{
				ch_printf( ch, "Che caos.. il tuo incantesimo di '%s' non ha bersaglio!\r\n", skill->name );
				return TRUE;
			}
			else if ( VALID_STR(argument) && (victim = get_char_room(ch, argument, TRUE)) == NULL )
			{
				send_to_char( ch, "Non è qui.\r\n" );
				return TRUE;
			}

			if ( is_safe(ch, victim, TRUE) )
				return TRUE;

			if ( ch == victim && SPELL_FLAG(skill, SKELL_NOSELF) )
			{
				send_to_char( ch, "Non puoi usare questo incantesimo su di te!\r\n" );
				return TRUE;
			}

			if ( IS_PG(ch) )
			{
				if ( IS_PG(victim) )
				{
					if ( get_timer(ch, TIMER_PKILLED) > 0 || get_timer(ch, TIMER_KILLED) > 0 )
					{
						send_to_char( ch, "La tua anima è tornata in questo corpo da troppo poco tempo.\r\n" );
						return TRUE;
					}

					if ( get_timer(victim, TIMER_PKILLED) > 0 || get_timer(victim, TIMER_KILLED) > 0 )
					{
						send_to_char( ch, "La sua anima è ritornata nel corpo da troppo poco tempo.\r\n" );
						return TRUE;
					}

					if ( victim != ch )
						send_to_char( ch, "Non è permesso fare una cosa del genere su di un altro avventuriero..\r\n" );
				} /* chiude l'if */

				if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) && ch->master == victim )
				{
					send_to_char( ch, "Non ti è permessa una cosa del genere su questo bersaglio.\r\n" );
					return TRUE;
				}
			} /* chiude l'if */

			is_illegal_pk( ch, victim );
			vo = (void *) victim;
			break;

		  case TARGET_CHAR_DEFENSIVE:
			if ( VALID_STR(argument) && (victim = get_char_room(ch, argument, TRUE)) == NULL )
			{
				send_to_char( ch, "Non è qui.\r\n" );
				return TRUE;
			}

			if ( !victim )
				victim = ch;

			if ( ch == victim && SPELL_FLAG(skill, SKELL_NOSELF) )
			{
				send_to_char( ch, "Non puoi usare questo incantesimo su di te!\r\n" );
				return TRUE;
			}

			vo = (void *) victim;
			break;

		  case TARGET_CHAR_SELF:
			vo = (void *) ch;
			break;

		  case TARGET_OBJ_INV:
			if ( (obj = get_obj_carry(ch, argument, TRUE)) == NULL )
			{
				send_to_char( ch, "Non hai nulla del genere.\r\n" );
				return TRUE;
			}

			vo = (void *) obj;
			break;

		} /* chiude lo switch */

		/* Waitstate */
		WAIT_STATE( ch, skill->beats * 3 );	/* (WT) */

		/* Check for failure */
		if ( (number_percent( ) + skill->difficulty * 5)
		  > (IS_MOB(ch)  ?  75  :  knows_skell(ch, skill_lookup(skill->name))) )
		{
			failed_casting( skill, ch, vo, obj );
if ( !str_cmp(skill->name, "aggressive style")
  || !str_cmp(skill->name, "berserk style")
  || !str_cmp(skill->name, "defensive style")
  || !str_cmp(skill->name, "evasive style")
  || !str_cmp(skill->name, "normal style") )
send_log( NULL, LOG_MONI, "avvertire onirik: stile digitato %s zona 1", skill->name );	/* (TT) */
			learn_skell( ch, skill_lookup(skill->name), FALSE );

			if ( mana != 0 )
				ch->points[POINTS_MANA] -= mana / 2;

			return TRUE;
		}

		if ( mana != 0 )
			ch->points[POINTS_MANA] -= mana;

		start_timer(&time_used);
		retcode = (*skill->spell_fun) ( skill_lookup(skill->name), get_level(ch)/2, ch, vo );
		end_timer(&time_used);
		update_userec( &time_used, &skill->use_count );

		if ( retcode == rCHAR_DIED || retcode == rERROR )
			return TRUE;

		if ( char_died(ch) )
			return TRUE;

		if ( retcode == rSPELL_FAILED )
		{
if ( !str_cmp(skill->name, "aggressive style")
  || !str_cmp(skill->name, "berserk style")
  || !str_cmp(skill->name, "defensive style")
  || !str_cmp(skill->name, "evasive style")
  || !str_cmp(skill->name, "normal style") )
send_log( NULL, LOG_MONI, "avvertire onirik: stile digitato %s zona 2", skill->name );	/* (TT) */
			learn_skell( ch, skill_lookup(skill->name), FALSE );
			retcode = rNONE;
		}
		else
		{
if ( !str_cmp(skill->name, "aggressive style")
  || !str_cmp(skill->name, "berserk style")
  || !str_cmp(skill->name, "defensive style")
  || !str_cmp(skill->name, "evasive style")
  || !str_cmp(skill->name, "normal style") )
send_log( NULL, LOG_MONI, "avvertire onirik: stile digitato %s zona 3", skill->name );	/* (TT) */
			learn_skell( ch, skill_lookup(skill->name), TRUE );
		}

		if ( skill->target == TARGET_CHAR_OFFENSIVE
		  && victim != ch
		  && !char_died(victim) )
		{
			CHAR_DATA *vch;
			CHAR_DATA *vch_next;

			for ( vch = ch->in_room->first_person;  vch;  vch = vch_next )
			{
				vch_next = vch->next_in_room;

				if ( vch != victim )								continue;
				if ( victim->fighting )								continue;

				if ( victim->master != ch )
				{
					retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
					break;
				}
			}
		}
		return TRUE;
    } /* chiude l'if */

	if ( mana != 0 )
		ch->points[POINTS_MANA] -= mana;

	ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
	ch->last_cmd = skill->skill_fun;
	start_timer( &time_used );
	(*skill->skill_fun) ( ch, argument );
	end_timer( &time_used );
	update_userec( &time_used, &skill->use_count );

	tail_chain( );
	return TRUE;
}


/*
 * Standard luck check.
 * Mental state bonus/penalty:  Your mental state is a ranged value with zero (0)
 *  being at a perfect mental state (bonus of 10).
 * Negative values would reflect how sedated one is, and
 *	positive values would reflect how stimulated one is.
 * In most circumstances you'd do best at a perfectly balanced state.
 */
bool chance( CHAR_DATA *ch, int percent )
{
	int	ms;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "chance: null ch!" );
		return FALSE;
	}

	ms = 10 - abs( ch->mental_state );

	/* Migrazione perché è l'attributo più vicino alla fortuna in un certo senso,
	 *	la migrazione è la manipolazione della realtà astrale, quindi più la si conosce
	 *	più si fa pesare le sorti */
	if ( number_percent() - (get_curr_attr(ch, ATTR_MIG)/4 + 13) - ms <= percent )
		return TRUE;
	else
		return FALSE;
}


#if T2_ALFA
/* Da xandra */
char *get_skill_alias( const int learned )
{
	if		( learned >= 90 ) 	return "superbo";
	else if ( learned >= 75 )	return "esperto";
	else if ( learned >= 60 )	return "buono";
	else if ( learned >= 45 )	return "sufficiente";
	else if ( learned >= 30 )	return "mediocre";
	else if ( learned >= 15 )	return "scarso";
	else if ( learned >	  0 )	return "dilettante";	/* (TT) parto da 1 o 0? */

	/* Altrimenti è un valore errato */
	send_log( NULL, LOG_BUG, "get_skill_alias: errore nel valore learned: %d", learned );
	return "(errore)";
}
#endif


/*
 * Comando di pratica, serve per imparare da un maestro
 */
DO_RET do_practice( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];
	int		sn;
#ifdef T2_SKILL
	char	preability[MSL];
	int		preskill;
#endif

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		int		col;
		int		lasttype;
		int		cnt;

		col = cnt = 0;
		lasttype = SKILLTYPE_SPELL;

		set_char_color( AT_SKILL, ch );

		/* Elenca incantesimi, skill e weapon */
		for ( sn = 0;  sn < top_sn;  sn++ )
		{
			if ( !VALID_STR(table_skill[sn]->name) )
				continue;

			if ( !strcmp(table_skill[sn]->name, "reserved")
			  && (IS_ADMIN(ch) || can_cast(ch)) )
			{
				if ( col%3 != 0 )
					send_to_pager( ch, "\r\n" );
				set_char_color( AT_MAGIC, ch );
				send_to_pager( ch, " ---------------------------------[&CIncantesimi&w]---------------------------------\r\n" );
				col = 0;
			}

			if ( table_skill[sn]->type != lasttype )
			{
				if ( !cnt )
					send_to_pager( ch, "                                  (nessuno)\r\n" );
				else if ( col % 3 != 0 )
					send_to_pager( ch, "\r\n" );
				set_char_color( AT_SKILL, ch );
				pager_printf( ch, " ----------------------------------[&C%s&w]----------------------------------\r\n",
					code_name(NULL, table_skill[sn]->type, CODE_SKILLTYPE) );
				col = cnt = 0;
			}

			lasttype = table_skill[sn]->type;

			if ( get_level(ch) < table_skill[sn]->skill_level[ch->class] )
				continue;
			if ( !IS_ADMIN(ch) && table_skill[sn]->skill_level[ch->class] == 0 )
				continue;

#ifdef T2_SKILL
			if ( VALID_STR(table_skill[sn]->pre_skill1) )
			{
				one_argument( table_skill[sn]->pre_skill1, preability );
				if ( skill_lookup(preability) != -1 )
				{
					preskill = skill_lookup( preability );
					if ( ch->pg->learned_skell[preskill] < 80 )
						continue;
				}
			}

			if ( VALID_STR(table_skill) )
			{
				one_argument( table_skill[sn]->pre_skill2, preability );

				if ( skill_lookup(preability) != -1 )
				{
					preskill= skill_lookup(preability);
					if (ch->pg->learned_skell[preskill] < 60 )
						continue;
				}
			}

			if ( VALID_STR(table_skill[sn]) )
			{
				one_argument( table_skill[sn]->pre_skill3, preability );
				if ( skill_lookup(preability) != -1 )
				{
					preskill= skill_lookup(preability);
					if ( ch->pg->learned_skell[preskill] < 40 )
						continue;
				}
			}
#endif
			if ( ch->pg->learned_skell[sn] <= 0 && SPELL_FLAG(table_skill[sn], SKELL_SECRETSKILL) )
				continue;

			++cnt;
			set_char_color( AT_SKILL, ch );
			pager_printf( ch, "%20.20s", table_skill[sn]->name );

			if		( ch->pg->learned_skell[sn] >= table_skill[sn]->skill_adept[ch->class] )
				set_char_color( AT_YELLOW, ch );
			else if ( ch->pg->learned_skell[sn] > 0 )
				set_char_color( AT_SCORE, ch );
			else
				set_char_color( AT_GREY, ch );

			pager_printf( ch, " &W%3d&w%% ", ch->pg->learned_skell[sn] );
			if ( (++col % 3) == 0 )
				send_to_pager( ch, "\r\n" );
		} /* chiude il for */

		if ( ch->pg->practice == 0 )
			send_to_pager( ch, "\r\nNon possiedi pratiche con cui poterti allenare.\r\n" );
		else
			pager_printf( ch, "\r\nPossiedi %d pratiche con cui poteri allenare.\r\n", ch->pg->practice );
	}
	else
	{
		CHAR_DATA  *mob;
		int			adept;
		int			cost;

		if ( !is_awake(ch) )
		{
			send_to_char( ch, "Nei tuoi sogni, o cosa?\r\n" );
			return;
		}

		for ( mob = ch->in_room->first_person;  mob;  mob = mob->next_in_room )
		{
			if ( IS_MOB(mob) && HAS_BIT_ACT(mob, MOB_PRACTICE) )
				break;
		}

		if ( mob == NULL )
		{
			send_to_char( ch, "Non lo puoi fare.\r\n" );
			return;
		}

		if ( ch->pg->practice <= 0 )
		{
			sprintf( buf, "say a %s Non hai accumulato abbastanza allenamento.", ch->name );	/* (FF) per fare effetti qui sarebbe meglio far alzare il sopracciglio come espressione faccina */
			send_command( mob, buf, CO );
			return;
		}

		sn = skill_lookup( argument );
		if ( sn == -1 )
		{
			/* Se non ha trovato nulla magari è una lingua */
			if ( get_lang(argument) )
			{
				sprintf( buf, "say a %s Non ho tempo da perdere con te, non insegno lingue.", ch->name );
				send_command( mob, buf, CO );
				return;
			}

			sprintf( buf, "say a %s Non conosco nulla del genere.", ch->name );
			send_command( mob, buf, CO );
			return;
		}

		if ( IS_PG(ch) && get_level(ch) < table_skill[sn]->skill_level[ch->class] )
		{
			sprintf( buf, "say a %s Non sei ancora pronto.", ch->name );
			send_command( mob, buf, CO );
			return;
		}

#ifdef T2_ALFA
		/* (FF) toglierlo e fare il futuro sistema Insegnati con quest personalizzate */
		/*
		 * Skill requires a special teacher
		 */
		if ( VALID_STR(table_skill[sn]->teachers) )
		{
			char	temp[MIL];

			sprintf( temp, "%d", mob->pIndexData->vnum );
			if ( !is_name(temp, table_skill[sn]->teachers) )
			{
				sprintf( buf, "say a %s Prima devi uccidere il Grande Drago, poi ne riparliamo.", ch->name );
				send_command( mob, buf, CO );
				return;
			}
		}
#endif

#ifdef T2_ALFA
		/* (FF) figata... peccato che i le classi dei mob non siano caratterizzate a dovere */
		if ( get_level(mob) < table_skill[sn]->skill_level[ch->class]
		  || get_level(mob) < table_skill[sn]->skill_level[mob->class] )
		{
			sprintf( buf, "say a %s Spiacente non sono il tuo insegnante.", ch->name );
			send_command( mob, buf, CO );
			return;
		}
#endif

		/* Calcola il costo della lezione */
		if ( get_level(ch) <= LVL_NEWBIE/2 )
			cost = 0;
		else
			cost = compute_cost( ch, mob, (table_skill[sn]->skill_level[ch->class]/3) * (get_level(ch)/4) * 6 );

		if ( cost <= 0 )
		{
			sprintf( buf, "say a %s Pare che oggi sia il vostro giorno fortunato, vi allenerò gratuitamente!", ch->name );
			send_command( mob, buf, CO );
		}

		if ( ch->gold < cost )
		{
			sprintf( buf, "say a %s La lezione ti costerà %d e noto che ti manca della moneta.", ch->name, cost );
			send_command( mob, buf, CO );
			return;
		}

		if ( cost > 0 )
			ch->gold -= cost;

		/* Al massimo può insegnare per il 40% apprendibile dalla classe */
		adept = table_skill[sn]->skill_adept[ch->class] * 0.4;

		if ( ch->pg->learned_skell[sn] >= adept )
		{
			if ( ch->pg->learned_skell[sn] >= get_adept_skell(ch, sn) )
				sprintf( buf, "say a %s Hai appreso tutto su %s.", ch->name, table_skill[sn]->name );
			else
				sprintf( buf, "say a %s Hai appreso tutto ciò che posso insegnarti su %s.", ch->name, table_skill[sn]->name );

			send_command( mob, buf, CO );
		}
		else
		{
			ch->pg->practice--;

			/* Minimo 4, massimo 29 */
			ch->pg->learned_skell[sn] += 4 + get_curr_attr( ch, ATTR_COC ) / 4;
			if ( ch->pg->learned_skell[sn] >= adept )
				ch->pg->learned_skell[sn] = adept;

			act( AT_ACTION, "Mi alleno in $T.", ch, NULL, table_skill[sn]->name, TO_CHAR );		/* (FF) magari creare un messaggio personalizzato di allenamento per ogni skill */
			act( AT_ACTION, "$n si allena in $T.", ch, NULL, table_skill[sn]->name, TO_ROOM );
		}
		sprintf( buf, "say a %s Vai ad esercitarti ora.", ch->name );
		send_command( mob, buf, CO );
	} /* chiude l'else */
}


/*
---------------------------------------------------------------------
 *  Slotlist immortal command
 *  Author: Cronel (cronel_kal@hotmail.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
---------------------------------------------------------------------
 */
DO_RET do_slotlist( CHAR_DATA *ch, char *argument )
{
	int slot;
	int sn;
	int sn_count;
	int last_slot_found;

	sn_count = top_sn;
	last_slot_found = 0;
	for ( slot = 0;  sn_count > 0;  slot++ )
	{
		for ( sn = 0;  sn < top_sn;  sn++ )
		{
			if ( table_skill[sn]->slot != slot )
				continue;

			if ( slot - last_slot_found > 1 )
			{
				set_char_color( AT_YELLOW, ch );
				if ( slot - last_slot_found > 2 )
					pager_printf( ch, "Slot: %-5d - %5d \t   Unused\n\r", last_slot_found + 1, slot - 1 );
				else
					pager_printf( ch, "Slot: %-5d       \t   Unused\n\r", slot - 1 );
			}
			set_char_color( AT_BLUE, ch );
			pager_printf( ch, "Slot: %-5d \t (%-7s) %-20.20s ",
				slot,
				code_name(NULL, table_skill[sn]->type, CODE_SKILLTYPE),
				table_skill[sn]->name );

			if ( slot == 0 && table_skill[sn]->type == SKILLTYPE_SPELL )
			{
				set_char_color( AT_RED, ch );
				send_to_pager( ch, "(WARNING: Spell with slot 0) " );
			}

			if ( slot == last_slot_found && slot != 0 )
			{
				set_char_color( AT_RED, ch );
				send_to_pager( ch, "(WARNING: Duplicate slot) " );
			}

			send_to_pager( ch, "\n\r" );
			last_slot_found = slot;
			sn_count--;
		}
	}
}


/*
 * Dummy function
 */
DO_RET skill_notfound( CHAR_DATA *ch, char *argument )
{
	send_to_char( ch, NOFOUND_MSG );
}


DO_RET do_skills( CHAR_DATA *ch, char *argument )
{
	char	skn[MIL];
	char	buf[MIL];
	char	arg1[MIL];
	char	arg2[MIL];
	int		sn;
	int		x;
	int		low_level;
	int		hig_level;
	int		lasttype = SKILLTYPE_SPELL;
	bool	lFound;

	if ( IS_MOB(ch) )
		return;

	/* I pg vedono solo nel range tra 1 e il proprio livello più 5, gli admin tutti i livelli */
	low_level = 1;
	if ( IS_ADMIN(ch) )
		hig_level = LVL_LEGEND;
	else
		hig_level = get_level(ch) + 5;

	/* Gli admin possono decidere il range di livello */
	if ( IS_ADMIN(ch) )
	{
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );

		if ( VALID_STR(arg1) )
			low_level = atoi( arg1 );

		if ( low_level < 1 || low_level > LVL_LEGEND )
			low_level = 1;

		if ( VALID_STR(arg2) )
			hig_level = atoi( arg2 );

		if ( hig_level < 0  || hig_level > LVL_LEGEND )
			hig_level = LVL_LEGEND;

		if ( low_level > hig_level )
			low_level = hig_level;
	}

	set_char_color( AT_MAGIC, ch );
	send_to_pager( ch, "&wLista delle &RAbilità &we degli &CIncantesimi&w:\r\n" );
	send_to_pager( ch, "&B-----------------------------------------&w\r\n" );

	for ( x = low_level;  x <= hig_level;  x++ )
	{
		lFound = FALSE;
		sprintf( skn, "Spell" );
		for ( sn = 0;  sn < top_sn;  sn++ )
		{
			if ( !VALID_STR(table_skill[sn]->name) )
				break;

			if ( table_skill[sn]->type != lasttype )
			{
				lasttype = table_skill[sn]->type;
				strcpy( skn, code_name(NULL, lasttype, CODE_SKILLTYPE) );
			}

			if ( ch->pg->learned_skell[sn] <= 0 && SPELL_FLAG(table_skill[sn], SKELL_SECRETSKILL) )
				continue;

			if ( lFound == FALSE )
			{
				lFound = TRUE;
				if ( IS_ADMIN(ch) )
					pager_printf( ch, "&G[ADM] Livello %d&w\r\n", x );
				else if ( get_level(ch)+1 == x )
					send_to_pager( ch, "&GLe prossime &RAbilità&G e &CIncantesimi&G che apprenderò sono:&w\r\n" );
			}

			if ( table_skill[sn]->skill_level[ch->class] != x )
				continue;

			switch ( table_skill[sn]->min_position )
			{
			  default:
				send_log( NULL, LOG_BUG, "do_skills: posizione %d inesistente per la skill %s",
					table_skill[sn]->min_position, table_skill[sn]->name );
				strcpy( buf, "sconosciuta" );
				break;
			  case POSITION_DEAD:		strcpy ( buf, "&zqualsiasi"							);	break;
			  case POSITION_MORTAL:		sprintf( buf, "&rferit%c mortalmente", gramm_ao(ch)	);	break;
			  case POSITION_INCAP:		sprintf( buf, "&Bincapacitat%c", gramm_ao(ch)		);	break;
			  case POSITION_STUN:		sprintf( buf, "&cstordit%c", gramm_ao(ch)			);	break;
			  case POSITION_SLEEP:		strcpy ( buf, "&Cdormiente"							);	break;
			  case POSITION_REST:		strcpy ( buf, "&gmentre mi riposo"					);	break;
			  case POSITION_STAND:		strcpy ( buf, "&win piedi"							);	break;
			  case POSITION_FIGHT:		strcpy ( buf, "&Rcombattendo"						);	break;
			  case POSITION_EVASIVE:		strcpy ( buf, "&Rcombat. &CEvasivo"					);	break;
			  case POSITION_DEFENSIVE:	strcpy ( buf, "&Rcombat. &GDifensivo"				);	break;
			  case POSITION_AGGRESSIVE:	strcpy ( buf, "&Rcombat. &YAggressivo"				);	break;
			  case POSITION_BERSERK:		strcpy ( buf, "&Rcombat. Berserk"					);	break;
			  case POSITION_MOUNT:		strcpy ( buf, "&Osu una cavalcatura"				);	break;
			  case POSITION_SIT:			sprintf( buf, "&Gsedut%c", gramm_ao(ch)				);	break;
			  case POSITION_SHOVE:		strcpy ( buf, "Spingendo"							);	break;
			  case POSITION_DRAG:		strcpy ( buf, "Trascinando"							);	break;
			}

			pager_printf( ch, "%-7s: &W%-20.20s &Y%3d&w%%  &wMax &R%3d  &wPos. minima: %s&w\r\n",
				skn,
				table_skill[sn]->name,
				ch->pg->learned_skell[sn],
				table_skill[sn]->skill_adept[ch->class],
				buf );
		} /* chiude il for */
	} /* chiude il for */
}


/*
 * Lookup a skills information.
 */
DO_RET do_showskill( CHAR_DATA *ch, char *argument )
{
	SKILL_DATA	*skill = NULL;
	char		 buf[MSL];
	int			 sn;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Vedere quale skill o spell?\r\n" );
		return;
	}

	if ( !str_cmp_all(argument, FALSE) )
	{
		for ( sn = 0;  sn < top_sn && table_skill[sn] && table_skill[sn]->name;  sn++ )
		{
			pager_printf( ch, "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\r\n",
				sn,
				table_skill[sn]->slot,
				table_skill[sn]->name,
				code_name(NULL, SPELL_DAMAGE(table_skill[sn]), CODE_SPELLDAMAGE) );
		}
	}

	if ( is_name(argument, "erba erbe herb herbs") )
	{
		for ( sn = 0;  sn < MAX_HERB && table_herb[sn] && table_herb[sn]->name;  sn++ )
			pager_printf( ch, "%d) %s\r\n", sn, table_herb[sn]->name );
	}
	else
	{
		SMAUG_AFF *saf;
		int		   cnt = 0;

		if ( argument[0] == 'h' && is_number(argument+1) )
		{
			sn = atoi( argument+1 );
			if ( !is_valid_herb(sn) )
			{
				send_to_char( ch, "Erba non adatta.\r\n" );
				return;
			}
			skill = table_herb[sn];
		}
		else if ( is_number(argument) )
		{
			sn = atoi( argument );
			if ( (skill = get_skilltype(sn)) == NULL )
			{
				send_to_char( ch, "sn non valida.\r\n" );
				return;
			}
			sn %= 1000;
		}
		else if ( (sn = skill_lookup(argument)) >= 0 )
		{
			skill = table_skill[sn];
		}
		else if ( (sn = herb_lookup(argument)) >= 0 )
		{
			skill = table_herb[sn];
		}
		else
		{
			send_to_char( ch, "Non ho mai sentito nulla di simile.\r\n" );
			return;
		}

		if ( !skill )
		{
			send_to_char( ch, "Non è ancora conosciuto nulla di simile.\r\n" );
			return;
		}

		ch_printf( ch, "Sn: %4d Slot: %4d %s: '%-20s'\r\n",
			sn, skill->slot, code_name(NULL, skill->type, CODE_SKILLTYPE), skill->name );

		if ( skill->info )
		{
			ch_printf( ch, "Danno: %s  Azione: %s   TipoClasse: %s   Potere: %s\r\n",
				code_name(NULL, SPELL_DAMAGE(skill), CODE_SPELLDAMAGE),
				code_name(NULL, SPELL_ACTION(skill), CODE_SPELLACTION),
				code_name(NULL, SPELL_CLASS(skill), CODE_SPELLSPHERE),
				code_name(NULL, SPELL_POWER(skill), CODE_SPELLPOWER) );
		}

		if ( skill->flags )
		{
			int x;

			strcpy( buf, "Flags:" );
			for ( x = 0;  x < MAX_SKELL;  x++ )
			{
				if ( SPELL_FLAG(skill, 1 << x) )
				{
					strcat( buf, " " );
					strcat( buf, code_name(NULL, x, CODE_SKELL) );
				}
			}
			ch_printf( ch, " %s\r\n", buf );
		}
		ch_printf( ch, "Saves: %s  SaveEffect: %s\r\n",
			code_name(NULL, skill->saves, CODE_SAVETHROW),
			code_name(NULL, SPELL_SAVE(skill), CODE_SPELLSAVE) );

		if ( skill->difficulty )
			ch_printf( ch, "Difficoltà: %d\r\n", skill->difficulty );

		ch_printf( ch, "Tipo: %s  Bersaglio: %s  Minpos: %d  Mana: %d  Beats: %d  Range: %d\r\n",
			code_name(NULL, skill->type, CODE_SKILLTYPE),
			code_name(NULL, URANGE(TARGET_IGNORE, skill->target, TARGET_OBJ_INV), CODE_TARGET),
			skill->min_position,
			skill->min_mana,
			skill->beats,
			skill->range );

		ch_printf( ch, "Flag: %s", code_bitint(NULL, skill->flags, CODE_SKELL) );

  		ch_printf( ch, "Valore: %d  Info: %d  Funzione: %s\r\n",
			skill->value,
			skill->info,
			skill->fun_name );

		ch_printf( ch, "Settori consentiti: %s\n", (IS_EMPTY(skill->spell_sector))  ?
			code_bit(NULL, skill->spell_sector, CODE_SECTOR)  :  "Tutti" );

		ch_printf( ch, "Dammsg: %s\r\nWearoff: %s\n",
			skill->noun_damage,
			skill->msg_off  ?  skill->msg_off  :  "(none set)" );

		if ( VALID_STR(skill->dice) )
			ch_printf( ch, "Dice: %s\r\n", skill->dice );
		if ( VALID_STR(skill->teachers) )
			ch_printf( ch, "Maestri: %s\r\n", skill->teachers );
		if ( VALID_STR(skill->components) )
			ch_printf( ch, "Componenti: %s\r\n", skill->components );
		if ( skill->participants )
			ch_printf( ch, "Participanti: %d\r\n", (int) skill->participants );
		if ( skill->use_count.num_uses )
			send_timer(&skill->use_count, ch);

		for ( saf = skill->first_affect;  saf;  saf = saf->next )
		{
			if ( saf == skill->first_affect )
				ch_printf( ch, "\r\n" );

			sprintf( buf, "Affect %d", ++cnt );

			if ( saf->location )
			{
				strcat( buf, " modifica " );
				strcat( buf, code_name(NULL, saf->location%REVERSE_APPLY, CODE_APPLY) );	/* (TT) da controllare che non cambiando l'enumerazione degli affected_by o degli apply questo calcolo % sballi */
				strcat( buf, " per '" );
				strcat( buf, saf->modifier );
				if ( saf->bitvector != -1 )
					strcat( buf, "' e" );
				else
					strcat( buf, "'" );
			}

			if ( saf->bitvector != -1 )
			{
				strcat( buf, " applica " );
				strcat( buf, code_str(NULL, saf->bitvector, CODE_AFFECT) );
			}

			if ( VALID_STR(saf->duration) )
			{
				strcat( buf, " per '" );
				strcat( buf, saf->duration );
				strcat( buf, "' turni" );
			}

			if ( saf->location >= REVERSE_APPLY )
				strcat( buf, " (affetto solo il caster)" );

			ch_printf( ch, "%s\r\n", buf );

			if ( !saf->next )
				send_to_char( ch, "\r\n" );
		} /* chiude il for */

		if ( VALID_STR(skill->hit_char) )
			ch_printf( ch, "Hitchar   : %s\r\n", skill->hit_char );
		if ( VALID_STR(skill->hit_vict) )
			ch_printf( ch, "Hitvict   : %s\r\n", skill->hit_vict );
		if ( VALID_STR(skill->hit_room) )
			ch_printf( ch, "Hitroom   : %s\r\n", skill->hit_room );
		if ( VALID_STR(skill->hit_dest) )
			ch_printf( ch, "Hitdest   : %s\r\n", skill->hit_dest );
		if ( VALID_STR(skill->miss_char) )
			ch_printf( ch, "Misschar  : %s\r\n", skill->miss_char );
		if ( VALID_STR(skill->miss_vict) )
			ch_printf( ch, "Missvict  : %s\r\n", skill->miss_vict );
		if ( VALID_STR(skill->miss_room) )
			ch_printf( ch, "Missroom  : %s\r\n", skill->miss_room );
		if ( VALID_STR(skill->die_char) )
			ch_printf( ch, "Diechar   : %s\r\n", skill->die_char );
		if ( VALID_STR(skill->die_vict) )
			ch_printf( ch, "Dievict   : %s\r\n", skill->die_vict );
		if ( VALID_STR(skill->die_room) )
			ch_printf( ch, "Dieroom   : %s\r\n", skill->die_room );
		if ( VALID_STR(skill->imm_char) )
			ch_printf( ch, "Immchar   : %s\r\n", skill->imm_char );
		if ( VALID_STR(skill->imm_vict) )
			ch_printf( ch, "Immvict   : %s\r\n", skill->imm_vict );
		if ( VALID_STR(skill->imm_room) )
			ch_printf( ch, "Immroom   : %s\r\n", skill->imm_room );

		if ( skill->type != SKILLTYPE_HERB )
		{
			int x;

			if ( skill->type != SKILLTYPE_RACIAL )
			{
				send_to_char( ch, "--------------------------[CLASS USE]--------------------------\r\n" );

				for ( x = 0;  x < MAX_CLASS;  x++ )
				{
					strcpy( buf, table_class[x]->name_male );
					sprintf(buf+3, ") lvl: %3d max: %2d%%",
						skill->skill_level[x],
						skill->skill_adept[x] );

					if ( x%3 == 2 )
						strcat( buf, "\r\n" );
					else
						strcat( buf, "  " );

					send_to_char( ch, buf );
				}
			}
			else
			{
				send_to_char( ch, "\r\n--------------------------[RACE MODIFIER]--------------------------\r\n" );

				for ( x = 0;  x < MAX_RACE;  x++ )
				{
					sprintf(buf, "%8.8s) modifier: %2d%%",
						table_race[x]->name_male,
						skill->race_mod[x] );	/* (GG) modificatore adept, cmq tutto questo rimetterlo come race skill magari e fare da parte questo mod */

					if ( (x > 0) && (x%2 == 1) )
						strcat( buf, "\r\n" );
					else
						strcat( buf, "   " );

					send_to_char( ch, buf );
				}
			}
		}
		send_to_char( ch, "\r\n" );
	} /* chiude l'else */
}


/*
 * Set a skill's attributes or what skills a player has.
 */
DO_RET do_setskill( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	char		arg1[MIL];
	char		arg2[MIL];
	int			value;
	int			sn;
	bool		fAll;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(argument) || !is_number(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  setskill <victim> <skill> <value>\r\n" );
		send_to_char( ch, "       o:  setskill <victim> all     <value>\r\n" );
		return;
	}

	victim = get_player_mud( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non è online.\r\n" );
		return;
	}

	fAll = !str_cmp_all( arg2, FALSE );
	sn   = 0;
	if ( !fAll && (sn = skill_lookup(arg2)) < 0 )
	{
		send_to_char( ch, "No such skill or spell.\r\n" );
		return;
    }

	value = atoi( argument );
	if ( value < 0 || value > MAX_LEARN_SKELL )
	{
		ch_printf( ch, "Value range is 0 to %d.\r\n", MAX_LEARN_SKELL );
		return;
	}

	if ( fAll )
	{
		for ( sn = 0;  sn < top_sn;  sn++ )
		{
			if ( !VALID_STR(table_skill[sn]->name) )
				continue;
            if ( get_level(victim) >= table_skill[sn]->skill_level[victim->class] || value == 0 )
			{
				if ( !IS_ADMIN(victim) && value > get_adept_skell(victim, sn) )
					victim->pg->learned_skell[sn] = get_adept_skell( victim, sn );
				else
					victim->pg->learned_skell[sn] = value;
			}
		}
	}
	else
	{
		victim->pg->learned_skell[sn] = value;
	}

	save_player( victim );
}


DO_RET do_gouge( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	AFFECT_DATA	*aff;
	int			 dam;
	int			 gouge_chance;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Non riesco a concentrarmi abbastanza per farlo.\r\n" );
		return;
	}

	if ( !can_use_skill(ch, 0, gsn_gouge) )
	{
		send_to_char( ch, "Non conosco abbastanza quest'abilità per utilizzarla.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso avvicinarmi abbastanza se monto una cavalcatura.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo nessuno.\r\n" );
		return;
	}

	gouge_chance = ( (get_curr_attr(victim, ATTR_AGI)/4 - get_curr_attr(ch, ATTR_AGI)/4) * 2 ) + 10;
	if ( IS_PG(ch) && IS_PG(victim) )
		gouge_chance += sysdata.gouge_plr_vs_plr;
	if ( victim->fighting && victim->fighting->who != ch )
		gouge_chance += sysdata.gouge_nontank;

	if ( can_use_skill(ch, number_percent() + gouge_chance, gsn_gouge) )
	{
		dam = number_range( 5, get_level(ch)/2 );
		global_retcode = damage( ch, victim, dam, gsn_gouge );
		if ( global_retcode == rNONE )
		{
			if ( !HAS_BIT(victim->affected_by, AFFECT_BLIND) )
			{
				int num;

				CREATE( aff, AFFECT_DATA, 1 );
				aff->type      = gsn_blindness;
				aff->location  = APPLY_HITROLL;
				aff->modifier  = -6;
				num = get_curr_attr( victim, ATTR_RES ) / 4;
				num = UMAX( 1, num );	/* per evitare valori a zero */
				aff->duration = ( (get_level(ch)/2 + 10) / num ) * 3;
				aff->bitvector = meb( AFFECT_BLIND );
				affect_to_char( victim, aff );

				act( AT_SKILL, "Non riesco a vedere nulla!", victim, NULL, NULL, TO_CHAR );
			}

			WAIT_STATE( ch, PULSE_VIOLENCE );

			if ( IS_PG(ch) && IS_PG(victim) )
			{
				if ( number_bits(1) == 0 )
				{
					act( AT_SKILL, "$N sembra frastornat$x.\r\n", ch, NULL, victim, TO_CHAR );
					act( AT_SKILL, "Mi sento frastornat$x..\r\n", victim, NULL, NULL, TO_CHAR );
					WAIT_STATE( victim, PULSE_VIOLENCE );
				}
			}
			else
			{
				WAIT_STATE( victim, PULSE_VIOLENCE );
			}
		}
		else if ( global_retcode == rVICT_DIED )
		{
			act( AT_DRED, "Le mie dita raggiungono il cervello della vittima causandone la morte istantanea!",
				ch, NULL, NULL, TO_CHAR );
		}

		if ( global_retcode != rCHAR_DIED && global_retcode != rBOTH_DIED )
			learn_skell( ch, gsn_gouge, TRUE );
	}
	else
	{
		WAIT_STATE( ch, table_skill[gsn_gouge]->beats * 3 );	/* (WT) */
		global_retcode = damage( ch, victim, 0, gsn_gouge );
		learn_skell( ch, gsn_gouge, FALSE );
	}
}


DO_RET do_skin( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *korps;
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *skin;
	char	 *name;
	char	  buf[MSL];
	bool	  found;

	found = FALSE;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Quale corpo dovrei scuoiare?\r\n" );
		return;
	}

	if ( (corpse = get_obj_here(ch, argument)) == NULL )
	{
		send_to_char( ch, "Non trovo nulla del genere.\r\n" );
		return;
	}

	if ( (obj = get_eq_char(ch, WEARLOC_WIELD)) == NULL )
	{
		send_to_char( ch, "Non ho l'arma adatta per questo.\r\n" );
		return;
	}

	if ( corpse->type != OBJTYPE_CORPSE_PG )
	{
		send_to_char( ch, "Posso farlo solo con corpi di avventurieri.\r\n" );
		return;
	}

	if ( obj->corpse->tick != 1
	  && obj->corpse->tick != 2
	  && obj->corpse->tick != 3
	  && obj->corpse->tick != VNUM_OBJ_CORPSE_PC )
	{
		send_to_char( ch, "Non posso fare nulla con questo corpo.\r\n" );
		return;
	}

	if ( get_obj_proto(NULL, VNUM_OBJ_SKIN) == NULL )
	{
		send_log( NULL, LOG_BUG, "do_skin: Vnum %d not found for do_skin!", VNUM_OBJ_SKIN );
		return;
	}

	korps	= make_object( get_obj_proto(NULL, VNUM_OBJ_CORPSE_PC), 0 );
	skin	= make_object( get_obj_proto(NULL, VNUM_OBJ_SKIN), 0 );
	name	= IS_MOB(ch)  ?  korps->short_descr  :  corpse->short_descr;

	sprintf( buf, skin->short_descr, name );
	DISPOSE( skin->short_descr );
	skin->short_descr = str_dup( buf );
	sprintf( buf, skin->long_descr, name );
	DISPOSE( skin->long_descr );
	skin->long_descr = str_dup( buf );

	act( AT_DRED, "$n strappa le pelli da $p.", ch, corpse, NULL, TO_ROOM);
	act( AT_DRED, "Strappo le pelli da $p.", ch, corpse, NULL, TO_CHAR);

	obj_to_char( skin, ch );
}


DO_RET do_detrap( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *trap;
	char arg  [MIL];
	int		  percent;
	bool	  found = FALSE;

	switch ( ch->substate )
	{
	  default:
		if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
		{
			send_to_char( ch, "Non riesco a concentrarmi abbastanza per farlo.\r\n" );
			return;
		}

		argument = one_argument( argument, arg );
		if ( !can_use_skill(ch, 0, gsn_detrap ) )
		{
			send_to_char( ch, "Non conosco abbastanza quest'abilità.\r\n" );
			return;
		}

		if ( !VALID_STR(arg) )
		{
			send_to_char( ch, "Su cosa dovrei farlo?\r\n" );
			return;
		}

		if ( ms_find_obj(ch) )
			return;

		if ( ch->mount )
		{
			send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
			return;
		}

		if ( !ch->in_room->first_content )
		{
			send_to_char( ch, "Non la trovo qui.\r\n" );
			return;
		}

		for ( obj = ch->in_room->first_content;  obj;  obj = obj->next_content )
		{
			if ( !can_see_obj(ch, obj, FALSE) )
				continue;

			if ( nifty_is_name(arg, obj->name) )
			{
				found = TRUE;
				break;
			}
		}

		if ( !found )
		{
			send_to_char( ch, "Non la trovo qui.\r\n" );
			return;
		}

		act( AT_ACTION, "Con cautela cerco di rimuovere la trappola da $p..", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n con cautela cerca di rimuovere la trappola da $p.", ch, obj, NULL, TO_ROOM );

		DISPOSE( ch->alloc_ptr );
		ch->alloc_ptr = str_dup( obj->name );

		add_timer( ch, TIMER_DO_FUN, PULSE_VIOLENCE * 3, do_detrap, 1 );
		return;

	  case 1:
		if ( !ch->alloc_ptr )
		{
			send_to_char( ch, "Accidenti! Niente da fare!\r\n" );
			send_log( NULL, LOG_BUG, "do_detrap: ch->alloc_ptr è NULL." );
			return;
		}
		strcpy( arg, ch->alloc_ptr );
		DISPOSE( ch->alloc_ptr );
		ch->substate = SUB_NONE;
		break;

	  case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->alloc_ptr );
		ch->substate = SUB_NONE;
		send_to_char( ch, "Qualcosa mi sfugge.. Interrompo ciò che stavo facendo.\r\n" );
		return;
	}

	if ( !ch->in_room->first_content )
	{
		send_to_char( ch, "Non la trovo qui.\r\n" );
		return;
	}

	for ( obj = ch->in_room->first_content;  obj;  obj = obj->next_content )
	{
		if ( !can_see_obj(ch, obj, FALSE) )
			continue;

		if ( nifty_is_name(arg, obj->name) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char( ch, "Non trovo qui quell'oggetto.\r\n" );
		return;
	}

	trap = get_trap( obj );
	if ( !trap )
	{
		send_to_char( ch, "Non scovo nessuna trappola in quest'oggetto.\r\n" );
		return;
	}

	percent = number_percent() - ( get_level(ch)/30 ) - ( (get_curr_attr(ch, ATTR_COC)/4 + get_curr_sense(ch, SENSE_SIXTH)/4) - 16 );

	split_obj( obj, 1 );
	if ( !can_use_skill( ch, percent, gsn_detrap ) )
	{
		send_to_char( ch, "Ooops!!\r\n" );
		spring_trap( ch, trap );
		learn_skell( ch, gsn_detrap, FALSE );
		return;
	}

	free_object( trap );

	send_to_char( ch, "Disattivo la trappola!\r\n" );
	learn_skell( ch, gsn_detrap, TRUE );
}


DO_RET do_dig( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA  *obj;
	OBJ_DATA  *startobj;
	EXIT_DATA *pexit;
	char arg   [MIL];
	bool	   found;
	bool	   shovel;

	switch ( ch->substate )
	{
	  default:
		if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
		{
			send_to_char( ch, "Non riesco a imporre al mio corpo l'azione.\r\n" );
			return;
		}

		if ( ch->mount )
		{
			send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
			return;
		}

		one_argument( argument, arg );
		if ( VALID_STR(arg) )
		{
			pexit = find_door( ch, arg, TRUE );
			if ( !pexit && get_dir(arg) == -1 )
			{
				send_to_char( ch, "In che direzione dovrei scavare?\r\n" );
				return;
			}

			if ( pexit
			  && !HAS_BIT(pexit->flags, EXIT_DIG)
			  && !HAS_BIT(pexit->flags, EXIT_CLOSED) )
			{
				send_to_char( ch, "Non ho bisogno di scavare verso quell'uscita.\r\n" );
				return;
			}
		}
		else
		{
			switch ( ch->in_room->sector )
			{
			  case SECTOR_CITY:
			  case SECTOR_INSIDE:
				send_to_char( ch, "Il terreno è troppo duro, non mi permette di scavare.\r\n" );
				return;

			  case SECTOR_WATER_SWIM:
			  case SECTOR_WATER_NOSWIM:
			  case SECTOR_UNDERWATER:
				send_to_char( ch, "Non posso scavare qui! Nell'acqua?\r\n" );
				return;

			  case SECTOR_AIR:
				send_to_char( ch, "Scavare? Nell'aria?\r\n" );
				return;
			}
		}
		add_timer( ch, TIMER_DO_FUN, PULSE_VIOLENCE * UMIN(table_skill[gsn_dig]->beats / 10, 3), do_dig, 1 );
		DISPOSE( ch->alloc_ptr );
		ch->alloc_ptr = str_dup( arg );
		send_to_char( ch, "Comincio a scavare..\r\n" );
		act( AT_PLAIN, "$n comincia a scavare.", ch, NULL, NULL, TO_ROOM );
		return;

	  case 1:
		if ( !ch->alloc_ptr )
		{
			send_to_char( ch, "Smetto di scavare!\r\n" );
			act( AT_PLAIN, "$n smette di scavare!", ch, NULL, NULL, TO_ROOM );
			send_log( NULL, LOG_BUG, "do_dig: alloc_ptr è NULL" );
			return;
		}
		strcpy( arg, ch->alloc_ptr );
		DISPOSE( ch->alloc_ptr );
		break;

	  case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->alloc_ptr );
		ch->substate = SUB_NONE;
		send_to_char( ch, "Smetto di scavare.\r\n" );
		act( AT_PLAIN, "$n smette di scavare.", ch, NULL, NULL, TO_ROOM );
		return;
	}

	ch->substate = SUB_NONE;

	/* Not having a shovel makes it harder to succeed */
	shovel = FALSE;
	for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
	{
		if ( obj->type == OBJTYPE_SHOVEL )
		{
			shovel = TRUE;
			break;
		}
	}

	/* dig out an EXIT_DIG exit... */
	if ( VALID_STR(arg) )
	{
		pexit = find_door( ch, arg, TRUE );
		if ( pexit
		  && HAS_BIT(pexit->flags, EXIT_DIG)
		  && HAS_BIT(pexit->flags, EXIT_CLOSED)
		  && can_use_skill(ch, (number_percent() * (shovel) ? 1 : 4), gsn_dig) )
		{
			REMOVE_BIT( pexit->flags, EXIT_CLOSED );
			send_to_char( ch, "I miei sforzi rivelano l'esistenza di un passaggio!\r\n" );
			act( AT_PLAIN, "$n scavando rivela l'esistenza di un passaggio!", ch, NULL, NULL, TO_ROOM );
			learn_skell( ch, gsn_dig, TRUE );
			return;
		}
		learn_skell( ch, gsn_dig, FALSE );
		send_to_char( ch, "Non trovo niente di niente! Peccato, tanta fatica per nulla..\r\n" );
		act( AT_PLAIN, "Lo scavare di $n non rivela nulla di interessante.", ch, NULL, NULL, TO_ROOM );
		return;
	}

	startobj = ch->in_room->first_content;
	found = FALSE;

	for ( obj = startobj;  obj;  obj = obj->next_content )
	{
		/* twice as hard to find something without a shovel */
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_BURIED )
		  && (can_use_skill( ch, (number_percent () * (shovel) ? 1 : 2), gsn_dig)) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char( ch, "Non trovo niente di niente! Peccato, tanta fatica per nulla..\r\n" );
		act( AT_PLAIN, "Lo scavare di $n non rivela nulla di interessante.", ch, NULL, NULL, TO_ROOM );
		learn_skell( ch, gsn_dig, FALSE );
		return;
	}

	split_obj( obj, 1 );
	REMOVE_BIT( obj->extra_flags, OBJFLAG_BURIED );
	act( AT_SKILL, "Riesco a trovare $p!", ch, obj, NULL, TO_CHAR );
	act( AT_SKILL, "$n trova $p scavando nel terreno!", ch, obj, NULL, TO_ROOM );
	learn_skell( ch, gsn_dig, TRUE );
}


DO_RET do_search( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *container;
	OBJ_DATA *startobj;
	char	  arg[MIL];
	int		  percent;
	int		  door;

	door = -1;
	switch ( ch->substate )
	{
	  default:
		if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
		{
			send_to_char( ch, "La mia volontà viene frenata prima che l'azione sia compiuta.\r\n" );
			return;
		}

		if ( ch->mount )
		{
			send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
			return;
		}

		argument = one_argument( argument, arg );
		if ( VALID_STR(arg) && (door = get_door(arg)) == -1 )
		{
			container = get_obj_here( ch, arg );
			if ( !container )
			{
				send_to_char( ch, "Non trovo nulla di simile qui.\r\n" );
				return;
			}

			if ( container->type != OBJTYPE_CONTAINER )
			{
				send_to_char( ch, "Non mi pare sia un contenitore.\r\n" );
				return;
			}

			if ( HAS_BITINT(container->container->flags, CONTAINER_CLOSED) )
			{
				send_to_char( ch, "Il contenitore è chiuso.\r\n" );
				return;
			}
		}
		add_timer( ch, TIMER_DO_FUN, PULSE_VIOLENCE * UMIN(table_skill[gsn_search]->beats / 10, 3), do_search, 1 );
		send_to_char( ch, "Comincio a cercare..\r\n" );
		DISPOSE( ch->alloc_ptr );
		ch->alloc_ptr = str_dup( arg );
		return;

	  case 1:
		if ( !ch->alloc_ptr )
		{
			send_to_char( ch, "Qualcosa interrompe la mia ricerca!\r\n" );
			send_log( NULL, LOG_BUG, "do_search: alloc_ptr è NULL" );
			return;
		}
		strcpy( arg, ch->alloc_ptr );
		DISPOSE( ch->alloc_ptr );
		break;

	  case SUB_TIMER_DO_ABORT:
		DISPOSE( ch->alloc_ptr );
		ch->substate = SUB_NONE;
		send_to_char( ch, "Interrompo la mia ricerca..\r\n" );
		return;
	}
	ch->substate = SUB_NONE;

	if ( !VALID_STR(arg) )
		startobj = ch->in_room->first_content;
	else
	{
		door = get_door( arg );
		if ( door != -1 )
			startobj = NULL;
		else
		{
			container = get_obj_here( ch, arg );
			if ( !container )
			{
				send_to_char( ch, "Non trovo nulla di simile qui.\r\n" );
				return;
			}
			startobj = container->first_content;
		}
	}

	if ( (!startobj && door == -1) || IS_MOB(ch) )
	{
		send_to_char( ch, "Non trovo nulla.\r\n" );
		learn_skell( ch, gsn_search, FALSE );
		return;
	}

	percent  = number_percent() + number_percent() - get_level(ch)/20;

	if ( door != -1 )
	{
		EXIT_DATA *pexit;

		pexit = get_exit( ch->in_room, door );
		if ( pexit
		  && HAS_BIT(pexit->flags, EXIT_SECRET)
		  && HAS_BIT(pexit->flags, EXIT_xSEARCHABLE)
		  && can_use_skill(ch, percent, gsn_search) )
		{
			act( AT_SKILL, "La tua ricerca svela l'esistenza di $d!", ch, NULL, pexit->keyword, TO_CHAR );
			act( AT_SKILL, "$n trova $d!", ch, NULL, pexit->keyword, TO_ROOM );
			REMOVE_BIT( pexit->flags, EXIT_SECRET );
			learn_skell( ch, gsn_search, TRUE );
			return;
		}
	}
	else
	{
		for ( obj = startobj;  obj;  obj = obj->next_content )
		{
			if ( !HAS_BIT(obj->extra_flags, OBJFLAG_HIDDEN) )
				continue;

			if ( can_use_skill(ch, percent, gsn_search) )
			{
				split_obj( obj, 1 );
				REMOVE_BIT( obj->extra_flags, OBJFLAG_HIDDEN );
				act( AT_SKILL, "La mia ricerca rivela l'esistenza di $p!", ch, obj, NULL, TO_CHAR );
				act( AT_SKILL, "$n trova $p!", ch, obj, NULL, TO_ROOM );
				learn_skell( ch, gsn_search, TRUE );
				return;
			}
		}
	}

	send_to_char( ch, "Non trovo nulla.\r\n" );
	learn_skell( ch, gsn_search, FALSE );
}


DO_RET do_steal( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	CHAR_DATA *mst;
	OBJ_DATA  *obj;
	char	   arg1[MIL];
	char	   arg2[MIL];
	int		   percent;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Dovrei rubare cosa e a chi?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	victim = get_char_room( ch, arg2, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "La mia vittima non si trova qui.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Che cosa intelligente.. Rubare a me stesso?!\r\n" );
		return;
	}

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "Non posso farlo qui, E' un luogo sacro.\r\n" );
		return;
	}

	/* Blocca il free stealing tra giocatori */
	if ( IS_PG(ch) && IS_PG(victim) && !sysdata.free_stealing )
	{
		set_char_color( AT_ADMIN, ch );
		send_to_char( ch, "No, è meglio non tentare di rubare a questa persona.\r\n" );
		return;
	}

	if ( IS_MOB(victim) && HAS_BIT_ACT(victim, MOB_PACIFIST)  )
	{
		send_to_char( ch, "E tu Mob saresti un pacifista? Vergogna!\r\n" );
		return;
	}

	percent  = number_percent() + (is_awake(victim)) ? 10 : -50
		- ( (get_curr_attr(ch, ATTR_COC)/4 + get_curr_sense(ch, SENSE_TOUCH)/4) - 15 )
		+ ( get_curr_sense(victim, SENSE_SIXTH)/4 - 13 );

	/* Changed the level check, made it 20 levels instead of ten and made the
	 *	victim not attack in the case of a too high level difference.  This is
	 *	to allow mobprogs where the mob steals eq without having to put level
	 *	checks into the progs.  Also gave the mobs a 10% chance of failure.
	 * (TT) (FF)
	 */
	if ( IS_MOB(victim) && get_level(ch)+20 < get_level(victim) )
	{
		send_to_char( ch, "Non riuscirei mai a rubare a questa vittima!\r\n" );
		return;
	}

	if ( victim->position == POSITION_FIGHT || !can_use_skill(ch, percent, gsn_steal) )
	{
		/*
		 * Failure.
		 */
		send_to_char( ch, "Oops..\r\n" );
		act( AT_ACTION, "$n ha cercato di derubarti!\r\n", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "$n ha cercato di derubare $N.\r\n",  ch, NULL, victim, TO_NOVICT );

		send_command( victim, "yell Al ladro!", CO );

		learn_skell( ch, gsn_steal, FALSE );
		if ( legal_loot(ch, victim) )
		{
			global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
		}
		else
		{
/*			send_log( NULL, LOG_STEAL, "do_steal: %s ha compiuto un furto.", ch->name ); */
			mst = ch->master;
			if ( IS_MOB(ch) && !mst )
				return;
			else
				mst = ch;

			if ( IS_MOB(mst) )
				return;

			if ( !HAS_BIT_PLR(mst, PLAYER_THIEF) )
			{
				SET_BIT( mst->act, PLAYER_THIEF );
				set_char_color( AT_WHITE, ch );
				send_to_char( ch, "Dannazone, mi hanno visto!\r\n" );
				save_player( mst );
			}
		}

		return;
	} /* chiude l'if */

	if ( is_name(arg1, "moneta monete oro gold coin coins") )
	{
		int amount;

		amount = (int) ( victim->gold * number_range(1, 10) / 100 );
		if ( amount <= 0 )
		{
			send_to_char( ch, "Non riesco a prendere più oro.\r\n" );
			learn_skell( ch, gsn_steal, FALSE );
			return;
		}

		ch->gold     += amount;
		victim->gold -= amount;
		ch_printf( ch, "Aha! 'Prelevo' %d monete d'oro.\r\n", amount );
		learn_skell( ch, gsn_steal, TRUE );
		return;
	}

	/* equip a FALSE perché sta cercando l'oggetto sulla vittima */
	obj = get_obj_carry( victim, arg1, FALSE );
	if ( !obj )
	{
		send_to_char( ch, "Non trovo nulla di simile sulla vittima.\r\n" );
		learn_skell( ch, gsn_steal, FALSE );
		return;
	}

	if ( !can_drop_obj(ch, obj)
	  || HAS_BIT(obj->extra_flags, OBJFLAG_INVENTORY)
	  || obj->level > get_level(ch) )
	{
		send_to_char( ch, "Non riesco a prendere quell'oggetto.\r\n" );
		learn_skell( ch, gsn_steal, FALSE );
		return;
	}

	if ( ch->carry_number > can_carry_number(ch) )
	{
		send_to_char( ch, "Devo liberarmi di qualcosa prima di poter prendere altri oggetti.\r\n" );
		learn_skell( ch, gsn_steal, FALSE );
		return;
	}

	if ( ch->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_weight(ch) )
	{
		send_to_char( ch, "E' troppo pesante.\r\n" );
		learn_skell( ch, gsn_steal, FALSE );
		return;
	}

	split_obj( obj, 1 );
	obj_from_char( obj );
	obj_to_char( obj, ch );
	send_to_char( ch, "Ecco fatto.\r\n" );

	learn_skell( ch, gsn_steal, TRUE );
}


DO_RET do_backstab( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	char	   arg[MIL];
	int		   percent;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "A chi dovrei concedere questo 'servizio'?\r\n" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non lo trova nei paraggi.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Lo trovo un po' stupido.. Non sono un suicida.\r\n" );
		return;
	}

	if ( is_safe(ch, victim, TRUE) )
		return;

	/* Added stabbing weapon */
	obj = get_eq_char( ch, WEARLOC_WIELD );
	if ( !obj || (obj->weapon->damage1 != DAMAGE_PIERCE && obj->weapon->damage1 != DAMAGE_STAB) )
	{
		send_to_char( ch, "Prima debbo impugnare un'arma adatta.\r\n" );
		return;
	}

	if ( victim->fighting )
	{
		send_to_char( ch, "La mia vittima ora sta combattendo, ho bisogno che sia ferma e rilassata.\r\n" );
		return;
	}

	/* Can backstab a char even if it's hurt as long as it's sleeping */
	if ( victim->points[POINTS_LIFE] < victim->points_max[POINTS_LIFE] && is_awake(victim) )
	{
		act( AT_PLAIN, "$N è troppo sospettos$X ora che è ferit$X, non riuscirò a prenderl$X di sorpresa.",
			ch, NULL, victim, TO_CHAR );
		return;
	}

	percent = number_percent()
		- ( (get_curr_attr(ch, ATTR_SPE)/4 + get_curr_attr(victim, ATTR_COC)/4) - 14 )
		+ ( (get_curr_attr(victim, ATTR_REF)/4 + get_curr_sense(victim, SENSE_SIXTH)/4) - 13 );

	check_attacker( ch, victim );
	WAIT_STATE( ch, table_skill[gsn_backstab]->beats * 3 );	/* (WT) */
	if ( !is_awake(victim) || can_use_skill(ch, percent, gsn_backstab) )
	{
		learn_skell( ch, gsn_backstab, TRUE );
		global_retcode = multi_hit( ch, victim, gsn_backstab );
		is_illegal_pk( ch, victim );
	}
	else
	{
		learn_skell( ch, gsn_backstab, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_backstab );
		is_illegal_pk( ch, victim );
	}
}


DO_RET do_rescue( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	CHAR_DATA *fch;
	char	   arg[MIL];
	int		   percent;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia volontà.\r\n" );
		return;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_BERSERK) )
	{
		send_to_char( ch, "L'insana rabbia non mi consente opere di salvataggio per nessuno!\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Chi dovrei salvare?\r\n" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non posso salvare chi non si trova qui.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Molto generoso da parte mia..\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non pusso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	/* (FF) pensare se invece attivarlo */
	if ( IS_PG(ch) && IS_MOB(victim) )
	{
		send_to_char( ch, "Non ha bisogno del mio aiuto.\r\n" );
		return;
	}

	if ( !ch->fighting )
	{
		send_to_char( ch, "Troppo tardi! Sono già impegnato in un altro combattimento.\r\n" );
		return;
	}

	fch = who_fighting( victim );
	if ( !fch )
	{
		send_to_char( ch, "Non sta combattendo in questo momento.\r\n" );
		return;
	}

	if ( who_fighting(victim) == ch )
	{
		send_to_char( ch, "Sta combattendo proprio contro di ME!\r\n" );
		return;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_BERSERK) )
	{
		send_to_char( ch, "No, meglio di no, non è in sé e potrebbe attaccarmi.\r\n" );
		return;
	}

	/* Al posto che utilizzare la concentrazione per tutti e due direi un misto di attr ed "empatia di gruppo", cmq tutte e due erano ex-luck */
	percent = number_percent()
		- ( (get_curr_attr(ch, ATTR_AGI)/4 + get_curr_attr(ch, ATTR_SPE)/4) - 14 )
		- ( (get_curr_attr(victim, ATTR_AGI)/4 + get_curr_attr(victim, ATTR_EMP)/4) - 16 );

	/* (FF) aggiungerci un check sul peso e l'altezza */

	WAIT_STATE( ch, table_skill[gsn_rescue]->beats * 3 );	/* (WT) */
	if ( !can_use_skill(ch, percent, gsn_rescue) )
	{
		send_to_char( ch, "Fallisco il salvataggio.\r\n" );
		act( AT_SKILL, "$n cerca di salvarmi.. ma senza successo!", ch, NULL, victim, TO_VICT );
		act( AT_SKILL, "$n cerca di salvare $N ma senza successo!", ch, NULL, victim, TO_NOVICT );
		learn_skell( ch, gsn_rescue, FALSE );
		return;
	}

	act( AT_SKILL, "Riesco a metterti tra $N e il suo avversario!",  ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n si frappone tra me e il mio avversario!", ch, NULL, victim, TO_VICT );
	act( AT_SKILL, "$n si frappone tra $N e il suo avversario!",  ch, NULL, victim, TO_NOVICT );

	learn_skell( ch, gsn_rescue, TRUE );

	stop_fighting( fch, FALSE );
	stop_fighting( victim, FALSE );

	if ( ch->fighting )
		stop_fighting( ch, FALSE );

	set_fighting( ch, fch );
	set_fighting( fch, ch );
}


DO_RET do_kick( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_MOB(ch) && HAS_BIT( ch->affected_by, AFFECT_CHARM ) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_kick]->skill_level[ch->class] )
	{
		send_to_char( ch, "Meglio lasciare i calci a chi li sa tirare.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_kick]->beats * 3 );	/* (WT) */

	if ( can_use_skill(ch, number_percent(), gsn_kick) )
	{
		learn_skell( ch, gsn_kick, TRUE );
		global_retcode = damage( ch, victim, number_range(1, get_level(ch)/2), gsn_kick );
	}
	else
	{
		learn_skell( ch, gsn_kick, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_kick );
	}
}


DO_RET do_punch( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_punch]->skill_level[ch->class] )
	{
		send_to_char( ch, "Meglio non utilizzare questa tecnica, non la conosco.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_punch]->beats * 3 );	/* (WT) */

	if ( can_use_skill(ch, number_percent(), gsn_punch) )
	{
		learn_skell( ch, gsn_punch, TRUE );
		global_retcode = damage( ch, victim, number_range(1, get_level(ch)/2), gsn_punch );
	}
	else
	{
		learn_skell( ch, gsn_punch, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_punch );
	}
}


DO_RET do_bite( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_bite]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho le capacità per farlo in maniera efficace.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_bite]->beats * 3 );	/* (WT) */

	if ( can_use_skill( ch, number_percent(), gsn_bite) )
	{
		learn_skell( ch, gsn_bite, TRUE );
		global_retcode = damage( ch, victim, number_range(1, get_level(ch)/2), gsn_bite );
	}
	else
	{
		learn_skell( ch, gsn_bite, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_bite );
	}
}


DO_RET do_claw( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_claw]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho le capacità giuste per farlo.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_claw]->beats * 3 );	/* (WT) */

	if ( can_use_skill( ch, number_percent(), gsn_claw ) )
	{
		learn_skell( ch, gsn_claw, TRUE );
		global_retcode = damage( ch, victim, number_range(1, get_level(ch)/2), gsn_claw );
	}
	else
	{
		learn_skell( ch, gsn_claw, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_claw );
	}
}


DO_RET do_sting( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_sting]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho le capacità giuste per farlo.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_sting]->beats * 3 );	/* (WT) */
	if ( can_use_skill( ch, number_percent(), gsn_sting) )
	{
		learn_skell( ch, gsn_sting, TRUE );
		global_retcode = damage( ch, victim, number_range(1, get_level(ch)/2), gsn_sting );
	}
	else
	{
		learn_skell( ch, gsn_sting, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_sting );
	}
}


DO_RET do_tail( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_tail]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho le capacità giuste per farlo.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_tail]->beats * 3 );	/* (WT) */

	if ( can_use_skill( ch, number_percent(), gsn_tail ) )
	{
		learn_skell( ch, gsn_tail, TRUE );
		global_retcode = damage( ch, victim, number_range(1, get_level(ch)/2), gsn_tail );
	}
	else
	{
		learn_skell( ch, gsn_tail, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_tail );
	}
}


DO_RET do_bash( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	int			bash_chance;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_bash]->skill_level[ch->class] )
	{
		send_to_char( ch, "Meglio che non utilizzi questa tecnica, non la conosco.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	/* Messo res alla vittima che dovrebbe attendersi il colpo e si mette in posizione per resistere allo stesso */
	bash_chance = ( ((get_curr_attr(victim, ATTR_STR)/4 + get_curr_attr(victim, ATTR_RES)/4)
				   - (get_curr_attr(ch,	    ATTR_STR)/4 + get_curr_attr(ch,	   ATTR_AGI)/4)) * 10 ) + 10;

	if ( IS_PG(ch) && IS_PG(victim) )
		bash_chance += sysdata.bash_plr_vs_plr;
	if ( victim->fighting && victim->fighting->who != ch )
		bash_chance += sysdata.bash_nontank;

	WAIT_STATE( ch, table_skill[gsn_bash]->beats * 3 );	/* (WT) */

	if ( can_use_skill(ch, (number_percent () + bash_chance), gsn_bash) )
	{
		learn_skell( ch, gsn_bash, TRUE );
		/* Do not change anything here! */
		WAIT_STATE( victim, PULSE_VIOLENCE * 2 );
		victim->position = POSITION_SIT;
		global_retcode = damage( ch, victim, number_range(1, get_level(ch)/2), gsn_bash );
	}
	else
	{
		learn_skell( ch, gsn_bash, FALSE );
		global_retcode = damage( ch, victim, 0, gsn_bash );
	}
}


DO_RET do_stun( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	int			 stun_chance;
	bool		 fail;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_stun]->skill_level[ch->class] )
	{
		send_to_char( ch, "Meglio non utilizzare questa tecnica, non la conosco.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	if ( IS_PG(ch) && ch->points[POINTS_MOVE] < ch->points_max[POINTS_MOVE]/10 && !stop_lowering_points(ch, POINTS_MOVE) )
	{
		set_char_color( AT_SKILL, ch );
		send_to_char( ch, "Non ho abbastanza fiato per farlo ora, devo riposare.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_stun]->beats * 3 );	/* (WT) */

	fail = FALSE;
	stun_chance = ris_save( victim, get_level(ch)/2, RIS_PARALYSIS );
	if ( stun_chance == 1000 )
		fail = TRUE;
	else
		fail = compute_savthrow( stun_chance, victim, SAVETHROW_PARA_PETRI );

	/* Messa costituzione alla vittima invece che str o res perché indica robustezza */
	stun_chance = ( ((get_curr_attr(victim, ATTR_AGI)/4 + get_curr_attr(victim, ATTR_CON)/4)
				   - (get_curr_attr(ch,	    ATTR_AGI)/4 + get_curr_attr(ch,     ATTR_STR)/4)) * 5 ) + 10;
	/* harder for player to stun another player */
	if ( IS_PG(ch) && IS_PG(victim) )
		stun_chance += sysdata.stun_plr_vs_plr;
	else
		stun_chance += sysdata.stun_regular;

	if ( !fail && can_use_skill(ch, (number_percent() + stun_chance), gsn_stun) )
	{
		learn_skell( ch, gsn_stun, TRUE );

		/* Do not change anything here! */
		if ( IS_PG(ch) )
			ch->points[POINTS_MOVE] -= ch->points_max[POINTS_MOVE] / 10;

		WAIT_STATE( ch,     PULSE_VIOLENCE * 2 );
		WAIT_STATE( victim, PULSE_VIOLENCE );

		act( AT_SKILL, "$N mi stordisce con un colpo massacrante!", victim, NULL, ch, TO_CHAR );
		act( AT_SKILL, "Stordisco $N con un colpo massacrante!", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n stordisce $N con un colpo massacrante!", ch, NULL, victim, TO_NOVICT );

		if ( !HAS_BIT(victim->affected_by, AFFECT_PARALYSIS) )
		{
			AFFECT_DATA	*aff;

			CREATE( aff, AFFECT_DATA, 1 );
			aff->type		= gsn_stun;
			aff->location	= APPLY_AC;
			aff->modifier	= 20;
			aff->duration	= 9;
			aff->bitvector	= meb( AFFECT_PARALYSIS );
			affect_to_char( victim, aff );

			update_position( victim );
		}
	}
	else
	{
		WAIT_STATE( ch, PULSE_VIOLENCE * 2 );

		if ( IS_PG(ch) )
			ch->points[POINTS_MOVE] -= ch->points_max[POINTS_MOVE] / 15;

		learn_skell( ch, gsn_stun, FALSE );
		act( AT_SKILL, "$n cerca di colpirmi con violenza inaudita ma riesco a schivare!", ch, NULL, victim, TO_VICT );
		act( AT_SKILL, "Cerco di stordire $N ma il mio colpo viene schivato!, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$N evita un terribile colpo sferrato da $n!", ch, NULL, victim, TO_NOVICT );
	}
}


/*#ifdef T2_VAMPIRE*/
/*
 * Skill vampiriche, da convertire in effetti collaterale del vampirismo. (FF)
 */
DO_RET do_bloodlet( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA	*obj;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "No, i mob non possono.\r\n" );
		return;
	}

	if ( ch->fighting )
	{
		send_to_char( ch, "Sto combattendo ora.\r\n" );
		return;
	}

	if ( ch->pg->condition[CONDITION_BLOODTHIRST] < 10 )
	{
		send_to_char( ch, "Non ho così tanto sangue per offrirne.\r\n" );
		return;
	}

	WAIT_STATE( ch, PULSE_VIOLENCE );
	if ( can_use_skill(ch, number_percent(), gsn_bloodlet ) )
	{
		gain_condition( ch, CONDITION_BLOODTHIRST, -7 );

		act( AT_DRED, "Solcando la mia carne con la dentatura lascio che il sangue scivoli via..", ch, NULL, NULL, TO_CHAR );
		act( AT_DRED, "$n addenta il suo polso ferendosi e lascia il suo sangue scorrere giù..", ch, NULL, NULL, TO_ROOM );

		learn_skell( ch, gsn_bloodlet, TRUE );

		obj = make_object( get_obj_proto(NULL, VNUM_OBJ_BLOODLET), 0 );
		obj->timer				  = 1;
		obj->drinkcon->curr_quant = 6;

		obj_to_room( obj, ch->in_room );
	}
	else
	{
		act( AT_DRED, "Dalla mia ferita esce qualche goccia di sangue.. Ma non abbastanza da scivolare giù.", ch, NULL, NULL, TO_CHAR );
		act( AT_DRED, "$n taglia le sue carni aprendo una ferita, il sangue rimane in gocce sulla sua pelle.", ch, NULL, NULL, TO_ROOM );

		learn_skell( ch, gsn_bloodlet, FALSE );
	}
}


DO_RET do_feed( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	int		   dam;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	/* (FF) malattia vampirica */
	if ( IS_PG(ch) && ch->race == RACE_VAMPIRE )
	{
		send_to_char( ch, "E' pericoloso nutrirmi di creature della mia specie.\r\n" );
		return;
	}

	if ( !can_use_skill(ch, 0, gsn_feed) )
	{
		send_to_char( ch, "Non ssono ancora in grado di farlo.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_feed]->beats * 3 );	/* (WT) */

	if ( can_use_skill( ch, number_percent(), gsn_feed ) )
	{
		dam = number_range( 1, get_level(ch)/2 );
		global_retcode = damage( ch, victim, dam, gsn_feed );

		if ( global_retcode == rNONE
		  && IS_PG(ch)
		  && dam
		  && ch->fighting
		  && ch->pg->condition[CONDITION_BLOODTHIRST] < (10 + get_level(ch)/2) )
		{
			gain_condition( ch, CONDITION_BLOODTHIRST,
				UMIN( number_range(1, ((get_level(ch)/2)+(get_level(victim)/2) / 20) + 3 ),
				(10 + get_level(ch)/2) - ch->pg->condition[CONDITION_BLOODTHIRST] ) );

			if ( ch->pg->condition[CONDITION_FULL] <= 37 )
				gain_condition( ch, CONDITION_FULL, 2);

			gain_condition( ch, CONDITION_THIRST, 2);
			act( AT_DRED, "Riesco a privare $N del suo sangue, ora mio.", ch, NULL, victim, TO_CHAR );
			act( AT_DRED, "$n mi succhia via del sangue!", ch, NULL, victim, TO_VICT );
			learn_skell( ch, gsn_feed, TRUE );
		}
	}
	else
	{
		global_retcode = damage( ch, victim, 0, gsn_feed );
		if ( global_retcode == rNONE && IS_PG(ch)
		  &&  ch->fighting
		  &&  ch->pg->condition[CONDITION_BLOODTHIRST] < (10 + get_level(ch)/2) )
		{
			act( AT_DRED, "L'odore del sangue di $N mi sta facendo impazzire!", ch, NULL, victim, TO_CHAR );
			act( AT_DRED, "$n sta bramando il mio sangue!", ch, NULL, victim, TO_VICT );
			learn_skell( ch, gsn_feed, FALSE );
		}
	}
}


/*
 * Convertita in funzione per essere utilizzata dai vampiri.
 */
DO_RET do_mistwalk( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg[MIL];
	bool	   allowday = FALSE;

	set_char_color( AT_DGREEN, ch );
	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Chi sarà la mia vittima?\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_mistwalk]->beats * 3 );	/* (WT) */

	victim = get_char_area( ch, arg, TRUE );
	if ( !victim || victim == ch )
	{
		send_to_char( ch, "Non riesco a percepire la presenza della mia vittima.\r\n" );
		return;
	}

	if ( ch->pg->condition[CONDITION_BLOODTHIRST] > 22 )
		allowday = TRUE;

	if ( (calendar.hour < 21 && calendar.hour > 5 && !allowday)
	  || !victim->in_room
	  || HAS_BIT(victim->in_room->flags, ROOM_PRIVATE)
	  || HAS_BIT(victim->in_room->flags, ROOM_SOLITARY)
	  || HAS_BIT(victim->in_room->flags, ROOM_NOASTRAL)
	  || HAS_BIT(victim->in_room->flags, ROOM_DEATH)
	  || HAS_BIT(ch->in_room->flags, ROOM_NORECALL)
	  || get_level(victim) >= get_level(ch)+15	/* (FF) */
	  || (IS_MOB(victim) && compute_savthrow(get_level(ch)/2, victim, SAVETHROW_SPELL_STAFF))
	  || !can_pkill(ch, victim) )
	{
		send_to_char( ch, "Non riesco a percepire la presenza della mia vittima.\r\n" );
		learn_skell( ch, gsn_mistwalk, FALSE );
		return;
	}

	/* Subtract 22 extra bp for mist walk from 0500 to 2100 SB */
	if  ( calendar.hour < 21 && calendar.hour > 5 && IS_PG(ch) )
		gain_condition( ch, CONDITION_BLOODTHIRST, - 22 );
	act( AT_DGREEN, "Il mondo intorno a me sfuma, il mio corpo muta in nube..", ch, NULL, NULL, TO_CHAR );
	act( AT_DGREEN, "$n dissolve il suo corpo in nebbia e svanisce nel nulla!", ch, NULL, NULL, TO_ROOM );
	learn_skell( ch, gsn_mistwalk, TRUE );
	char_from_room( ch );
	char_to_room( ch, victim->in_room );
	act( AT_DGREEN, "Una densa nube appare dal nulla e prende forma rivelando la presenza di $n!", ch, NULL, NULL, TO_ROOM );
	send_command( ch, "look", CO );
}
/*#endif*/


DO_RET do_broach( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA *pexit;
	char	   arg[MIL];

	set_char_color( AT_DGREEN, ch );

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "In che direzione?\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_broach]->beats * 3 );	/* (WT) */

	pexit = find_door( ch, arg, TRUE );
	if ( !pexit )
	{
		EXIT_DATA	*pexit_rev;

		if ( !HAS_BIT(pexit->flags, EXIT_CLOSED)
		  || !HAS_BIT(pexit->flags, EXIT_LOCKED)
		  || HAS_BIT(pexit->flags, EXIT_PICKPROOF)
		  || can_use_skill(ch, number_percent(), gsn_broach) )
		{
			send_to_char( ch, "Il mio tentativo non ha esito.\r\n" );
			learn_skell( ch, gsn_broach, FALSE );
			if ( pexit->vdir >= DIR_NORTH && pexit->vdir < DIR_SOMEWHERE )
				check_room_for_traps( ch, TRAPFLAG_PICK | table_dir[pexit->vdir].trap_door );
			return;
		}
		REMOVE_BIT(pexit->flags, EXIT_LOCKED);
		send_to_char( ch, "Riesco a far breccia!\r\n" );
		learn_skell( ch, gsn_broach, TRUE );

		if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
			REMOVE_BIT( pexit_rev->flags, EXIT_LOCKED );

		if ( pexit->vdir >= DIR_NORTH && pexit->vdir < DIR_SOMEWHERE )
			check_room_for_traps( ch, TRAPFLAG_PICK | table_dir[pexit->vdir].trap_door );
		return;
	}
	send_to_char( ch, "Il mio tentativo fallisce.\r\n" );
}


bool check_grip( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int grip_chance;

	if ( !is_awake(victim) )
		return FALSE;

	if ( IS_MOB(victim) && !HAS_BIT(victim->defenses, DEFENSE_GRIP) )
		return FALSE;

	if ( IS_MOB(victim) )
		grip_chance  = UMIN( 60, get_level(victim) );
	else
		grip_chance  = (int) ( knows_skell(victim, gsn_grip) / 2 );

	/* (FF) magari aggiungere un check anche a chi cerca di disarmare */
	grip_chance += ( (get_curr_attr(victim, ATTR_STR)/4 + get_curr_attr(victim, ATTR_COC)/4) - 13 ) * 2 ;

	if ( number_percent() >= grip_chance + get_level(victim)/2 - get_level(ch)/2 )
	{
		learn_skell( victim, gsn_grip, FALSE );
		return FALSE;
	}
	act( AT_SKILL, "Riesco a sfuggire il tentativo di $n di disarmarti!", ch, NULL, victim, TO_VICT );
	act( AT_SKILL, "Cerco di disarmare $N ma senza successo!", ch, NULL, victim, TO_CHAR );
	learn_skell( victim, gsn_grip, TRUE );
	return TRUE;
}


/*
 * Disarm a creature
 * Caller must check for successful attack
 * Check for loyalty flag (weapon disarms to inventory)
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
	OBJ_DATA *obj;
	OBJ_DATA *tmpobj;

	obj = get_eq_char( victim, WEARLOC_WIELD );
	if ( !obj )
		return;

	tmpobj = get_eq_char( victim, WEARLOC_DUAL );
	if ( tmpobj && number_bits(1) == 0 )
		obj = tmpobj;

	if ( get_eq_char(ch, WEARLOC_WIELD) == NULL && number_bits(1) == 0 )
	{
		learn_skell( ch, gsn_disarm, FALSE );
		return;
	}

	if ( IS_MOB(ch) && !can_see_obj(ch, obj, FALSE) && number_bits(1) == 0 )
	{
		learn_skell( ch, gsn_disarm, FALSE );
		return;
	}

	if ( check_grip(ch, victim) )
	{
		learn_skell( ch, gsn_disarm, FALSE );
		return;
	}

	act( AT_SKILL, "$n mi DISARMA!", ch, NULL, victim, TO_VICT );
	act( AT_SKILL, "Riesco a disarmare $N!",  ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n disarma $N!",  ch, NULL, victim, TO_NOVICT );
	learn_skell( ch, gsn_disarm, TRUE );

	tmpobj = get_eq_char( victim, WEARLOC_DUAL );
	if ( obj == get_eq_char(victim, WEARLOC_WIELD) && tmpobj )
		tmpobj->wear_loc = WEARLOC_WIELD;

	obj_from_char( obj );

	/* Se un pg disarma un'altro pg allora l'oggetto prende la flag di disarmato */
	if ( IS_PG(ch) && IS_PG(victim) && !HAS_BIT(obj->extra_flags, OBJFLAG_LOYAL) )
	{
		SET_BIT( obj->magic_flags, OBJMAGIC_PKDISARMED );
		obj->weapon->lvl_disarm = get_level( victim );	/* corretto, senza il /2 */
	}

	if ( IS_MOB(victim) || (HAS_BIT(obj->extra_flags, OBJFLAG_LOYAL) && IS_PG(ch)) )
		obj_to_char( obj, victim );
	else
		obj_to_room( obj, victim->in_room );
}


DO_RET do_disarm( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	int		   percent;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_disarm]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho la minima idea di come si possa disarmare.\r\n" );
		return;
	}

	if ( get_eq_char(ch, WEARLOC_WIELD) == NULL )
	{
		send_to_char( ch, "Devo possedere un'arma per disarmare l'avversario.\r\n" );
		return;
	}

	victim = who_fighting( ch );
	if ( !victim )
	{
		send_to_char( ch, "Non sto combattendo.\r\n" );
		return;
	}

	obj = get_eq_char( victim, WEARLOC_WIELD );
	if ( !obj )
	{
		send_to_char( ch, "Il mio avversario non sta impugnando nessun'arma.\r\n" );
		return;
	}

    WAIT_STATE( ch, table_skill[gsn_disarm]->beats * 3 );	/* (WT) */

	/* Per disarmare direi un misto di forza e velocità, per evitare il disarmo direi
	 *	un misto di forza e riflessi */
	percent = number_percent() + get_level(victim)/2 - get_level(ch)/2
			- ( (get_curr_attr(ch, ATTR_STR)/4 + get_curr_attr(ch, ATTR_SPE)/4) - 15 )
			+ ( (get_curr_attr(victim, ATTR_AGI)/4 + get_curr_attr(victim, ATTR_REF)/4) - 15 );

	if ( !can_see_obj(ch, obj, FALSE) )
		percent += 10;

	if ( can_use_skill(ch, (percent*3/2), gsn_disarm) )
		disarm( ch, victim );
	else
	{
		send_to_char( ch, "Fallisco miseramente il tentativo.\r\n" );
		learn_skell( ch, gsn_disarm, FALSE );
	}
}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( HAS_BIT(victim->affected_by, AFFECT_FLYING)
	  || HAS_BIT(victim->affected_by, AFFECT_FLOATING) )
	{
		return;
	}

	if ( victim->mount )
	{
		if ( HAS_BIT(victim->mount->affected_by, AFFECT_FLYING)
		  || HAS_BIT(victim->mount->affected_by, AFFECT_FLOATING) )
		{
			return;
		}

		act( AT_SKILL, "$n fa inciampare la mia cavalcatura e cado a terra!", ch, NULL, victim, TO_VICT    );
		act( AT_SKILL, "Faccio inciampare la cavalcatura di $N facendol$X cadere a terra!", ch, NULL, victim, TO_CHAR    );
		act( AT_SKILL, "$n fa inciampare la cavalcautra di $N facendol$X cadere a terra!", ch, NULL, victim, TO_NOVICT );

		REMOVE_BIT( victim->mount->act, MOB_MOUNTED );
		victim->mount = NULL;
		WAIT_STATE( ch,     PULSE_VIOLENCE * 2 );
		WAIT_STATE( victim, PULSE_VIOLENCE * 2 );
		victim->position = POSITION_REST;

		return;
	}

	if ( victim->wait == 0 )
	{
		act( AT_SKILL, "$n mi sgambetta buttandomi a terra!", ch, NULL, victim, TO_VICT );
		act( AT_SKILL, "Sgambetto $N facendol$X cadere a terra!", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n sgambetta $N mandandol$X a terra!", ch, NULL, victim, TO_NOVICT );

		WAIT_STATE( ch,     PULSE_VIOLENCE * 2 );
		WAIT_STATE( victim, PULSE_VIOLENCE * 2 );
		victim->position = POSITION_REST;
	}
}


DO_RET do_pick( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;
	OBJ_DATA  *obj;
	EXIT_DATA *pexit;
	char	   arg[MIL];

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Quale lucchetto dovrei scassinare?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_pick_lock]->beats * 3 );	/* (WT) */

	/* look for guards */
	for ( gch = ch->in_room->first_person;  gch;  gch = gch->next_in_room )
	{
		if ( IS_PG(gch) )		continue;
		if ( !is_awake(gch) )	continue;

		if ( get_level(ch) + 5 < get_level(gch) )
		{
			act( AT_PLAIN, "$N è troppo vicino alla serratura.", ch, NULL, gch, TO_CHAR );
			return;
		}
	}

	if ( !can_use_skill(ch, number_percent(), gsn_pick_lock) )
	{
		send_to_char( ch, "Non ci riesco.\r\n" );
		learn_skell( ch, gsn_pick_lock, FALSE );

#ifdef T2_ALFA
		for ( gch = ch->in_room->first_person;  gch;  gch = gch->next_in_room )
		{
			if ( IS_PG(gch) )		continue;
			if ( !is_awake(gch) )	continue;

			if ( HAS_BIT_ACT(gch, MOB_GUARDIAN) )
				multi_hit( gch, ch, TYPE_UNDEFINED );
		}
#endif
		return;
	}

	pexit = find_door( ch, arg, TRUE );
	if ( pexit )
	{
		/* 'pick door' */
/*		ROOM_DATA	*to_room; */	/* Unused */
		EXIT_DATA	*pexit_rev;

		if ( !HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			send_to_char( ch, "Non è chiuso.\r\n" );
			return;
		}

		if ( pexit->key < 0 )
		{
			send_to_char( ch, "Non riuscirò mai a scassinare questa serratura.\r\n" );
			return; }

		if ( !HAS_BIT(pexit->flags, EXIT_LOCKED) )
		{
			send_to_char( ch, "Questa serratura è già aperta.\r\n" );
			return;
		}

		if ( HAS_BIT(pexit->flags, EXIT_PICKPROOF) )
		{
			send_to_char( ch, "Fallisco il tentativo, questa porta è al di là delle mie possibilità.\r\n" );
			learn_skell( ch, gsn_pick_lock, FALSE );
			if ( pexit->vdir >= DIR_NORTH && pexit->vdir < DIR_SOMEWHERE )
				check_room_for_traps( ch, TRAPFLAG_PICK | table_dir[pexit->vdir].trap_door );
			return;
		}

		REMOVE_BIT( pexit->flags, EXIT_LOCKED );

		send_to_char( ch, "*Click*\r\n" );
#ifdef T2_MSP
		send_audio( ch->desc, "unlock.wav", TO_CHAR );
#endif
		act( AT_ACTION, "$n scassina $d.", ch, NULL, pexit->keyword, TO_ROOM );

		learn_skell( ch, gsn_pick_lock, TRUE );

		/* Pick the other side */
		if ( (pexit_rev = pexit->rexit) != NULL
		  && pexit_rev->to_room == ch->in_room )
		{
		    REMOVE_BIT( pexit_rev->flags, EXIT_LOCKED );
		}
		if ( pexit->vdir >= DIR_NORTH && pexit->vdir < DIR_SOMEWHERE )
			check_room_for_traps( ch, TRAPFLAG_PICK | table_dir[pexit->vdir].trap_door );
		return;
	} /* chiude l'if */

	obj = get_obj_here( ch, arg );
	if ( obj )
	{
		/* 'pick object' */
		if ( !obj->container )
		{
			send_to_char( ch, "Non è un contenitore.\r\n" );
			return;
		}

		if ( !HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			send_to_char( ch, "Il contenitore non è chiuso.\r\n" );
			return;
		}

		if ( obj->container->key < 0 )
		{
			send_to_char( ch, "Questo contenitore non può essere scassinato.\r\n" );
			return;
		}

		if ( !HAS_BITINT(obj->container->flags, CONTAINER_LOCKED) )
		{
			send_to_char( ch, "Questo contenitore è già aperto.\r\n" );
			return;
		}

		if ( HAS_BITINT(obj->container->flags, CONTAINER_PICKPROOF) )
		{
			send_to_char( ch, "Fallisco il mio tentativo.\r\n" );
			learn_skell( ch, gsn_pick_lock, FALSE );
			check_for_trap( ch, obj, TRAPFLAG_PICK );
			return;
		}

		split_obj( obj, 1 );
		REMOVE_BITINT( obj->container->flags, CONTAINER_LOCKED );
		send_to_char( ch, "*Click*\r\n" );
		act( AT_ACTION, "$n riesce ad aprire $p.", ch, obj, NULL, TO_ROOM );
		learn_skell( ch, gsn_pick_lock, TRUE );
		check_for_trap( ch, obj, TRAPFLAG_PICK );
		return;
	} /* chiude l'if */

	ch_printf( ch, "Non vedo nessun %s qui.\r\n", arg );
}


DO_RET do_sneak( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	send_to_char( ch, "Cerco di muovermi il più silenziosamente possibile.\r\n" );

	if ( can_use_skill(ch, number_percent(), gsn_sneak) )
	{
		AFFECT_DATA	*aff;

		CREATE( aff, AFFECT_DATA, 1 );
		aff->type      = gsn_sneak;
		aff->duration  = (get_level(ch)/2) * 69;
		aff->location  = APPLY_NONE;
		aff->modifier  = 0;
		aff->bitvector = meb( AFFECT_SNEAK );
		affect_to_char( ch, aff );

		learn_skell( ch, gsn_sneak, TRUE );
	}
	else
		learn_skell( ch, gsn_sneak, FALSE );
}


DO_RET do_hide( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	send_to_char( ch, "Cerco di nascondermi.\r\n" );

	if ( HAS_BIT(ch->affected_by, AFFECT_HIDE) )
		REMOVE_BIT(ch->affected_by, AFFECT_HIDE);

	if ( can_use_skill(ch, number_percent(), gsn_hide ) )
	{
		SET_BIT( ch->affected_by, AFFECT_HIDE );
		learn_skell( ch, gsn_hide, TRUE );
	}
	else
		learn_skell( ch, gsn_hide, FALSE );
}


/*
 * This is an update for the visible command, to make a player become visible.
 * This version handles any form of invisibility/hiding, regardless of
 *	the Slot Number of the skill causing it.
 */
DO_RET do_visible( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	bool		 found = FALSE;

	for ( paf = ch->first_affect;  paf;  paf = paf_next )
	{
		paf_next = paf->next;

		if ( HAS_BIT(paf->bitvector, AFFECT_SNEAK)
		  || HAS_BIT(paf->bitvector, AFFECT_INVISIBLE)
		  || HAS_BIT(paf->bitvector, AFFECT_HIDE) )
		{
			found = TRUE;
			affect_remove( ch, paf );
		}
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_HIDE)
	 ||  HAS_BIT(ch->affected_by, AFFECT_INVISIBLE)
	 ||  HAS_BIT(ch->affected_by, AFFECT_SNEAK) )
	{
		found = TRUE;

		REMOVE_BIT( ch->affected_by, AFFECT_HIDE		);
		REMOVE_BIT( ch->affected_by, AFFECT_INVISIBLE	);
		REMOVE_BIT( ch->affected_by, AFFECT_SNEAK		);
	}

	if ( found )
	{
		act( AT_PLAIN, "Ritorno visibile.", ch, NULL, NULL, TO_CHAR );
		act( AT_PLAIN, "$n ritorna visibile.", ch, NULL, NULL, TO_ROOM );
	}
	else
		ch_printf( ch, "Sono già visibilissim%c.\r\n", gramm_ao(ch) );
}


DO_RET do_aid( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	int		   percent;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Chi dovrei soccorrere?\r\n" );
		return;
	}

	victim = get_char_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non posso aiutare qualcuno che non si trova qui.\r\n" );
		return;
	}

	/* (FF) Pensare se togliere e farla anche sui mob, problema equilibrio mascotte però */
	if ( IS_MOB(victim) )
	{
		send_to_char( ch, "Posso farlo solo su avventurieri.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Non è una cosa che possa farmi da solo adeguatamente.\r\n" );
		return;
	}

	if ( victim->position > POSITION_STUN )
	{
		act( AT_PLAIN, "$N non ha bisogno del mio aiuto per ora.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if ( victim->points_max[POINTS_LIFE] <= -6 )
	{
		act( AT_PLAIN, "Le condizioni di $N sono gravi, il mio aiuto giunge in ritardo.", ch, NULL, victim, TO_CHAR);
		return;
	}

	percent = number_percent() - ( get_curr_attr(ch, ATTR_SPI)/4 - 13 );

	WAIT_STATE( ch, table_skill[gsn_aid]->beats * 3 );	/* (WT) */

	if ( !can_use_skill(ch, percent, gsn_aid) )
	{
		send_to_char( ch, "Il mio tentativo di soccorso fallisce.\r\n" );
		learn_skell( ch, gsn_aid, FALSE );
		return;
	}

	act( AT_SKILL, "Riesco a soccorrere $N!", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n riesce a soccorrere $N!", ch, NULL, victim, TO_NOVICT );
	learn_skell( ch, gsn_aid, TRUE );
	if ( victim->points[POINTS_LIFE] < 1 )
		victim->points[POINTS_LIFE] = 1;

	update_position( victim );
	act( AT_SKILL, "$n viene in mio soccorso!", ch, NULL, victim, TO_VICT );
}


DO_RET do_mount( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_mount]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non credo che sia una buona idea.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Cavalco già una montatura!\r\n" );
		return;
	}

	victim = get_char_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non trovo nulla di simile qui.\r\n" );
		return;
	}

	if ( IS_PG(victim) )
	{
		char x;

		x = gramm_ao( victim );
		ch_printf( ch, "Non credo che %s sarà content%c se tentassi di cavalcarl%c.\r\n",
			victim->name, x, x );
		return;
	}

	if ( !HAS_BIT_ACT(victim, MOB_MOUNTABLE) )
	{
		send_to_char( ch, "Non è una cavalcatura!\r\n" );
		return;
	}

	if ( HAS_BIT_ACT(victim, MOB_MOUNTED) )
	{
		send_to_char( ch, "Il posto è già occupato, non vedi?\r\n" );
		return;
	}

	if ( victim->position < POSITION_STAND )
	{
		send_to_char( ch, "La cavalcatura deve essere in piedi.\r\n" );
		return;
	}

	if ( victim->position == POSITION_FIGHT || victim->fighting )
	{
		send_to_char( ch, "Non riesco a montare la cavalcatura mentre combatte.\r\n" );
		return;
	}

	/* Le creature charmate non possono essere cavalcate da chi non è il master */
	if ( HAS_BIT(victim->affected_by, AFFECT_CHARM) && ch != victim->master )
	{
		send_to_char( ch, "Questa creatura non mi riconosce come suo padrone.\r\n" );
		return;
	}

	if ( victim->level > ch->level )
	{
		ch_printf( ch, "Sono troppo poco espert%c per questa cavalcatura.\r\n", gramm_ao(ch) );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_mount]->beats * 3 );	/* (WT) */

	if ( can_use_skill(ch, number_percent(), gsn_mount ) )
	{
		SET_BIT( victim->act, MOB_MOUNTED );
		ch->mount = victim;
		act( AT_SKILL, "Monto $N.", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n monta su $N.", ch, NULL, victim, TO_NOVICT );
		act( AT_SKILL, "$n monta su di me.", ch, NULL, victim, TO_VICT );
		learn_skell( ch, gsn_mount, TRUE );
		ch->position = POSITION_MOUNT;
	}
	else
	{
		act( AT_SKILL, "Non riesco a montare $N.", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n fallisce nel tentativo di montare $N.", ch, NULL, victim, TO_NOVICT );
		act( AT_SKILL, "$n cerca di montarmi senza successo.", ch, NULL, victim, TO_VICT );
		learn_skell( ch, gsn_mount, FALSE );
	}
}


DO_RET do_dismount( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;

	victim = ch->mount;
	if ( !victim )
	{
		send_to_char( ch, "Non sto montando nulla.\r\n" );
		return;
	}

	if ( IS_PG(victim) )
	{
		send_log( NULL, LOG_BUG, "do_dismount: victim %s è una cavalcatura pg", victim->name );
		ch->mount = NULL;
		ch->position = POSITION_STAND;
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_mount]->beats * 3 );	/* (WT) */
	if ( can_use_skill(ch, number_percent(), gsn_mount) )
	{
		act( AT_SKILL, "Scendo giù da $N.", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n scende giù da $N.", ch, NULL, victim, TO_NOVICT );
		act( AT_SKILL, "$n scende giù.", ch, NULL, victim, TO_VICT );
		REMOVE_BIT( victim->act, MOB_MOUNTED );
		ch->mount = NULL;
		ch->position = POSITION_STAND;
		learn_skell( ch, gsn_mount, TRUE );
	}
	else
	{
		act( AT_SKILL, "Cado a terra cercando di scendere da $N.  Ouch!", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n cade a terra cercando di scendere da $N.. che figura..", ch, NULL, victim, TO_NOVICT );
		act( AT_SKILL, "$n cade a terra cercando di scendere da me.", ch, NULL, victim, TO_VICT );
		learn_skell( ch, gsn_mount, FALSE );
		REMOVE_BIT( victim->act, MOB_MOUNTED );
		ch->mount = NULL;
		ch->position = POSITION_SIT;
		global_retcode = damage( ch, ch, 1, TYPE_UNDEFINED );
	}
}


/**************************************************************************/


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int		parry_chance;

	if ( !is_awake(victim) )
		return FALSE;

	if ( IS_MOB(victim) && !HAS_BIT(victim->defenses, DEFENSE_PARRY) )
		return FALSE;

	if ( IS_MOB(victim) )
		parry_chance = UMIN( 60, get_level(victim) );
	else
	{
		if ( get_eq_char(victim, WEARLOC_WIELD) == NULL )
			return FALSE;
		parry_chance = (int) ( knows_skell(victim, gsn_parry) / sysdata.mod_parry );
	}

	/* Put in the call to chance() to allow penalties for misaligned clannies. */
	if ( parry_chance != 0 && victim->morph )
		parry_chance += victim->morph->parry;

	if ( !chance(victim, parry_chance + get_level(victim)/2 - get_level(ch)/2) )
	{
		learn_skell( victim, gsn_parry, FALSE );
		return FALSE;
	}

	if ( get_eq_char(victim, WEARLOC_WIELD) == NULL )
	{
		act( AT_SKILL, "Devio l'attacco di $n.",  ch, NULL, victim, TO_VICT );
		act( AT_SKILL, "$N devia il mio attacco.", ch, NULL, victim, TO_CHAR );
	}
	else
	{
		act( AT_SKILL, "Blocco con la mia arma l'attacco di $n.",  ch, NULL, victim, TO_VICT );
		act( AT_SKILL, "$N blocca con l'arma il mio attacco.", ch, NULL, victim, TO_CHAR );
	}

#ifdef T2_MSP
	if ( IS_PG(victim) && HAS_BIT(victim->pg->protocols, PROTOCOL_COMBAT) && number_range(0, 3) == 0 )
		send_audio( victim->desc, "parry.wav", TO_CHAR );
#endif

	learn_skell( victim, gsn_parry, TRUE );
	return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int		dodge_chance;

	if ( !is_awake(victim) )
		return FALSE;

	if ( IS_MOB(victim) && !HAS_BIT(victim->defenses, DEFENSE_DODGE) )
		return FALSE;

	if ( IS_MOB(victim) )
		dodge_chance  = UMIN( 60, get_level(victim) );
	else
		dodge_chance  = (int) (knows_skell(victim, gsn_dodge) / sysdata.mod_dodge );

	if ( dodge_chance != 0 && victim->morph )
		dodge_chance += victim->morph->dodge;

	/* Consider luck as a factor */
	if ( !chance(victim, dodge_chance + get_level(victim)/2 - get_level(ch)/2) )
	{
		learn_skell( victim, gsn_dodge, FALSE );
		return FALSE;
	}

	act( AT_SKILL, "Schivo l'attacco di $n.", ch, NULL, victim, TO_VICT );
	act( AT_SKILL, "$N schiva il mio attacco.", ch, NULL, victim, TO_CHAR );

	learn_skell( victim, gsn_dodge, TRUE );
	return TRUE;
}


bool check_tumble( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int		tumble_chance;

	/* (FF) al posto del controllo sul ladro devo fare un controllo sulle classi categoria ladro */
	if ( victim->class != CLASS_THIEF || !is_awake(victim) )
		return FALSE;

	if ( IS_PG(victim) && !victim->pg->learned_skell[gsn_tumble] > 0 )
		return FALSE;

	if ( IS_MOB(victim) )
		tumble_chance = UMIN( 60, get_level(victim) );
	else
		tumble_chance = (int) ( knows_skell(victim, gsn_tumble) / sysdata.mod_tumble
						+ (get_curr_attr(victim, ATTR_AGI)/4 - 13) );

	if ( tumble_chance != 0 && victim->morph )
		tumble_chance += victim->morph->tumble;

	if ( !chance(victim, tumble_chance + get_level(victim)/2 - get_level(ch)/2) )
		return FALSE;

	act( AT_SKILL, "Evito l'attacco di $n con un salto su di un lato.", ch, NULL, victim, TO_VICT );
	act( AT_SKILL, "$N evita con un salto laterale il mio attacco.", ch, NULL, victim, TO_CHAR );

	learn_skell( victim, gsn_tumble, TRUE );
	return TRUE;
}


DO_RET do_poison_weapon( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *pobj;
	OBJ_DATA *wobj;
	char	  arg[MIL];
	int		  percent;

	if ( IS_PG(ch) && table_skill[gsn_poison_weapon]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho le conoscenze giuste per farlo adeguatamente.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Cosa dovrei avvelenare?\r\n" );
		return;
	}

	if ( ch->fighting )
	{
		send_to_char( ch, "Non mentre sto combattendo.\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( !(obj = get_obj_carry(ch, arg, TRUE)) )
	{
		send_to_char( ch, "Non ho quest'arma.\r\n" );
		return;
	}

	if ( obj->type != OBJTYPE_WEAPON )
	{
		send_to_char( ch, "Non si tratta di un'arma.\r\n" );
		return;
	}

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_POISONED) )
	{
		send_to_char( ch, "Quest'arma è già avvelenata.\r\n" );
		return;
	}

	/* Il check sul timer è per via che ci sarebbe conflitto di informazioni sull'oggetto avvellenato poi */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_CLANOBJECT) || obj->timer )
	{
		send_to_char( ch, "Su quest'arma il veleno scivola via.\r\n" );
		return;
	}

	/* Now we have a valid weapon... check to see if we have the powder. */
	for ( pobj = ch->first_carrying;  pobj;  pobj = pobj->next_content )
	{
		if ( pobj->pObjProto->vnum == VNUM_OBJ_BLACK_POWDER )
			break;
	}

	if ( !pobj )
	{
		send_to_char( ch, "Mi manca il veleno giusto.\r\n" );
		return;
	}

	/* Okay, we have the powder...do we have water? */
	for ( wobj = ch->first_carrying;  wobj;  wobj = wobj->next_content )
	{
		if ( wobj->type == OBJTYPE_DRINK_CON
		  && wobj->drinkcon->curr_quant >  0
		  && wobj->drinkcon->liquid == 0 )	/* acqua? */
		{
			break;
		}
	}

	if ( !wobj )
	{
		send_to_char( ch, "Mi serve dell'acqua per miscelare il veleno.\r\n" );
		return;
	}

	/* And does the thief have steady enough hands? */
	if ( IS_PG(ch) && (get_curr_attr(ch, ATTR_AGI) < 60 || ch->pg->condition[CONDITION_DRUNK] > 0) )
	{
		send_to_char( ch, "Verso tutto per terra! Disastro! Uff..\r\n" );
		return;
	}

	WAIT_STATE( ch, table_skill[gsn_poison_weapon]->beats * 3 );	/* (WT) */

	percent = number_percent() - ( get_curr_attr(ch, ATTR_COC)/4 - 14 );

	/* Check the skill percentage */
	split_obj( pobj, 1 );
	split_obj( wobj, 1 );
	if ( !can_use_skill(ch, percent, gsn_poison_weapon) )
	{
		set_char_color( AT_RED, ch );
		send_to_char( ch, "Argh! Me lo sono versato addosso!\r\n" );

		set_char_color( AT_GREY, ch );
		act( AT_RED, "$n si fa cadere un miscuglio nero addosso. Che imbranat$x!", ch, NULL, NULL, TO_ROOM );

		damage( ch, ch, get_level(ch)/2, gsn_poison_weapon );
		learn_skell( ch, gsn_poison_weapon, FALSE );

		free_object( pobj );
		free_object( wobj );

		return;
	}
	split_obj( obj, 1 );

	/* Well, I'm tired of waiting.  Are you? */
	act(AT_RED, "Mescolo sapientemente $p con $P ottenendo un veleno micidiale!", ch, pobj, wobj, TO_CHAR );
	act(AT_RED, "$n mescola con dovizia $p con $P ottenendo una mistura nera.",ch, pobj, wobj, TO_ROOM );
	act(AT_GREEN, "Verso il veleno su $p che brilla lievemente.",ch, obj, NULL, TO_CHAR  );
	act(AT_GREEN, "$n versa la mistura su $p che brilla lievemente.",ch, obj, NULL, TO_ROOM  );
	SET_BIT( obj->extra_flags, OBJFLAG_POISONED );
	obj->cost *= 2;

	/* Set an object timer */
	obj->timer = UMIN( obj->level/2, get_level(ch)/2 );

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_BLESS) )
		obj->timer *= 2;

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
		obj->timer *= 2;

	/* WHAT?  All of that, just for that one bit?  How lame. ;) */
	act( AT_BLUE, "Il veleno pervade $p danneggiando irreparabilmente l'oggetto.", ch, wobj, NULL, TO_CHAR );
	act( AT_BLUE, "Degli strani rumorini metallici provengono da $p.", ch, wobj, NULL, TO_ROOM );

	free_object( pobj );
	free_object( wobj );

	learn_skell( ch, gsn_poison_weapon, TRUE );
}


DO_RET do_scribe( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *scroll;
	char	  buf1[MSL];
	char	  buf2[MSL];
	char	  buf3[MSL];
	int		  sn;
	int		  mana;

	if ( IS_MOB(ch) )
		return;

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_scribe]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho sufficiente abilità per farlo.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) || strcmp(argument, "") )
	{
		send_to_char( ch, "Cosa dovrei scrivere?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	sn = find_spell( ch, argument, TRUE );
	if ( sn < 0 )
	{
		send_to_char( ch, "Non ho ancora imparato questo incantesimo.\r\n" );
		return;
	}

	if ( table_skill[sn]->spell_fun == spell_null )
	{
		send_to_char( ch, "Non è un incantesimo!\r\n" );
		return;
	}

	if ( SPELL_FLAG(table_skill[sn], SKELL_NOSCRIBE) )
	{
		send_to_char( ch, "Non posso trascrivere questo incantesimo.\r\n" );
		return;
	}

	if ( IS_MOB(ch) )
		mana = 0;
	else
		mana = UMAX( table_skill[sn]->min_mana, 100 / UMAX(1, (2 + get_level(ch) - table_skill[sn]->skill_level[ch->class])) );

	mana *= 5;

	if ( IS_PG(ch) && ch->points[POINTS_MANA] < mana && !stop_lowering_points(ch, POINTS_MANA) )
	{
		send_to_char( ch, "Ho bisogno di un maggior quantitativo di Mana.\r\n" );
		return;
	}

	scroll = get_eq_char( ch, WEARLOC_HOLD );
	if ( !scroll )
	{
		send_to_char( ch, "Ho bisogno di un papiro per incantesimi bianco per poter scrivere.\r\n" );
		return;
	}

	if ( scroll->pObjProto->vnum != VNUM_OBJ_SCROLL_SCRIBING )
	{
		send_to_char( ch, "Ho bisogno di un papiro per incantesimi bianco per poter scrivere.\r\n" );
		return;
	}

	if ( (scroll->scroll->sn1 != -1)
	  && (VNUM_OBJ_SCROLL_SCRIBING == scroll->pObjProto->vnum) )
	{
		send_to_char( ch, "Questo incantesimo è già stato trascritto.\r\n");
		return;
	}

	if ( !process_components(ch, sn) )
	{
		learn_skell( ch, gsn_scribe, FALSE );
		ch->points[POINTS_MANA] -= mana / 2;
		return;
	}

	if ( !can_use_skill(ch, number_percent(), gsn_scribe) )
	{
		set_char_color ( AT_MAGIC, ch );
		send_to_char( ch, "Fallisco la scrittura.\r\n" );
		learn_skell( ch, gsn_scribe, FALSE );
		ch->points[POINTS_MANA] -= mana / 2;
		return;
	}

	scroll->scroll->sn1 = sn;
	scroll->scroll->level = get_level( ch );	/* corretto, senza /2 */
	sprintf( buf1, "Incantesimo %s", table_skill[sn]->name );
	DISPOSE( scroll->short_descr );
	scroll->short_descr = str_dup( aoran(buf1) );

	sprintf( buf2, "Una lucente pergamena con l'iscrizione '%s' giace qui per terra.", table_skill[sn]->name );

	DISPOSE( scroll->long_descr );
	scroll->long_descr = str_dup( buf2 );

	sprintf( buf3, "Trascrizione dell'incantesimo di %s", table_skill[sn]->name );
	DISPOSE( scroll->name );
	scroll->name = str_dup( buf3 );

	act( AT_MAGIC, "$n trascrive magicamente l'incantesimo di $p.", ch,scroll, NULL, TO_ROOM );
	act( AT_MAGIC, "Trascrivo magicamente l'incantesimo di $p.", ch,scroll, NULL, TO_CHAR );

	learn_skell( ch, gsn_scribe, TRUE );
	ch->points[POINTS_MANA] -= mana;
}


DO_RET do_brew( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *potion;
	OBJ_DATA *fire;
	int		  sn;
	char	  buf1[MSL];
	char	  buf2[MSL];
	char	  buf3[MSL];
	int		  mana;
	bool	  found;

	if ( IS_MOB(ch) )
		return;

	if ( IS_PG(ch) && get_level(ch) < table_skill[gsn_brew]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho l'abilità necessaria per fare una cosa del genere.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Da cosa dovrei ottenere una pozione?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	sn = find_spell( ch, argument, TRUE );
	if ( sn < 0 )
	{
		send_to_char( ch, "Non conosco questo incantesimo.\r\n" );
		return;
	}

	if ( table_skill[sn]->spell_fun == spell_null )
	{
		send_to_char( ch, "Non è un incantesimo!\r\n" );
		return;
	}

	if ( SPELL_FLAG(table_skill[sn], SKELL_NOBREW) )
	{
		send_to_char( ch, "Non posso mettere in pozione questo incantesimo.\r\n" );
		return;
	}

	if ( IS_MOB(ch) )
		mana = 0;
	else
		mana = UMAX( table_skill[sn]->min_mana, 100 / UMAX(1, (2 + get_level(ch) - table_skill[sn]->skill_level[ch->class])) );

	mana *= 4;

	if ( IS_PG(ch) && ch->points[POINTS_MANA] < mana && !stop_lowering_points(ch, POINTS_MANA) )
	{
		send_to_char( ch, "Mi serve un po' di Mana in più.\r\n" );
		return;
	}

	found = FALSE;

	for ( fire = ch->in_room->first_content;  fire;  fire = fire->next_content )
	{
		if ( fire->type == OBJTYPE_FIRE )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char( ch, "Ho bisogno di un fuoco per ottenere la pozione.\r\n" );
		return;
	}

	potion = get_eq_char( ch, WEARLOC_HOLD );
	if ( !potion )
	{
		send_to_char( ch, "Ho bisogno di una fiala in cui mettere la pozione.\r\n" );
		return;
	}

	if ( potion->pObjProto->vnum != VNUM_OBJ_FLASK_BREWING )
	{
		send_to_char( ch, "Ho bisogno di una fiala in cui mettere la pozione.\r\n" );
		return;
	}

	if ( (potion->potion->sn1 != -1)
	  && (potion->pObjProto->vnum == VNUM_OBJ_FLASK_BREWING) )
	{
		send_to_char( ch, "Questo contenitore non è adatto per la mia pozione.\r\n" );
		return;
	}

	if ( !process_components(ch, sn) )
	{
		learn_skell( ch, gsn_brew, FALSE );
		ch->points[POINTS_MANA] -= mana / 2;
		return;
	}

	if ( !can_use_skill(ch, number_percent(), gsn_brew ) )
	{
		set_char_color ( AT_MAGIC, ch );
		send_to_char( ch, "Fallisco il tentativo per la pozione.\r\n" );
		learn_skell( ch, gsn_brew, FALSE );
		ch->points[POINTS_MANA] -= mana / 2;
		return;
	}

	potion->potion->sn1 = sn;
	potion->potion->level = get_level(ch);	/* corretto, senza /2 */
	sprintf( buf1, "Pozione di %s", table_skill[sn]->name );
	DISPOSE( potion->short_descr );
	potion->short_descr = str_dup( aoran(buf1) );

	sprintf( buf2, "Una strana fiala con l'etichetta '%s' brilla per terra.", table_skill[sn]->name );

	DISPOSE( potion->long_descr );
	potion->long_descr = str_dup( buf2 );

	sprintf( buf3, "Creazione della pozione di %s", table_skill[sn]->name );
	DISPOSE( potion->name );
	potion->name = str_dup( buf3 );

	act( AT_MAGIC, "$n crea una pozione di $p.",   ch,potion, NULL, TO_ROOM );
	act( AT_MAGIC, "Riesco nel tentativo di creare la pozione di $p.",   ch,potion, NULL, TO_CHAR );

	learn_skell( ch, gsn_brew, TRUE );
	ch->points[POINTS_MANA] -= mana;
}


DO_RET do_circle( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	char	   arg[MIL];
	int		   percent;

	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Qualcosa frena la mia forza di volontà.\r\n" );
		return;
	}

	if ( ch->mount )
	{
		send_to_char( ch, "Non posso farlo mentre monto una cavalcatura.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Chi dovrebbe essere la mia vittima?\r\n" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non trovo nessuno del genere qui in giro.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Dovrei farmelo a me stesso?!\r\n" );
		return;
	}

	if ( is_safe(ch, victim, TRUE) )
		return;

	if ( (obj = get_eq_char(ch, WEARLOC_WIELD)) == NULL
	  || (obj->weapon->damage1 != DAMAGE_PIERCE && obj->weapon->damage1 != DAMAGE_STAB) )
	{
		send_to_char( ch, "Mi serve un'arma adatta: un coltello o una daga.\r\n" );
		return;
	}

	if ( !ch->fighting )
	{
		send_to_char( ch, "Non posso farlo mentre l'avversario mi sta fronteggiando.\r\n" );
		return;
	}

	if ( !victim->fighting )
	{
		send_to_char( ch, "Non posso farlo contro qualcuno che non sta combattendo.\r\n" );
		return;
	}

	/* (FF) possibile parte per la spacetofight? */
	if ( victim->num_fighting < 2 )
	{
		act( AT_PLAIN, "Non posso farlo, troppa confusione.", ch, NULL, victim, TO_CHAR );
		return;
	}

	percent = number_percent()
			- ( (get_curr_attr(ch, ATTR_SPE)/4 + get_curr_attr(ch, ATTR_AGI)/4) - 16 )
			+ ( (get_curr_attr(victim, ATTR_REF)/4 + get_curr_sense(victim, SENSE_SIXTH)/4) - 13 );	/* Sesto senso al posto di concentrazione, difatti il sixth è più una concentrazione dell'inconscio */

	check_attacker( ch, victim );

	WAIT_STATE( ch, table_skill[gsn_circle]->beats * 3 );	/* (WT) */

	if ( can_use_skill( ch, percent, gsn_circle ) )
	{
		learn_skell( ch, gsn_circle, TRUE );
		WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
		global_retcode = multi_hit( ch, victim, gsn_circle );
		is_illegal_pk( ch, victim );
	}
	else
	{
		learn_skell( ch, gsn_circle, FALSE );
		WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
		global_retcode = damage( ch, victim, 0, gsn_circle );
	}
}


/*
 * Berserk and HitAll.
 */
DO_RET do_berserk( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA *aff;
	int			 percent;

	if ( !ch->fighting )
	{
		send_to_char( ch, "Non sto combattendo!\r\n" );
		return;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_BERSERK) )
	{
		send_to_char( ch, "La rabbia scorre già furiosa nelle mie vene!\r\n" );
		return;
	}

	percent = knows_skell( ch, gsn_berserk );
	WAIT_STATE( ch, table_skill[gsn_berserk]->beats * 3 );	/* (WT) */
	if ( !chance(ch, percent) )
	{
		send_to_char( ch, "Non riesco a richiamare la mia collera.\r\n" );
		learn_skell( ch, gsn_berserk, FALSE );
		return;
	}

	CREATE( aff, AFFECT_DATA, 1 );

	aff->type = gsn_berserk;

	/* Hmmm.. 30-60 seconds at level 100.. good enough for most mobs, and
	 *	if not they can always go berserk again.. shrug.. maybe even too high.
	 */
	aff->duration = number_range( get_level(ch)/10 + 1, get_level(ch)/5 + 1 );

	/* Hmm.. you get stronger when yer really enraged.. mind over matter type thing.. */
	aff->location = APPLY_STR;
	aff->modifier = 1;
	aff->bitvector = meb( AFFECT_BERSERK );
	affect_to_char( ch, aff );

	send_to_char( ch, "Sento la rabbia pervadermi lentamente.. Già comincio a perdere il controllo!\r\n" );
	learn_skell( ch, gsn_berserk, TRUE );
}


DO_RET do_hitall( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int		   nvict = 0;
	int		   nhit  = 0;
	int		   percent;

	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		send_to_char( ch, "&BUna Forza mi impedisce di farlo.\r\n" );
		return;
	}

	if ( !ch->in_room->first_person )
	{
		send_to_char( ch, "Non vedo nessun altro qui!\r\n" );
		return;
	}

	percent = knows_skell( ch, gsn_hitall );
	for ( vch = ch->in_room->first_person;  vch;  vch = vch_next )
	{
		vch_next = vch->next_in_room;

		if ( is_same_group(ch, vch) )	continue;
		if ( !is_legal_kill(ch, vch) )	continue;
		if ( !can_see(ch, vch) )		continue;
		if ( is_safe(ch, vch, TRUE) )	continue;

		if ( ++nvict > get_level(ch) / 10 )	/* (LV) (TT) dovrebbe essere ok, era /5, però... */
			break;

		is_illegal_pk(ch, vch);
		if ( chance(ch, percent) )
		{
			nhit++;
			global_retcode = one_hit( ch, vch, TYPE_UNDEFINED );
		}
		else
		{
			global_retcode = damage( ch, vch, 0, TYPE_UNDEFINED );
		}

		/* Fireshield, etc. could kill ch too.. :>.. */
		if ( global_retcode == rCHAR_DIED || global_retcode == rBOTH_DIED || char_died(ch) )
			return;
	}

	if ( !nvict )
	{
		send_to_char( ch, "Non vedo così tanta gente qui!\r\n" );
		return;
	}

	ch->points[POINTS_MOVE] = UMAX( 0, ch->points[POINTS_MOVE] - (nvict*3) + nhit );
	if ( nhit )
		learn_skell( ch, gsn_hitall, TRUE );
	else
		learn_skell( ch, gsn_hitall, FALSE );
}


DO_RET do_scan( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*was_in_room;
	EXIT_DATA	*pexit;
	int			 dir = -1;
	int			 dist;
	int			 max_dist = 8;

	set_char_color( AT_ACTION, ch );

	if ( HAS_BIT(ch->affected_by, AFFECT_BLIND) )
	{
		send_to_char( ch, "Troppo difficile, non vedo nulla!\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "In quale direzione guardare?\r\n" );
		return;
	}

	dir = get_door( argument );
	if ( dir == -1 )
	{
		send_to_char( ch, "In quale direzione guardare?\r\n" );
		return;
	}

	was_in_room = ch->in_room;
	act( AT_GREY, "Guardo $t.", ch, table_dir[dir].to, NULL, TO_CHAR );
	act( AT_GREY, "$n guarda $t.", ch, table_dir[dir].to, NULL, TO_ROOM );

	if ( !can_use_skill(ch, number_percent(), gsn_scan) )
	{
		act( AT_GREY, "Smetto di guardare $t poiché la vista comincia a sfocare.", ch, table_dir[dir].to, NULL, TO_CHAR );
		learn_skell( ch, gsn_scan, FALSE );
		return;
	}

/* (FF) malattia vampirica */
	if ( ch->race == RACE_VAMPIRE )
	{
		if ( calendar.hour < 21 && calendar.hour > 5 )
		{
			send_to_char( ch, "Non riesco a vedere chiaramente a causa della troppa luce.\r\n" );
			max_dist = 1;
		}
	}

	pexit = get_exit( ch->in_room, dir );
	if ( !pexit )
	{
		act( AT_GREY, "Riesco a vedere $t.", ch, table_dir[dir].to, NULL, TO_CHAR );
		return;
	}

	if ( get_level(ch) < LVL_LEGEND		  )	--max_dist;
	if ( get_level(ch) < LVL_LEGEND * 4/5 )	--max_dist;
	if ( get_level(ch) < LVL_LEGEND * 3/5 )	--max_dist;

	for ( dist = 1;  dist <= max_dist;  )
	{
		if ( HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			if ( HAS_BIT(pexit->flags, EXIT_SECRET) || HAS_BIT(pexit->flags, EXIT_DIG) )
				act( AT_GREY, "La mia vista $t è bloccata da un muro.", ch, table_dir[dir].to, NULL, TO_CHAR );
			else
				act( AT_GREY, "La mia vista $t è bloccata da una porta.", ch, table_dir[dir].to, NULL, TO_CHAR );

			break;
		}

		char_from_room( ch );
		char_to_room( ch, pexit->to_room );
		set_char_color( AT_RMNAME, ch );
		ch_printf( ch, "%s\r\n", ch->in_room->name );
#ifdef T2_MXP
		show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE, eItemNothing );
#else
		show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
#endif
		show_char_to_char( ch->in_room->first_person, ch );

		switch( ch->in_room->sector )
		{
		  default:
			dist++;
			break;

		  case SECTOR_AIR:
			if ( number_percent() < 80 )
				dist++;
			break;

		  case SECTOR_INSIDE:
		  case SECTOR_FIELD:
		  case SECTOR_UNDERGROUND:
			dist++;
			break;

		  case SECTOR_FOREST:
		  case SECTOR_CITY:
		  case SECTOR_DESERT:
		  case SECTOR_HILLS:
			dist += 2;
			break;

		  case SECTOR_WATER_SWIM:
		  case SECTOR_WATER_NOSWIM:
			dist += 3;
			break;

		  case SECTOR_MOUNTAIN:
		  case SECTOR_UNDERWATER:
		  case SECTOR_OCEANFLOOR:
			dist += 4;
			break;
		} /* chiude lo switch */

		if ( dist >= max_dist )
		{
			act( AT_GREY, "La mia visione sfoca a causa della distanza e non riesco a vedere più lontano di così $t.", ch, table_dir[dir].to, NULL, TO_CHAR );
			break;
		}

		pexit = get_exit( ch->in_room, dir );
		if ( !pexit )
		{
			act( AT_GREY, "La mia visuale $t è bloccata da un muro.", ch, table_dir[dir].to, NULL, TO_CHAR );
			break;
		}
	} /* chiude il for */

	char_from_room( ch );
	char_to_room( ch, was_in_room );
	learn_skell( ch, gsn_scan, TRUE );
}


/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.
 */
CHAR_DATA *scan_for_victim( CHAR_DATA *ch, EXIT_DATA *pexit, char *name )
{
	CHAR_DATA	*victim;
	ROOM_DATA	*was_in_room;
	int			 dist;
	int			 dir;
	int			 max_dist = 8;

	if ( HAS_BIT(ch->affected_by, AFFECT_BLIND) || !pexit )
		return NULL;

	was_in_room = ch->in_room;

#ifdef T2_VAMPIRE
	/* (FF) malattia vampirica */
	if ( ch->race == RACE_VAMPIRE && calendar.hour < 21 && calendar.hour > 5 )
		max_dist = 1;
#endif

	if ( get_level(ch) < LVL_LEGEND		  )	--max_dist;
	if ( get_level(ch) < LVL_LEGEND * 4/5 )	--max_dist;
	if ( get_level(ch) < LVL_LEGEND * 3/5 )	--max_dist;

	for ( dist = 1;  dist <= max_dist;  )
	{
		if ( HAS_BIT(pexit->flags, EXIT_CLOSED) )
			break;

		char_from_room( ch );
		char_to_room( ch, pexit->to_room );

		victim = get_char_room( ch, name, TRUE );
		if ( victim )
		{
			char_from_room( ch );
			char_to_room(ch, was_in_room);
			return victim;
		}

		switch( ch->in_room->sector )
		{
		  default:
			dist++;
			break;

		  case SECTOR_AIR:
			if ( number_percent() < 80 )
				dist++;
			break;

		  case SECTOR_INSIDE:
		  case SECTOR_FIELD:
		  case SECTOR_UNDERGROUND:
			dist++;
			break;

	      case SECTOR_FOREST:
	      case SECTOR_CITY:
	      case SECTOR_DESERT:
	      case SECTOR_HILLS:
			dist += 2;
			break;

	      case SECTOR_WATER_SWIM:
	      case SECTOR_WATER_NOSWIM:
			dist += 3;
			break;

	      case SECTOR_MOUNTAIN:
	      case SECTOR_UNDERWATER:
	      case SECTOR_OCEANFLOOR:
			dist += 4;
			break;
	}

	if ( dist >= max_dist )
		break;

	dir = pexit->vdir;
	if ( (pexit = get_exit(ch->in_room, dir)) == NULL )
		break;
	}

	char_from_room( ch );
	char_to_room( ch, was_in_room );

	return NULL;
}


/*
 * Search inventory for an appropriate projectile to fire
 * Also search open quivers
 */
OBJ_DATA *find_projectile( CHAR_DATA *ch, int type )
{
	OBJ_DATA	*obj;
	OBJ_DATA	*obj2;

	for ( obj = ch->last_carrying;  obj;  obj = obj->prev_content )
	{
		if ( !can_see_obj(ch, obj, TRUE) )
			continue;

		if ( obj->type == OBJTYPE_QUIVER && !HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
		{
			for ( obj2 = obj->last_content;  obj2;  obj2 = obj2->prev_content )
			{
				if ( obj2->type != OBJTYPE_PROJECTILE )
					continue;
				if ( obj2->weapon->damage1 == type )
					return obj2;
			}
		}

		if ( obj->type == OBJTYPE_PROJECTILE && obj->weapon->damage1 == type )
			return obj;
	}

	return NULL;
}


/*
 * Perform the actual attack on a victim
 */
ch_ret ranged_got_target( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon, OBJ_DATA *projectile,
	int dist, int dt, char *stxt, int color )
{
	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
	{
		/* safe room, bubye projectile */
		if ( projectile )
		{
			ch_printf( ch, "Il mio %s viene distrutto da una forza misteriosa.", myobj(projectile) );
			act( color, "Una forza misteriosa colpisce con un fulmine $p!", ch, projectile, NULL, TO_ROOM );
			free_object( projectile );
		}
		else
		{
			ch_printf(ch, "Il mio %s viene distrutto da una forza misteriosa.", stxt );
			act( color, "Una forza miesteriosa colpisce con un fulmine $p!", ch, aoran(stxt), NULL, TO_ROOM );
		}
		return rNONE;
	}

	if ( HAS_BIT_ACT(victim, MOB_SENTINEL) && ch->in_room != victim->in_room )
	{
		/*
		 * Letsee, if they're high enough.. attack back with fireballs
		 * long distance or maybe some minions... go herne! heh..
		 * For now, just always miss if not in same room
		 */
		if ( projectile )
		{
			learn_skell( ch, gsn_missile_weapons, FALSE );

			/* 50% chance of projectile getting lost */
			if ( number_percent() < 50 )
				free_object(projectile);
			else
			{
				if ( projectile->in_obj )
					obj_from_obj(projectile);
				if ( projectile->carried_by )
					obj_from_char(projectile);
				obj_to_room(projectile, victim->in_room);
			}
		}
		return damage( ch, victim, 0, dt );
	}

	if ( number_percent() > 50 || (projectile && weapon
	  && can_use_skill(ch, number_percent(), gsn_missile_weapons)) )
	{
		if ( projectile )
			global_retcode = projectile_hit( ch, victim, weapon, projectile, dist );
		else
			global_retcode = spell_attack( dt, get_level(ch)/2, ch, victim );
	}
	else
	{
		learn_skell( ch, gsn_missile_weapons, FALSE );
		global_retcode = damage( ch, victim, 0, dt );

		if ( projectile )
		{
			/* 50% chance of getting lost */
			if ( number_percent() < 50 )
				free_object(projectile);
			else
			{
				if ( projectile->in_obj )
					obj_from_obj(projectile);
				if ( projectile->carried_by )
					obj_from_char(projectile);
				obj_to_room(projectile, victim->in_room);
			}
		}
	}

	return global_retcode;
}


/*
 * Generic use ranged attack function.
 */
ch_ret ranged_attack( CHAR_DATA *ch, char *argument, OBJ_DATA *weapon, OBJ_DATA *projectile, int dt, int range )
{
	CHAR_DATA	*victim, *vch;
	EXIT_DATA	*pexit;
	ROOM_DATA	*was_in_room;
	SKILL_DATA	*skill = NULL;
	char		*stxt = "esplosione d'energia";
	char		 arg[MIL];
	char		 arg1[MIL];
	char		 temp[MIL];
	char		 buf[MSL];
	int			 color = AT_GREY;
	int			 dir = -1;
	int			 dist = 0;
	int			 rdir;		/* direzione inversa */
	int			 count;

	if ( VALID_STR(argument) && argument[0] == '\'' )
	{
		one_argument( argument, temp );
		argument = temp;
	}

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg1 );

	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Dove e a chi?\r\n" );
		return rNONE;
	}

	victim = NULL;

	/* get an exit or a victim */
	pexit = find_door( ch, arg, TRUE );
	if ( !pexit )
	{
		victim = get_char_room( ch, arg, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "In che direzione dovrei mirare?\r\n" );
			return rNONE;
		}
		else if ( who_fighting(ch) == victim )
		{
			send_to_char( ch, "E' troppo vicino per un simile tipo di attacco!\r\n" );
			return rNONE;
		}
	}
	else
	{
		dir = pexit->vdir;
	}

	/* check for ranged attacks from private rooms, etc */
	if ( !victim && ch->in_room->tunnel > 0 )
	{
		count = 0;
		for ( vch = ch->in_room->first_person;  vch;  vch = vch->next_in_room )
			++count;

		if ( count >= ch->in_room->tunnel )
		{
			send_to_char( ch, "Sono in un ambiente troppo stretto per un attacco simile!\r\n" );
			return rNONE;
		}
	}

	if ( is_valid_sn(dt) )
		skill = table_skill[dt];

	if ( pexit && !pexit->to_room )
	{
		send_to_char( ch, "E come potrei farlo? Trapassando il muro?!\r\n" );
		return rNONE;
	}

	/* Check for obstruction */
	if ( pexit && HAS_BIT(pexit->flags, EXIT_CLOSED) )
	{
		if ( HAS_BIT(pexit->flags, EXIT_SECRET) || HAS_BIT(pexit->flags, EXIT_DIG) )
			send_to_char( ch, "E come potrei farlo? Trapassando il muro?!\r\n" );
		else
			send_to_char( ch, "E come potrei farlo? Trapassando la porta?!\r\n" );

		return rNONE;
	}

	vch = NULL;
	if ( pexit && VALID_STR(arg1) )
	{
		vch = scan_for_victim( ch, pexit, arg1 );
		if ( !vch )
		{
			send_to_char( ch, "Non riesco a vedere il mio bersaglio.\r\n" );
			return rNONE;
		}

		/* Don't allow attacks on mobs that are in a no-missile room */
		if ( HAS_BIT(vch->in_room->flags, ROOM_NOMISSILE) )
		{
			send_to_char( ch, "Non riesco ad avere una traiettoria pulita.\r\n" );
			return rNONE;
		}

#ifdef T2_ALFA
		/* Can't properly target someone heavily in battle */
		if ( space_to_fight(ch, vch) == FALSE )
		{
			send_to_char( ch, "C'è troppa confusione perché io possa avere una traiettoria pulita.\r\n" );
			return rNONE;
		}
#endif
	}

	if ( vch && is_safe(ch, vch, TRUE) )
		return rNONE;

	was_in_room = ch->in_room;

	if ( projectile )
	{
		split_obj( projectile, 1 );

		if ( pexit )
		{
			if ( weapon )
			{
				act( AT_GREY, "Colpisco con $p $T.", ch, projectile, table_dir[dir].to, TO_CHAR );
				act( AT_GREY, "$n colpisce con $p $T.", ch, projectile, table_dir[dir].to, TO_ROOM );
			}
			else
			{
				act( AT_GREY, "Lancio $p $T.", ch, projectile, table_dir[dir].to, TO_CHAR );
				act( AT_GREY, "$n lancia $p $T.", ch, projectile, table_dir[dir].to, TO_ROOM );
			}
		}
		else
		{
			if ( weapon )
			{
				act( AT_GREY, "Centro $N con $p.", ch, projectile, victim, TO_CHAR );
				act( AT_GREY, "$n centra $N con $p.", ch, projectile, victim, TO_NOVICT );
				act( AT_GREY, "$n mi centra con $p!", ch, projectile, victim, TO_VICT );
			}
			else
			{
				act( AT_GREY, "Lancio $p contro $N.", ch, projectile, victim, TO_CHAR );
				act( AT_GREY, "$n lancia $p contro $N.", ch, projectile, victim, TO_NOVICT );
				act( AT_GREY, "$n mi lancia $p contro!", ch, projectile, victim, TO_VICT );
			}
		}
	}
	else if ( skill )
	{
		if ( VALID_STR(skill->noun_damage) )
			stxt = skill->noun_damage;
		else
			stxt = skill->name;

		/* A plain "spell" flying around seems boring */
		if ( is_name(stxt, "incantesimo spell") )	/* (TT) vedere se era il caso di aggiungere la str_cmp tradotta */
			stxt = "esplosione d'energia";

		if ( skill->type == SKILLTYPE_SPELL )
		{
			color = AT_MAGIC;

			if ( pexit )
			{
				act( AT_MAGIC, "Lascio partire $t $T.", ch, aoran(stxt), table_dir[dir].to, TO_CHAR );
				act( AT_MAGIC, "$n lascia partire $t $T.", ch, stxt, table_dir[dir].to, TO_ROOM );
			}
			else
			{
				act( AT_MAGIC, "Lascio partire $t contro $N.", ch, aoran(stxt), victim, TO_CHAR );
				act( AT_MAGIC, "$n lascia partire la su$x $t contro $N.", ch, stxt, victim, TO_NOVICT );
				act( AT_MAGIC, "$n lascia partire la su$x $t contro di me!", ch, stxt, victim, TO_VICT );
			}
		}
	}
	else
	{
		send_log( NULL, LOG_BUG, "ranged_attack: no projectile, no skill dt %d", dt );
		return rNONE;
	}

	/* Victim in same room */
	if ( victim )
	{
		is_illegal_pk( ch, victim );
		check_attacker( ch, victim );
		return ranged_got_target( ch, victim, weapon, projectile, 0, dt, stxt, color );
	}

	/* assign scanned victim */
	victim = vch;

	/* reverse direction text from move_char */
	rdir = table_dir[pexit->vdir].rev_dir;

	while ( dist <= range )
	{
		char_from_room(ch);
		char_to_room(ch, pexit->to_room);

		if ( HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			/* whadoyahknow, the door's closed */
			if ( projectile )
				sprintf( buf,"Vedo il mio %s conficcarsi contro una porta %s.", myobj(projectile), table_dir[dir].to );
			else
				sprintf( buf, "Vedo il mio %s conficcarsi contro una porta %s.", stxt, table_dir[dir].to );

			act( color, buf, ch, NULL, NULL, TO_CHAR );

			if ( projectile )
			{
				sprintf( buf,"$p scappa %s barricandosi dietro la porta %s.", table_dir[rdir].from, table_dir[dir].to );
				act( color, buf, ch, projectile, NULL, TO_ROOM );
			}
			else
			{
				sprintf(buf, "%s scappa %s barricandosi dietro la porta %s.", aoran(stxt), table_dir[rdir].from, table_dir[dir].to );
				buf[0] = toupper(buf[0]);
				act( color, buf, ch, NULL, NULL, TO_ROOM );
			}
			break;
		} /* chiude l'if */


		/* no victim? pick a random one */
		if ( !victim )
		{
			for ( vch = ch->in_room->first_person;  vch;  vch = vch->next_in_room )
			{
				if ( ((IS_MOB(ch) && IS_PG(vch))
				  || (IS_PG(ch) &&  IS_MOB(vch)))
				  && number_bits(1) == 0 )
				{
					victim = vch;
					break;
				}
			}

			if ( victim && is_safe(ch, victim, TRUE) )
			{
				char_from_room(ch);
				char_to_room(ch, was_in_room);
				return rNONE;
			}
		}

		/* In the same room as our victim? */
		if ( victim && ch->in_room == victim->in_room )
		{
			if ( projectile )
				act( color, "$p scappa $T.", ch, projectile, table_dir[rdir].from, TO_ROOM );
			else
				act( color, "$t scappa $T.", ch, aoran(stxt), table_dir[rdir].from, TO_ROOM );

			/* get back before the action starts */
			char_from_room( ch );
			char_to_room( ch, was_in_room );

			is_illegal_pk( ch, victim );
			check_attacker( ch, victim );
			return ranged_got_target( ch, victim, weapon, projectile, dist, dt, stxt, color );
		}

		if ( dist == range )
		{
			if ( projectile )
			{
				act( color, "Il mio $t cade inerte a terra $T.",
					ch, myobj(projectile), table_dir[dir].to, TO_CHAR );
				act( color, "$p vola $T ma cade a terra privo di spinta.",
					ch, projectile, table_dir[rdir].from, TO_ROOM );

				if ( projectile->in_obj )
					obj_from_obj( projectile );
				if ( projectile->carried_by )
					obj_from_char( projectile );

				obj_to_room( projectile, ch->in_room );
			}
			else
			{
				act( color, "Il mio $t cade inerte a terra $T.",
					ch, stxt, table_dir[dir].to, TO_CHAR );
				act( color, "$t vola $T ma cade a terra privo di spinta.",
					ch, aoran(stxt), table_dir[rdir].from, TO_ROOM );
			}
			break;
		}

		pexit = get_exit( ch->in_room, dir );
		if ( !pexit )
		{
			if ( projectile )
			{
				act( color, "Il mio $t colpisce un muro e rimbalza a terra $T.", ch, myobj(projectile), table_dir[dir].to, TO_CHAR );
				act( color, "$p colpisce il muro $T e cade a terra vicino a me.", ch, projectile, table_dir[dir].to, TO_ROOM );

				if ( projectile->in_obj )
					obj_from_obj( projectile );
				if ( projectile->carried_by )
					obj_from_char( projectile );

				obj_to_room( projectile, ch->in_room );
			}
			else
			{
				act( color, "Il mio $t colpisce un muro e rimbalza a terra $T.", ch, stxt, table_dir[dir].to, TO_CHAR );
				act( color, "$p colpisce il muro $T e cade a terra vicino a te.", ch, aoran(stxt), table_dir[dir].to, TO_ROOM );
			}
			break;
		}

		if ( projectile )
			act( color, "$p schizza $T.", ch, projectile, table_dir[rdir].from, TO_ROOM );
		else
			act( color, "$t schizza $T.", ch, aoran(stxt), table_dir[rdir].from, TO_ROOM );

		dist++;
    } /* chiude il while */

	char_from_room( ch );
	char_to_room( ch, was_in_room );

	return rNONE;
}


/*
 * Fire <direction> <target>
 * Fire a projectile from a missile weapon (bow, crossbow, etc)
 * Support code (see projectile_hit(), quiver support, other changes to
 * fight.c
 */
DO_RET do_fire( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *arrow;
	OBJ_DATA *bow;
	int		  max_dist;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Chi e dove dovrei colpire?\r\n" );
		return;
	}

	if ( HAS_BIT( ch->in_room->flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( ch, "Qualcosa mi impedisce di portare a termine l'attacco.\r\n" );
		return;
	}

	/*
	 * Find the projectile weapon
	 */
	bow = get_eq_char( ch, WEARLOC_MISSILE );
	if ( !bow )
	{
		send_to_char( ch, "Cosa dovrei lanciare?\r\n" );
		return;
	}

	if ( bow->type != OBJTYPE_MISSILE_WEAPON )
		bow = NULL;

	/* Modify maximum distance based on bow-type and ch's class/str/etc */
	max_dist = URANGE( 1, bow->weapon->range, 10 );

	arrow = find_projectile( ch, bow->weapon->damage1 );
	if ( !arrow )
	{
		char *msg = "Non ho nulla con cui colpire.\r\n";

		switch ( bow->weapon->damage1 )
		{
		  case DAMAGE_BOLT:	    msg = "Non ho fulmini.\r\n";	break;
		  case DAMAGE_ARROW:	msg = "Non ho frecce.\r\n";		break;
		  case DAMAGE_DART:	    msg = "Non ho dardi.\r\n";		break;
		  case DAMAGE_STONE:	msg = "Non ho pietre.\r\n";		break;
		  case DAMAGE_PEA:		msg = "Non ho semi.\r\n";		break;
		}
		send_to_char( ch, msg );
		return;
	}

	/* Add wait state to fire for pkill, etc... */
	WAIT_STATE( ch, PULSE_IN_SECOND );

	/* Handle the ranged attack */
	ranged_attack( ch, argument, bow, arrow, TYPE_HIT + arrow->weapon->damage1, max_dist );
}


/*
 * Attempt to fire at a victim.
 * Returns FALSE if no attempt was made
 */
bool mob_fire( CHAR_DATA *ch, char *name )
{
	OBJ_DATA *arrow;
	OBJ_DATA *bow;
	int		  max_dist;

	if ( HAS_BIT( ch->in_room->flags, ROOM_SAFE ) )
		return FALSE;

	bow = get_eq_char( ch, WEARLOC_MISSILE );
	if ( !bow )
		return FALSE;

	if ( bow->type != OBJTYPE_MISSILE_WEAPON )
		bow = NULL;

	/* Modify maximum distance based on bow-type and ch's class/str/etc */
	max_dist = URANGE( 1, bow->weapon->range, 10 );

	arrow = find_projectile( ch, bow->weapon->damage1 );
	if ( !arrow )
		return FALSE;

	ranged_attack( ch, name, bow, arrow, TYPE_HIT + arrow->weapon->damage1, max_dist );

	return TRUE;
}


/* -- working on --
 * Sintassi:
 *	throw object  (assumed already fighting)
 *	throw object direction target  (all needed args for distance throwing)
 *	throw object  (assumed same room throw)
 */
/*
DO_RET do_throw( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*was_in_room;
	CHAR_DATA	*victim;
	OBJ_DATA	*throw_obj;
	EXIT_DATA	*pexit;
	char		 arg[MIL];
	char		 arg1[MIL];
	char		 arg2[MIL];
	int			 dir;
	int			 dist;
	int			 max_dist = 3;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	for ( throw_obj = ch->last_carrying;  throw_obj;  throw_obj = throw_obj=>prev_content )
	{
---		if ( can_see_obj(ch, throw_obj, TRUE)
		  && (WEAR_HELD			== throw_obj->wear_loc
		  ||  WEAR_WIELDED		== throw_obj->wear_loc
		  ||  WEAR_DUAL_WIELDED == throw_obj->wear_loc)
		  && nifty_is_name(arg, throw_obj->name) )
		{
			break;
		}
 ----
		if ( can_see_obj(ch, throw_obj, TRUE) && nifty_is_name( arg, throw_obj->name )
			break;
	}

	if ( !throw_obj )
	{
		send_to_char( ch, "You aren't holding or wielding anything like that.\r\n" );
		return;
	}

----
	if ( ( throw_obj->type != OBJTYPE_WEAPON)
	{
		send_to_char( ch, "You can only throw weapons.\r\n" );
		return;
	}
----

	if ( get_obj_weight(throw_obj) - ((get_curr_attr(ch, ATTR_STR)/4 - 15) * 3) > 0 )
	{
		send_to_char( ch, "That is too heavy for you to throw.\r\n" );
		if (!number_range(0,10))
			learn_skell( ch, gsn_throw, FALSE );
		return;
	}

	if ( ch->fighting )
	{
		victim = ch->fighting;
	}
	else
	{
		victim = get_char_room( ch, arg1, TRUE );
		if ( !victim && !VALID_STR(arg2) )
		{
			act( AT_GREY, "Throw $t at whom?", ch, obj->short_descr, NULL, TO_CHAR );
			return;
		}
	}
}
*/

DO_RET do_slice( CHAR_DATA *ch, char *argument )
{
	MOB_PROTO_DATA *pMobIndex;
	OBJ_DATA	   *corpse;
	OBJ_DATA	   *obj;
	OBJ_DATA	   *slice;
	char			buf[MSL];
	char			buf1[MSL];
	bool			found;

	found = FALSE;

	if ( IS_PG(ch) && !IS_ADMIN(ch)
	  && get_level(ch) < table_skill[gsn_slice]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non conosco quest'abilità.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Da quali carni dovrei ricavare il mio pranzo?\r\n" );
		return;
	}

	if ( (obj = get_eq_char(ch, WEARLOC_WIELD)) == NULL
	  || (obj->weapon->damage1 != DAMAGE_SLICE
	  &&  obj->weapon->damage1 != DAMAGE_STAB
	  &&  obj->weapon->damage1 != DAMAGE_SLASH
	  &&  obj->weapon->damage1 != DAMAGE_PIERCE) )
	{
		send_to_char( ch, "Ho bisogno di una lama per tagliare.\r\n" );
		return;
	}

	corpse = get_obj_here( ch, argument );
	if ( !corpse )
	{
		send_to_char( ch, "Non trovo nulla di simile qui.\r\n" );
		return;
	}

	if ( corpse->type != OBJTYPE_CORPSE_MOB || corpse->corpse->tick < 75 )		/* (bb) ration? */
	{
		send_to_char( ch, "Di certo non è da lì che ricaverò della carne.\r\n" );
		return;
	}

	pMobIndex = get_mob_index( NULL, (VNUM) -(corpse->corpse->time) );	/* se non si capisse quel VNUM è un cast */
	if ( !pMobIndex )
	{
		send_log( NULL, LOG_BUG, "do_slice: impossibile trovare il mob per la variabile time del cadavere." );
		return;
	}

	if ( get_obj_proto(NULL, VNUM_OBJ_SLICE) == NULL )
	{
		send_log( NULL, LOG_BUG, "do_slice: vnum %d non trovato.", VNUM_OBJ_SLICE );
		return;
	}

	if ( !can_use_skill(ch, number_percent(), gsn_slice) && !IS_ADMIN(ch) )
	{
		send_to_char( ch, "Non riesco a tagliare per bene la carne.\r\n" );
		learn_skell( ch, gsn_slice, FALSE );	/* Just in case they die :> */

		if ( number_percent() + (get_curr_attr(ch, ATTR_AGI)/4 - 13) < 10 )
		{
			act( AT_DRED, "Ahi! Mi sono tagliato!", ch, NULL, NULL, TO_CHAR );
			damage( ch, ch, get_level(ch)/2, gsn_slice );
		}
		return;
	}

	slice = make_object( get_obj_proto(NULL, VNUM_OBJ_SLICE), 0 );

	sprintf( buf, "una fetta di carne di %s", pMobIndex->name );
	DISPOSE( slice->name );
	slice->name = str_dup( buf );

	sprintf( buf, "una fetta di carne di %s", pMobIndex->short_descr );
	DISPOSE( slice->short_descr );
	slice->short_descr = str_dup( buf );

	sprintf( buf1, "Una fetta di carne di %s giace a terra.", pMobIndex->short_descr );
	DISPOSE( slice->long_descr );
	slice->long_descr = str_dup( buf1 );

	act( AT_DRED, "$n ricava delle sottili fette di carne da $p.", ch, corpse, NULL, TO_ROOM);
	act( AT_DRED, "Ricavo delle sottili fette di carne da $p.", ch, corpse, NULL, TO_CHAR);

	obj_to_char( slice, ch );
	corpse->corpse->tick -= 25;		/* (bb) ration? */
	learn_skell( ch, gsn_slice, TRUE );
}


/*------------------------------------------------------------
 *  Fighting Styles
 */
DO_RET do_style( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
		return;

	if ( is_name_prefix(argument, "evasivo evasive") )
	{
		if ( get_level(ch) < table_skill[gsn_style_evasive]->skill_level[ch->class] )
		{
			send_to_char( ch, "Non ho ancora imparato lo stile di lotta evasivo.\r\n" );
			return;
		}

		WAIT_STATE( ch, table_skill[gsn_style_evasive]->beats * 3 );	/* (WT) */

		if ( number_percent( ) < knows_skell(ch, gsn_style_evasive) )
		{
			/* success */
			if ( ch->fighting )
			{
				ch->position = POSITION_EVASIVE;
				act( AT_ACTION, "$n porto il corpo indietro in una posizione evasiva.", ch, NULL, NULL, TO_ROOM );
			}
			ch->style = STYLE_EVASIVE;
			send_to_char( ch, "Adotto uno stile di lotta evasivo.\r\n" );
			return;
		}
		else
		{
			/* failure */
			send_to_char( ch, "Per poco non inciampo nel tentativo di cambiare stile di lotta..\r\n" );
			return;
		}
	}
	else if ( is_name_prefix(argument, "difensivo defensive") )
	{
		if ( get_level(ch) < table_skill[gsn_style_defensive]->skill_level[ch->class] )
		{
			send_to_char( ch, "Non ho ancora imparato a combattere con questo stile.\r\n" );
			return;
		}

		WAIT_STATE( ch, table_skill[gsn_style_defensive]->beats * 3 );	/* (WT) */

		if ( number_percent() < knows_skell(ch, gsn_style_defensive) )
		{
			/* success */
			if ( ch->fighting )
			{
				ch->position = POSITION_DEFENSIVE;
				act( AT_ACTION, "$n sposto indietro il baricentro tenendo alte le difese.", ch, NULL, NULL, TO_ROOM );
			}
			ch->style = STYLE_DEFENSIVE;
			send_to_char( ch, "Adotto uno stile di lotta difensivo.\r\n" );
			return;
		}
		else
		{
			/* failure */
			send_to_char( ch, "Per poco non inciampo nel tentativo di cambiare stile di lotta..\r\n" );
			return;
		}
	}
	else if ( !str_prefix(argument, "normale") )
	{
		if ( get_level(ch) < table_skill[gsn_style_normal]->skill_level[ch->class] )
		{
			send_to_char( ch, "Devo ancora imparare questo tipo di lotta.\r\n" );
			return;
		}

		WAIT_STATE( ch, table_skill[gsn_style_normal]->beats * 3 );	/* (WT) */

		if ( number_percent() < knows_skell(ch, gsn_style_normal) )
		{
			/* success */
			if ( ch->fighting )
			{
				ch->position = POSITION_FIGHT;
				act( AT_ACTION, "$n cambio posizione di combattimento tornando in guardia.", ch, NULL, NULL, TO_ROOM );
			}
			ch->style = STYLE_FIGHTING;
			send_to_char( ch, "Adotto lo stile di combattimento normale.\r\n" );
			return;
		}
		else
		{
			/* failure */
			send_to_char( ch, "Per poco non inciampo nel tentativo di cambiare stile di lotta..\r\n" );
			return;
		}
	}
	else if ( is_name_prefix(argument, "aggressivo aggressive") )
	{
		if ( get_level(ch) < table_skill[gsn_style_aggressive]->skill_level[ch->class] )
		{
			send_to_char( ch, "Devo ancora imparare a padroneggiare lo stile di lotta aggressivo.\r\n" );
			return;
		}

		WAIT_STATE( ch, table_skill[gsn_style_aggressive]->beats * 3 );	/* (WT) */

		if ( number_percent() < knows_skell(ch, gsn_style_aggressive) )
		{
			/* success */
			if ( ch->fighting )
			{
				ch->position = POSITION_AGGRESSIVE;
				act( AT_ACTION, "$n sposto in avanti il baricentro proiettando in avanti il suo corpo.", ch, NULL, NULL, TO_ROOM );
			}
			ch->style = STYLE_AGGRESSIVE;
			send_to_char( ch, "Adotto uno stile di lotta aggressivo.\r\n" );
			return;
		}
		else
		{
			/* failure */
			send_to_char( ch, "Per poco non inciampo nel tentativo di cambiare stile di lotta..\r\n" );
			return;
		}
	}
	else if ( !str_prefix(argument, "berserk") )
	{
		if ( get_level(ch) < table_skill[gsn_style_berserk]->skill_level[ch->class] )
		{
			send_to_char( ch, "Non ho ancora imparato a lottare come un berserker.\r\n" );
			return;
		}

		WAIT_STATE( ch, table_skill[gsn_style_berserk]->beats * 3 );	/* (WT) */

		if ( number_percent() < knows_skell(ch, gsn_style_berserk) )
		{
			/* success */
			if ( ch->fighting )
			{
				ch->position = POSITION_BERSERK;
				act( AT_ACTION, "$n adotto uno stile di combattimento completamente aggressivo e selvaggio!", ch, NULL, NULL, TO_ROOM );
			}
			ch->style = STYLE_BERSERK;
			send_to_char( ch, "Affido ogni risorsa all'attacco: in berserker!\r\n" );
			return;
		}
		else
		{
			/* failure */
			send_to_char( ch, "Per poco non inciampo nel tentativo di cambiare stile di lotta..\r\n" );
		}
	}
	else
	{
		send_to_char( ch, "&wQuale stile di lotta sarà meglio adottare?\r\n" );
		ch_printf	( ch, "Attualmente uso lo stile %s&w\r\n",
			(ch->style == STYLE_BERSERK	  )  ?  "&Rberserk"		:
			(ch->style == STYLE_AGGRESSIVE)  ?  "&Raggressivo"	:
			(ch->style == STYLE_DEFENSIVE )  ?  "&Ydifensivo"	:
			(ch->style == STYLE_EVASIVE	  )  ?  "&Yevasivo"		:
												"normale" );
	}
}


/*
 * Comando Cook
 */
DO_RET do_cook ( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *food;
	OBJ_DATA *fire;
	char	  arg[MIL];
	char	  buf[MSL];

	if ( IS_MOB(ch) || get_level(ch) < table_skill[gsn_cook]->skill_level[ch->class] )
	{
		send_to_char( ch, "Non ho mai saputo cucinare, sarebbe ora che imparassi.\r\n" );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Cosa dovrei cucinare?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	food = get_obj_carry( ch, arg, TRUE );
	if ( !food )
	{
		send_to_char( ch, "Non ho quell'oggetto.\r\n" );
		return;
	}

	if ( food->type != OBJTYPE_COOK )
	{
		send_to_char( ch, "E come potrei cucinare una cosa del genere?\r\n" );
		return;
	}

	if ( food->food->dam_cook > 2 )
	{
		send_to_char( ch, "Non mi sembra che difetti in cottura.\r\n" );
		return;
	}

	for ( fire = ch->in_room->first_content;  fire;  fire = fire->next_content )
	{
		if ( fire->type == OBJTYPE_FIRE )
			break;
	}

	if ( !fire )
	{
		send_to_char( ch, "Mi serve un fuoco per cucinare.\r\n" );
		return;
	}

	split_obj( food, 1 );
	if ( number_percent() > knows_skell(ch, gsn_cook) )
	{
		food->timer = food->timer/2;
        food->food->hours = 0;
		food->food->dam_cook = 3;

		act( AT_MAGIC, "$p rosola tra le fiamme cuocendosi a puntino!\r\n", ch, food, NULL, TO_CHAR );
		act( AT_MAGIC, "$n tiene $p vicino al fuoco cuocendo a puntino.",  ch, food, NULL, TO_ROOM);

		/* (FF) problema del sesso per gli oggetti */
		strcpy( buf, food->pObjProto->name );
		DISPOSE( food->short_descr );
		food->short_descr = str_dup( buf );
		sprintf( buf, "Un bruciato %s.", food->pObjProto->name);
		DISPOSE( food->long_descr );
		food->long_descr = str_dup( buf );

		return;
	}

	if ( number_percent() > 85 )
	{
		food->timer = food->timer * 3;
		food->food->dam_cook += 2;

		act( AT_MAGIC, "$n brucia $p.",ch, food, NULL, TO_ROOM );
		act( AT_MAGIC, "Purtroppo ho esagerato e $p si brucia..",ch, food, NULL, TO_CHAR );

		sprintf( buf, "un troppo cotto %s", food->pObjProto->name );
		DISPOSE( food->short_descr );
		food->short_descr = str_dup( buf );
		sprintf( buf, "Un troppo cotto %s.", food->pObjProto->name );
		DISPOSE( food->long_descr );
		food->long_descr = str_dup( buf );
	}
	else
	{
		food->timer = food->timer * 4;
		food->food->hours *= 2;

		act( AT_MAGIC, "$n arrostisce $p.",ch, food, NULL, TO_ROOM );
		act( AT_MAGIC, "Arrostisco $p.",ch, food, NULL, TO_CHAR );

		sprintf( buf, "un arrostito %s", food->pObjProto->name );
		DISPOSE( food->short_descr );
		food->short_descr = str_dup( buf );
		sprintf( buf, "un arrostito %s.", food->pObjProto->name );
		DISPOSE( food->long_descr );
		food->long_descr = str_dup( buf );

		food->food->dam_cook++;
	}

	learn_skell( ch, gsn_cook, TRUE );
}


DO_RET do_shove( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*to_room;
	EXIT_DATA	*pexit;
	CHAR_DATA	*victim;
	char		 arg[MIL];
	char		 arg2[MIL];
	int			 exit_dir;
	int			 shove_chance = 0;
	bool		 nogo;

	argument = one_argument( argument, arg  );
	argument = one_argument( argument, arg2 );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I Mob non possono farlo.\r\n" );
		return;
	}

	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Dovrei spingere chi?\r\n" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non si trova qui.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Spingermi? Oh ma che bella idea..\r\n" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		ch_printf( ch, "Non posso farlo su di %s.\r\n", (victim->sex == SEX_FEMALE) ? "lei" : "lui" );	/* (GR) trovare una funzione o usare una act forse è meglio */
		return;
	}

	if ( get_timer(ch, TIMER_PKILLED) > 0 || get_timer(ch, TIMER_KILLED) > 0 )
	{
		send_to_char( ch, "La tua anima è tornata in questo corpo da troppo poco tempo.\r\n" );
		return;
	}

	if ( get_timer(victim, TIMER_PKILLED) > 0 || get_timer(victim, TIMER_KILLED) > 0 )
	{
		ch_printf( ch, "Non posso spingerl%c per ora.\r\n", gramm_ao(victim) );
		return;
	}

	if ( victim->position != POSITION_STAND )
	{
		act( AT_PLAIN, "$N non è in piedi.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( !VALID_STR(arg2) )
	{
		send_to_char( ch, "Spingerlo in che direzioni?\r\n" );
		return;
	}

	exit_dir = get_dir( arg2 );
	if ( HAS_BIT(victim->in_room->flags, ROOM_SAFE )
	  && get_timer(victim, TIMER_SHOVEDRAG) <= 0 )
	{
		ch_printf( ch, "Non posso spingerl%c per ora.\r\n", gramm_ao(victim) );
		return;
	}

	victim->position = POSITION_SHOVE;
	nogo = FALSE;
	pexit = get_exit( ch->in_room, exit_dir );
	if ( !pexit )
	{
		nogo = TRUE;
	}
	else if ( HAS_BIT(pexit->flags, EXIT_CLOSED)
	  && (!HAS_BIT(victim->affected_by, AFFECT_PASS_DOOR)
	  || HAS_BIT(pexit->flags, EXIT_NOPASSDOOR)) )
	{
		nogo = TRUE;
	}

	if ( nogo )
	{
		send_to_char( ch, "Non trovo nessuna uscita in quella direzione.\r\n" );
		victim->position = POSITION_STAND;
		return;
	}

	to_room = pexit->to_room;
	if ( HAS_BIT(to_room->flags, ROOM_DEATH) )
	{
		ch_printf( ch, "Non posso spingerl%c di là.\r\n", gramm_ao(victim) );
		victim->position = POSITION_STAND;
		return;
	}

#ifdef T2_ALFA
	if ( ch->in_room->area != to_room->area
	  && is_restricted(victim, to_room->area->restr) )	/* (FF) da aggiungere la flag apposita */
	{
		ch_printf( ch, "Non posso spingerl%c di là.\r\n", gramm_ao(victim) );
		victim->position = POSITION_STAND;
		return;
	}
#endif

	/* Add 4 points to chance for every 3 str point above 20, subtract for below 20 */
	shove_chance = 40;
	shove_chance += ( get_curr_attr(ch, ATTR_STR)/3 - 20 ) * 4;
	shove_chance += get_level(ch)/2 - get_level(victim)/2;

	/* Debugging purposes. Show percentage for testing. */
	send_log( NULL, LOG_SHOVEDRAG, "do_shove: percent %d di %s", shove_chance, ch->name );

	if ( shove_chance < number_percent() )
	{
		send_to_char( ch, "Fallisco.\r\n" );
		victim->position = POSITION_STAND;
		return;
	}

	act( AT_ACTION, "Spingo $N.", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n mi spinge.", ch, NULL, victim, TO_VICT );

	move_char( victim, get_exit(ch->in_room,exit_dir), 0, FALSE );

	if ( !char_died(victim) )
		victim->position = POSITION_STAND;

	WAIT_STATE( ch, PULSE_IN_SECOND * 3 );

	/* Remove protection from shove/drag if char shoves */
	if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) && get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
		add_timer( ch, TIMER_SHOVEDRAG, PULSE_VIOLENCE * 10, NULL, 0 );
}


/*
 * Da spostare in drag.
 */
DO_RET do_drag( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	EXIT_DATA	*pexit;
	ROOM_DATA	*to_room;
	char		 arg[MIL];
	char		 arg2[MIL];
	int			 exit_dir;
	int			 drag_chance = 0;
	int			 temp_pos;
	bool		 nogo;

	argument = one_argument( argument, arg  );
	argument = one_argument( argument, arg2 );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I Mob non possono farlo.\r\n" );
		return;
	}

	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Dovrei trascinare chi?\r\n" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non lo vedo qui intorno.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Trascinarmi?? Oh ma che bell'idea..\r\n" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		ch_printf( ch, "Non posso trascinarl%c.\r\n", gramm_ao(victim) );
		return;
	}

	if ( get_timer(ch, TIMER_PKILLED) > 0 || get_timer(ch, TIMER_KILLED) > 0 )
	{
		send_to_char( ch, "La tua anima è tornata in questo corpo da troppo poco tempo.\r\n" );
		return;
	}

	if ( get_timer(victim, TIMER_PKILLED) > 0 || get_timer(victim, TIMER_KILLED) > 0 )
	{
		ch_printf( ch, "Non posso trascinarl%c per ora.\r\n", gramm_ao(victim) );
		return;
	}

	if ( victim->fighting )
	{
		send_to_char( ch, "Non posso avvicinarmi più di così, sta combattendo.\r\n" );
		return;
	}

	if ( victim->position > POSITION_STUN )
	{
		send_to_char( ch, "Non credo che abbisogni di assistenza.\r\n" );
		return;
	}

	if ( !VALID_STR(arg2) )
	{
		ch_printf( ch, "Trascinarl%c in quale direzione?\r\n", gramm_ao(victim) );
		return;
	}

	exit_dir = get_dir( arg2 );

	if ( HAS_BIT(victim->in_room->flags, ROOM_SAFE)
	  && get_timer(victim, TIMER_SHOVEDRAG) <= 0 )
	{
		ch_printf( ch, "Non posso trascinarl%c per ora.\r\n", gramm_ao(victim) );
		return;
	}

	nogo = FALSE;
	pexit = get_exit( ch->in_room, exit_dir );
	if ( !pexit )
	{
		nogo = TRUE;
	}
	else if ( HAS_BIT(pexit->flags, EXIT_CLOSED)
	  && (!HAS_BIT(victim->affected_by, AFFECT_PASS_DOOR)
	  || HAS_BIT(pexit->flags, EXIT_NOPASSDOOR)) )
	{
		nogo = TRUE;
	}

	if ( nogo )
	{
		send_to_char( ch,  "Non trovo nessuna uscita in quella direzione.\r\n" );
		return;
	}

	to_room = pexit->to_room;
	if ( HAS_BIT(to_room->flags, ROOM_DEATH) )
	{
		ch_printf( ch, "Non posso spingerl%c laggiù.\r\n", gramm_ao(victim) );
		return;
	}

#ifdef T2_ALFA
	if ( ch->in_room->area != to_room->area
	  && is_restricted(victim, to_room->area->restr) )	/* (FF) da aggiungere la flag apposita */
	{
		ch_printf( ch, "Non posso spingerl%c laggiù.\r\n", gramm_ao(victim) );
		victim->position = POSITION_STAND;
		return;
	}
#endif

	/* Add 4 points to chance for every 3 str point above 20, subtract for below 20 */
	drag_chance = 40;
	drag_chance += ( get_curr_attr(ch, ATTR_STR)/3 - 20 ) * 4;
	drag_chance += get_level(ch)/2 - get_level(victim)/2;

	/* Informazioni di debug sulla percentuale del drag */
	send_log( NULL, LOG_SHOVEDRAG, "do_drag: percentuale %d di %s", drag_chance, ch->name );

	if ( drag_chance < number_percent() )
	{
		send_to_char( ch, "Fallisco.\r\n" );
		victim->position = POSITION_STAND;
		return;
	}

	if ( victim->position >= POSITION_STAND )
	{
		ch_printf( ch, "Non posso trascinarl%c, è in piedi.\r\n", gramm_ao(victim) );
		return;
	}

	temp_pos = victim->position;
	victim->position = POSITION_DRAG;

	act( AT_ACTION, "Trascino $N.", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n ti afferra e ti trascina.", ch, NULL, victim, TO_VICT );

	move_char( victim, get_exit(ch->in_room,exit_dir), 0, FALSE );
	if ( !char_died(victim) )
		victim->position = temp_pos;
	/* Move ch to the room too.. they are doing dragging */
	move_char( ch, get_exit(ch->in_room,exit_dir), 0, FALSE );

	WAIT_STATE( ch, PULSE_IN_SECOND * 3 );
}
