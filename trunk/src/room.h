#ifndef ROOM_H
#define ROOM_H


/*
 * Flag di Uscita
 * EX_RES# are reserved for use by the SMAUG development team (RR) da togliere dalle palle insomma, dopo la conversione
 * Utilizzate in #ROOMS
 */
typedef enum
{
	EXIT_ISDOOR,
	EXIT_CLOSED,
	EXIT_LOCKED,
	EXIT_SECRET,
	EXIT_SWIM,
	EXIT_PICKPROOF,
	EXIT_FLY,
	EXIT_CLIMB,					/* (CC) Da convertire in xCLIMB? */
	EXIT_DIG,
	EXIT_EATKEY,
	EXIT_NOPASSDOOR,
	EXIT_HIDDEN,
	EXIT_PASSAGE,
	EXIT_PORTAL,	/* (FF) Da utilizzare per i portali sui piani, visto che esiste già la DIR_SOMEWHERE che funziona da portale */
	EXIT_RES1,
	EXIT_RES2,
	EXIT_xCLIMB,
	EXIT_xENTER,
	EXIT_xLEAVE,
	EXIT_xAUTO,
	EXIT_NOFLEE,
	EXIT_xSEARCHABLE,
	EXIT_BASHED,
	EXIT_BASHPROOF,
	EXIT_NOMOB,
	EXIT_WINDOW,
	EXIT_xLOOK,
	EXIT_ISBOLT,
	EXIT_BOLTED,
	MAX_EXIT
} exit_flags;


/*
 * Room flags.
 * Used in #ROOMS.
 */
typedef enum
{
	ROOM_DARK,
	ROOM_DEATH,
	ROOM_NOMOB,
	ROOM_INDOORS,
	ROOM_LAWFUL,		/* (CC) _GOOD Stanze intrinse da benignità, i malvagi possono entrarci ma soffrono */
	ROOM_NEUTRAL,
	ROOM_CHAOTIC,		/* (CC) _EVIL Stanze intrinse da malvagità, i malvagi possono entrarci ma soffrono */
	ROOM_NOMAGIC,
	ROOM_TUNNEL,
	ROOM_PRIVATE,
	ROOM_SAFE,
	ROOM_SOLITARY,
	ROOM_MASCOTTESHOP,
	ROOM_NORECALL,
	ROOM_DONATION,
	ROOM_NODROPALL,
	ROOM_SILENCE,
	ROOM_LOGSPEECH,
	ROOM_NODROP,
	ROOM_STOREROOM,
	ROOM_NOSUMMON,
	ROOM_NOASTRAL,
	ROOM_TELEPORT,
	ROOM_TELESHOWDESC,
	ROOM_NOFLOOR,
	ROOM_NOSUPPLICATE,
	ROOM_ARENA,			/* La room è una zona in cui avvengono combattiment di arena */
	ROOM_TERRACE,		/* Spalti, la room è una zona da cui si possono vedere i combattimenti dell'arena */
	ROOM_NOMISSILE,
	ROOM_BFS_MARK,		/* Serve per il track */
	ROOM_RENT,			/* Lì si può affittare una stanza per dormire */
	ROOM_PROTOTYPE,		/* (CC) da togliere */
	ROOM_HELL,			/* La stanza è l'inferno, i pg vengono inviati là per punizione, solo gli imple possono entrarvi per interagire con i player */
#ifdef T2_ALFA
	ROOM_FLYING,		/* Riesce a volare con creature volanti */
	ROOM_NOLANDING,		/* Non riesce ad atterrare creature volanti anche se questo e sopra possono essere calcolati con una aculata gestione dei settori */
#endif
	/* 33 tipologie di flag di room */
	MAX_ROOM
} room_flag_types;


/*
 * (FF) Definire i settori vegetazione, anzi utilizzare una variabile a parte.
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
	SECTOR_INSIDE,
	SECTOR_CITY,			/* (FF) Dovrei cercare di gestire la folla, forse suddividendo vari livelli di grandezza cittadina? come variabile a parte nell'#AREA, così da gestire il rumore */
	SECTOR_FIELD,
	SECTOR_FOREST,
	SECTOR_HILLS,
	SECTOR_MOUNTAIN,
	SECTOR_WATER_SWIM,
	SECTOR_WATER_NOSWIM,	/* (FF) Da togliere, si può nuotare sempre ma con certi limiti, dati dalla densità del liquido */
	SECTOR_UNDERWATER,
	SECTOR_AIR,
	SECTOR_DESERT,
	SECTOR_DUNNO,
	SECTOR_OCEANFLOOR,
	SECTOR_UNDERGROUND,
	SECTOR_LAVA,
	SECTOR_SWAMP,
	MAX_SECTOR
} sectors_types;


