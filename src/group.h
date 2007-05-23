#ifndef GROUP_H
#define GROUP_H


/****************************************************************************\
 >								Modulo dei negozi							<
\****************************************************************************/


/*
 * Funzioni
 */
int		count_group		( CHAR_DATA *ch );
int		count_group_room( CHAR_DATA *ch );
void	die_follower	( CHAR_DATA *ch );
bool	circle_follow	( CHAR_DATA *ch, CHAR_DATA *victim );
void	add_follower	( CHAR_DATA *ch, CHAR_DATA *master );
void	stop_follower	( CHAR_DATA *ch );
bool	is_same_group	( CHAR_DATA *ach, CHAR_DATA *bch );


#endif	/* GROUP_H */
