#ifndef SPELL_H
#define SPELL_H

#ifdef T2_ALFA
/*
 * Serve come moltiplicatore della durata di un affect
 * (GG) da aggiungere nel futuro sistema di affect skill e spell
 */
#define DUR_CONV	4
#endif


#define HAS_SPELL_INDEX		-1


/*
 * Variabili globali
 */
extern	char			*target_name;
extern	char			*target_name_ranged;
extern	const CODE_DATA  code_spellaction[];
extern	const CODE_DATA  code_spelldamage[];
extern	const CODE_DATA  code_spellpower[];
extern	const CODE_DATA  code_spellsave[];
extern	const CODE_DATA  code_spellsphere[];
extern	const int		 max_check_spellaction;
extern	const int		 max_check_spelldamage;
extern	const int		 max_check_spellpower;
extern	const int		 max_check_spellsave;
extern	const int		 max_check_spellsphere;


/*
 * Funzioni
 */
void		load_spells				( void );
SPELL_FUN  *spell_function			( const char *name );
bool		is_spelldamage			( const int sn, const int sd );
SPELL_RET	spell_energy_drain		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_fire_breath		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_frost_breath		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_acid_breath		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_lightning_breath	( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_gas_breath		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_spiral_blast		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_poison			( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_blindness			( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_cause_serious		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_earthquake		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_cause_critical	( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_curse				( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_flamestrike		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_harm				( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_fireball			( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_colour_spray		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_weaken			( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_smaug				( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_dispel_magic		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_dispel_evil		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_teleport			( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_cure_blindness	( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_cure_poison		( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_remove_curse		( int sn, int level, CHAR_DATA *ch, void *vo );
void		identify_object			( CHAR_DATA *ch, OBJ_DATA *obj );
int			dice_parse				( CHAR_DATA *ch, int level, char *experience );
bool		can_cast				( CHAR_DATA *ch );
bool		can_charm				( CHAR_DATA *ch );
bool		SPELL_FLAG				( SKILL_DATA *skill, int flag );
void		failed_casting			( SKILL_DATA *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj );
int			find_spell				( CHAR_DATA *ch, const char *name, bool know );
int			ris_save				( CHAR_DATA *ch, int ris_chance, int ris );
bool		process_components		( CHAR_DATA *ch, int sn );
ch_ret		spell_attack			( int, int, CHAR_DATA *, void * );
SPELL_RET	spell_null				( int sn, int level, CHAR_DATA *ch, void *vo );
SPELL_RET	spell_notfound			( int sn, int level, CHAR_DATA *ch, void *vo );
int			ch_slookup				( CHAR_DATA *ch, const char *name );
int			skill_lookup			( const char *name );
int			slot_lookup				( int slot );
int			bsearch_skill_exact		( const char *name, int first, int top );
bool		compute_savthrow		( int level, CHAR_DATA *victim, int saving );
ch_ret		obj_cast_spell			( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj );
SPELL_RET	spell_word_of_recall	( int sn, int level, CHAR_DATA *ch, void *vo );
void		successful_casting		( SKILL_DATA *skill, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj );


#endif	/* SPELL_H */
