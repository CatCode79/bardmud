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


/*****************************************************************************\
 >				Modulo di interpretazione comandi, skill e social	 		 <
\*****************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>

/* Le seguenti funzioni sono ridefinite come macro per una compilazione gcc -ansi corretta */
#ifndef timerisset
#	define	timerisset(tvp)	( (tvp)->tv_sec || (tvp)->tv_usec )
#endif
#ifndef timerclear
#define	timerclear(tvp)	( (tvp)->tv_sec = (tvp)->tv_usec = 0 )
#endif
#ifndef timercmp
#	define	timercmp(tvp, uvp, cmp)	( (tvp)->tv_sec cmp (uvp)->tv_sec			\
 										||	((tvp)->tv_sec == (uvp)->tv_sec		\
 										&&	(tvp)->tv_usec cmp (uvp)->tv_usec) )
#endif

#include "mud.h"
#include "affect.h"
#include "alias.h"
#include "clan.h"
#include "command.h"
#include "editor.h"
#include "fread.h"
#include "gramm.h"
#include "interpret.h"
#include "movement.h"
#include "room.h"
#include "timer.h"
#include "watch.h"


/*
 * Log-all switch.
 */
bool	fLogAll = FALSE;


/*
 * Variabili esterne.
 */
char	lastcmd_typed[MIL*2];	/* contiene l'ultimo comando inviato */


/*
 * Controlla se ch è nella giusta posizione per effettuare il comando o la skill
 */
bool check_position( CHAR_DATA *ch, int position )
{
	if ( IS_MOB(ch) && ch->position > POSITION_STUN )	/* (FF) Da rivedere, vorrei fare invece dei check simili a quelli sotto per il pg */
		return TRUE;

	if ( ch->position >= position )
		return TRUE;

	switch ( ch->position )
	{
	  default:
		send_log( NULL, LOG_BUG, "check_position: posizione mancante o errata: %d", ch->position );
		break;

	  case POSITION_DEAD:
		send_to_char( ch, "Un po' difficile fino a che rimango MORTO..\r\n" );
		break;

	  case POSITION_MORTAL:
	  case POSITION_INCAP:
		send_to_char( ch, "Sono troppo ferito per farlo.\r\n" );
		break;

	  case POSITION_STUN:
		send_to_char( ch, "Sono troppo intontito per farlo.\r\n" );
		break;

	  case POSITION_SLEEP:
		send_to_char( ch, "Nei miei sogni, o cosa?\r\n" );
		break;

	  case POSITION_REST:
		send_to_char( ch, "Nah.. Sono troppo rilassato ora..\r\n" );
		break;

	  case POSITION_SIT:
		send_to_char( ch, "Non posso farlo così da seduto.\r\n" );
		break;

	  case POSITION_FIGHT:
	  case POSITION_DEFENSIVE:
	  case POSITION_AGGRESSIVE:
	  case POSITION_BERSERK:
		if ( position <= POSITION_EVASIVE )
			send_to_char( ch, "Richiederebbe troppo per questo stile di combattimento!\r\n" );
		else
			send_to_char( ch, "Non ora, sto combattendo!\r\n" );
		break;

	  case POSITION_EVASIVE:
		send_to_char( ch, "Non ora, sto combattendo!\r\n" );
		break;
	} /* chiude lo switch */

	return FALSE;
}


/*
 * Passando una stringa converte tutti i tag ° traducendoli ad uopo i comandi,
 *	social o skill in essi secondo il set di lingua dei comandi utilizzata dal pg.
 */
