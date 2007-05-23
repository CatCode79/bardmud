#ifndef EXPERIENCE_H
#define EXPERIENCE_H


/* Definizioni */
#define LVL_NEWBIE			10		/* Livelli da niubbo */
#define LVL_LEGEND		   100		/* Livello delle leggende */


/*
 * Funzioni
 */
int		get_exp_worth	( CHAR_DATA *ch );
int		exp_level		( CHAR_DATA *ch, int level );
void	advance_level	( CHAR_DATA *ch );
void	gain_exp		( CHAR_DATA *ch, int gain, bool dream );
void	exp_death_hole	( CHAR_DATA *ch );
int		xp_compute		( CHAR_DATA *gch, CHAR_DATA *victim );
void	group_gain		( CHAR_DATA *ch, CHAR_DATA *victim );
void	exp_for_damage	( CHAR_DATA *ch, CHAR_DATA *victim, const int damage, const int damtype );


#endif	/* EXPERIENCE_H */
