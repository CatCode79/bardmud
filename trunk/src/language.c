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
 > 						Modulo riguardante le lingue						<
\****************************************************************************/

#include <ctype.h>

#include "mud.h"
#include "economy.h"
#include "command.h"
#include "fread.h"
#include "interpret.h"
#include "room.h"
#include "save.h"


/*
 * Variabili globali
 */
LANG_DATA  *table_language[MAX_LANGUAGE];	/* (FF) da convertire in lista? penso di sì */


/*
 * Variabili locali
 */
int		top_language = 0;


/*
 * Definizioni locali
 */
#define LANGUAGE_FILE		"../tables/languages.dat"
#define LANGUAGE_FILE_T2	"../tables/languages.t2.dat"


/*
 * Tabella dei codici delle lingue
 */
const CODE_DATA code_language[] =
{
	{ LANGUAGE_COMMON,		"LANGUAGE_COMMON",		"comune"			},
	{ LANGUAGE_ELVEN,		"LANGUAGE_ELVEN",		"elfico"			},
	{ LANGUAGE_DWARVEN,		"LANGUAGE_DWARVEN",		"nanesco"			},
	{ LANGUAGE_PIXIE,		"LANGUAGE_PIXIE",		"pixie"				},
	{ LANGUAGE_OGRE,		"LANGUAGE_OGRE",		"ogrese"			},
	{ LANGUAGE_ORCISH,		"LANGUAGE_ORCISH",		"orchesco"			},
	{ LANGUAGE_TROLLISH,	"LANGUAGE_TROLLISH",	"trollese"			},
	{ LANGUAGE_RODENT,		"LANGUAGE_RODENT",		"roditore"			},
	{ LANGUAGE_INSECTOID,	"LANGUAGE_INSECTOID",	"insetto"			},
	{ LANGUAGE_MAMMAL,		"LANGUAGE_MAMMAL",		"mammifero"			},
	{ LANGUAGE_REPTILE,		"LANGUAGE_REPTILE",		"rettile"			},
	{ LANGUAGE_DRAGON,		"LANGUAGE_DRAGON",		"draconiano"		},
	{ LANGUAGE_DEATH,		"LANGUAGE_DEATH",		"linguamortis"		},
	{ LANGUAGE_MAGICAL,		"LANGUAGE_MAGICAL",		"magico"			},
	{ LANGUAGE_GOBLIN,		"LANGUAGE_GOBLIN",		"goblinese"			},
	{ LANGUAGE_ANCIENT,		"LANGUAGE_ANCIENT",		"antico"			},
	{ LANGUAGE_HALFLING,	"LANGUAGE_HALFLING",	"halfling"			},
	{ LANGUAGE_CLAN,		"LANGUAGE_CLAN",		"clan"				},
	{ LANGUAGE_GITH,		"LANGUAGE_GITH",		"gith"				},
	{ LANGUAGE_GNOME,		"LANGUAGE_GNOME",		"gnomese"			},
	{ LANGUAGE_DEMON,		"LANGUAGE_DEMON",		"demoniaco"			},
	{ LANGUAGE_GNOLLISH,	"LANGUAGE_GNOLLISH",	"gnoll"				},
	{ LANGUAGE_DROW,		"LANGUAGE_DROW",		"drow"				},
	{ LANGUAGE_CANINE,		"LANGUAGE_CANINE",		"canino"			},
	{ LANGUAGE_FELINE,		"LANGUAGE_FELINE",		"felino"			},
	{ LANGUAGE_AVIS,		"LANGUAGE_AVIS",		"volatile"			},
	{ LANGUAGE_MARINE,		"LANGUAGE_MARINE",		"marino"			},
	{ LANGUAGE_TREELEAF,	"LANGUAGE_TREELEAF",	"alberofoglia"		},
	{ LANGUAGE_MONSTER,		"LANGUAGE_MONSTER",		"lingua mostruosa"	}
};
const int max_check_language = sizeof(code_language)/sizeof(CODE_DATA);


/*
 * Languages loading functions
 */
