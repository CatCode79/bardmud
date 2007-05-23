#ifndef QUOTE_H
#define QUOTE_H


/************************************************************************************\
 > 						Header del Modulo delle Citazioni							<
\************************************************************************************/


/*
 * Tipo di struttura Quote
 */
typedef struct  quote_data		QUOTE_DATA;


/*
 * Struttura di un Quote
 */
struct quote_data
{
	QUOTE_DATA	*prev;
	QUOTE_DATA	*next;
	char		*author;	/* Autore del Quote, è facoltativo inserirlo */
	char		*text;		/* Testo del Quote */
};


/*
 * Variabili
 */
extern	int	top_quote;


/*
 * Funzioni
 */
void	load_quotes		( void );
void	free_all_quotes	( void );
void	send_quote		( CHAR_DATA *ch );


#endif	/* QUOTE_H */
