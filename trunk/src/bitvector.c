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

/****************************************************************************
 *                             Bitvector module                             *
 ****************************************************************************/

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>

#include "mud.h"
#include "fread.h"


/*
 * Controlla se a sinistra ci sono variabili vuote, quindi eliminabili
 */
#ifdef DEBUG_MEMORYLEAK
void check_free_bv( BITVECTOR *bits, const char *file, const int line )
#else
void check_free_bv( BITVECTOR *bits )
#endif
{
	int *base;
	int *newflags;
	int  newsize = 0;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
	}

	for ( base = bits->flags;  !(*base) && newsize < bits->size;  base++, newsize++ )
		;

	if ( !(*base) )
	{
		DISPOSE( bits->flags );
		bits->flags = NULL;
		bits->size = 0;
		return;
	}

	if ( base != bits->flags )
	{
		newsize = bits->size - ( base - bits->flags );
		/* Questo è il punto, presente più o meno in tutte le funzioni,
		 *	diciamo tutte quelle con CREATE che è in assoluto più pesante:
		 *	prima c'è un memset chiamato da CREATE, poi ricopia tutto il
		 *	bitvector */
#ifdef DEBUG_MEMORYLEAK
		REAL_CREATE( newflags, int, newsize );
		add_memorylist( newflags, file, line, "int" );
#else
		CREATE( newflags, int, newsize );
#endif
		memcpy( newflags, base, newsize*sizeof(int) );
		DISPOSE( bits->flags );
		bits->flags = newflags;
		bits->size = newsize;
	}
}


/*
 * Controlla se il bit (uno solo) si trova nella variabile BITVECTOR
 */
#ifdef DEBUG_MEMORYLEAK
bool real_HAS_BIT( BITVECTOR *bits, int bit, const char *file, const int line )
#else
bool HAS_BIT( BITVECTOR *bits, int bit )
#endif
{
	int *base;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
	}

	if ( bit < 0 )
	{
		send_log( NULL, LOG_BIT, "HAS_BIT: bit passato è negativo: %d", bit );
		return FALSE;
	}

	if ( (bit/(sizeof(int)*8))+1 > bits->size )
		return FALSE;

	base = bits->flags + ( bits->size-1)-(bit/(sizeof(int)*8) );

	if ( base < bits->flags )	/* non dovrebbe essere possibile */
		return FALSE;

	bit %= sizeof(int) * 8;

	if ( *base & (1 << bit) )
		return TRUE;

	return FALSE;
}


/*
 * Imposta il bit passato (uno solo) nella variabile di BITVECTOR
 */
#ifdef DEBUG_MEMORYLEAK
void real_SET_BIT( BITVECTOR **bits, int bit, const char *file, const int line )
#else
void real_SET_BIT( BITVECTOR **bits, int bit )
#endif
{
	int *base;

	if ( !(*bits) )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( *bits, file, line );
#else
		createbv( *bits );
#endif
	}


	if ( bit < 0 )
	{
		send_log( NULL, LOG_BIT, "SET_BIT: bit passato è negatvio: %d", bit );
		return;
	}

	if ( (*bits)->size == 0 )
	{
#ifdef DEBUG_MEMORYLEAK
		REAL_CREATE( (*bits)->flags, int, 1 );
		add_memorylist( (*bits)->flags, file, line, "int" );
#else
		CREATE( (*bits)->flags, int, 1 );
#endif
		(*bits)->size++;
	}

	base = (*bits)->flags + ( (*bits)->size-1 ) - ( bit/(sizeof(int)*8) );
	/* idea di base, va tutto a destra, posizionandosi sui bit meno significativi
	 *	dopodiché va a sinistra di quanti bit servono per memorizzare (bit), tutto
	 *	questo a salti di sizeof(int)*8 bit (i bit contenuti da un int) */

	if ( base < (*bits)->flags )
	{
		int *newflags;
		int  newsize;

		newsize = ( (bit)/(sizeof(int)*8) ) + 1;
#ifdef DEBUG_MEMORYLEAK
		REAL_CREATE( newflags, int, newsize );
		add_memorylist( newflags, file, line, "int" );
#else
		CREATE( newflags, int, newsize );
#endif
		memcpy( newflags+(newsize-(*bits)->size), (*bits)->flags, (*bits)->size*sizeof(int) );
		DISPOSE( (*bits)->flags );
		(*bits)->size = newsize;
		(*bits)->flags = newflags;
		base = (*bits)->flags;
	}

	bit %= sizeof(int) * 8;

	*base |= (1 << bit);
}