void fread_cnv( MUD_FILE *fp, LANG_CNV_DATA **first_cnv, LANG_CNV_DATA **last_cnv )
{
	for ( ; ; )
	{
		LANG_CNV_DATA *cnv;
		char	   letter;

		letter = fread_letter( fp );

		if ( letter == EOF )
		{
			send_log( fp, LOG_FREAD, "fread_cnv: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		/* Commento, salta la linea */
		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		/* Fine della lettura della lista di conversione */
		if ( letter == '~' )
			break;

		ungetc( letter, fp->file );

		CREATE( cnv, LANG_CNV_DATA, 1 );
		cnv->old  = str_dup( fread_word(fp) );
		cnv->olen = strlen( cnv->old );
		cnv->new  = str_dup( fread_word(fp) );
		cnv->nlen = strlen( cnv->new );

		fread_to_eol( fp );

		LINK( cnv, *first_cnv, *last_cnv, next, prev );
	}
}


void fread_language( MUD_FILE *fp )
{
	LANG_DATA *lng;
	char	  *word;
	int		   lang_number = -1;

	CREATE( lng, LANG_DATA, 1 );

	for ( ; ; )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_language: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Name"		 ) )	lng->name	=			fread_string( fp );
		else if ( !str_cmp(word, "LangNumber") )	lang_number	=			code_code( fp, str_dup(fread_word(fp)), CODE_LANGUAGE );
		else if ( !str_cmp(word, "Valid"	 ) )	lng->valid	=			fread_bool( fp );
		else if ( !str_cmp(word, "PreCnvList") )	fread_cnv( fp, &lng->first_precnv, &lng->last_precnv );
		else if ( !str_cmp(word, "Alphabet"	 ) )	lng->alphabet =			fread_string( fp );
		else if ( !str_cmp(word, "CnvList"	 ) )	fread_cnv( fp, &lng->first_cnv, &lng->last_cnv );
		else if ( !str_cmp(word, "End"		 ) )	break;
		else
			send_log( fp, LOG_FREAD, "fread_language: nessuna etichetta per la parola %s.", word );
	} /* chiude il for */

	/*
	 * Finito di caricare il linguaggio lo controlla
	 */

	if ( !VALID_STR(lng->name) )
	{
		send_log( fp, LOG_FREAD, "fread_language: nome di linguaggio inesistente" );
		exit( EXIT_FAILURE );
	}

	if ( lang_number < 0 || lang_number >= MAX_RACE )
	{
		send_log( fp, LOG_BUG, "fread_language: numero della lingua errato: %d", lang_number );
		exit( EXIT_FAILURE );
	}

	table_language[lang_number] = lng;
	top_language++;
}


void load_languages( void )
{
	int x;

	fread_section( LANGUAGE_FILE,	 "LANGUAGE", fread_language, FALSE );

	/* Controlla globale delle lingue */

/*	if ( top_language != MAX_LANGUAGE )
	{
		send_log( NULL, LOG_LOAD, "load_languages: numero di lingue caricate diverso da MAX_LANGUAGE(%d): %d",
			MAX_LANGUAGE, top_language );
		exit( EXIT_FAILURE );
	}*/

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		if ( table_language[x] == NULL )
		{
			send_log( NULL, LOG_LOAD, "load_languages: lingua numero %d non caricata", x );
			exit( EXIT_FAILURE );
		}
	}
}


/*
 * Libera una lista di termini di conversione di lingua
 */
void free_lang_conversion( LANG_CNV_DATA **first_langcnv, LANG_CNV_DATA **last_langcnv )
{
	LANG_CNV_DATA *langcnv;

	for ( langcnv = *first_langcnv;  langcnv;  langcnv = *first_langcnv )
	{
		DISPOSE( langcnv->old );
		DISPOSE( langcnv->new );
		UNLINK( langcnv, *first_langcnv, *last_langcnv, next, prev );
		DISPOSE( langcnv );
	}
}

/*
 * Libera dalla memoria tutte le lingue
 */
