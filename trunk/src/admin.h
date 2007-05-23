#ifndef ADMIN_H
#define ADMIN_H


/*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*\
 >								Header del Modulo Admin									<
\*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*/


/*
 * Funzioni
 */
void			equip_newbieset	( CHAR_DATA *victim );
void			close_area		( AREA_DATA *pArea );
ROOM_DATA	   *find_location	( CHAR_DATA *ch, char *arg );
void			check_mobile	( MOB_PROTO_DATA *pMobProto, MUD_FILE *fp );
void			check_room		( ROOM_DATA *pRoom, MUD_FILE *fp );
void			check_object	( OBJ_DATA *obj, MUD_FILE *fp );


#endif	/* ADMIN_H */
