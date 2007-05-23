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
 > 		Modulo riguardante caricamento e gestione dei dei sociali			<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "command.h"
#include "fread.h"
#include "interpret.h"
#include "mprog.h"


/*
 * Definizioni locali
 */
#define	MAX_SOCIAL	200		/* Numero massimo di social caricabili */


/*
 * Variabili esterne
 */
SOCIAL_DATA	*first_social	= NULL;
SOCIAL_DATA	*last_social	= NULL;
int			 top_social		= 0;


/*
 * Tipologie di messaggi social
 */
typedef enum
{
	SOCMSG_CHAR_NO_ARG,
	SOCMSG_CHAR_FOUND,
	SOCMSG_CHAR_AUTO,
	SOCMSG_VICT_FOUND,
	SOCMSG_OTHERS_NO_ARG,
	SOCMSG_OTHERS_FOUND,
	SOCMSG_OTHERS_AUTO,
	MAX_SOCMSG
} social_message_types;


/*
 * Elenco dei tipi di voce
 */
const char *table_voice[] =
{
	"nasale",
	"gutturale",
	"sforzata",
	"infantile",
	"chiara",
	"limpida",
	"argentina",
	"squillante",
	"velata",
	"roca",
	"cupa",
	"cavernosa",
	"sorda",
	"soave",
	"melodiosa",
	"pastosa",
	"morbida",
	"vellutata",
	"suadente",
	"secca",
	"aspra",
	"stizzosa",
	"stridula",
	"vibrante",
	"debole",
	"fioca",
	"flebile",
	"tenue",
	"sottile",
	"forte",
	"grossa",
	"tonante",
	"imperiosa",
	"autoritaria",
	"acuta",
	"alta",
	"bassa",
	"profonda",
	"piena",
	"soffocata",
	"sommessa",
	"bianca",
	"ferma",
	"incerta",
	"tremante",
	"implorante",
	"lamentosa",
	"querula",
	"commossa",
	"rotta",
	"ironica",
	"beffarda",
	"sarcastica",
	"espressiva",
	"inespressiva",
	"maschile",
	"femminile"
};


/*
 * Legge i messaggi dalle sottoetichette di un social
 * Le sottosezioni non supportano i commenti generalmente, quindi niente controllo su *
 */