/*
 * Rimuove il bit (uno solo) dalla variabile di BITVECTOR
 */
#ifdef DEBUG_MEMORYLEAK
void real_REMOVE_BIT( BITVECTOR **bits, int bit, const char *file, const int line )
#else
void real_REMOVE_BIT( BITVECTOR **bits, int bit )
#endif
{
	int *base;

	if ( !(*bits) )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( *bits, file, line );
#else
		createbv( *bits );
#endif
	}


	if ( bit < 0 )
	{
		send_log( NULL, LOG_BIT, "REMOVE_BIT: bit passato è negativo: %d", bit );
		return;
	}

	if ( (bit/(sizeof(int)*8))+1 > (*bits)->size )
		return;

	base = (*bits)->flags + ( (*bits)->size-1 ) - ( bit/(sizeof(int)*8) );

	if ( base < (*bits)->flags )	/* non dovrebbe essere possibile */
		return;

	bit %= sizeof(int) * 8;

	*base &= ~(1 << bit);

#ifdef DEBUG_MEMORYLEAK
	check_free_bv( *bits, file, line );
#else
	check_free_bv( *bits );
#endif
}


/*
 * Inverte il bit (uno solo) della variabile BITVECTOR
 */
#ifdef DEBUG_MEMORYLEAK
void real_TOGGLE_BIT( BITVECTOR **bits, int bit, const char *file, const int line )
#else
void real_TOGGLE_BIT( BITVECTOR **bits, int bit )
#endif
{
	int *base;

	if ( !(*bits) )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( *bits, file, line );
#else
		createbv( *bits );
#endif
	}


	if ( bit < 0 )
	{
		send_log( NULL, LOG_BIT, "TOGGLE_BIT: bit passato è negativo: %d", bit );
		return;
	}

	if ( (*bits)->size == 0 )
	{
#ifdef DEBUG_MEMORYLEAK
		REAL_CREATE( (*bits)->flags, int, 1 );
		add_memorylist( (*bits)->flags, file, line, "int" );
#else
		CREATE( (*bits)->flags, int, 1 );
#endif
		(*bits)->size++;
	}

	base = (*bits)->flags + ( (*bits)->size-1 ) - ( bit/(sizeof(int)*8) );

	if ( base < (*bits)->flags )
	{
		int *newflags, newsize;
		newsize = ( (bit)/(sizeof(int)*8) ) + 1;
#ifdef DEBUG_MEMORYLEAK
		REAL_CREATE( newflags, int, newsize );
		add_memorylist( newflags, file, line, "int" );
#else
		CREATE( newflags, int, newsize );
#endif
		memcpy( newflags+(newsize-(*bits)->size), (*bits)->flags, (*bits)->size*sizeof(int) );
		DISPOSE( (*bits)->flags );
		(*bits)->size = newsize;
		(*bits)->flags = newflags;
		base = (*bits)->flags;
	}

	bit %= sizeof(int) * 8;

	*base ^= (1 << bit);

#ifdef DEBUG_MEMORYLEAK
	check_free_bv( *bits, file, line );
#else
	check_free_bv( *bits );
#endif
}


/*
 * Controlla se la prima variabile dei bitvector contiene tutte le flags della variabile passata per secondo
 */
#ifdef DEBUG_MEMORYLEAK
bool real_HAS_BITS( BITVECTOR *bits, BITVECTOR *bits2, const char *file, const int line )
#else
bool HAS_BITS( BITVECTOR *bits, BITVECTOR *bits2 )
#endif
{
	int	x;
	bool	fNull = FALSE;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
		fNull = TRUE;
	}

	if ( !bits2 )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits2, file, line );
