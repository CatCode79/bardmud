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


#ifdef T2_MXP


/*****************************************************************************\
 >						Modulo di gestione protocollo MXP					 <
\*****************************************************************************/


#include <stdarg.h>
#include <arpa/telnet.h>

#include "mud.h"
#include "mxp.h"


/* Snippet:
-------------------------------------------------------------------------------
 * Mxp stuff - Nick Gammon - 18 June 2001
-------------------------------------------------------------------------------
 */

/*const */unsigned char will_mxp_str 	[] = { IAC, WILL, TELOPT_MXP, '\0' };
		unsigned char start_mxp_str	[] = { IAC, SB,	  TELOPT_MXP, IAC, SE, '\0' };
/*const */unsigned char do_mxp_str   	[] = { IAC, DO,	  TELOPT_MXP, '\0' };	/* controllare se questi due possono essere const o no */
/*const */unsigned char dont_mxp_str 	[] = { IAC, DONT, TELOPT_MXP, '\0' };


/*
 * Ritorna vero se il pg può e vuole utilizzare l'mxp
 */
bool use_mxp( DESCRIPTOR_DATA *d )
{
	if ( !d )
		return FALSE;

	if ( !d->mxp )
		return FALSE;

	if ( d->character && IS_PG(d->character) && !HAS_BIT(d->character->pg->protocols, PROTOCOL_MXP) )
		return FALSE;

	return TRUE;
}


/*
 * Cambia i tag di mxp secondo il set di lingua utilizzato per i comandi.
 */
void mxp_toggle_language( DESCRIPTOR_DATA *d )
{
	/* Può capitare quando si sconnette prima di aver creato un pg o entrato in gioco */
	if ( !d->character )
	{
		send_log( NULL, LOG_WARNING, "mxp_toggle_language: d->character è NULL." );
		return;
	}

	/* Get tag, per oggetti a terra */
	if ( HAS_BIT_PLR(d->character, PLAYER_ITALIAN) )
	{
		write_to_buffer( d, MXPTAG("!EL get '<SEND href=\""
			"prendi &#39;&name;&#39;|"
			"esamina &#39;&name;&#39;|"
			"bevi &#39;&name;&#39;\" "
			"hint=\"Click sinistro per raccogliere questo oggetto|"
			"Prendi &desc;|"
			"Esamina &desc;|"
			"Bevi da &desc;"
			"\">' ATT=\"name desc\""), 0 );
	}
	else
	{
		write_to_buffer( d, MXPTAG("!EL get '<SEND href=\""
			"get &#39;&name;&#39;|"
			"examine &#39;&name;&#39;|"
			"drink &#39;&name;&#39;\" "
			"hint=\"Click sinistro per raccogliere questo oggetto|"
			"Prendi &desc;|"
			"Esamina &desc;|"
			"Bevi da &desc;"
			"\">' ATT=\"name desc\""), 0 );
 	}

	/* Drop tag, per gli oggetti in inventario */
	if ( HAS_BIT_PLR(d->character, PLAYER_ITALIAN) )
	{
		write_to_buffer( d, MXPTAG("!EL drp '<SEND href=\""
			"abbandona &#39;&name;&#39;|"
			"esamina &#39;&name;&#39;|"
			"guarda in &#39;&name;&#39;|"
			"indossa &#39;&name;&#39;|"
			"mangia &#39;&name;&#39;|"
			"bevi &#39;&name;&#39;\" "
			"hint=\"Click sinistro per abbandonare questo oggetto|"
			"Abbandona &desc;|"
			"Esamina &desc;|"
			"Guarda dentro &desc;|"
			"Indossa &desc;|"
			"Mangia &desc;|"
			"Bevi &desc;"
			"\">' ATT=\"name desc\""), 0 );
	}
	else
	{
		write_to_buffer( d, MXPTAG("!EL drp '<SEND href=\""
			"drop &#39;&name;&#39;|"
			"examine &#39;&name;&#39;|"
			"look in &#39;&name;&#39;|"
			"wear &#39;&name;&#39;|"
			"eat &#39;&name;&#39;|"
			"drink &#39;&name;&#39;\" "
			"hint=\"Click sinistro per abbandonare questo oggetto|"
			"Abbandona &desc;|"
			"Esamina &desc;|"
			"Guarda dentro &desc;|"
			"Indossa &desc;|"
			"Mangia &desc;|"
			"Bevi &desc;"
			"\">' ATT=\"name desc\""), 0 );
	}

	/* Lst tag, per le liste di oggetti nei negozi */
	if ( HAS_BIT_PLR(d->character, PLAYER_ITALIAN) )
	{
		write_to_buffer( d, MXPTAG("!EL lst '<SEND href=\""
			"compra &#39;&name;&#39;\" "
			"hint=\"Compra &desc;"
			"\">' ATT=\"name desc\""), 0 );
	}
	else
	{
		write_to_buffer( d, MXPTAG("!EL lst '<SEND href=\""
			"buy &#39;&name;&#39;\" "
			"hint=\"Compra &desc;"
			"\">' ATT=\"name desc\""), 0 );
	}

	/* Pg tag, per il comando who e tell */
	if ( HAS_BIT_PLR(d->character, PLAYER_ITALIAN) )
	{
		write_to_buffer( d, MXPTAG("!EL plr '<SEND href=\""
			"comunica &#39;&name;&#39;\" "
			"hint=\"Manda un messaggio a &name;\""
			" prompt>' ATT=\"name\""), 0 );
	}
	else
	{
		write_to_buffer( d, MXPTAG("!EL plr '<SEND href=\""
			"tell &#39;&name;&#39;\" "
			"hint=\"Manda un messaggio a &name;\""
			" prompt>' ATT=\"name\""), 0 );
	}
}


