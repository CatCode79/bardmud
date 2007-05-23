#ifndef SAVE_H
#define SAVE_H


/*
 * Definizioni
 */
#define ADMIN_DIR	PLAYER_DIR	"admin/"	/* Dir info degli Amministratori	*/
#define BACKUP_DIR	PLAYER_DIR	"backup/"	/* Dir backup dei file dei PG		*/
#define CORPSE_DIR	PLAYER_DIR	"corpses/"	/* Dir salvataggio cadaveri			*/

#define OS_CARRY	0
#define OS_CORPSE	1

#define MAX_NEST	4	/* Maximum container nesting */


/*
 * Funzioni
 */
void	re_equip_char		( CHAR_DATA *ch );
void	de_equip_char		( CHAR_DATA *ch );
void	fread_object_save	( CHAR_DATA *ch, MUD_FILE *fp, int os_type );
void	fwrite_object		( CHAR_DATA *ch, OBJ_DATA  *obj, MUD_FILE *fp, int iNest, int os_type );
void	set_alarm			( long seconds );
void	load_corpses		( void );
void	write_corpses		( CHAR_DATA *ch, char *name, OBJ_DATA *objrem );
void	save_player			( CHAR_DATA *ch );
bool	load_player			( DESCRIPTOR_DATA *d, char *name, bool preload );


#endif	/* SAVE_H */
