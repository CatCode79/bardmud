/*--------------------------------------------------------------------------------------*\
                                      Copyright 2002, 2003, 2004, 2005 by T2Team         |
  Bard Code                           The Bard Code has been developed by T2Team:        |
  Driad Release     ..;;;;;.      (_) - Amlugil, Celyr, Miciko, Onirik, Raul, Sharas     |
  BARD v0.6u      .~`" >------___/  / Older Members of T2Team:                           |
               ,'"  J"!||||||||:/  /  - Jamirithiana, Qaba, Tanegashima                  |
    -~,      ,'  ,;"!|||||||||!/  /   ---------------------------------------------------|
 ;;/   `^~`~_._/!|||||||||||!//  /    The Bard Code is derived from:                     |
 "".  =====!/!|||||||||||||!//  /     - DikuMUD coded by: Hans Staerfeldt, Katja Nyboe,  |
 .'  / |||||||||||||||||||//   /        Tom Madsen, Michael Seifert and Sebastian Hammer |
.'  /  ||||||||||||||||!//    /       - MERC 2.1 coded by Hatchet, Furey, and Kahn       |
;  !   ||||||||||||||!/X/    /        - SMAUG 1.4a coded by Thoric (Derek Snider) with:  |
:  !   ||||||||||||!/X/     /           Altrag, Blodkai, Haus, Narn, Scryn, Swordbearer, |
:  !   |||||||||||/X/      /            Tricops, Gorog, Rennard, Grishnakh, Fireblade    |
:  !   |||||||||!/X/      /             and Nivek                                        |
:   \  |||||||!/X/       /                                                               |
 "   \ |||||!/x/        /                                                                |
  "   \|||!/X/         /                                                                 |
   \   \|/X/          /                                                                  |
    "   !/           /                ... and the blood!                                 |
     ;'/            /                                                                    |
      "------------(                  --------------------------------------------------*/


/****************************************************************************\
 >                    Modulo di comunicazione Basso-livello                 <
\****************************************************************************/

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

/* Include riguardanti socket e TCP/IP */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

#include "mud.h"
#include "affect.h"
#include "animation.h"
#include "ban.h"
#include "calendar.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "gramm.h"
#include "interpret.h"
#include "mccp.h"
#include "mprog.h"
#include "msp.h"
#include "mxp.h"
#include "nanny.h"
#include "olc.h"
#include "question.h"
#include "morph.h"
#include "reboot.h"
#include "room.h"
#include "save.h"


const /*unsigned*/ char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const /*unsigned*/ char	echo_on_str		[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const /*unsigned*/ char	go_ahead_str	[] = { IAC, GA, '\0' };
const unsigned char		eor_on_str		[] = { IAC, WILL, TELOPT_EOR, '\0' };

#ifdef T2_DETCLIENT
/* Terminal detection */
#define	TERMINAL_TYPE	'\x18'
#define	IS				'\x00'
#define	SEND			'\x01'

const /*unsigned*/ char will_termtype_str	[] = { IAC, WILL, TERMINAL_TYPE, '\0' };
const /*unsigned*/ char wont_termtype_str	[] = { IAC, WONT, TERMINAL_TYPE, '\0' };
const /*unsigned*/ char do_termtype_str		[] = { IAC, DO, TERMINAL_TYPE, '\0' };
const /*unsigned*/ char term_call_back_str	[] = { IAC, SB, TERMINAL_TYPE, IS };
const /*unsigned*/ char req_termtype_str	[] = { IAC, SB, TERMINAL_TYPE, SEND, IAC, SE, '\0' };
#endif


/*
 * Definizioni locali
 */
#define GREETING_FILE	SYSTEM_DIR "greeting.file"	/* File di login al mud */


/*
 * Variabili globali
 */
DESCRIPTOR_DATA *first_descriptor = NULL;	/* First descriptor */
DESCRIPTOR_DATA *last_descriptor  = NULL;	/* Last descriptor */
DESCRIPTOR_DATA *d_next;					/* Next descriptor in loop */
int				 num_descriptors = 0;
bool			 service_shut_down;			/* Shutdown by operator closing down service */
time_t			 current_time;				/* Time of this pulse */
int				 control;					/* Controlling descriptor */
int				 newdesc;					/* New descriptor */
fd_set			 in_set;					/* Set of desc's for reading */
fd_set			 out_set;					/* Set of desc's for writing */
fd_set			 exc_set;					/* Set of desc's with errors */
int 			 maxdesc;
char			*alarm_section = "(sconosciuto)";


int init_socket( void )
{
	char		hostname[64];
	struct		sockaddr_in	 sa;
	socklen_t	x = 1;	/* (TT) era int */
	int			fd;

	gethostname( hostname, sizeof(hostname) );	/* (FF) provare ad utilizzare al suo posto la uname */

	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( fd < 0 )
	{
		send_log( NULL, LOG_BUG, "init_socket: socket" );
		exit( EXIT_FAILURE );
	}

	if ( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &x, sizeof(x)) < 0 )
	{
		send_log( NULL, LOG_BUG, "init_socket: SO_REUSEADDR" );
		close( fd );
		exit( EXIT_FAILURE );
	}

#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct	linger	ld;

		ld.l_onoff  = 1;
		ld.l_linger = 1000;

		if ( setsockopt(fd, SOL_SOCKET, SO_DONTLINGER, (void *) &ld, sizeof(ld)) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			close( fd );
			exit( EXIT_FAILURE );
		}
	}
#endif

	memset( &sa, 0, sizeof(sa) );
	sa.sin_family = AF_INET;
	sa.sin_port	  = htons( sysdata.mud_port );

	if ( bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1 )
	{
		perror( "Init_socket: bind" );
		close( fd );
		exit( EXIT_FAILURE );
	}

	if ( listen(fd, sysdata.max_socket_con) < 0 )
	{
		perror( "Init_socket: listen" );
		close( fd );
		exit( EXIT_FAILURE );
	}

	return fd;
}


/*
 * Allarme LAG
 */
void caught_alarm( int num_alarm )
{
    char buf[MSL];

	sprintf( buf, "caught_alarm: allarme lag nella sezione %s", alarm_section );
	send_log( NULL, LOG_WARNING, "%s", buf );

	if ( newdesc != 0 )
	{
		FD_CLR( newdesc, &in_set );
		FD_CLR( newdesc, &out_set );
		FD_CLR( newdesc, &exc_set );
		send_log( NULL, LOG_NORMAL, "caught_alarm: pulizia del newdesc" );
	}
}


bool check_bad_desc( int desc )
{
	if ( FD_ISSET(desc, &exc_set) )
	{
		FD_CLR( desc, &in_set );
		FD_CLR( desc, &out_set );
		send_log( NULL, LOG_NORMAL, "Bad FD caught and disposed." );
		return TRUE;
	}

	return FALSE;
}


void init_descriptor( DESCRIPTOR_DATA *dnew, int desc )
{
	dnew->next			= NULL;
	dnew->descriptor	= desc;
	dnew->connected		= CON_GET_NAME;
	dnew->outsize		= 2000;
	dnew->idle			= 0;
	dnew->lines			= 0;
	dnew->total_output	= 0;
	dnew->password		= NULL;
	dnew->name			= NULL;
	dnew->tempnum		= 0;
	dnew->reroll		= 0;

#ifdef T2_DETCLIENT
	dnew->client		= str_dup( "???" );
#endif

#ifdef T2_MCCP
	CREATE( dnew->mccp, MCCP_DATA, 1 );
	dnew->mccp->can_compress	= FALSE;
	dnew->mccp->compressing		= 0;
#endif
#ifdef T2_MSP
	dnew->msp			= FALSE;
#endif
#ifdef T2_MXP
	dnew->mxp			= FALSE;	/* All'inizio l'MXP è disattivo */
#endif
	dnew->animation		= -1;
	dnew->frame			= -1;

	CREATE( dnew->outbuf, char, dnew->outsize );
}


void free_desc( DESCRIPTOR_DATA *d )
{
	close( d->descriptor );

	DISPOSE( d->host );
	DISPOSE( d->host_old );
#ifdef T2_DETCLIENT
	DISPOSE( d->client );
#endif
	DISPOSE( d->outbuf );
	DISPOSE( d->pagebuf );
	DISPOSE( d->name );
	DISPOSE( d->password );

#ifdef T2_MCCP
	compressEnd( d, d->mccp->compressing );
	DISPOSE( d->mccp );
#endif

	cleanup_olc( d );
#ifdef T2_QUESTION
	free_question_handler_desc( d );
#endif

	DISPOSE( d );
/*	--num_descriptors;  	* This is called from more than close_socket */
}


/*
 * Ritorna l'host del pg passato
 */
char *get_host( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_host: ch passato è NULL" );
		return "(errore)";
	}

	if ( ch->desc && VALID_STR(ch->desc->host) )
		return ch->desc->host;

	if ( ch->pg && VALID_STR(ch->pg->host) )
		return ch->pg->host;

	return "(sconosciuto)";
}


/*
 * Writes to a descriptor, usually best used when there's no character to send to ( like logins )
 */
