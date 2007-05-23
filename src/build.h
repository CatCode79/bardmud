#ifndef BUILD_H
#define BUILD_H


/*
 * Tipo di struttura Rel
 */
typedef struct	rel_data		REL_DATA;


/*
 * Dir di salvataggio delle aree create online
 */
#define BUILD_DIR	AREA_DIR	"building/"


/*
 * Short cut crash bug fix provided by gfinello@mail.karmanet.it
 */
typedef enum
{
   relMRESTRING_ON,
   relORESTRING_ON
} relation_type;

/*
 * Struttura di una relazione
 */
struct rel_data
{
	void		 *actor;
	void		 *subject;
	REL_DATA	 *next;
	REL_DATA	 *prev;
	relation_type type;
};


/*
 * Variabili
 */
extern REL_DATA		*first_relation;
extern REL_DATA		*last_relation;


/*
 * Funzioni
 */
bool				can_edit_mob	( CHAR_DATA *ch, MOB_PROTO_DATA *imob );
bool				can_modify_mob	( CHAR_DATA *ch, CHAR_DATA *mob );
bool				can_modify_obj	( CHAR_DATA *ch, OBJ_DATA *obj );
bool				can_modify_room	( CHAR_DATA *ch, ROOM_DATA *room );
char			   *sprint_reset	( CHAR_DATA *ch, RESET_DATA *pReset, int num, bool rlist );
RESET_DATA		   *parse_reset		( char *argument, CHAR_DATA *ch );
void				assign_area		( CHAR_DATA *ch );
EXTRA_DESCR_DATA   *set_obj_extra	( OBJ_DATA *obj, char *keywords );
void				fold_area		( AREA_DATA *tarea, char *filename, bool install );
int					get_dir			( const char *txt );


#endif	/* BUILD_H */
