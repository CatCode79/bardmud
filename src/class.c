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
 >						Modulo Gestione Classi								<
\****************************************************************************/

#include <time.h>

#include "mud.h"
#include "fread.h"


/*
 * Variabili esterne
 */
CLASS_DATA	*table_class[MAX_CLASS];	/* Tabella delle classi */
int			 top_class_play = 0;		/* Il totale delle classi giocabili */


/*
 * Definizioni locali
 */
#define CLASS_LIST		"../classes/classes.lst"	/* File con la Lista delle Classi */


/*
 * Tabella di codici delle classi
 * Ci sono i codici anche nella parte del nome per far funzionare
 *	il mprog "isclass" tanto il nome viene ricavato dalla funzione
 *
 */
const CODE_DATA code_class[] =
{
	{ CLASS_MAGE,			"CLASS_MAGE",		"mago"			},
	{ CLASS_PRIEST,			"CLASS_PRIEST",		"sacerdote"		},
	{ CLASS_THIEF,			"CLASS_THIEF",		"ladro"			},
	{ CLASS_FIGHTER,		"CLASS_FIGHTER",	"combattente"	},
	{ CLASS_DRUID,			"CLASS_DRUID",		"druido"		},
	{ CLASS_WANDERING,		"CLASS_WANDERING",	"ramingo"		},
	{ CLASS_PALADIN,		"CLASS_PALADIN",	"paladino"		},
	{ CLASS_ALCHEMIST,		"CLASS_ALCHEMIST",	"alchimista"	},
	{ CLASS_NECROMANCER,	"CLASS_NECROMANCER","necromante"	},
	{ CLASS_BARD,			"CLASS_BARD",		"bardo"			},
	{ CLASS_09,				"CLASS_09",			"System09"		},
	{ CLASS_10,				"CLASS_10",			"System10"		},
	{ CLASS_11,				"CLASS_11",			"System11"		},
	{ CLASS_12,				"CLASS_12",			"System12"		},
	{ CLASS_13,				"CLASS_13",			"System13"		},
	{ CLASS_14,				"CLASS_14",			"System14"		},
	{ CLASS_15,				"CLASS_15",			"System15"		},
	{ CLASS_16,				"CLASS_16",			"System16"		},
	{ CLASS_17,				"CLASS_17",			"System17"		},
	{ CLASS_18,				"CLASS_18",			"System18"		},
	{ CLASS_19,				"CLASS_19",			"System19"		},
	{ CLASS_BAKER,			"CLASS_BAKER",		"Panettiere"	},
	{ CLASS_BUTCHER,		"CLASS_BUTCHER",	"Macellaio"		},
	{ CLASS_BLACKSMITH,		"CLASS_BLACKSMITH",	"Fabbro"		},
	{ CLASS_MAYOR,			"CLASS_MAYOR",		"Maggiore"		},
	{ CLASS_KING,			"CLASS_KING",		"Re"			},
	{ CLASS_QUEEN,			"CLASS_QUEEN",		"Regina"		}
};
const int max_check_class = sizeof(code_class)/sizeof(CODE_DATA);


/*
 * Tabella dei codici delle flag di classe
 */
const CODE_DATA code_classflag[] =
{
	{ CLASSFLAG_PLAYABLE,	"CLASSFLAG_PLAYABLE",	"giocabile"	},
	{ CLASSFLAG_CASTER,		"CLASSFLAG_CASTER",		"caster"	}
};
const int max_check_classflag = sizeof(code_classflag)/sizeof(CODE_DATA);


/*
 * Legge un file di classe.
 */