void send_to_descriptor( DESCRIPTOR_DATA *d, const char *txt )
{
	char	   *colstr;
	const char *prevstr = txt;
	char		colbuf[20];
	int			ln;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "send_to_descriptor: descrittore NULL." );
		return;
	}

	if ( txt && d->descriptor )
	{
		colbuf[0] = '\0';	/* Inizializza colbuf */

		while( (colstr = strpbrk( prevstr, "&^}" )) != NULL )
		{
			if ( colstr > prevstr )
				write_to_buffer( d, prevstr, (colstr-prevstr) );

			ln = colorcode( colstr, colbuf, d->character );
			if ( ln < 0 )
			{
				prevstr = colstr+1;
				break;
			}
			else if ( ln > 0 )
				write_to_buffer( d, colbuf, ln );

			prevstr = colstr + 2;	/* Salta il codice di colore */
		}
	}

	if ( *prevstr )
		write_to_buffer( d, prevstr, 0 );
}


void new_descriptor( int new_desc )
{
	DESCRIPTOR_DATA	   *dnew;
	struct hostent	   *from;
	struct sockaddr_in	sock;
	socklen_t			size;
	char				buf[MSL];
	int					desc;

	size = sizeof( sock );

	if ( check_bad_desc(new_desc) )
	{
		set_alarm( 0 );
		return;
	}

	set_alarm( 20 );
	alarm_section = "new_descriptor: accept";

	if ( (desc = accept(new_desc, (struct sockaddr *) &sock, &size)) < 0 )
	{
		perror( "new_descriptor: accept" );
		send_log( NULL, LOG_COMM, "new_descriptor: accept" );
		set_alarm( 0 );
		return;
	}

	if ( check_bad_desc(new_desc) )
	{
		set_alarm( 0 );
		return;
	}

#if !defined( FNDELAY )
#  define FNDELAY O_NDELAY
#endif

	set_alarm( 20 );
	alarm_section = "new_descriptor: dopo accept";

	if ( fcntl(desc, F_SETFL, FNDELAY) == -1 )
	{
		perror( "New_descriptor: fcntl: FNDELAY" );
		set_alarm( 0 );
		return;
	}
	if ( check_bad_desc(new_desc) )
		return;

	CREATE( dnew, DESCRIPTOR_DATA, 1 );

	init_descriptor( dnew, desc );
	dnew->port = ntohs( sock.sin_port );

	strcpy( buf, inet_ntoa(sock.sin_addr) );
	send_log( NULL, LOG_COMM, "Sock.sinaddr:  %s, porta %hd.", buf, dnew->port );

	DISPOSE( dnew->host );
	if ( !sysdata.name_resolving )
	{
		dnew->host = str_dup( buf );
	}
	else
	{
		from = gethostbyaddr( (char *) &sock.sin_addr, sizeof(sock.sin_addr), AF_INET );
		dnew->host = str_dup( (char *) (from)  ?  from->h_name  :  buf );
	}

	if ( check_total_bans(dnew) )
	{
		write_to_descriptor( dnew, "Il tuo sito è stato esiliato dal questo Mud.\r\n", 0 );
		free_desc( dnew );
		set_alarm( 0 );
		return;
	}

	/* Init descriptor data. */
	if ( !last_descriptor && first_descriptor )
	{
		DESCRIPTOR_DATA *d;

		send_log( NULL, LOG_BUG, "new_descriptor: last_desc è NULL, ma non first_desc! ...correzione" );
		for ( d = first_descriptor;  d;  d = d->next )
		{
			if ( !d->next )
				last_descriptor = d;
		}
	}

	LINK( dnew, first_descriptor, last_descriptor, next, prev );

#ifdef T2_MCCP
	write_to_buffer( dnew, (char *) eor_on_str,			0 );
	write_to_buffer( dnew, (char *) compress2_on_str,	0 );
	write_to_buffer( dnew, (char *) compress_on_str,	0 );
#endif

	/* Controlla se il client possiede il protocollo msp */
	write_to_buffer( dnew, (char *) eor_on_str, 0 );
#ifdef T2_MSP
	write_to_buffer( dnew, (char *) msp_on_str, 0 );
#endif

	/* Invia la schermata iniziale di greeting */
	show_file( dnew, GREETING_FILE, TRUE );

	if ( ++num_descriptors > mudstat.boot_max_players )
		mudstat.boot_max_players = num_descriptors;

	if ( mudstat.boot_max_players > mudstat.alltime_max )
	{
		DISPOSE( mudstat.time_max );
		mudstat.time_max = str_dup( friendly_ctime(&current_time) );
		mudstat.alltime_max = mudstat.boot_max_players;
		send_log( NULL, LOG_COMM, "Record di giocatori massimi sul Mud: %d!", mudstat.alltime_max );
		save_sysdata( );
	}
	/* (GG) (RR) fare anche la cosa del numero e data max nomulty del mudstat. */

	set_alarm( 0 );
}


void accept_new( int ctrl )
{
	static struct timeval null_time;
	DESCRIPTOR_DATA *d;

	/* Poll all active descriptors. */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( ctrl, &in_set );
	maxdesc = ctrl;
	newdesc = 0;
	for ( d = first_descriptor;  d;  d = d->next )
	{
		maxdesc = UMAX( maxdesc, d->descriptor );
		FD_SET( d->descriptor, &in_set  );
		FD_SET( d->descriptor, &out_set );
		FD_SET( d->descriptor, &exc_set );
		if ( d == last_descriptor )
			break;
	}

	if ( select(maxdesc+1, &in_set, &out_set, &exc_set, &null_time) < 0 )
	{
		perror( "accept_new: select: poll" );
		exit( EXIT_FAILURE );
	}

	if ( FD_ISSET(ctrl, &exc_set) )
	{
		send_log( NULL, LOG_BUG, "accept_new: eccezione chiamata nel controllare il descrittore %d", ctrl );
		FD_CLR( ctrl, &in_set );
		FD_CLR( ctrl, &out_set );
	}
	else if ( FD_ISSET(ctrl, &in_set) )
	{
		newdesc = ctrl;
		new_descriptor( newdesc );
	}
}


bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
#ifdef T2_DETCLIENT
	unsigned char   *p;
#endif
	int				iStart;
	int				iErr;

	/* Hold horses if pending command already. */
	if ( VALID_STR(d->incomm) )
		return TRUE;

#ifdef T2_DETCLIENT
	/*
	 * Look for a client response
	 */
	for ( p = (unsigned char *) d->inbuf;  *p;  p++ )
	{
		if ( *p == IAC )
		{
			if ( memcmp(p, will_termtype_str, strlen(will_termtype_str)) == 0 )
			{
				memmove( p, &p[strlen(will_termtype_str)], strlen((char*) &p[strlen(will_termtype_str)]) + 1 );
				p--;
				write_to_buffer( d, req_termtype_str, 0 );
			}
			else if ( memcmp(p, wont_termtype_str, strlen(wont_termtype_str)) == 0 )
			{
				memmove(p, &p[strlen(wont_termtype_str)], strlen((char*) &p[strlen(wont_termtype_str)]) + 1 );
				p--;
			}
			else if ( memcmp(p, term_call_back_str, strlen(term_call_back_str)) == 0 )
			{
				char tempbuf[120];

				sprintf( tempbuf, "%s", p+4 );
				tempbuf[strlen(tempbuf)-2] = '\0';
				DISPOSE( d->client );
				d->client = str_dup( tempbuf );
				memmove( p, &p[strlen((char*) p)], strlen((char*) &p[strlen((char*) p)]) + 1 );
				p--;
			}
		}
	} /* end of finding an IAC */
#endif

	/* Check for overflow. */
	iStart = strlen( d->inbuf );
	if ( iStart >= (int) sizeof(d->inbuf) - 10 )
	{
		send_log( NULL, LOG_NORMAL, "read_from_descriptor: %s overflow in input!", d->host );
		write_to_descriptor( d,
			"\r\n** Diamoci un taglio!! **"
			"\r\nNon puoi inviare lo stesso comando più di 20 volte consecutivamente!\r\n", 0 );
		return FALSE;
	}

	for ( ; ; )
	{
		int nRead;

		nRead = recv( d->descriptor, d->inbuf + iStart, sizeof(d->inbuf) - 10 - iStart, 0 );

		iErr = errno;

		if ( nRead > 0 )
		{
			iStart += nRead;
			if ( d->inbuf[iStart-1] == '\r' || d->inbuf[iStart-1] == '\n' )
				break;
		}
		else if ( nRead == 0 )
		{
			send_log( NULL, LOG_COMM, "read_from_descriptor: EOF incontrato nella lettura." );
			return FALSE;
		}
		else if ( iErr == EWOULDBLOCK )
		{
			break;
		}
		else
		{
			perror( "Read_from_descriptor" );
			return FALSE;
		}
	} /* chiude il for */

	d->inbuf[iStart] = '\0';
	return TRUE;
}


/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
	int		i;
	int		j;
	int		k;
	int		iac = 0;
#ifdef T2_MXP
	unsigned char *p;
#endif

	/* Hold horses if pending command already */
	if ( VALID_STR(d->incomm) )
		return;

#ifdef T2_MXP
	/* Look for incoming telnet negotiation */
	for ( p = (unsigned char*) d->inbuf;  *p;  p++ )
	{
		if ( *p == IAC )
		{
			if ( memcmp(p, do_mxp_str, strlen((char*)do_mxp_str)) == 0 )
			{
				turn_on_mxp( d );
				/* remove string from input buffer */
				memmove( p,		  &p[strlen((char*)do_mxp_str)],
					strlen((char*)&p[strlen((char*)do_mxp_str)]) + 1 );
				p--;	/* adjust to allow for discarded bytes */
			} /* end of turning on MXP */
			else  if ( memcmp(p, dont_mxp_str, strlen((char*)dont_mxp_str)) == 0 )
			{
				d->mxp = FALSE;
				/* remove string from input buffer */
				memmove (p,		  &p[strlen((char*)dont_mxp_str)],
					strlen((char*)&p[strlen((char*)dont_mxp_str)]) + 1 );
				p--;	/* adjust to allow for discarded bytes */
			} /* end of turning off MXP */
		} /* end of finding an IAC */
	}
