#ifndef HELP_H
#define HELP_H


/****************************************************************************\
 >							Header help module								<
\****************************************************************************/

/*
 * Definizione del tipo di help
 */
typedef struct	help_data		HELP_DATA;


/*
 * Enumerazione dei tipi di help
 */
typedef enum
{
	HELPTYPE_MOVEMENT,	/* Tipo di help movimento */
	HELPTYPE_CHANNEL,	/* Tipo di help comunicazione, rpg e non, e settaggio canali */
	HELPTYPE_INFO,		/* Tipo di help informazione, comandi che fanno vedere qualcosa */
	HELPTYPE_OBJECT,	/* Tipo di help manipola oggetti, comandi per l'utilizzo oggetti */
	HELPTYPE_COMBAT,	/* Tipo di help combattimento */
	HELPTYPE_GAMING,	/* Tipo di help di gioco in generale (cioè quando non si sa che tipo piazzargli, si mette questo) */
	HELPTYPE_GROUP,		/* Tipo di help gruppo */
	HELPTYPE_OPTION,	/* Tipo di help opzioni, settaggi opzioni pg (config per esempio) */
	HELPTYPE_EDIT,		/* Tipo di help di editazione desrizione e di gestione dell'editor e delle note */
	HELPTYPE_CLAN,		/* Tipo di help riguardante i clan */
	HELPTYPE_SYSTEM,	/* Tipo di help che non deve essere letto nel caricamento degli help di web */
	HELPTYPE_SKILL,		/* Tipo di help per le skill */
	HELPTYPE_SPELL,		/* Tipo di help per gli spell */
	HELPTYPE_MUDPROG,	/* Tipo di help mobprogram */
	HELPTYPE_ADMIN,		/* Tipo di help degli admin */
	MAX_HELPTYPE
} help_types;


/*
 * Struttura di un Help
 */
struct help_data
{
	HELP_DATA	*next_it;
	HELP_DATA	*prev_it;
	HELP_DATA	*next_en;
	HELP_DATA	*prev_en;
	char 		*keyword_it;
	char 		*keyword_en;
	int			 type;
	int			 trust;
	char 		*text;
	char		*text_to_admin;
	char		*to_see;
};


/*
 * Variabili
 */
extern	int				top_help;
extern	const CODE_DATA code_helptype[];
extern	const int		max_check_helptype;


/*
 * Funzioni
 */
void	load_helps		( void );
void	free_all_helps	( void );
DO_RET	do_help			( CHAR_DATA *ch, char *argument );


#endif	/* HELP_H */
