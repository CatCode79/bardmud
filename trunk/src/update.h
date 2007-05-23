#ifndef UPDATE_H
#define UPDATE_H


#define	SECONDS_PER_TICK	70

#define PULSE_IN_SECOND		12
#define PULSE_VIOLENCE		(PULSE_IN_SECOND * 4)					/* ex-3 (TT) e TNN invece 5, (FF) sarebbe bello toglierlo l'update_violence e gestirlo tramite la velocità del pg e dell'arma usata in update_handler */
#define PULSE_MOBILE		(PULSE_IN_SECOND * 6)					/* ex-4 */
#define PULSE_TICK			(PULSE_IN_SECOND * SECONDS_PER_TICK)
#define PULSE_AREA			(PULSE_IN_SECOND * 90)


/*
 * Variabili globali
 */
extern	CHAR_DATA *gch_prev;
extern	OBJ_DATA  *gobj_prev;


/*
 * Funzioni globali
 */
void	update_handler	( void );
void	update_area		( void );
void	remove_portal	( OBJ_DATA *portal );
void	subtract_times	( struct timeval *estime_time, struct timeval *start_time );
void	gain_condition	( CHAR_DATA *ch, int iCond, int value );
void	reboot_check	( time_t reset );


#endif	/* UPDATE_H */