#endif

	/* Look for at least one new line. */
	for ( i = 0;  d->inbuf[i] != '\r' && d->inbuf[i] != '\n' && i<MAX_INBUF_SIZE;  i++ )
	{
		if ( d->inbuf[i] == '\0' )
			return;
	}

	/* Canonical input processing. */
	for ( i = 0, k = 0;  d->inbuf[i] != '\r' && d->inbuf[i] != '\n';  i++ )
	{
		if ( k >= 254 )
		{
			write_to_descriptor( d, "Linea troppo lunga.\r\n", 0 );

			d->inbuf[i]   = '\n';
			d->inbuf[i+1] = '\0';
			break;
		}

		if ( d->inbuf[i] == (signed char)IAC )
		{
			iac = 1;
		}
		else if ( iac == 1 && (d->inbuf[i] == (signed char)DO || d->inbuf[i] == (signed char)DONT) )
		{
			iac = 2;
		}
		else if ( iac == 2 )
		{
			iac = 0;
#ifdef T2_MCCP
			if ( d->inbuf[i] == (signed char)TELOPT_COMPRESS )
			{
				if ( d->inbuf[i-1] == (signed char)DO && !d->mccp->compressing )
					compressStart( d, TELOPT_COMPRESS );
				else if ( d->inbuf[i-1] == (signed char)DONT && d->mccp->compressing == TELOPT_COMPRESS )
					compressEnd( d, TELOPT_COMPRESS );
			}
			else if ( d->inbuf[i] == (signed char)TELOPT_COMPRESS2 )
			{
				if ( d->inbuf[i-1] == (signed char)DO && !d->mccp->compressing )
					compressStart( d, TELOPT_COMPRESS2 );
				else if ( d->inbuf[i-1] == (signed char)DONT && d->mccp->compressing == TELOPT_COMPRESS2 )
					compressEnd( d, TELOPT_COMPRESS2 );
			}
			else
#endif
#ifdef T2_MSP
			if ( d->inbuf[i] == (signed char)TELOPT_MSP )
			{
				if ( d->inbuf[i-1] == (signed char)DO )
					d->msp = TRUE;
			}
#endif
		}
		else if ( d->inbuf[i] == '\b' && k > 0 )
			--k;
		else if ( (isascii(d->inbuf[i]) && isprint(d->inbuf[i])) || is_accent(d->inbuf[i]) )
			d->incomm[k++] = d->inbuf[i];
	}

	/* Finish off the line. */
	if ( k == 0 )
		d->incomm[k++] = ' ';

	d->incomm[k] = '\0';

	/* Deal with bozos with #repeat 1000 ... */
	if ( k > 1 )
	{
		if ( !strcmp(d->incomm, d->inlast) )
		{
			if ( ++d->repeat >= 20 )
			{
/*				send_log( NULL, LOG_NORMAL, "%s input spamming!", d->host ); */
				write_to_descriptor( d,
					"\r\n** Diamoci un taglio!! **\r\nNon puoi mandare lo stesso comando 20 volte di seguito!\r\n", 0 );
				strcpy( d->incomm, translate_command(d->character, "quit") );	/* (bb) si ma durante il combattimento sparerebbe ilm sg di adrenalina alta e non quitterebbe.. */
			}
		}
		else
		{
			d->repeat = 0;
		}
	}

	/* Do '!' substitution. */
	if ( d->incomm[0] == '!' )
		strcpy( d->incomm, d->inlast );
	else
		strcpy( d->inlast, d->incomm );

	/* Shift the input buffer. */
	while ( d->inbuf[i] == '\r' || d->inbuf[i] == '\n' )
		i++;

	for ( j = 0;  ( d->inbuf[j] = d->inbuf[i+j] ) != '\0';  j++ )
	{
		;
	}
}


/*
 * Controlla se il pg sia idle.
 */
bool is_idle( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_idle: ch è NULL" );
		return FALSE;
	}

	if ( IS_MOB(ch) )
		return FALSE;

	if ( HAS_BIT_PLR(ch, PLAYER_IDLE) )
		return TRUE;

	return FALSE;
}


void stop_idling( CHAR_DATA *ch )
{
	ROOM_DATA *was_in_room;

	if ( !ch )									return;
	if ( IS_MOB(ch) )							return;
	if ( ch->desc == NULL )						return;
	if ( ch->desc->connected != CON_PLAYING )	return;
	if ( !is_idle(ch) )							return;

	ch->timer = 0;
	was_in_room = ch->was_in_room;
	char_from_room( ch );
	char_to_room( ch, was_in_room );
	ch->was_in_room = ch->in_room;

	REMOVE_BIT( ch->pg->flags, PLAYER_IDLE );

	/* Non invia il messaggio di stop_idle se admin */
	if ( !IS_ADMIN(ch) )
		act( AT_ACTION, "$n è tornato da un risucchio astrale.", ch, NULL, NULL, TO_ROOM );
}


/*
 * Append onto an output buffer.
 */
void write_to_buffer_handler( DESCRIPTOR_DATA *d, const char *txt, int length, bool snoop )
{
#ifdef T2_MXP
	int origlength;
#endif

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "write_to_buffer: il descrittore è NULL." );
		return;
	}

	/* Normally a bug... but can happen if loadup is used. */
	if ( !d->outbuf )	/* (TT) immagino che se mettessi un VALID_STR qui eviterei di inviare righe vuote ai pg, sarà un bene? */
		return;

	/* Find length in case caller didn't. */
	if ( length <= 0 )
		length = strlen( txt );

#ifdef T2_MXP
	origlength = length;

	/* work out how much we need to expand/contract it */
	length += count_mxp_tags( d->mxp, txt, length );
#endif

	/* Initial \r\n if needed. */
	if ( d->outtop == 0 && !d->fcommand )
	{
		d->outbuf[0] = '\r';
		d->outbuf[1] = '\n';
		d->outtop	 = 2;
	}

	/* Expand the buffer as needed. */
	while ( d->outtop + length >= d->outsize )
	{
		if ( d->outsize > MSL*8 )	/* 32768 */
		{
			/* empty buffer */
			d->outtop = 0;
			send_log( NULL, LOG_BUG, "write_to_buffer: overflow di buffer. Chiusura di %s.",
				d->character  ?  d->character->name  :  "???" );
			close_socket( d, TRUE );
			return;
		}
		d->outsize *= 2;
		RECREATE( d->outbuf, char, d->outsize );
	}

	/* Copy. */
#ifdef T2_MXP
	convert_mxp_tags( d->mxp, d->outbuf + d->outtop, txt, origlength );
#else
	strncpy( d->outbuf + d->outtop, txt, length );
#endif

	if ( d->snoop_by && snoop )
		write_to_buffer( d->snoop_by, txt, length );

	d->outtop += length;
	d->outbuf[d->outtop] = '\0';
}

void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
	write_to_buffer_handler( d, txt, length, TRUE );
}

