#ifndef ARCHETYPE_H
#define ARCHETYPE_H


/*
 * Definizioni
 */
#define	MAX_ARCHETYPE	34		/* Numero massimo di archetipi */


/*
 * Funzioni
 */
char   *get_archetype		( CHAR_DATA *ch );
int		get_archetype_num	( const char *argument );
int		get_sizeof_archetype( void );
void	show_archetype_list	( CHAR_DATA *ch );


#endif	/* ARCHETYPE_H */
