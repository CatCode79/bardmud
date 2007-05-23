#ifndef TRACK_H
#define TRACK_H

/*
 * Funzioni
 */
void	start_hunting	( CHAR_DATA *ch, CHAR_DATA *victim );
void	start_hating 	( CHAR_DATA *ch, CHAR_DATA *victim );
void	hunt_victim		( CHAR_DATA *ch );
void	found_prey		( CHAR_DATA *ch, CHAR_DATA *victim );


#endif	/* TRACK_H */
