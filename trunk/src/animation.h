#ifndef ANIMATION_H
#define ANIMATION_H


/*
 * Variabili
 */
extern	int	top_animation;


/*
 * Funzioni
 */
void	load_animations		( void );
void	free_all_animations	( void );
int		get_sizeof_animation( void );
void	init_desc_ani		( DESCRIPTOR_DATA *d );
bool	want_animation		( DESCRIPTOR_DATA *d );
void	update_animation	( void );


#endif	/* ANIMATION_H */
