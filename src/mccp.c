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


#ifdef T2_MCCP


/*****************************************************************************\
 >						Modulo di gestione protocollo MCCP					 <
\*****************************************************************************/


/* Snippet:
-------------------------------------------------------------------------------
 * Ported to SMAUG by Garil of DOTDII Mud
 * aka Jesse DeFer <dotd@dotd.com>  http://www.dotd.com
 *
 * revision 1: MCCP v1 support
 * revision 2: MCCP v2 support
 * revision 3: Correct MMCP v2 support
 * revision 4: clean up of write_to_descriptor() suggested by Noplex@CB
 *
 * See the web site below for more info.
 */

/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
-------------------------------------------------------------------------------
 */

#include <errno.h>
#include <arpa/telnet.h>
#include <unistd.h>
#include <zlib.h>

#include "mud.h"
#include "mccp.h"


/*
 * Variabili globali
 */
const unsigned char	compress_on_str	[] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
const unsigned char	compress2_on_str[] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };


/*
 * Variabili locali
 */
unsigned char enable_compress [] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, 0 };
unsigned char enable_compress2[] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, 0 };


void *zlib_alloc( void *opaque, unsigned int items, unsigned int size )
{
	return calloc( items, size );
}

void zlib_free( void *opaque, void *address )
{
	DISPOSE( address );
}

bool process_compressed( DESCRIPTOR_DATA *d )
{
	int iStart = 0;
	int	nBlock;
	int	nWrite;
	int	len;

	if ( !d->mccp )
	{
		send_log( NULL, LOG_BUG, "process_compressed: struttura mccp del descrittore è NULL" );
		return FALSE;
	}

	if ( !d->mccp->out_compress )
		return TRUE;

	/* Try to write out some data.. */
	len = d->mccp->out_compress->next_out - d->mccp->out_compress_buf;

	if ( len > 0 )
	{
		/* we have some data to write */
		for ( iStart = 0;  iStart < len;  iStart += nWrite )
        {
        	d->mccp->total_output += len;		/* Aumenta l'output inviato compresso del descrittore */
        	sysdata.total_output_mccp += len;

			nBlock = UMIN( (len - iStart), 4096 );
			if ( (nWrite = write(d->descriptor, d->mccp->out_compress_buf + iStart, nBlock)) < 0 )
			{
				if ( errno == EAGAIN || errno == ENOSR )
					break;

				return FALSE;
			}

			if ( !nWrite )
				break;
		}

		if ( iStart )
		{
			/* We wrote "iStart" bytes */
			if ( iStart < len )
				memmove(d->mccp->out_compress_buf, d->mccp->out_compress_buf+iStart, len - iStart);

			d->mccp->out_compress->next_out = d->mccp->out_compress_buf + len - iStart;
		}
	}

	return TRUE;
}


bool compressStart( DESCRIPTOR_DATA *d, unsigned char telopt )
{
	z_stream *s;

	if ( !d->mccp )
	{
		send_log( NULL, LOG_BUG, "compressStart: struttura mccp del descrittore è NULL" );
		return FALSE;
	}

	if ( d->mccp->out_compress )
		return TRUE;

	send_log( NULL, LOG_MCCP, "compressStart: inizio compressione di tipo %d per il descrittore %d",
		telopt,
		d->descriptor );

	CREATE( s, z_stream, 1 );
	CREATE( d->mccp->out_compress_buf, unsigned char, COMPRESS_BUF_SIZE );

	s->next_in = NULL;
	s->avail_in = 0;

	s->next_out = d->mccp->out_compress_buf;
	s->avail_out = COMPRESS_BUF_SIZE;

	s->zalloc = zlib_alloc;
	s->zfree  = zlib_free;
	s->opaque = NULL;

	if ( deflateInit(s, 9) != Z_OK )
	{
		DISPOSE( d->mccp->out_compress_buf );
		DISPOSE( s );
		return FALSE;
	}

	if		( telopt == TELOPT_COMPRESS )
		write_to_descriptor( d, (char *) enable_compress, 0 );
	else if ( telopt == TELOPT_COMPRESS2 )
		write_to_descriptor( d, (char *) enable_compress2, 0 );
	else
		send_log( NULL, LOG_BUG, "compressStart: TELOPT passato è errato." );

	d->mccp->can_compress = TRUE;
	d->mccp->compressing = telopt;
	d->mccp->out_compress = s;

	return TRUE;
}


bool compressEnd( DESCRIPTOR_DATA *d, unsigned char telopt )
{
	unsigned char dummy[1];

	if ( !d->mccp )
	{
		send_log( NULL, LOG_BUG, "compressEnd: struttura mccp del descrittore è NULL" );
		return FALSE;
	}

	if ( !d->mccp->out_compress )
		return TRUE;

	send_log( NULL, LOG_MCCP, "compressEnd: interruzione della compressione di tipo %d per il descrittore %d",
		telopt,
		d->descriptor );

	d->mccp->out_compress->avail_in = 0;
	d->mccp->out_compress->next_in = dummy;

	if ( deflate(d->mccp->out_compress, Z_FINISH) != Z_STREAM_END )
		return FALSE;

	if ( !process_compressed(d) )	/* try to send any residual data */
		return FALSE;

	deflateEnd( d->mccp->out_compress );
	DISPOSE( d->mccp->out_compress_buf );
	DISPOSE( d->mccp->out_compress );
	d->mccp->compressing = 0;

	return TRUE;
}


/*
 * Ritorna se il descrittore ha l'mccp attivo
 */
bool use_mccp( DESCRIPTOR_DATA *d )
{
	if ( !d )
		return FALSE;

	if ( !d->mccp )
	{
		send_log( NULL, LOG_BUG, "use_mccp: struttura mccp del descrittore è NULL" );
		return FALSE;
	}

	if ( !d->mccp->can_compress )
		return FALSE;

	if ( d->character && IS_PG(d->character) && !HAS_BIT(d->character->pg->protocols, PROTOCOL_MCCP) )
		return FALSE;

	return TRUE;
}


#endif	/* T2_MCCP */