void subfread_social_messages( SOCIAL_DATA *social, MUD_FILE *fp )
{
	SOCIALMSG_DATA *socmsg;
	char			races[MSL];

	CREATE( socmsg, SOCIALMSG_DATA, 1 );

	for ( ; ; )
	{
		char	*word;

		word = fread_word( fp );

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_social: fine prematura del file nella lettura" );
			break;
		}

		if		( !str_cmp(word, "CharNoArg"	) )		socmsg->char_no_arg =	fread_string( fp );
		else if ( !str_cmp(word, "CharFound"	) )		socmsg->char_found =	fread_string( fp );
		else if ( !str_cmp(word, "CharAuto"		) )		socmsg->char_auto =		fread_string( fp );
		else if ( !str_cmp(word, "EndMessages"	) )		break;
		else if ( !str_cmp(word, "OthersNoArg"	) )		socmsg->others_no_arg =	fread_string( fp );
		else if ( !str_cmp(word, "OthersFound"	) )		socmsg->others_found =	fread_string( fp );
		else if ( !str_cmp(word, "OthersAuto"	) )		socmsg->others_auto =	fread_string( fp );
#ifdef T2_ALFA
		else if ( !str_cmp(word, "OutRoom"		) )		socmsg->outroom =		fread_string( fp );
#endif
		else if ( !str_cmp(word, "Races"		) )		socmsg->races =			fread_code_bit( fp, CODE_RACE );
		else if ( !str_cmp(word, "VictFound"	) )		socmsg->vict_found =	fread_string( fp );
		else
			send_log( fp, LOG_FREAD, "subfread_social_messages: nessuna etichetta per la parola %s", word );
	}

	/* Se il bitvector della razza è vuoto e socmsg non è il primo dei messaggi di social è errore */
	if ( IS_EMPTY(socmsg->races) && social->first_socmsg )
	{
		send_log( fp, LOG_BUG, "subfread_social_messages: bitvector delle razze è NULL in un messaggio di social(%s) che non è il primo",
			social->italian );
		exit( EXIT_FAILURE );
	}

	/* Se il bitvector delle razze non è vuoto e socmsg è il primo dei messaggi di social è errore */
	if ( !IS_EMPTY(socmsg->races) && !social->first_socmsg )
	{
		send_log( fp, LOG_BUG, "subfread_social_messages: bitvector delle razze non è NULL per il primo messaggio di social(%s)",
			social->italian );
		exit( EXIT_FAILURE );
	}

	/* (FF) bisogna controllare che non vi siano più razze settate in diversi messaggi */

	if ( IS_EMPTY(socmsg->races) )
		strcpy( races, "Nessuna" );
	else
		strcpy( races, code_bit(NULL, socmsg->races, CODE_RACE) );

	if ( !VALID_STR(socmsg->char_no_arg) )
	{
		send_log( fp, LOG_FREAD, "subfread_messages_social: CharNoArg non trovato per il social %s con razza %s",
			social->italian, races );
/*		exit( EXIT_FAILURE ); */
	}

	if ( !VALID_STR(socmsg->others_no_arg) )
	{
		send_log( fp, LOG_FREAD, "subfread_messages_social: OthersNoArg non trovato per il social %s con la razza %s",
			social->italian, races );
/*		exit( EXIT_FAILURE ); */
	}

	if ( !VALID_STR(socmsg->char_found) )
	{
		send_log( fp, LOG_FREAD, "subfread_messages_social: CharFound non trovato per il social %s con la razza %s",
			social->italian, races );
/*		exit( EXIT_FAILURE ); */
	}

	if ( !VALID_STR(socmsg->others_found) )
	{
		send_log( fp, LOG_FREAD, "subfread_messages_social: OthersFound non trovato per il social %s con la razza %s",
			social->italian, races );
/*		exit( EXIT_FAILURE ); */
	}

	if ( !VALID_STR(socmsg->vict_found) )
	{
		send_log( fp, LOG_FREAD, "subfread_messages_social: VictFound non trovato per il social %s con la razza %s",
			social->italian, races );
/*		exit( EXIT_FAILURE ); */
	}

	/* I CharAuto e gli OthersAuto non sono obbligatori da inserire, però o devono esserci tutti e due, o nessuno dei due */
	if ( !VALID_STR(socmsg->char_auto) &&  VALID_STR(socmsg->others_auto)
	  &&  VALID_STR(socmsg->char_auto) && !VALID_STR(socmsg->others_auto) )
	{
		send_log( fp, LOG_FREAD, "subfread_messages_social: uno dei messaggi messaggi di Auto manca, o l'altro deve essere tolto per il social %s alla razza %s",
			social->italian, races );
/*		exit( EXIT_FAILURE ); */
	}

	LINK( socmsg, social->first_socmsg, social->last_socmsg, next, prev );
}


/*
 * Legge un social dal file
 */
void fread_social( MUD_FILE *fp )
{
	SOCIAL_DATA	*social;

	CREATE( social, SOCIAL_DATA, 1 );
	social->intention = -1;

	for ( ; ; )
	{
		char	*word;

		word = fread_word( fp );

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_social: fine prematura del file nella lettura" );
			break;
		}

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "English"		) )		social->english =		fread_string( fp );
		else if ( !str_cmp(word, "Expression"	) )		social->expression =	fread_string( fp );
		else if ( !str_cmp(word, "End"			) )		break;
		else if ( !str_cmp(word, "Italian"		) )		social->italian =		fread_string( fp );
		else if ( !str_cmp(word, "Intention"	) )		social->intention =		code_code( fp, fread_word(fp), CODE_INTENT );
		else if ( !str_cmp(word, "Messages"		) )								subfread_social_messages( social, fp );
		else if ( !str_cmp(word, "Smiles"		) )		social->smiles =		fread_string( fp );
		else
			send_log( fp, LOG_FREAD, "fread_social: nessuna etichetta per la parola %s", word );
	} /* chiude il for */

	/*
	 * Controlla il social letto
	 */

	if ( !VALID_STR(social->italian) )
	{
		send_log( fp, LOG_FREAD, "fread_social: italian non trovato" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(social->english) )
	{
		send_log( fp, LOG_FREAD, "fread_social: english non trovato per il social %s", social->italian );
		exit( EXIT_FAILURE );
	}

	if ( social->intention < 0 || social->intention >= MAX_INTENT )
	{
		send_log( fp, LOG_FREAD, "fread_social: intention non trovato o errato: %d", social->intention );
		exit( EXIT_FAILURE );
	}

	/* Se no ntrova almeno un sistema di messaggio social, quello di default, è errore */
	if ( !social->first_socmsg )
	{
		send_log( fp, LOG_FREAD, "fread_social: non esiste neppure un gruppo di messaggi al social %s", social->italian );
		exit( EXIT_FAILURE );
	}

	/* Se c'è una stringa tra expressione e smile, anche l'altra deve esserci */
	if ( ( VALID_STR(social->expression) && !VALID_STR(social->smiles))
	  || (!VALID_STR(social->expression) &&  VALID_STR(social->smiles)) )
	{
		send_log( fp, LOG_BUG, "fread_social: stringa expression valida, ma non quella smiles (oppure viceversa) al social %s",
			social->italian );
		exit( EXIT_FAILURE );
	}

	LINK( social, first_social, last_social, next, prev );
	top_social++;
}


