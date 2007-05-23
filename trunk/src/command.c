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
 > 			Modulo riguardante caricamento e gestione dei comandi			<
\****************************************************************************/


#include <ctype.h>
#include <dlfcn.h>		/* libdl */
#include <errno.h>

#include "mud.h"
#include "command.h"
#include "db.h"
#include "fread.h"
#include "interpret.h"


/*
 * Definizioni
 */
#define	MAX_COMMAND		400		/* Numero di comandi caricabili */
#define COMMAND_DIR		"../commands/"
#define CMDORDER_FILE	"command_order.txt"


/*
 * Variabili esterne
 */
CMDFUN_DATA		*first_cmdfun		= NULL;
CMDFUN_DATA		*last_cmdfun		= NULL;
COMMAND_DATA	*first_command_it	= NULL;
COMMAND_DATA	*last_command_it	= NULL;
COMMAND_DATA	*first_command_en	= NULL;
COMMAND_DATA	*last_command_en	= NULL;
int				 top_cmdfun			= 0;	/* Numero di funzioni di comando caricate */
int				 top_command_it		= 0;	/* Numero di comandi italiani caricati */
int				 top_command_en		= 0;	/* Numero di comandi inglesi caricati */


/*
 * Tabella dei codici delle tipologie di comando
 */
const CODE_DATA code_cmdtype[] =
{
	{ CMDTYPE_MOVEMENT,		"CMDTYPE_MOVEMENT",		"movimento"		},
	{ CMDTYPE_CHANNEL,		"CMDTYPE_CHANNEL",		"comunicazioe"	},
	{ CMDTYPE_INFORMATION,	"CMDTYPE_INFORMATION",	"informazione"	},
	{ CMDTYPE_OBJECT,		"CMDTYPE_OBJECT",		"oggetti"		},
	{ CMDTYPE_COMBAT,		"CMDTYPE_COMBAT",		"combattimento" },
	{ CMDTYPE_GAMING,		"CMDTYPE_GAMING",		"gioco"			},
	{ CMDTYPE_GROUPING,		"CMDTYPE_GROUPING",		"gruppo"		},
	{ CMDTYPE_OPTION,		"CMDTYPE_OPTION",		"opzione"		},
	{ CMDTYPE_CLAN,			"CMDTYPE_CLAN",			"clan"			},
	{ CMDTYPE_MUDPROG,		"CMDTYPE_MUDPROG",		"mudprog"		},
	{ CMDTYPE_INTERACTION,	"CMDTYPE_INTERACTION",	"interazione"	},
	{ CMDTYPE_EDITING,		"CMDTYPE_EDITING",		"editing"		},
	{ CMDTYPE_BUILDING,		"CMDTYPE_BUILDING",		"costruzione"	},
	{ CMDTYPE_PUNITION,		"CMDTYPE_PUNITION",		"punizione"		},
	{ CMDTYPE_SYSTEM,		"CMDTYPE_SYSTEM",		"sistema"		}
};
const int max_check_cmdtype = sizeof(code_cmdtype)/sizeof(CODE_DATA);


/*
 * Tabella dei codici delle flag di comando
 */
const CODE_DATA code_cmdflag[] =
{
	{ CMDFLAG_POSSESS,		"CMDFLAG_POSSESS",		"possess"		},
	{ CMDFLAG_POLYMORPH,	"CMDFLAG_POLYMORPH",	"polymorph"		},
	{ CMDFLAG_WATCH,		"CMDFLAG_WATCH",		"watch"			},
	{ CMDFLAG_NOSPIRIT,		"CMDFLAG_NOSPIRIT",		"nospirit"		},
	{ CMDFLAG_INTERACT,		"CMDFLAG_INTERACT",		"interact"		},
	{ CMDFLAG_TYPEALL,		"CMDFLAG_TYPEALL",		"typeall"		}
};
const int max_check_cmdflag = sizeof(code_cmdflag)/sizeof(CODE_DATA);


/*
------------------------------------------------------------------------------
 * DLSym Snippet.
 * Written by Trax of Forever's End
------------------------------------------------------------------------------
 * (FF) questa funzione è uguale alla skill_function in tables.c
 */
DO_FUN *command_function( const char *name )
{
   void		  *funHandle;
   const char *error;

	funHandle = dlsym( sysdata.dlHandle, name );
	if ( (error = dlerror()) != NULL )
	{
		send_log( NULL, LOG_BUG, "Error locating %s in symbol table. %s", name, error );
		return skill_notfound;
	}

	return (DO_FUN *) funHandle;
}


/*
 * Carica i comandi da un file
 */