void free_all_languages( void )
{
	int x;

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		top_language--;

		DISPOSE( table_language[x]->name );
		free_lang_conversion( &table_language[x]->first_precnv, &table_language[x]->last_precnv );
		DISPOSE( table_language[x]->alphabet );
		free_lang_conversion( &table_language[x]->first_cnv, &table_language[x]->last_cnv );
		DISPOSE( table_language[x] );
	}

	if ( top_language != 0 )
		send_log( NULL, LOG_BUG, "free_all_languages: top_language non è 0 dopo aver liberato le lingue: %d", top_language );
}


/*
 * Passata una stringa la cerca tra i nomi delle lingue
 *	e ne ritorna il numero se uguale ad una di esse
 */
int get_lang_num( const char *name )
{
	int x;

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		if ( !str_cmp(name, table_language[x]->name) )
			return x;
	}

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		if ( !str_prefix(name, table_language[x]->name) )
			return x;
	}

	return LANGUAGE_UNKNOWN;
}


/*
 * Ritorna il massimo che il pg può imparare per quella skill/spell
 */
int get_adept_lang( CHAR_DATA *ch, const int lang )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_adept_lang: ch è NULL" );
		return 75;
	}

	if ( lang < 0 || lang >= MAX_LANGUAGE )
	{
		send_log( NULL, LOG_BUG, "get_adept_lang: lang errata: %d", lang );
		return 0;
	}

	if ( HAS_BIT(table_race[ch->race]->lang_natural, lang) )
		return 100;

	if ( HAS_BIT(table_race[ch->race]->lang_learnable, lang) )
		return 95;

	return 0;
}


/*
 * Ritorna il valore imparato di una lingua
 */
int knows_language( CHAR_DATA *ch, const int language, CHAR_DATA *cch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "knows_language: ch è NULL" );
		return 0;
	}

	if ( language < 0 || language >= MAX_LANGUAGE )
	{
		send_log( NULL, LOG_BUG, "knows_language: valore di language errato per %s: %d",
			ch->name, language );
		return 0;
	}

	/* Se un mob non ha settate delle lingue però la sua razza di natura parla quella lingua allora la conosce */
	if ( IS_MOB(ch) && IS_EMPTY(ch->speaks) && HAS_BIT(table_race[ch->race]->lang_natural, language) )
		return 100;

	/* Se il mob ha settata la lingua tra quelle parlate allora la conosce */
	if ( IS_MOB(ch) && HAS_BIT(ch->speaks, (language & ~LANGUAGE_CLAN)) )	/* evita il check sulla lingua di clan che viene eseguito dopo */
		return 100;

	if ( IS_ADMIN(ch) )
		return 100;

	if ( language == LANGUAGE_CLAN )
	{
		/* La lingua di clan è comune tra i mob */
		if ( IS_MOB(ch) || IS_MOB(cch) /* (FF) && ch->clan == cch->clan */ )
			return 100;

		if ( IS_PG(ch) && ch->pg->clan != NULL && ch->pg->clan == cch->pg->clan )
			return ch->pg->learned_lang[language];
	}

	if ( IS_MOB(ch) )
		return 0;

	return ch->pg->learned_lang[language];
}


/*
 * Ritorna se ch può imparare una determinata lingua
 */
bool can_learn_lang( CHAR_DATA *ch, const int language )
{
	/* Se ch è un mob allora non può apprendere la lingua */
	if ( IS_MOB(ch) )
		return FALSE;

	/* Se il linguaggio è di un clan è il pg è di un clan */
	if ( language == LANGUAGE_CLAN && ch->pg->clan )
		return TRUE;

	/* Se il linguaggio è naturale per la razza del pg */
	if ( HAS_BIT(table_race[ch->race]->lang_natural, language) )
		return TRUE;

	/* Se il linguaggio è apprendibile da quella razza */
	if ( HAS_BIT(table_race[ch->race]->lang_learnable, language) )
		return TRUE;

	/* In tutti gli altri casi è falso */
	return FALSE;
}


/*
 * Calcola quanto il pg ha memorizzato in lingue e ritorna se può impararne ancora o no
 */