/*
 * Carica e controlla i Sociali
 */
void load_socials( void )
{
	SOCIAL_DATA *social;

	fread_section( TABLES_DIR "socials.dat", "SOCIAL", fread_social, FALSE );

	/* Ora che li ha caricati tutti li controlla */
	if ( top_social >= MAX_SOCIAL )
	{
		send_log( NULL, LOG_LOAD, "check_last_social: numero di sociali caricati troppo alto: %d",
			top_social );
		exit( EXIT_FAILURE );
	}

	for ( social = first_social;  social;  social = social->next )
	{
		char	*soc_italian;
		char	*soc_english;
		char	 name_italian[MIL];
		char	 name_english[MIL];

		soc_italian = social->italian;
		soc_english = social->english;

		while ( VALID_STR(soc_italian) || VALID_STR(soc_english) )
		{
			soc_italian = one_argument( soc_italian, name_italian );
			soc_english = one_argument( soc_english, name_english );

			if ( VALID_STR(soc_italian) )
			{
				if ( find_command(NULL, name_italian, TRUE, FALSE) )
					send_log( NULL, LOG_WARNING, "load_socials: sociale %s conflitta con un nome di comando", name_italian );

				if ( find_skill(NULL, name_italian, TRUE, FALSE, 0, top_sn) )
					send_log( NULL, LOG_WARNING, "load_socials: sociale %s conflitta con un nome di skill", name_italian );
			}

			if ( VALID_STR(soc_english) )
			{
				if ( find_command(NULL, name_english, TRUE, FALSE) )
					send_log( NULL, LOG_WARNING, "load_socials: sociale %s conflitta con un nome di comando", name_english );

				if ( find_skill(NULL, name_english, TRUE, FALSE, 0, top_sn) )
					send_log( NULL, LOG_WARNING, "load_socials: sociale %s conflitta con un nome di skill", name_english );
			}
		}
	}

	/* (FF) da fare l'ordine alfabetico (o forse è meglio un sistema tipo quello dei comandi? anche se qui non c'è una funzione a cui puntare..) */
}


/*
 * Libera dalla memoria tutti i social
 */
void free_all_socials( void )
{
	SOCIAL_DATA *social;

	for ( social = first_social;  social;  social = first_social )
	{
		SOCIALMSG_DATA *socmsg;

		UNLINK( social, first_social, last_social, next, prev );
		top_social--;

		DISPOSE( social->italian );
		DISPOSE( social->english );
		for ( socmsg = social->first_socmsg;  socmsg;  socmsg = social->first_socmsg )
		{
			UNLINK( socmsg, social->first_socmsg, social->last_socmsg, next, prev );

			destroybv( socmsg->races );
			DISPOSE( socmsg->char_no_arg );
			DISPOSE( socmsg->char_found );
			DISPOSE( socmsg->char_auto );
			DISPOSE( socmsg->vict_found );
			DISPOSE( socmsg->others_no_arg );
			DISPOSE( socmsg->others_found );
			DISPOSE( socmsg->others_auto );
#ifdef T2_ALFA
			DISPOSE( socmsg->outroom );
#endif
		}
#ifdef T2_ALFA
		destroybv( social->flags );
#endif
		DISPOSE( social->smiles );
		DISPOSE( social->expression );
		DISPOSE( social );
	}

	if ( top_social != 0 )
		send_log( NULL, LOG_BUG, "free_all_socials: top_social non è 0 dopo aver liberato i social: %d", top_social );
}


