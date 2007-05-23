#ifndef SOCIAL_H
#define SOCIAL_H


/****************************************************************************\
 > 		Header riguardante caricamento e gestione dei dei sociali			<
\****************************************************************************/


/*
 * Tipo di struttura Social
 */
typedef struct	social_data				SOCIAL_DATA;
typedef struct	social_messages_data	SOCIALMSG_DATA;


#ifdef T2_ALFA
/* Forse convertirli in flag */
typedef enum
{
	SOCIAL_SONOROUS,
	SOCIAL_GESTURAL
} socials_types;
#endif


/*
 * Struttura di un sociale
 */
struct social_data
{
	SOCIAL_DATA		*prev;
	SOCIAL_DATA		*next;
	char			*italian;		/* Nomi in italiano */
	char			*english;		/* Nomi in inglese	*/
	int				 use_count;		/* (GG) Quante volte viene utilizzato il social */
	int				 intention;		/* Tipo di intenzione del social, se aggressivo, neutrale o amichevole */
#ifdef T2_ALFA
	int				 position;		/* Posizione minina di utilizzo per questo social */
	BITVECTOR		*flags;			/* Flag del social (FF) forse basterebbe un tipo */
#endif
	char			*smiles;		/* Elenco degli smile supportati */
	char			*expression;	/* Espressione relativa se ha degli smile */
	SOCIALMSG_DATA	*first_socmsg;	/* Lista con i messaggi relative al social suddivise per razza */
	SOCIALMSG_DATA	*last_socmsg;
};


/*
 * Struttura dei messaggi di un social
 */
struct social_messages_data
{
	SOCIALMSG_DATA	*next;
	SOCIALMSG_DATA	*prev;
	BITVECTOR		*races;			/* Razze che supportano i messaggi */
	char			*char_no_arg;	/* Quello che il pg legge se lo usa senza argomento */
	char			*char_found;	/* Quello che il pg legge se ha trovato la vittima */
	char			*char_auto;		/* Quello che il pg legge se il pg lo usa su se stesso */
	char			*vict_found;	/* Quello che la vittima legge se il pg l'ha trovata */
	char			*others_no_arg;	/* Quello che gli altri leggono se il pg lo usa senza argomento */
	char			*others_found;	/* Quello che gli altri leggono se il pg ha trovato la vittima */
	char			*others_auto;	/* Quello che gli altri leggono se il pg lo usa su se stesso */
#ifdef T2_ALFA
	char			*outroom;		/* Se il social è sonoro è quello che si sente all'esterno */
#endif
};


/*
 * Variabili globali
 */
extern	SOCIAL_DATA *first_social;
extern	SOCIAL_DATA *last_social;
extern	int			 top_social;
extern	const char	*table_voice[];


/*
 * Funzioni globali
 */
void		 load_socials		( void );
void		 free_all_socials	( void );
SOCIAL_DATA *find_social		( CHAR_DATA *ch, const char *command, bool exact, bool ita );
bool		 check_social		( CHAR_DATA *ch, const char *command, char *argument, bool exact, bool ita );


#endif	/* SOCIAL_H */
