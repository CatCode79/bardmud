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


/***************************************************************************\
 >                    Modulo Gestione bug in allocazione memoria            <
\***************************************************************************/


#include <unistd.h>

#include "mud.h"
#include "fread.h"

unsigned long long int	totalalloched;
unsigned long long int	totalfreed;
unsigned long long int	initmemory;
unsigned long long int	prevmemory;
unsigned long int		numalloc;
unsigned long int		numfree;


/*
 * Custom str_dup using create
 */
#ifdef DEBUG_MEMORYLEAK
char *real_str_dup( const char *str, const char *file, const int line )
#else
char *str_dup( const char *str )
#endif
{
	static char *ret;
	int			 len;

	if ( !str )
		return NULL;

	len = strlen( str ) + 1;

#ifdef DEBUG_MEMORYLEAK
	REAL_CREATE( ret, char, len );
	add_memorylist( ret, file, line, "char" );
#else
	CREATE( ret, char, len );
#endif
	strcpy( ret, str );
	return ret;
}


char *LLMultSelect( long long int byte )
{
	char *out;
	out = malloc(7);

	if		( byte < 1024 )				sprintf( out, "%lldb",  byte );
	else if ( byte < 1024*1024 )		sprintf( out, "%lldkb", byte/1024);
	else if ( byte < 1024*1024*1024 )	sprintf( out, "%lldmb", byte/(1024*1024) );
	else								sprintf( out, "%lldgb", byte/(1024*1024*1024) );

	return out;
}


char *LMultSelect( long int byte )
{
	char *out;
	out = malloc(7);

	if		( byte < 1024 )				sprintf( out, "%ldb",  byte );
	else if ( byte < 1024*1024 )		sprintf( out, "%ldkb", byte/1024 );
	else if ( byte < 1024*1024*1024 )	sprintf( out, "%ldmb", byte/(1024*1024) );
	else								sprintf( out, "%ldgb", byte/(1024*1024*1024) );

	return out;
}


void *mymalloc( size_t size )
{
	char *ptr;

	size += sizeof(size_t) + sizeof(char);
	totalalloched += size;
	numalloc++;
	ptr = malloc( size );
	*(size_t *)ptr = size;
	*(char *)((int)ptr + size-1) = 'F';
	ptr += sizeof( size_t );

	return (void *)ptr;
}


void myfree( void *ptr2 )
{
	char *ptr = (char *) ptr2;

	ptr -= sizeof( size_t );
	totalfreed += *(size_t *)ptr;
	numfree++;
	if ( *(char *)((int)ptr+(*(size_t *)ptr)-1) == 'F' )	/* Se qui arrivano delle stringhe non struduppate crasha, non è un baco di questa zona, bisogna cercare alla radice del problema */
		free( ptr );
	else
	{
		send_log( NULL, LOG_MEMORY, "myfree: memory allocation bug" );
		free( ptr );
	}
}


void *mycalloc( size_t nmemb, size_t size )
{
	char *ptr;

	totalalloched += size*nmemb;
	ptr = mymalloc( size*nmemb );
	memset( ptr, 0, size*nmemb );

	return (void *)ptr;
}


void *myrealloc( void *ptr2, size_t size )
{
	char *newptr;
	char *ptr = (char *)ptr2;

	size = size + sizeof(size_t) + sizeof(char);
	totalalloched += size;
	ptr -= sizeof( size_t );
	totalfreed += *(size_t *)ptr;
	numfree++;
	numalloc++;
	if ( *(char *)((int)ptr+(*(size_t *)ptr)-1) == 'F' )
		newptr = realloc(ptr, size);
	else
	{
		send_log( NULL, LOG_MEMORY, "myrealloc: memory allocation bug" );
		newptr = realloc( ptr, size );
	}
	*(size_t *)newptr = size;
	*(char *)(newptr + size -1) = 'F';
	newptr += sizeof( size_t );

	return newptr;
}


