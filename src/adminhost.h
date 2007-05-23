#ifndef ADMHOST_H
#define ADMHOST_H


/*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*\
 >							Header del Modulo di Admin Host								<
\*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*/


/*
 * Variabili
 */
extern	int top_adminhost;


/*
 * Funzioni
 */
void	load_admhosts		( void );
void	free_all_adminhosts	( void );
int		get_sizeof_adminhost( void );
bool	check_adm_domain	( const char *name, const char *host );
void	add_ip_to_list		( CHAR_DATA *ch );


#endif	/* ADMHOST_H */