void fread_class( MUD_FILE *fp )
{
	CLASS_DATA *class;
	int			x;
	int			class_number = -1;

	CREATE( class, CLASS_DATA, 1 );

	for ( ; ; )
	{
		char   *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if ( !str_cmp(word, "AvarageAttr") )
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
				class->avarage_attr[x] =									fread_number(fp );
		}
		/* (FF) AvarageSkill */
		else if ( !str_cmp(word, "Color"		 ) )	class->color =		fread_synonym( fp, CODE_COLOR );
		else if ( !str_cmp(word, "End"			 ) )	break;
		else if ( !str_cmp(word, "ExpBase"		 ) )	class->exp_base =	fread_number( fp );
		else if ( !str_cmp(word, "Flags"		 ) )	class->flags =		fread_code_bit( fp, CODE_CLASSFLAG );
		else if ( !str_cmp(word, "ClassNumber"	 ) )	class_number =		code_code( fp, fread_word(fp), CODE_CLASS );
		else if ( !str_cmp(word, "NameMale"		 ) )	class->name_male =	fread_string( fp );
		else if ( !str_cmp(word, "NameFemale"	 ) )	class->name_fema =	fread_string( fp );
		else if ( !str_cmp(word, "PointsName"	 ) )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
				class->points_name[x] =		str_dup( fread_word(fp) );
		}
		else if ( !str_cmp(word, "RegainBase"	 ) )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
				class->regain_base[x] = 	fread_number( fp );
		}
		else if ( !str_cmp(word, "Skill"		 ) )
		{
			int	sn;

			if ( class_number < 0 || class_number >= MAX_CLASS )
			{
				send_log( fp, LOG_FREAD, "fread_class: skill %s, class bad/not found (%d)", word, class_number );
			    exit( EXIT_FAILURE );
			}

			word = fread_word( fp );
			sn = skill_lookup( word );
			if ( !is_valid_sn(sn) )
			{
				send_log( fp, LOG_FREAD, "fread_class: skill %s unknown", word );
				exit( EXIT_FAILURE );
			}

			table_skill[sn]->skill_level[class_number] = fread_number( fp );
			table_skill[sn]->skill_adept[class_number] = fread_number( fp );

			if ( table_skill[sn]->skill_level[class_number] < 1 || table_skill[sn]->skill_level[class_number] > LVL_LEGEND )
			{
				send_log( fp, LOG_FREAD, "fread_class: livello della skill errata: %d",
					table_skill[sn]->skill_level[class_number] );
				exit( EXIT_FAILURE );
			}

			if ( table_skill[sn]->skill_adept[class_number] < 0 || table_skill[sn]->skill_adept[class_number] > MAX_LEARN_SKELL )
			{
				send_log( fp, LOG_FREAD, "fread_class: adept della skill errata: %d",
					table_skill[sn]->skill_adept[class_number] );
				exit( EXIT_FAILURE );
			}
		}
		else if ( !str_cmp(word, "Thac0") )		class->thac0_00 =	fread_number( fp );
		else if ( !str_cmp(word, "Thac32") )	class->thac0_32 =	fread_number( fp );
		else
			send_log( fp, LOG_FREAD, "fread_class: non trovato %s alla classe %d.", word, class_number );
	} /* chiude il for */

	/*
	 * Caricata la classe la controlla
	 */

	if ( !VALID_STR(class->name_male) || !VALID_STR(class->name_fema) )
	{
		send_log( fp, LOG_FREAD, "fread_class: nome maschile o femminile non valido alla classe numero %d",
			class_number );
		exit( EXIT_FAILURE );
	}

	if ( strlen(class->name_male) > MAX_LEN_NAME_CLASS
	  || strlen(class->name_fema) > MAX_LEN_NAME_CLASS )
	{
		send_log( fp, LOG_FREAD, "fread_class: nome maschile o femminile troppo lungo alla classe numero %d",
			class_number );
		exit( EXIT_FAILURE );
	}

	if ( class_number < 0 || class_number >= MAX_CLASS )
	{
		send_log( fp, LOG_FREAD, "fread_class: classe %d errata", class_number );
		exit( EXIT_FAILURE );
	}

	/* Controlla che i valori della tabella dei codici siano uguali al maschile caricato */
	if ( str_cmp(class->name_male, code_name(NULL, class_number, CODE_CLASS)) )
	{
		send_log( fp, LOG_FREAD, "fread_class: nome maschile della classe %s non corrispondende al nome di codice della tabella classe %s",
			class->name_male, code_name(NULL, class_number, CODE_CLASS) );
/*		exit( EXIT_FAILURE ); */
	}

	table_class[class_number] = class;
	if ( HAS_BIT(class->flags, CLASSFLAG_PLAYABLE) )
		top_class_play++;
}


/*
 * Carica e controlla tutte le classi
 */
void load_classes( void )
{
	int x;

	for ( x = 0;  x < MAX_CLASS;  x++ )
		table_class[x] = NULL;

	fread_list( CLASS_LIST, "CLASS", fread_class, TRUE );

	/*
	 * Controlla le classi
	 */
	if ( top_class_play <= 0 || top_class_play >= MAX_CLASS )
	{
		send_log( NULL, LOG_FREAD, "fread_class: numero classi giocabili troppo basso o troppo alto: %d",
			top_class_play );
		exit( EXIT_FAILURE );
	}

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		if ( !table_class[x] )
		{
			send_log( NULL, LOG_LOAD, "check_all_classes: classe numero %d non caricata", x );
			exit( EXIT_FAILURE );
		}
	}
}


/*
 * Libera dalla memoria tutte le classi
 */
void free_all_classes( void )
{
	int x;

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		int y;

		DISPOSE( table_class[x]->name_male );
		DISPOSE( table_class[x]->name_fema );
		free_synonym( table_class[x]->color );
		destroybv( table_class[x]->flags );
		for ( y = 0;  y < MAX_POINTS;  y++ )
			DISPOSE( table_class[x]->points_name[y] );
		DISPOSE( table_class[x] );
	}
}


