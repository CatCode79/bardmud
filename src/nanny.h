#ifndef NANNY_H
#define NANNY_H

/*
 * Definizioni
 */
#define MENU_FILE		SYSTEM_DIR	"menu.file"		/* File contenente il menù					*/
#define TITLE_FILE		SYSTEM_DIR	"mudtitle.file"	/* Schermata di titolo del mud				*/

#define MAX_FACTOR_HEIGHT	0.20	/* Massimo di percentuale di scarto in altezza dal quella razziale */
#define MAX_FACTOR_WEIGHT	0.40	/* Massimo di percentuale di scarto in peso da quella razziale */


/*
 * Connected state for a channel.
 */
typedef enum
{
	CON_GET_NAME,				/* E' alla schermata di avvio */
	CON_GET_PASSWORD,			/* Digitazione della password di un pg già esistente */
	CON_NEW_GET_NAME,			/* Sceglie come modificare il nome */
	CON_NEW_GET_PASSWORD,		/* Inserisce la password di un nuovo pg */
	CON_NEW_CONFIRM_PASSWORD,	/* Conferma la password di un nuovo pg */
	CON_NEW_WANT_SEX,			/* Sceglie il sesso di un nuovo pg */
	CON_NEW_WANT_CREATION,		/* Indica quale tipo di creazione pg si voglia */
	CON_NEW_GET_RACE,			/* Sceglie la razza di un nuovo pg */
	CON_NEW_GET_ALIGNMENT,		/* Tara l'allineamento di un nuovo pg */
	CON_NEW_GET_CLASS,			/* Sceglie la classe di un nuovo pg */
#ifdef T2_QUESTION
	CON_NEW_WANT_QUESTION,		/* Fa scegliere la classe attraverso delle domande */
	CON_NEW_GET_QUESTION,		/* La domanda seguente */
#endif
	CON_NEW_GET_WEIGHT,			/* Scelta del peso di un nuovo pg */
	CON_NEW_GET_HEIGHT,			/* Scelta dell'altezza di un nuovo pg */
	CON_NEW_GET_HAIRCOLOR,		/* Scelta del colore dei capelli di un nuovo pg */
	CON_NEW_GET_HAIRTYPE,		/* Scelta del tipo di capelli di un nuovo pg */
	CON_NEW_GET_HAIRLENGTH,		/* Scelta della lunghezza dei capelli di un nuovo pg */
	CON_NEW_GET_EYECOLOR,		/* Scelta del colore degli occhi di un nuovo pg */
#ifdef T2_ALFA
	CON_NEW_GET_SKINCOLOR,		/* Scelta del colore della pelle di un nuovo pg */
#endif
	CON_NEW_REROLL_ATTRS,		/* Rirollaggio attributi di un nuovo pg */
	CON_NEW_WANT_HAND,			/* Scelta del tipo di mano principale, destra o sinistra? */
	CON_NEW_GET_ARCHETYPE,		/* Scelta dell'archetipo per il carattere del pg */
	CON_NEW_WANT_LANGUAGE,		/* Scelta del set di lingua dei comandi, ita o ing? */
	CON_NEW_WANT_ACCENT,		/* Scelta della tipologia di accenti, e o e'? */
	CON_NEW_WANT_BACK,			/* Scelta di ritorno ad un particolare punto della creazione */
	CON_MENU,					/* Menu di connessione */
	CON_PRESS_ENTER,			/* Premi invio per continuare */
	CON_PLAYING,				/* Indica che la persona connessa sta giocando */
	CON_EDITING,				/* Editor */
	CON_MEDIT,					/* OLC mob  editing */
	CON_OEDIT,					/* OLC obj  editing */
	CON_REDIT,					/* OLC room editing */
	CON_COPYOVER_RECOVER,		/* Recupero dal copyover */
	/* 28 tipi di connessione */
	MAX_CONNECTED
} connections_types;


/*
 * Variabile
 */
extern	const CODE_DATA		code_connected[];
extern	const int			max_check_connected;


/*
 * Funzioni
 */
void	nanny				( DESCRIPTOR_DATA *d, char *argument );
bool	check_parse_name	( char *name, bool newchar );
char   *check_password		( CHAR_DATA *ch, char *argument );
bool	check_class			( CHAR_DATA *ch, const int x );


#endif	/* NANNY_H */
