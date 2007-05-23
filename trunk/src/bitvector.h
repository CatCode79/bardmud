#ifndef BITVECTOR_H
#define BITVECTOR_H


/*
 * Struttura di un bitvector
 */
struct bitvector_data
{
	int	   *flags;		/* Le flag del bitvector */
	int		size;		/* Di quanti interi è formato il bitvector */
/*	bool	toggle;*/	/* (GG) Se TRUE tutte le flag del bitvector sono da considerarsi invertite (serve per il TOGGLE_MULTIBIT essenzialmente, forse anche per il TOGGLE_BITS) */
};


#ifdef DEBUG_MEMORYLEAK
#define CLEAR_BITS(x)						\
	{										\
		destroybv( (x) )					\
		createbv( (x), __FILE__, __LINE__ )	\
	}
#else
#define CLEAR_BITS(x)						\
	{										\
		destroybv( (x) )					\
		createbv( (x) )						\
	}
#endif

#ifdef DEBUG_MEMORYLEAK
#define createbv( x, file, line )						\
	{													\
		REAL_CREATE( (x), BITVECTOR, 1 );				\
		add_memorylist( (x), file, line, "BITVECTOR" );	\
		(x)->flags = NULL;								\
		(x)->size = 0;									\
	}
#else
#define createbv( x )									\
	{													\
		CREATE( (x), BITVECTOR, 1 );					\
		(x)->flags = NULL;								\
		(x)->size = 0;									\
	}
#endif

#define destroybv( x )					\
	{									\
		if ( (x) )						\
		{								\
			if ( (x)->flags )			\
				DISPOSE( (x)->flags );	\
			(x)->flags = NULL;			\
			(x)->size = 0;				\
			DISPOSE( (x) );				\
		}								\
	}


/*
 * Queste tre macro sono gestite così giusto per comodità,
 *	ovvero per non dover passare la variabile bitvector tramite indirizzo
 */
/* Imposta i bit in un bitvector int */
#define SET_BITINT( bitvector, bit )														\
	do																						\
	{																						\
		if ( (bit) < 0 || (bit) >= sizeof(int)*8 )											\
		{																					\
			send_log( NULL, LOG_BIT, "SET_BITINT: bit errato: %d (file: %s line: %d)",		\
				(bit), __FILE__, __LINE__ );												\
		}																					\
		else																				\
		{																					\
			check_type_int( (bitvector) );													\
			check_type_int( (bit) );														\
			(bitvector) |= 1 << (bit);														\
		}																					\
	} while (0)

/* Rimuove un bit da un bitvector int */
#define REMOVE_BITINT( bitvector, bit )														\
	do																						\
	{																						\
		if ( (bit) < 0 || (bit) >= sizeof(int)*8 )											\
		{																					\
			send_log( NULL, LOG_BIT, "REMOVE_BITINT: bit errato: %d (file: %s line: %d)",	\
				(bit), __FILE__, __LINE__ );												\
		}																					\
		else																				\
		{																					\
			check_type_int( (bitvector) );													\
			check_type_int( (bit) );														\
			(bitvector) &= ~(1 << (bit));													\
		}																					\
	} while (0)

/* Inverte un bit di un bitvetor int */
#define TOGGLE_BITINT( bitvector, bit )														\
	do																						\
	{																						\
		if ( (bit) < 0 || (bit) >= sizeof(int)*8 )											\
		{																					\
			send_log( NULL, LOG_BIT, "REMOVE_BITINT: bit errato: %d (file: %s line: %d)",	\
				(bit), __FILE__, __LINE__ );												\
		}																					\
		else																				\
		{																					\
			check_type_int( (bitvector) );													\
			check_type_int( (bit) );														\
			(bitvector) ^= (1 << (bit));													\
		}																					\
	} while(0)


#ifdef DEBUG_MEMORYLEAK

#define HAS_BIT( bits, bit )				real_HAS_BIT			( bits, bit, __FILE__, __LINE__ )
#define	SET_BIT( bits, bit ) 				real_SET_BIT			( &bits, bit, __FILE__, __LINE__ )
#define	REMOVE_BIT( bits, bit )				real_REMOVE_BIT			( &bits, bit, __FILE__, __LINE__ )
#define	TOGGLE_BIT( bits, bit )				real_TOGGLE_BIT			( &bits, bit, __FILE__, __LINE__ )
#define HAS_BITS( bits, bits2 )				real_HAS_BITS			( bits, bits2, __FILE__, __LINE__ )
#define	SET_BITS( bits, bits2 ) 			real_SET_BITS			( &bits, bits2, __FILE__, __LINE__ )
#define	REMOVE_BITS( bits, bits2 )			real_REMOVE_BITS		( &bits, bits2, __FILE__, __LINE__ )
#define	TOGGLE_BITS( bits, bits2 )			real_TOGGLE_BITS		( &bits, bits2, __FILE__, __LINE__ )
#define SAME_BITS( bits, bits2 )			real_SAME_BITS			( bits, bits2, __FILE__, __LINE__ )
#define IS_EMPTY( bits )					real_IS_EMPTY			( bits, __FILE__, __LINE__ )
#define fread_old_bitvector( fp, type )		real_fread_old_bitvector( fp, type, __FILE__, __LINE__ )
/*#define	fread_bitvector( fp, bits )		real_fread_bitvector	( fp, bits, __FILE__, __LINE__ ) */
#define meb( bit )							real_meb				( bit, __FILE__, __LINE__ )
/*#define fwrite_bitvector( fp, bits )		real_fwrite_bitvector	( fp, bits, __FILE__, __LINE__ )*/
#define convert_bitvector( old_bitvector )	real_convert_bitvector	( old_bitvector, __FILE__, __LINE__ )
#define print_bitvector( bits )				real_print_bitvector	( bits, __FILE__, __LINE__ )