void write_to_buffer_private( DESCRIPTOR_DATA *d, const char *txt, int length )
{
	write_to_buffer_handler( d, txt, length, FALSE );
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *  try lowering the max block size.
 */
bool write_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
{
	char	*acc;
	int		 iStart = 0;
	int		 nWrite = 0;
	int		 nBlock = 0;
	int		 x, y;
	int		 num_accents = 0;
#ifdef T2_MCCP
	int		 len;
#endif

	if ( length <= 0 )
		length = strlen( txt );

	if ( !d )
		return FALSE;

	if ( d->character && HAS_BIT_PLR(d->character, PLAYER_ACCENT) )
		acc = txt;
	else
	{
		/* Conta quanti accenti ci sono nella stringa da inviare */
		for ( x = 0;  x < length;  x++ )
		{
			if ( is_accent(txt[x]) )
				num_accents++;
		}

		/* Crea una stringa grande quando txt + il numero di accenti trovati */
		CREATE( acc, char, length + num_accents + 1 );

		for ( x = 0, y = 0;  x < length;  x++, y++ )
		{
			switch ( txt[x] )
			{
			  default:
				acc[y] = txt[x];	/* Copia il carattere così comé se non accentato */
				break;
			  case 'à':		acc[y++] = 'a';		acc[y] = '\'';		break;
			  case 'è':		acc[y++] = 'e';		acc[y] = '\'';		break;
			  case 'é':		acc[y++] = 'e';		acc[y] = '\'';		break;
			  case 'ì':		acc[y++] = 'i';		acc[y] = '\'';		break;
			  case 'ò':		acc[y++] = 'o';		acc[y] = '\'';		break;
			  case 'ù':		acc[y++] = 'u';		acc[y] = '\'';		break;
			}
		}

		/* Aggiunge a length il numero di accenti ora che non serve più il suo valore originale */
		length += num_accents;
		acc[y] = '\0';
	}

#ifdef T2_MCCP
	/* Check for output compression */
	if ( use_mccp(d) && d->mccp->out_compress )
	{
		d->mccp->out_compress->next_in = (unsigned char *)acc;
		d->mccp->out_compress->avail_in = length;

		while ( d->mccp->out_compress->avail_in )
		{
			d->mccp->out_compress->avail_out = COMPRESS_BUF_SIZE - ( d->mccp->out_compress->next_out - d->mccp->out_compress_buf );

			if ( d->mccp->out_compress->avail_out )
			{
				int status = deflate( d->mccp->out_compress, Z_SYNC_FLUSH );

				if ( status != Z_OK )
				{
					if ( acc != txt )
						DISPOSE( acc );
					return FALSE;	/* Boom */
				}
			}

			/* Try to write out some data.. */
			len = d->mccp->out_compress->next_out - d->mccp->out_compress_buf;
			if ( len > 0 )
			{
				/* we have some data to write */
				for ( iStart = 0;  iStart < len;  iStart += nWrite )
				{
					nBlock = UMIN( length - iStart, 4096 );
					nWrite = send( d->descriptor, acc + iStart, nBlock, 0 );

					if ( nWrite < 0 )
					{
						perror( "Write_to_descriptor: compressed" );
						if ( acc != txt )
							DISPOSE( acc );
						return FALSE;
					}

					if ( !nWrite )
						break;
				}

				if ( !iStart )
					break;	/* Can't write any more */

				/* We wrote "iStart" bytes */
				if ( iStart < len )
					memmove( d->mccp->out_compress_buf, d->mccp->out_compress_buf+iStart, len - iStart );

				d->mccp->out_compress->next_out = d->mccp->out_compress_buf + len - iStart;
			}
		}
		if ( acc != txt )
			DISPOSE( acc );
		return TRUE;
	}
#endif

	for ( iStart = 0;  iStart < length;  iStart += nWrite )
	{
		d->total_output += length;				/* Salva il totale di quanto è stato inviato al descrittore */
		mudstat.total_output += length;

		nBlock = UMIN( length - iStart, 4096 );
		nWrite = send( d->descriptor, acc + iStart, nBlock, 0 );

		if ( nWrite < 0 )
		{
			perror( "Write_to_descriptor" );
			if ( acc != txt )
				DISPOSE( acc );
			return FALSE;
		}
	}

	if ( acc != txt )
		DISPOSE( acc );
	return TRUE;
}


void write_to_pager( DESCRIPTOR_DATA *d, const char *txt, int length )
{
	unsigned long oldsize;
#ifdef T2_MXP
	int origlength;
#endif

	if ( length <= 0 )
		length = strlen( txt );
	if ( length == 0 )
		return;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "write_to_pager: d è NULL" );
		return;
	}

	oldsize = d->pagesize;
	
	if ( oldsize )
		oldsize--;

	if ( d->snoop_by )
		write_to_pager( d->snoop_by, txt, length );

#ifdef T2_MXP
	origlength = length;

	/* work out how much we need to expand/contract it */
	length += count_mxp_tags( d->mxp, txt, length );
#endif

	if ( !d->pagebuf )
	{
		d->pagesize = length+1;
		oldsize = 0;
		d->pagecmd = '\0';
		CREATE( d->pagebuf, char, d->pagesize );
		d->pagebuf[0] = '\0';
		if ( !d->fcommand )
		{
			d->pagesize += 2;
			oldsize += 2;
			RECREATE( d->pagebuf, char, d->pagesize+1 );
			sprintf( d->pagebuf, "\r\n" );
		}
	}
	else
	{
		if ( d->pagesize + length <= UINT_MAX )
		{
			d->pagesize += length;
		}
		else
		{
			send_log( NULL, LOG_BUG, "Pager overflow. Ignoring." );
			DISPOSE( d->pagebuf );
			d->pagebuf = NULL;
			d->pagesize = 0;
			return;
		}
	}

	RECREATE( d->pagebuf, char, d->pagesize );

#ifdef T2_MXP
	convert_mxp_tags( d->mxp, d->pagebuf, txt, origlength );	/* (bb) (TT) (FF) forse bisogna rivedere questa parte con le nuove modifiche della pager */
#else
	strncpy( (char *)d->pagebuf+oldsize, txt, length );
	d->pagebuf[d->pagesize-1] = '\0';
#endif
}


/*
 * Write to one char. Convert color into ANSI sequences.
 */
void send_to_char( CHAR_DATA *ch, const char *txt )
{
	char		*colstr;
	const char	*prevstr = txt;
	char		 colbuf[20];
	int			 ln;

	if ( ch == NULL )
	{
		send_log( NULL, LOG_BUG, "send_to_char: NULL ch!" );
		return;
	}

	if ( txt && ch->desc )
	{
		colbuf[0] = '\0';	/* Inizializza colbuf */

		while ( (colstr = strpbrk(prevstr, "&^}")) != NULL )
		{
			if ( colstr > prevstr )
				write_to_buffer( ch->desc, prevstr, (colstr - prevstr) );

			ln = colorcode( colstr, colbuf, ch );
			if ( ln < 0 )
			{
				prevstr = colstr+1;
				break;
			}
			else if ( ln > 0 )
				write_to_buffer( ch->desc, colbuf, ln );

			prevstr = colstr+2;
		}
		if ( *prevstr )
			write_to_buffer( ch->desc, prevstr, 0 );
	}
}


void send_to_pager( CHAR_DATA *ch, const char *txt )
{
	char	   *colstr;
	const char *prevstr = txt;
	char		colbuf[20];
	int			ln;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "send_to_pager: NULL ch!" );
		return;
	}

	if ( txt && ch->desc )
	{
		DESCRIPTOR_DATA *d = ch->desc;
		colbuf[0] = '\0';	/* Inizializza colbuf */

		if ( !d->original || !d->original->desc )
			ch = d->character;
		else
			ch = d->original;

		/* Se è mob oppure il pager non è attivo invia il testo completamente */
		if ( IS_MOB(ch) || ch->pg->pagerlen == -1 )
		{
			send_to_char( d->character, txt );
			return;
		}

		while( (colstr = strpbrk(prevstr, "&^}")) != NULL )
		{
			if ( colstr > prevstr )
				write_to_pager( ch->desc, prevstr, (colstr-prevstr) );

			ln = colorcode( colstr, colbuf, ch );
			if ( ln < 0 )
			{
				prevstr = colstr+1;
				break;
			}
			else if ( ln > 0 )
				write_to_pager( ch->desc, colbuf, ln );

			prevstr = colstr+2;
		}
		if ( *prevstr )
			write_to_pager( ch->desc, prevstr, 0 );
	}
}


/*
 * The primary output interface for formatted output.
 */
void ch_printf( CHAR_DATA *ch, const char *fmt, ... )
{
	char			 buf[MSL*2];
	va_list			 args;

	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );

	send_to_char( ch, buf );
}


void pager_printf( CHAR_DATA *ch, const char *fmt, ... )
{
	char			 buf[MSL*2];
	va_list			 args;

	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );

	send_to_pager( ch, buf );
}


void send_to_descriptor_nocolor( DESCRIPTOR_DATA *d, const char *txt )
{
	if ( !d )
	{
		send_log( NULL, LOG_BUG, "send_to_descriptor_nocolor: d è NULL" );
		return;
	}

	if ( txt && d->descriptor )
		write_to_buffer( d, txt, 0 );
}


void send_to_char_nocolor( CHAR_DATA *ch, const char *txt )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "send_to_char_nocolor: ch è NULL" );
		return;
	}

	if ( txt && ch->desc )
		write_to_buffer( ch->desc, txt, 0 );
}

void send_to_pager_nocolor( CHAR_DATA *ch, const char *txt )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "send_to_pager_nocolor: ch è NULL" );
		return;
	}

	if ( txt && ch->desc )
	{
		DESCRIPTOR_DATA *d = ch->desc;

		ch = d->original ? d->original : d->character;
		if ( IS_MOB(ch) || ch->pg->pagerlen == -1 )
		{
			send_to_char( d->character, txt );
			return;
		}

		write_to_pager( d, txt, 0 );
	}
}


/*
 * The primary output interface for formatted output.
 */
void ch_printf_nocolor( CHAR_DATA *ch, const char *fmt, ... )
{
	char			 buf[MSL*2];
	va_list			 args;

	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );

	send_to_char_nocolor( ch, buf );
}

void pager_printf_nocolor( CHAR_DATA *ch, const char *fmt, ... )
{
	char			 buf[MSL*2];
	va_list			 args;

	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );

	send_to_pager_nocolor( ch, buf );
}


/*
 * Il colore della vita scelto nella display_prompt.
 */
char *default_prompt( CHAR_DATA *ch )
{
	static char buf[MIL];

	if ( IS_ADMIN(ch) )
	{
		strcpy( buf, "&c%k&g%n: "			);
		strcat( buf, "%l&w/&R%L Vita &w| "	);
		strcat( buf, "%v&w/&G%V Movi &w| "	);
		strcat( buf, "%m&w/&C%M Mana &w| "	);
		strcat( buf, "%o&w/&W%O Anima&w "	);
	}
	else
	{
		strcpy( buf, "&c%k&g%n: "			);
		strcat( buf, "%l &RVita &w| "		);
		strcat( buf, "%v &GMovi &w| "		);
		strcat( buf, "%m &CMana&w "			);
	}

	return buf;
}


/*
 * Il colore della vita scelto nella display_prompt.
 */
