#ifndef LANGUAGE_H
#define LANGUAGE_H


/****************************************************************************\
 > 						Header riguardante le lingue						<
\****************************************************************************/


/*
 * Tipi di struttura
 */
typedef struct	language_data				LANG_DATA;		/* Lingua */
typedef struct	language_conversion_data	LANG_CNV_DATA;	/* Lista di conversione */


/*
 * Enumerazione delle lingue
 * Da tenere in ordine dalla più parlata alla meno parlata
 *	(rpgicamente, a seconda del bg del proprio mud)
 *	perché così la funzione get_lang_speakable richiama prima quelle più parlate
 */
typedef enum
{
	LANGUAGE_UNKNOWN = -1,		/* Indica una lingua non trovata oppure					*/
	LANGUAGE_COMMON,			/* Lingua base di molte razze intelligenti e giocabili	*/
	LANGUAGE_ELVEN,				/* Lingua base degli Elfi								*/
	LANGUAGE_DWARVEN,			/* Lingua base dei Nani									*/
	LANGUAGE_PIXIE,				/* Lingua base di Pixie e Fate							*/
	LANGUAGE_OGRE,				/* Lingua base degli Ogre								*/
	LANGUAGE_ORCISH,			/* Lingua base degli Orchi								*/
	LANGUAGE_TROLLISH,			/* Lingua base dei Troll								*/
	LANGUAGE_RODENT,			/* Lingua base dei roditori								*/
	LANGUAGE_INSECTOID,			/* Lingua base degli insetti, dei ragni e anellidi		*/
	LANGUAGE_MAMMAL,			/* Lingua base dei Mammiferi							*/
	LANGUAGE_REPTILE,			/* Lingua base dei Rettili								*/
	LANGUAGE_DRAGON,			/* Lingua base dei Draghi e dei draconiani				*/
	LANGUAGE_DEATH,				/* Lingua base degli spiriti e dei non-morti			*/
	LANGUAGE_MAGICAL,			/* Lingua base delle creature magiche					*/
	LANGUAGE_GOBLIN,			/* Lingua base dei Goblin								*/
	LANGUAGE_ANCIENT,			/* Lingua degli antichi, una razza sconosciuta			*/
	LANGUAGE_HALFLING,			/* Lingua base degli Halfling							*/
	LANGUAGE_CLAN,				/* Rappresenta le diverse lingue dei clan				*/
	LANGUAGE_GITH,				/* Lingua base dei Gith									*/
	LANGUAGE_GNOME,				/* Lingua base degli Gnomi								*/
	LANGUAGE_DEMON,				/* Lingua base dei Demoni								*/
	LANGUAGE_GNOLLISH,			/* Lingua base degli Gnoll								*/
	LANGUAGE_DROW,				/* Lingua base dei Drow									*/
	LANGUAGE_CANINE,			/* Lingua base dei Canidi								*/
	LANGUAGE_FELINE,			/* Lingua base dei Felini								*/
	LANGUAGE_AVIS,				/* Lingua base dei Volatili								*/
	LANGUAGE_MARINE,			/* Lingua base dei Pesci e del mondo dell'acqua			*/
	LANGUAGE_TREELEAF,			/* Lingua base dei vegetali, alberi e Ent				*/
	LANGUAGE_MONSTER,			/* Lingua di mostri o non apprendibile					*/
	/* 28 Languages */
	MAX_LANGUAGE
} languages_types;


/*
 * Strutture dei linguaggi
 */
struct language_conversion_data
{
	LANG_CNV_DATA *next;
	LANG_CNV_DATA *prev;
	char		  *old;
	int			   olen;
	char 		  *new;
	int			   nlen;
};

struct language_data
{
	char 	 	  *name;			/* Nome della lingua */
	bool	 	   valid;			/* Se può essere appresa dal pg normalmente	*/
	LANG_CNV_DATA *first_precnv;	/* Lista di parole per la preconversione */
	LANG_CNV_DATA *last_precnv;
	char		  *alphabet;		/* Alfabeto per lo scrambling */
	LANG_CNV_DATA *first_cnv;		/* Lista di parole per la conversione */
	LANG_CNV_DATA *last_cnv;
};


/*
 * Variabili esterne
 */
extern	LANG_DATA		   *table_language[MAX_LANGUAGE];	/* (FF) convertirlo in lista (TT) chissà perchè poi, dovrò ripensarci */
extern	const CODE_DATA		code_language[];
extern	const int			max_check_language;


/*
 * Funzioni esterne
 */
void		load_languages		( void );
void		free_all_languages	( void );
LANG_DATA  *get_lang			( const char *name );
int			get_lang_num		( const char *name );
int			get_lang_speakable	( CHAR_DATA *ch, CHAR_DATA *victim );
int			knows_language		( CHAR_DATA *ch, int language, CHAR_DATA *cch );
bool		can_learn_lang		( CHAR_DATA *ch, int language );
void		learn_language		( CHAR_DATA *ch, const int language );
char	   *translate			( int percent, const char *in, const int lang );
char	   *scramble			( const char *argument, int modifier );


#endif	/* LANGUAGE_H */
