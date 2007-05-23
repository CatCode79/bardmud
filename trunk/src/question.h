#ifdef T2_QUESTION

#ifndef QUESTION_H
#define QUESTION_H


/*
 * Variabili
 */
extern	int	top_question;


/*
 * Funzioni
 */
void	load_questions				( void );
void	free_all_questions			( void );
void	free_question_handler_desc	( DESCRIPTOR_DATA *d );
void	free_all_question_handlers	( void );
int		get_sizeof_question			( void );
void	show_want_question			( DESCRIPTOR_DATA *d );
void	show_get_question			( DESCRIPTOR_DATA *d );
bool	check_get_question			( DESCRIPTOR_DATA *d, char *argument );
bool	get_another_question		( DESCRIPTOR_DATA *d );
int		answers_elaboration			( DESCRIPTOR_DATA *d );


#endif	/* QUESTION_H */

#endif	/* T2_QUESTION */