char *default_fprompt( CHAR_DATA *ch )
{
	static char buf[MIL];

	if ( IS_ADMIN(ch) )
	{
		strcpy( buf, "&c%k&g%n: "			);
		strcat( buf, "%l&w/&R%L Vita &w| "	);
		strcat( buf, "%v&w/&G%V Movi &w| "	);
		strcat( buf, "%m&w/&C%M Mana &w| "	);
		strcat( buf, "%o&w/&W%O Anima&w "	);
		strcat( buf, "(&R%c &wvs &R%C&w) "	);
	}
	else
	{
		strcpy( buf, "&c%k&g%n: "			);
		strcat( buf, "%l &RVita &w| "		);
		strcat( buf, "%v &GMovi &w| "		);
		strcat( buf, "%m &CMana&w "			);
		strcat( buf, "(&R%c &wvs &R%C&w) "	);
	}

	return buf;
}


/*
 * Ritorna la stringa relativa alla salute di un ch
 */
char *get_health_string( CHAR_DATA *ch )
{
	static char buf[MIL];
	int percent;

	percent = get_percent( ch->points[POINTS_LIFE], ch->points_max[POINTS_LIFE] );

	if		( percent >= 100 )		sprintf( buf, "in perfetta salute" );
	else if ( percent >=  90 )		sprintf( buf, "leggermente graffiat%c", gramm_ao(ch) );
	else if ( percent >=  80 )		sprintf( buf, "qualche contusione" );
	else if ( percent >=  70 )		sprintf( buf, "qualche taglio" );
	else if ( percent >=  60 )		sprintf( buf, "alcune ferite" );
	else if ( percent >=  50 )		sprintf( buf, "gravi lesioni" );
	else if ( percent >=  40 )		sprintf( buf, "sanguina abbondantemente" );
	else if ( percent >=  30 )		sprintf( buf, "copert%c di sangue", gramm_ao(ch) );
	else if ( percent >=  20 )		sprintf( buf, "perde le interiora" );
	else if ( percent >=  10 )		sprintf( buf, "quasi mort%c", gramm_ao(ch) );
	else							sprintf( buf, "morente" );

	return buf;
}


void display_prompt( DESCRIPTOR_DATA *d )
{
	CHAR_DATA	*ch = d->character;
	CHAR_DATA	*och = (d->original)  ?  d->original  :  d->character;
	CHAR_DATA	*victim;
	const char	*prompt;
	char		 buf[MSL];
	char		*pbuf = buf;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "display_prompt: ch è NULL." );
		return;
	}

	if ( IS_PG(ch) && ch->substate != SUB_NONE && VALID_STR(ch->pg->subprompt) )
	{
		prompt = ch->pg->subprompt;
	}
	else if ( IS_MOB(ch) || (!ch->fighting && !VALID_STR(ch->pg->prompt)) )
	{
		prompt = default_prompt( ch );
	}
	else if ( ch->fighting )
	{
		if ( !VALID_STR(ch->pg->fprompt) )
			prompt = default_fprompt( ch );
		else
			prompt = ch->pg->fprompt;
	}
	else
	{
		prompt = ch->pg->prompt;
	}

	if ( IS_PG(och) && HAS_BIT_PLR(och, PLAYER_COLOR) )
	{
		strcpy( pbuf, ANSI_RESET );
		pbuf += 4;
	}

	for ( ;  *prompt;  prompt++ )
	{
		/*
		 * '%' = prompt commands
		 * Note: foreground changes will revert background to 0 (black)
		 */
		if ( *prompt != '%' )
		{
			*(pbuf++) = *prompt;
			continue;
		}

		++prompt;

		if ( !VALID_STR(prompt) )
			break;

		if ( *prompt == *(prompt-1) )
		{
			*(pbuf++) = *prompt;
			continue;
		}

		switch ( *(prompt-1) )
		{
		  default:
			send_log( NULL, LOG_BUG, "display_prompt: carattere '%c' errato.", *(prompt-1) );
			break;

		  case '%':
			*pbuf = '\0';

			switch ( *prompt )
			{
			  case '%':
				*pbuf++ = '%';
				*pbuf = '\0';
				break;

			  /* Allineamento, solo per Admin */
			  case 'a':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", ch->alignment );
				break;

			  /* Affect */
			  case 'A':
				sprintf( pbuf, "%s%s%s",
					HAS_BIT(ch->affected_by, AFFECT_INVISIBLE)	?  "I"  :  "",
					HAS_BIT(ch->affected_by, AFFECT_HIDE)		?  "N"  :  "",
					HAS_BIT(ch->affected_by, AFFECT_SNEAK)		?  "S"  :  "" );
				break;

			  /* Stringa di salute del ch */
			  case 'c':
				if ( !ch->fighting || ch->fighting->who == NULL )
					strcpy( pbuf, "N/A" );
				else
					strcpy( pbuf, get_health_string(ch) );
				break;

			  /* Stringa di salute della vittima */
			  case 'C':
				if ( !ch->fighting || ch->fighting->who == NULL )
					strcpy( pbuf, "N/A" );
				else
				{
					victim = ch->fighting->who;
					strcpy( pbuf, get_health_string(victim) );
				}
				break;

			  /* Flag della room */
			  case 'F':
				if ( IS_ADMIN(och) )
					strcpy( pbuf, code_bit(NULL, ch->in_room->flags, CODE_ROOM) );
				break;

			  /* Oro */
			  case 'g':
				sprintf( pbuf, "%d", ch->gold );
				break;

			  case 'i':
				if ( (ch->pg && ch->pg->incognito != 0) || ch->mobinvis != 0 )
					sprintf( pbuf, "(Invis %d) ", IS_MOB(ch)  ?  ch->mobinvis  :  ch->pg->incognito );
				else if ( HAS_BIT(ch->affected_by, AFFECT_INVISIBLE) )
					sprintf( pbuf, "(Invis) " );
				break;

			  case 'I':
				if ( IS_MOB(ch) )
					sprintf( pbuf, "%d", HAS_BIT_ACT(ch, MOB_INCOGNITO)  ?  ch->mobinvis  :  0 );
				else
					sprintf( pbuf, "%d", ch->pg->incognito );
				break;

			  /* AFK */
			  case 'k':
				if ( IS_PG(ch) && (HAS_BIT_PLR(ch, PLAYER_AFK) || HAS_BIT_PLR(ch, PLAYER_AFK2)) )
				{
					if ( HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
						sprintf( pbuf, "<LDT> " );
					else
						sprintf( pbuf, "<AFK> " );
				}
				break;

			  /* Punti di vita */
			  case 'l':
				strcpy( pbuf, get_points_colored(ch, ch, POINTS_LIFE, TRUE) );
				break;

			  /* Punti di vita massimi */
			  case 'L':
			  	if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", ch->points_max[POINTS_LIFE] );
				break;

			  /* Punti di mana */
			  case 'm':
				strcpy( pbuf, get_points_colored(ch, ch, POINTS_MANA, TRUE) );
				break;

			  /* Punti di mana massimi */
			  case 'M':
			  	if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", ch->points_max[POINTS_MANA] );
				break;

			  /* Il proprio nome */
			  case 'n':
				if ( IS_MOB(ch) )
					strcpy( pbuf, ch->short_descr );
				else
					strcpy( pbuf, ch->name );
				pbuf[0] = toupper( pbuf[0] );
				break;

			  /* Nome di chi sta combattendo */
			  case 'N':
				if ( !ch->fighting || ch->fighting->who == NULL )
					strcpy( pbuf, "N/A" );
				else
				{
					victim = ch->fighting->who;

					if		( ch == victim )	strcpy ( pbuf, "Io" );
					else if ( IS_MOB(victim) )	strcpy ( pbuf, victim->short_descr );
					else						strcpy ( pbuf, victim->name );

					pbuf[0] = toupper( pbuf[0] );
				}
				break;

			  /* Punti di anima */
			  case 'o':
				strcpy( pbuf, get_points_colored(ch, ch, POINTS_SOUL, TRUE) );
				break;

			  /* Punti di anima massimi */
			  case 'O':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", ch->points_max[POINTS_SOUL] );
				break;

			  /* Vnum della stanza */
			  case 'r':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "<#%d>", ch->in_room->vnum );
				break;

			  case 'S':
				if		( ch->style == STYLE_BERSERK	)	strcpy( pbuf, "Berserk"		);
				else if ( ch->style == STYLE_AGGRESSIVE )	strcpy( pbuf, "Aggressiva"	);
				else if ( ch->style == STYLE_DEFENSIVE	)	strcpy( pbuf, "Difensiva"	);
				else if ( ch->style == STYLE_EVASIVE	)	strcpy( pbuf, "Evasiva"		);
				else										strcpy( pbuf, "Normale"		);
				break;

			  /* Tempo */
			  case 'T':
				if		( calendar.hour <  5 )	strcpy( pbuf, "notte"	 );
				else if ( calendar.hour <  6 )	strcpy( pbuf, "alba"	 );
				else if ( calendar.hour < 19 )	strcpy( pbuf, "giorno"	 );
				else if ( calendar.hour < 21 )	strcpy( pbuf, "tramonto" );
				else							strcpy( pbuf, "notte"	 );
				break;

			  /* Numero del descrittore */
			  case 'u':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", num_descriptors );
				break;

			  /* Picco massimo di numero di giocatori nel mud */
			  case 'U':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", mudstat.boot_max_players );
				break;

			  /* Punti di movimento */
			  case 'v':
				strcpy( pbuf, get_points_colored(ch, ch, POINTS_MOVE, TRUE) );
				break;

			  /* Punti di movimento massimi */
			  case 'V':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", ch->points_max[POINTS_MOVE] );
				break;

			  case 'x':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", ch->exp );
				break;

			  case 'X':
				if ( IS_ADMIN(och) )
					sprintf( pbuf, "%d", exp_level(ch, ch->level+1) );
				break;
			} /* chiude lo switch */

			pbuf += strlen( pbuf );
			break;
		} /* chiude lo switch */
	} /* chiude il for */

	*pbuf = '\0';
	send_to_char( ch, buf );
}


