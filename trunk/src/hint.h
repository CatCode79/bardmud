#ifndef HINT_H
#define HINT_H


/*
 * Definizione del tipo di hint
 */
typedef struct	hint_data			HINT_DATA;


/*
 * Struttura di un Hint
 */
struct hint_data
{
	HINT_DATA	*prev;
	HINT_DATA	*next;
	int			 level;		/* Livello minimo di invio dell'hint */
	int			 trust;		/* Fiducia minima per la lettura dell'hint */
	BITVECTOR	*classes;	/* Flag di classe che possono leggere l'hint */
	BITVECTOR	*races;		/* Flag di razza che possono leggere l'hint */
	char		*text;		/* Testo dell'hint */
};


/*
 * Variabili
 */
extern	int	top_hint;


/*
 * Funzioni
 */
void	load_hints		( void );
void	free_all_hints	( void );
void	update_hint		( void );


#endif	/* HINT_H */
