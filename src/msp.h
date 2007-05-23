#ifndef MSP_H
#define MSP_H

#ifdef T2_MSP

#include <arpa/telnet.h>


/*
 * Definizioni
 */
#define TELOPT_MSP 90


/*
 * Funzioni
 */
bool	use_msp				( DESCRIPTOR_DATA *d );
void	stop_music			( DESCRIPTOR_DATA *d );
void	stop_sound			( DESCRIPTOR_DATA *d );
void	stop_music_combat	( DESCRIPTOR_DATA *d );
void	send_audio_url		( DESCRIPTOR_DATA *d );
void	send_audio			( DESCRIPTOR_DATA *d, const char *name, int to );
void	check_audio_location( CHAR_DATA *ch );
void	send_music_combat	( CHAR_DATA *ch, CHAR_DATA *victim );
void	download_audio		( CHAR_DATA *ch );
void	update_sound		( void );


/*
 * Variabili
 */
extern	const unsigned char		msp_on_str[];
extern		  unsigned char		enable_msp[];


#endif	/* T2_MSP */

#endif	/* MSP_H */
