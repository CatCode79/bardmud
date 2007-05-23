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
 >							Varie funzioni di utilità						<
\****************************************************************************/

#include <ctype.h>

#include "mud.h"
#include "fread.h"
#include "gramm.h"
#include "nanny.h"


/*
 * Aggiunge un punto ogni tre cifre.
 */
char *num_punct( int foo )
{
	char		buf[16];
	static char buf_new[16];
	int			pos;
	int			pos_new;
	int			rest;

	sprintf( buf, "%d", foo );
	rest = strlen( buf ) % 3;

	for ( pos = pos_new = 0;  pos < (int)strlen(buf);  pos++, pos_new++ )
	{
		if ( pos != 0 && (pos-rest)%3 == 0 )
		{
			buf_new[pos_new] = '.';
			pos_new++;
			buf_new[pos_new] = buf[pos];
		}
		else
			buf_new[pos_new] = buf[pos];
	}
	buf_new[pos_new] = '\0';

	return buf_new;
}


/*
 * Toglie i caratteri \r\n da fine di una riga.
 */
char *strip_crnl_end( char *str )
{
	int		len;

	if ( !VALID_STR(str) )
		return "";

	len = strlen( str );

	/* Controlla l'ultima lettera se c'è */
	if ( len >= 1 && (str[len-1] == '\n' || str[len-1] == '\r') )
		str[len-1] = '\0';

	/* Controlla la penultima lettera se c'è */
	if ( len >= 2 && (str[len-2] == '\r' || str[len-2] == '\n') )
		str[len-2] = '\0';

	return str;
}


/*
 * Rimuove il carattere c passato dalla stringa str.
 * Serve ad esempio per togliere gli '\r' e salvare le stringhe su file.
 */
char *strip_char( const char *str, char c )
{
	static char newstr[MSL];
	int			i;
	int			j;

	if ( !VALID_STR(str) )
		return "";

	if ( c == '\0' )
	{
		send_log( NULL, LOG_BUG, "strip_char: il carattere passato da strippare è il NULL" );
		strcpy( newstr, str );
		return newstr;
	}

	for ( i = j = 0;  str[i] != '\0';  i++ )
	{
		if ( str[i] != c )
			newstr[j++] = str[i];
	}

	newstr[j] = '\0';
	return newstr;
}


/*
 * Toglie dalla stringa tutte le punteggiature
 */
char *strip_punct( const char *str )
{
	static char newstr[MSL];
	int			i;
	int			j;

	if ( !VALID_STR(str) )
		return "";

	for ( i = j = 0;  str[i] != '\0';  i++ )
	{
		if ( !ispunct(str[i]) )
			newstr[j++] = str[i];
	}

	newstr[j] = '\0';
	return newstr;
}


/*
 * Rimuove le tilde da una stringa.
 * Utilizzata per salvare su file le stringhe digitate nel Mud.
 */
void smash_tilde( char *str )
{
	if ( !VALID_STR(str) )
		return;

	for ( ;  VALID_STR(str);  str++ )
	{
		if ( *str == '~' )
			*str = '-';
	}
}


#if 0
/*
 * Rimuove la tilde da una linea, tranne se è l'ultimo carattere.
 * (FF) Non viene mai utilizzata, la commento, ma rimane sempre utile in futuro
 */
void smush_tilde( char *str )
{
	int		 len;
	char	 last;
	char	*strptr;

	if ( !VALID_STR(str) )
		return;

	strptr = str;

	len  = strlen( str );
	if ( len )
		last = strptr[len-1];
	else
		last = '\0';

	for ( ;  VALID_STR(str);  str++ )
	{
		if ( *str == '~' )
			*str = '-';
	}
	if ( len )
		strptr[len-1] = last;
}
#endif