void fread_cmdfun( MUD_FILE *fp )
{
	CMDFUN_DATA *cmdfun;

	CREATE( cmdfun, CMDFUN_DATA, 1 );

	for ( ; ; )
	{
		char *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_cmdfun: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "End"			) )		break;
		else if ( !str_cmp(word, "Flags"		) )		cmdfun->flags =			fread_code_bit( fp, CODE_CMDFLAG );
		else if ( !str_cmp(word, "FunName"		) )		cmdfun->fun_name =		str_dup( fread_word(fp) );
		else if ( !str_cmp(word, "Log"			) )		cmdfun->log =			code_code( fp, fread_word(fp), CODE_LOG );
		else if ( !str_cmp(word, "MiniHelp"		) )		cmdfun->minihelp =		fread_string( fp );
		else if ( !str_cmp(word, "MsgNoSpirit"	) )		cmdfun->msg_nospirit =	fread_string( fp );
		else if ( !str_cmp(word, "Position"		) )		cmdfun->min_position =	code_code( fp, fread_word(fp), CODE_POSITION );
		else if ( !str_cmp(word, "Type"			) )		cmdfun->type =			code_code( fp, fread_word(fp), CODE_CMDTYPE );
		else if ( !str_cmp(word, "Trust"		) )		cmdfun->trust =			code_code( fp, fread_word(fp), CODE_TRUST );
		else
			send_log( fp, LOG_FREAD, "fread_cmdfun: etichetta non trovata: %s", word );
	}

	/*
	 * Finito di leggere il comando ne controlla i valori
	 */

	/*
	--------------------------
	 * Mods by Trax
	 * Fread in code into char* and try linkage here then deal in the "usual" way I suppose..
	--------------------------
	 */
	if ( !VALID_STR(cmdfun->fun_name) )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: No function name supplied" );
		exit( EXIT_FAILURE );
	}
	cmdfun->do_fun = skill_function( cmdfun->fun_name );
	if ( cmdfun->do_fun == (DO_FUN *) skill_notfound )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: funzione %s non trovata", cmdfun->fun_name );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(cmdfun->minihelp) && cmdfun->type != CMDTYPE_MUDPROG )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: il minihelp è vuoto o scorretto per %s", cmdfun->fun_name );
		exit( EXIT_FAILURE );
	}

	if ( VALID_STR(cmdfun->minihelp) &&  cmdfun->type == CMDTYPE_MUDPROG )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: il minihelp non è vuoto per la funzione di comando %s di tipo MUDPROG",
			cmdfun->fun_name );
		exit( EXIT_FAILURE );
	}

	if ( HAS_BIT(cmdfun->flags, CMDFLAG_NOSPIRIT) && !VALID_STR(cmdfun->msg_nospirit) )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: messaggio di nospirit inesistente per comando con flag NOSPIRIT" );
		exit( EXIT_FAILURE );
	}

	if ( !HAS_BIT(cmdfun->flags, CMDFLAG_NOSPIRIT) && VALID_STR(cmdfun->msg_nospirit) )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: messaggio di nospirit esistente per comando senza flag NOSPIRIT" );
		exit( EXIT_FAILURE );
	}

	if ( cmdfun->type < 0 || cmdfun->type >= MAX_CMDTYPE )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: tipo del comando errato" );
		exit( EXIT_FAILURE );
	}

	if ( cmdfun->trust < 0 || cmdfun->trust >= MAX_TRUST )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: fiducia del comando errata" );
		exit( EXIT_FAILURE );
	}

	if ( cmdfun->min_position < 0 || cmdfun->min_position >= MAX_POSITION )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: posizione del comando errata" );
		exit( EXIT_FAILURE );
	}

	if ( cmdfun->log < 0 || cmdfun->log >= MAX_LOG )
	{
		send_log( fp, LOG_FREAD, "fread_cmdfun: tipo di log del comando errato" );
		exit( EXIT_FAILURE );
	}

	LINK( cmdfun, first_cmdfun, last_cmdfun, next, prev );
	top_cmdfun++;
}


/*
 * Funzione riguardante i controllo di comandi con nome doppio al caricamento
 */
void check_dup_commands( void )
{
	COMMAND_DATA *cmd;
	COMMAND_DATA *check;
	int			  lang;

	/* Controlla che non siano uguali i comandi tra di loro nellla lingua italiana e in quella inglese */
	lang = IT;
	while ( lang <= EN )
	{
		for ( cmd = (lang == IT) ? first_command_it : first_command_en;  cmd;  cmd = cmd->next )
		{
			for ( check = cmd->next;  check;  check = check->next )
			{
				if ( !isalpha(cmd->name[0]) )	continue;
				if ( !isalpha(check->name[0]) )	continue;
	
				if ( nifty_is_name(cmd->name, check->name) )
				{
					send_log( NULL, LOG_LOAD, "check_dup_command: due comandi hanno la stessa etichetta (%s): (%s-%s)",
						(lang == IT) ? "italiano-italiano" : "inglese-inglese",
						cmd->name,
						check->name );
					exit( EXIT_FAILURE );
				}
			}
		}
		lang++;
	}

	/* Poi controlla la lista di comandi eseguendo il confronto tra quella italiana e quella inglese */
	for ( cmd = first_command_it;  cmd;  cmd = cmd->next )
	{
		for ( check = first_command_en;  check;  check = check->next )
		{
			if ( !isalpha(cmd->name[0]) )	continue;
			if ( !isalpha(check->name[0]) )	continue;

			if ( cmd->fun->do_fun == check->fun->do_fun )
				continue;

			if ( nifty_is_name(cmd->name, check->name) )
			{
				send_log( NULL, LOG_LOAD, "check_dup_command: due comandi hanno la stessa etichetta (italiano-inglese): (%s-%s)",
					cmd->name,
					check->name );
				exit( EXIT_FAILURE );
			}
		}
	}
}


/*
 * Legge la lista dei comandi nelle due lingue
 */