/*
 * Cerca un social nell'array dei social
 */
SOCIAL_DATA *find_social( CHAR_DATA *ch, const char *command, bool exact, bool ita )
{
	SOCIAL_DATA *soc;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "find_social: ch è NULL" );
		return NULL;
	}

	if ( !VALID_STR(command) )
	{
		send_log( NULL, LOG_BUG, "find_social: argomento command passato non è valido" );
		return NULL;
	}

	ch = get_original( ch );
	if ( IS_MOB(ch) && (ita == TRUE || ita == ALL) )
	{
		send_log( NULL, LOG_BUG, "find_social: ita passato a %s per il mob %s #%d",
			ita ? "TRUE" : "ALL", ch->short_descr, ch->pIndexData->vnum );
		ita = FALSE;
	}

	/* Cerca in maniera esatta il social nella lingua del pg */
	if ( exact == TRUE || exact == ALL )
	{
		if ( ch && (ita == TRUE || ita == ALL) )
		{
			for ( soc = first_social;  soc;  soc = soc->next )
			{
				if ( is_name(command, soc->italian) )
					return soc;
			}
		}

		if ( ita == FALSE || ita == ALL )
		{
			for ( soc = first_social;  soc;  soc = soc->next )
			{
				if ( is_name(command, soc->english) )
					return soc;
			}
		}
	}

	/* Cerca il prefisso del social nella lingua del pg */
	if ( ch && (exact == FALSE || exact == ALL) )
	{
		if ( ita == TRUE || ita == ALL )
		{
			for ( soc = first_social;  soc;  soc = soc->next )
			{
				if ( is_name_prefix(command, soc->italian) )
					return soc;
			}
		}

		if ( ita == FALSE || ita == ALL )
		{
			for ( soc = first_social;  soc;  soc = soc->next )
			{
				if ( is_name_prefix(command, soc->english) )
					return soc;
			}
		}
	}

	return NULL;
}


/*
 * Passatogli un numero ritorna il social corrispondente nella lista
 */
SOCIAL_DATA *get_social_num( const int number )
{
	SOCIAL_DATA *social;
	int			 count = 0;

	if ( number < 0 || number >= top_social )
	{
		send_log( NULL, LOG_BUG, "get_social_num: numero passato errato: %d", number );
		return NULL;
	}

	for ( social = first_social;  social;  social = social->next )
	{
		if ( count == number )
			return social;
		count++;
	}

	return NULL;
}


/*
 * Ritorna il tipo di messaggio voluto sulla base della razza di ch
 */
char *get_socmsg( CHAR_DATA *ch, SOCIAL_DATA *social, const int type )
{
	SOCIALMSG_DATA *socmsg;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_socmsg: ch passato è NULL" );
		return "";
	}

	if ( !social )
	{
		send_log( NULL, LOG_BUG, "get_socmsg: social passato è NULL" );
		return "";
	}

	if ( type < 0 || type >= MAX_SOCMSG )
	{
		send_log( NULL, LOG_BUG, "get_socmsg: type passato è errato: %d", type );
		return "";
	}

	for ( socmsg = social->first_socmsg;  socmsg;  socmsg = socmsg->next )
	{
		if ( HAS_BIT(socmsg->races, ch->race) )
		{
			switch ( type )
			{
			  default:
				send_log( NULL, LOG_BUG, "get_socmsg: tipo passato errato o mancante: %d", type );
				return "";
			  case SOCMSG_CHAR_NO_ARG:	 return socmsg->char_no_arg;
			  case SOCMSG_CHAR_FOUND:	 return socmsg->char_found;
			  case SOCMSG_CHAR_AUTO:	 return socmsg->char_auto;
			  case SOCMSG_VICT_FOUND:	 return socmsg->vict_found;
			  case SOCMSG_OTHERS_NO_ARG: return socmsg->others_no_arg;
			  case SOCMSG_OTHERS_FOUND:	 return socmsg->others_found;
			  case SOCMSG_OTHERS_AUTO:	 return socmsg->others_auto;
			}
		}	
	}

	/* Se non ha trovato nessuna tipologia di messaggi per la razza di ch  allora invia quella di default */
	switch ( type )
	{
	  default:
		send_log( NULL, LOG_BUG, "get_socmsg: tipo passato errato o mancante: %d", type );
		return "";
	  case SOCMSG_CHAR_NO_ARG:	 return social->first_socmsg->char_no_arg;
	  case SOCMSG_CHAR_FOUND:	 return social->first_socmsg->char_found;
	  case SOCMSG_CHAR_AUTO:	 return social->first_socmsg->char_auto;
	  case SOCMSG_VICT_FOUND:	 return social->first_socmsg->vict_found;
	  case SOCMSG_OTHERS_NO_ARG: return social->first_socmsg->others_no_arg;
	  case SOCMSG_OTHERS_FOUND:	 return social->first_socmsg->others_found;
	  case SOCMSG_OTHERS_AUTO:	 return social->first_socmsg->others_auto;
	}
}


