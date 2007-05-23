#ifndef HANDLER_CH_H
#define HANDLER_CH_H


/*
 * Enumerazione delle flag di entità, cioè quelle settabili sia per mob che per pg
 */
typedef enum
{
	ENFLAG_FLYCHANGED,
	MAX_ENFLAG
} entity_flags;


/*
 * Variabili e costanti
 */
extern	const CODE_DATA	code_enflag[];
extern	const int		max_check_enflag;


/*
 * Funzioni
 */
CHAR_DATA  *ch_original			( DESCRIPTOR_DATA *d );
int			get_ac_natural		( CHAR_DATA *ch );
void		add_kill			( CHAR_DATA *ch, CHAR_DATA *mob );
int			times_killed		( CHAR_DATA *ch, CHAR_DATA *mob );
bool		is_drunk			( CHAR_DATA *ch, int drunk );
bool		is_floating			( CHAR_DATA *ch );
char	   *get_name			( CHAR_DATA *ch );
void		clean_char_queue	( void );
void		better_mental_state	( CHAR_DATA *ch, int mod );
bool		IS_MOB				( CHAR_DATA *ch );
bool		IS_PG				( CHAR_DATA *ch );
char	   *PERS				( CHAR_DATA *ch, CHAR_DATA *looker );
char	   *MORPHPERS			( CHAR_DATA *ch, CHAR_DATA *looker );
bool		HAS_BIT_PLR			( CHAR_DATA *ch, int bit );
bool		HAS_BIT_ACT			( CHAR_DATA *ch, int bit );
bool		is_in_game			( DESCRIPTOR_DATA *d );
CHAR_DATA  *get_original		( CHAR_DATA *ch );
bool		IS_ADMIN			( CHAR_DATA *ch );
bool		is_align			( CHAR_DATA *ch, int type );
char	   *get_align_alias		( const int align );
int			get_age				( CHAR_DATA *ch, int type );
int			get_curr_attr		( CHAR_DATA *ch, int attribute );
int			check_total_attr	( CHAR_DATA *ch );
int			get_curr_sense		( CHAR_DATA *ch, int sense );
int			check_total_sense	( CHAR_DATA *ch );
int			get_level			( CHAR_DATA *ch );
int			get_trust			( CHAR_DATA *ch );
int			get_ac				( CHAR_DATA *ch );
int			get_hitroll			( CHAR_DATA *ch );
int			get_damroll			( CHAR_DATA *ch );
bool		is_awake			( CHAR_DATA *ch );
int			can_carry_number	( CHAR_DATA *ch );
int			can_carry_weight	( CHAR_DATA *ch );
int			get_adept_skell		( CHAR_DATA *ch, int sn );
int			knows_skell			( CHAR_DATA *ch, int sn );
bool		is_outside			( CHAR_DATA *ch );
bool		can_see				( CHAR_DATA *ch, CHAR_DATA *victim );
void		char_from_room		( CHAR_DATA *ch );
void		char_to_room		( CHAR_DATA *ch, ROOM_DATA *pRoom );
void		extract_char		( CHAR_DATA *ch, bool fPull );
CHAR_DATA  *get_char_room		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_char_area		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_char_mud		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_mob_room		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_mob_area		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_mob_mud			( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_player_room		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_player_area		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_player_mud		( CHAR_DATA *ch, char *argument, bool check_see );
CHAR_DATA  *get_offline			( const char *argument, bool exact );
void		set_cur_char		( CHAR_DATA *ch );
bool		char_died			( CHAR_DATA *ch );
char	   *get_points_colored	( CHAR_DATA *ch, CHAR_DATA *victim, int point, bool prompt );
bool		stop_lowering_points( CHAR_DATA *ch, const int type );
int			check_total_points	( CHAR_DATA *ch );
bool		ms_find_obj			( CHAR_DATA *ch );
void		worsen_mental_state	( CHAR_DATA *ch, int mod );
void		WAIT_STATE			( CHAR_DATA *ch, int pulse );


#endif	/* HANDLER_CH_H */