/*
 * Set up MXP
 * (BB) Vi è un baco nella implementazione dell'MXP,
 *	se qualche stringa possedesse il carattere '
 *	non farebbe funzionare a dovere il TAG.
 * Si risolve così: quando nella stringa c'è un carattere '''
 *	li si invia con il carattere "
 *	Altrimenti con quello '
 */
void turn_on_mxp( DESCRIPTOR_DATA *d )
{
	d->mxp = TRUE;	/* turn it on now */

	write_to_buffer( d, (char*) start_mxp_str, 0 );
	write_to_buffer( d, MXPMODE(6), 0 );	/* permanent secure mode */
/*	write_to_buffer( d, MXPTAG("!-- Set up MXP elements --"), 0 );*/

	/* Elemento tag dell'uscita */
	write_to_buffer( d, MXPTAG("!EL ex '<SEND>' FLAG=RoomExit"), 0 );

	/* Elemento tag per visualizzare l'help di una parola (FF) pe ora non serve, quando verranno rifatti gli help in maniera decente allora questo tag servirà ai riferimenti a fine help */
/*	write_to_buffer( d, MXPTAG( "!EL hlp '<SEND href=\""
								"help &command;\" "
								"hint=\"Cerca aiuto per la parola &desc;"
								"\">' ATT='command desc'"), 0 );*/

	/* (FF) Passa la dir di default ove scaricare i file di suono, bisogna fare attenzione che forse alcuni client non accettano questa sintassi dell'url di default, come è successo con l'msp */
/*	write_to_buffer( d, "<SOUND Off U=\"http://www.terra-secunda.net:5001/sounds\">" ); */

	mxp_toggle_language( d );
}


/*
 * Count number of mxp tags need converting
 *	ie. < becomes &lt;
 *		> becomes &gt;
 *		& becomes &amp;
 */
int count_mxp_tags( const bool bMXP, const char *txt, int length )
{
	char		c;
	const char *p;
	int			count;
	int			bInTag	  = FALSE;
	int			bInEntity = FALSE;

	for ( p = txt, count = 0;  length > 0;  p++, length-- )
	{
		c = *p;

		if ( bInTag )		/* in a tag, eg. <send> */
		{
			if ( !bMXP )
				count--;	/* not output if not MXP */
			if ( c == MXP_ENDc )
				bInTag = FALSE;
		} /* end of being inside a tag */
		else if ( bInEntity )	/* in a tag, eg. <send> */
		{
			if ( !bMXP )
				count--;		/* not output if not MXP */
			if ( c == ';' )
				bInEntity = FALSE;
		} /* end of being inside a tag */
		else switch ( c )
		{
		  case MXP_BEGc:
			bInTag = TRUE;
			if ( !bMXP )
				count--;		/* not output if not MXP */
			break;

		  case MXP_ENDc:		/* shouldn't get this case */
			if (!bMXP)
				count--;		/* not output if not MXP */
			break;

		  case MXP_AMPc:
			bInEntity = TRUE;
			if ( !bMXP )
				count--;		/* not output if not MXP */
			break;

		  default:
			if ( bMXP )
			{
				switch ( c )
				{
				  case '<':			/* < becomes &lt; */
				  case '>':			/* > becomes &gt; */
					count += 3;
					break;

				  case '&':
					count += 4;		/* & becomes &amp; */
					break;

				  case '"':			/* " becomes &quot; */
					count += 5;
					break;

				}
			}
		} /* end of switch on character */
	} /* end of counting special characters */

	return count;
}


void convert_mxp_tags( const bool bMXP, char *dest, const char *src, int length )
{
	char		c;
	const char *ps;
	char	   *pd;
	int			bInTag	  = FALSE;
	int			bInEntity = FALSE;

	for ( ps = src, pd = dest;  length > 0;  ps++, length-- )
	{
		c = *ps;
		if ( bInTag )			/* in a tag, eg. <send> */
		{
			if ( c == MXP_ENDc )
			{
				bInTag = FALSE;
				if ( bMXP )
					*pd++ = '>';
			}
			else if ( bMXP )
				*pd++ = c;		/* copy tag only in MXP mode */
		} /* end of being inside a tag */
		else if ( bInEntity )  /* in a tag, eg. <send> */
		{
			if ( bMXP )
				*pd++ = c;		/* copy tag only in MXP mode */
			if ( c == ';' )
				bInEntity = FALSE;
		} /* end of being inside a tag */
		else
		{
			switch ( c )
			{
			  case MXP_BEGc:
				bInTag = TRUE;
				if ( bMXP )
					*pd++ = '<';
				break;

			  case MXP_ENDc:	/* shouldn't get this case */
				if ( bMXP )
					*pd++ = '>';
				break;

			  case MXP_AMPc:
				bInEntity = TRUE;
				if ( bMXP )
					*pd++ = '&';
				break;

			  default:
				if ( bMXP )
				{
					switch ( c )
					{
					  case '<':
						memcpy( pd, "&lt;", 4 );
						pd += 4;
						break;

					  case '>':
						memcpy( pd, "&gt;", 4 );
						pd += 4;
						break;

					  case '&':
						memcpy( pd, "&amp;", 5 );
						pd += 5;
						break;

					  case '"':
						memcpy( pd, "&quot;", 6 );
						pd += 6;
						break;

					  default:
						*pd++ = c;
						break;
					}
				}
				else
					*pd++ = c;  /* not MXP - just copy character */
				break;
			} /* end of switch on character */
		}
	} /* end of converting special characters */
}


#endif	/* T2_MXP */