void fread_command_list( char *filename, const int lang, COMMAND_DATA **first_command, COMMAND_DATA **last_command, int *top_command )
{
	COMMAND_DATA *cmd;				/* serve per il check finale al nome di funzione */
	CMDFUN_DATA	 *cmdfun;			/* serve a ricavare la funzione dal nome di funzione nel file */
	MUD_FILE	 *fp;				/* file della lista */

	fp = mud_fopen( "", filename, "r", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	if ( lang != IT && lang != EN )
	{
		send_log( NULL, LOG_FREAD, "fread_command_list: lang passato errato: %d", lang );
		exit( EXIT_FAILURE );
	}

	for ( ; ; )
	{
		COMMAND_DATA *command;
		char		 *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_command_list: (%s) fine prematura del file durante la lettura",
				lang == IT ? "ita" : "eng" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		/* Se incontra nella lettura l'etichetta #END ferma la lettura della lista */
		if ( !strcmp(word, "#END") )
			break;

		CREATE( command, COMMAND_DATA, 1 );
		command->synonym = FALSE;

		/* Se incontra un commento lo salta */
		if ( word[0] == '=' && strlen(word) == 1 )
		{
			command->synonym = TRUE;
			word = fread_word( fp );
		}

		if ( !VALID_STR(word) )
		{
			send_log( NULL, LOG_FREAD, "fread_command_list: (%s) nome della funzione acquisito non valido",
				lang == IT ? "ita" : "eng" );
			exit( EXIT_FAILURE );
		}
		for ( cmdfun = first_cmdfun;  cmdfun;  cmdfun = cmdfun->next )
		{
			if ( !strcmp(cmdfun->fun_name, word) )
				command->fun = cmdfun;
		}
		if ( !command->fun )
		{
			send_log( fp, LOG_FREAD, "fread_command_list: (%s) nome di funzione %s inesistente",
				lang == IT ? "ita" : "eng", word );
			exit( EXIT_FAILURE );
		}

		command->name = fread_string( fp );
		if ( !VALID_STR(command->name) )
		{
			send_log( fp, LOG_FREAD, "fread_command_list: (%s) il nome del comando non è valido",
				lang == IT ? "ita" : "eng" );
			exit( EXIT_FAILURE );
		}

		/* Aumenta l'argomento passato top_command_in o top_command_en */
		*top_command++;
		LINK( command, *first_command, *last_command, next, prev );
	}

	/* Controlla che non abbia raggiunto il limite di comandi caricabili */
	if ( *top_command >= MAX_COMMAND )
	{
		send_log( NULL, LOG_LOAD, "load_commands: (%s) comandi caricati totali %d, raggiunto il massimo caricabile: %d",
			lang == IT ? "ita" : "eng", *top_command, MAX_COMMAND );
		exit( EXIT_FAILURE );
	}

	/* Controlla che vi sia un solo nome di funzione non sinonimo per comando nella lista */
	for ( cmd = *first_command;  cmd;  cmd = cmd->next )
	{
		COMMAND_DATA *cmd_check;
		int			  count;

		if ( cmd->synonym )
			continue;

		count = 0;
		for ( cmd_check = cmd->next;  cmd_check;  cmd_check = cmd_check->next )
		{
			if ( cmd_check->synonym )
				continue;

			if ( !str_cmp(cmd->fun->fun_name, cmd_check->fun->fun_name) )
				count++;
		}
	}

	for ( cmdfun = first_cmdfun;  cmdfun;  cmdfun = cmdfun->next )
	{
		int	count;

		count = 0;
		for ( cmd = *first_command;  cmd;  cmd = cmd->next )
		{
			if ( cmd->synonym )
				continue;
			if ( cmd->fun == cmdfun )
				count++;
		}

		if ( count == 0
		  && strcmp(cmdfun->fun_name, "do_whisper_ref") && strcmp(cmdfun->fun_name, "do_whisper_group")
		  && strcmp(cmdfun->fun_name, "do_murmur_ref") && strcmp(cmdfun->fun_name, "do_murmur_group")
		  && strcmp(cmdfun->fun_name, "do_say_ref") && strcmp(cmdfun->fun_name, "do_say_group")
		  && strcmp(cmdfun->fun_name, "do_yell_ref") && strcmp(cmdfun->fun_name, "do_yell_group")
		  && strcmp(cmdfun->fun_name, "do_shout_ref") && strcmp(cmdfun->fun_name, "do_shout_group") )
		{
			if ( cmdfun->type != CMDTYPE_MUDPROG || (cmdfun->type == CMDTYPE_MUDPROG && lang != IT) )	/* non controlla le funzioni di comandi dei mudprog nella lista in italiano visto che non esistono */
			{
				send_log( NULL, LOG_FREAD, "fread_command_list: (%s) non è stato trovato nessun comando con la funzione %s",
					lang == IT ? "ita" : "eng", cmdfun->fun_name );
/*				exit( EXIT_FAILURE ); (TT) è abbastanza normale per ora con il bard che è sempre sotto "lavori in corso" */
			}
		}

		if ( count >= 2 )
		{
			send_log( NULL, LOG_FREAD, "fread_command_list: (%s) trovati %d comandi con funzione in più oltre ai sinonimi: %s",
				lang == IT ? "ita" : "eng", count, cmdfun->fun_name );
			exit( EXIT_FAILURE );
		}
	}
}


/*
 * Funzione di boot per caricare tutti i comandi
 */
void load_commands( void )
{
	CMDFUN_DATA	*cmdfun;
	CMDFUN_DATA	*check;
	char	file[MIL];

	/* Carica prima le informazioni sulle funzioni di comandi */
	fread_section( COMMAND_DIR "player.cmdfun", "CMDFUN", fread_cmdfun, FALSE );
	fread_section( COMMAND_DIR "admin.cmdfun", "CMDFUN", fread_cmdfun, FALSE );
	fread_section( COMMAND_DIR "mprog.cmdfun", "CMDFUN", fread_cmdfun, FALSE );

	/*
	 * Controlla che tutti i nomi di funzioni di comando siano differenti
	 */
	for ( cmdfun = first_cmdfun;  cmdfun;  cmdfun = cmdfun->next )
	{
		for ( check = cmdfun->next;  check;  check = check->next )
		{
			if ( !str_cmp(cmdfun->fun_name, check->fun_name) )
			{
				send_log( NULL, LOG_FREAD, "load_commands: nome di funzione di comando %s trovata duplicata", cmdfun->fun_name );
				exit( EXIT_FAILURE );
			}
		}
	}

	/* Carica poi le liste di comandi in italiano e in inglese */
	fread_command_list( COMMAND_DIR "list_italian.command", IT, &first_command_it, &last_command_it, &top_command_it );
	fread_command_list( COMMAND_DIR "list_english.command", EN, &first_command_en, &last_command_en, &top_command_en );

	/* Controlla che il nome di ogni comando sia unico */
	check_dup_commands( );

	/* Carica le informazioni di ordinamento dei comandi dal file creato nel copyover */
	sprintf( file, "%s%s", COMMAND_DIR, CMDORDER_FILE );
	if ( !remove(file) )
	{
		send_log( NULL, LOG_LOAD, "load_commands: file con i comandi ordinati distrutto" );
	}
	else if ( errno != ENOENT )
	{
		send_log( NULL, LOG_FP, "load_commands: unknown error #%d - %s (file %s)",
			errno, strerror(errno), file );
	}
}


/*
 * Libera dalla memoria tutti i comandi e le funzioni dei comandi
 */
void free_all_commands( void )
{
	COMMAND_DATA *command;
	CMDFUN_DATA	 *cmdfun;

	/* Prima la lista in italiano */
	for ( command = first_command_it;  command;  command = first_command_it )
	{
		UNLINK( command, first_command_it, last_command_it, next, prev );
		top_command_it--;

		DISPOSE( command->name );
		DISPOSE( command );
	}

	if ( top_command_it != 0 )
		send_log( NULL, LOG_BUG, "free_all_commands: top_command_it non è 0 dopo aver liberato tutti i comandi italiani: %d", top_command_it );

	/* Poi libera la lista in inglese */
	for ( command = first_command_en;  command;  command = first_command_en )
	{
		UNLINK( command, first_command_en, last_command_en, next, prev );
		top_command_en--;

		DISPOSE( command->name );
		DISPOSE( command );
	}

	if ( top_command_it != 0 )
		send_log( NULL, LOG_BUG, "free_all_commands: top_command_en non è 0 dopo aver liberato tutti i comandi inglesi: %d", top_command_en );

	/* Infine libera tutta la lista delle funzioni dei comandi */
	for ( cmdfun = first_cmdfun;  cmdfun;  cmdfun = first_cmdfun )
	{
		UNLINK( cmdfun, first_cmdfun, last_cmdfun, next, prev );
		top_cmdfun--;

		DISPOSE( cmdfun->fun_name );
		DISPOSE( cmdfun->minihelp );
		DISPOSE( cmdfun->msg_nospirit );
		destroybv( cmdfun->flags );
		DISPOSE( cmdfun );
	}

	if ( top_cmdfun != 0 )
		send_log( NULL, LOG_BUG, "free_all_commands: top_cmdfun non è 0 dopo aver liberato tutte le funzioni dei comandi: %d", top_cmdfun );
}


/*
 * Trova il comando corrispondente all'argomento command nella lisat dei comandi
 * exact a TRUE cerca i comandi solo in maniera esatta
 * exact a FALSE cerca i comandi in maniera prefissa
 * exact ad ALL cerca sia esatti prima e poi in maniera prefissa,
 *	che non è la stessa cosa che un semplice prefix (basti pensare al comando "force"/"for" in questo ordine)
 * ita a TRUE cerca i comandi in italiano
 * ita a FALSE cerca i comandi in inglese
 * ita cerca tra i comandi in italiano e in inglese
 */
COMMAND_DATA *find_command( CHAR_DATA *ch, const char *command, bool exact, bool ita )
{
	COMMAND_DATA *cmd;

	if ( !VALID_STR(command) )
	{
		send_log( NULL, LOG_BUG, "find_command: argomento command passato non è valido" );
		return NULL;
	}

	/* Nella fread_mudprog e nella load_socials all'avvio viene utilizzata la find_command,
	 *	quindi ch NULL è lecito solo all'avvio */
	if ( !ch && !fBootDb )
	{
		send_log( NULL, LOG_BUG, "find_command: ch è NULL" );
		return NULL;
	}

	if ( ch )
	{
		ch = get_original( ch );
		if ( IS_MOB(ch) && (ita == TRUE || ita == ALL) )
		{
			send_log( NULL, LOG_BUG, "find_command: ita passato a %s per il mob %s #%d",
				ita ? "TRUE" : "ALL", ch->short_descr, ch->pIndexData->vnum );
			ita = FALSE;
		}
	}

	/* Fa una ricerca esatta del comando nella lingua del pg */
	if ( exact == TRUE || exact == ALL )
	{
		if ( ch && (ita == TRUE || ita == ALL) )
		{
			for ( cmd = first_command_it;  cmd;  cmd = cmd->next )
			{
				if ( cmd->fun->trust > get_trust(ch) )
					continue;
				if ( is_name_noquote(command, cmd->name) )	/* utilizza noquote per riuscire a digitare comandi scorciatonia come l'apostrofo */
					return cmd;
			}
		}

		if ( ita == FALSE || ita == ALL )
		{
			for ( cmd = first_command_en;  cmd;  cmd = cmd->next )
			{
				if ( ch && cmd->fun->trust > get_trust(ch) )
					continue;
				if ( is_name_noquote(command, cmd->name) )	/* la find_command con ch == NULL all'avvio del mud checka il comando qui */
					return cmd;
			}
		}
	}

	if ( ch && (exact == FALSE || exact == ALL) )
	{
		if ( ita == TRUE || ita == ALL )
		{
			for ( cmd = first_command_it;  cmd;  cmd = cmd->next )
			{
				if ( cmd->fun->trust > get_trust(ch) )
					continue;
				if ( is_name_prefix(command, cmd->name) )
					return cmd;
			}
		}

		if ( ita == FALSE || ita == ALL )
		{
			for ( cmd = first_command_en;  cmd;  cmd = cmd->next )
			{
				if ( cmd->fun->trust > get_trust(ch) )
					continue;
				if ( is_name_prefix(command, cmd->name) )
					return cmd;
			}
		}
	}

	return NULL;
}


/*
 * Passandogli il nome della funzione del comando voluto
 *	ritorna 
 */
char *command_from_fun( const char *fun_name, bool ita )
{
	COMMAND_DATA *command;
	static char	  arg[MIL];

	if ( !VALID_STR(fun_name) )
	{
		send_log( NULL, LOG_BUG, "command_from_fun: fun_name passato non è valido" );
		return NULL;
	}

	for ( command = ita ? first_command_it : first_command_en;  command;  command = command->next )
	{
		if ( command->synonym )
			continue;

		if ( !str_cmp(command->fun->fun_name, fun_name) )
		{
			one_argument( command->name, arg );
			return arg;
		}
	}

	return NULL;
}


/*
 * Elenca tutti i comandi.
 * Con l'argomento 'italiano' elenca tutti quelli in italiano
 * Con l'argomento 'inglese' elenca tutti quelli in inglese
 * Con l'argomento 'traduzione' li traduce
 * Con l'argomento 'tipi' li suddivide per tipologia
 * Con l'argomento 'fiducia' li suddivide per trust
 * Se si passa un altro argomento lo cerca
 */
DO_RET do_commands( CHAR_DATA *ch, char *argument )
{
	COMMAND_DATA *cmd;
	char		  cmd_tr[MIL];
	char		  arg[MIL];
	char		  buf[MIL];
	char		  letter;
	int			  col = 0;
	bool		  ita;
	bool		  minihelp = FALSE;
	bool		  found;

	set_char_color( AT_PLAIN, ch );

	/* Visualizza la sintassi del comando */
	strcpy( cmd_tr, translate_command(ch, "commands") );

	pager_printf( ch, "&YSintassi&w:  %s\r\n",						cmd_tr	);
	pager_printf( ch, "           %s inglese\r\n",					cmd_tr	);
	pager_printf( ch, "           %s italiano\r\n",					cmd_tr	);
	pager_printf( ch, "           %s traduzione\r\n",				cmd_tr	);
	pager_printf( ch, "           %s tipi\r\n",						cmd_tr	);
	if ( IS_ADMIN(ch) )
		pager_printf( ch, "           %s fiducia\r\n",				cmd_tr	);
	pager_printf( ch, "           %s <comando cercato>\r\n\r\n",	cmd_tr	);

	send_to_pager( ch, "Se si vuole una breve spiegazione di quello che serve ogni comando aggiungere alla\r\n" );
	pager_printf ( ch, "sintassi dei comandi sopra la parola  &gmini%s&w\r\n", translate_command(ch, "help") );
	pager_printf ( ch, "Esempio: %s tipi mini%s\r\n\r\n", translate_command(ch, "commands"), translate_command(ch, "help") );

	/* Se il pg usa i comandi in italiano allora richiama quelli */
	if ( (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN)) )
		ita = TRUE;
	else
		ita = FALSE;

	/* Controlla se sia stata passata la parola chiave minihelp */
	argument = one_argument( argument, arg );
	if ( is_name(argument, "minihelp miniaiuto") || (!VALID_STR(argument) && is_name(arg, "minihelp miniaiuto")) )
		minihelp = TRUE;

	if ( is_name(arg, "italiano italian") )	/* Se chiede di vedere i comandi in italiano */
		ita = TRUE;
	if ( is_name(arg, "inglese english") )		/* Se chiede di vedere i comandi in inglese */
		ita = FALSE;

	/* Visualizza la lista dei comandi in italiano o in inglese. */
	if ( !VALID_STR(arg) || is_name(arg, "italiano italian inglese english minihelp miniaiuto") )
	{
		pager_printf( ch, "Lista dei comandi in %s:\r\n\r\n", (ita)  ?  "italiano"  :  "inglese" );

		for ( letter = 'a'; letter <= 'z'; letter++ )
		{
			int		x = 0;

			for ( cmd = (ita) ? first_command_it : first_command_en;  cmd;  cmd = cmd->next, x++ )
			{
				if ( cmd->name[0] != letter )				continue;
				if ( cmd->synonym )							continue;
				if ( cmd->fun->trust > get_trust(ch) )		continue;
				if ( cmd->fun->type == CMDTYPE_MUDPROG )	continue;

				one_argument( cmd->name, buf );

				if ( !minihelp )
				{
					pager_printf( ch, "%-16s", buf );
					if ( (++col % 5) == 0 )
						send_to_pager( ch, "\r\n" );
					if ( (col % 5) == 0
					  && (( ita && x == top_command_it-1)
					  ||  (!ita && x == top_command_en-1)) )
					{
						send_to_pager( ch, "\r\n" );
					}
				}
				else
				{
					pager_printf( ch, "&G%-16s&w %s\r\n", buf, cmd->fun->minihelp );
				}
			}
		}
		return;
	}

	/* Visualizza i comandi traducendoli dalla lingua che si sta utilizzando */
	if ( is_name(arg, "traduzione traduzioni translations") )
	{
		pager_printf( ch, "Lista dei comandi tradotti dall'%s all'%s:\r\n\r\n",
			(ita)  ?  "inglese"  :  "italiano",
			(ita)  ?  "italiano"   :  "inglese" );

		for ( letter = 'a'; letter <= 'z'; letter++ )
		{
			int		x = 0;

			for ( cmd = ita ? first_command_en : first_command_it;  cmd;  cmd = cmd->next, x++ )
			{
				char	arg_cmdit[MIL];
				char	arg_cmden[MIL];

				if ( cmd->name[0] != letter )				continue;
				if ( cmd->synonym )							continue;
				if ( cmd->fun->trust > get_trust(ch) )		continue;
				if ( cmd->fun->type == CMDTYPE_MUDPROG )	continue;

				one_argument( cmd->name, arg_cmdit );
				strcpy( arg_cmden, command_from_fun(cmd->fun->fun_name, !ita) );

				if ( !minihelp )
				{
					pager_printf( ch, "%-16s -> %-16s",
						(ita)  ?  arg_cmden  :  arg_cmdit,
						(ita)  ?  arg_cmdit  :  arg_cmden );
					if ( (++col % 2) == 0 )
						send_to_pager( ch, "\r\n" );
					if ( (col % 5) == 0
					  && (( ita && x == top_command_it-1)
					  ||  (!ita && x == top_command_en-1)) )
					{
						send_to_pager( ch, "\r\n" );
					}
				}
				else
				{
					pager_printf( ch, "&G%-16s&w %s\r\n", (ita) ? arg_cmdit : arg_cmden, cmd->fun->minihelp );
				}
			}
		}
		return;
	}

	/* Visualizza i comandi suddivisi in tipologie */
	if ( is_name(arg, "tipo tipi types") )
	{
		int type;

		for ( type = 0;  type < MAX_CMDTYPE;  type++ )
		{
			col = 0;

			if ( type == CMDTYPE_MUDPROG )
				continue;
			if ( type > CMDTYPE_MUDPROG && !IS_ADMIN(ch) )
				continue;

			switch ( type )
			{
			  default:
				send_log( NULL, LOG_BUG, "do_commands: tipologia di comando mancante %d", type );
				break;
			  case CMDTYPE_MOVEMENT:
				send_to_pager( ch, "&R- &WComandi&w relativi ai movimenti e posizioni:\r\n" );
				break;
			  case CMDTYPE_CHANNEL:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi ai canali di comunicazione:\r\n" );
				break;
			  case CMDTYPE_INFORMATION:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi alle informazioni generiche o dell'avventuriero:\r\n" );
				break;
			  case CMDTYPE_OBJECT:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi alla interazione con gli oggetti:\r\n" );
				break;
			  case CMDTYPE_COMBAT:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi al combattimento:\r\n" );
				break;
			  case CMDTYPE_GAMING:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi al gioco in generale:\r\n" );
				break;
			  case CMDTYPE_GROUPING:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi al gruppo di avventurieri:\r\n" );
				break;
			  case CMDTYPE_OPTION:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi alla configurazione e alle opzioni di gioco:\r\n" );
				break;
			  case CMDTYPE_CLAN:
				send_to_pager( ch, "\r\n&R- &WComandi&w relativi alla gestione e informazioni del Clan:\r\n" );
				break;
			  case CMDTYPE_INTERACTION:
				send_to_pager( ch, "\r\n&R- &WComandi&w d'Amministratore relativi alla interazione con i giocatori:\r\n" );
				break;
			  case CMDTYPE_EDITING:
				send_to_pager( ch, "\r\n&R- &WComandi&w d'Amministratore relativi all'editazione di costrutti non area:\r\n" );
				break;
			  case CMDTYPE_BUILDING:
				send_to_pager( ch, "\r\n&R- &WComandi&w d'Amministratore relativi alla creazione di aree:\r\n" );
				break;
			  case CMDTYPE_PUNITION:
				send_to_pager( ch, "\r\n&R- &WComandi&w d'Amministratore relativi gestione delle punizioni o 'pulizia':\r\n" );
				break;
			  case CMDTYPE_SYSTEM:
				send_to_pager( ch, "\r\n&R- &WComandi&w d'Amministratore relativi al Sistema Mud:\r\n" );
				break;
			}

			for ( letter = 'a'; letter <= 'z'; letter++ )
			{
				int		x = 0;

				for ( cmd =  ita ? first_command_it : first_command_en;  cmd;  cmd = cmd->next, x++ )
				{
					if ( cmd->name[0] != letter )			continue;
					if ( cmd->synonym )						continue;
					if ( cmd->fun->trust > get_trust(ch) )	continue;
					if ( cmd->fun->type != type )			continue;

					one_argument( cmd->name, buf );

					if ( !minihelp )
					{

						pager_printf( ch, "%-16s", buf );
						if ( (++col % 5) == 0 )
							send_to_pager( ch, "\r\n" );
						if ( (col % 5) == 0
						  && (( ita && x == top_command_it-1)
						  ||  (!ita && x == top_command_en-1)) )
						{
							send_to_pager( ch, "\r\n" );
						}
					}
					else
					{
						pager_printf( ch, "&G%-16s&w %s\r\n", buf, cmd->fun->minihelp );
					}
				}
			}
		}
		return;
	}

	/* Visualizza i comandi suddividendoli per fiducia */
	if ( is_name(arg, "fiducia trust") )
	{
		int trust;

		for ( trust = TRUST_PLAYER+1;  trust < MAX_TRUST;  trust++ )
		{
			col = 0;

			if ( trust <= get_trust(ch) )
			{
				switch ( trust )
				{
				  default:
					send_log( NULL, LOG_BUG, "do_commands: fiducia mancante: %d", trust );
					break;
				  case TRUST_NEOMASTER:
					send_to_pager( ch, "&R-&W Comandi&w per &YNeoMaster&w:\r\n" );
					break;
				  case TRUST_MASTER:
					send_to_pager( ch, "\r\n&R-&W Comandi&w per &YGameMaster&w:\r\n" );
					break;
				  case TRUST_NEOBUILDER:
					send_to_pager( ch, "\r\n&R-&W Comandi&w per &YNeoBuilder&w:\r\n" );
					break;
				  case TRUST_BUILDER:
					send_to_pager( ch, "\r\n&R-&W Comandi&w per &YBuilder&w:\r\n" );
					break;
				  case TRUST_IMPLE:
					send_to_pager( ch, "\r\n&R-&W Comandi&w per gli &YImplementor&w:\r\n" );
					break;
				}

				for ( letter = 'a';  letter <= 'z';  letter++ )
				{
					int		x = 0;

					for ( cmd = ita ? first_command_it : first_command_en;  cmd;  cmd = cmd->next, x++ )
					{
						if ( cmd->name[0] != letter )	continue;
						if ( cmd->synonym )				continue;
						if ( cmd->fun->trust != trust )	continue;

						one_argument( cmd->name, buf );

						if ( !minihelp )
						{
							pager_printf( ch, "%-16s", buf );
							if ( (++col % 5) == 0 )
								send_to_pager( ch, "\r\n" );
							if ( (col % 5) == 0
							  && (( ita && x == top_command_it-1)
							  ||  (!ita && x == top_command_en-1)) )
							{
								send_to_pager( ch, "\r\n" );
							}
						}
						else
						{
							pager_printf( ch, "&G%-16s&w %s\r\n", buf, cmd->fun->minihelp );
						}
					}
				}
			}
		}
		return;
	}

	/* Ricerca del comando tramite argomento */
	found = FALSE;
	for ( letter = 'a'; letter <= 'z'; letter++ )
	{
		int		x = 0;

		for ( cmd = ita ? first_command_it : first_command_en;  cmd;  cmd = cmd->next, x++ )
		{
			if ( cmd->name[0] != letter )				continue;
			if ( cmd->synonym )							continue;
			if ( cmd->fun->trust > get_trust(ch) )		continue;
			if ( cmd->fun->type == CMDTYPE_MUDPROG )	continue;
			if ( str_prefix(arg, cmd->name) )			continue;

			one_argument( cmd->name, buf );

			if ( !minihelp )
			{
				pager_printf( ch, "%-16s", buf );
				found = TRUE;
				if ( (++col % 5) == 0 )
					send_to_pager( ch, "\r\n" );
				if ( (col % 5) == 0
				  && (( ita && x == top_command_it-1)
				  ||  (!ita && x == top_command_en-1)) )
				{
					send_to_pager( ch, "\r\n" );
				}
			}
			else
			{
				pager_printf( ch, "&G%-16s&w %s\r\n", buf, cmd->fun->minihelp );
			}
		}
	}

	if ( !found )
		ch_printf( ch, "Comando non trovato con argomento %s.\r\n", arg );
}


