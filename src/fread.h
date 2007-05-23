#ifndef FREAD_H
#define FREAD_H


/*
 * Funzioni
 */
#ifdef DEBUG_MEMORYLEAK
#define			fread_string( fp )	real_fread_string( fp, __FILE__, __LINE__ )
char		   *real_fread_string	( MUD_FILE *fp, const char *file, const int line );
#else
char		   *fread_string		( MUD_FILE *fp );
#endif

MUD_FILE	   *mud_fopen			( const char *dirname, const char *filename, const char *modes, bool msg );
char			fread_letter		( MUD_FILE *fp );
int				fread_number		( MUD_FILE *fp );
int				fread_vnum			( MUD_FILE *fp );
OBJ_PROTO_DATA *fread_vnum_obj		( MUD_FILE *fp );
MOB_PROTO_DATA *fread_vnum_mob		( MUD_FILE *fp );
ROOM_DATA	   *fread_vnum_room		( MUD_FILE *fp );
int				fread_positive		( MUD_FILE *fp );
int				fread_negative		( MUD_FILE *fp );
long			fread_long			( MUD_FILE *fp );
float			fread_float			( MUD_FILE *fp );
void			fread_to_eol		( MUD_FILE *fp );
char		   *fread_line			( MUD_FILE *fp );
char		   *fread_word			( MUD_FILE *fp );
char		   *fwrite_bool			( bool bit );
bool			fread_bool			( MUD_FILE *fp );
char		   *print_bool			( const bool value );
SYNO_DATA	   *fread_synonym		( MUD_FILE *fp, int type );
void			fwrite_synonym		( MUD_FILE *fp, SYNO_DATA *synonym, int type );
void			free_synonym		( SYNO_DATA *synonym );
void			fread_list			( const char *list_file, const char *section, FREAD_FUN *freadfun, bool unique );
void			fread_section		( const char *SECTION_FILE, const char *section, FREAD_FUN *freadfun, bool unique );


#endif	/* FREAD_H */
