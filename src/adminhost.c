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


/****************************************************************************************\
 >									Modulo Admin Host									<
 > Permette all'Amministratore di decidere da quali ip potersi connettere in maniera	<
 >	tale da evitare che qualcuno cerchi di connettersi da un altro ip con il pg			<
 >	Amministratore.																		<
\****************************************************************************************/


#include <ctype.h>
#include <unistd.h>

#include "mud.h"
#include "channel.h"
#include "fread.h"


/*
 * Definizione di File
 */
#define	ADMHOST_FILE	SYSTEM_DIR "admin_host.dat"		/* Lista degli adminhost */
#define IPLIST_FILE		SYSTEM_DIR "admin_ip.list"		/* Lista degli ip di connessione degli admin */


/*
 * Struttura di un AdminHost
 */
typedef struct admin_host_data	ADMINHOST_DATA;

struct admin_host_data
{
	ADMINHOST_DATA	*next;
	ADMINHOST_DATA	*prev;
	char			*name;
	char			*host;
	bool			 prefix;
	bool			 suffix;
};


/*
 * Variabili globali
 */
static	ADMINHOST_DATA *first_adminhost	= NULL;
static	ADMINHOST_DATA *last_adminhost	= NULL;
		int				top_adminhost	= 0;


/*
 * Legge da file un Admin Host
 */
static void fread_admhost( MUD_FILE *fp )
{
	ADMINHOST_DATA	*host;
	char			*word;

	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fread_admhost: fp è NULL" );
		exit( EXIT_FAILURE );
	}

	CREATE( host, ADMINHOST_DATA, 1 );

	for ( ; ; )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_admhost: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !strcmp(word, "End"	) )		break;
		else if ( !strcmp(word, "Host"	) )		host->host =		fread_string( fp );
		else if ( !strcmp(word, "Name"	) )		host->name =		fread_string( fp );
		else if ( !strcmp(word, "Prefix") )		host->prefix =		fread_bool  ( fp );
		else if ( !strcmp(word, "Suffix") )		host->suffix =		fread_bool  ( fp );
		else
			send_log( fp, LOG_FREAD, "fread_admhost: nessuna etichetta per la parola %s", word );
	}

	LINK( host, first_adminhost, last_adminhost, next, prev );
	top_adminhost++;
}

/*
 * Carica tutti gli AdmHost
 */
void load_admhosts( void )
{
	fread_section( ADMHOST_FILE, "ADMINHOST", fread_admhost, FALSE );
}


/*
 * Scrive tutti gli admin host su file
 */
static void fwrite_admhost( void )
{
	ADMINHOST_DATA *temp;
	MUD_FILE	   *fp;

	if ( !first_adminhost )
	{
		unlink( ADMHOST_FILE );		/* Distrugge il file */
		return;
	}

	fp = mud_fopen( "", ADMHOST_FILE, "w", TRUE );
	if ( !fp )
		return;

	for ( temp = first_adminhost;  temp;  temp = temp->next )
	{
		fprintf( fp->file, "#ADMINHOST_DATA\n" );
		fprintf( fp->file, "Name    %s~\n", temp->name );
		fprintf( fp->file, "Host    %s~\n", temp->host );
		fprintf( fp->file, "Prefix  %s\n", fwrite_bool(temp->prefix) );
		fprintf( fp->file, "Suffix  %s\n", fwrite_bool(temp->suffix) );
		fprintf( fp->file, "End\n\n" );
	}
	fprintf( fp->file, "#END\n" );

	MUD_FCLOSE( fp );
}


/*
 * Libera una struttura di adminhost
 */
static void free_adminhost( ADMINHOST_DATA *host )
{
	if ( !host )
	{
		send_log( NULL, LOG_BUG, "free_adminhost: host passato è NULL" );
		return;
	}

	DISPOSE( host->name );
	DISPOSE( host->host );
	DISPOSE( host );
}

/*
 * Toglie dalla doppia lista e libera dalla memoria un adminhost
 */
static void remove_adminhost( ADMINHOST_DATA *host )
{
	if ( !host )
	{
		send_log( NULL, LOG_BUG, "remove_adminhost: host passato è NULL" );
		return;
	}

	UNLINK( host, first_adminhost, last_adminhost, next, prev );
	top_adminhost--;

	free_adminhost( host );
}

/*
 * Libera dalla memoria tutti gli adminhost
 */
void free_all_adminhosts( void )
{
	ADMINHOST_DATA *host;

	for ( host = first_adminhost;  host;  host = first_adminhost )
		remove_adminhost( host );

	if ( top_adminhost != 0 )
		send_log( NULL, LOG_BUG, "free_all_adminhost: top_adminhost non è 0 dopo aver liberato tutti gli adminhost: %d", top_adminhost );
}


/*
 * Ritorna la grandezza stimata di adminhost
 */
int get_sizeof_adminhost( void )
{
	return top_adminhost * sizeof( ADMINHOST_DATA );
}


/*
 * Controlla se il sito da cui si connette l'admin è uguale ad uno degli admin host
 */