/*
 * Controlla se l'argomento passato è un social e lo invia
 */
bool check_social( CHAR_DATA *ch, const char *command, char *argument, bool exact, bool ita )
{
	CHAR_DATA	*victim;
	SOCIAL_DATA	*social;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_social: ch è NULL" );
		return FALSE;
	}

	if ( !VALID_STR(command) )
	{
		send_log( NULL, LOG_BUG, "check_social: command non valido" );
		return FALSE;
	}

	social = find_social( ch, command, exact, ita );
	if ( !social )
		return FALSE;

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_NOSOCIAL) )
	{
		send_to_char( ch, "Non riesco ad esprimere le mie emozioni!\r\n" );
		return TRUE;
	}

	switch ( ch->position )
	{
	  case POSITION_DEAD:
		send_to_char( ch, "Mi è difficile: sono morto!\r\n" );
		return TRUE;

	  case POSITION_INCAP:
	  case POSITION_MORTAL:
		send_to_char( ch, "Sono troppo ferito per poterlo fare.\r\n" );
		return TRUE;

	  case POSITION_STUN:
		send_to_char( ch, "Sono troppo stordito per poterlo fare.\r\n" );
		return TRUE;

	  case POSITION_SLEEP:
		if ( is_name(social->italian, "russa snore") )
			break;
		send_to_char( ch, "Nei miei sogni o cosa?\r\n" );
		return TRUE;
	} /* chiude lo switch */

	social->use_count++;

	victim = NULL;
	if ( !VALID_STR(argument) )
	{
		act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_OTHERS_NO_ARG), ch, NULL, victim, TO_ROOM );
		act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_CHAR_NO_ARG), ch, NULL, victim, TO_CHAR );
	}
	else if ( (victim = get_char_room(ch, argument, TRUE)) == NULL )
	{
		send_to_char( ch, "Non lo trovo qui intorno.\r\n" );
	}
	else if ( victim == ch )
	{
		char	*msg_auto;

		/* Se il social ha il msg auto lo utilizza, sennò una il noarg che fa da auto in questi casi */

		msg_auto = get_socmsg( ch, social, SOCMSG_OTHERS_AUTO );
		if ( VALID_STR(msg_auto) )
			act( AT_SOCIAL, msg_auto, ch, NULL, victim, TO_ROOM );
		else
			act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_OTHERS_NO_ARG), ch, NULL, victim, TO_ROOM );

		msg_auto = get_socmsg( ch, social, SOCMSG_CHAR_AUTO );
		if ( VALID_STR(msg_auto) )
			act( AT_SOCIAL, msg_auto, ch, NULL, victim, TO_CHAR );
		else
			act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_CHAR_NO_ARG), ch, NULL, victim, TO_CHAR );
	}
	else
	{
		act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_OTHERS_FOUND), ch, NULL, victim, TO_NOVICT );
		act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_CHAR_FOUND), ch, NULL, victim, TO_CHAR );
		act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_VICT_FOUND), ch, NULL, victim, TO_VICT );

		if ( IS_PG(ch) && IS_MOB(victim)
		  && !HAS_BIT(victim->affected_by, AFFECT_CHARM)
		  && is_awake(victim)
		  && !has_trigger(victim->pIndexData->first_mudprog, MPTRIGGER_ACT)
		  && (!victim->desc || !victim->desc->original) )	/* (TT) se il mob è switchato non usa gli emote automatici */
		{
			int		mod = 0;	/* modificatore comportamentale */

			if ( is_align(ch, ALIGN_GOOD) )
			{
				if		( is_align(victim, ALIGN_GOOD) )		mod += 5;
				else if ( is_align(victim, ALIGN_EVIL) )		mod -= 5;
			}
			else if ( is_align(ch, ALIGN_NEUTRAL) )
			{
				if		( is_align(victim, ALIGN_GOOD) )		mod += 2;
				else if ( is_align(victim, ALIGN_NEUTRAL) )		mod += 5;
				else if ( is_align(victim, ALIGN_EVIL) )		mod -= 2;
			}
			else
			{
				if		( is_align(victim, ALIGN_GOOD) )		mod -= 5;
				else if ( is_align(victim, ALIGN_EVIL) )		mod += 5;
			}

			if		( social->intention == INTENT_FRIENDLY )	mod += 5;
			else if ( social->intention == INTENT_AGGRESSIVE )	mod -= 5;

			switch ( number_range(0, 8) )
			{
			  /* caso negativo: il mob risponde in malo modo */
			  case 0:
				if ( mod == -10 && !is_safe(victim, ch, TRUE) )
					multi_hit( victim, ch, TYPE_UNDEFINED );
				else if ( mod < -5 )
				{
					act( AT_ACTION, "$n schiaffeggia $N.",  victim, NULL, ch, TO_NOVICT );
					act( AT_ACTION, "$n mi schiaffeggia.", victim, NULL, ch, TO_VICT );
				}
				else
				{
					act( AT_ACTION, "$n si comporta come se $N non esistesse.", victim, NULL, ch, TO_NOVICT );
					act( AT_ACTION, "$n sembra ignorarmi.", victim, NULL, ch, TO_VICT );
				}
				break;

			  /* caso uguale: il mob risponde con lo stesso social */
			  case 1: case  2: case  3: case  4: case 5:
				act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_OTHERS_FOUND), victim, NULL, ch, TO_NOVICT );
				act( AT_SOCIAL, get_socmsg(ch, social, SOCMSG_VICT_FOUND), victim, NULL, ch, TO_VICT );
				break;

			  /* caso positivo: il mob risponde con lo stesso tipo di intention, solo se non neutral */
			  case  6: case  7: case  8:
			  {
				SOCIAL_DATA	*soc_reply;

				/* Se il modificatore è neutrale non fa nulla */
				if ( mod >= -7 && mod <= 7 )
				{
					act( AT_ACTION, "$n si comporta come se $N non esistesse.", victim, NULL, ch, TO_NOVICT );
					act( AT_ACTION, "$n sembra ignorarmi.", victim, NULL, ch, TO_VICT );
					break;
				}

				for ( ; ; )
				{
					soc_reply = get_social_num( number_range(0, top_social-1) );
					if ( (soc_reply->intention == INTENT_FRIENDLY && mod > 7)
					  || (soc_reply->intention == INTENT_AGGRESSIVE && mod < -7) )
					{
						act( AT_SOCIAL, get_socmsg(ch, soc_reply, SOCMSG_OTHERS_FOUND), victim, NULL, ch, TO_NOVICT );
						act( AT_SOCIAL, get_socmsg(ch, soc_reply, SOCMSG_VICT_FOUND), victim, NULL, ch, TO_VICT );
						break;
					}
				}
				break;
			  }
			}
		} /* chiude l'if */
	} /* chiude l'else */

	return TRUE;
}