char *change_command_tag( char *string, CHAR_DATA *ch )
{
	static char	buf[MSL];
	char 	   *pbuf = buf;
	char		arg[MIL];
	char		cmd[MIL];
	char	   *pcmd;
	int			num;

	while ( VALID_STR(string) )
	{
		if ( *string == '°' )
		{
			string++;
			pcmd = cmd;
			switch ( *string )
			{
			  case 'T':
			    string++;	/* Salta la T, che sta per Translate */
				string = one_command( string, arg );
				strcpy( cmd, color_str(AT_COMMAND, ch) );
				strcat( cmd, translate_command(ch, arg) );
				strcat( cmd, "&w" );
				break;
			  /* Capitalizza la prima lettera del comando */
			  case 'F':
				string++;	/* Salta la F, che sta First letter */
				string = one_command( string, arg );
				strcpy( cmd, color_str(AT_COMMAND, ch) );
				strcat( cmd, translate_command(ch, arg) );
				strcat( cmd, "&w" );
				capitalize( cmd );
				break;
			  /* Capitalizza tutte le lettere del comando */
			  case 'W':
				string++;	/* Salta la W, che sta per Word */
				string = one_command( string, arg );
				strcpy( cmd, color_str(AT_COMMAND, ch) );
				strcat( cmd, translate_command(ch, arg) );
				strcat( cmd, "&w" );
				strupper( cmd );
				break;

			  /* Invia i codici stampandoli su linea (FF) sistema di c e n per inviare il codice o solo il nome */
			  case 'L':
				string++;	/* Salta la L, che sta per stampa i codici in una Linea */
				string = one_command( string, arg );
				num = code_num( NULL, arg, CODE_CODE );
				strcpy( cmd, code_all(num, FALSE) );
				break;
			  /* Invia i codici stampandoli su colonna (FF) sistema di c e n per inviare proprio il codice o solo il nome */
			  case 'S':
				string++;	/* Salta la S, che sta per stampa la liSta di codici */
				string = one_command( string, arg );
				num = code_num( NULL, arg, CODE_CODE );
				strcpy( cmd, code_all(num, TRUE) );
				break;

			  /* Sistema carattere maschile/femminile */
			  case 'x':
				string++;	/* Salva la x, che sta per seX */
				if ( ch->sex == SEX_FEMALE )
					strcpy( cmd, "a" );
				else
					strcpy( cmd, "o" );
				break;

			  /* Copia così come è tornando un carattere indietro */
			  default:
				string--;
				*pbuf++ = *string++;
				break;
			}
		}
		else
		{
			/* Copia così come è */
			*pbuf++ = *string++;
			continue;
		}

		/* Copia il comando tradotto */
		while ( (*pbuf = *pcmd) != '\0' )
		{
			pbuf++;
			pcmd++;
		}

/*		string++;*/
	} /* chiude il while */

	*pbuf = '\0';
	return buf;
}


/*
 * Se l'argomento passato e la lingua del ch non corrispondono cerca tra i comandi
 *	della lingua opposta e la passa alla funzione interpret.
 * Cerca solo se la lingua dell'admin e del pg non sono uguali.
 * Non bisogna inviare comandi con la stessa lingua del pg se l'admin l'ha diversa.
 * Passando l'argomento cmd_lang == IT si indica che il comando è stato inviato presumibilmente in
 *	italiano e deve essere inviato il corrispettivo in inglese
 * Passando l'argomento cmd_lang == EN si indica che il comando è stato inviato presumibilmente in
 *	inglese e deve essere inviato il corrispettivo in italiano
 * Passando l'argomento cmd_lang == TR si indica che il comando è in inglese e si vuole la traduzione
 *	del corrispettivo in italiano, la ritorna e non la invia all'interpret
 * Passando l'argomento cmd_lang == CO si indica che il comando è stato inviato in inglese dal codice,
 *	è come l'EN ma avvisa se il comando è errato
 */