/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 * (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_cmp: null astr, bstr: %s", bstr );
		return TRUE;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_cmp: astr: %s, null bstr", astr );
		return TRUE;
	}

	for ( ; *astr || *bstr; astr++, bstr++ )
	{
		if ( tolower(*astr) != tolower(*bstr) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 * (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_prefix: null astr, bstr: %s", bstr );
		return TRUE;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_prefix: astr: %s, null bstr", astr );
		return TRUE;
	}

	if ( !VALID_STR(astr) )
		return TRUE;
	if ( !VALID_STR(bstr) )
		return TRUE;

	for ( ; *astr; astr++, bstr++ )
	{
		if ( tolower(*astr) != tolower(*bstr) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 * (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
	int	 sstr1;
	int	 sstr2;
	int	 ichar;
	char c0;

	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_infix: null astr, bstr: %s", bstr );
		return TRUE;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_infix: astr: %s, null bstr", astr );
		return TRUE;
	}

	if ( (c0 = tolower(astr[0])) == '\0' )
		return FALSE;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for ( ichar = 0;  ichar <= sstr2 - sstr1;  ichar++ )
	{
		if ( c0 == tolower(bstr[ichar]) && !str_prefix(astr, bstr+ichar) )
			return FALSE;
	}

	return TRUE;
}


/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 * (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
	int sstr1;
	int sstr2;

	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_suffix: null astr, bstr: %s", bstr );
		return TRUE;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_suffix: astr: %s, null bstr", astr );
		return TRUE;
	}

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	if ( sstr1 <= sstr2 && !str_cmp(astr, bstr+sstr2-sstr1) )
		return FALSE;
	else
		return TRUE;
}


/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 * (compatibility with historical functions).
 * In più, se incontra il carattere #, salta il controllo di quel carattere a quello sucessivo.
 * (exc sta per EXCeption)
 */
bool str_cmp_exc( const char *astr, const char *bstr )
{
	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_cmp_exc: null astr, bstr: %s", bstr );
		return TRUE;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_cmp_exc: astr: %s, null bstr", astr );
		return TRUE;
	}

	for ( ;  *astr || *bstr;  astr++, bstr++ )
	{
		if ( *bstr == '#' )
			continue;

		if ( tolower(*astr) != tolower(*bstr) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 * (compatibility with historical functions).
 * In più, se incontra degli accenti a fine parola controlla adeguatamente se il pg
 *	ha scritto (prendendo come esempio la vocale a): à, a', a
 */
bool str_cmp_acc( const char *astr, const char *bstr )
{
	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_cmp_acc: null astr, bstr: %s", bstr );
		return TRUE;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_cmp_acc: astr: %s, null bstr", astr );
		return TRUE;
	}

	for ( ;  *astr || *bstr;  astr++, bstr++ )
	{
		if ( is_accent(*bstr) )
		{
			bool found = FALSE;

			/* Se di vocale uguale è ok, se però non è accentato e si trova un apostrofo
			 *	dopo allora avanza il puntatore della astr perché la considera un accento-apostrofo */
			switch ( *bstr )
			{
			  case 'à':
				if ( *astr == 'à' )
					found = TRUE;
				if ( *astr == 'a' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'è':
				if ( *astr == 'è' )
					found = TRUE;
				if ( *astr == 'e' && *astr+1 == '\'')
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'é':
				if ( *astr == 'é' )
					found = TRUE;
				if ( *astr == 'e' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'ì':
				if ( *astr == 'ì' )
					found = TRUE;
				if ( *astr == 'i' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'ò':
				if ( *astr == 'ò' )
					found = TRUE;
				if ( *astr == 'o' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'ù':
				if ( *astr == 'ù' )
					found = TRUE;
				if ( *astr == 'u' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;
			}

			if ( found )
				continue;
			else
				return TRUE;	/* Non corrispondono quindi la comparazione fallisce */
		}
		if ( tolower(*astr) != tolower(*bstr) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 * (compatibility with historical functions).
 * Versione str_prefix controlla accenti, vedere str_cmp_acc
 */
bool str_prefix_acc( const char *astr, const char *bstr )
{
	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_prefix_acc: null astr, bstr: %s", bstr );
		return TRUE;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_prefix_acc: astr: %s, null bstr", astr );
		return TRUE;
	}

	for ( ;  *astr;  astr++, bstr++ )
	{
		if ( is_accent(*bstr) )
		{
			bool found = FALSE;

			/* Se di vocale uguale è ok, se però non è accentato e si trova un apostrofo
			 *	dopo allora avanza il puntatore della astr perché la considera un accento-apostrofo */
			switch ( *bstr )
			{
			  case 'à':
				if ( *astr == 'à' )
					found = TRUE;
				if ( *astr == 'a' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'è':
				if ( *astr == 'è' )
					found = TRUE;
				if ( *astr == 'e' && *astr+1 == '\'')
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'é':
				if ( *astr == 'é' )
					found = TRUE;
				if ( *astr == 'e' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'ì':
				if ( *astr == 'ì' )
					found = TRUE;
				if ( *astr == 'i' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'ò':
				if ( *astr == 'ò' )
					found = TRUE;
				if ( *astr == 'o' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;

			  case 'ù':
				if ( *astr == 'ù' )
					found = TRUE;
				if ( *astr == 'u' )
				{
					found = TRUE;
					if ( *(astr+1) == '\'' )
						astr++;
				}
				break;
			}

			if ( found )
				continue;
			else
				return TRUE;	/* Non corrispondono quindi la comparazione fallisce */
		}
		if ( tolower(*astr) != tolower(*bstr) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Funzione che controlla se l'argomento passato sia all, tutto, tutti e tutte.
 * Ritorna TRUE se astr NON è un argomento all o simili (compatibilità con le altre funzioni simili str_cmp())
 */
bool str_cmp_all( char *astr, bool dot )
{
/*	int length;*/

	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_cmp_all: astr è NULL " );
		return TRUE;
	}

	if ( !VALID_STR(astr) )
		return TRUE;

	if ( (dot == FALSE || dot == ALL) && is_name(astr, "tutto tutti tutte all") )
		return FALSE;

	if ( (dot == TRUE || dot == ALL) )
	{
		if ( !strncmp(astr, "tutto.", 6) )	return FALSE;
		if ( !strncmp(astr, "tutti.", 6) )	return FALSE;
		if ( !strncmp(astr, "tutte.", 6) )	return FALSE;
		if ( !strncmp(astr, "all.",   4) )	return FALSE;
	}

	return TRUE;
}


/*
 * Find the position of a target substring in a source string.
 * Returns pointer to the first occurrence of the string pointed to
 * bstr in the string pointed to by astr. It returns a null pointer
 * if no match is found.
 *
 * Note I made a change when modifying str_infix. If the target string is
 * null, I return NULL (meaning no match was found). str_infix returns
 * FALSE (meaning a match was found).  *grumble*
 */
char *str_str( char *astr, char *bstr )
{
	int  sstr1;
	int	 sstr2;
	int	 ichar;
	char c0;

	if ( !astr )
	{
		send_log( NULL, LOG_BUG, "str_str: astr: NULL, bstr: %s", bstr ? bstr : "NULL" );
		return NULL;
	}

	if ( !bstr )
	{
		send_log( NULL, LOG_BUG, "str_str: astr: %s, bstr: NULL", astr );
		return NULL;
	}

	c0 = tolower( bstr[0] );
	if ( c0 == '\0' )
		return NULL;

	sstr1 = strlen( astr );
	sstr2 = strlen( bstr );

	for ( ichar = 0;  ichar <= sstr1-sstr2;  ichar++ )
	{
		if ( c0 != tolower(astr[ichar]) )
			continue;
		if ( !str_prefix(bstr, astr+ichar) )
			return astr + ichar;
	}

	return NULL;
}


/*
 * Definizioni per il passaggio di argomento della one_argument_handler
 */
#define ONE_ARG			0
#define ONE_ARG_EDITOR	2
#define ONE_ARG_NOQUOTE 3

/*
 * Gestisce one_argument e one_argument2
 */
char *one_argument_handler( char *argument, char *arg_first, int type )
{
	char	cEnd;
	int		count;

	count = 0;

	if ( !VALID_STR(argument) )
	{
		arg_first[0] = '\0';
		return "";
	}

	while ( isspace(*argument) )
		argument++;

	cEnd = ' ';
	if ( type != ONE_ARG_NOQUOTE && (*argument == '\'' || *argument == '"') )
		cEnd = *argument++;

	while ( *argument != '\0' || ++count >= 255 )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}

		if ( type == ONE_ARG_EDITOR )
			*arg_first = *argument;
		else
			*arg_first = tolower( *argument );

		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( isspace(*argument) )
		argument++;

	return argument;
}


char *one_argument( char *argument, char *arg_first )
{
	return one_argument_handler( argument, arg_first, ONE_ARG );
}

/*
 * Serve per evitare che renda minuscole le stringhe durante la formattazione
 *	dell'editor
 */
char *one_argument_editor( char *argument, char *arg_first )
{
	return one_argument_handler( argument, arg_first, ONE_ARG_EDITOR );
}

/*
 * Serve ad evitare di mangiarsi la quote nella ricerca del comando '
 *	sinonimo di say
 */
char *one_argument_noquote( char *argument, char *arg_first )
{
	return one_argument_handler( argument, arg_first, ONE_ARG_NOQUOTE );
}


/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes e la puteggiatura.
 * Utilizzato in change_command_tag
 * (bb) se una frase finisce con il comando non prende arg_first e ritorna
 *	solo command, bisognerebbe evitarlo
 */
char *one_command( char *command, char *arg_first )
{
	char	cEnd;
	int		count;

	count = 0;

	if ( !VALID_STR(command) )
	{
		arg_first[0] = '\0';
		return "";
	}

	while ( isspace(*command) )
		command++;

	cEnd = '\0';
	if ( (*command == '\'' || *command == '"') )
		cEnd = *command++;

	while ( VALID_STR(command) || ++count >= 255 )
	{
		if ( *command == cEnd || ispunct(*command)
		  || isspace(*command)|| *command == '\r'
		  || *command == '\n' || *command == '\0' )
		{
			if ( *command == cEnd )
				command++;
			break;
		}
		*arg_first = tolower( *command );
		arg_first++;
		command++;
	}

	*arg_first = '\0';

	return command;
}


/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument( namelist, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( !str_cmp(str, name) )
			return TRUE;
	}
}

/*
 * See if a string is one of the names of an object.
 */
bool is_name_acc( const char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument( namelist, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( !str_cmp_acc(str, name) )
			return TRUE;
	}
}

/*
 * Come la is_name, questa richiama la one_argument
 */
bool is_name_noquote( const char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument_noquote( namelist, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( !str_cmp(str, name) )
			return TRUE;
	}
}

/*
 * Identica alla is_name solo che effettua un controll sul prefisso
 */
bool is_name_prefix( const char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument( namelist, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( !str_prefix(str, name) )
			return TRUE;
	}
}


/*
 * Identica alla is_name_prefix ma richiama la str_prefix_acc.
 */
bool is_name_prefix_acc( const char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument( namelist, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( !str_prefix_acc(str, name) )
			return TRUE;
	}
}


/*
 * See if a string is one of the names of an object.
 * Treats a dash as a word delimiter as well as a space
 */
bool is_name2( const char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument2( namelist, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( !str_cmp_acc(str, name) )		/* Controllo anche sugli accenti */
			return TRUE;
    }
}


bool is_name2_prefix( const char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		namelist = one_argument2( namelist, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( !str_prefix_acc(str, name) )	/* Controllo anche sugli accenti */
			return TRUE;
	}
}


/*
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name( char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		str = one_argument2( str, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( is_name2(name, namelist) )
			return TRUE;
	}
}


bool nifty_is_name_prefix( char *str, char *namelist )
{
	char name[MIL];

	if ( !VALID_STR(str) )
		return FALSE;

	if ( !VALID_STR(namelist) )
		return FALSE;

	for ( ; ; )
	{
		str = one_argument2( str, name );
		if ( !VALID_STR(name) )
			return FALSE;
		if ( is_name2_prefix(name, namelist) )
			return TRUE;
	}
}


/*
 * Counts the number of times a target string occurs in a source string.
 * case insensitive
 */
int str_count( char *psource, char *ptarget )
{
	char *ptemp = psource;
	int	  count = 0;

	if ( !VALID_STR(psource) )
		return 0;
	if ( !VALID_STR(ptarget) )
		return 0;

	while ( (ptemp = str_str(ptemp, ptarget)) )
	{
		ptemp++;
		count++;
	}

	return count;
}


/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
	static char strcap[MSL];
	int			x;
	bool		first = TRUE;

	if ( !VALID_STR(str) )
		return "";

	for ( x = 0;  str[x];  x++ )
	{
		if ( str[x] == '&' )
		{
			strcap[x] = str[x];
			x++;
			if ( !str[x] )
				break;
			strcap[x] = str[x];
		}
		else
		{
			if ( first )
			{
				strcap[x] = toupper( str[x] );
				first = FALSE;
			}
			else
				strcap[x] = tolower( str[x] );
		}
	}

	strcap[x] = '\0';
	return strcap;
}


/*
 * Returns a strlower string.
 */
char *strlower( const char *str )
{
	static char strlow[MSL];
	int i;

	if ( !VALID_STR(str) )
		return "";

	for ( i = 0;  str[i];  i++ )
		strlow[i] = tolower( str[i] );

	strlow[i] = '\0';
	return strlow;
}

/*
 * Returns an strupper string.
 */
char *strupper( const char *str )
{
	static char strup[MSL];
	int i;

	if ( !VALID_STR(str) )
		return "";

	for ( i = 0;  str[i] != '\0';  i++ )
		strup[i] = toupper( str[i] );

	strup[i] = '\0';
	return strup;
}


/*
 * Finds string from end.
 */
char *strlstr( char *line, char *token )
{
	char *pos = NULL;

	if ( !VALID_STR(line) )
		return NULL;

	if ( !VALID_STR(token) )
		return NULL;

	while ( (line = strstr(line, token)) )
	{
		pos = line;
		line++;
	}

	return pos;
}


/*
 * Dump a text file to a player, a line at a time
 */
void show_file( DESCRIPTOR_DATA *d, char *filename, bool msg )
{
	MUD_FILE *fp;
	char	 buf[MSL];
	int		 c;
	int		 num = 0;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "show_file: d è NULL" );
		return;
	}

	if ( !VALID_STR(filename) )
	{
		send_log( NULL, LOG_BUG, "show_file: filename passato non valido" );
		return;
	}

	fp = mud_fopen( "", filename, "r", msg );
	if ( !fp )
		return;

	/* Se in gioco invia con il pager (login, title e menu no ad esempio) */
	while ( !feof(fp->file) )
	{
		while ( (buf[num] = fgetc(fp->file)) != EOF
		  && buf[num] != '\r'
		  && buf[num] != '\n'
		  && num < (MSL-2) )
		{
			num++;
		}

		c = fgetc( fp->file );
		if ( (c != '\r' && c != '\n') || c == buf[num] )
			ungetc( c, fp->file );

		buf[num++] = '\r';
		buf[num++] = '\n';
		buf[num  ] = '\0';

		if ( d && d->connected == CON_PLAYING )
			send_to_pager( d->character, buf );
		else
			send_to_descriptor( d, buf );

		num = 0;
	}

	MUD_FCLOSE( fp );
}


/*
 * Append a string to a file.
 */
void append_to_file( char *file, char *str )
{
	MUD_FILE   *fp;

	if ( !VALID_STR(file) )
	{
		send_log( NULL, LOG_BUG, "show_file: filename passato non valido" );
		return;
	}

	if ( !VALID_STR(str) )
		return;

	fp = mud_fopen( "", file, "a", TRUE );
	if ( !fp )
		return;

	fprintf( fp->file, "%s\n", str );
	MUD_FCLOSE( fp );
}


/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
	switch ( number_bits(2) )
	{
	  case 0:	number -= 1; break;
	  case 3:	number += 1; break;
	}

	return UMAX( 1, number );
}


/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 */
static int rgiState[2+55];

void init_mm( void )
{
	int *piState;
	int iState;

	piState = &rgiState[2];

	piState[-2]	= 55 - 55;
	piState[-1]	= 55 - 24;

	piState[0]	= ((int) current_time) & ((1 << 30) - 1);
	piState[1]	= 1;

	for ( iState = 2;  iState < 55;  iState++ )
		piState[iState] = (piState[iState-1] + piState[iState-2]) & ((1 << 30) - 1);
}


int number_mm( void )
{
	int *piState;
	int  iState1;
	int  iState2;
	int  iRand;

	piState			 = &rgiState[2];
	iState1			 = piState[-2];
	iState2			 = piState[-1];
	iRand			 = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
	piState[iState1] = iRand;

	if ( ++iState1 == 55 )
		iState1 = 0;

	if ( ++iState2 == 55 )
		iState2 = 0;

	piState[-2]		= iState1;
	piState[-1]		= iState2;

	return iRand >> 6;
}


/*
 * Generate a random number.
 * Ooops was (number_mm() % to) + from which doesn't work -Shaddai
 */
int number_range( int from, int to )
{
	if ( (to-from) < 1 )
		return from;

	return ( (number_mm() % (to-from+1)) + from );
}


/*
 * Generate a percentile roll.
 * number_mm() % 100 only does 0-99, changed to do 1-100 -Shaddai
 */
int number_percent( void )
{
	return (number_mm() % 100)+1;
}


int number_bits( int width )
{
	return number_mm( ) & ( (1 << width) - 1 );
}


/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *  gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *  where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 */
void tail_chain( void )
{
	return;
}


/*
 * Versione più friendly della c_time.
 * Non invia l'\n alla fine.
 * La stringa ha una lunghezza totale di 24 (escluso 0).
 */
char *friendly_ctime( time_t *time_passed )
{
	static char	*month[12] = { "Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott" , "Nov" , "Dic" };
	static char	 strtime[48];
	struct tm   *timet = localtime( time_passed );

	sprintf( strtime, "%2d %3s %04d %02d:%02d:%02d",
		timet->tm_mday,
		month[timet->tm_mon],
		timet->tm_year + 1900,
		(timet->tm_hour == 0)  ?  24  :  timet->tm_hour,
		timet->tm_min,
		timet->tm_sec );

	return strtime;
}

/*
 * Come sopra ma non invia l'ora
 */
char *friendly_ctime_nohour( time_t *time_passed )
{
	static char	*month[12] = { "Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott" , "Nov" , "Dic" };
	static char	 strtime[24];
	struct tm   *timet = localtime( time_passed );

	sprintf( strtime, "%2d %3s %4d",
		timet->tm_mday,
		month[timet->tm_mon],
		timet->tm_year + 1900 );

	return strtime;
}


/*
 * Ritorna la data con ordine: anno, mese e giorno
 * Utile per un ordinamento crescente o descrescente
 */
char *reverse_ctime( time_t *time_passed )
{
	struct tm   *timet = localtime( time_passed );
	static char	 strtime[48];

	sprintf( strtime, "%4d-%2d-%2d",
		timet->tm_year + 1900,
		timet->tm_mon,
		timet->tm_mday );

	return strtime;
}


/*
 * Stampa la data in formato gg/mm/aa e ritorna la stringa
 */
char *stamp_time( void )
{
	static char buf[MSL];
	struct tm *t = localtime( &current_time );

	sprintf( buf, "&W%02d&w/&W%02d&w/%04d&w", t->tm_mday, t->tm_mon+1, t->tm_year+1900 );
	return buf;
}


/*
 * Toglie il prefisso xxx_ da una stringa, spesso usato per i Codici
 *	(es da OBJTYPE_KEY ritorna key)
 * E' utile per ritornare una stringa più leggibile e succinta di alcuni Codici
 */
char *slash_code( char *code )
{
	strlower( code );

	while ( *(code++) )
	{
		if ( *code == '_' )
			return code;
	}

	return "";
}


/*
 * Se una stringa è tipo: /nome.txt ritorna nome
 */
char *name_from_file( char *text )
{
	static char	*newtext;
	int			 x;

	if ( !VALID_STR(text) )
		return "";

	newtext = strrchr( text, '/' );

	/* Se non trova la blackslash riprende il nome del file, altrimenti la passa oltre */
	if ( !newtext )
		newtext = text;
	else
		newtext++;

	for ( x = 0;  newtext[x];  x++ )
	{
		if ( newtext[x] == '.' )
		{
			newtext[x] = '\0';		/* (FF) questa è una cosa pericolosa mi sa.. */
			break;
		}
	}

	return newtext;
}


/*
 * Cambia qualsiasi occorrenza di '\' in '/'
 */
void backslash_to_slash( char *text )
{
	char *pbuf = text;

	if ( !VALID_STR(text) )
		return;

	do {
		if ( *pbuf == '\\' )
			*pbuf = '/';
	} while ( *(++pbuf) );
}


/*
 * Ritorna in percentuale il valore passato rispetto ad un massimo passato.
 * Se il valore è negativo, ritorna -1
 */
int get_percent( int val, int max )
{
	if ( max > 0 )
		return (val * 100) / max;

	return 0;
}


/*
 * Passa il pollice anglosassone e lo converte in centimetri.
 */
int inch_to_centimetre( const int inch )
{
	/* Ogni pollice è 2,5 cm */
	return (int) ( inch * 2.54 );
}


/*
 * Passa l'oncia anglosassone e la converte in etti.
 */
int once_to_gram( const int once )
{
	/* Ogni oncia (sistema Avoirdupoids spero che abbiano utilizzato) è 28,35 grammi */
	return (int) ( once * 28.35 );
}


/*
 * Converte un numero in stringa
 */
char *itostr( int num )
{
	static char  buf[12];
	char		*c;

	c = buf+10;
	*c = '\0';

	for ( c--;  num;  num = num/10, c-- )
		*c = num%10+'0';

	return c+1;
}


/*
 * Ritorna vero se i due pg passati hanno lo stesso ip
 */
bool is_same_ip( CHAR_DATA *ch, CHAR_DATA *wch )
{
	char *ip1 = NULL;
	char *ip2 = NULL;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_same_ip: ch è NULL" );
		return FALSE;
	}

	if ( !wch )
	{
		send_log( NULL, LOG_BUG, "is_same_ip: wch è NULL" );
		return FALSE;
	}

	if ( IS_MOB(ch) || IS_MOB(wch) )
		return FALSE;

	if ( ch->desc )
	{
		if ( VALID_STR(ch->desc->host) )
			ip1 = ch->desc->host;
	}

	if ( !ip1 )
	{
		if ( ch->pg && VALID_STR(ch->pg->host) )
			ip1 = ch->pg->host;
	}

	if ( wch->desc )
	{
		if ( VALID_STR(wch->desc->host) )
			ip2 = wch->desc->host;
	}

	if ( !ip2 )
	{
		if ( wch->pg && VALID_STR(wch->pg->host) )
			ip1 = wch->pg->host;
	}

	if ( !ip1 || !ip2 )
	{
		send_log( NULL, LOG_BUG, "same_ip: host è NULL" );
		return FALSE;
	}

	if ( strcmp(ip1, ip2) )
		return FALSE;

	return TRUE;
}


#ifdef T2_ALFA
/*
 * Calcola il numero di cifre di un numero (inutilizzata per ora)
 */
int count_digit( const int num )
{
	int div;
	int x;

	/* Basta contare il numero di divisioni per 10 che ridanno num maggiore di 0 */
	x = 0;
	div = num;
	while ( div > 0 )
	{
		div = div / 10;
		x++;
	}

	return x;
}


/*
 * (FF) Ribalta la stringa, da utilizzare nel sistema dei libri
 */
void reverse_chars( char *s )
{
	char *begin;
	char *end;
	char  c;

	for ( begin = s, end = s+strlen(s)-1;  begin <= end;  begin++, end-- )
	{
		c = *begin;
		*begin = *end;
		*end = c;
	}
}
#endif


/*
 * Funzione che serve solamente a controllare, in fase di compilazione,
 *	che ad una macro sia stato passato un int
 */
bool check_type_int( int integer )
{
	return TRUE;
}
