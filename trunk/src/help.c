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
 >                   	    Modulo gestione Help                            <
\****************************************************************************/

#include <ctype.h>

#include "mud.h"
#include "command.h"
#include "help.h"
#include "fread.h"
#include "interpret.h"


/*
 * Definizioni
 */
#define HELP_LIST	"../helps/help.lst" /* File della list di help */
#define MAX_HELP	800					/* Numero massimo di help caricabili */


/*
 * Variabili
 */
static	HELP_DATA	*first_help_it	= NULL;
static	HELP_DATA	*last_help_it	= NULL;
static	HELP_DATA	*first_help_en	= NULL;
static	HELP_DATA	*last_help_en	= NULL;
		int			 top_help		= 0;		/* Numero di help caricati */


/*
 * Tabella codici delle tipologie di help
 */
const CODE_DATA code_helptype[]=
{
	{ HELPTYPE_MOVEMENT, "HELPTYPE_MOVEMENT",	"movement"	},
	{ HELPTYPE_CHANNEL,	 "HELPTYPE_CHANNEL",	"channel"	},
	{ HELPTYPE_INFO,	 "HELPTYPE_INFO",		"info"		},
	{ HELPTYPE_OBJECT,	 "HELPTYPE_OBJECT",		"object"	},
	{ HELPTYPE_COMBAT,	 "HELPTYPE_COMBAT",		"combat"	},
	{ HELPTYPE_GAMING,	 "HELPTYPE_GAMING",		"gaming"	},
	{ HELPTYPE_GROUP,	 "HELPTYPE_GROUP",		"group"		},
	{ HELPTYPE_OPTION,	 "HELPTYPE_OPTION",		"option"	},
	{ HELPTYPE_EDIT,	 "HELPTYPE_EDIT",		"edit"		},
	{ HELPTYPE_CLAN,	 "HELPTYPE_CLAN",		"clan"		},
	{ HELPTYPE_SYSTEM,	 "HELPTYPE_SYSTEM",		"system"	},
	{ HELPTYPE_SKILL,	 "HELPTYPE_SKILL",		"skill"		},
	{ HELPTYPE_SPELL,	 "HELPTYPE_SPELL",		"spell"		},
	{ HELPTYPE_MUDPROG,	 "HELPTYPE_MUDPROG",	"mudprog"	},
	{ HELPTYPE_ADMIN,	 "HELPTYPE_ADMIN",		"admin"		}
};
const int max_check_helptype = sizeof(code_helptype)/sizeof(CODE_DATA);


/*
 * Ordina la lista degli help alfabeticamente
 * (TT) controlla che vada tutto bene
 */
void sort_help_alfa( HELP_DATA *help, bool ita )
{
	HELP_DATA *help2;

	/* Viene ordinata alfabeticamente sulla base della prima lettera della keyword_it */
	for ( help2 = (ita) ? first_help_it : last_help_it;  help2;  help2 = (ita) ? help2->next_it : help2->next_en )
	{
		char	*keyword;
		char	*keyword2;

		if ( ita )
		{
			keyword  = help->keyword_it;
			keyword2 = help2->keyword_it;
		}
		else
		{
			keyword  = help->keyword_en;
			keyword2 = help2->keyword_en;
		}

		keyword	 = (keyword[0] == '\'')  ? keyword+1  : keyword;
		keyword2 = (keyword2[0] == '\'') ? keyword2+1 : keyword2;

		if ( strcmp(keyword, keyword2) )
		{
			if ( ita )
				INSERT( help, help2, first_help_it, next_it, prev_it );
			else
				INSERT( help, help2, first_help_en, next_en, prev_en );
			break;
		}
	} /* chiude il for */

	if ( !help2 )
	{
		if ( ita )
			LINK( help, first_help_it, last_help_it, next_it, prev_it );
		else
			LINK( help, first_help_en, last_help_en, next_en, prev_en );
	}
}


/*
 * Load a help section.
 */