/*
 * Visualizza tutti i sociali
 */
DO_RET do_socials( CHAR_DATA *ch, char *argument )
{
	SOCIAL_DATA *soc;
	char		 soc_tr[MIL];
	char		 letter;
	bool		 ita;
	bool		 found;
	int			 col = 0;

	set_char_color( AT_PLAIN, ch );

	/* Visualizza l'help e la sintassi del comando se richiamata */
	pager_printf( ch, "&YSintassi&w:  %s\r\n",						soc_tr );
	pager_printf( ch, "           %s inglese\r\n",					soc_tr );
	pager_printf( ch, "           %s italiano\r\n",					soc_tr );
	pager_printf( ch, "           %s traduzione\r\n",				soc_tr );
	pager_printf( ch, "           %s faccine\r\n",					soc_tr );
	pager_printf( ch, "           %s <sociale cercato>\r\n\r\n",	soc_tr );


	/* Questa macro serve per l'ordinamento alfabetico */
#	define STRCMP_1ST_LETTER( struct )			\
	if ( (ita) == TRUE )						\
	{											\
		if ( (struct)->italian[0] != letter )	\
			continue;							\
	}											\
	else										\
	{											\
		if ( (struct)->english[0] != letter )	\
			continue;							\
	}


	/* Se il pg usa i comandi in italiano allora richiama quelli */
	if ( (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN)) )
		ita = TRUE;
	else
		ita = FALSE;

	if ( is_name_prefix(argument, "italiano italian") )	/* Se chiede di vedere i social in italiano */
		ita = TRUE;
	if ( is_name_prefix(argument, "inglese english") )		/* Se chiede di vedere i sociali in inglese */
		ita = FALSE;

	/* Visualizza la lista dei sociali in italiano o in inglese. */
	if ( !VALID_STR(argument) || is_name_prefix(argument, "italiano italian inglese english") )
	{
		pager_printf( ch, "Lista dei sociali in %s:\r\n\r\n", (ita)  ?  "italiano"  :  "inglese" );

		for ( letter = 'a'; letter <= 'z'; letter++ )
		{
			int x = 0;

			for ( soc = first_social;  soc;  soc = soc->next, x++ )
			{
				char	arg[MIL];

				STRCMP_1ST_LETTER( soc );

				one_argument( (ita) ? soc->italian : soc->english, arg );
				pager_printf( ch, "%-16s", arg );
				if ( (++col % 5) == 0 )
					send_to_pager( ch, "\r\n" );
				if ( (col % 5) == 0 && x == top_social-1 )
					send_to_pager( ch, "\r\n" );
			}
		}
		return;
	}

	/* Visualizza i sociali traducendoli dalla lingua che si sta utilizzando */
	if ( is_name_prefix(argument, "traduzione translation") )
	{
		pager_printf( ch, "Lista dei sociali tradotti dall'%s all'%s:\r\n\r\n",
			(ita)  ?  "italiano"  :  "inglese",
			(ita)  ?  "inglese"   :  "italiano" );

		for ( letter = 'a'; letter <= 'z'; letter++ )
		{
			int x = 0;

			for ( soc = first_social;  soc;  soc = soc->next, x++ )
			{
				char	arg_lang1[MIL];
				char	arg_lang2[MIL];

				STRCMP_1ST_LETTER( soc );

				one_argument( (ita) ? soc->italian : soc->english, arg_lang1 );
				one_argument( (ita) ? soc->english : soc->italian, arg_lang2 );
				pager_printf( ch, "%-16s -> %-16s",
					arg_lang1,
					arg_lang2 );
				if ( (++col % 2) == 0 )
					send_to_pager( ch, "\r\n" );
				if ( (col % 2) == 0 && x == top_social-1 )
					send_to_pager( ch, "\r\n" );
			}
		}
		return;
	}

	/* Visualizza i sociali traducendoli dalla lingua che si sta utilizzando */
	if ( is_name_prefix(argument, "faccine smiles") )
	{
		send_to_pager( ch, "Lista dei sociali che posseggono una faccina:\r\n\r\n" );

		for ( letter = 'a'; letter <= 'z'; letter++ )
		{
			int x = 0;

			for ( soc = first_social;  soc;  soc = soc->next, x++ )
			{
				STRCMP_1ST_LETTER( soc );

				if ( soc->smiles )
				{
					char	arg_social[MIL];
					char	arg_smile [MIL];

					one_argument( (ita) ? soc->italian : soc->english, arg_social );
					one_argument( soc->smiles, arg_smile );
					pager_printf( ch, "%-16s %-4s  ", arg_social, arg_smile );
					if ( (++col % 2) == 0 )
						send_to_pager( ch, "\r\n" );
					if ( (col % 2) == 0 && x == top_social-1 )
						send_to_pager( ch, "\r\n" );
				}
			}
		}
		return;
	}

	found = FALSE;
	for ( letter = 'a'; letter <= 'z'; letter++ )
	{
		int x = 0;

		for ( soc = first_social;  soc;  soc = soc->next, x++ )
		{
			STRCMP_1ST_LETTER( soc );

			if ( !str_prefix(argument, (ita)  ?  soc->italian  :  soc->english) )
			{
				char	arg[MIL];

				one_argument( (ita) ? soc->italian : soc->english, arg );
				pager_printf( ch, "%-16s", arg );
				found = TRUE;
				if ( (++col % 5) == 0 )
					send_to_pager( ch, "\r\n" );
				if ( (col % 5) == 0 && x == top_social-1 )
					send_to_pager( ch, "\r\n" );
			}
		}
	}

	if ( found == FALSE )
		ch_printf( ch, "Sociale non trovato con argomento %s.\r\n", argument );
}