bool can_memorize_lang( CHAR_DATA *ch )
{
	int lang;
	int count = 0;

	if ( IS_MOB(ch) )
		return FALSE;

	if ( IS_ADMIN(ch) )
		return TRUE;

	for ( lang = 0;  lang < MAX_LANGUAGE;  lang++ )
	{
		if ( lang == LANGUAGE_CLAN && ch->pg->clan )
			count += ch->pg->learned_lang[lang];
		else if ( HAS_BIT(table_race[ch->race]->lang_natural, lang) )
			count += ch->pg->learned_lang[lang] * 3/4;
		else if ( HAS_BIT(table_race[ch->race]->lang_learnable, lang) )
			count += ch->pg->learned_lang[lang] * 4/3;
	}

	if ( count >= (500 + get_level(ch)/2 + get_curr_attr(ch, ATTR_COC)/2) )
		return FALSE;

	return TRUE;
}


/*
 * Il pg che parla la lingua la impara anche un poco
 */
void learn_language( CHAR_DATA *ch, const int language )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "learn_language: ch è NULL" );
		return;
	}

	if ( language < 0 || language >= MAX_LANGUAGE )
	{
		send_log( NULL, LOG_BUG, "learn_language: valore language passato errato: %d", language );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( ch->pg->learned_lang[language] <= 1 )
		return;

	if ( ch->pg->learned_lang[language] >= get_adept_lang(ch, language) )
		return;

	if ( !can_learn_lang(ch, language) )
		return;

	if ( !can_memorize_lang(ch) )
		return;

	/* (TT) 500 sarà abbastanza o si alzano ancora troppo? */
	if ( number_range(1, 500) == 0 )
	{
send_log( NULL, LOG_MONI, "learn_language: aumento della lingua per %s", ch->name );	/* (TT) mettere un log a parte magari dopo il testing */
		ch->pg->learned_lang[language]++;
	}
}


/*
 * Comando languages, permette di imparare nuove lingue con argomento learn/impara.
 * Senza argomento elenca le lingue
 * Con argomento la lingua cambia il linguaggio a quella lingua
 */