#else
		createbv( bits2 );
#endif
		fNull = TRUE;
	}

	if ( fNull )
		return FALSE;

	/* Se uno o più bitvector è vuoto il check non è valido */
	if ( IS_EMPTY(bits) || IS_EMPTY(bits2) )
		return FALSE;

	/* Se già la size del secondo bitvector supera il primo questo ultimo non può contenere tutti i bit */
	if ( bits2->size > bits->size )
		return FALSE;

	for ( x = 0;  x < bits2->size*(sizeof(int)*8);  x++ )
	{
		if ( HAS_BIT(bits2, x) && !HAS_BIT(bits, x) )
			return FALSE;
	}

	return TRUE;
}


/*
 * Imposta i bit che si trovano nel secondo bitvector in quelli del primo
 */
#ifdef DEBUG_MEMORYLEAK
void real_SET_BITS( BITVECTOR **bits, BITVECTOR *bits2, const char *file, const int line )
#else
void real_SET_BITS( BITVECTOR **bits, BITVECTOR *bits2 )
#endif
{
	int x;

	if ( !(*bits) )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( *bits, file, line );
#else
		createbv( *bits );
#endif
	}


	if ( !bits2 )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits2, file, line );
#else
		createbv( bits2 );
#endif
		return;
	}

	for ( x = 0;  x < bits2->size*(sizeof(int)*8);  x++ )
	{
		if ( HAS_BIT(bits2, x) )
#ifdef DEBUG_MEMORYLEAK
			real_SET_BIT( &(*bits), x, file, line );
#else
			SET_BIT( *bits, x );
#endif
	}
}

/*
 * Rimuove i bit nel primo bitvector, ma solo quelli impostati nel secondo bitvector
 */
#ifdef DEBUG_MEMORYLEAK
void real_REMOVE_BITS( BITVECTOR **bits, BITVECTOR *bits2, const char *file, const int line )
#else
void real_REMOVE_BITS( BITVECTOR **bits, BITVECTOR *bits2 )
#endif
{
	int x;

	if ( !(*bits) )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( *bits, file, line );
#else
		createbv( *bits );
#endif
	}


	if ( !bits2 )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits2, file, line );
#else
		createbv( bits2 );
#endif
		return;
	}

	for ( x = 0;  x < bits2->size*(sizeof(int)*8);  x++ )
	{
		if ( HAS_BIT(bits2, x) )
#ifdef DEBUG_MEMORYLEAK
			real_REMOVE_BIT( &(*bits), x, file, line );
#else
			REMOVE_BIT( *bits, x );
#endif
	}
}


/*
 * Inverte i bit nel primo bitvector, ma solo quelli impostati nel secondo bitevector
 */
#ifdef DEBUG_MEMORYLEAK
void real_TOGGLE_BITS( BITVECTOR **bits, BITVECTOR *bits2, const char *file, const int line )
#else
void real_TOGGLE_BITS( BITVECTOR **bits, BITVECTOR *bits2 )
#endif
{
	int x;

	if ( !(*bits) )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( *bits, file, line );
#else
		createbv( *bits );
#endif
	}

	if ( !bits2 )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits2, file, line );
#else
		createbv( bits2 );
#endif
		return;
	}

	for ( x = 0;  x < bits2->size*(sizeof(int)*8);  x++ )
	{
		if ( HAS_BIT(bits2, x) )
#ifdef DEBUG_MEMORYLEAK
			real_TOGGLE_BIT( &(*bits), x, file, line );
#else
			TOGGLE_BIT( *bits, x );
#endif
	}
}


/*
 * Controlla se i due bitvector sono uguali
 */
#ifdef DEBUG_MEMORYLEAK
bool real_SAME_BITS( BITVECTOR *bits, BITVECTOR *bits2, const char *file, const int line )
#else
bool SAME_BITS( BITVECTOR *bits, BITVECTOR *bits2 )
#endif
{
	int		x;
	int		*base;
	int		*base2;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
	}

	if ( !bits2 )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits2, file, line );
