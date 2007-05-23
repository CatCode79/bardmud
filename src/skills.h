#ifndef SKILL_H
#define SKILL_H

/*
 * Definizioni
 */
#define SKILL_DIR				"../skills/"		/* Dir dati sulle abilità */
#define	SKILL_FILE	SKILL_DIR	"skills.skill"		/* Elenco delle abilità */
#define	SPELL_FILE	SKILL_DIR	"spells.skill"		/* Elenco degli incantesimi */
#define	WEAPON_FILE	SKILL_DIR	"weapons.skill"		/* Elenco delle abilità e talenti */


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED	  -1
#define TYPE_HIT		1000	/* allows for 1000 skills/spells */
#define TYPE_HERB		2000	/* allows for 1000 attack types	 */
#define TYPE_RACIAL		3000	/* allows for 1000 herb types	 */
#define TYPE_DISEASE	4000	/* allows for 1000 racial types	 */


/*
 * Enumerazione dei tipi di skill.
 */
typedef enum
{
	SKILLTYPE_UNKNOWN,
	SKILLTYPE_SPELL,
	SKILLTYPE_SKILL,
	SKILLTYPE_WEAPON,	/* (FF) da creare una struttura per loro */
	SKILLTYPE_TONGUE,	/* (CC) da togliere poi*/
	SKILLTYPE_HERB,		/* (FF) da creare una struttura per loro */
	SKILLTYPE_RACIAL,	/* (CC) da togliere */
	SKILLTYPE_DISEASE,	/* (CC) da togliere direi */
	/* 8 tipi di skill */
	MAX_SKILLTYPE
} skill_types;


/*
 * Enumerazione delle intenzioni delle skill
 */
typedef enum
{
	INTENT_NONE = -1,
	INTENT_NEUTRAL,
	INTENT_FRIENDLY,
	INTENT_AGGRESSIVE,
	MAX_INTENT
} intention_types;


/*
 * old flags for conversion purposes -- will not conflict with the flags below
 */
#define OLD_SF_SAVE_HALF_DAMAGE		18	/* old save for half damage	*/
#define OLD_SF_SAVE_NEGATES			19	/* old save negates affect	*/


/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
typedef enum
{
	SKELL_WATER,
	SKELL_EARTH,
	SKELL_AIR,
	SKELL_ASTRAL,
	SKELL_AREA,				/* is an area spell					*/
	SKELL_DISTANT,			/* affects something far away		*/
	SKELL_REVERSE,
	SKELL_NOSELF,			/* Can't target yourself!			*/
	SKELL_UNUSED2,			/* free for use!					*/
	SKELL_ACCUMULATIVE,		/* is accumulative					*/
	SKELL_RECASTABLE,		/* can be refreshed					*/
	SKELL_NOSCRIBE,			/* cannot be scribed				*/
	SKELL_NOBREW,			/* cannot be brewed					*/
	SKELL_GROUPSPELL,		/* only affects group members		*/
	SKELL_OBJECT,			/* directed at an object			*/
	SKELL_CHARACTER,		/* directed at a character			*/
	SKELL_SECRETSKILL,		/* hidden unless learned			*/
	SKELL_PKSENSITIVE,		/* much harder for plr vs. plr		*/
	SKELL_STOPONFAIL,		/* stops spell on first failure		*/
	SKELL_NOFIGHT,			/* stops if char fighting			*/
	SKELL_NODISPEL,			/* stops spell from being dispelled	*/
	SKELL_RANDOMTARGET,		/* chooses a random target			*/
	SKELL_SILENT,			/* (GG) non invia output			*/
	MAX_SKELL
} skell_flags;


typedef enum
{
	SPELLDAMAGE_NONE,
	SPELLDAMAGE_FIRE,
	SPELLDAMAGE_COLD,
	SPELLDAMAGE_ELECTRICITY,
	SPELLDAMAGE_ENERGY,
	SPELLDAMAGE_ACID,
	SPELLDAMAGE_POISON,
	SPELLDAMAGE_DRAIN,
	/* 8 spell dam */
	MAX_SPELLDAMAGE
} spelldamage_types;


