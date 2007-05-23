#ifndef HERB_H
#define HERB_H


/****************************************************************************\
 >								Header delle Erbe						    <
\****************************************************************************/


/*
 * Definizioni globali
 */
#define MAX_HERB	20			/* Limite di erbette caricabili			*/


/*
 * Pipe flags
 */
typedef enum
{
	PIPE_TAMPED,
	PIPE_LIT,
	PIPE_HOT,
	PIPE_DIRTY,
	PIPE_FILTHY,
	PIPE_GOINGOUT,
	PIPE_BURNT,
	PIPE_FULLOFASH,
	/* 8 pipe flags */
	MAX_PIPE
} pipe_flags;


/*
 * Variabili globali
 */
extern	int				 top_herb;
extern	SKILL_DATA		*table_herb[MAX_HERB];
extern	const CODE_DATA  code_pipe[];
extern	const int		 max_check_pipe;


/*
 * Funzioni globali
 */
void		load_herbs		( void );
void		free_all_herbs	( void );
bool		is_valid_herb	( int sn );
int			herb_lookup		( const char *name );


#endif	/* HERB_H */