/*
 * Comando Admin che visualizza l'utilizzo dei comandi e il lag dello stesso
 * (FF) convertirlo nel comando usetable che visualizza gli utilizzi e i lag dei
 *	comandi, skills e social etc etc
 */
DO_RET do_cmdtable( CHAR_DATA *ch, char *argument )
{
	COMMAND_DATA *cmd;
	int			  column = 0;
	bool		  ita;

	if ( !VALID_STR(argument) )
	{
		pager_printf( ch, "Sintassi: %s",			translate_command(ch, "cmdtable") );
		pager_printf( ch, "Sintassi: %s italiano",	translate_command(ch, "cmdtable") );
		pager_printf( ch, "Sintassi: %s inglese",	translate_command(ch, "cmdtable") );
		ita = ( HAS_BIT_PLR(ch, PLAYER_ITALIAN) );
	}
	else
	{
		if ( !str_prefix(argument, "italiano") )
			ita = TRUE;
		else if ( !str_prefix(argument, "inglese") )
			ita = FALSE;
		else
		{
			send_command( ch, "cmdtable", CO );
			return;
		}
	}

	set_char_color( AT_ADMIN, ch );
	pager_printf( ch, "%-15.15s %5s %3s  %-15.15s %5s %3s  %-15.15s %5s %3s\r\n",
		"Comando", "Uso", "Lag",
		"Comando", "Uso", "Lag",
		"Comando", "Uso", "Lag" );

	set_char_color( AT_PLAIN, ch );
	for ( cmd = ita ? first_command_it : first_command_en;  cmd;  cmd = cmd->next )
	{
		char	arg[MIL];

		/* carpisce solo il primo nome del comando nella lingua del pg */
		one_argument( cmd->name, arg );

		pager_printf( ch,"%-15.15s %5d %3d",
			arg,
			cmd->use_count.num_uses,
			cmd->lag_count );

		if ( ++column % 3 )
			send_to_pager( ch, "  " );
		else
			send_to_pager( ch, "\r\n" );
	}

	send_to_char( ch, "\r\n" );
}