typedef enum
{
	SPELLACTION_NONE,
	SPELLACTION_CREATE,
	SPELLACTION_DESTROY,
	SPELLACTION_RESIST,
	SPELLACTION_SUSCEPT,
	SPELLACTION_DIVINATE,
	SPELLACTION_OBSCURE,
	SPELLACTION_CHANGE,
	/* 8 spell action */
	MAX_SPELLACTION
} spellaction_types;


typedef enum
{
	SPELLPOWER_NONE,
	SPELLPOWER_MINOR,
	SPELLPOWER_GREATER,
	SPELLPOWER_MAJOR,
	MAX_SPELLPOWER
} spellpower_types;


typedef enum
{
	SPELLSPHERE_NONE,
	SPELLSPHERE_LUNAR,
	SPELLSPHERE_SOLAR,
	SPELLSPHERE_TRAVEL,
	SPELLSPHERE_SUMMON,
	SPELLSPHERE_LIFE,
	SPELLSPHERE_DEATH,
	SPELLSPHERE_ILLUSION,
	/* 8 spell class */
	MAX_SPELLSPHERE
} spellsphere_types;


typedef enum
{
	SPELLSAVE_NONE,
	SPELLSAVE_NEGATE,
	SPELLSAVE_EIGHTHDAM,
	SPELLSAVE_QUARTERDAM,
	SPELLSAVE_HALFDAM,
	SPELLSAVE_3QTRDAM,
	SPELLSAVE_REFLECT,
	SPELLSAVE_ABSORB,
	/* 8 save affects */
	MAX_SPELLSAVE
} spellsave_types;


#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~( (1 << 0) | (1 << 1) | (1 << 2) )
#define SACT_MASK		ALL_BITS & ~( (1 << 3) | (1 << 4) | (1 << 5) )
#define SCLA_MASK		ALL_BITS & ~( (1 << 6) | (1 << 7) | (1 << 8) )
#define SPOW_MASK		ALL_BITS & ~( (1 << 9) | (1 << 10) )
#define SSAV_MASK		ALL_BITS & ~( (1 << 11) | (1 << 12) | (1 << 13) )


/*
 * Spell Macro
 */
#define SPELL_DAMAGE(skill)		( ((skill)->info	  ) & 7 )
#define SPELL_ACTION(skill)		( ((skill)->info >>  3) & 7 )
#define SPELL_CLASS(skill)		( ((skill)->info >>  6) & 7 )
#define SPELL_POWER(skill)		( ((skill)->info >>  9) & 3 )
#define SPELL_SAVE(skill)		( ((skill)->info >> 11) & 7 )

#define SET_SDAM(skill, val)	( (skill)->info = ((skill)->info & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->info = ((skill)->info & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->info = ((skill)->info & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->info = ((skill)->info & SPOW_MASK) + (((val) & 3) << 9) )
#define SET_SSAV(skill, val)	( (skill)->info = ((skill)->info & SSAV_MASK) + (((val) & 7) << 11) )


/*
 * Skills include spells as a particular case.
 */
