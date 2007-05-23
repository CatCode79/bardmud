#ifndef DREAM_H
#define DREAM_H


/****************************************************************************\
 >							File Header dei Sogni							<
\****************************************************************************/


/*
 * Definizioni
 */
#define MAX_DREAM_PART	20							/* Numero massimo di parti di un sogno */
#define MAX_DREAM_NEXT	4							/* Numero massimo di scelte possibile per la parte sucessiva */


/*
 * Struttura di un Sogno
 */
struct dream_data
{
	DREAM_DATA	*prev;
	DREAM_DATA	*next;
	char		*name;					/* Nome che serve per richiamarlo ai pg */
	bool		 nightmare;				/* Il dream è un incubo? */
	BITVECTOR	*classes;				/* Classi a cui si può inviare il dream, se vuoto a tutte */
	BITVECTOR	*races;					/* Razza a cui si può inviare il dream, se vuoto a tutte */
	int			 sex;					/* Indica a quale sesso inviare, se -1 a nessuno */
	int			 num_part;				/* Numero di parti composto da un dream */
	char		*part[MAX_DREAM_PART];	/* Testo di una parte (RR) convertirla in lista */
	int			 next_part[MAX_DREAM_PART][MAX_DREAM_NEXT];	/* Numero di possibili parti da inviare dopo quella appena inviata (RR) convertirla in lista */
};


/*
 * Variabili
 */
extern	int	top_dream;


/*
 * Funzioni
 */
void		load_dreams		( void );
void		free_all_dreams	( void );
DREAM_DATA *get_dream		( const char *str, bool exact );
void		dream_reset		( CHAR_DATA *ch );
void		update_dream	( void );


#endif	/* DREAM_H */
