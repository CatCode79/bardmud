#ifndef COMMAND_H
#define COMMAND_H


/***************************************************************************************\
 > 				Header riguardante caricamento e gestione dei comandi					<
\***************************************************************************************/


/*
 * Definizioni di struttura
 */
typedef struct	command_function_data	CMDFUN_DATA;


/*
 * Elenco dei tipi di comando
 */
typedef enum
{
	CMDTYPE_MOVEMENT,		/* Movimento */
	CMDTYPE_CHANNEL,		/* Comunicazione */
	CMDTYPE_INFORMATION,	/* Informazione */
	CMDTYPE_OBJECT,			/* Manipola oggetti */
	CMDTYPE_COMBAT,			/* Combattimento */
	CMDTYPE_GAMING,			/* Di gioco */
	CMDTYPE_GROUPING,		/* Gruppo */
	CMDTYPE_OPTION,			/* Opzioni */
	CMDTYPE_CLAN,			/* Clan */
	CMDTYPE_MUDPROG,		/* Mobprogram */
	CMDTYPE_INTERACTION,	/* Interazione */
	CMDTYPE_EDITING,		/* Editazione */
	CMDTYPE_BUILDING,		/* Costruzione */
	CMDTYPE_PUNITION,		/* Punizioni */
	CMDTYPE_SYSTEM,			/* Sistema */
	/* 16 tipologie di comando */
	MAX_CMDTYPE
} commands_types;


/*
 * Tipi di flag di comando
 */
typedef enum
{
	CMDFLAG_POSSESS,    /* Il comando non può essere utilizzatto da posseduti */
	CMDFLAG_POLYMORPH,  /* Il comando non può essere utilizzatto da trasformati */
	CMDFLAG_WATCH,
	CMDFLAG_NOSPIRIT,	/* Il comando non può essere utilizzato se fantasmi */
	CMDFLAG_INTERACT,	/* Il comando interagisce con il mondo bloccando le azioni che eventualmente si stanno compiendo (FF) bisogna ancora farlo, un comando di esempio che verrebbe interrotto è do_search */
	CMDFLAG_TYPEALL,	/* Il comando deve essere digitato per intero */
	MAX_CMDFLAG
} command_flags;


/*
 * Struttura di un comando, o sinonimo di comando
 */
struct command_function_data
{
	CMDFUN_DATA	*next;
	CMDFUN_DATA	*prev;
	DO_FUN		*do_fun;		/* Puntatore alla funzione */
	char		*fun_name;		/* Stringa del nome della funzione */
	char		*minihelp;		/* Piccolo aiuto descrittivo */
	char		*msg_nospirit;	/* Messaggio per i comandi inutilizzabili dai pg spirito */
	int			 type;			/* Tipologia di comando */
	int			 trust;			/* Livello di fiducia per utilizzarlo */
	int			 min_position;	/* Posizione minima per utilizzarlo */
	int			 log;			/* Tipologia con cui viene loggato il comando */
	BITVECTOR	*flags;			/* Flag di comando (usate soprattutto per l'interpret) */
};


/*
 * Struttura di una funzione di comando
 */
struct command_data
{
	COMMAND_DATA	*next;
	COMMAND_DATA	*prev;
	char			*name;		/* Nomi e sinonimi del comando, italiano o inglese */
	bool			 synonym;	/* Indica se il comando è un sinonimo */
	struct timerset	 use_count;	/* Contatore di utilizzo del comando */
	int				 lag_count;	/* Contatore lag del comando */
	CMDFUN_DATA		*fun;		/* Struttura con le caratteristiche del comando di riferimento */
};


/*
 * Variabili
 */
extern	COMMAND_DATA	*first_command_it;
extern	COMMAND_DATA	*last_command_it;
extern	COMMAND_DATA	*first_command_en;
extern	COMMAND_DATA	*last_command_en;
extern	CMDFUN_DATA		*first_cmdfun;
extern	CMDFUN_DATA		*last_cmdfun;
extern	int				 top_cmdfun;
extern	int				 top_command_it;
extern	int				 top_command_en;
extern	const CODE_DATA	 code_cmdflag[];
extern	const CODE_DATA	 code_cmdtype[];
extern	const int		 max_check_cmdflag;
extern	const int		 max_check_cmdtype;


/*
 * Funzioni esterne
 */
void			load_commands	 ( void );
void			free_all_commands( void );
char		   *command_from_fun ( const char *fun_name, bool ita );
COMMAND_DATA   *find_command	 ( CHAR_DATA *ch, const char *command, bool exact, bool ita );
void 			write_order_file ( void );


#endif	/* COMMAND_H */
