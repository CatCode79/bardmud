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
 >			Modulo di lettura dei file con sezioni e file di area			<
\****************************************************************************/

#include <ctype.h>
#include <errno.h>

#include "mud.h"
#include "db.h"
#include "fread.h"


/*
 * Funzione fopen creata appositamente per gestire meglio la tipologia di file del BARD: MUD_FILE
 *
 * bool msg indica se visualizzare o meno il messaggio di errore apertura file
 *	gli sì dà un valore FALSE per i file aperti molto spesso oppure che non è
 *	obbligatorio che esistano
 */
MUD_FILE *mud_fopen( const char *dirname, const char *filename, const char *modes, bool msg )
{
	MUD_FILE	*fp;
	char		 buf[MIL];

	CREATE( fp, MUD_FILE, 1 );

	/* Ha bisogno di filename non nullo perché lo utilizza per stamparlo nel messaggio di bug
	 *	in caso di fallimento apertura mud_fopen */
	if ( !VALID_STR(filename) )
	{
		send_log( NULL, LOG_BUG, "mud_fopen: filename passato non valido. dirname: %s modes:%s",
			(dirname)  ?  dirname  :  "?",
			(modes)    ?  modes    :  "?" );
		filename = "?";
	}

	sprintf( buf, "%s%s", dirname, filename );
	fp->path = str_dup( buf );

	fp->file = fopen( fp->path, modes );
	if ( !fp->file )
	{
		if ( msg )
		{
			send_log( NULL, LOG_FP, "mud_fopen: errore nell'apertura del file %s in modalità %s", fp->path, modes );
			send_log( NULL, LOG_FP, "mud_fopen: strerror: %s", strerror(errno) );
		}

		MUD_FCLOSE( fp );
		return NULL;
	}

	return fp;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *
 *					Funzioni di lettura/scrittura generiche (fread/fwrite o print)				   *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game. Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far. These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 */

/*
 * Read a letter from a file.
 */
char fread_letter( MUD_FILE *fp )
{
	char		c;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_letter: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return '\0';
	}

	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_letter: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return '\0';
		}
		c = getc( fp->file );
	} while ( isspace(c) );

	return c;
}


/*
 * Legge un numero int da un file.
 */
int fread_number( MUD_FILE *fp )
{
	int		number;
	bool	sign;
	char	c;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_number: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return 0;
	}

	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_number: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return 0;
		}
		c = getc( fp->file );
	} while( isspace(c) );

	number = 0;

	sign   = FALSE;
	if		( c == '+' )
		c = getc( fp->file );
	else if ( c == '-' )
	{
		sign = TRUE;
		c = getc( fp->file );
	}

	if ( !isdigit(c) )
	{
		send_log( fp, LOG_FREAD, "fread_number: il carattere non è numerico." );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return 0;
	}

	while ( isdigit(c) )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_number: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return number;
		}
		number = number * 10 + c - '0';
		c	  = getc( fp->file );
	}

	if ( sign )
		number = 0 - number;

	if ( c != ' ' )
		ungetc( c, fp->file );

	return number;
}


/*
 * Legge un numero di VNUM da un file
 */
int fread_vnum( MUD_FILE *fp )
{
	VNUM vnum;

	vnum = fread_number( fp );

	if ( vnum < 0 || vnum > MAX_VNUM_AREA )
	{
		send_log( fp, LOG_FREAD, "fread_vnum: vnum acquisito errato: %d", vnum );
		if ( fBootDb )
			exit( EXIT_FAILURE );
	}

	return vnum;
}

/*
 * Legge un numero di VNUM da un file
 * Ritorna il prototipo di oggetto relativo al vnum
 */
OBJ_PROTO_DATA *fread_vnum_obj( MUD_FILE *fp )
{
	OBJ_PROTO_DATA *obj;
	VNUM			vnum;

	vnum = fread_vnum( fp );
	if ( vnum == 0 )
		return NULL;

	obj = get_obj_proto( fp, vnum );

	return obj;
}

/*
 * Legge un numero di VNUM da un file
 * Ritorna il prototipo del mob relativo al vnum
 */
MOB_PROTO_DATA *fread_vnum_mob( MUD_FILE *fp )
{
	MOB_PROTO_DATA *mob;
	VNUM			vnum;

	vnum = fread_vnum( fp );
	if ( vnum == 0 )
		return NULL;

	mob = get_mob_index( fp, vnum );

	return mob;
}

