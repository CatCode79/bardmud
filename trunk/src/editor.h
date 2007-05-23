#ifndef EDITOR_H
#define EDITOR_H


/*
 * Character substates
 */
typedef enum
{
	SUB_NONE,
	SUB_PAUSE,
	SUB_PERSONAL_DESCR,
	SUB_PERSONAL_CHARACTER,
	SUB_PERSONAL_BIO,
	SUB_MOB_DESCR,
	SUB_OBJ_LONG,
	SUB_OBJ_EXTRA,
	SUB_ROOM_DESCR,
	SUB_ROOM_EXTRA,
	SUB_MPROG_EDIT,		/* (GG) da rimettere bene */
	SUB_WRITING_NOTE,
	SUB_BAN_DESCR,
	SUB_REPEAT_CMD,
	SUB_RESTRICTED,
	SUB_NEWS_POST,
	SUB_NEWS_EDIT,
	/* timer types ONLY below this point */
	SUB_TIMER_DO_ABORT = 128,
	SUB_TIMER_CANT_ABORT
	/* 26 tipi di substates */
} char_substates;


struct editor_data
{
	int		   numlines;
	int		   on_line;
	int		   size;
	char	   line[99][160];
};


/*
 * Funzioni
 */
void	editor_header	( CHAR_DATA *ch );
void	edit_buffer		( CHAR_DATA *ch, char *argument );
void	start_editing	( CHAR_DATA *ch, char *data );
void	stop_editing	( CHAR_DATA *ch );
char   *copy_buffer		( CHAR_DATA *ch );
bool	is_editing		( DESCRIPTOR_DATA *d );


#endif	/* EDITOR_H */
