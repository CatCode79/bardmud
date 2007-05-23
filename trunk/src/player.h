#ifndef PLAYER_H
#define PLAYER_H


/* Valore per la lunghezza del pager di default	*/
#define DEF_LEN_PAGER		80

/*
 * Bit per la Flag dei giocatori
 */
typedef enum
{
	/* Config per visualizzazioni */
	PLAYER_ACCENT,			/* Visualizza gli accenti normalmente, altrimenti li cambia con output */
	PLAYER_BLANK,
	PLAYER_BRIEF,
	PLAYER_COMBINE,
	PLAYER_COLOR,
	PLAYER_HINT,			/* Visualizza gli hint al giocatore ogni tot */
	PLAYER_LOOKEQUIP,
	PLAYER_MAPPER,
	PLAYER_PKGROUP,			/* Assiste nei combattimenti di gruppo contro dei pg */
	PLAYER_PROMPT,			/* Visualizza il prompt */
	PLAYER_PROMPTBAR,		/* Visualizza le barre dei punteggi al posto dei valori in percentuale */

	/* Config per automatizzazioni */
	PLAYER_AUTOEXIT,
	PLAYER_AUTOGOLD,
	PLAYER_AUTOSPLIT,
	PLAYER_AUTOLOOT,
	PLAYER_AUTORECALL,
	PLAYER_AUTOSAC,

	/* Config varie */
	PLAYER_INTRO,
	PLAYER_ITALIAN,			/* Cambia da uso dei comandi in inglese a quelli in italiano */
	PLAYER_DOUBLELANG,		/* Invia i comandi sia in inglese che in italiano, dando la precedenza al set di lingua scelta */
	PLAYER_TELNETGA,

	/* Config per admin */
	PLAYER_ADMSIGHT,		/* La vecchia holylight */
	PLAYER_ALLEXITS,		/* Per visualizzare anche le uscite che i pg non vedono */
	PLAYER_ECHOIN,			/* Invia gli echo recho e aecho con l'admin come vittima automaticamente */
	PLAYER_FLAG,			/* Per visualizzare le flag delle varie entità	*/
	PLAYER_INVISWHO,		/* Per rendersi invisibili nel who agli occhi dei giocatori non admin */
	PLAYER_LASTCMD,			/* Per visualizzare l'ultimo comando digitato, dove e da chi nei messaggi di log */
	PLAYER_NOADMIN,			/* Se attiva l'amministratore si comporterà come un giocatore normale */
	PLAYER_NOWAIT,			/* Se attivato non c'è wait per l'adim */
	PLAYER_VNUM,			/* Per visualizzare vnum delle varie entità		*/

	/* Opzioni non config */
	PLAYER_AFK,
	PLAYER_AFK2,
	PLAYER_DIAGNOSE,
	PLAYER_IDLE,			/* Il giocatore è in Linkdead */
	PLAYER_LOG,
	PLAYER_WATCH,			/* see function "do_watch" (FF) evitare di visualizzare tra gli olc la flag o saltarla, così da evitare che chi tra gli admin è watchato lo venga a sapere */
	PLAYER_SPIRIT,			/* Flag di quando si è spirito */

	/* Status riguardati crimini o pkilling */
	PLAYER_ATTACKER,		/* Indica che è un aggressore			*/
#ifdef T2_ALFA
	PLAYER_HEADHUNTER,		/* (GG) Indica che è un cacciatore di taglie	*/
#endif
	PLAYER_KILLER,			/* Indica che è un pkiller				*/
	PLAYER_OUTCAST,			/* Indica che è reietto da una clan		*/
	PLAYER_THIEF,			/* Indica che è un ladro				*/

	/* Punizioni */
	PLAYER_DENY,
	PLAYER_EXPHALVE,
	PLAYER_FREEZE,
	PLAYER_LITTERBUG,
	PLAYER_NOEMOTE,
	PLAYER_NOSOCIAL,
	PLAYER_NOTELL,
	PLAYER_NOTITLE,
	PLAYER_NUISANCE,
	PLAYER_SILENCE,

	/* (FF) aggiungere quella che raddoppierebbe il wait state dei pg nel gioco se rompono */
	MAX_PLAYER
} player_flags;


/*
 * Enumerazione per le opzioni protocollo
 */
typedef enum
{
#ifdef T2_MSP
	PROTOCOL_MSP,		/* Protocollo MSP */
	PROTOCOL_MUSIC,		/* Opzione per attivare o disattivare la musica */
	PROTOCOL_SOUND,		/* Opzione per attivare o disattivare il sonoro */
	PROTOCOL_SURROUND,	/* Opzione per ottenere un effetto sourround nel sonoro */
	PROTOCOL_COMBAT,	/* Invia il sonoro del combattimento */
	PROTOCOL_BEEP,		/* Invia un suono se si riceve un tell o un reply */
#endif
#ifdef T2_MCCP
	PROTOCOL_MCCP,		/* Protocollo MCCP*/
#endif
#ifdef T2_MXP
	PROTOCOL_MXP,		/* Protocollo MXP */
#endif
	MAX_PROTOCOL
} protocol_flags_types;


/*
 * Struttura della tabella delle flag PLR_
 */
struct player_type
{
	CODE_DATA	 code;
	int			 trust;		/* trust minima per attivare l'opzione */
	char		*active;
	char		*deactive;
};


/*
 * Struttura della tabella delle flag PROT_
 */
struct protocol_type
{
	CODE_DATA	 code;
	char		*active;
	char		*deactive;
};


/*
 * Data which only PG's have
 */