#else
		createbv( bits2 );
#endif
	}

	if ( bits->size != bits2->size )
		return FALSE;

	base = bits->flags;
	base2 = bits2->flags;

	for ( x = bits->size;  x;  x-- )
	{
		if ( (*base) != (*base2) )
			return FALSE;

		base++;
		base2++;
	}

	return TRUE;
}


/*
 * Ritorna vero se il bitvector non ha nessuna flag, cioè è size == 0
 */
#ifdef DEBUG_MEMORYLEAK
bool real_IS_EMPTY( BITVECTOR *bits, const char *file, const int line )
#else
bool IS_EMPTY( BITVECTOR *bits )
#endif
{
	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
	}

	if ( bits->size == 0 )
		return TRUE;

	return FALSE;
}


#if 0
/* Le 4 funzioni di _MULTIBIT() sono inutilizzate per ora */

/*
 * Controlla se sono impostate uno o più bits alla variabile BITVECTOR
 * Come ultimo argomento DEVE avere un -1 ad indicare la fine della lista dei bit da controllare
 */
bool HAS_MULTIBIT( BITVECTOR **bits, int bit, ... )
{
	va_list args;
	int		argument;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, __FILE__, __LINE__ );
#else
		createbv( bits );
#endif
	}

	if ( bit == -1 )
	{
		send_log( NULL, LOG_BIT, "HAS_MULTIBIT: primo bit passato è -1" );
		return FALSE;
	}

	if ( !HAS_BIT(bits, bit) )
		return FALSE;

	va_start( args, bit );

	for ( argument = va_arg(args, int);  argument != -1;  argument = va_arg(args, int) )
	{
		if ( !HAS_BIT(bits, argument) )
		{
			va_end( args );
			return FALSE;
		}
	}

	va_end( args );

	return TRUE;
}


/*
 * Imposta uno o più bits alla variabile BITVECTOR
 * Come ultimo argomento DEVE avere un -1 ad indicare la fine della lista dei bit da settare
 */
void SET_MULTIBIT( BITVECTOR **bits, int bit, ... )
{
	va_list args;
	int argument;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, __FILE__, __LINE__ );
#else
		createbv( bits );
#endif
	}

	if ( bit == -1 )
	{
		send_log( NULL, LOG_BIT, "SET_MULTIBIT: primo bit passato è -1" );
		return;
	}

	SET_BIT( bits, bit );

	va_start( args, bit );

	for ( argument = va_arg(args, int);  argument != -1;  argument = va_arg(args, int) )
		SET_BIT( bits, argument );

	va_end( args );
}


/*
 * Rimuove uno o più bits alla variabile BITVECTOR
 * Come ultimo argomento DEVE avere un -1 ad indicare la fine della lista dei bit da rimuovere
 */
void REMOVE_MULTIBIT( BITVECTOR **bits, int bit, ... )
{
	va_list args;
	int		argument;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, __FILE__, __LINE__ );
#else
		createbv( bits );
#endif
	}

	if ( bit == -1 )
	{
		send_log( NULL, LOG_BIT, "REMOVE_MULTIBIT: primo bit passato è -1" );
		return;
	}

	REMOVE_BIT( bits, bit );

	va_start( args, bit );

	for ( argument = va_arg(args, int);  argument != -1;  argument = va_arg(args, int) )
		REMOVE_BIT( bits, argument );

	va_end( args );
}


/*
 * Inverte uno o più bits alla variabile BITVECTOR
 * Come ultimo argomento DEVE avere un -1 ad indicare la fine della lista dei bit da invertire
 */
void TOGGLE_MULTIBIT( BITVECTOR **bits, int bit, ... )
{
	va_list args;
	int		argument;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, __FILE__, __LINE__ );
#else
		createbv( bits );