void fread_help( MUD_FILE *fp )
{
	HELP_DATA *help;
	HELP_DATA *help2;

	CREATE( help, HELP_DATA, 1 );
	help->type	= -1;
	help->trust = -1;

	for ( ; ; )
	{
		char *word;

		word = fread_word( fp );

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_help: fine prematura del file nella lettura" );
			break;
		}

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "KeywordItalian") )	help->keyword_it =		fread_string( fp );
		else if ( !str_cmp(word, "KeywordEnglish") )	help->keyword_en =		fread_string( fp );
		else if ( !str_cmp(word, "Type"			 ) )	help->type =			code_code( fp, fread_word(fp), CODE_HELPTYPE );
		else if ( !str_cmp(word, "Trust"		 ) )	help->trust =			code_code( fp, fread_word(fp), CODE_TRUST );
		else if ( !str_cmp(word, "Text"			 ) )	help->text =			fread_string( fp );
		else if ( !str_cmp(word, "TextToAdmin"	 ) )	help->text_to_admin =	fread_string( fp );
		else if ( !str_cmp(word, "ToSee"		 ) )	help->to_see =			fread_string( fp );
		else if ( !str_cmp(word, "End"			 ) )	break;
		else
			send_log( fp, LOG_FREAD, "fread_help: nessuna etichetta per la parola %s", word );
	}

	if ( !VALID_STR(help->keyword_it) )
	{
		send_log( fp, LOG_BUG, "fread_help: keyword italiana non valida" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(help->keyword_en) )
	{
		send_log( fp, LOG_BUG, "fread_help: keyword inglese non valida" );
		exit( EXIT_FAILURE );
	}

	if ( help->trust < 0 || help->trust >= MAX_TRUST )
	{
		send_log( fp, LOG_BUG, "fread_help: help con trust errata o non acquisita: %d", help->trust );
		exit( EXIT_FAILURE );
	}

	if ( help->type < 0 || help->type >= MAX_HELPTYPE )
	{
		send_log( fp, LOG_BUG, "fread_help: help con tipologia errata o non acquisita: %d", help->type );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(help->text) )
	{
		send_log( fp, LOG_BUG, "fread_help: testo dell'help non valido" );
		exit( EXIT_FAILURE );
	}

	/* Le stringhe text_to_admin e to_see possono essere NULL, quindi salta i controlli per loro */


	/* Controlla che non vi siano keyword duplicate tra gli help */
	for ( help2 = first_help_it;  help2;  help2 = help2->next_it )
	{
		if ( (nifty_is_name(help->keyword_it, help2->keyword_it)
		  ||  nifty_is_name(help->keyword_en, help2->keyword_en)
		  ||  nifty_is_name(help->keyword_it, help2->keyword_en)
		  ||  nifty_is_name(help->keyword_en, help2->keyword_it))
		  && !is_name(help->keyword_en, "sleep") )	/* Sleep è un caso particolare perchè vi esiste sia come comando che come spell */
		{
			send_log( fp, LOG_WARNING, "add_help: trovata una parola chiave uguale in un altro help: it:%s en:%s",
				help2->keyword_it, help2->keyword_en );
		}
	}

	sort_help_alfa( help, TRUE );	/* prima ordina alfabeticamente le keyword in italiano */
	sort_help_alfa( help, FALSE );	/* Poi quelle in inglese */
	top_help++;
}


/*
 * Carica tutti gli Help
 */
void load_helps( void )
{
	fread_list( HELP_LIST, "HELP", fread_help, FALSE );

	if ( top_help >= MAX_HELP )
	{
		send_log( NULL, LOG_FREAD, "load_helps: numero di help caricati: %d uguale o maggiore di quelli caricabili: %d",
			top_help, MAX_HELP );
		exit( EXIT_FAILURE );
	}
}


/*
 * Libera dalla memoria tutti gli help
 */
void free_all_helps( void )
{
	HELP_DATA *help;

	for ( help = first_help_it;  help;  help = first_help_it )
	{
		UNLINK( help, first_help_it, last_help_it, next_it, prev_it );
		UNLINK( help, first_help_en, last_help_en, next_en, prev_en );
		top_help--;

		DISPOSE( help->keyword_it );
		DISPOSE( help->keyword_en );
		DISPOSE( help->text );
		DISPOSE( help->text_to_admin );
		DISPOSE( help->to_see );
		DISPOSE( help );
	}

	if ( top_help != 0 )
		send_log( NULL, LOG_BUG, "free_all_helps: top_help non è 0 dopo aver liberato gli help: %d", top_help );
}


/*
---------------------------------------------------------------------
 * Ranks by number of matches between two whole words.
 * Coded for the Similar Helpfiles Snippet by Senir.
---------------------------------------------------------------------
 */
int str_similarity( const char *astr, const char *bstr )
{
	int matches = 0;

	if ( !VALID_STR(astr) || !VALID_STR(bstr) )
		return matches;

	for ( ;  *astr;  astr++ )
	{
		if ( tolower(*astr) == tolower(*bstr) )
			matches++;

		if ( ++bstr == '\0' )
			return matches;
	}

	return matches;
}


/*
---------------------------------------------------------------------
 * Ranks by number of matches until there's a nonmatching character between two words.
 * Coded for the Similar Helpfiles Snippet by Senir.
---------------------------------------------------------------------
 */