struct pg_data
{
	AREA_DATA	*build_area;				/* Area assegnata per il building personale */
	CHAR_DATA	*mascotte;
	CLAN_DATA	*clan;						/* Clan a cui il pg appartiene */
	MEMBER_DATA *member;					/* Informazioni di membro del clan */
	HAIR_PART	*hair;						/* dati sui capelli del pg */
	EYE_PART	*eye;						/* dati sugli occhi del pg */
	ALIAS_DATA	*first_alias;				/* Alias */
	ALIAS_DATA	*last_alias;
    IGNORE_DATA	*first_ignored;				/* Lista degli ignorati */
    IGNORE_DATA	*last_ignored;    
	NOTE_DATA	*first_note;				/* Lista delle note offrpg */
	NOTE_DATA	*last_note;
	NOTE_DATA	*first_mail;				/* Lista delle lettere rpg */
	NOTE_DATA	*last_mail;
	char		*pwd;
	char		*host;						/* Variabile di riserva per contenere l'ip quando il descrittore va' in link death */
	char		*surname;					/* Cognome del pg */
	char		*title;						/* Titolo del pg */
	char		*bio;						/* Biografia personale */
	char		*afk_message;				/* Messaggio di afk */
	char		*msg_login_you;				/* Messaggio personale di login */
	char		*msg_login_room;			/* Messaggio pubblico di login */
	char		*msg_logout_you;			/* Messaggio personale di logout */
	char		*msg_logout_room;			/* Messaggio pubblico di logout */
	char		*objective;					/* Obiettivo cui il pg si prefigge */
	char		*music_file;				/* File musicale inviato per ultimo, per stopparlo correttamente */
	char		*msg_slay_victim;			/* Messaggio personalizzato di slay alla vittima */
	char		*msg_slay_room;				/* Messaggio personalizzato di slay alla stanza */
	int			 counter_quit;				/* Contatore alla rovescia per l'uscita dal mud */
	BITVECTOR   *flags;						/* Whether the player is deadly and whatever else we add */
	BITVECTOR   *protocols;					/* Flags per le opzioni riguardanti i protocolli */
	int			 pkills;					/* Number of pkills on behalf of clan */
	int			 pdeaths;					/* Number of times pkilled (legally) */
	int			 mkills;					/* Number of mobs killed */
	int			 mdeaths;					/* Number of deaths due to mobs */
	int			 illegal_pk;				/* Number of illegal pk's committed */
	VNUM		 vnum_range_low;			/* Range di Vnum minimo */
	VNUM		 vnum_range_high;			/* Range di Vnum massimo */
	int			 incognito;					/* Livello di Incognito */
	int			 trust;						/* Grado di fiducia del pg */
	int			 min_snoop;					/* minimum snoop trust */
	int			 condition[MAX_CONDITION];	/* Condizioni del pg: sete, fame, .. */
	int			 learned_skell[MAX_SKILL];	/* Apprendimento relativo alle skell/weapon */
	int			 learned_lang[MAX_LANGUAGE];/* Apprendimento relativo alle lingue */
	KILLED_DATA	 killed[MAX_KILLTRACK];		/* Variabile in cui salva quali mob uccisi */
	int			 glory;						/* Punti di gloria correnti */
	int			 glory_accum;				/* Gloria accumulata durante la vita del pg */
#ifdef T2_ALFA
	int			 rpg_points					/* (GG) Punti di rpg, maggiori sono e più il pg interpreta bene */
#endif
	int			 archetype;					/* Tipologia di carattere del pg */
	char		*personality;				/* Descrizione della personalità del pg */
	int			 charmies;					/* Number of Charmies */
	time_t		 release_date;				/* Auto-helling... */
	char		*helled_by;
	char		*prompt;					/* User config prompts */
	char		*fprompt;					/* Fight prompts */
	char		*subprompt;					/* Substate prompt */
	int			 pagerlen;					/* For pager (NOT menus) */
	int			 played;
	time_t		 creation_date;				/* Data della creazione del pg */
	time_t		 logon_old;
	time_t		 logon;
	time_t		 save_time;
	int			 zodiac;					/* Se -1 il pg non conosce il suo segno zodiacale */
	DREAM_DATA	*dream;						/* Sogno che il pg sta facendo */
	int			 dream_part;				/* Parte del sogno che il pg sta leggendo */
	int			 practice;
	BITVECTOR   *deaf;
	BITVECTOR   *msglog;
	int			 colors[MAX_AT_COLORS];
	CHAR_DATA	*permission_sender;			/* Chi ha domandato il permesso di qualcosa a ch */
	CHAR_DATA	*permission_receiver;		/* A chi è stata inviata una domanda */
	char		*permission_action;			/* L'azione che qualcuno ha domandato il permesso di fare a ch */
	int			 permission_answer;			/* La risposta di ch a chi gli ha domandato il permesso di qualcosa */
#ifdef T2_WEB
	int			 report;					/* Numero di bug, idea e typo segnalati */
#endif
};


/*
 * Funzioni
 */
char	*get_style_name	( CHAR_DATA *ch );
void	 set_title		( CHAR_DATA *ch, char *title );
void	 score_handler	( CHAR_DATA *ch, CHAR_DATA* victim, char *argument );
void	 add_offline	( CHAR_DATA *ch );
bool	 remove_offline	( CHAR_DATA *ch );
void	 load_offlines	( void );
void	 refresh_offline( CHAR_DATA *ch );
void	 init_pg		( PG_DATA *pg );


/*
 * Variabili
 */
extern	const struct player_type 	table_player[];
extern	const struct protocol_type 	table_protocol[];
extern	const int					max_check_player;
extern	const int					max_check_protocol;


#endif	/* PLAYER_H */