#endif
	}

	if ( bit == -1 )
	{
		send_log( NULL, LOG_BIT, "TOGGLE_MULTIBIT: primo bit passato è -1" );
		return;
	}

	TOGGLE_BIT( bits, bit );

	va_start( args, bit );

	for ( argument = va_arg(args, int);  argument != -1;  argument = va_arg(args, int) )
		TOGGLE_BIT( bits, argument );

	va_end( args );
}
#endif


/*
 * Ritorna TRUE se il bit passato si trova nel bitvector int passato
 * Serve soprattutto per le flags di int dei values
 * Le altre tre funzioni per la gestione del bitvector int sono delle macro
 *	e sono definite naturalmente in bitvector.h
 */
bool HAS_BITINT( int bitvector, const int bit )
{
	if ( bit < 0 || bit >= sizeof(int)*8 )
	{
		send_log( NULL, LOG_BIT, "HAS_BITINT: bit passato errato: %d", bit );
		return FALSE;
	}

	if ( bitvector & (1 << bit) )
		return TRUE;

	return FALSE;
}


#ifdef DEBUG_MEMORYLEAK
BITVECTOR *real_meb( const int bit, const char *file, const int line )
#else
BITVECTOR *meb( const int bit )
#endif
{
	BITVECTOR *bits;

#ifdef DEBUG_MEMORYLEAK
	createbv( bits, file, line );
	real_SET_BIT( &bits, bit, file, line );
#else
	createbv( bits );
	SET_BIT( bits, bit );
#endif
	return bits;
}


BITVECTOR *multimeb( int bit, ... )
{
	BITVECTOR *bits;
	va_list	   args;
	int		   argument;

	if ( bit == -1 )
	{
		send_log( NULL, LOG_BIT, "multimeb: primo bit passato è -1" );
		return NULL;
	}

#ifdef DEBUG_MEMORYLEAK
	createbv( bits, __FILE__, __LINE__ );
#else
	createbv( bits );
#endif
	SET_BIT( bits, bit );

	va_start( args, bit );
	for ( argument = va_arg(args, int);  argument != -1;  argument = va_arg(args, int) )
		SET_BIT( bits, argument );
	va_end( args );

	return bits;
}


/* leggo dai meno ai più significativi, come mi sembra siano fatti gli ext_bv
 * anche se hanno una struttura completamente diversa e non so come farai a trovarti
 * questa procedura prende i vari numeri e li mette nel vettore partendo da desta, i meno significativi
 * lo puoi utilizzare per leggere un normale BV o anche un EXT_BV senza alcun problema
 * (CC) da togliere dopo
 */
#ifdef DEBUG_MEMORYLEAK
BITVECTOR *real_fread_old_bitvector( MUD_FILE *fp, const int type, const char *file, const int line )
#else
BITVECTOR *fread_old_bitvector( MUD_FILE *fp, const int type )
#endif
{
	BITVECTOR  *bits;
	long		offset;
	int			length = 1;
	int		   *base;
#ifdef DEBUG_LOAD
	int			x;
#endif
	char		c;

#ifdef DEBUG_MEMORYLEAK
	createbv( bits, file, line );
#else
	createbv( bits );
#endif
	
	offset = ftell( fp->file );
	for ( c = fgetc(fp->file);  c == '&' || isdigit(c);  c = fgetc(fp->file) )
	{
		if ( c == '&' )
			length++;
	}
	fseek( fp->file, offset, SEEK_SET );

	bits->size = length;

#ifdef DEBUG_MEMORYLEAK
	REAL_CREATE( bits->flags, int, bits->size );
	add_memorylist( bits->flags, file, line, "int" );
#else
	CREATE( bits->flags, int, bits->size );
#endif		
	
	base = bits->flags+length-1;
	
	for( ;  length;  length--, base-- )	/* parte dalla "destra" lsb e viene a "sinistra" msb */
	{
		*base = fread_number( fp );
		if ( (c = fgetc(fp->file)) != '&' )
			ungetc( c, fp->file );
	}

#ifdef DEBUG_LOAD
	/* Controllo sui limiti del bit ricavato */
	for ( x = code_max(type);  x < bits->size*(sizeof(int)*8);  x++ )
	{
		if ( HAS_BIT(bits, x) )
		{
			send_log( fp, LOG_FREAD, "fread_bitvector: bit di troppo: %d al tipo %s",
				x, table_code[type].code.name );
		}
	}
#endif

	return bits;
}