/*
 * Struttura per la tabella dei settori
 */
struct sector_type
{
	CODE_DATA	code;			/* Codice del settore */
	char	   *color_back;		/* Colore di background */
	char	   *color_fore;		/* Colore di foreground */
	char	   *color_invert;	/* Colore di background invertito al caso che quello del pg sia uguale a quello di background normale */
	char	   *symbol;			/* Simbolo utilizzato per il settore */
	bool		canpass;		/* Indica se il tipo di settore impassabile nella wild */
	int			move_loss;		/* Movimento consumato normalemente */
};


/*
 * Struttura di una Room Indicizzata, il Prototipo
 */
struct room_data
{
	ROOM_DATA		 *next;
	ROOM_DATA		 *next_sort;
	CHAR_DATA		 *first_person;		/* Prima persona nella stanza */
	CHAR_DATA		 *last_person;		/* Ultima persona nella stanza */
	OBJ_DATA		 *first_content;	/* Primo oggetto nella stanza */
	OBJ_DATA		 *last_content;		/* Ultimo oggetto nella stanza */
	EXTRA_DESCR_DATA *first_extradescr;	/* Prima descrizione extra */
	EXTRA_DESCR_DATA *last_extradescr;	/* Ultima descrizione extra */
	EXIT_DATA		 *first_exit;		/* Prima uscita dalla stanza */
	EXIT_DATA		 *last_exit;		/* Ultima uscita dalla stanza */
	AFFECT_DATA		 *first_affect;		/* Prima affezione nella stanza */
	AFFECT_DATA		 *last_affect;		/* Ultima affezione nella stanza */
	AREA_DATA		 *area;				/* Puntatore all'area della stanza */
	VNUM			  vnum;				/* Vnum della stanza */
	char			 *name;				/* Nome della stanza */
	char			 *description;		/* Descrizione della stanza */
	BITVECTOR		 *flags;			/* Flag della stanza */
	int				  sector;			/* Tipo di settore */
	int				  light;			/* Ammontare di luce nella stanza */
	int				  tunnel;			/* Persone massime che possono starci nella stanza (FF) se sarà zero vedere di calcolare il numero dinamicamente sulla base delle info dell'area */
	VNUM			  tele_vnum;
	int				  tele_delay;
	MPROG_DATA		 *first_mudprog;	/* roomprogs */
	MPROG_DATA		 *last_mudprog;		/* roomprogs */
	MPROG_ACT_LIST	 *mpact;			/* roomprogs */
	int				  mpactnum;			/* roomprogs */
};


/*
 * Struttura di un'Uscita
 */
struct exit_data
{
	EXIT_DATA	*prev;			/* Uscita precedente nella lista */
	EXIT_DATA	*next;			/* Uscita seguente nella lista */
	EXIT_DATA	*rexit;			/* Puntatore dell'uscita inversa */
	ROOM_DATA	*to_room;		/* Puntatore alla stanza di destinazione */
	char		*keyword;		/* Parole chiavi per l'uscita o la porta */
	char		*description;	/* Descrizione dell'uscita */
	VNUM		 vnum;			/* Vnum della stanza ove l'uscita porta */
	VNUM		 rvnum;			/* Vnum della stanza da dove l'uscita parte */
	VNUM		 key;			/* Vnum della chiave per aprire l'uscita */
	BITVECTOR	*flags;			/* Flag di stato della porta o dell'uscita */
	int			 vdir;			/* "Direzione" cardinale dell'uscita */
	int			 distance;		/* Quanto è lontana in metri la prossima stanza (GG)*/
	int			 pull;			/* Spinta della direzione (corrente) */
	int			 pulltype;		/* Tipo di spinta (corrente, vento) */
};


/*
 * Variabili
 */
extern	const struct sector_type	table_sector[];
extern	const CODE_DATA				code_exit[];
extern	const CODE_DATA				code_room[];
extern	const int					max_check_exit;
extern	const int					max_check_room;
extern	const int					max_check_sector;


/*
 * Funzioni
 */
void		 fwrite_storage			( CHAR_DATA *ch );
void		 load_storages			( void );
bool		 room_is_dark			( ROOM_DATA *pRoom );


#endif	/* ROOM_H */
