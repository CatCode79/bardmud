#ifndef DB_H
#define DB_H

/*
 * Definizioni di strutture
 */
typedef struct mud_stat_data	MUDSTAT_DATA;


/*
 * Definizioni di file e dir
 */
#define AREA_LIST		AREA_DIR	"area.lst"		/* Lista Aree */
#define MUDSTAT_FILE	SYSTEM_DIR	"mudstat.dat"	/* Statistiche del mud */


/*
 * Struttura delle statistiche del mud
 */
struct mud_stat_data
{
	char		   *time_max;				/* Tempo di massimo per i giocatori online */
	int				alltime_max;			/* Maximum di sempre di giocatori online */
	char		   *time_max_nomulty;		/* (GG) Tempo di massimo per i giocatori con ip unico */
	int				alltime_max_nomulty;	/* (GG) Massimo di sempre di giocatori con ip unico online */

	/* Statistiche NON salvate tra un boot ed un altro: */
	unsigned int	total_output;		/* Byte inviati totali di output */
#ifdef MCCP
	unsigned int	total_output_mccp;	/* Byte inviati totali di output compresso */
#endif
	int				boot_max_players;	/* Maximum players this boot */
	int				global_looted;		/* Gold looted this boot */
	int				upill_val;			/* Used pill value */
	int				upotion_val;		/* Used potion value */
	int				used_brewed;		/* Brewed potions used */
	int				used_scribed;		/* Scribed scrolls used */
};


/*
 * Variabili
 */
MUDSTAT_DATA	mudstat;	/* variabile con le stastiche del mud */

extern bool		fBootDb;

extern int		top_area;
extern int		top_ed;
extern int		top_exit;
extern int		top_mob_proto;
extern int		top_obj_proto;
extern int		top_offline;
extern int		top_room;

ROOM_DATA	   *room_index_hash[MAX_KEY_HASH];

extern int	 	cur_qchars;
extern int      cur_qobjs;

extern	CHAR_DATA		*first_char;
extern	CHAR_DATA		*last_char;
extern	CHAR_DATA		*first_player;
extern	CHAR_DATA		*last_player;
extern	CHAR_DATA		*first_offline;
extern	CHAR_DATA		*last_offline;
extern	MOB_PROTO_DATA	*first_mob_proto;
extern	MOB_PROTO_DATA	*last_mob_proto;
extern	OBJ_DATA		*first_object;
extern	OBJ_DATA		*last_object;
extern	OBJ_DATA		*first_object_proto;
extern	OBJ_DATA		*last_obj_proto;
extern	AREA_DATA		*first_area;
extern	AREA_DATA		*last_area;
extern	AREA_DATA		*first_build;
extern	AREA_DATA		*last_build;


/*
 * Funzioni
 */
bool				free_mobile_proto	( MOB_PROTO_DATA *mob );
bool				free_object_proto	( OBJ_PROTO_DATA *obj );
bool				free_room			( ROOM_DATA *room );
void				clean_object		( OBJ_DATA *obj );
void				fix_area_exits		( AREA_DATA *tarea );
void				load_area_file		( AREA_DATA *tarea, MUD_FILE *fp );
void				init_char			( CHAR_DATA *ch, bool mob );
void				free_char			( CHAR_DATA *ch );
MOB_PROTO_DATA	   *get_mob_index		( MUD_FILE *fp, VNUM vnum );
OBJ_PROTO_DATA	   *get_obj_proto		( MUD_FILE *fp, VNUM vnum );
ROOM_DATA		   *get_room_index		( MUD_FILE *fp, VNUM vnum );
MOB_PROTO_DATA	   *make_mobile_proto	( VNUM vnum, VNUM cvnum, char *name );
OBJ_PROTO_DATA	   *make_object_proto	( VNUM vnum, VNUM cvnum, char *name );
ROOM_DATA		   *make_room_index		( VNUM vnum );
ROOM_DATA		   *make_room			( VNUM vnum );
CHAR_DATA		   *make_mobile			( MOB_PROTO_DATA *pMobIndex );
OBJ_PROTO_DATA	   *make_object			( OBJ_DATA *pObjIndex, int level );
int					number_door			( void );
int					dice				( int number, int size );
int					interpolate			( int level, int value_00, int value_32 );
void				free_room_proto		( ROOM_DATA *room );
EXIT_DATA		   *make_exit			( ROOM_DATA *pRoom, ROOM_DATA *to_room, int door );
void				randomize_exits		( ROOM_DATA *room, int maxdir );
void				save_mudstat		( void );
void				load_databases		( bool fCopyover );
void				close_databases		( void );


#endif	/* DB_H */