/*
 * Controlla se l'argomento passato sia un nome di una classe
 * Ritorna FALSE se ha trovato che lo è
 * (RR) da cambiare in get_class
 */
CLASS_DATA *is_name_class( const char *argument, bool exact )
{
	int x;

	if ( !VALID_STR(argument) )
		return NULL;

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		if ( !str_cmp(argument, table_class[x]->name_male) )
			return table_class[x];
		if ( !str_cmp(argument, table_class[x]->name_fema) )
			return table_class[x];
	}

	if ( exact )
		return NULL;

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		if ( !str_prefix(argument, table_class[x]->name_male) )
			return table_class[x];
		if ( !str_prefix(argument, table_class[x]->name_fema) )
			return table_class[x];
	}

	return NULL;
}

/*
 * Da convertire nella code_num quando ci sarà il sistema gramm. maschile/femminile (GR)
 */
int get_class_num( const char *type, bool exact )
{
	CLASS_DATA *class;
	int			x;

	class = is_name_class( type, exact );
	if ( !class )
		return -1;

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		if ( class == table_class[x] )
			return x;
	}

	return -1;
}


/*
 * Acquisisce la classe del char
 */
char *get_class( CHAR_DATA *ch, bool color )
{
	static char	buf[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_classe: ch è NULL" );
		return (color) ? "&wSconosciuta" : "Sconosciuta";
	}

	/* (RR) in realtà bisognerebbe dividere il test per mob e per pg, visto che le classi
	 *	son diverse, controllare anche il playable o no */
	if ( ch->class < 0 && ch->class >= MAX_CLASS )
	{
		send_log( NULL, LOG_BUG, "get_class: valore classe errato per il pg %s: %d", ch->name, ch->class );
		return (color) ? "&wSconosciuta" : "Sconosciuta";
	}

	buf[0] = '\0';
	if ( color )
		sprintf( buf, "&%c", get_clr(table_class[ch->class]->color->cat) );

	/* (GR) cambiarla nella funzione che cerca il nome in base al sesso */
	if ( ch->sex == SEX_FEMALE )
		strcat( buf, table_class[ch->class]->name_fema );
	else
		strcat( buf, table_class[ch->class]->name_male );

	if ( color )
		strcat( buf, "&w" );

	return buf;
}


/*
 * Comando Admin che mostra una classe
 * (RR) Da rivedere se mancano alcune info
 */
DO_RET do_showclass( CHAR_DATA *ch, char *argument )
{
	CLASS_DATA *class;
	int			cl;
	char		buf[MIL];
	int			x;
	int			cnt;

	set_char_color( AT_PLAIN, ch );

	if ( !VALID_STR(argument) )
	{
		int ct = 0;

		send_to_char( ch, "&YSintassi&w: showclass <nome classe>\r\n" );
		send_to_char( ch, "Le classi in verde chiaro sono giocabili\r\n\r\n" );

		for ( x = 0;  x < MAX_CLASS;  x++ )
		{
			ct++;

			pager_printf(ch, "%3d] &%c%-20s &w",
				x,
				(HAS_BIT(table_class[x]->flags, CLASSFLAG_PLAYABLE)) ? 'G' : 'w',
				table_class[x]->name_male );

			if ( (ct%3) == 0 || ct == 0 )
				send_to_pager( ch, "\r\n" );
		}
		send_to_pager( ch, "\r\n" );
		return;
	}

	cl = get_class_num( argument, FALSE );
	class = is_name_class( argument, FALSE );
	if ( !class || cl == -1 )
	{
		send_to_char( ch, "No such class.\r\n" );
		return;
	}

	pager_printf( ch, "&wClass: &W%s&w/&W%s\r\n",
		class->name_male, class->name_fema );
	pager_printf( ch, "&wExpBase: &W%d\r\n", class->exp_base );
    pager_printf( ch, "&wThac0 : &W%-5d     &wThac32: &W%d\n\r",
		class->thac0_00, class->thac0_32 );

	pager_printf( ch, "&wAttributi Classe:\r\n"
		"Str Res Con Int Coc Wil Agi Ref Spe Spi Mig Abs Emp Bea Lea\r\n" );
	strcpy( buf, "&W" );
	for ( x = 0;  x < MAX_ATTR;  x++ )
		sprintf( buf+strlen(buf), "%3d ", class->avarage_attr[x] );

	cnt = 0;
	set_char_color( AT_BLUE, ch );
	for ( x = gsn_first_spell;  x < gsn_top_sn;  x++ )
	{
		pager_printf( ch, "  %-7s %-19s%3d     ",
			code_name(NULL, table_skill[x]->type, CODE_SKILLTYPE),
			table_skill[x]->name,
			table_skill[x]->skill_adept[cl] );
		if ( ++cnt % 2 == 0 )
			send_to_pager( ch, "\r\n" );
	}

	pager_printf( ch, "%s\r\n", buf );
}