int str_prefix_level( const char *astr, const char *bstr )
{
	int matches = 0;

	if ( !VALID_STR(astr) || !VALID_STR(bstr) )
		return matches;

	for ( ;  *astr;  astr++ )
	{
		if ( tolower(*astr) == tolower(*bstr) )
			matches++;
		else
			return matches;

		if ( ++bstr == '\0' )
			return matches;
	}

	return matches;
}


/*
---------------------------------------------------------------------
 * Main function of Similar Helpfiles Snippet by Senir.
 * It loops through all of the helps, using the string matching
 *	function defined to find the closest matching helpfiles to the argument.
 * It then checks for singles. Then, if matching helpfiles are found at all,
 *	it loops through and prints out the closest matching helpfiles.
 * If its a single(there's only one), it opens the helpfile.
---------------------------------------------------------------------
 * (FF) secondo me si potrebbe migliorare un poco
 */
void similar_help_files( CHAR_DATA *ch, char *argument )
{
	HELP_DATA	*pHelp = NULL;
	char		*extension;
	char		 buf[MSL];
	int			 lvl = 0;
	bool		 single = FALSE;

	send_to_pager( ch, "&C&BArgomenti di aiuto simili:\n\r" );

	for ( pHelp = first_help_it;  pHelp;  pHelp = pHelp->next_it )
	{
		if ( pHelp->trust > get_trust(ch) )
			continue;

		buf[0] = '\0';
		extension = HAS_BIT(ch->pg->flags, PLAYER_ITALIAN) ? pHelp->keyword_it : pHelp->keyword_en;

		while ( VALID_STR(extension) )
		{
			extension = one_argument( extension, buf );

			if ( str_similarity(argument, buf) > lvl )
			{
				lvl = str_similarity( argument, buf );
				single = TRUE;
			}
			else if ( str_similarity(argument, buf) == lvl && lvl > 0 )
				single = FALSE;
		}
	}

	if ( lvl == 0 )
	{
		send_to_pager( ch, "&C&GNon sono stati trovati argomenti di aiuto simili.\r\n" );
		return;
	}

	for ( pHelp = first_help_it;  pHelp;  pHelp = pHelp->next_it )
	{
		if ( pHelp->trust > get_trust(ch) )
			continue;

		buf[0] = '\0';
		extension = HAS_BIT(ch->pg->flags, PLAYER_ITALIAN) ? pHelp->keyword_it : pHelp->keyword_en;

		while ( VALID_STR(extension) )
		{
			extension = one_argument( extension, buf );

			if ( str_similarity(argument, buf) >= lvl )
			{
				if ( single )
				{
					send_to_pager( ch, "&C&GTrovato solo un argomento di aiuto simile:&C\n\r" );
					do_help( ch, buf );
					return;
				}

				pager_printf( ch, "&C&G   %s\n\r", HAS_BIT(ch->pg->flags, PLAYER_ITALIAN) ? pHelp->keyword_it : pHelp->keyword_en );
				break;
			}
		}
	}
}


/*
 * Moved into a separate function so it can be used for other things.
 */
HELP_DATA *get_help( CHAR_DATA *ch, char *argument )
{
	HELP_DATA  *pHelp;
	char		arg_all[MIL];
	char		arg_one[MIL];

	if ( !VALID_STR(argument) )
		argument = "SOMMARIO";

	ch = get_original(ch);

	/*
	 * Tricky argument handling so 'help a b' doesn't match a.
	 */
	arg_all[0] = '\0';
	while ( VALID_STR(argument) )
	{
		argument = one_argument( argument, arg_one );
		if ( VALID_STR(arg_all) )
			strcat( arg_all, " " );
		strcat( arg_all, arg_one );
	}

	/*
	 * Esegue le ricerche sulle keyword di help in maniera esatta
	 */
	/* Prima nella lingua del pg */
	for ( pHelp = first_help_it;  pHelp;  pHelp = pHelp->next_it )
	{
		if ( pHelp->trust > ch->pg->trust )
			continue;
		if ( is_name_acc(arg_all, HAS_BIT(ch->pg->flags, PLAYER_ITALIAN) ? pHelp->keyword_it : pHelp->keyword_en) )
			return pHelp;
	}

	/* Controlla ora l'altra lingua non usata dal pg */
	for ( pHelp = first_help_it;  pHelp;  pHelp = pHelp->next_it )
	{
		if ( pHelp->trust > ch->pg->trust )
			continue;
		if ( is_name_acc(arg_all, HAS_BIT(ch->pg->flags, PLAYER_ITALIAN) ? pHelp->keyword_en : pHelp->keyword_it) )
			return pHelp;
	}

	/*
	 * Esegue poi una ricerca sulle keyword di help in maniera prefix
	 */
	/* Prima nella lingua utilizzata dal pg */
	for ( pHelp = first_help_it;  pHelp;  pHelp = pHelp->next_it )
	{
		if ( pHelp->trust > ch->pg->trust )
			continue;
		if ( is_name_prefix_acc(arg_all, HAS_BIT(ch->pg->flags, PLAYER_ITALIAN) ? pHelp->keyword_it : pHelp->keyword_en) )
			return pHelp;
	}

	/* Ora invece nella lingua non utilizzata dal pg */
	for ( pHelp = first_help_it;  pHelp;  pHelp = pHelp->next_it )
	{
		if ( pHelp->trust > ch->pg->trust )
			continue;
		if ( is_name_prefix_acc(arg_all, HAS_BIT(ch->pg->flags, PLAYER_ITALIAN) ? pHelp->keyword_it : pHelp->keyword_en) )
			return pHelp;
	}

	return NULL;
}


