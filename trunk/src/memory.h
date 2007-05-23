#ifndef MEMORY_H
#define MEMORY_H


#ifdef DEBUG_MEMORY

void	*mymalloc		( size_t size );
void	 myfree			( void *ptr2 );
void	*mycalloc		( size_t nmemb, size_t size );
void	*myrealloc		( void *ptr2, size_t size );
char	*mystrdup		( const char *s );
char	*LLMultSelect	( long long int byte );
char	*LMultSelect	( long int byte );

#else

#define	mymalloc(x)		malloc(x)
#define	myfree(x)		free(x)
#define	mycalloc(x)		calloc(x)
#define	myrealloc(x)	realloc(x)
#define	mystrdup(x)		strdup(x)

#endif


#ifdef DEBUG_MEMORYLEAK
	void add_memorylist	( void *data, const char *file, const int line, const char *type );
	void del_memorylist	( void *data, const char *file, const int line );
	void show_leaks		( CHAR_DATA *ch, bool filewrite );
	char *real_str_dup	( const char *str, const char *file, const int line );
	#define str_dup(x)	real_str_dup( x, __FILE__, __LINE__ )
#else
	char *str_dup		( const char *str );
#endif


#define STR(x) #x

#ifdef DEBUG_MEMORYLEAK
	#define CREATE( result, type, number )								\
		do																\
		{																\
			REAL_CREATE( result, type, number );						\
			add_memorylist( result, __FILE__, __LINE__, STR(type) );	\
		} while (0)
	
	#define RECREATE( result, type, number )							\
		do																\
		{																\
			if ( result )												\
				del_memorylist( result, __FILE__, __LINE__ );			\
			REAL_RECREATE( result, type, number );						\
			add_memorylist( result, __FILE__, __LINE__, STR(type) );	\
		} while (0)	
	
	#define DISPOSE( point )											\
		do																\
		{																\
			REAL_DISPOSE( point );										\
			if ( point )												\
				del_memorylist( point, __FILE__, __LINE__ );			\
		} while (0)
#else
	#define CREATE( result, type, number )		REAL_CREATE( result, type, number )
	#define RECREATE( result, type, number )	REAL_RECREATE( result, type, number )
	#define DISPOSE( point )					REAL_DISPOSE( point )
#endif


/*
 * Macro per l'allocazione della memoria
 */

#define REAL_CREATE( result, type, number )												\
	do																					\
	{																					\
		if ( !((result) = (type *) mycalloc((number), sizeof(type))) )					\
		{																				\
			send_log( NULL, LOG_MEMORY, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ );	\
			abort( );																	\
		}																				\
	} while (0)

#define REAL_RECREATE( result, type, number )											\
	do																					\
	{																					\
		if ( !((result) = (type *) myrealloc((result), sizeof(type) * (number))) )		\
		{																				\
			send_log( NULL, LOG_MEMORY, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ );	\
			abort( );																	\
		}																				\
	} while (0)

#define REAL_DISPOSE( point ) 	\
	do							\
	{							\
		if ( (point) )			\
		{						\
			myfree( (point) );	\
			(point) = NULL;		\
		}						\
	} while (0)


/*
 * Variabili
 */
extern	unsigned long long int	totalalloched;
extern	unsigned long long int	totalfreed;
extern	unsigned long long int	initmemory;
extern	unsigned long long int	prevmemory;


#endif /* MEMORY_H */