char *send_command( CHAR_DATA *ch, char *argument, const int cmd_lang )
{
	COMMAND_DATA *cmd;
	SOCIAL_DATA  *soc;
	static char	  arg[MIL];
	static char	  arg_cmd[MIL];
	bool		  italian;
	bool		  found = FALSE;

	arg[0] = '\0';
	arg_cmd[0] = '\0';

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "send_command: ch è NULL" );
		return NULL;	/* per TR */
	}

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "send_command: argument è NULL, ch: %s", ch->name );
		return NULL;	/* per TR */
	}

	if ( cmd_lang < IT && cmd_lang > TR )
	{
		send_log( NULL, LOG_BUG, "send_command: cmd_lang passato errato: %d, ch: %s, argument: %s",
			cmd_lang, ch->name, argument );
		return NULL;	/* per TR */
	}

	/* Se è un mob oppure è un pg che utilizza i comandi in inglese */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
		italian = TRUE;
	else
		italian = FALSE;		/* Di default tutti i mob usano i comandi in inglese */

	/* Se le lingue corrispondono chiama interpret normalmente */
	if ( (cmd_lang == IT && italian == TRUE)
	  || (cmd_lang != IT && italian == FALSE) )
	{
		/* Se è stata chiamata la send_command per tradurre il comando allora ritorna quello passato */
		if ( cmd_lang == TR )
		{
			return argument;
		}
		else
		{
			interpret( ch, argument );
			return NULL;
		}
	}

	/* Se le lingue non corrispondono bisogna convertire il comando passato nell'altra lingua */
	argument = one_argument( argument, arg );	/* Acquisisce solo il comando */

	/* Confronta arg con il corrispettivo della lingua di cmd_lang,
	 *	se uguale torna quella dell'altra lingua */

	/* Ricerca esatta nei comandi */
	for ( cmd = (cmd_lang == IT) ? first_command_it : first_command_en;  cmd;  cmd = cmd->next )
	{
		if ( !is_name(arg, cmd->name) )
			continue;

		/* ricava il comando dell'altra lingua */
		strcpy( arg_cmd, command_from_fun(cmd->fun->fun_name, cmd_lang == IT ? FALSE : TRUE) );
		if ( cmd_lang == TR )
			return arg_cmd;

		found = TRUE;
		break;
	}

	/* Ricerca di prefisso nei comandi */
	for ( cmd = (cmd_lang == IT) ? first_command_it : first_command_en;  cmd;  cmd = cmd->next )
	{
		if ( !is_name_prefix(arg, cmd->name) )
			continue;

		/* ricava il comando dell'altra lingua */
		strcpy( arg_cmd, command_from_fun(cmd->fun->fun_name, cmd_lang == IT ? FALSE : TRUE) );
		if ( cmd_lang == TR )
			return arg_cmd;

		found = TRUE;
		break;
	}

	/* Ricerca esatta nei sociali */
	for ( soc = first_social;  soc;  soc = soc->next )
	{
		if ( cmd_lang == IT && !is_name(arg, soc->italian) )	continue;
		if ( cmd_lang != IT && !is_name(arg, soc->english) )	continue;

		if ( cmd_lang == IT )
			one_argument( soc->english, arg_cmd );
		else
			one_argument( soc->italian, arg_cmd );

		if ( cmd_lang == TR )
			return arg_cmd;

		found = TRUE;
		break;
	}

	/* Ricerca di prefisso nei sociali */
	for ( soc = first_social;  soc;  soc = soc->next )
	{
		if ( cmd_lang == IT && !is_name_prefix(arg, soc->italian) )	continue;
		if ( cmd_lang != IT && !is_name_prefix(arg, soc->english) )	continue;

		if ( cmd_lang == IT )
			one_argument( soc->english, arg_cmd );
		else
			one_argument( soc->italian, arg_cmd );

		if ( cmd_lang == TR )
			return arg_cmd;

		found = TRUE;
		break;
	}

	/* Se lo ha trovato lo invia all'interpret */
	if ( found )
	{
		char txt[MIL*2];

		if ( VALID_STR(argument) )
			sprintf( txt, "%s %s", arg_cmd, argument );
		else
			strcpy( txt, arg_cmd );

		interpret( ch, txt );

		if ( cmd_lang == TR )
			return argument;
		else
			return NULL;
	}

	if ( cmd_lang == CO /*|| cmd_lang == TR*/ )	/* (FF) commentato fino a che non si traducono anche skills e spells */
		send_log( NULL, LOG_BUG, "send_command: comando con linguaggio %d non trovato: %s", cmd_lang, arg );

	if ( cmd_lang == TR )
		return arg;	/* per TR */
	else
		return NULL;
}

/*
 * Questa funzione richiama la send_command con tipo TR
 * Esiste perchè a volte si utilizza per sbaglio la send_command con CO al posto della TR
 *	creando loop al caso venga richiamata nella funzione di comando con lo stesso nome
 */
