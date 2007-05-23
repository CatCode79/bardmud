#ifdef T2_LOCKER

#ifndef LOCKER_H
#define LOCKER_H


/*
 * Struttura di un locker
 */
typedef struct locker_data	LOCKER_DATA;

struct locker_data
{
	LOCKER_DATA *next;
	LOCKER_DATA *prev;
	char		*owner;			/* nome del proprietario del locker */
	OBJ_DATA	*first_object;	/* lista oggetti nel locker */
	OBJ_DATA	*last_object;
};


/*
 * Variabili
 */
extern	int		top_locker;


/*
 * Funzioni
 */
void	load_lockers			( void );
void	fwrite_locker			( CHAR_DATA *ch, LOCKER_DATA *locker );
void	remove_all_file_locker	( CHAR_DATA *ch );
void	remove_file_locker		( LOCKER_DATA *locker );
void	rename_locker			( CHAR_DATA *ch, const char *newname );
void	free_all_lockers		( void );


#endif	/* LOCKER_H */

#endif	/* T2_LOCKER */
