#ifndef CHANNEL_H
#define CHANNEL_H


/*
 * Dir e File riguardanti i vari tipi di report
 */
#define IDEA_FILE		SYSTEM_DIR "ideas.txt"		/* File con i messaggi di idea dei giocatori */
#define PBUG_FILE		SYSTEM_DIR "pbugs.txt"		/* File con i messaggi di bug segnalati dai giocatori */
#define TYPO_FILE		SYSTEM_DIR "typos.txt"		/* File con i messaggi di typo */
#define COMMENT_FILE	SYSTEM_DIR "comments.txt"	/* File con i messaggi di commento */
#define FIXED_FILE		SYSTEM_DIR "fixed.txt"		/* For 'fixed' command */


/*
 * Tipi di Canali
 */
typedef enum
{
	CHANNEL_MURMUR,		/* Mormora		 */
	CHANNEL_WHISPER,	/* Sussurra 	 */
	CHANNEL_SAY,		/* Dice			 */
	CHANNEL_SING,		/* Canta		 */
	CHANNEL_YELL,		/* Urla			 */
	CHANNEL_SHOUT,		/* Grida		 */
	CHANNEL_TELL,		/* Comunica 	 */
	CHANNEL_GTELL,		/* Gcomunica	 */
	CHANNEL_THINK,		/* Pensa		 */
	CHANNEL_QTALK,		/* Qparla		 */
	CHANNEL_ADMIN,		/* Admtalk		 */
	MAX_CHANNEL
} channel_types;


/*
 * Enumerazione dei tipi di log
 */
typedef enum
{
	LOG_NORMAL,
	LOG_ALWAYS,
	LOG_NEVER,
	LOG_BUILD,
	LOG_ADMIN,
	LOG_COMM,
	LOG_WARNING,
	LOG_MONI,
	LOG_ALL,		/* (FF) Forse questo non serve */
	LOG_MPROG,
	LOG_TRIGGER,
	LOG_SUPERMOB,
	LOG_LOAD,
	LOG_OLC,
	LOG_DAMAGE,
	LOG_DEATH,
	LOG_PK,
	LOG_BUG,
	LOG_PUNY,
	LOG_SPEECH,
	LOG_FP,
	LOG_FREAD,
	LOG_FWRITE,
	LOG_BOT,
	LOG_MEMORY,
	LOG_BIT,
	LOG_REPORT,
	LOG_CHECK,
	LOG_RNDATTR,
	LOG_SPACEFIGHT,
	LOG_RESET,
	LOG_CODE,
	LOG_APPLY,
	LOG_CMDMOB,
	LOG_CMDPG,
	LOG_ORDER,
	LOG_LAG,
	LOG_TITLE,
	LOG_WHO,
	LOG_SAVE,
	LOG_SHOVEDRAG,
	LOG_LEVEL,
	LOG_YELLSHOUT,
	LOG_ECHO,
	LOG_LEARN,
	LOG_EXP,
	LOG_SHARE,
	LOG_LYRIC,
#ifdef T2_MCCP
	LOG_MCCP,
#endif
	LOG_AUTOQUEST,
	LOG_SNOOP,
	MAX_LOG
} log_types;


#define MAX_IGNORE	12	/* Numero massimo di persone che si possono ignorare per volta */

/*
 * Struttura per la lista dei pg
 */
struct ignore_data
{
	IGNORE_DATA	*next;
	IGNORE_DATA	*prev;
	char		*name;
};


/*
 * Struttura per la tabella dei canali
 */
struct channel_type
{
	CODE_DATA	 code;		/* Codice di CHANNEL_ */
	char		*verb_I;	/* Verbo del canale alla prima persona */
	char		*verb_it;	/* Verbo del canale alla terza persona */
	bool		 rpg;		/* Se il canale è RPG oppure no */
};


/*
 * Struttura per la tabella table_log
 */
struct log_type
{
	CODE_DATA	code;		/* Codice di LOG_ */
	int			trust;		/* Trust minima per visualizzare quei messaggi di log */
	bool		file;		/* Se scrive su file o no il messaggio di log */
	bool		lastcmd;	/* Se stampa o no l'ultimo comando assieme al messaggio di bug */
	bool		def;		/* Se attiva di default il messaggio di log */
	char	   *descr;		/* Descrizione della tipologia di log */
};


/*
 * Variabili
 */
extern	const struct channel_type	table_channel[];
extern	const struct log_type		table_log	 [];
extern	const int					max_check_channel;
extern	const int					max_check_log;


/*
 * Funzioni
 */
void	manage_append_msg	( CHAR_DATA *ch, char *file, char *str );
void	update_counter_quit	( void );
void	echo_to				( const char *arg, CHAR_DATA *ch, int tar );
void	send_log			( MUD_FILE *fp, const int log_type, const char *fmt, ... ) __attribute__ ( (format(printf, 3, 4)) );


#endif	/* CHANNEL_H */