char *mystrdup( const char *s )
{
	char *ptr;

	ptr = mymalloc( strlen(s)*sizeof(char)+1 );
	strcpy( ptr, s );

	return ptr;
}


char *mem_stats( void )
{
	char	*s;
	char	*alloched;
	char	*talloched;
	char	*tfreed;
	char	*dmemory;

	s = malloc( 400 );

	alloched = LLMultSelect( totalalloched-totalfreed );
	talloched = LLMultSelect( totalalloched );
	tfreed = LLMultSelect( totalfreed );
	dmemory = LMultSelect( (numalloc-numfree)*(sizeof(size_t)+sizeof(char)*2) );

	strcpy ( s,			  "****Inizio Statistiche****\r\n" );
	sprintf( s+strlen(s), "Alloched: %lld (%s)\r\n", totalalloched-totalfreed, alloched );
	sprintf( s+strlen(s), "Total Alloched: %lld (%s)\r\n", totalalloched, talloched );
	sprintf( s+strlen(s), "Total freed: %lld (%s)\r\n", totalfreed, tfreed );
	sprintf( s+strlen(s), "Allocation Calls: %ld\r\n", numalloc );
	sprintf( s+strlen(s), "Free Calls: %ld\r\n", numfree );
	sprintf( s+strlen(s), "Debug Memory %ld (%s)\r\n", (numalloc-numfree)*(sizeof(size_t)+sizeof(char)*2), dmemory );
	sprintf( s+strlen(s), "*****Fine Statistiche*****" );

	free( alloched );
	free( talloched );
	free( tfreed );
	free( dmemory );

	return s;
}



#ifdef DEBUG_MEMORYLEAK
/*******************************************************************************
 * Sistema di tracking di allocazione di memoria per individuare i memory leak *
 *******************************************************************************/

typedef struct	sallocmem	ALLOC_DATA;

struct sallocmem
{
	ALLOC_DATA	*next;
	ALLOC_DATA	*prev;
	char		*file;
	int			 line;
	void		*data;
	char		*type;
};

ALLOC_DATA	*first_memory = NULL;
ALLOC_DATA	*last_memory = NULL;

void add_memorylist( void *data, const char *file, const int line, const char *type )
{
	ALLOC_DATA *newalloc;

	REAL_CREATE( newalloc, ALLOC_DATA, 1 );

	REAL_CREATE( newalloc->file, char, strlen(file)+1 );
	strcpy( newalloc->file, file );
	REAL_CREATE( newalloc->type, char, strlen(type)+1 );
	strcpy( newalloc->type, type );
	newalloc->line = line;
	newalloc->data = data;

	LINK( newalloc, first_memory, last_memory, next, prev );
}

void del_memorylist( void *data, const char *file, const int line )
{
	ALLOC_DATA *memorydata;

	/* inizio dal fondo della lista, è più efficente */

	for ( memorydata = last_memory;  memorydata;  memorydata = memorydata->prev )
	{
		if ( data == memorydata->data )
		{
			REAL_DISPOSE( memorydata->file );
			REAL_DISPOSE( memorydata->type );
			UNLINK( memorydata, first_memory, last_memory, next, prev );
			REAL_DISPOSE( memorydata );
			return;
		}
	}

	send_log( NULL, LOG_MEMORY, "Tentativo di liberare memoria mai allocata (%s) (%d)", file, line );
}


/* Struttura per la lista di visualizzazione leak */
typedef struct memorylist_data
{
	struct memorylist_data	*next;
	struct memorylist_data	*prev;
	char					*type;
	char					*file;
	long long				 size;
	int						 line;
	int						 num;
} MEMORYLIST_DATA;

MEMORYLIST_DATA	*first_memorylist = NULL;
MEMORYLIST_DATA	*last_memorylist  = NULL;

