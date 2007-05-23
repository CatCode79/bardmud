#ifndef FIGTH_H
#define FIGTH_H


#define MAX_FIGHT		10		/* Numero massimo di combattenti addosso ad uno, il calcolo viene dalla space_to_fight(), questo è il limite massimo supportatabile */
#define MAX_KILLTRACK	30		/* Track mob vnums killed */
#define	SPIRIT_TIME		(PULSE_IN_SECOND * 360)	/* Tempo totale in cui si è spirito e metà-spirito dopo la morte */


/*
 * Attack types
 */
typedef enum
{
	ATTACK_BITE,
	ATTACK_CLAWS,
	ATTACK_TAIL,
	ATTACK_STING,
	ATTACK_PUNCH,
	ATTACK_KICK,
	ATTACK_TRIP,
	ATTACK_BASH,
	ATTACK_STUN,
	ATTACK_GOUGE,
	ATTACK_BACKSTAB,
	ATTACK_FEED,				/* Attaco tipico del vampiro */
	ATTACK_DRAIN,
	ATTACK_FIREBREATH,
	ATTACK_FROSTBREATH,
	ATTACK_ACIDBREATH,
	ATTACK_LIGHTNBREATH,
	ATTACK_GASBREATH,
	ATTACK_POISON,
	ATTACK_NASTYPOISON,
	ATTACK_GAZE,
	ATTACK_BLINDNESS,
	ATTACK_CAUSESERIOUS,
	ATTACK_EARTHQUAKE,
	ATTACK_CAUSECRITICAL,
	ATTACK_CURSE,
	ATTACK_FLAMESTRIKE,
	ATTACK_HARM,
	ATTACK_FIREBALL,
	ATTACK_COLORSPRAY,
	ATTACK_WEAKEN,
	ATTACK_SPIRALBLAST,
	/* 32 tipi di attacco */
	MAX_ATTACK
} attack_types;


/*
 * Defense types
 */
typedef enum
{
	DEFENSE_PARRY,
	DEFENSE_DODGE,
	DEFENSE_HEAL,
	DEFENSE_CURELIGHT,
	DEFENSE_CURESERIOUS,
	DEFENSE_CURECRITICAL,
	DEFENSE_DISPELMAGIC,
	DEFENSE_DISPELEVIL,
	DEFENSE_SANCTUARY,
	DEFENSE_FIRESHIELD,
	DEFENSE_SHOCKSHIELD,
	DEFENSE_SHIELD,
	DEFENSE_BLESS,
	DEFENSE_STONESKIN,
	DEFENSE_TELEPORT,
	DEFENSE_MONSUM1,
	DEFENSE_MONSUM2,
	DEFENSE_MONSUM3,
	DEFENSE_MONSUM4,
	DEFENSE_DISARM,
	DEFENSE_ICESHIELD,
	DEFENSE_GRIP,
	DEFENSE_TRUESIGHT,
	DEFENSE_ACIDMIST,
	DEFENSE_VENOMSHIELD,
	/* 25 tipi di difesa */
	MAX_DEFENSE
} defense_types;


/*
 * Tipologie di trappole
 */
typedef enum
{
	DEATH_NORMAL,
	DEATH_TRAP,
	DEATH_ARENA
} death_types;


struct hunt_hate_fear
{
	char	  *name;
	CHAR_DATA *who;
};


struct fighting_data
{
	CHAR_DATA *who;
	int		   xp;
	int		   align;
	int		   duration;
	int		   timeskilled;
};


struct killed_data
{
	VNUM		vnum;
	char		count;
};


/*
 * Variabili
 */
extern	int				last_pkroom;
extern	const CODE_DATA	code_attack[];
extern	const CODE_DATA	code_defense[];
extern	const int		max_check_attack;
extern	const int		max_check_defense;


/*
 * Funzioni
 */
void		update_violence	( void );
void		free_fight		( CHAR_DATA *ch );
int			ris_damage		( CHAR_DATA *ch, int dam, int ris );
void		start_fearing	( CHAR_DATA *ch, CHAR_DATA *victim );
void		death_cry		( CHAR_DATA *ch );
ch_ret		one_hit			( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
ch_ret		projectile_hit	( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield, OBJ_DATA *projectile, int dist );
bool		space_to_fight	( CHAR_DATA *ch, CHAR_DATA *victim );
int			align_compute	( CHAR_DATA *gch, CHAR_DATA *victim );
bool		legal_loot		( CHAR_DATA *ch, CHAR_DATA *victim );
bool		is_hating		( CHAR_DATA *ch, CHAR_DATA *victim );
bool		is_fearing		( CHAR_DATA *ch, CHAR_DATA *victim );
ch_ret		multi_hit		( CHAR_DATA *ch, CHAR_DATA *victim, int dt );
ch_ret		damage			( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt );
void		update_position		( CHAR_DATA *victim );
void		set_fighting	( CHAR_DATA *ch, CHAR_DATA *victim );
void		stop_fighting	( CHAR_DATA *ch, bool fBoth );
CHAR_DATA  *who_fighting	( CHAR_DATA *ch );
void		check_attacker	( CHAR_DATA *ch, CHAR_DATA *victim );
void		stop_hunting	( CHAR_DATA *ch );
void		stop_hating		( CHAR_DATA *ch );
void		stop_fearing	( CHAR_DATA *ch );
bool		is_safe			( CHAR_DATA *ch, CHAR_DATA *victim, bool SHOW );
bool		can_pkill		( CHAR_DATA *ch, CHAR_DATA *victim );
bool		is_illegal_pk	( CHAR_DATA *ch, CHAR_DATA *victim );
void		death_penalty	( CHAR_DATA *ch, const int type );
void		raw_kill		( CHAR_DATA *ch, CHAR_DATA *victim );


#endif	/* FIGHT_H */
