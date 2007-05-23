#ifdef T2_MXP

#ifndef MXP_H
#define MXP_H


/*****************************************************************************\
 >						Modulo di gestione protocollo MXP					 <
\*****************************************************************************/


#define TELOPT_MXP	'\x5B'

extern const unsigned char will_mxp_str[];
extern const unsigned char start_mxp_str[];
extern const unsigned char do_mxp_str[];
extern const unsigned char dont_mxp_str[];


/******************************************
 * Mxp stuff - Nick Gammon - 18 June 2001 *
 ******************************************/

/*
 * To simply using MXP we'll use special tags where we want to use MXP tags
 *	and then change them to <, > and & at the last moment.
 * eg.	MXP_BEG "send" MXP_END    becomes: <send>
 *		MXP_AMP "version;"        becomes: &version;
 */

/* strings */
#define MXP_BEG "\x03"		/* becomes < */
#define MXP_END "\x04"		/* becomes > */
#define MXP_AMP "\x05"		/* becomes & */

/* characters */
#define MXP_BEGc '\x03'		/* becomes < */
#define MXP_ENDc '\x04'		/* becomes > */
#define MXP_AMPc '\x05'		/* becomes & */

/* constructs an MXP tag with < and > around it */
#define MXPTAG(arg)  MXP_BEG arg MXP_END

#define ESC "\x1B"			/* esc character */

#define MXPMODE(arg) ESC "[" #arg "z"


/*
 * Flags for show_list_to_char
 */
typedef enum
{
	eItemNothing,	/* item is not readily accessible */
	eItemGet,		/* item on ground				  */
	eItemDrop		/* item in inventory			  */
} showlist_flag_types;


#define MXP_open		0	/* only MXP commands in the "open" category are allowed. */
#define MXP_secure		1	/* all tags and commands in MXP are allowed within the line. */
#define MXP_locked		2	/* no MXP or HTML commands are allowed in the line.  The line is not parsed for any tags at all. */
#define MXP_reset		3	/* close all open tags			 */
#define MXP_secure_once	4	/* next tag is secure only		 */
#define MXP_perm_open	5	/* open mode until mode change	 */
#define MXP_perm_secure	6	/* secure mode until mode change */
#define MXP_perm_locked	7	/* locked mode until mode change */


/*
 * Funzioni
 */
bool	use_mxp				( DESCRIPTOR_DATA *d );
void	mxp_toggle_language	( DESCRIPTOR_DATA *d );
void	turn_on_mxp			( DESCRIPTOR_DATA *d );
int		count_mxp_tags		( const bool bMXP, const char *txt, int length );
void	convert_mxp_tags	( const bool bMXP, char *dest, const char *src, int length );


#endif	/* MXP_H */

#endif	/* T2_MXP */