/* Funzione per la generazione della lista di visualizzazione leak */
bool check_exist( ALLOC_DATA *check )
{
	MEMORYLIST_DATA	*current;

	for ( current = first_memorylist;  current;  current = current->next )
	{
		if ( !strcmp(check->file, current->file) && current->line == check->line )
		{
			current->num++;
			current->size+= *(size_t *)(((char *)check->data)-sizeof(size_t));
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Questa funzione non può contenere add_memorylist troppo casino se no
 */
void show_leaks( CHAR_DATA *ch, bool filewrite )
{
	MEMORYLIST_DATA	*current;
	ALLOC_DATA		*sleak;
	MUD_FILE		*fp = NULL;
	int				 count;

	if ( !ch && !filewrite )
	{
		send_log( NULL, LOG_BUG, "show_leaks: ch passato è NULL (filewrite a FALSE)" );
		return;
	}

	if ( ch && filewrite )
	{
		send_log( NULL, LOG_BUG, "show_leaks: ch passato non è NULL (filewrite a TRUE)" );
		return;
	}

	last_memorylist	 = NULL;
	first_memorylist = NULL;

	for ( sleak = first_memory;  sleak;  sleak = sleak->next )
	{
		if ( !check_exist(sleak) )
		{
			REAL_CREATE( current, MEMORYLIST_DATA, 1 );
			current->num = 1;
			current->line = sleak->line;
			current->file = strdup( sleak->file );	/* strdup è voluto, niente memorylist */
			current->type = strdup( sleak->type );
			current->size = *(size_t *) ( ((char *)sleak->data)-sizeof(size_t) );
			LINK( current, first_memorylist, last_memorylist, next, prev );
		}
	}

	for ( current = first_memorylist;  current;  current = current->next )
	{
		MEMORYLIST_DATA	*tosort;

		for ( tosort = current->next;  tosort;  tosort = tosort->next )
		{
			if ( tosort->num > current->num )
				EXCHANGE( current, tosort, next, prev, MEMORYLIST_DATA );
		}
	}

	for ( ;  first_memorylist->prev;  first_memorylist = first_memorylist->prev )
		;
	for ( ;  last_memorylist->prev;  last_memorylist = last_memorylist->prev )
		;

	if ( filewrite )
	{
		char filename[MIL];

		sprintf( filename, "memoryleaks.%d", getpid() );
		fp = mud_fopen( SYSTEM_DIR, filename, "w", TRUE );
		if ( !fp )
			return;
		fputs( "[NUM] [QTY] [BYTES] [FILE]           [LINE]   [TYPE]\r\n", fp->file );
	}
	else
		send_to_char( ch, "[NUM] [QTY] [BYTES] [FILE]           [LINE]   [TYPE]\r\n" );

	count = 0;
	for ( current = first_memorylist;  current;  current = current->next )
	{
		count++;

		if ( filewrite )
		{
			fprintf( fp->file, "%-6d%-6d%-8lld%-17s%-9d%s\r\n",
				count,
				current->num,
				current->size,
				current->file,
				current->line,
				current->type );
		}
		else
		{
			ch_printf( ch, "%-6d%-6d%-8lld%-17s%-9d%s\r\n",
				count,
				current->num,
				current->size,
				current->file,
				current->line,
				current->type );
		}
	}

	if ( filewrite )
		fprintf( fp->file, "Trovati in totale %d memory leak.. Buon divertimento!\r\n", count );
	else
		ch_printf( ch, "Trovati in totale %d memory leak.. Buon divertimento!\r\n", count );

	for ( current = first_memorylist;  current;  current = first_memorylist )
	{
		UNLINK( current, first_memorylist, last_memorylist, next, prev );
		free( current->type );	/* voluto, no DISPOSE! */
		free( current->file );
		REAL_DISPOSE( current );
	}

	if ( filewrite )
		fputs( mem_stats(), fp->file );
	else
		send_to_char( ch, mem_stats() );
}


/*
 * Comando che visualizza i memory leak
 */
DO_RET do_showleaks( CHAR_DATA *ch, char *argument )
{
	show_leaks( ch, FALSE );
}


#endif	/* DEBUG_MEMORYLEAK */