/*
 * Legge un numero di VNUM da un file
 * Ritorna il prototipo della stanza relativo al vnum
 */
ROOM_DATA *fread_vnum_room( MUD_FILE *fp )
{
	ROOM_DATA	*room;
	VNUM		 vnum;

	vnum = fread_vnum( fp );
	if ( vnum == 0 )
		return NULL;

	room = get_room_index( fp, vnum );

	return room;
}


/*
 * Legge un numero positivo da un file
 * E' incluso lo zero
 */
int fread_positive( MUD_FILE *fp )
{
	int num;

	num = fread_number( fp );
	if ( num < 0 )
	{
		send_log( fp, LOG_FREAD, "fread_positive: numero acquisito negativo: %d", num );
		if ( fBootDb )
			exit( EXIT_FAILURE );
	}

	return num;
}


/*
 * Legge un numero negativo da un file
 * E' incluso lo zero
 */
int fread_negative( MUD_FILE *fp )
{
	int num;

	num = fread_number( fp );
	if ( num > 0 )
	{
		send_log( fp, LOG_FREAD, "fread_positive: numero acquisito positivo: %d", num );
		if ( fBootDb )
			exit( EXIT_FAILURE );
	}

	return num;
}


/*
 * Legge un numero da un file.
 */
long fread_long( MUD_FILE *fp )
{
	long		number;
	bool		sign;
	char		c;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_number: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return 0;
	}

	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_number: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return 0;
		}
		c = getc( fp->file );
	}
	while( isspace(c) );

	number = 0;

	sign   = FALSE;
	if		( c == '+' )
		c = getc( fp->file );
	else if ( c == '-' )
	{
		sign = TRUE;
		c = getc( fp->file );
	}

	if ( !isdigit(c) )
	{
		send_log( fp, LOG_FREAD, "fread_number: il carattere non è numerico." );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return 0;
	}

	while ( isdigit(c) )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_number: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return number;
		}
		number = number * 10 + c - '0';
		c	  = getc( fp->file );
	}

	if ( sign )
		number = 0 - number;

	if ( c != ' ' )
		ungetc( c, fp->file );

	return number;
}


/*
 * Legge un numero float da un file.
 */
float fread_float( MUD_FILE *fp )
{
	char	   *fstr;

	fstr = fread_word( fp );

	/* (FF) bisognerebbe anche controllare che non ve ne sia più di uno */
	if ( !strchr(fstr, ',') && !strchr(fstr, '.') )
	{
		send_log( fp, LOG_FREAD, "fread_float: numero floating senza virgola o punto" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
	}

	return atof( fstr );
}


/*
 * Legge una stringa, fino alla tilde, da un file
 * (FF) bool line, TRUE significa che la stringa da leggere non deve andare a capo
 */
#ifdef DEBUG_MEMORYLEAK
char *real_fread_string( MUD_FILE *fp, const char *file, const int line )
#else
char *fread_string( MUD_FILE *fp )
#endif
{
	char	   *plast;
	char		buf[MSL];
	char		c;
	int			ln;

	plast = buf;
	buf[0] = '\0';
	ln = 0;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_string: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
#ifdef DEBUG_MEMORYLEAK
		return real_str_dup( "", file, line );
#else	
		return str_dup( "" );
#endif
		}

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_string: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
#ifdef DEBUG_MEMORYLEAK
		return real_str_dup( "", file, line );
#else	
		return str_dup( "" );
#endif
		}
		c = getc( fp->file );
	} while ( isspace(c) );

	if ( (*plast++ = c) == '~' )
#ifdef DEBUG_MEMORYLEAK
		return real_str_dup( "", file, line );
#else	
		return str_dup( "" );
#endif

	for ( ; ; )
	{
		if ( ln >= (MSL-1) )
		{
			send_log( fp, LOG_FREAD, "fread_string: stringa più lunga di %d.", MSL-1 );
			*plast = '\0';
#ifdef DEBUG_MEMORYLEAK
			return real_str_dup( buf, file, line );
#else	
			return str_dup( buf );
#endif
		}

		switch ( *plast = getc(fp->file) )
		{
		  default:
			plast++;
			ln++;
			break;

		  case EOF:
			send_log( fp, LOG_FREAD, "fread_string: EOF." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			*plast = '\0';
			return str_dup( buf );
			break;

		  case '\n':
			*plast++ = '\r';  ln++;
			*plast++ = '\n';  ln++;
			break;

		  case '\r':
			break;

		  case '~':
			*plast = '\0';
#ifdef DEBUG_MEMORYLEAK
			return real_str_dup( buf, file, line );
#else	
			return str_dup( buf );
#endif
		}
	} /* chiude il for */
}