DO_RET do_languages( CHAR_DATA *ch, char *argument )
{
	char	arg[MIL];
	int		lang;

	if ( VALID_STR(argument) )
	{
		argument = one_argument( argument, arg );

		if ( IS_PG(ch) && is_name(arg, "impara imparo imparare learn") )
		{
			CHAR_DATA *sch;
			char	   arg2[MIL];
			char	   buf[MSL];
			int		   percent;
			int		   prac;
			int		   cost;

			argument = one_argument( argument, arg2 );
			if ( !VALID_STR(arg2) )
			{
				send_to_char( ch, "Quale lingua dovrei apprendere?\r\n" );
				return;
			}

			lang = get_lang_num( arg2 );
			if ( lang == LANGUAGE_UNKNOWN )
			{
				send_to_char( ch, "Non esiste tale lingua.\r\n" );
				return;
			}

			if ( !can_learn_lang(ch, lang) )
			{
				send_to_char( ch, "Non ho le doti necessarie per apprendere questa lingua.\r\n" );
				return;
			}

			if ( ch->pg->learned_lang[lang] == MAX_LEARN_SKELL )
			{
				act( AT_PLAIN, "Possiedo già un'ottima padronanza nella lingua $t.", ch,
					table_language[lang]->name, NULL, TO_CHAR );
				return;
			}

			for ( sch = ch->in_room->first_person;  sch;  sch = sch->next_in_room )
			{
				if ( IS_PG(sch) )									continue;
				if ( !HAS_BIT_ACT(sch, MOB_SCHOLAR) )				continue;
				if ( knows_language(sch, ch->speaking, ch) == 0 )	continue;
				if ( knows_language(ch, sch->speaking, sch) == 0 )	continue;

				if ( knows_language(sch, lang, sch) )
					break;
			}

			if ( !sch )
			{
				send_to_char( ch, "Qui non c'è nessuno che mi possa insegnare qualcosa di più nelle lingue.\r\n" );
				return;
			}

			if ( !can_memorize_lang(ch) )
			{
				sprintf( buf, "say a %s Per il momento non posso istruirti su di una nuova lingua, ne conosci già troppe.", ch->name );
				send_command( sch, buf, CO );
				return;
			}

			if ( get_curr_attr(ch, ATTR_EMP) > 75 || get_curr_attr(ch, ATTR_BEA) > 75 )
				prac = 1;
			else
				prac = 2;

			if ( ch->pg->practice < prac )
			{
				sprintf( buf, "say a %s Non hai abbastanza pratica!", ch->name );
				send_command( sch, buf, CO );
				return;
			}

			/* Calcola il costo della lezione */
			if ( get_level(ch) <= LVL_NEWBIE/2 )
				cost = 0;
			else
				cost = compute_cost( ch, sch, get_level(ch)/4 * 50 );

			if ( cost <= 0 )
			{
				sprintf( buf, "say a %s Pare che oggi sia il vostro giorno fortunato, vi insegnerò gratuitamente!", ch->name );
				send_command( sch, buf, CO );
			}

			if ( ch->gold < cost )
			{
				sprintf( buf, "say a %s Questa lezione costa %d, ma ti manca della moneta.", ch->name, cost );
				send_command( sch, buf, CO );
				return;
			}

			if ( cost > 0 )
				ch->gold -= cost;
			ch->pg->practice -= prac;

			/* Minimo 5, massimo 25 con 100 di concentrazione */
			percent = 5 + get_curr_attr( ch, ATTR_COC ) / 5;
			ch->pg->learned_lang[lang] += percent;
			ch->pg->learned_lang[lang] = UMIN( ch->pg->learned_lang[lang], MAX_LEARN_SKELL );

			if ( ch->pg->learned_lang[lang] == percent )
			{
				act( AT_PLAIN, "Inizio la lezione nella lingua $t.",
					ch, table_language[lang]->name, NULL, TO_CHAR );
			}
			else if ( ch->pg->learned_lang[lang] < 60 )
			{
				act( AT_PLAIN, "Continuo la lezione nella lingua $t.",
					ch, table_language[lang]->name, NULL, TO_CHAR );
			}
			else if ( ch->pg->learned_lang[lang] < 60 + percent )
			{
				act( AT_PLAIN, "Mi sento preparato riguardo la lingua $t.",
					ch, table_language[lang]->name, NULL, TO_CHAR );
			}
			else if ( ch->pg->learned_lang[lang] < 99 )
			{
				act( AT_PLAIN, "Sento di aver ottenuto maggior scorrevolezza nella lingua $t.",
					ch, table_language[lang]->name, NULL, TO_CHAR );
			}
			else
			{
				act( AT_PLAIN, "Parlo in maniera pressoché perfetta la lingua $t.",
					ch, table_language[lang]->name, NULL, TO_CHAR );
			}

			return;
		}
		else
		{
			for ( lang = 0;  lang < MAX_LANGUAGE;  lang++ )
			{
				if ( str_prefix(arg, table_language[lang]->name) )		continue;
				if ( knows_language(ch, lang, ch) == 0 )				continue;
				/* Se la lingua è di clan la parla solo se il pg è in un clan */
				if ( lang == LANGUAGE_CLAN )
				{
					if ( IS_MOB(ch) || !ch->pg->clan )					continue;
				}
				else
				{
					if ( can_learn_lang(ch, lang) == FALSE )			continue;
				}

				set_char_color( AT_SAY, ch );
				ch_printf( ch, "Ora parlo in %s.\r\n", table_language[lang]->name );
				ch->speaking = lang;

				return;
			}

			send_to_char( ch, "Non conosco tale lingua.\r\n" );
			return;
		}
	} /* chiude l'if */

	/* Se non si passa nulla allora visualizza la lista delle lingue */
	ch_printf( ch, "Attualmente sto parlando in %s\r\n", table_language[ch->speaking]->name );

	send_to_char( ch, "\r\nConosco inoltre le lingue:\r\n" );
	for ( lang = 0;  lang < MAX_LANGUAGE;  lang++ )
	{
		/* Se la lingua è di clan la visualizza solo se il pg è in un clan */
		if ( lang == LANGUAGE_CLAN )
		{
			if ( IS_MOB(ch) || !ch->pg->clan )			continue;
		}
		else
		{
			if ( can_learn_lang(ch, lang) == FALSE )	continue;
		}

		if ( knows_language(ch, lang, ch) > 0 )
		{
			ch_printf( ch, "%-16s ", table_language[lang]->name );

			if		( ch->pg->learned_lang[lang] >= get_adept_lang(ch, lang) )
				set_char_color( AT_YELLOW, ch );
			else if ( ch->pg->learned_lang[lang] > 0 )
				set_char_color( AT_SCORE, ch );
			else
				set_char_color( AT_GREY, ch );

			ch_printf( ch, "%3d%%\r\n", ch->pg->learned_lang[lang] );
		}
	}

	send_to_char( ch, "\r\nIn futuro potrei conoscere le seguenti lingue:\r\n" );
	for ( lang = 0;  lang < MAX_LANGUAGE;  lang++ )
	{
		/* Se la lingua è di clan la visualizza solo se il pg è in un clan */
		if ( lang == LANGUAGE_CLAN )
		{
			if ( IS_MOB(ch) || !ch->pg->clan )			continue;
		}
		else
		{
			if ( can_learn_lang(ch, lang) == FALSE )	continue;
		}

		if ( knows_language(ch, lang, ch) == 0 )
			ch_printf( ch, "%s\r\n", table_language[lang]->name );
	}
}


