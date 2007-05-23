#ifndef CALENDAR_H
#define CALENDAR_H


/*
 * Definizioni
 */
/* Quanto viene diviso il tempo reale per ottenere queello del Mud */
/*#define TIME_FACTOR			24*/	/* Inutilizzato per ora, cmq sempre utile saperlo */

#define DAYS_IN_WEEK		 7		/* Giorni in una settimana		*/
#define MONTHS_IN_YEAR		12		/* Mesi-mud in un anno-mud		*/
#define DAYS_IN_MONTH		30		/* Giorni-mud in un mese-mud	*/
#define HOURS_IN_DAY		24		/* Ore-mud in un giorno-mud		*/
#define SECONDS_IN_HOUR		150		/* Secondi reali in un'ora-mud	*/

#define HOURS_IN_YEAR		(HOURS_IN_DAY * DAYS_IN_MONTH * MONTHS_IN_YEAR)
#define SECONDS_IN_DAY		(SECONDS_IN_HOUR * HOURS_IN_DAY)	/*									Secondi reali di un giorno-mud	*/
#define SECONDS_IN_MONTH	(SECONDS_IN_HOUR * HOURS_IN_DAY * DAYS_IN_MONTH)	/*					Secondi reali di un mese-mud	*/
#define SECONDS_IN_YEAR		(SECONDS_IN_HOUR * HOURS_IN_DAY * DAYS_IN_MONTH * MONTHS_IN_YEAR)	/*	Secondi reali di un anno-mud	*/

/* Ore importanti */
#define HOUR_SUNRISE	 6
#define HOUR_NOON		12
#define HOUR_SUNSET		18
#define HOUR_MIDNIGHT	24


/*
 * Tipi di tempo utilizzati nella get_age()
 */
typedef enum
{
	TIME_YEAR,
	TIME_MONTH,
	TIME_DAY
} calendar_types;


/*
 * Variabili e strutture per gli anniversari
 */
typedef struct	anniversary_data	ANNIVERSARY_DATA;

extern	ANNIVERSARY_DATA	*first_anniversary;
extern	ANNIVERSARY_DATA	*last_anniversary;

struct anniversary_data
{
	ANNIVERSARY_DATA	*next;
	ANNIVERSARY_DATA	*prev;
	char				*name;		/* Breve stringa dell'anniversario, fatto storico o persona che sia */
	char				*message;	/* Messaggio descrittivo dell'anniversario inviato al pg */
	bool				 festivity;	/* Se il giorno è festa non indica quanti anni fa capitò, e non richiede lo year */
	int					 day;		/* Giorno dell'anniversario */
	int					 month;		/* Mese dell'anniversario */
	int					 year;		/* Anno dell'anniversario */
#ifdef T2_ALFA
	char				*nations;	/* Elenco nazione o nazioni che festeggiano l'anniversario */
#endif
	BITVECTOR			*races;		/* Razza o razze che festeggiano il tal anniversario */
};


/*
 * Struttura di un calendario
 */
struct calendar_data
{
	int		hour;
	int		day;
	int		month;
	int		year;
	int		sunlight;
};


/*
 * Variabili
 */
extern	CALENDAR_DATA	 calendar;
extern	const char		*day_name[DAYS_IN_WEEK];
extern	const char		*month_name[MONTHS_IN_YEAR];


/*
 * Funzioni
 */
void	load_calendar			( void );
void	update_calendar			( void );
char   *get_calendar_string		( void );
void	free_all_anniversaries	( void );
int		get_hours_played		( CHAR_DATA *ch );


#endif	/* CALENDAR_H */