/*
 * Comando Admin per visualizzare un social
 */
DO_RET do_showsocial( CHAR_DATA *ch, char *argument )
{
	SOCIAL_DATA		*social;
	SOCIALMSG_DATA	*socmsg;

	set_char_color( AT_SOCIAL, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Indica quale sociale vorresti vedere.\r\n" );
		return;
	}

	social = find_social( ch, argument, FALSE, ALL );
	if ( !social )
	{
		send_to_char( ch, "Sociale non trovato.\r\n" );
		return;
	}

	ch_printf( ch, "Italian     %s\r\n", social->italian );
	ch_printf( ch, "English     %s\r\n", social->english );
	ch_printf( ch, "Intention   %s\r\n", code_name(NULL, social->intention, CODE_INTENT) );
	ch_printf( ch, "Smile       %s\r\n", (VALID_STR(social->smiles)) ? social->smiles : "(non settato)" );
	ch_printf( ch, "Expression  %s\r\n\r\n", (VALID_STR(social->expression)) ? social->expression : "(non settato)" );

	for ( socmsg = social->first_socmsg;  socmsg;  socmsg = socmsg->next )
	{
		if ( socmsg == social->first_socmsg )
			send_to_char( ch, "Messaggi di defaul:\r\n" );
		else
			ch_printf( ch, "\r\nMessaggi inviati dalle razze %s:\r\n", code_bit(NULL, socmsg->races, CODE_RACE) );
		ch_printf( ch, "CharNoArg   %s\r\n", socmsg->char_no_arg );
		ch_printf( ch, "CharFound   %s\r\n", socmsg->char_found );
		ch_printf( ch, "CharAuto    %s\r\n", (VALID_STR(socmsg->char_auto)) ? socmsg->char_auto : "(non settato)" );
		ch_printf( ch, "VictimFound %s\r\n", socmsg->vict_found );
		ch_printf( ch, "OtherNoArg  %s\r\n", socmsg->others_no_arg );
		ch_printf( ch, "OtherFound  %s\r\n", socmsg->others_found );
		ch_printf( ch, "OtherAuto   %s\r\n", (VALID_STR(socmsg->others_auto)) ? socmsg->others_auto : "(non settato)" );
	}
}


/*
 * Comando Admin che visualizza gli utilizzi dei social
 */
DO_RET do_soctable( CHAR_DATA *ch, char *argument )
{
	SOCIAL_DATA *soc;
	int			 column = 0;

	set_char_color( AT_ADMIN, ch );
	pager_printf( ch, "%-12.12s %5s  %-12.12s %5s  %-12.12s %5s %-12.12s %5s\r\n",
		"Social", "Uso",
		"Social", "Uso",
		"Social", "Uso",
		"Social", "Uso" );

	set_char_color( AT_PLAIN, ch );
	for ( soc = first_social;  soc;  soc = soc->next )
	{
		char arg_cmd[MIL];

		/* carpisce solo il primo nome del comando nella lingua del pg */
		if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
			one_argument( soc->italian, arg_cmd );
		else
			one_argument( soc->english, arg_cmd );

		pager_printf( ch,"%-12.12s %5d",
			arg_cmd,
			soc->use_count );

		if ( ++column % 4 )
			send_to_pager( ch, "  " );
		else
			send_to_pager( ch, "\r\n" );
	}

	send_to_char( ch, "\r\n" );
}