struct skill_data
{
	char				*name;						/* Name of skill */
	SPELL_FUN			*spell_fun;					/* Spell pointer (for spells) */
	DO_FUN				*skill_fun;					/* Skill pointer (for skills) */
	char				*fun_name;					/* Spell function name */
	int					 skill_adept[MAX_CLASS];	/* Max attainable % in this skill */
	int					 skill_level[MAX_CLASS];	/* Level needed by class */
	int					 race_mod[MAX_RACE];		/* Modificatore razziale (GG) */
	int					 target;					/* Legal targets */
	int					 min_position;				/* Position for caster / user */
	int					 slot;						/* Slot for #OBJECT loading */
	int					 min_mana;					/* Minimum mana used */
	int					 beats;						/* Rounds required to use skill */
	char				*noun_damage;				/* Damage message */
	char				*msg_off;					/* Wear off message */
	int					 type;						/* Spell/Skill/Weapon */
	int					 intention;					/* La tipologia di intenzione */
	int					 range;						/* Range of spell (rooms) */
	int					 info;						/* Spell action/class/etc */
	int					 flags;						/* Flags */
	char				*hit_area;					/* Success message to area */
	char				*hit_around;				/* Success message to area */
	char				*hit_char;					/* Success message to caster */
	char				*hit_vict;					/* Success message to victim */
	char				*hit_room;					/* Success message to room */
	char				*hit_dest;					/* Success message to dest room */
	char				*mast_area;					/* Success master message to area */
	char				*mast_around;				/* Success master message to area */
	char				*mast_char;					/* Success master message to caster */
	char				*mast_vict;					/* Success master message to victim */
	char				*mast_room;					/* Success master message to room */
	char				*mast_dest;					/* Success master message to dest room */
	char				*miss_area;					/* Failure message to area */
	char				*miss_around;				/* Failure message to area */
	char				*miss_char;					/* Failure message to caster */
	char				*miss_vict;					/* Failure message to victim */
	char				*miss_room;					/* Failure message to room */
	char				*die_area;					/* Victim death msg to area */
	char				*die_around;				/* Victim death msg to area */
	char				*die_char;					/* Victim death msg to caster */
	char				*die_vict;					/* Victim death msg to victim */
	char				*die_room;					/* Victim death msg to room */
	char				*imm_area;					/* Victim immune msg to area */
	char				*imm_around;				/* Victim immune msg to area */
	char				*imm_char;					/* Victim immune msg to caster */
	char				*imm_vict;					/* Victim immune msg to victim */
	char				*imm_room;					/* Victim immune msg to room */
	char				*dice;						/* Dice roll */
	int					 value;						/* Misc value, solitamente utilizzato per salvare il vnum da caricare, tipo obj o mob */
	BITVECTOR			*spell_sector;				/* Sector Spell work */
	int					 difficulty;				/* Difficulty of casting/learning */
#ifdef T2_SKILL
    char				*pre_skill1;				/* Pre required skill 1 */
    char				*pre_skill2;				/* Pre required skill 2 */
    char				*pre_skill3;				/* Pre required skill 3 */
#endif
	int					 saves;						/* What saving spell applies */
	SMAUG_AFF			*first_affect;				/* Spell affects, if any */
	SMAUG_AFF			*last_affect;
	char 				*components;				/* Spell components, if any */
	char 				*teachers;					/* Skill requires a special teacher */
	char				 participants;				/* # of required participants */
	struct timerset		 use_count;					/* Usage record */
};


/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	int		gsn_style_evasive;
extern	int		gsn_style_defensive;
extern	int		gsn_style_normal;
extern	int		gsn_style_aggressive;
extern	int		gsn_style_berserk;

extern	int		gsn_detrap;
extern	int		gsn_backstab;
extern	int		gsn_circle;
extern	int		gsn_cook;
extern	int		gsn_dodge;
extern	int		gsn_hide;
extern	int		gsn_peek;
extern	int		gsn_pick_lock;
extern	int		gsn_scan;
extern	int		gsn_sneak;
extern	int		gsn_steal;
extern	int		gsn_gouge;
extern	int		gsn_track;
extern	int		gsn_search;
extern	int		gsn_dig;
extern	int		gsn_mount;
extern	int		gsn_bashdoor;
extern	int		gsn_berserk;
extern	int		gsn_hitall;

extern	int		gsn_disarm;
extern	int		gsn_enhanced_damage;
extern	int		gsn_kick;
extern	int		gsn_parry;
extern	int		gsn_rescue;
extern	int		gsn_second_attack;
extern	int		gsn_third_attack;
extern	int		gsn_fourth_attack;
extern	int		gsn_fifth_attack;
extern	int		gsn_dual_wield;

/* Per vampiri */
extern	int		gsn_feed;
extern	int		gsn_bloodlet;
extern	int		gsn_broach;
extern	int		gsn_mistwalk;

extern	int		gsn_aid;

/* used to do specific lookups */
extern	int		gsn_first_spell;
extern	int		gsn_first_skill;
extern	int		gsn_first_weapon;
extern	int		gsn_top_sn;

