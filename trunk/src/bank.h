#ifndef BANK_H
#define BANK_H


/*
 * Funzioni
 */
void	update_share				( void );
void	load_share					( void );
int		get_share_value				( void );
int		get_share_band				( void );
void	load_accounts				( void );
void	remove_all_accounts_player	( CHAR_DATA *ch );
void	rename_account				( CHAR_DATA *ch, const char *newname );
void	free_all_accounts			( void );
void	show_player_accounts		( CHAR_DATA *ch, CHAR_DATA *victim );


#endif	/* BANK_H */
