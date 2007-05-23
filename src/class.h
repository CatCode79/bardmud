#ifndef CLASS_H
#define CLASS_H


/****************************************************************************\
 >						Header per la Gestione Classi						<
\****************************************************************************/


/*
 * Definizione della struttura di una classe
 */
typedef struct	class_data			CLASS_DATA;


/* Lunghezza max per il nome della classe */
#define MAX_LEN_NAME_CLASS	19


/*
 * Enumerazione delle classi
 */
typedef enum
{
	CLASS_NONE = -1,

	CLASS_MAGE,			/* Mago			*/
	CLASS_PRIEST,		/* Sacerdote	*/
	CLASS_THIEF,		/* Ladro		*/
	CLASS_FIGHTER,		/* Combattente	*/
	CLASS_DRUID,		/* Druido		*/
	CLASS_WANDERING,	/* Ramingo		*/
	CLASS_PALADIN,		/* Paladino		*/

	CLASS_ALCHEMIST,	/* Alchimista	(GG) */
	CLASS_NECROMANCER,	/* Necromante	(GG) */
	CLASS_BARD,			/* Bardo		(GG) */

	CLASS_09,			/* (CC) Devo tenerle per mantenere la compatibilità con le classi mob */
	CLASS_10,
	CLASS_11,
	CLASS_12,
	CLASS_13,
	CLASS_14,
	CLASS_15,
	CLASS_16,
	CLASS_17,
	CLASS_18,
	CLASS_19,

	CLASS_BAKER,		/* (FF) npc_smaug da qui */
	CLASS_BUTCHER,
	CLASS_BLACKSMITH,
	CLASS_MAYOR,
	CLASS_KING,
	CLASS_QUEEN,		/* a qui */

	MAX_CLASS
} classes_types;


/*
 * Enumerazione delle flag di classe
 */
typedef enum
{
	CLASSFLAG_PLAYABLE,
	CLASSFLAG_CASTER,
	MAX_CLASSFLAG
} classflags_types;


/*
 * Struttura di una classe.
 */
struct class_data
{
	char	   *name_male;					/* Nomi al maschile per allineamento	*/
	char	   *name_fema;					/* Nomi al femminile per allineamento	*/
	SYNO_DATA  *color;						/* Sinonimo del colore se				*/
	BITVECTOR  *flags;						/* Flags di classe						*/
	int			exp_base;					/* Esperienza di base per avanzare		*/
	int			thac0_00;					/* Thac0 per il livello					*/
	int			thac0_32;					/* Thac0 per il livello 32				*/
	int			avarage_attr[MAX_ATTR];		/* Attributi modello per i max e min	*/
	char	   *points_name[MAX_POINTS];	/* Nomi dei punti						*/
	int			regain_base[MAX_POINTS];	/* Regain di base (GG)					*/
};


/*
 * Variabili globali
 */
extern	CLASS_DATA		   *table_class[MAX_CLASS];
extern	int					top_class_play;
extern	const CODE_DATA		code_class[];
extern	const CODE_DATA		code_classflag[];
extern	const int			max_check_class;
extern	const int			max_check_classflag;


/*
 * Funzioni globali
 */
void		 load_classes	 ( void );
void		 free_all_classes( void );
char		*get_class		 ( CHAR_DATA *ch, bool color );
int			 get_class_num	 ( const char *type, bool exact );
CLASS_DATA	*is_name_class	 ( const char *argument, bool exact );


#endif	/* CLASS_H */