char *translate_command( CHAR_DATA *ch, char *argument )
{
	return send_command( ch, argument, TR );
}


/*
 * Salva su file l'ultimo comando inviato
 */
void save_lastcmd( void )
{
	MUD_FILE   *fp;
	char		file[MIL];

	sprintf( file, "lastcmd.%d", getpid() );
	fp = mud_fopen( "", file, "w", FALSE );
	if ( fp )
	{
		fputs( lastcmd_typed, fp->file );
		MUD_FCLOSE( fp );
	}
}


/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
	COMMAND_DATA   *cmd = NULL;
	char			command[MIL];
	char			logline[MIL];
	char			logname[MIL];
	int				loglvl;
	bool			found = FALSE;
	bool			ita;			/* lingua di comandi utilizzata dal pg */
	struct timeval	time_used;
	long			tmptime;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "interpret: ch è NULL, argomento passato: %s", argument );
		return;
	}

	if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "interpret: in_room è NULL per il ch %s, argomento passato: %s", ch->name, argument );
		return;
	}

	/* Se il contatore quit è maggiore di zero allora il pg sta annullando il quit, lo annulla ed esce */
	if ( IS_PG(ch) && ch->pg->counter_quit > 0 )
	{
		ch->pg->counter_quit = 0;
		return;
	}

	if ( IS_MOB(ch) )
		ita = FALSE;
	else
		ita = ( HAS_BIT_PLR(ch, PLAYER_ITALIAN) );

	if ( ch->substate == SUB_REPEAT_CMD )
	{
		DO_FUN *fun;

		fun = ch->last_cmd;
		if ( !fun )
		{
			ch->substate = SUB_NONE;
			send_log( NULL, LOG_BUG, "interpret: SUB_REPEAT_CMD with NULL last_cmd" );
			return;
		}

		for ( cmd = ita ? first_command_it : first_command_en;  cmd;  cmd = cmd->next )
		{
			if ( cmd->synonym )
				continue;

			if ( cmd->fun->do_fun == fun )
			{
				send_to_char( ch, ">>> Test: REPEATCMDS <<<" );
				found = TRUE;
				break;
			}
		}

		if ( !cmd )
		{
			send_log( NULL, LOG_BUG, "interpret: SUB_REPEAT_CMD: last_cmd invalid" );
			return;
		}

		one_argument( cmd->name, command );
		sprintf( logline, "(%s) %s", command, argument );
	}

	if ( !cmd )
	{
		/* Toglie gli spazi iniziali, serve sopratutto per i comandi inviati dai mudprog */
		while ( isspace(*argument) )
			argument++;

		strcpy( logline, argument );

		/* Cambiato l'ordine a questi if per prevenire eventuali crash */
		if ( !argument )
		{
			send_log( NULL, LOG_BUG, "interpret: argument è NULL!" );
			return;
		}

		/* Se il pg ha inviato e basta esce dalla funzione senza messaggio log */
		if ( argument[0] == '\0' )	/* Qui non ci và il testa VALID_STR, difatti bisogna che il test !argument sia sopra */
			return;

/*		REMOVE_BIT( ch->affected_by, AFFECT_HIDE );	*/		/* (FF) fa pensare in futuro riguardo ad alcune skill-spell */

		/*
		 * Implement freeze command.
		 */
		if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_FREEZE) )
		{
			send_to_char( ch, "You're totally frozen!\r\n" );
			return;
		}

		/*
		 * Grab the command word.
		 * Special parsing so ' can be a command, also no spaces needed after punctuation.
		 */
		if ( !isalpha(argument[0]) && !isdigit(argument[0]) && !is_accent(argument[0]) )	/* invio di accento */
		{
			command[0] = argument[0];	/* Comando monocarattere-simbolo */
			command[1] = '\0';

			argument++;
			while ( isspace(*argument) )
				argument++;
		}
		else
			argument = one_argument( argument, command );
	}

	/* Check for a timer delayed command (search, dig, detrap, etc) */
	if ( !check_delayed_command(ch) )
		return;

	/* Prima esegue una ricerca esatta nella lingua principale utilizzata da ch */
	if ( !found && (cmd = find_command(ch, command, TRUE, ita)) )		found = TRUE;
	if ( !found && check_skill(ch, command, argument, TRUE, ita) )		found = TRUE;	/* (FF) utilizzare una find_skill modificata similmente alla find_command per quel sistema di classi dinamico */
	if ( !found && check_alias(ch, command, argument, TRUE) )			found = TRUE;
	if ( !found && check_social(ch, command, argument, TRUE, ita) )		found = TRUE;

	/* Esegue poi una ricerca prefix nella lingua principale utilizzata da ch */
	if ( !found )
	{
		cmd = find_command( ch, command, FALSE, ita );
		if ( cmd )
		{
			found = TRUE;
			if ( HAS_BIT(cmd->fun->flags, CMDFLAG_TYPEALL) )	/* flag per i comandi che devono essere digitati per intero */
			{
				char	temp[MIL];

				one_argument( cmd->name, temp );
				ch_printf( ch, "Se vuoi che lo faccia devi dirmi per intero %s", temp );
				return;
			}
		}
	}
	if ( !found && check_skill(ch, command, argument, FALSE, ita) )		found = TRUE;
	if ( !found && check_alias(ch, command, argument, FALSE) )			found = TRUE;
	if ( !found && check_social(ch, command, argument, FALSE, ita) )	found = TRUE;

	/* Se il ch è un pg ed ha la doppialingua attiva riesegue le ricerche nell'altra lingua */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_DOUBLELANG) )
	{
		ita = !ita;

		if ( !found && (cmd = find_command(ch, command, TRUE, ita)) )		found = TRUE;
/*		if ( !found && check_skill(ch, command, argument, TRUE, ita) )		found = TRUE;	(FF) fino a quando le skill non vengono tradotte questo è inutile */
		if ( !found && check_social(ch, command, argument, TRUE, ita) )		found = TRUE;

		if ( !found && (cmd = find_command(ch, command, FALSE, ita)) )		found = TRUE;
/*		if ( !found && check_skill(ch, command, argument, FALSE, ita) )		found = TRUE;	(FF) fino a quando le skill non vengono tradotte questo è inutile */
		if ( !found && check_social(ch, command, argument, FALSE, ita) )	found = TRUE;
	}

	/*
	 * Log and snoop.
	 */
	if ( cmd && cmd->fun->log == LOG_NEVER )
		strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );

	/* Salva su file l'ultimo input inviato */
	sprintf( lastcmd_typed, "Data: %s  Room: #%d  Nome: %s  Cmd: %s",
		friendly_ctime(&current_time),
		ch->in_room->vnum,
		ch->name,
		logline );
	save_lastcmd( );

	loglvl = cmd ? cmd->fun->log : LOG_NORMAL;

	/*
	 * Write input line to watch files if a	licable
	 */
	if ( IS_PG(ch) && ch->desc && valid_watch(ch, logline) )
	{
		if ( cmd && HAS_BIT(cmd->fun->flags, CMDFLAG_WATCH) )
			write_watch_files( ch, cmd, logline );
		else if ( HAS_BIT_PLR(ch, PLAYER_WATCH) )
			write_watch_files( ch, NULL, logline );
	}

	/* (RR) da rivedere i vari LOG_ e loglvl */
	if ( (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_LOG))
	  || fLogAll
	  || loglvl == LOG_BUILD
	  || loglvl == LOG_ADMIN
	  || loglvl == LOG_ALWAYS )
	{
		/*
		 * Make it so a 'log all' will send most output to the log
		 *	file only, and not spam the log channel to death.
		 */
		if ( fLogAll && loglvl == LOG_NORMAL && (IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_LOG)) )
			loglvl = LOG_ALL;

		/* Added to show who is switched into a mob that executes a logged
		 *	command.  Check for descriptor in case force is used. */
		if ( ch->desc && ch->desc->original )
			send_log( NULL, loglvl, "%s (%s): %s", ch->name, ch->desc->original->name, logline );
		else
			send_log( NULL, loglvl, "%s: %s", ch->name, logline );

	}

	if ( ch->desc && ch->desc->snoop_by )
	{
		if ( !cmd || (!is_name(cmd->fun->fun_name, "do_tell do_reply do_gtell")
		  && cmd->fun->log != LOG_NEVER) )
		{
			sprintf( logname, "&Y%s%% %s&w\r\n", ch->name, logline );

			if ( ch->desc->snoop_by->character )
				send_to_char( ch->desc->snoop_by->character, logname );
			else
				send_to_char( ch->desc->snoop_by->original, logname );
		}
	}

	/* Se il comando è uno dei mprog e chi lo utilizza è un master o un builder switchato lo ferma */
	if ( cmd && cmd->fun->type == CMDTYPE_MUDPROG )
	{
		CHAR_DATA	*och;

		och = get_original( ch );
		if ( IS_PG(och) && och->pg->trust != TRUST_IMPLE )
		{
			send_to_char( ch, "Non puoi utilizzare i comandi di mudprog.\r\n" );
			return;
		}
	}

	/* Se ha trovato una skill o alias o social e non un comando, esce */
	if ( found && !cmd )
		return;

	/* Se non ha trovato nulla prova a cercare sulle keyword di uscita */
	if ( !found )
	{
		EXIT_DATA *pexit;

		/* check for an auto-matic exit command (FF) forse da togliere, gestire tutto con enter al limite */
		if ( (pexit = find_door(ch, command, TRUE)) != NULL
		  && HAS_BIT(pexit->flags, EXIT_xAUTO) )
		{
			if ( HAS_BIT(pexit->flags, EXIT_CLOSED)
			  && (!HAS_BIT(ch->affected_by, AFFECT_PASS_DOOR)
			  || HAS_BIT(pexit->flags, EXIT_NOPASSDOOR)) )
			{
				if ( !HAS_BIT(pexit->flags, EXIT_SECRET) )
					act( AT_PLAIN, "La $d è chiusa.", ch, NULL, pexit->keyword, TO_CHAR );
				else
					send_to_char( ch, "Non puoi fare questo qui.\r\n" );
				return;
			}
			move_char( ch, pexit, 0, FALSE );
			return;
		}

		/* Controlla che il comando inviato e non trovato sia dell'altro set di lingua dei comandi,
		 *	se sì avverte il giocatore che può cambiare lingua dei comandi oppure scegliere di utilizzare
		 *	tutte e due */
		if ( IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_DOUBLELANG) && (cmd = find_command(ch, command, TRUE, HAS_BIT_PLR(ch, PLAYER_ITALIAN) ? FALSE : TRUE)) )
		{
			ch_printf( ch, "Il comando che stai inviando è in &W%s&w, tu invece stai utilizzando il set\r\n",
				(HAS_BIT_PLR(ch, PLAYER_ITALIAN)) ?  "inglese"  :  "italiano" );
			ch_printf( ch, "di comandi in &W%s&w. Per saperne di più digita &Gaiuto comandi&w\r\n",
				(HAS_BIT_PLR(ch, PLAYER_ITALIAN)) ?  "italiano"  :  "inglese" );
		}
		else
		{
			send_to_char( ch, NOFOUND_MSG );

			/* Logga i comandi non andati a buon fine, per i mob bisogna risolverli che saranno al 99% degli errori
			 *	per i pg è utile per vedere quali social inviano o quali comandi i niubbi cercano */
			if ( IS_MOB(ch) )
			{
				send_log( NULL, LOG_CMDMOB, "interpret: mob #%d ha inviato senza successo: %s",
					ch->pIndexData->vnum, logline );
			}
			else
			{
				send_log( NULL, LOG_CMDPG, "interpret: pg %s ha inviato senza successo: %s",
					ch->name, logline );
			}
		}
		return;
	} /* chiude l'if */

	/*
	 * Character not in position for command?
	 */
	if ( !check_position(ch, cmd->fun->min_position) )
		return;

	/*
	 * E' un anima e non può inviare quel comando?
	 */
	if ( (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SPIRIT)) && HAS_BIT(cmd->fun->flags, CMDFLAG_NOSPIRIT) )
	{
		if ( IS_ADMIN(ch) )
			send_to_char( ch, "[ADM] Anche se sono spirito riesco ad eseguire l'azione.\r\n" );
		else
		{
			ch_printf( ch, "%s\r\n", cmd->fun->msg_nospirit );
			return;
		}
	}

	/* Casistiche in cui non si fugge dal combattimento */
	if ( (is_name(cmd->fun->fun_name, "do_flee")) )
	{
		if ( HAS_BIT(ch->affected_by, AFFECT_BERSERK) )
		{
			ch_printf( ch, "Fuggire ora che la mia furia berserk è al massimo? Ruargh!!\r\n" );
			return;
		}

		if ( IS_PG(ch) && ch->pg->clan && ch->pg->member && HAS_BIT(ch->pg->clan->flags, CLANFLAG_NOFLEE) )
		{
			ch_printf( ch, "L'onore del mio Clan non mi permette di fuggire.\r\n" );
			return;
		}

		/* (FF) aggiungere anche quando uno è ubriaco */
	}

	/* (FF) messaggi appositi nel file dei comandi */
	if ( HAS_BIT(ch->affected_by, AFFECT_POSSESS) && HAS_BIT(cmd->fun->flags, CMDFLAG_POSSESS) )
	{
		char temp[MIL];

		one_argument( cmd->name, temp );
		ch_printf( ch, "You can't %s while you are possessing someone!\r\n", temp );
	}
	else if ( ch->morph && HAS_BIT(cmd->fun->flags, CMDFLAG_POLYMORPH) )
	{
		char temp[MIL];

		one_argument( cmd->name, temp );
		ch_printf( ch, "You can't %s while you are polymorphed!\r\n", temp );
	}

	/*
	 * Dispatch the command.
	 */
	ch->prev_cmd = ch->last_cmd;		/* for automating */
	ch->last_cmd = cmd->fun->do_fun;
	start_timer( &time_used );
	( *cmd->fun->do_fun ) ( ch, argument );
	end_timer( &time_used );

	/*
	 * Update the record of how many times this command has been used
	 */
	update_userec( &time_used, &cmd->use_count );
	tmptime = UMIN( time_used.tv_sec, 19 ) * 1000000 + time_used.tv_usec;

	/* laggy command notice: command took longer than 1 second */
	if ( tmptime > 1000000 )
	{
		char	arg_cmd[MIL];

		one_argument( cmd->name, arg_cmd );
#ifdef TIMEFORMAT
		send_log( NULL, LOG_LAG, "[*****] LAG: %s: %s %s (R:%d S:%ld.%06ld)",
			ch->name,
			arg_cmd,
			(cmd->fun->log == LOG_NEVER)  ?  "XXX"  :  argument,
			ch->in_room  ?  ch->in_room->vnum  :  0,
			time_used.tv_sec,
			time_used.tv_usec );
#else
		send_log( NULL, LOG_LAG, "[*****] LAG: %s: %s %s (R:%d S:%d.%06d)",
			ch->name,
			arg_cmd,
			(cmd->fun->log == LOG_NEVER)  ?  "XXX"  :  argument,
			ch->in_room  ?  ch->in_room->vnum  :  0,
			time_used.tv_sec,
			time_used.tv_usec );
#endif
		cmd->lag_count++;		/* count the lag flags */
	}

	/* Cambia sempre il colore in plain */
	set_char_color( AT_PLAIN, ch );

	tail_chain( );
}


