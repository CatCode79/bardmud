#ifndef AFFECT_H
#define AFFECT_H


/*
 * Flag bits per 'affected_by'
 * Utilizzati in #MOBILES.
 */
typedef enum
{
	AFFECT_BLIND,
	AFFECT_INVISIBLE,
	AFFECT_DETECT_EVIL,
	AFFECT_DETECT_INVIS,
	AFFECT_DETECT_MAGIC,
	AFFECT_DETECT_HIDDEN,
	AFFECT_HOLD,				/* non codificata */
	AFFECT_SANCTUARY,
	AFFECT_FAERIE_FIRE,
	AFFECT_INFRARED,
	AFFECT_CURSE,
	AFFECT_FLAMING,				/* non codificata */
	AFFECT_POISON,
	AFFECT_PROTECT,
	AFFECT_PARALYSIS,
	AFFECT_SNEAK,
	AFFECT_HIDE,
	AFFECT_SLEEP,
	AFFECT_CHARM,
	AFFECT_FLYING,
	AFFECT_PASS_DOOR,
	AFFECT_FLOATING,
	AFFECT_TRUESIGHT,
	AFFECT_DETECT_TRAPS,
	AFFECT_SCRYING,
	AFFECT_FIRESHIELD,
	AFFECT_SHOCKSHIELD,
	AFFECT_HAUS1,				/* inutilizzata */
	AFFECT_ICESHIELD,
	AFFECT_POSSESS,
	AFFECT_BERSERK,
	AFFECT_AQUA_BREATH,
	AFFECT_RECURRING_SPELL,
	AFFECT_CONTAGIOUS,
	AFFECT_ACIDMIST,
	AFFECT_VENOMSHIELD,
	AFFECT_DETECT_GOOD,
	AFFECT_DETECT_ALIGN,
	/* 37 affects per i mob */
	MAX_AFFECT
} affected_by_types;


/*
 * Resistant Immune Susceptible flags
 */
typedef enum
{
	RIS_FIRE,
	RIS_COLD,
	RIS_ELECTRICITY,
	RIS_ENERGY,
	RIS_BLUNT,
	RIS_PIERCE,
	RIS_SLASH,
	RIS_ACID,
	RIS_POISON,
	RIS_DRAIN,
	RIS_SLEEP,
	RIS_CHARM,
	RIS_HOLD,
	RIS_NONMAGIC,
	RIS_PLUS1,
	RIS_PLUS2,
	RIS_PLUS3,
	RIS_PLUS4,
	RIS_PLUS5,
	RIS_PLUS6,
	RIS_MAGIC,
	RIS_PARALYSIS,
	/* 22 RIS per i Mob */
	MAX_RIS
} resistance_immune_susceptible_types;


/*
 * Tipi di Apply (per gli affects).
 * Utilizzati in #OBJECTS.
 * (RR) Toglierli e utilizzare quelli sotto dopo la conversione.
 */
