#ifdef T2_MCCP

#ifndef MCCP_H
#define MCCP_H


/*****************************************************************************\
 >			Header del modulo protocollo di compressione MCCP				 <
\*****************************************************************************/

#include "zlib.h"


/*
 * Definizioni
 */
#define TELOPT_COMPRESS		85
#define TELOPT_COMPRESS2	86

#define COMPRESS_BUF_SIZE	MSL


/*
 * Struttura di dati mccp
 * E' stata creata principalmente per evitare di inserire la zlib.h in comm.h
 */
struct mccp_data
{
	bool			 can_compress;		/* Indica se il client del descrittore può supportare l'MCCP */
	unsigned char	 compressing;
	z_stream		*out_compress;
	unsigned char	*out_compress_buf;
	int				 total_output;		/* Output totale compresso inviato al descrittore*/
};


/*
 * Variabili
 */
extern /*const */unsigned char	compress_on_str	[];
extern /*const */unsigned char	compress2_on_str[];


/*
 * Funzioni
 */
bool	compressStart	( DESCRIPTOR_DATA *d, unsigned char telopt );
bool	compressEnd		( DESCRIPTOR_DATA *d, unsigned char telopt );
bool	use_mccp		( DESCRIPTOR_DATA *d );


#endif	/* MCCP_H */

#endif	/* T2_MCCP */