/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
	bool first = TRUE;

	if ( !VALID_STR(arg) )
		return FALSE;

	for ( ;  VALID_STR(arg);  arg++ )
	{
		if ( first && *arg == '-' )
		{
			first = FALSE;
			continue;
		}

		if ( !isdigit(*arg) )
			return FALSE;

		first = FALSE;
	}

	return TRUE;
}


/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
	char   *pdot;
	int		number;

	for ( pdot = argument;  VALID_STR(pdot);  pdot++ )
	{
		if ( *pdot == '.' )
		{
			*pdot = '\0';
			number = atoi( argument );
			*pdot = '.';
			strcpy( arg, pdot+1 );
			return number;
		}
	}

	strcpy( arg, argument );

	return 1;
}


DO_RET do_timecmd( CHAR_DATA *ch, char *argument )
{
	struct timeval	  start_time;
	struct timeval	  estime_time;
	static bool		  timing;
	extern CHAR_DATA *timechar;
	char			  arg[MIL];

	send_to_char( ch, "Timing\r\n" );

	if ( timing )
		return;

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "No command to time.\r\n" );
		return;
	}

	set_char_color( AT_PLAIN, ch );
	if ( is_name(arg, "aggiorna update") )
	{
		if ( timechar )
		{
			send_to_char( ch, "Another person is already timing updates.\r\n" );
		}
		else
		{
			timechar = ch;
			send_to_char( ch, "Setting up to record next update loop.\r\n" );
		}
		return;
	}

	send_to_char( ch, "Starting timer.\r\n" );
	timing = TRUE;
	gettimeofday( &start_time, NULL );
	/* Si suppone che ch abbia inviato argument nella sua lingua quindi niente send_command() */
	interpret( ch, argument );
	gettimeofday( &estime_time, NULL );
	timing = FALSE;
	set_char_color( AT_PLAIN, ch );
	send_to_char( ch, "Timing complete.\r\n" );
	subtract_times( &estime_time, &start_time );
	ch_printf( ch, "Timing took %ld.%6ld seconds.\r\n", estime_time.tv_sec, estime_time.tv_usec );
}