/* spells */
extern	int		gsn_blindness;
extern	int		gsn_charm_person;
extern	int		gsn_aqua_breath;
extern	int		gsn_curse;
extern	int		gsn_invis;
extern	int		gsn_mass_invis;
extern	int		gsn_poison;
extern	int		gsn_sleep;
extern	int		gsn_possess;
extern	int		gsn_fireball;		/* for fireshield  */
extern	int		gsn_chill_touch;	/* for iceshield   */
extern	int		gsn_lightning_bolt;	/* for shockshield */

/* newer attack skills */
extern	int		gsn_punch;
extern	int		gsn_bash;
extern	int		gsn_stun;
extern	int		gsn_bite;
extern	int		gsn_claw;
extern	int		gsn_sting;
extern	int		gsn_tail;

extern	int		gsn_poison_weapon;
extern	int		gsn_scribe;
extern	int		gsn_brew;
extern	int		gsn_climb;

extern	int		gsn_pugilism;
extern	int		gsn_long_blades;
extern	int		gsn_short_blades;
extern	int		gsn_flexible_arms;
extern	int		gsn_talonous_arms;
extern	int		gsn_bludgeons;
extern	int		gsn_missile_weapons;
extern	int		gsn_shieldwork;

extern	int		gsn_grip;
extern	int		gsn_slice;

extern	int		gsn_tumble;


/*
 * Variabili
 */
extern	int					top_sn;		/* (FF) rinominare in top_skill appena si staccano gli spell dalle skill */
extern	int					top_spell;
extern	SKILL_DATA		   *table_skill[MAX_SKILL];
#ifdef T2_ALFA
extern	SPELL_DATA		   *spell_table[MAX_SPELL];
#endif
extern	const CODE_DATA		code_skilltype[];
extern	const int			max_check_skilltype;
extern	const CODE_DATA		code_intent[];
extern	const int			max_check_intent;
extern	const CODE_DATA		code_skell[];
extern	const int			max_check_skell;


/*
 * Funzioni
 */
void		load_skills		  ( void );
void		fread_skill		  ( MUD_FILE *fp, const char *section );	/* (FF) da togliere per la modularità in futuro */
DO_FUN	   *skill_function	  ( char *name );		/* (FF) in realtà quaesto si trova in tables.c per ora */
DO_RET		skill_notfound	  ( CHAR_DATA *ch, char *argument );
void		free_skill		  ( SKILL_DATA *skill );
void		free_all_skills	  ( void );
int			get_skill		  ( const char *skilltype );
void		sort_skill_table  ( void );
bool		is_valid_sn		  ( int sn );
bool		can_use_skill	  ( CHAR_DATA *ch, int percent, int gsn );
void		remap_slot_numbers( void );
bool		chance			  ( CHAR_DATA *ch, int percent );
void		trip			  ( CHAR_DATA *ch, CHAR_DATA *victim );
void		disarm			  ( CHAR_DATA *ch, CHAR_DATA *victim );
bool		check_tumble	  ( CHAR_DATA *ch, CHAR_DATA *victim );
bool		check_dodge		  ( CHAR_DATA *ch, CHAR_DATA *victim );
bool		check_parry		  ( CHAR_DATA *ch, CHAR_DATA *victim );
void		modify_skill	  ( CHAR_DATA *ch, int sn, int mod, bool fAdd );
SKILL_DATA *get_skilltype	  ( int sn );
SKILL_DATA *find_skill		  ( CHAR_DATA *ch, const char *command, bool exact, bool ita, const int first, const int top );
bool		check_skill		  ( CHAR_DATA *ch, const char *command, char *argument, bool exact, bool ita );
int			bsearch_skill	  ( const char *name, int first, int top );
int			ch_bsearch_skill  ( CHAR_DATA *ch, const char *name, int first, int top );
ch_ret		ranged_attack	  ( CHAR_DATA *ch, char *argument, OBJ_DATA *weapon, OBJ_DATA *projectile, int dt, int range );
void		learn_skell		  ( CHAR_DATA *ch, int sn, bool success );
CHAR_DATA  *scan_for_victim	  ( CHAR_DATA *ch, EXIT_DATA *pexit, char *name );
bool		mob_fire		  ( CHAR_DATA *ch, char *name );


#endif	/* SKILL_H */