/*
 * Legge da file un bitvector formato numero
 */
#ifdef DEBUG_MEMORYLEAK
void real_fread_bitvector( MUD_FILE *fp, BITVECTOR *bits, const char *file, const int line )
#else
void fread_bitvector( MUD_FILE *fp, BITVECTOR *bits )
#endif
{
	int			*base;
	int			 sizebits;
	char		 check;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_bitvector: la struttura di fp è NULL" );
		return;
	}

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
	}

	if ( bits->flags )
		DISPOSE( bits->flags );
	bits->flags = NULL;

	bits->size = fread_number( fp );

	if ( bits->size > 0 )
	{
#ifdef DEBUG_MEMORYLEAK
		REAL_CREATE( bits->flags, int, bits->size );
		add_memorylist( bits->flags, file, line, "int" );
#else
		CREATE( bits->flags, int, bits->size );
#endif
	}
	else
	{
		destroybv( bits );
		return;
	}

	for ( base = bits->flags, sizebits = bits->size;  sizebits > 0;  sizebits--, base++ )
	{
		check = fgetc( fp->file );

		if ( check != '&' )
		{
			destroybv( bits );
			return;
		}

		*base = fread_number( fp );
	}

	return ;
}


/*
 * Scrive su file il bitvector in forma di numeri divisi da &
 */
#ifdef DEBUG_MEMORYLEAK
void real_fwrite_bitvector( MUD_FILE *fp, BITVECTOR *bits, const char *file, const int line )
#else
void fwrite_bitvector( MUD_FILE *fp, BITVECTOR *bits )
#endif
{
	char	*buf;
	char	 pbuf[11];
	int		*base;
	int		 size;

	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
	}

	base = bits->flags;

	/* Lo spazio create è spesso sovrabbondante */
	CREATE( buf, char, bits->size*11 + bits->size + 2 );

	sprintf( buf, "%d", bits->size );

	for ( size = bits->size;  size;  size--, base++ )
	{
		sprintf( pbuf, "&%d", *base );
		strcat( buf, pbuf );
	}

	fputs( buf, fp->file );
	fputc( '\n', fp->file );

	DISPOSE( buf );
}


/*
 * Converte un int bitvector ad un BITVECTOR
 * così da gestire le schifezze smauggose tipo gli affect
 * (FF) bisognerà fare decisamente qualcosa a riguardo
 */
#ifdef DEBUG_MEMORYLEAK
BITVECTOR *real_convert_bitvector( int old_bitvector, const char *file, const int line )
#else
BITVECTOR *convert_bitvector( int old_bitvector )
#endif
{
	BITVECTOR *bits;
	int		   x;

#ifdef DEBUG_MEMORYLEAK
	createbv( bits, file, line );
#else
	createbv( bits );
#endif

	for ( x = 0;  x < sizeof(int)*8;  x++ )
	{
		if ( old_bitvector & (1 << x) )
			SET_BIT( bits, x );
	}

	return bits;
}


/*
 * Ritorna l'int meno significativo di un BITVECTOR
 * (FF) anche questa è una funzione che serve tanto per far compilare il codice di robe
 *	old bitvector smauggose, sarà mano a mano da togliere in giro
 */
#ifdef DEBUG_MEMORYLEAK
int real_print_bitvector( BITVECTOR *bits, const char *file, const int line )
#else
int print_bitvector( BITVECTOR *bits )
#endif
{
	if ( !bits )
	{
#ifdef DEBUG_MEMORYLEAK
		createbv( bits, file, line );
#else
		createbv( bits );
#endif
		return 0;
	}

	if ( bits->flags )
		return *( bits->flags + bits->size-1 );
	else
		return 0;
}