bool check_adm_domain( const char *name, const char *host )
{
	ADMINHOST_DATA  *temp;
	bool			 found = FALSE;

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "check_adm_domain: name passato non è una stringa valido" );
		return TRUE;
	}

	if ( !VALID_STR(host) )
	{
		send_log( NULL, LOG_BUG, "check_adm_domain: host passato non è una stringa valida" );
		return TRUE;
	}

	for ( temp = first_adminhost;  temp;  temp = temp->next )
	{
		if ( !str_cmp(name, strlower(temp->name)) )
		{
			found = TRUE;	/* Per ora dà per scontato che abbia trovato il nome dell'admin tra quelli protetti */

			if		( temp->prefix && temp->suffix && strstr(temp->host, host) )	return TRUE;
			else if ( temp->prefix && !str_suffix(temp->host, host) )				return TRUE;
			else if ( temp->suffix && !str_prefix(temp->host, host) )				return TRUE;
			else if ( !str_cmp(temp->host, host) )									return TRUE;
		}
	}

	if ( found )
		return FALSE;

	return TRUE;
}


/*
 * Salva tutti gli ip utilizzati dagli admin per impostare poi al meglio gli admhost
 */
void add_ip_to_list( CHAR_DATA *ch )
{
	MUD_FILE *fp;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "add_ip_to_list: ch passato è NULL" );
		return;
	}

	fp = mud_fopen( "", IPLIST_FILE, "a", TRUE );
	if ( fp )
	{
		fprintf( fp->file, "%-12s %s\n", ch->name, ch->desc->host );
		MUD_FCLOSE( fp );
	}
}


/*
 * Aggiunge un host di amministratore
 */
DO_RET do_admhost( CHAR_DATA *ch, char *argument )
{
	ADMINHOST_DATA  *temp;
	ADMINHOST_DATA	*host = NULL;
	char			 type[MIL];
	char			 name[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_amhost: ch è NULL" );
		return;
	}

	argument =	one_argument( argument, type );
	argument =	one_argument( argument, name );

	set_char_color( AT_ADMIN, ch );

	if ( is_name(type, "aggiungi add") )
	{
		bool	prefix = FALSE;
		bool	suffix = FALSE;

		smash_tilde( name );
		smash_tilde( argument );

		if ( argument[0] == '*' )
		{
			prefix = TRUE;
			argument++;
		}

		if ( argument[strlen(argument)-1] == '*' )
		{
			suffix = TRUE;
			argument[strlen(argument)-1] = '\0';
		}

		name[0] = toupper( name[0] );
		strcpy( argument, strlower(argument) );

		for ( temp = first_adminhost;  temp;  temp = temp->next )
		{
			if ( !str_cmp(temp->name, name)
			  && !str_cmp(temp->host, argument) )
			{
				send_to_char( ch, "Vi è già uno uguale a questo.\r\n" );
				return;
			}
		}

		CREATE( host, ADMINHOST_DATA, 1 );
		host->name = str_dup( name );
		host->host = str_dup( argument );
		host->prefix = prefix;
		host->suffix = suffix;
		LINK( host, first_adminhost, last_adminhost, next, prev );

		send_to_char( ch, "Aggiunto un nuovo host di amministratore.\r\n" );
		send_to_char( ch, "Ricordati di salvare se vuoi che questo admhost rimanga attivo anche al prossimo riavvio.\r\n" );
		return;
	}
	else if ( !str_cmp(type, "cancella") || !str_cmp(type, "delete") )
	{
		if ( argument[0] == '*' )
			argument++;

		if ( argument[strlen(argument)-1] == '*' )
			argument[strlen(argument)-1] = '\0';

		for ( temp = first_adminhost;  temp;  temp = temp->next )
		{
			if ( !str_cmp(temp->name, name)
			  && !str_cmp(temp->host, argument) )
			{
				host = temp;
				break;
			}
		}

		if ( !host )
		{
			send_to_char( ch, "Non è stato trovato questo admin host.\r\n" );
			return;
		}

		remove_adminhost( host );

		send_to_char( ch, "Admin host cancellato.\r\n" );
		send_to_char( ch, "Ricordati di salvare se vuoi che questo admhost non ci sia anche al prossimo riavvio.\r\n" );
		return;
	}
	else if ( is_name(type, "salva save") )
	{
		/* Ok we have a new entry make sure it doesn't contain a ~ */
		fwrite_admhost( );
		send_to_char( ch, "Host di amministratori salvati.\r\n" );
		return;
	}
	else
	{
		send_to_char( ch, "&YSintassi&w: admhost aggiungi <nome> <host>\r\n" );
		send_to_char( ch, "          admhost cancella <nome> <host>\r\n" );
		send_to_char( ch, "          admhost salva\r\n\r\n"				  );

		if ( first_adminhost == NULL )
		{
			send_to_char( ch, "Nessun amministratore è protetto per ora.\r\n" );
			return;
		}

		send_to_char( ch, "-------------+-----------------\r\n" );
		set_char_color( AT_PLAIN, ch );
		for ( temp = first_adminhost;  temp;  temp = temp->next )
		{
			ch_printf( ch, "%-12s | %c%s%c\r\n",
				temp->name,
				(temp->prefix) ? '*' : ' ', temp->host, (temp->suffix) ? '*' : ' ' );
		}
	}
}
