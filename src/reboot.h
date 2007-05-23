#ifndef REBOOT_H
#define REBOOT_H


/*
 * Definizione del tipo di struttura per i valori di reboot
 */
typedef struct	hour_min_sec	HOUR_MIN_SEC;


/*
 * Definizioni
 */
#define USAGE_FILE		SYSTEM_DIR "usage.txt"	/* How many people are on every half hour - trying to determine best reboot time */


/*
 * Struttura per i valori di reboot
 */
struct hour_min_sec
{
	int hour;
	int min;
	int sec;
	int manual;
};


/*
 * Variabili
 */
extern	HOUR_MIN_SEC *set_boot_time;
extern	struct tm	 *new_boot_time;
extern	time_t		  boot_time;
extern	time_t		  new_boot_time_t;
extern	char		  reboot_time[];
extern	char		  str_boot_time[MIL];
extern	bool		  mud_down;


/*
 * Funzioni
 */
struct tm	*update_boot_time	( struct tm *old_time );
void		 get_reboot_string	( void );
void		 init_reboot_time	( void );
void		 load_copyover		( void );


#endif	/* REEBOT_H */
