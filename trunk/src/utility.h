#ifndef UTILITY_H
#define UTILITY_H


/*
 * Funzioni
 */
char   *str_str				 ( char *astr, char *bstr );
char   *strlstr				 ( char *line, char *token );
char   *strip_char			 ( const char *str, char c );
char   *strip_punct			 ( const char *str );
char   *strupper			 ( const char *str );
int		str_count			 ( char *psource, char *ptarget );
char   *one_command			 ( char *command, char *arg_first );
char   *reverse_ctime		 ( time_t *time_passed );
char   *stamp_time			 ( void );
char   *num_punct			 ( int foo );
char   *strip_crnl_end		 ( char *str );
void	init_mm				 ( void );
int		number_mm			 ( void );
void	smash_tilde			 ( char *str );
bool	str_cmp				 ( const char *astr, const char *bstr );
bool	str_prefix			 ( const char *astr, const char *bstr );
bool	str_prefix_acc		 ( const char *astr, const char *bstr );
bool	str_infix			 ( const char *astr, const char *bstr );
bool	str_suffix			 ( const char *astr, const char *bstr );
bool	str_cmp_exc			 ( const char *astr, const char *bstr );
bool	str_cmp_acc			 ( const char *astr, const char *bstr );
bool	str_cmp_all			 ( char *astr, bool dot );
char   *one_argument		 ( char *argument, char *arg_first );
char   *one_argument_editor	 ( char *argument, char *arg_first );	/* editor.c */
bool	is_name				 ( const char *str, char *namelist );
bool	is_name_acc			 ( const char *str, char *namelist );
bool	is_name_noquote		 ( const char *str, char *namelist );
bool	is_name_prefix		 ( const char *str, char *namelist );
bool	is_name_prefix_acc	 ( const char *str, char *namelist );
bool	nifty_is_name		 ( char *str, char *namelist );
bool	nifty_is_name_prefix ( char *str, char *namelist );
char   *capitalize			 ( const char *str );
char   *strlower			 ( const char *str );
void	show_file			 ( DESCRIPTOR_DATA *d, char *filename, bool msg );
void	append_to_file		 ( char *file, char *str );
int		number_fuzzy		 ( int number );
int		number_range		 ( int from, int to );
int		number_percent		 ( void );
int		number_bits			 ( int width );
void	tail_chain			 ( void );
char   *friendly_ctime		 ( time_t *time_passed );
char   *friendly_ctime_nohour( time_t *time_passed );
char   *stripclr			 ( char *text );				/* (GG) */
char   *strcolor_close		 ( const char *string );
char   *slash_code			 ( char *code );
char   *name_from_file		 ( char *text );
int		get_percent			 ( int val, int max );
int		inch_to_centimetre	 ( const int inch );
int		once_to_gram		 ( const int once );
char   *itostr				 ( int num );
bool	is_same_ip			 ( CHAR_DATA *ch, CHAR_DATA *wch );
#ifdef T2_ALFA
void	reverse_chars		 ( char *s );
#endif
bool	check_type_int		 ( int integer );


#endif	/* UTILITY_H */
