#ifndef RESET_H
#define RESET_H


/*
 * Reset commands:
 *	'*': comment
 *	'M': read a mobile
 *	'O': read an object
 *	'P': put object in object
 *	'G': give object to mobile
 *	'E': equip object to mobile
 *	'H': hide an object
 *	'B': set a bitvector
 *	'T': trap an object
 *	'D': set state of door
 *	'R': randomize room exits
 *	'S': stop (end of list)
 */


/*
 * Definizioni per arg2 del comando reset 'B'
 */
#define	BIT_RESET_DOOR				0
#define BIT_RESET_OBJECT			1
#define BIT_RESET_MOBILE			2
#define BIT_RESET_ROOM				3
#define BIT_RESET_TYPE_MASK			0xFF		/* 256 dovrebbe essere abbastanza */

#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK			0xFF00		/* 256 dovrebbe essere abbastanza */

#define BIT_RESET_SET				30
#define BIT_RESET_TOGGLE			31

#define BIT_RESET_FREEBITS			0x3FFF0000	/* Per riferimento */


/*
 * Struttura di un Reset
 */
struct reset_data
{
	RESET_DATA *next;		/* Reset precedente nella lista */
	RESET_DATA *prev;		/* Resete seguente nella lista */
	char		command;	/* Tipo di comando del reset */
	int			extra;
/*	BITVECTOR  *flags; */	/* (GG) E' l'extra utilizzato come bitvector nello smaug, per esempio per le flag di trappola */
	int			arg1;
	int			arg2;
	int			arg3;
};


/*
 * Variabili
 */
extern	int		top_reset;


/*
 * Funzioni
 */
int			generate_itemlevel	( AREA_DATA *pArea, OBJ_DATA *pObjIndex );
void		delete_reset		( AREA_DATA *pArea, RESET_DATA *pReset );
bool		is_room_reset		( RESET_DATA *pReset, ROOM_DATA *aRoom, AREA_DATA *pArea );
RESET_DATA *make_reset			( char letter, int extra, int arg1, int arg2, int arg3 );
void		renumber_put_resets	( AREA_DATA *pArea );
void		fread_reset			( AREA_DATA *tarea, MUD_FILE *fp );
RESET_DATA *add_reset			( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 );
void		reset_area			( AREA_DATA * pArea );


#endif	/* RESET_H */