/*
 * Comando Admin che visualizza un comando oppure la lista di tutti per le priorità
 */
DO_RET do_showcmd( CHAR_DATA *ch, char *argument )
{
	COMMAND_DATA *command;	/* comando ricavato dall'argomento passato */
	COMMAND_DATA *cmd;		/* puntatore al comando per il ciclo for */

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Sintassi: showcmd <comando>\r\n" );
		send_to_char( ch, "Sintassi: showcmd italiano\r\n" );
		send_to_char( ch, "Sintassi: showcmd inglese\r\n" );
		return;
	}

	command = find_command( ch, argument, FALSE, ALL );
	if ( !command )
	{
		send_to_char( ch, "Comando non trovato.\r\n" );
		return;
	}

	if ( is_name(argument, "italiano inglese") )
	{
		char	arg[MIL];

		one_argument( command->name, arg );
		pager_printf( ch, "Priority placement for [%s]:\r\n", arg );

		for ( cmd = (!str_cmp(argument, "italiano")) ? first_command_it : first_command_en;  cmd;  cmd = cmd->next )
		{
			if ( cmd == command )
				set_char_color( AT_GREEN, ch );
			else
				set_char_color( AT_PLAIN, ch );

			send_to_pager( ch, cmd->name );
			send_to_pager( ch, "\r\n" );
		}

		return;
	}

	send_to_char( ch, "Lista di comandi in italiano:\r\n" );
	for ( cmd = first_command_it;  cmd;  cmd = cmd->next )
	{
		if ( cmd->fun != command->fun )
			continue;
		if ( cmd->synonym )
			set_char_color( AT_GREEN, ch );
		else
			set_char_color( AT_PLAIN, ch );
		ch_printf( ch, "%s\r\n", cmd->name );
	}
	send_to_char( ch, "\r\n" );

	send_to_char( ch, "Lista di comandi in inglese:\r\n" );
	for ( cmd = first_command_en;  cmd;  cmd = cmd->next )
	{
		if ( cmd->fun != command->fun )
			continue;
		if ( cmd->synonym )
			set_char_color( AT_GREEN, ch );
		else
			set_char_color( AT_PLAIN, ch );
		ch_printf( ch, "%s\r\n", cmd->name );
	}
	send_to_char( ch, "\r\n" );

	if ( command->use_count.num_uses )
		send_timer( &command->use_count, ch );

	ch_printf( ch, "Funzione   %s\r\n", command->fun->fun_name );
	ch_printf( ch, "Minihelp   %s\r\n", command->fun->minihelp );
	ch_printf( ch, "Tipo       %s\r\n", code_name(NULL, command->fun->type, CODE_CMDTYPE) );
	ch_printf( ch, "Fiducia    %s\r\n", code_name(NULL, command->fun->trust, CODE_TRUST) );
	ch_printf( ch, "Posizione  %d\r\n", command->fun->min_position );
	ch_printf( ch, "Log        %d\r\n", command->fun->log );
	ch_printf( ch, "Flags      %s\r\n", code_bit(NULL, command->fun->flags, CODE_CMDFLAG) );
}