/*
 * Funzione di output di basso livello.
 */
bool flush_buffer( DESCRIPTOR_DATA *d, bool fPrompt )
{
	char		buf[MIL];

	/* If buffer has more than 4K inside, spit out 0.5K at a time */
	if ( !mud_down && d->outtop > 4096 )
	{
		memcpy( buf, d->outbuf, 512 );
		d->outtop -= 512;
		memmove( d->outbuf, d->outbuf + 512, d->outtop );

		if ( !write_to_descriptor(d, buf, 512) )
		{
			d->outtop = 0;
			return FALSE;
		}
		return TRUE;
	}

	/* Bust a prompt */
	if ( fPrompt && !mud_down && d->connected == CON_PLAYING )
	{
		CHAR_DATA *ch;

		ch = d->original  ?  d->original  :  d->character;
		if ( HAS_BIT_PLR(ch, PLAYER_BLANK) )
			write_to_buffer( d, "\r\n", 2 );

		/* Controllo sul conto alla rovescia del quit */
		if ( HAS_BIT_PLR(ch, PLAYER_PROMPT) && ch->pg->counter_quit == 0 )
			display_prompt( d );
		else
			send_to_char( ch, "&w" );	/* si assicura così di resettare i colori */

#ifdef T2_MXP
	/* reset MXP to default operation */
	if ( d->mxp )
		send_to_char( ch, ESC "[3z" );
#endif

		if ( HAS_BIT_PLR(ch, PLAYER_TELNETGA) )
			write_to_buffer( d, go_ahead_str, 0 );
	}

	/* Short-circuit if nothing to write */
	if ( d->outtop == 0 )
		return TRUE;

	/* Output OS-dipendente */
	if ( !write_to_descriptor(d, d->outbuf, d->outtop) )
	{
		d->outtop = 0;
		return FALSE;
	}
	else
	{
		d->outtop = 0;
		return TRUE;
	}
}


bool pager_output( DESCRIPTOR_DATA *d )
{
	CHAR_DATA	*ch;
	char		*endpoint;
	char		*newbuf;
	int			 pclines;
	int			 lines = 0;
	int			 size;
	bool		 nonstop = FALSE;

	if ( !d || d->pagecmd == -1 )
		return TRUE;

	ch = d->original  ?  d->original  :  d->character;
	lines = 0;
	pclines = ch->pg->pagerlen - 1;

	switch ( tolower(d->pagecmd) )
	{
	  default:
		break;

	  case 'n':
		nonstop = TRUE;
		break;

	  case 'e':
		flush_buffer( d, TRUE );
		DISPOSE( d->pagebuf );
		d->pagebuf = NULL;
		d->pagesize = 0;
		d->pagecmd = -1;
		return TRUE;
	}
	
	d->pagecmd = -1;
	
	if ( !nonstop )
	{
		for ( endpoint = d->pagebuf;  *endpoint && lines < pclines;  endpoint++ )
		{
			if ( *endpoint == '\n' )
				lines++;
		}

		if ( write_to_descriptor(d, d->pagebuf, endpoint - d->pagebuf) == FALSE )
			return FALSE;

		size = d->pagesize - ( endpoint - d->pagebuf );
		if ( size-1 > 0 )
		{
			CREATE( newbuf, char, size );
			strncpy( newbuf, endpoint, size );
			d->pagesize = size;
			DISPOSE( d->pagebuf );
			d->pagebuf = newbuf;
			if ( !d->character || (d->character && IS_PG(d->character) && HAS_BIT_PLR(d->character, PLAYER_COLOR)) )
			{
				if (write_to_descriptor(d, table_color[AT_LCYAN].ansi[CLR_FORE], 7) == FALSE)
					return FALSE;
			}
	
			if ( write_to_descriptor(d, "(C)ontinua, (N)on-stop, (E)sci: [C] ", 0) == FALSE )
				return FALSE;
		
			if ( !d->character || (d->character && IS_PG(d->character) && HAS_BIT_PLR(d->character, PLAYER_COLOR)) )
			{
				if ( write_to_descriptor(d, color_str(d->pagecolor, ch), 0) == FALSE )
					return FALSE;
			}
		}
		else
		{
			DISPOSE( d->pagebuf );
			d->pagebuf = NULL;
			d->pagesize = 0;
		}
	}
	else
	{
		write_to_buffer( d, d->pagebuf, 0 );
		DISPOSE( d->pagebuf );
		d->pagebuf = NULL;
		d->pagesize = 0;
	}
	
	return TRUE;
}


void game_loop( void )
{
	DESCRIPTOR_DATA    *d;
	struct timeval		last_time;
	char				cmdline[MIL];
	char			   *pcmdline = cmdline;

    signal( SIGPIPE, SIG_IGN );
    signal( SIGALRM, caught_alarm );

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while ( !mud_down )
	{
		accept_new( control  );

		/*
		 * Kick out descriptors with raised exceptions or have been idle, then check for input.
		 */
		for ( d = first_descriptor;  d;  d = d_next )
		{
			if ( d == d->next )
			{
				send_log( NULL, LOG_BUG, "game_loop: loop trovato e corretto." );
				d->next = NULL;
			}

			d_next = d->next;

			d->idle++;		/* make it so a descriptor can idle out */
			if ( FD_ISSET(d->descriptor, &exc_set) )
			{
				FD_CLR( d->descriptor, &in_set  );
				FD_CLR( d->descriptor, &out_set );
				if ( d->character && is_in_game(d) )
					save_player( d->character );

				d->outtop	= 0;
				close_socket( d, TRUE );
				continue;
			}
			else if ( (!d->character && d->idle > (3*60*PULSE_IN_SECOND))			/* login		3 minuti  */
			  || (d->connected != CON_PLAYING && d->idle > 10*60*PULSE_IN_SECOND)	/* non giocante 10 minuti  */
			  || d->idle > 180*60*PULSE_IN_SECOND )									/* oppure 3 ore */
			{
				write_to_descriptor( d, "\r\n\r\nTi lascio ai tuoi pensieri, ma torna presto!\r\n\r\n", 0 );
				d->outtop = 0;
				close_socket( d, TRUE );
				continue;
			}
			else
			{
				char	logline[MIL];

				d->fcommand	= FALSE;

				if ( FD_ISSET(d->descriptor, &in_set) )
				{
					d->idle = 0;
					if ( d->character )
					{
						d->character->timer = 0;
						if (d->original)
							d->original->timer = 0;
					}

					if ( !read_from_descriptor(d) )
					{
						FD_CLR( d->descriptor, &out_set );
						if ( d->character && is_in_game(d) )
							save_player( d->character );

						d->outtop = 0;
						close_socket( d, FALSE );
						continue;
					}
				}

				if ( d->character && d->character->wait > 0 )
				{
					--d->character->wait;
					continue;
				}

				read_from_buffer( d );

				if ( !VALID_STR(d->incomm) )
					continue;

				d->fcommand	= TRUE;
				stop_idling( d->character );

				strcpy( cmdline, d->incomm );
				d->incomm[0] = '\0';

				if ( d->character )
					set_cur_char( d->character );

				/* A meno che non stia editando vengono tolti gli spazi iniziali e finali */
				if ( !is_editing(d) )
				{
					int length;

					while ( isspace(cmdline[0]) )
						strcpy( cmdline, cmdline+1 );

					length = strlen( cmdline );
					while ( isspace(cmdline[--length]) )
						cmdline[length] = '\0';
				}

				/*
				 * Salva l'ultimo comando inviato
				 */
				if ( d->connected == CON_GET_PASSWORD
				  || d->connected == CON_NEW_GET_PASSWORD
				  || d->connected == CON_NEW_CONFIRM_PASSWORD )
				{
					strcpy( logline, "XXXXXXXX" );	/* impedisce di visualizzare la password */
				}
				else
				{
					strcpy( logline, cmdline );
				}
				sprintf( lastcmd_typed, "Data: %s  Conn: %s  Nome: %s  Cmd: %s",
					friendly_ctime(&current_time),
					(d) ? code_name(NULL, d->connected, CODE_CONNECTED) : "connection sconosciuta",
					(d && d->character) ? d->character->name : "sconosciuto",
					logline );
				save_lastcmd( );

				if ( d && d->pagebuf )
				{
					d->pagecmd = *pcmdline;
					continue;
				}

				switch ( d->connected )
				{
				  default:
					if ( want_animation(d) == FALSE )
						nanny( d, cmdline );
					break;

				  case CON_PLAYING:
					d->character->cmd_recurse = 0;

					/* Turn off afk bit when any command performed.
					 * Spostato da interpret poiché viene richiamato in varie parti
					 *	del codice indipendentemente dall'invio di qualcosa da parte del pg */
					if ( IS_PG(d->character) && HAS_BIT_PLR(d->character, PLAYER_AFK) )
					{
						send_command( d->character, "afk", CO );

						if ( !str_cmp(cmdline, "afk")  || !str_cmp(cmdline, "ldt")
						  || !str_cmp(cmdline, "afk2") || !str_cmp(cmdline, "ldt2")
						  || !strncasecmp(cmdline, "afk ", 4)  || !strncasecmp(cmdline, "ldt ", 4)
						  || !strncasecmp(cmdline, "afk2 ", 5) || !strncasecmp(cmdline, "ldt2 ", 5) )
							break;
					}

					interpret( d->character, cmdline );
					break;

				  case CON_EDITING:
					edit_buffer( d->character, cmdline );
					break;
				}
			} /* chiude l'else */

			if ( d == last_descriptor )
				break;
		} /* chiude il for */

		/* Autonomous game motion. */
		update_handler( );

		/* Output.*/
		for ( d = first_descriptor;  d;  d = d_next )
		{
			d_next = d->next;

			if ( (d->fcommand || d->pagesize > 0 || d->outtop > 0) && FD_ISSET(d->descriptor, &out_set) )
			{
				if ( d->pagebuf )
				{
					if ( !pager_output(d) )
					{
						if ( d->character && is_in_game(d) )
							save_player( d->character );

						d->outtop = 0;
						close_socket( d, FALSE );
					}
				}
				else if ( !flush_buffer(d, TRUE) )
				{
					if ( d->character && is_in_game(d) )
						save_player( d->character );

					d->outtop	= 0;
					close_socket( d, FALSE );
				}
			} /* chiude l'if */

			if ( d == last_descriptor )
				break;
		} /* chiude il for */

		/* Synchronize to a clock.
		 * Sleep( last_time + 1/PULSE_IN_SECOND - now ).
		 * Careful here of signed versus unsigned arithmetic. */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;

			gettimeofday( &now_time, NULL );
			usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec) + 1000000 / PULSE_IN_SECOND;
			secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
			while ( usecDelta < 0 )
			{
				usecDelta += 1000000;
				secDelta  -= 1;
			}

			while ( usecDelta >= 1000000 )
			{
				usecDelta -= 1000000;
				secDelta  += 1;
			}

			if ( secDelta > 0 || (secDelta == 0 && usecDelta > 0) )
			{
				struct timeval stall_time;

				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec  = secDelta;
				if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 && errno != EINTR )
				{
					perror( "game_loop: select: stall" );
					exit( EXIT_FAILURE );
				}
			}
		}

		gettimeofday( &last_time, NULL );
		current_time = (time_t) last_time.tv_sec;

	} /* chiude il while, main loop */

	fflush( stderr );		/* make sure strerr is flushed */
}


