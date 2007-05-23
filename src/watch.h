#ifndef WATCH_H
#define WATCH_H


/****************************************************************************\
 > 						Header riguardante i watch							<
\****************************************************************************/


/*
 * Tipo di struttura Watch
 */
typedef struct	watch_data		WATCH_DATA;


/* The watch directory contains a maximum of one file for each admin
 * that contains output from "player watches". The name of each file
 * in this directory is the name of the admin who requested the watch
 */
#define WATCH_DIR		"../watch/"		/* Dir delle sorveglianze degli amministratori	*/
#define WATCH_LIST		"watch.lst"		/* Lista sorveglianze							*/


/*
 * Struttura di un watch
 */
struct watch_data
{
	WATCH_DATA *next;
	WATCH_DATA *prev;
	int			adm_trust;
	char 	   *admin_name;		/* adm doing the watching 		   */
	char 	   *target_name;	/* player or command being watched */
	char 	   *player_site;	/* site being watched			   */
};


/*
 * Variabili esterne
 */
extern	WATCH_DATA		   *first_watch;
extern	WATCH_DATA		   *last_watch;


/*
 * Funzioni esterne
 */
void	load_watchlist		( void );
void	free_all_watchs		( void );
bool	valid_watch			( CHAR_DATA *ch, char *logline );
void	write_watch_files	( CHAR_DATA *ch, COMMAND_DATA *cmd, char *logline );
bool	chk_watch			( int player_level, char *player_name, char *player_site );


#endif	/* WATCH_H */
