#ifndef SPECIAL_H
#define SPECIAL_H


/*
 * Tipo di struttura delle funzioni speciali
 */
typedef struct	special_function_data	SPECFUN_DATA;


/*
 * Struttura di una special, serveper lo DLSym Snippet
 */
struct special_function_data
{
	SPECFUN_DATA *next;
	SPECFUN_DATA *prev;
	char		 *name;
};


/*
 * Funzioni
 */
bool		validate_specfun	( char *name );
void		load_specfuns		( void );
void		free_all_specfuns	( void );
SPEC_FUN   *specfun_lookup		( const char *name );


#endif	/* SPECIAL_H */