typedef enum
{
	APPLY_NONE,			/* 0 */

	APPLY_STR,
	APPLY_AGI,
	APPLY_INT,
	APPLY_SPI,
	APPLY_CON,

	APPLY_SEX,
	APPLY_CLASS,
	APPLY_LEVEL,
	APPLY_AGE,			/* uhmmmm, da pensare, direi cmq di tenerlo anche se è una apply abb. "pericoloso" */
	APPLY_HEIGHT,		/* 10 */
	APPLY_WEIGHT,
	APPLY_MANA,
	APPLY_LIFE,
	APPLY_MOVE,
	APPLY_GOLD,
	APPLY_EXP,
	APPLY_AC,
	APPLY_HITROLL,
	APPLY_DAMROLL,
	APPLY_SAVE_POISON,	/* 20 */
	APPLY_SAVE_ROD,
	APPLY_SAVE_PARA,
	APPLY_SAVE_BREATH,
	APPLY_SAVE_SPELL,
	APPLY_EMP,
	APPLY_AFFECT,
	APPLY_RESISTANT,
	APPLY_IMMUNE,
	APPLY_SUSCEPTIBLE,
	APPLY_WEAPONSPELL,	/* 30 */
	APPLY_LCK,			/* (CC) da togliere */
	APPLY_BACKSTAB,
	APPLY_PICK,
	APPLY_TRACK,
	APPLY_STEAL,
	APPLY_SNEAK,
	APPLY_HIDE,
	APPLY_PALM,
	APPLY_DETRAP,
	APPLY_DODGE,		/* 40 */
	APPLY_PEEK,
	APPLY_SCAN,
	APPLY_GOUGE,
	APPLY_SEARCH,
	APPLY_MOUNT,
	APPLY_DISARM,
	APPLY_KICK,
	APPLY_PARRY,
	APPLY_BASH,
	APPLY_STUN,			/* 50 */
	APPLY_PUNCH,
	APPLY_CLIMB,
	APPLY_GRIP,
	APPLY_SCRIBE,
	APPLY_BREW,
	APPLY_WEARSPELL,
	APPLY_REMOVESPELL,
	APPLY_EMOTION,
	APPLY_MENTALSTATE,
	APPLY_STRIPSN,		/* 60 */
	APPLY_REMOVE,
	APPLY_DIG,
	APPLY_FULL,
	APPLY_THIRST,
	APPLY_DRUNK,
	APPLY_BLOOD,
	APPLY_COOK,
	APPLY_RECURRINGSPELL,
	APPLY_CONTAGIOUS,
	APPLY_EXT_AFFECT,	/* 70 */
	APPLY_ODOR,
	APPLY_ROOMFLAG,
	APPLY_SECTORTYPE,
	APPLY_ROOMLIGHT,
	APPLY_TELEVNUM,
	APPLY_TELEDELAY,
	APPLY_SOUL,

	APPLY_SIGHT,		/* aggiunti in coda */
	APPLY_HEARING,
	APPLY_SMELL,		/* 80 */
	APPLY_TASTE,
	APPLY_TOUCH,
	APPLY_SIXTH,

	APPLY_RES,
	APPLY_WIL,
	APPLY_COC,
	APPLY_REF,
	APPLY_SPE,
	APPLY_MIG,
	APPLY_ABS,			/* 90 */
	APPLY_BEA,
	APPLY_LEA,
	MAX_APPLY
} apply_types;

#define REVERSE_APPLY	1000


/*
 * Struttura dati di un Affect
 */
struct affect_data
{
	AFFECT_DATA *next;
	AFFECT_DATA *prev;
	int			 type;
	int			 duration;
	int			 location;
	int			 modifier;
	BITVECTOR	*bitvector;
};


/*
 * Struttura dati affect di uno SMAUG spell
 */
struct smaug_affect
{
	SMAUG_AFF *next;
	SMAUG_AFF *prev;
	char 	  *duration;
	int		   location;
	char 	  *modifier;
	int		   bitvector;	/* this is the bit number */
};


/*
 * Variabili
 */
extern	int					top_affect;
extern	const CODE_DATA		code_affect[];
extern	const CODE_DATA		code_apply[];
extern	const CODE_DATA		code_ris[];
extern	const int			max_check_affect;
extern	const int			max_check_apply;
extern	const int			max_check_ris;


/*
 * Funzioni
 */
void		 affect_modify		( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd );
void		 affect_to_char		( CHAR_DATA *ch, AFFECT_DATA *paf );
void		 affect_remove		( CHAR_DATA *ch, AFFECT_DATA *paf );
void		 affect_strip		( CHAR_DATA *ch, int sn );
void		 affect_join		( CHAR_DATA *ch, AFFECT_DATA *paf );
bool		 is_same_affect		( AFFECT_DATA *aff1, AFFECT_DATA *aff2, bool check_duration );
AFFECT_DATA *copy_affect		( AFFECT_DATA *source );
void		 free_affect		( AFFECT_DATA *aff );
void		 free_smaug_affect	( SMAUG_AFF *saff );
void		 update_aris		( CHAR_DATA *ch );
void		 update_affect		( void );
void		 room_affect		( ROOM_DATA *pRoom, AFFECT_DATA *paf, bool fAdd );
void		 show_affect		( CHAR_DATA *ch, AFFECT_DATA *paf );
bool		 is_affected		( CHAR_DATA *ch, int sn );


#endif	/* AFFECT_H */