bool		real_HAS_BIT			( BITVECTOR *bits, int bit, const char *file, const int line );
void		real_SET_BIT			( BITVECTOR **bits, int bit, const char *file, const int line );
void		real_REMOVE_BIT			( BITVECTOR **bits, int bit, const char *file, const int line );
void		real_TOGGLE_BIT			( BITVECTOR **bits, int bit, const char *file, const int line );
bool		real_HAS_BITS			( BITVECTOR *bits, BITVECTOR *bits2, const char *file, const int line );
void		real_SET_BITS			( BITVECTOR **bits, BITVECTOR *bits2, const char *file, const int line );
void		real_REMOVE_BITS		( BITVECTOR **bits, BITVECTOR *bits2, const char *file, const int line );
void		real_TOGGLE_BITS		( BITVECTOR **bits, BITVECTOR *bits2, const char *file, const int line );
bool		real_SAME_BITS			( BITVECTOR *bits, BITVECTOR *bits2, const char *file, const int line );
bool		real_IS_EMPTY			( BITVECTOR *bits, const char *file, const int line );
BITVECTOR  *real_fread_old_bitvector( MUD_FILE *fp, const int type, const char *file, const int line );
/*void		real_fread_bitvector	( MUD_FILE *fp, BITVECTOR *bits, const char *file, const int line );*/
BITVECTOR  *real_meb				( int bit, const char *file, const int line );
/*void		real_fwrite_bitvector	( MUD_FILE *fp, BITVECTOR *bits, const char *file, const int line ); */
BITVECTOR  *real_convert_bitvector	( int old_bitvector, const char *file, const int line );
int			real_print_bitvector	( BITVECTOR *bits, const char *file, const int line );

#else

#define	SET_BIT( bits, bit ) 			real_SET_BIT	( &bits, bit )
#define	REMOVE_BIT( bits, bit )			real_REMOVE_BIT	( &bits, bit )
#define	TOGGLE_BIT( bits, bit )			real_TOGGLE_BIT	( &bits, bit )
#define	SET_BITS( bits, bits2 ) 		real_SET_BITS	( &bits, bits2 )
#define	REMOVE_BITS( bits, bits2 )		real_REMOVE_BITS( &bits, bits2 )
#define	TOGGLE_BITS( bits, bits2 )		real_TOGGLE_BITS( &bits, bits2 )

bool		HAS_BIT				( BITVECTOR *bits, int bit );
void		real_SET_BIT		( BITVECTOR **bits, int bit );
void		real_REMOVE_BIT		( BITVECTOR **bits, int bit );
void		real_TOGGLE_BIT		( BITVECTOR **bits, int bit );
bool		HAS_BITS			( BITVECTOR *bits, BITVECTOR *bits2 );
void		real_SET_BITS		( BITVECTOR **bits, BITVECTOR *bits2 );
void		real_REMOVE_BITS	( BITVECTOR **bits, BITVECTOR *bits2 );
void		real_TOGGLE_BITS	( BITVECTOR **bits, BITVECTOR *bits2 );
bool		SAME_BITS			( BITVECTOR *bits, BITVECTOR *bits2 );
bool		IS_EMPTY			( BITVECTOR *bits );
BITVECTOR  *meb					( int bit );
BITVECTOR  *fread_old_bitvector	( MUD_FILE *fp, const int type );
/*void		fread_bitvector		( MUD_FILE *fp, BITVECTOR *bits );*/
BITVECTOR  *convert_bitvector	( int old_bitvector );
int			print_bitvector		( BITVECTOR *bits );

#endif


#if 0
bool	   HAS_MULTIBIT		( BITVECTOR *bits, int bit, ... );
void	   SET_MULTIBIT		( BITVECTOR **bits, int bit, ... );
void	   REMOVE_MULTIBIT	( BITVECTOR **bits, int bit, ... );
void	   TOGGLE_MULTIBIT	( BITVECTOR **bits, int bit, ... );
#endif
bool	   HAS_BITINT		( int bitvector, const int bit );
BITVECTOR *multimeb			( int bit, ... );


#endif	/* BITVECTOR_H */