int main( int argc, char **argv )
{
	char	hostn[64];
	bool	fCopyover = FALSE;

	init_reboot_time( );

	/*
	 * Caricamento delle opzioni di config
	 */
	load_sysdata( );				/* Prima quelle su file */
	if ( argc > 1 )
	{
		get_sysdata_options( argv );	/* Poi quelle eventualmente passate */
		save_sysdata( );
	}

	if ( sysdata.copyover )
	{
		fCopyover = TRUE;
		control = sysdata.control;

		sysdata.copyover = FALSE;
		sysdata.control = -1;
		save_sysdata( );	/* Salva per evitare che dopo un crash nel riavvio vi siano ancora le info del copyover */
	}

	/*
	 * Avvio del Mud
	 */
	send_log( NULL, LOG_LOAD, "Avvio del database" );
	load_databases( fCopyover );

	send_log( NULL, LOG_LOAD, "Inizializzazione del socket" );
	if ( !fCopyover )
		control = init_socket( );

	/* I don't know how well this will work on an unnamed machine as I don't
	 *	have one handy, and the man pages are ever-so-helpful.. */
	if ( gethostname(hostn, sizeof(hostn)) < 0 )
	{
		send_log( NULL, LOG_WARNING, "main: gethostname unresolved" );
		strcpy( hostn, "unresolved" );
	}

	send_log( NULL, LOG_NORMAL, "%s pronto all'indirizzo %s sulla porta %d con PID %d.",
		strip_color(MUD_NAME), hostn, sysdata.mud_port, getpid() );

	/*
	 * Loop principale del Mud
	 */
	game_loop( );

	/*
	 * Chiusura del Mud
	 */
	close_databases( );	/* Chiusura di tutti i database per il check sui memory leak */

	close( control );

	send_log( NULL, LOG_NORMAL, "Normale conclusione del gioco." );
	exit( EXIT_SUCCESS );
	return 0;
}


void close_socket( DESCRIPTOR_DATA *dclose, bool force )
{
	CHAR_DATA		*ch;
	DESCRIPTOR_DATA *d;
	bool			 DoNotUnlink = FALSE;


	/* Flush outbuf */
	if ( !force && dclose->outtop > 0 )
		flush_buffer( dclose, FALSE );

	/* Say bye to whoever's snooping this descriptor */
	if ( dclose->snoop_by )
		write_to_buffer( dclose->snoop_by, "Chi stavi snoopando se ne è andato.\r\n", 0 );

	/* Stop snooping everyone else */
	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( d->snoop_by == dclose )
			d->snoop_by = NULL;
	}

	/* Check for switched people who go link-dead. */
	if ( dclose->original )
	{
		ch = dclose->character;
		if ( ch )
		{
			send_command( ch, "return", CO );
		}
		else
		{
			send_log( NULL, LOG_BUG, "close_socket: dclose->original senza personaggio %s",
				(dclose->original->name)  ?  dclose->original->name  :  "sconosciuto" );
			dclose->character = dclose->original;
			dclose->original = NULL;
		}

/*
		if ( (ch = dclose->character) != NULL )
		{
			send_command( ch, "return", CO );
		}
		else
		{
			send_log( NULL, LOG_BUG, "close_socket: dclose->original senza personaggio %s",
				(dclose->original->name)  ?  dclose->original->name  :  "sconosciuto" );
			dclose->character = dclose->original;
			dclose->original = NULL;
		}*/
	}

	ch = dclose->character;

	/* Sanity check :( */
	if ( !dclose->prev && dclose != first_descriptor )
	{
		DESCRIPTOR_DATA *dp;
		DESCRIPTOR_DATA *dn;

		send_log( NULL, LOG_BUG, "close_socket: %s desc:%p != first_desc:%p e desc->prev = NULL!",
			ch  ?  ch->name  :  d->host, (void*) dclose, (void*) first_descriptor );
		dp = NULL;

		for ( d = first_descriptor;  d;  d = dn )
		{
			dn = d->next;
			if ( d == dclose )
			{
				send_log( NULL, LOG_BUG, "close_socket: %s desc:%p trovato, prev deve essere:%p, correzione.",
					ch  ?  ch->name  :  d->host, (void*) dclose, (void*) dp );
				dclose->prev = dp;
				break;
			}
			dp = d;
		}

		if ( !dclose->prev )
		{
			send_log( NULL, LOG_BUG, "close_socket: %s desc:%p non può essere trovato!",
				ch  ?  ch->name  :  dclose->host, (void*) dclose );
			DoNotUnlink = TRUE;
		}
	}

	if ( !dclose->next && dclose != last_descriptor )
	{
		DESCRIPTOR_DATA *dp, *dn;
		send_log( NULL, LOG_BUG, "close_socket: %s desc:%p != last_desc:%p e desc->next = NULL!",
			ch  ?  ch->name  :  d->host, (void*) dclose, (void*) last_descriptor );
		dn = NULL;

		for ( d = last_descriptor;  d;  d = dp )
		{
			dp = d->prev;
			if ( d == dclose )
			{
				send_log( NULL, LOG_BUG, "close_socket: %s desc:%p found, il prossimo deve essere:%p, correzione.",
					ch  ?  ch->name  :  d->host, (void*) dclose, (void*) dn );
				dclose->next = dn;
				break;
			}
			dn = d;
		}

		if ( !dclose->next )
		{
			send_log( NULL, LOG_BUG, "close_socket: %s desc:%p non può essere trovato!",
				ch  ?  ch->name  :  dclose->host, (void*) dclose );
			DoNotUnlink = TRUE;
		}
	}

	if ( dclose->character )
	{
		if ( is_in_game(dclose) )
		{
			send_log( NULL, LOG_COMM, "%s ha perso il collegamento.", ch->name );
			act( AT_ACTION, "Un Gorgo Astrale si apre e tenta di risucchiare $n.", ch, NULL, NULL, TO_ROOM );
			ch->desc = NULL;
		}
		else
		{
			send_log( NULL, LOG_COMM, "Chiuso il collegamento a %s in %s.", ch->name, code_name(NULL, dclose->connected, CODE_CONNECTED) );
			/* clear descriptor pointer to get rid of bug message in log */
			dclose->character->desc = NULL;
			free_char( dclose->character );
		}
	}

	if ( !DoNotUnlink )
	{
		/* make sure loop doesn't get messed up */
		if ( d_next == dclose )
			d_next = d_next->next;

		UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
	}

#ifdef T2_MCCP
	if ( dclose->mccp->compressing )
		compressEnd( dclose, dclose->mccp->compressing );	/* Secondo me inutile perché ci pensa la free_desc() */
#endif

	if ( dclose->descriptor == maxdesc )
		--maxdesc;

	free_desc( dclose );
	--num_descriptors;
}


