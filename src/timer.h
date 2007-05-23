#ifndef TIMER_H
#define TIMER_H


/*
 * Enumerazione dei tipi di timer
 */
typedef enum
{
	TIMER_NONE,
	TIMER_RECENTFIGHT,
	TIMER_SHOVEDRAG,
	TIMER_DO_FUN,
	TIMER_APPLIED,
	TIMER_KILLED,		/* Tempo da quando si è stati uccisi da un mob */
	TIMER_PKILLED,		/* Tempo da quando si è stati uccisi da un pg */
	TIMER_ASUPRESSED
	/* 7 tipi di timer */
} timers_types;


/*
 * Struttura di un timer
 */
struct timer_data
{
	TIMER  *prev;
	TIMER  *next;
	DO_FUN *do_fun;
	int		value;
	int		type;		/* Tipo di timer */
	int		pulse;		/* Durata del timer, in pulse del mud */
};


/*
 * Funzioni
 */
void	add_timer				( CHAR_DATA *ch, int type, int pulse, DO_FUN *fun, int value );
TIMER  *get_timerptr			( CHAR_DATA *ch, int type );
int		get_timer				( CHAR_DATA *ch, int type );
void	extract_timer			( CHAR_DATA *ch, TIMER *timer );
void	remove_timer			( CHAR_DATA *ch, int type );
void	update_timer			( void );
bool	is_attack_supressed		( CHAR_DATA *ch );
bool	check_delayed_command	( CHAR_DATA *ch );


#endif /* TIMER_H */
