#ifndef INTERPRET_H
#define INTERPRET_H


/*
 * Tipo di lingua per i comandi.
 * Utilizzati come argomenti passati alla funzione send_command() in interpret.c
 */
typedef enum
{
	IT,		/* Comando inviato in italiano								*/
	EN,		/* Comando inviato in inglese								*/
	CO,		/* Comandi inviati all'interpret dal mud, in inglese		*/
	TR		/* Comando inviato per ottenere il corrispondente tradotto	*/
} interpret_lang_types;


/*
 * Funzioni
 */
void	save_lastcmd		( void );
char   *change_command_tag	( char *string, CHAR_DATA *ch );
bool	check_position		( CHAR_DATA *ch, int position );
char   *send_command		( CHAR_DATA *ch, char *argument, const int cmd_lang );
char   *translate_command	( CHAR_DATA *ch, char *argument );
void	interpret			( CHAR_DATA *ch, char *argument );
bool	is_number			( char *arg );
int		number_argument		( char *argument, char *arg );
void	start_timer			( struct timeval *start_time );
time_t	end_timer			( struct timeval *start_time );
void	send_timer			( struct timerset *vtime, CHAR_DATA *ch );
void	update_userec		( struct timeval *time_used, struct timerset *userec );


/*
 * Variabili
 */
extern char lastcmd_typed[MIL*2];


#endif	/* INTERPRET_H */