/*
 * Legge fino a fine riga
 * Viene più utilizzata per leggere i commenti che per ritornare valori
 */
void fread_to_eol( MUD_FILE *fp )
{
	char		c;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_to_eol: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_to_eol: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;
		}
		c = getc( fp->file );
	} while ( c != '\r' && c != '\n' );

	do
	{
		c = getc( fp->file );
	} while ( c == '\r' || c == '\n' );

	ungetc( c, fp->file );
}


/*
 * Read to end of line into static buffer
 */
char *fread_line( MUD_FILE *fp )
{
	static char	line[MSL];
	char	   *pline;
	char		c;
	int			ln;

	pline = line;
	line[0] = '\0';
	ln = 0;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_line: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		strcpy( line, "" );
		return line;
	}

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_line: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			strcpy( line, "" );
			return line;
		}
		c = getc( fp->file );
	} while ( isspace(c) );

	ungetc( c, fp->file );

	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_line: EOF incotrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			*pline = '\0';
			return line;
		}
		c = getc( fp->file );
		*pline++ = c; ln++;

		if ( ln >= (MSL - 1) )
		{
			send_log( fp, LOG_FREAD, "fread_line: linea più lunga di %d.", MSL-1 );
			break;
		}
	} while ( c != '\r' && c != '\n' );

	do
	{
		c = getc( fp->file );
	} while ( c == '\r' || c == '\n' );

	ungetc( c, fp->file );
	*pline = '\0';
	return line;
}


/*
 * Legge una parola (alfa e/o numerica)
 * La inserisce in un buffer statico, quindi se si vuole
 *	mantenere il valore bisogna str_duparlo.
 * Se invece è un valore temporaneo ad uso della funzione non serve.
 */
char *fread_word( MUD_FILE *fp )
{
	static char	word[MIL];
	char	   *pword;
	char		cEnd;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_word: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		word[0] = '\0';
		return word;
	}

	do
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_word: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			word[0] = '\0';
			return word;
		}
		cEnd = getc( fp->file );
	} while ( isspace(cEnd) );

	if ( cEnd == '\'' || cEnd == '"' )
		pword   = word;
	else
	{
		word[0] = cEnd;
		pword   = word+1;
		cEnd	= ' ';
	}

	for ( ;  pword < word+MIL;  pword++ )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_word: EOF incontrato nella lettura." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			*pword = '\0';
			return word;
		}

		*pword = getc( fp->file );

		if ( (cEnd == ' ')  ?  isspace(*pword)  :  *pword == cEnd )
		{
			if ( cEnd == ' ' )
				ungetc( *pword, fp->file );

			*pword = '\0';
			return word;
		}
	}

	send_log( fp, LOG_FREAD, "fread_word: word troppo lunga." );
	if ( fBootDb )
		exit( EXIT_FAILURE );
	return NULL;
}


/*
 * Scrive True o False dipende dal valore booleano passato
 */
char *fwrite_bool( bool bit )
{
	if ( bit == TRUE )
		return "True";

	return "False";
}


/*
 * Legge la parola e la ritorna come booleano.
 */
bool fread_bool( MUD_FILE *fp )
{
	char	   *word;

	if ( !fp || !fp->file )
	{
		send_log( NULL, LOG_FREAD, "fread_bool: la struttura di fp è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return FALSE;
	}

	word = fread_word( fp );
	if		( !str_cmp(word, "False") )		return FALSE;
	else if ( !str_cmp(word, "True" ) )		return TRUE;

	/* Se giunge qui significa che c'è un errore */
	send_log( fp, LOG_FREAD, "fread_bool: Valore errato nella lettura." );
	if ( fBootDb )
		exit( EXIT_FAILURE );
	return FALSE;
}


char *print_bool( const bool value )
{
	if ( value == TRUE )
		return "True";
	else
		return "False";

	send_log( NULL, LOG_FWRITE, "print_bool: valore non booleano: %d.", value );
}


/*
 * Legge una struttura di sinonimo da un file.
 */
SYNO_DATA *fread_synonym( MUD_FILE *fp, int type )
{
	SYNO_DATA	*synonym;

	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fread_synonym: fp passato è NULL" );
		return NULL;
	}

	if ( type < 0 || type >= MAX_CODE )
	{
		send_log( NULL, LOG_BUG, "fread_synonym: type passato errato: %d", type );
		return NULL;
	}

	CREATE( synonym, SYNO_DATA, 1 );

	synonym->cat = code_code( fp, fread_word(fp), type );
	synonym->syn = fread_string( fp );

	return synonym;
}

