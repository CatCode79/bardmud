#ifndef BAN_H
#define BAN_H


/*
 * Definizioni dei tipi di Ban
 */
#define BAN_SITE	 1
#define BAN_CLASS	 2
#define BAN_RACE	 3
#define BAN_WARN	-1


/*
 * Funzioni
 */
void	load_bans		( void );
void	free_all_bans	( void );
bool	check_bans		( CHAR_DATA *ch, int type );
bool	check_total_bans( DESCRIPTOR_DATA *d );


#endif	/* BAN_H */