/*
 * Passata una stringa se è uguale ad un nome di una lingua la ritorna.
 */
LANG_DATA *get_lang( const char *name )
{
	int x;

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		if ( !str_cmp(name, table_language[x]->name) )
			return table_language[x];
	}

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		if ( !str_prefix(name, table_language[x]->name) )
			return table_language[x];
	}

	return NULL;
}


/*
 * Ritorna la lingua con cui ch si trova meglio a parlare con victim
 * Se victim è ch stesso è come se cercasse la lingua con cui si trovi meglio a parlare
 * Fa una media tra quello che ch sa della lingua che sa e quello che sa la vittima,
 *	quindi sceglie il valore maggiore
 */
int get_lang_speakable( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( IS_MOB(ch) )
		return LANGUAGE_COMMON;

	return LANGUAGE_COMMON;
}


/*
 * Traduce una lingua.
 * percent = percentuale di linguaggio conosciuto.
 */
char *translate( int percent, const char *in, const int lang )
{
	LANG_CNV_DATA  *cnv;
	static char		buf[MIL];
	char			buf2[MIL];
	const char	   *pbuf;
	char	  	   *pbuf2 = buf2;


	if ( percent > 99 || lang == LANGUAGE_COMMON )
	{
		strcpy( buf, in );
		return buf;
	}

	for ( pbuf = in;  *pbuf;  )
	{
		for ( cnv = table_language[lang]->first_precnv;  cnv;  cnv = cnv->next )
		{
			if ( !str_prefix(cnv->old, pbuf) )
			{
				if ( percent && (rand()%100) < percent )
				{
					strncpy( pbuf2, pbuf, cnv->olen );
					pbuf2[cnv->olen] = '\0';
					pbuf2 += cnv->olen;
				}
				else
				{
					strcpy( pbuf2, cnv->new );
					pbuf2 += cnv->nlen;
				}
				pbuf += cnv->olen;
				break;
			}
		}

		if ( !cnv )
		{
			if ( isalpha(*pbuf) && (!percent || (rand() % 100) > percent) )
			{
				*pbuf2 = table_language[lang]->alphabet[tolower(*pbuf) - 'a'];

				if ( isupper(*pbuf) )
					*pbuf2 = toupper( *pbuf2 );
			}
			else
				*pbuf2 = *pbuf;

			pbuf++;
			pbuf2++;
		}
	}
	pbuf2[0] = '\0';

	for ( pbuf = buf2, pbuf2 = buf;  *pbuf;  )
	{
		for ( cnv = table_language[lang]->first_cnv;  cnv;  cnv = cnv->next )
		{
			if ( !str_prefix(cnv->old, pbuf) )
			{
				strcpy( pbuf2, cnv->new );
				pbuf  += cnv->olen;
				pbuf2 += cnv->nlen;
				break;
			}
		}
		if ( !cnv )
			*(pbuf2++) = *(pbuf++);
	}

	*pbuf2 = '\0';
	return buf;
}


