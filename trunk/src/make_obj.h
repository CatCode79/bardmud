#ifndef MAKEOBJ_H
#define MAKEOBJ_H


void		make_blood		( CHAR_DATA *ch );
void		make_corpse		( CHAR_DATA *ch, CHAR_DATA *killer );
void		make_bloodstain	( CHAR_DATA *ch );
void		make_scraps		( OBJ_DATA *obj );
OBJ_DATA   *make_money		( int amount );
OBJ_DATA   *make_trap		( int v0, int v1, int v2, int v3 );


#endif	/* MAKEOBJ_H */