/*
 * Scrive su file una struttura synonym.
 */
void fwrite_synonym( MUD_FILE *fp, SYNO_DATA *synonym, int type )
{
	if ( !synonym )
	{
		send_log( fp, LOG_FWRITE, "fwrite_synonym: synonym è NULL, type: %s",
			table_code[type].code.name );
		return;
	}

	fprintf( fp->file, "    %s", code_str(fp, synonym->cat, type) );
	fprintf( fp->file, " %s~\n", synonym->syn );
}

/*
 * Libera dalla memoria un sinonimo
 */
void free_synonym( SYNO_DATA *synonym )
{
	if ( !synonym )
	{
		send_log( NULL, LOG_BUG, "free_synonym: synonym passato è NULL" );
		return;
	}

	DISPOSE( synonym->syn );
	DISPOSE( synonym );
}


/*
 * Funzione di lettura delle liste, cioè dei file *.lst.
 */
void fread_list( const char *list_file, const char *section, FREAD_FUN *freadfun, bool unique )
{
	MUD_FILE *fp;

	fp = mud_fopen( "", list_file, "r", TRUE );
	if ( !fp )
	{
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	for ( ; ; )
	{
		char   *filename;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_: fine del file prematuro nella lettura" );
			if ( fBootDb )
				exit( EXIT_FAILURE );
		}

		filename = fread_word( fp );

		if ( filename[0] == '*' )	/* Salta i commenti */
		{
			fread_to_eol( fp );
			continue;
		}

		/* Se arrivato alla fine esce */
		if ( !str_cmp(filename, "End") )
			break;

		/* Legge l'area se si tratta di una sezione file di area */
		if ( !str_cmp(section, "AREA_FILE") && freadfun == NULL )
		{
			MUD_FILE *fp_area;

			fp_area = mud_fopen( "", filename, "r", TRUE );
			if ( !fp_area )
			{
				if ( fBootDb )
					exit( EXIT_FAILURE );
				return;
			}

			load_area_file( NULL, fp_area );
		}
		else
			fread_section( filename, section, freadfun, unique );	/* Legge la sezione relativa al file ricavato dalla lista */
	}

	MUD_FCLOSE( fp );
}


/*
 * Funzione di lettura generica delle sezioni.
 * unique se TRUE indica che la sezione deve essere letta solo una volta in tutto il file
 */
void fread_section( const char *section_file, const char *section, FREAD_FUN *freadfun, bool unique )
{
	MUD_FILE	*fp;
	int			 count = 0;		/* conta quante occorrenze di section sono state trovate */

	fp = mud_fopen( "", section_file, "r", TRUE );
	if ( !fp )
	{
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	for ( ; ; )
	{
		char   *word;
		char	letter;

		letter = fread_letter( fp );

		if ( letter == EOF )
		{
			send_log( fp, LOG_FREAD, "fread_section: fine del file prematura durante la lettura" );
			if ( fBootDb )
				exit( EXIT_FAILURE );
		}

		/* Salta a fine riga se trova il carattere di commento */
		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		/* Errore se non trova l'etichetta di caricamento sezione. */
		if ( letter != '#' )
		{
			send_log( fp, LOG_FREAD, "fread_section: # non trovato" );
			break;
		}

		word = fread_word( fp );
		if ( !strcmp(word, section) )
		{
			freadfun( fp );
			count++;
		}
		else if ( !strcmp(word, "END") )	/* Ricordarsi di inserire un a capo dopo l'etichetta #END nei file altrimenti non lo legge correttamente */
		{
			break;
		}
		else
		{
			send_log( fp, LOG_FREAD, "fread_section: sezione errata %s", word );
			if ( fBootDb )
				exit( EXIT_FAILURE );
		}
	} /* chiude il for */

	if ( unique == TRUE && count > 1 )
	{
		send_log( fp, LOG_FREAD, "fread_section: sezione che dovrebbe essere caricata solo una volta in un file viene caricata più volte: %d", count );
		exit( EXIT_FAILURE );
	}

	MUD_FCLOSE( fp );
}
