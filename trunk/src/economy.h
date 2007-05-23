#ifndef ECONOMY_H
#define ECONOMY_H


/* Include per la gestione dell'economia e delle banche */


/*
 * Definizioni
 */
#define ECONOMY_FILE	TABLES_DIR	"economy.txt"	/* Gold looted, value of used potions/pills	*/


/*
 * Mud Economy System
 * Bank Investments
 *
 * Allows players to invest their money in a
 * mud fund, the value of the fund may change from time to time
 * making it possible for players to speculate on a random generator
 * (or if you want to call it this way: the economy)
 */


/*
 * Variabili
 */
extern int	share_value;


/*
 * Funzioni
 */
int		compute_cost				( CHAR_DATA *ch, CHAR_DATA *mob, int cost );
void	boost_economy				( AREA_DATA *tarea, int gold );
void	lower_economy				( AREA_DATA *tarea, int gold );
bool	economy_has					( AREA_DATA *tarea, int gold );
void	economize_mobgold			( CHAR_DATA *mob );


#endif	/* ECONOMY_H */