#ifdef T2_ALFA
/*
 * Mischia-Testo: disordina i caratteri per i PG che non conoscono quella lingua.
 */
char *scramble( const char *argument, int modifier )
{
	static char arg[MIL];
	int			position;
	int			conversion = 0;

	modifier %= number_range( 80, 300 );		/* Bitvectors get way too large #s */

	for ( position = 0;  position < MIL;  position++ )
	{
		if ( !VALID_STR(argument) || argument[position] == '\0' )
		{
			arg[position] = '\0';
			return arg;
		}

		if ( argument[position] >= 'A' && argument[position] <= 'Z' )
		{
			conversion = -conversion + position - modifier + argument[position] - 'A';
			conversion = number_range( conversion - 5, conversion + 5 );

			while ( conversion > 25 )
				conversion -= 26;

			while ( conversion < 0 )
				conversion += 26;

			arg[position] = conversion + 'A';
		}
		else if ( argument[position] >= 'a' && argument[position] <= 'z' )
		{
			conversion = -conversion + position - modifier + argument[position] - 'a';
			conversion = number_range( conversion - 5, conversion + 5 );

			while ( conversion > 25 )
				conversion -= 26;

			while ( conversion < 0 )
				conversion += 26;

			arg[position] = conversion + 'a';
		}
		else if ( argument[position] >= '0' && argument[position] <= '9' )
		{
			conversion = -conversion + position - modifier + argument[position] - '0';
			conversion = number_range( conversion - 2, conversion + 2 );

			while ( conversion > 9 )
				conversion -= 10;

			while ( conversion < 0 )
				conversion += 10;

			arg[position] = conversion + '0';
		}
		else
		{
			arg[position] = argument[position];
		}
	}
	arg[position] = '\0';

	return arg;
}
#endif


/*
 * Comando admin per settare le lingue di un pg
 */
DO_RET do_setlanguage( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	char		arg1[MIL];
	char		arg2[MIL];
	int			value;
	int			lang = -1;
	bool		fAll;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(argument) || !is_number(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  setlang <vittima> <lingua> <valore>\r\n" );
		send_to_char( ch, "       o:  setlang <vittima> tutte    <valore>\r\n" );
		return;
	}

	victim = get_player_mud( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è, oppure scrivi il suo nome per intero.\r\n" );
		return;
	}

	fAll = !str_cmp_all( arg2, FALSE );
	if ( !fAll && (lang = get_lang_num(arg2)) < 0 )
	{
		send_to_char( ch, "Nessuna lingua con questo nome.\r\n" );
		return;
    }

	value = atoi( argument );
	if ( value < 0 || value > MAX_LEARN_SKELL )
	{
		ch_printf( ch, "Value range is 0 to %d.\r\n", MAX_LEARN_SKELL );
		return;
	}

	if ( fAll )
	{
		for ( lang = 0;  lang < MAX_LANGUAGE;  lang++ )
		{
			if ( !VALID_STR(table_language[lang]->name) )
				continue;
			if ( !IS_ADMIN(victim) && value > get_adept_lang(victim, lang) )
				victim->pg->learned_lang[lang] = get_adept_lang( victim, lang );
			else
				victim->pg->learned_lang[lang] = value;
		}
	}
	else
	{
		victim->pg->learned_lang[lang] = value;
	}

	save_player( victim );
}


/*
 * Comando admin per vedere una lingua
 * (FF) da finire con più info
 */
DO_RET do_showlanguage( CHAR_DATA *ch, char argument )
{
	int x;

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		send_to_char( ch, "Lista delle lingue:" );
		ch_printf( ch, "%s\n", table_language[x]->name );
	}
}