char *obj_short( OBJ_DATA *obj )
{
	static char buf[MSL];

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "obj_short: obj è NULL" );
		return "un oggetto sconosciuto";
	}

	if ( obj->count > 1 )
	{
		sprintf( buf, "%s (%d)", obj->short_descr, obj->count );
		return buf;
	}

	return obj->short_descr;
}


#define MORPHNAME(ch)	((ch->morph&&ch->morph->morph)      ?				\
							ch->morph->morph->short_desc    :				\
							IS_MOB(ch)  ?  ch->short_descr  :  ch->name)
#define NAME(ch)		(IS_MOB(ch)  ?  ch->short_descr  :  ch->name)


char *act_string( const char *format, CHAR_DATA *to, CHAR_DATA *ch, const void *arg1, const void *arg2, const int type )
{
	static const char *he_she [] = { "lui", "lei" };
	static const char *him_her[] = { "gli", "le"  };
	static char  buf[MSL];
	char		 fname[MIL];
	char		 temp[MSL];
	char		*point = buf;
	const char	*str   = format;
	const char	*i;
	CHAR_DATA	*vch  = (CHAR_DATA *) arg2;
	OBJ_DATA	*obj1 = (OBJ_DATA  *) arg1;
	OBJ_DATA	*obj2 = (OBJ_DATA  *) arg2;

	while ( VALID_STR(str) )
	{
		if ( *str != '$' )
		{
			*point++ = *str++;
			continue;
		}

		++str;
		if ( !arg2 && *str >= 'A' && *str <= 'Z' )
		{
			send_log( NULL, LOG_BUG, "act_string: arg2 (vittima) mancante per il codice %c: %s", *str, format );
			i = " <@@@> ";
		}
		else
		{
			switch ( *str )
			{
			  case '$':		i = "$";				break;

			  case 't':		i = (char *) arg1;		break;

			  case 'T': 	i = (char *) arg2;		break;

			  case 'n':
				if ( ch->morph == NULL )
					i = ( to  ?  PERS(ch, to)  :  NAME(ch) );
				else if ( type == STRING_ADMIN )
					i = ( to  ?  MORPHPERS(ch, to)  :  MORPHNAME(ch) );
				else
				{
					sprintf( temp, "(MORPH) %s", (to  ?  PERS(ch,to)  :  NAME(ch)) );
					i = temp;
				}
				break;

			  case 'N':
				if ( vch->morph == NULL )
				{
					i = ( to  ?  PERS(vch, to)  :  NAME(vch) );
				}
				else if ( type == STRING_ADMIN )
					i = ( to  ?  MORPHPERS(vch, to)  :  MORPHNAME(vch) );
				else
				{
					sprintf( temp, "(MORPH) %s", (to  ?  PERS(vch,to)  :  NAME(vch)) );
					i = temp;
				}
				break;

			  case 'e':
				if ( ch->sex != SEX_MALE && ch->sex != SEX_FEMALE )
				{
					send_log( NULL, LOG_BUG, "act_string: il giocatore %s ha il sesso %d!", ch->name, ch->sex );
					i = "lui";
				}
				else
				{
					i = he_she[ch->sex];
				}
				break;

			  case 'E':
				if ( vch->sex != SEX_MALE && vch->sex != SEX_FEMALE )
				{
					send_log( NULL, LOG_BUG, "act_string: il giocatore %s ha il sesso %d!", vch->name, vch->sex );
					i = "lei";
				}
				else
				{
					i = he_she[vch->sex];
				}
				break;

			  case 'm':
				if ( ch->sex != SEX_MALE && ch->sex != SEX_FEMALE )
				{
					send_log( NULL, LOG_BUG, "act_string: il giocatore %s ha il sesso %d!", ch->name, ch->sex );
					i = "gli";
				}
				else
					i = him_her[ch->sex];
				break;

			  case 'M':
				if ( vch->sex != SEX_MALE && vch->sex != SEX_FEMALE )
				{
					send_log( NULL, LOG_BUG, "act_string: il giocatore %s ha il sesso %d!", vch->name, vch->sex );
					i = "gli";
				}
				else
					i = him_her[vch->sex];
				break;

			  case 'x':
				if ( ch->sex != SEX_MALE && ch->sex != SEX_FEMALE )
				{
					send_log( NULL, LOG_BUG, "act_string: il giocatore %s ha il sesso %d!", ch->name, ch->sex );
					i = "o";
				}
				else
					i = ( ch->sex == SEX_FEMALE )  ?  "a"  :  "o";
				break;

			  case 'X':
				if ( vch->sex != SEX_MALE && vch->sex != SEX_FEMALE )
				{
					send_log( NULL, LOG_BUG, "act_string: il giocatore %s ha il sesso %d!", vch->name, vch->sex );
					i = "o";
				}
				else
					i = ( vch->sex == SEX_FEMALE )  ?  "a"  :  "o";
				break;

			  case 'p':
				if ( !to || can_see_obj(to, obj1, FALSE) )
				{
					if ( !VALID_STR(obj1->short_descr) )
						send_log( NULL, LOG_BUG, "passa qui con short nulla" );
					i = obj_short( obj1 );
				}
				else
					i = "qualcosa";
				break;

			  case 'P':
				if ( !to || can_see_obj(to, obj2, FALSE) )
					i = obj_short( obj2 );
				else
					i = "qualcosa";
				break;

			  case 'd':
				if ( !arg2 || ((char *) arg2)[0] == '\0' )
					i = "porta";
				else
				{
					one_argument( (char *) arg2, fname );
					i = fname;
				}
				break;

			  default:
				send_log( NULL, LOG_BUG, "act_string: codice errato %c: %s.", *str, format );
				i = " <@@@> ";
				break;
			} /* chiude lo switch */
		} /* chiude l'else */

		++str;

		while ( (*point = *i) != '\0' )
		{
			++point;
			++i;
		}
	} /* chiude il while */

	strcpy( point, "\r\n" );

	buf[0] = toupper( buf[0] );	/* (TT) da pensare se rivedere il DONT_UPPER o gestirlo in altra maniera */
	return buf;
}

#undef NAME


/*
 * La funzione act.
 */
void act( int ATcolor, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
	char	  *txt;
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;

	/*
	 * Discard null and zero-length messages.
	 */
	if ( !VALID_STR(format) )
		return;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "act: ch è NULL. (%s)", format );
		return;
	}

	if		( !ch->in_room	  )		to = NULL;
	else if ( type == TO_CHAR )		to = ch;
	else							to = ch->in_room->first_person;

	/*
	 * MOB_SECRETIVE handling
	 */
	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_SECRETIVE) && type != TO_CHAR )
		return;

	if ( type == TO_VICT )
	{
		if ( !vch )
		{
			send_log( NULL, LOG_BUG, "act: vch è NULL con TO_VICT. %s (%s)", ch->name, format );
			return;
		}

		if ( !vch->in_room )
		{
			send_log( NULL, LOG_BUG, "act: vch in una stanza NULL! %s -> %s (%s)", ch->name, vch->name, format );
			return;
		}

		to = vch;
/*		to = vch->in_room->first_person; */
	}

	if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
	{
		OBJ_DATA *to_obj;

		txt = act_string( format, NULL, ch, arg1, arg2, STRING_ADMIN );
		rprog_act_trigger( txt, to->in_room, ch, (OBJ_DATA *)arg1, (void *)arg2 );

		for ( to_obj = to->in_room->first_content;  to_obj;  to_obj = to_obj->next_content )
			oprog_act_trigger( txt, to_obj, ch, (OBJ_DATA *)arg1, (void *)arg2 );
	}

	/* Anyone feel like telling me the point of looping through the whole
	 *	room when we're only sending to one char anyways..? */
	for ( ;   to;   to = (type == TO_CHAR || type == TO_VICT)  ?  NULL  :  to->next_in_room )
	{
		if ( !to->desc && (IS_MOB(to) && !has_trigger(to->pIndexData->first_mudprog, MPTRIGGER_ACT)) )
			continue;

		if ( type == TO_CHAR && to != ch )
			continue;

		if ( type == TO_VICT && (to != vch || to == ch) )
			continue;

		if ( type == TO_ROOM && to == ch )
			continue;

		if ( type == TO_ADMIN && (to == ch || !IS_ADMIN(to)) )
			continue;

		if ( type == TO_NOVICT && (to == ch || to == vch) )
			continue;

		if ( IS_PG(ch) && !IS_ADMIN(to) && to->level < ch->pg->incognito )
			continue;

		if ( to->desc && is_in_olc(to->desc) )
			continue;

		/* Se non si è in piedi ed amministratori non si sente */
		if ( !is_awake(to) )
		{
			if ( IS_ADMIN(to) )
				send_to_char( to, "[ADM] Anche se stai dormendo vedi o senti:\r\n" );
			else
				continue;
		}

		if ( IS_ADMIN(to) )
			txt = act_string( format, to, ch, arg1, arg2, STRING_ADMIN );
		else
			txt = act_string( format, to, ch, arg1, arg2, STRING_NONE );

		if ( to->desc )
		{
			set_char_color( ATcolor, to );
			send_to_char( to, txt );
		}

		if ( MOBtrigger )
		{
			/* Note: use original string, not string with ANSI. */
			mprog_act_trigger( txt, to, ch, (OBJ_DATA *)arg1, (void *)arg2 );
		}
	}

    MOBtrigger = TRUE;

#ifdef T2_ARENA
	/* Invia quello che accade nell'arena */
	search_direction( ch, type, ATcolor );
#endif
}
