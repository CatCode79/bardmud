#ifndef OBJECT_H
#define OBJECT_H


/*
 * Funzioni
 */
void		wear_obj			( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, int wear_bit, const char *verb );
bool		remove_obj			( CHAR_DATA *ch, int iWear, bool fReplace );
obj_ret		damage_obj			( OBJ_DATA *obj );
void		obj_fall			( OBJ_DATA *obj, bool through );
DO_RET		give_money			( CHAR_DATA *ch, char *argument );
int			get_obj_resistance	( OBJ_DATA *obj );


#endif	/* OBJECT_H */
