#ifndef INFORMATION_H
#define INFORMATION_H


/*
 * Funzioni
 */
void	actiondesc			( CHAR_DATA *ch, OBJ_DATA *obj );
char   *format_obj_to_char	( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort );
void	show_char_to_char	( CHAR_DATA *list, CHAR_DATA *ch );
#ifdef T2_MXP
void	show_list_to_char	( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing, const int iDefaultAction );
#else
void	show_list_to_char	( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing );
#endif
char   *get_extradescr		( char *name, EXTRA_DESCR_DATA *ed );


#endif	/* INFORMATION_H */