void start_timer( struct timeval *start_time )
{
	if ( !start_time )
	{
		send_log( NULL, LOG_BUG, "start_timer: NULL start_time." );
		return;
	}

	gettimeofday( start_time, NULL );
}


time_t end_timer( struct timeval *start_time )
{
	struct timeval estime_time;

	/* Mark estime_time before checking stime, so that we get a better reading.. */
	gettimeofday( &estime_time, NULL );
	if ( !start_time || (!start_time->tv_sec && !start_time->tv_usec) )
	{
		send_log( NULL, LOG_BUG, "end_timer: bad stime." );
		return 0;
	}
	subtract_times( &estime_time, start_time );
	/* stime becomes time used */
	*start_time = estime_time;
	return ( estime_time.tv_sec*1000000 ) + estime_time.tv_usec;
}


void send_timer( struct timerset *vtime, CHAR_DATA *ch )
{
	struct timeval ntime;
	int carry;

	if ( vtime->num_uses == 0 )
		return;

	ntime.tv_sec  =  vtime->total_time.tv_sec / vtime->num_uses;
	carry		  = (vtime->total_time.tv_sec % vtime->num_uses) * 1000000;
	ntime.tv_usec = (vtime->total_time.tv_usec + carry) / vtime->num_uses;
	ch_printf(ch, "Has been used %d times this boot.\r\n", vtime->num_uses);
	ch_printf(ch, "Time (in secs): min %ld.%6ld; avg: %ld.%6ld; max %ld.%6ld\r\n",
		vtime->min_time.tv_sec, vtime->min_time.tv_usec, ntime.tv_sec,
		ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec );
}


void update_userec( struct timeval *time_used, struct timerset *userec )
{
	userec->num_uses++;

	if ( !timerisset(&userec->min_time)
	  || timercmp(time_used, &userec->min_time, <) )
	{
		userec->min_time.tv_sec  = time_used->tv_sec;
		userec->min_time.tv_usec = time_used->tv_usec;
	}

	if ( !timerisset(&userec->max_time)
	  || timercmp(time_used, &userec->max_time, >) )
	{
		userec->max_time.tv_sec  = time_used->tv_sec;
		userec->max_time.tv_usec = time_used->tv_usec;
	}

	userec->total_time.tv_sec  += time_used->tv_sec;
	userec->total_time.tv_usec += time_used->tv_usec;

	while ( userec->total_time.tv_usec >= 1000000 )
	{
		userec->total_time.tv_sec++;
		userec->total_time.tv_usec -= 1000000;
	}
}