/*
 * Now this is cleaner
 */
DO_RET do_help( CHAR_DATA *ch, char *argument )
{
	HELP_DATA *pHelp;

	if ( !VALID_STR(argument) )
	{
		send_command( ch, "help HELP", CO );
		return;
	}

	pHelp = get_help( ch, argument );
	if ( !pHelp )
	{
		/* Looks better printing out the missed argument before going to similar helpfiles */
		pager_printf( ch, "&C&wNo help on \'%s\' found.\n\r", argument );
		similar_help_files( ch, argument );
		return;
	}

	/* Agli admi visualizza la keyword*/
	if ( IS_ADMIN(ch) )
	{
		if ( str_cmp(pHelp->keyword_it, pHelp->keyword_en) )
		{
			pager_printf( ch, "It: %s\r\n", pHelp->keyword_it );
			pager_printf( ch, "En: %s\r\n", pHelp->keyword_en );
		}
		else
			pager_printf( ch, "It/En: %s\r\n", pHelp->keyword_it );
	}

	/*
	 * Strip leading '.' to allow initial blanks.
	 */
	if ( pHelp->text[0] == '.' )
		send_to_pager( ch, change_command_tag(pHelp->text+1, ch) );
	else
		send_to_pager( ch, change_command_tag(pHelp->text, ch) );
}


/*
 * NoHelp added
 */
DO_RET do_nohelps( CHAR_DATA *ch, char *argument )
{
	COMMAND_DATA *cmd;
	int			  col = 0;
	int			  sn  = 0;

	if ( is_name(argument, "comandi commands") )
	{
		int		x = 0;
		int		lang = IT;

		send_to_char( ch, "&YComandi trovati senza un corrispondente Aiuto:&w\r\n\r\n" );
		while ( lang <= EN )
		{
			for ( cmd = (lang == IT) ? first_command_it : first_command_en;  cmd;  cmd = cmd->next, x++ )
			{
				char	arg[MIL];
	
				one_argument( cmd->name, arg );
	
				if ( !get_help(ch, arg) )
				{
					ch_printf( ch, "&W%-20s", arg );
					if ( (++col % 4) == 0 )
						send_to_char( ch, "\r\n" );
					if ( (col % 4) == 0
					  && ((lang == IT && x == top_command_it-1)
					  ||  (lang != IT && x == top_command_en-1)) )
					{
						send_to_char( ch, "\r\n" );
					}
				}
			}
			send_to_char( ch, "\r\n" );
		}
	}
	else if ( !str_cmp_acc(argument, "abilità")
	  || is_name(argument, "incantesimi skills spells") )
	{
		send_to_char( ch, "&CAbilità/Incantesimi trovati senza un corrispondente Aiuto:&w\r\n\r\n" );

		for ( sn = 0;  sn < top_sn && table_skill[sn] && table_skill[sn]->name; sn++ )
		{
			if ( !get_help(ch, table_skill[sn]->name) )
			{
				ch_printf( ch, "&W%-25s", table_skill[sn]->name );
				if ( (++col % 3) == 0 )
					send_to_char( ch, "\r\n" );
				if ( col % 3 == 0 && sn == top_sn-1 )
					send_to_char( ch, "\r\n" );
			}
		}
		send_to_char( ch, "\r\n" );
		return;
	}
	else
	{
		send_to_char( ch, "&YSintassi&w:  nohelps comandi\r\n" );
		send_to_char( ch, "           nohelps abilità\r\n" );
		send_to_char( ch, "           nohelps incantesimi\r\n" );
	}
}
