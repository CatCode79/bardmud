#ifndef MOVEMENT_H
#define MOVEMENT_H


/*
 * Direzioni
 */
typedef enum
{
	DIR_NORTH,
	DIR_EAST,
	DIR_SOUTH,
	DIR_WEST,
	DIR_UP,			/* (CC) cambiarli tutti in maniera che siano in ordine orario */
	DIR_DOWN,
	DIR_NORTHEAST,
	DIR_NORTHWEST,
	DIR_SOUTHEAST,
	DIR_SOUTHWEST,
	DIR_SOMEWHERE,
	MAX_DIR
} dirs_types;


/*
 * Push/pull types for exits
 */
typedef enum
{
	PULL_UNDEFINED,
	PULL_VORTEX,
	PULL_VACUUM,
	PULL_SLIP,
	PULL_ICE,
	PULL_MYSTERIOUS,
	/* acqua */
	PULL_CURRENT,
	PULL_WAVE,
	PULL_WHIRLPOOL,
	PULL_GEYSER,
	/* aria */
	PULL_WIND,
	PULL_STORM,
	PULL_COLDWIND,
	PULL_BREEZE,
	/* terra */
	PULL_LANDSLIDE,
	PULL_SINKHOLE,
	PULL_QUICKSAND,
	PULL_EARTHQUAKE,
	/* fuoco */
	PULL_LAVA,
	PULL_HOTAIR,
	/* 20 tipi di pull/push */
	MAX_PULL
} pull_types;


/*
 * Tipo di struttura di teleport
 */
typedef struct	teleport_data	TELEPORT_DATA;


/*
 * Enumerazione delle flag di teleport
 */
typedef enum
{
	TELE_SHOWDESC,
	TELE_TRANSALL,
	TELE_TRANSALLPLUS,
	TELE_TRANSNOMSG,		/* (GG) */
	MAX_TELE_FLAG
} tele_flags_types;


/*
 * Struttura di un teleport
 */
struct teleport_data
{
	TELEPORT_DATA	*next;
	TELEPORT_DATA	*prev;
	ROOM_DATA		*room;
	int				 timer;
	int				 flags;	/* (GG) */
};


/*
 * Struttura di un elemento della tabella delle direzioni
 */
struct dir_type
{
	CODE_DATA	 code;			/* Nomi dei codici e in italiano delle direzioni */
	char		*eng;			/* Nomi dell direzioni in inglese */
	char		*from;			/* Da dove arriva */
	char		*to;			/* Dove va' */
	int			 rev_dir;		/* Direzioni opposte */
	int			 trap_door;		/* Numero dei tipi di trappola assegnate alle direzioni */
};


/*
 * Variabili
 */
extern	TELEPORT_DATA			*first_teleport;
extern	TELEPORT_DATA			*last_teleport;
extern	const struct dir_type	 table_dir[];
extern	const CODE_DATA			 code_pull[];
extern	const int				 max_check_pull;
extern	const int				 max_check_dir;


/*
 * Funzioni
 */
int			get_door			( const char *arg );
EXIT_DATA  *find_door			( CHAR_DATA *ch, char *arg, bool quiet );
EXIT_DATA  *get_exit			( ROOM_DATA *room, int dir );
EXIT_DATA  *get_exit_to			( ROOM_DATA *room, int dir, VNUM vnum );
EXIT_DATA  *get_exit_num		( ROOM_DATA *room, int count );
bool		check_direction_arg	( const char *dir );
int			encumbrance			( CHAR_DATA *ch, int move );
ch_ret		pullcheck			( CHAR_DATA *ch, int pulse );
bool		will_fall			( CHAR_DATA *ch, int fall );
void		update_dt			( void );
ch_ret		move_char			( CHAR_DATA *ch, EXIT_DATA *pexit, int fall, bool running );
void		teleport			( CHAR_DATA *ch, VNUM room, int flags );
void		update_tele			( void );


#endif	/* MOVEMENT_H */
