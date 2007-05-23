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
 >							Modulo Gestione Razze							<
\****************************************************************************/

#include "mud.h"
#include "fread.h"


/*
 * Definizioni
 */
#define RACE_LIST	"../races/races.lst"	/* File con la Lista delle Razze */


/*
 * Variabili globali
 */
RACE_DATA 	*table_race[MAX_RACE];	/* Tabella delle razze */
int			 top_race_play = 0;		/* Il totale delle razze giocabili */


/*
 * Tabella di codici delle razze
 */
const CODE_DATA code_race[] =
{
	{ RACE_HUMAN,			"RACE_HUMAN",			"Umano"				},
	{ RACE_ELF,				"RACE_ELF",				"Elfo"				},
	{ RACE_DWARF,			"RACE_DWARF",			"Nano"				},
	{ RACE_HALFLING,		"RACE_HALFLING",		"Halfling"			},
	{ RACE_PIXIE,			"RACE_PIXIE",			"Pixie"				},
	{ RACE_VAMPIRE,			"RACE_VAMPIRE",			"Vampiro"			},
	{ RACE_HALFOGRE,		"RACE_HALFOGRE",		"Mezzogre"			},
	{ RACE_HALFORC,			"RACE_HALFORC",			"Mezzorco"			},
	{ RACE_HALFTROLL,		"RACE_HALFTROLL",		"MezzoTroll"		},
	{ RACE_HALFELF,			"RACE_HALFELF",			"Mezzelfo"			},
	{ RACE_GITH,			"RACE_GITH",			"Githyanki"			},
	{ RACE_DROW,			"RACE_DROW",			"Drow"				},
	{ RACE_SEAELF,			"RACE_SEAELF",			"Elfo Marino"		},
	{ RACE_LIZARDMAN,		"RACE_LIZARDMAN",		"Thepa"				},
	{ RACE_GNOME,			"RACE_GNOME",			"Gnomo"				},
	{ RACE_R5,				"RACE_R5",				"R5_Sistema"		},
	{ RACE_R6,				"RACE_R6",				"R6_Sistema"		},
	{ RACE_R7,				"RACE_R7",				"R7_Sistema"		},
	{ RACE_R8,				"RACE_R8",				"R8"				},
	{ RACE_TROLL,			"RACE_TROLL",			"Troll"				},
	{ RACE_ANT,				"RACE_ANT",				"Formica"			},
	{ RACE_APE,				"RACE_APE",				"Scimmia"			},
	{ RACE_BABOON,			"RACE_BABOON",			"Babbuino"			},
	{ RACE_BAT,				"RACE_BAT",				"Pipistrello"		},
	{ RACE_BEAR,			"RACE_BEAR",			"Orso"				},
	{ RACE_BEE,				"RACE_BEE",				"Fuco"				},
	{ RACE_BEETLE,			"RACE_BEETLE",			"Coleottero"		},
	{ RACE_BOAR,			"RACE_BOAR",			"Cinghiale"			},
	{ RACE_BUGBEAR,			"RACE_BUGBEAR",			"Bugbear"			},
	{ RACE_CAT,				"RACE_CAT",				"Gatto"				},
	{ RACE_DOG,				"RACE_DOG",				"Cane"				},
	{ RACE_DRAGON,			"RACE_DRAGON",			"Drago"				},
	{ RACE_FERRET,			"RACE_FERRET",			"Furetto"			},
	{ RACE_FLY,				"RACE_FLY",				"Mosca"				},
	{ RACE_GARGOYLE,		"RACE_GARGOYLE",		"Gargoyle"			},
	{ RACE_GELATIN,			"RACE_GELATIN",			"Cubo gelatinoso"	},
	{ RACE_GHOUL,			"RACE_GHOUL",			"Ghoul"				},
	{ RACE_GNOLL,			"RACE_GNOLL",			"Gnoll"				},
	{ RACE_GNOME_2,			"RACE_GNOME_2",			"Gnomo_2"			},
	{ RACE_GOBLIN,			"RACE_GOBLIN",			"Goblin"			},
	{ RACE_GOLEM,			"RACE_GOLEM",			"Golem"				},
	{ RACE_GORGON,			"RACE_GORGON",			"Gorgone"			},
	{ RACE_HARPY,			"RACE_HARPY",			"Arpia"				},
	{ RACE_HOBGOBLIN,		"RACE_HOBGOBLIN",		"Hobgoblin"			},
	{ RACE_KOBOLD,			"RACE_KOBOLD",			"Coboldo"			},
	{ RACE_LIZARDMAN_2,		"RACE_LIZARDMAN_2",		"Lizardman_2"		},
	{ RACE_LOCUST,			"RACE_LOCUST",			"Locusta"			},
	{ RACE_LYCANTHROPE,		"RACE_LYCANTHROPE",		"Licantropo"		},
	{ RACE_MINOTAUR,		"RACE_MINOTAUR",		"Minotauro"			},
	{ RACE_MOLD,			"RACE_MOLD",			"Muffa"				},
	{ RACE_MULE,			"RACE_MULE",			"Mulo"				},
	{ RACE_NEANDERTHAL,		"RACE_NEANDERTHAL",		"Neanderthal"		},
	{ RACE_OOZE,			"RACE_OOZE",			"Fanghiglia"		},
	{ RACE_ORC,				"RACE_ORC",				"Orco"				},
	{ RACE_RAT,				"RACE_RAT",				"Ratto"				},
	{ RACE_RUSTMONSTER,		"RACE_RUSTMONSTER",		"Corrosivo"			},
	{ RACE_SHADOW,			"RACE_SHADOW",			"Ombra"				},
	{ RACE_SHAPESHIFTER,	"RACE_SHAPESHIFTER",	"Mutaforma"			},
	{ RACE_SHREW,			"RACE_SHREW",			"Toporagno"			},
	{ RACE_SHRIEKER,		"RACE_SHRIEKER",		"FunghiUrlatori"	},
	{ RACE_SKELETON,		"RACE_SKELETON",		"Scheletro"			},
	{ RACE_SLIME,			"RACE_SLIME",			"Slime"				},
	{ RACE_SNAKE,			"RACE_SNAKE",			"Serpente"			},
	{ RACE_SPIDER,			"RACE_SPIDER",			"Ragno"				},
	{ RACE_STIRGE,			"RACE_STIRGE",			"Stirge"			},
	{ RACE_THOUL,			"RACE_THOUL",			"Thoul"				},
	{ RACE_TROGLODYTE,		"RACE_TROGLODYTE",		"Troglodita"		},
	{ RACE_UNDEAD,			"RACE_UNDEAD",			"Non-Morto"			},
	{ RACE_WIGHT,			"RACE_WIGHT",			"Wight"				},
	{ RACE_WOLF,			"RACE_WOLF",			"Lupo"				},
	{ RACE_WORM,			"RACE_WORM",			"Verme"				},
	{ RACE_ZOMBIE,			"RACE_ZOMBIE",			"Zombie"			},
	{ RACE_BOVINE,			"RACE_BOVINE",			"Bovino"			},
	{ RACE_CANINE,			"RACE_CANINE",			"Canina"			},
	{ RACE_FELINE,			"RACE_FELINE",			"Felino"			},
	{ RACE_PORCINE,			"RACE_PORCINE",			"Suino"				},
	{ RACE_MAMMAL,			"RACE_MAMMAL",			"Mammifero"			},
	{ RACE_RODENT,			"RACE_RODENT",			"Roditore"			},
	{ RACE_BIRD,			"RACE_BIRD",			"Volatile"			},
	{ RACE_REPTILE,			"RACE_REPTILE",			"Rettile"			},
	{ RACE_AMPHIBIAN,		"RACE_AMPHIBIAN",		"Anfibio"			},
	{ RACE_FISH,			"RACE_FISH",			"Pesce"				},
	{ RACE_CRUSTACEAN,		"RACE_CRUSTACEAN",		"Crostaceo"			},
	{ RACE_INSECT,			"RACE_INSECT",			"Insetto"			},
	{ RACE_SPIRIT,			"RACE_SPIRIT",			"Spirito"			},
	{ RACE_MAGICAL,			"RACE_MAGICAL",			"Magico"			},
	{ RACE_HORSE,			"RACE_HORSE",			"Cavallo"			},
	{ RACE_ANIMAL,			"RACE_ANIMAL",			"Animale"			},
	{ RACE_HUMANOID,		"RACE_HUMANOID",		"Umanoide"			},
	{ RACE_MONSTER,			"RACE_MONSTER",			"Mostro"			},
	{ RACE_GOD,				"RACE_GOD",				"Dio"				},
	{ RACE_FELAR,			"RACE_FELAR",			"Felar"				},
	{ RACE_ARIAL,			"RACE_ARIAL",			"Arial"				},
	{ RACE_DEEPGNOME,		"RACE_DEEPGNOME",		"DeepGnomo"			},
	/* Razze Miciko */
	{ RACE_DEMON,			"RACE_DEMON",			"Demone"			},
	{ RACE_SONGBIRD,		"RACE_SONGBIRD",		"Canarino"			},
	{ RACE_EAGLE,			"RACE_EAGLE",			"Aquila"			},
	{ RACE_GHOST,			"RACE_GHOST",			"Fantasma"			},
	{ RACE_CENTIPEDE,		"RACE_CENTIPEDE",		"Centopiedi"		},
	{ RACE_DOLL,			"RACE_DOLL",			"Bambola"			},	/* (CC) da togliere */
	{ RACE_WATER_ELEMENTAL,	"RACE_WATER_ELEMENTAL",	"Elementale Acqua"	},	/* da accorpare le elementali in un unica razza, poi ci pensano gli immune e simili a gestire il tipo */
	{ RACE_RABBIT,			"RACE_RABBIT",			"Coniglio"			},
	{ RACE_AIR_ELEMENTAL,	"RACE_AIR_ELEMENTAL",	"Elementale Aria"	},
	{ RACE_EARTH_ELEMENTAL,	"RACE_EARTH_ELEMENTAL",	"Elementale Terra"	},
	{ RACE_FIRE_ELEMENTAL,	"RACE_FIRE_ELEMENTAL",	"Elementale Fuoco"	},
	{ RACE_FOX,				"RACE_FOX",				"Volpe"				},
	{ RACE_DRACONIAN,		"RACE_DRACONIAN",		"Draconiano"		},
	{ RACE_SATYR,			"RACE_SATYR",			"Satiro"			},
	{ RACE_MINDFLAYER,		"RACE_MINDFLAYER",		"MindFlayer"		}	/* e illithid */
};
const int max_check_race = sizeof(code_race)/sizeof(CODE_DATA);


/*
 * Tabella codici delle flag di razza
 */
const CODE_DATA code_raceflag[] =
{
	{ RACEFLAG_PLAYABLE,	"RACEFLAG_PLAYABLE",	"giocabile"		},
	{ RACEFLAG_NOMONEY,		"RACEFLAG_NOMONEY",		"senzamonete"	}
};
const int max_check_raceflag = sizeof(code_raceflag)/sizeof(CODE_DATA);


/*
 * Legge un file di razza
 */
void fread_race( MUD_FILE *fp )
{
	RACE_DATA	*race;
	int			 x;
	int			 race_number = -1;
/*	int			 wear = 0; */

	CREATE( race, RACE_DATA, 1 );

	for ( x = 0;  x < MAX_WEARLOC;  x++ )
		race->wear_name[x] = str_dup( wear_name[x] );

	for ( ; ; )
	{
		char		*word;

		word   = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'				  )								fread_to_eol( fp );
		else if ( !str_cmp(word, "ACPlus"		) )	race->ac_plus  =			fread_number( fp );
		else if ( !str_cmp(word, "AlignStart"	) )	race->align_start =			fread_number( fp );
		else if ( !str_cmp(word, "Affected"		) )	race->affected =			fread_code_bit( fp, CODE_AFFECT );
		else if ( !str_cmp(word, "Attacks"		) )	race->attacks  =			fread_old_bitvector( fp, CODE_ATTACK );
		else if ( !str_cmp(word, "AttrPlus"		) )
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
				race->attr_plus[x] =											fread_number( fp );
		}
		else if ( !str_cmp(word, "Color"		) )	race->color =				fread_synonym( fp, CODE_COLOR );
		else if ( !str_cmp(word, "Defenses"		) )	race->defenses =			fread_old_bitvector( fp, CODE_DEFENSE );
		else if ( !str_cmp(word, "Flags"		) )	race->flags =				fread_code_bit( fp, CODE_RACEFLAG );
		else if ( !str_cmp(word, "End"			) )	break;
		else if ( !str_cmp(word, "ExpMult"		) )	race->exp_multiplier	 =	fread_number( fp );
		else if ( !str_cmp(word, "Height"		) )	race->height			 =	fread_number( fp );
		else if ( !str_cmp(word, "HungerMod"	) )	race->hunger_mod		 =	fread_number( fp );
		else if ( !str_cmp(word, "LangNatural"	) )	race->lang_natural		 =	fread_code_bit( fp, CODE_LANGUAGE );
		else if ( !str_cmp(word, "LangLearnable") )	race->lang_learnable	 =	fread_code_bit( fp, CODE_LANGUAGE );
		else if ( !str_cmp(word, "MinAlign"		) )	race->min_align			 =	fread_number( fp );
		else if ( !str_cmp(word, "MaxAlign"		) )	race->max_align			 =	fread_number( fp );
		else if ( !str_cmp(word, "NameMale"		) )	race->name_male			 =	fread_string( fp );
		else if ( !str_cmp(word, "NameFema"		) )	race->name_fema			 =	fread_string( fp );	/* (GRAMM)*/
		else if ( !str_cmp(word, "PointLimit"	) )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
				race->points_limit[x] =											fread_number( fp );
		}
		else if ( !str_cmp(word, "RaceNumber"	) )	race_number	 =				code_code( fp, fread_word(fp), CODE_RACE );
		else if ( !str_cmp(word, "RaceFlags"	) )								fread_to_eol( fp );
		else if ( !str_cmp(word, "Resist"		) )	race->resist =				fread_code_bit( fp, CODE_RIS );
		else if ( !str_cmp(word, "SavingThrow"	) )
		{
			for ( x = 0;  x < MAX_SAVETHROW;  x++ )
				race->saving_throw[x] =											fread_number( fp );
		}
		else if ( !str_cmp(word, "SensPlus") )
		{
			for ( x = 0;  x < MAX_SENSE;  x++ )
				race->sense_plus[x] =											fread_number( fp );
		}
		else if ( !str_cmp(word, "Suscept"		) )	race->suscept			  =	fread_code_bit( fp , CODE_RIS );
		else if ( !str_cmp(word, "ThirstMod"	) )	race->thirst_mod		  =	fread_number( fp );
		else if ( !str_cmp(word, "TonalitySound") )	race->tonality_sound	  = fread_string( fp );
		else if ( !str_cmp(word, "Tonality"		) )	race->tonality =			fread_number( fp );
		else if ( !str_cmp(word, "Weight"		) )	race->weight =				fread_float( fp ) * 1000;	/* converte in grammi prima di rendere intero */
#ifdef T2_ALFA
		else if ( !str_cmp(word, "WearName"		) )	/* (FF) provvisoria e non funzionante */
		{
			if (  < 0 ||  >= MAX_ )
			{
				char *tmp;

				send_log( fp, LOG_FREAD, "fread_race: locazione wear errata o non trovata: %d",  );
				tmp =															fread_string( fp );
				DISPOSE( tmp );
				tmp =															fread_string( fp );
				DISPOSE( tmp );
			}
			else if ( wear < MAX_PART )
			{
				race->wear_name[wear] =											fread_string( fp );
				++wear;
			}
			else
				send_log( fp, LOG_FREAD, "fread_race: Troppe locazioni wear_name." );
		}
#endif
		else
			send_log( fp, LOG_FREAD, "fread_race: non trovato %s alla razza %d.", word, race_number );
	} /* chiude il for */

	/*
	 * Dopo aver caricato la razza la controlla
	 */

	if ( !VALID_STR(race->name_male) || !VALID_STR(race->name_fema) )
	{
		send_log( fp, LOG_FREAD, "fread_race: nome maschile o femminile inesistente alla razza numero %d.",
			race_number );
		exit( EXIT_FAILURE );
	}

	if ( HAS_BIT(race->flags, RACEFLAG_PLAYABLE) && (strlen(race->name_male) > MAX_LEN_NAME_RACE || strlen(race->name_fema) > MAX_LEN_NAME_RACE) )
	{
		send_log( fp, LOG_FREAD, "fread_race: nome maschile o femminile troppo lungo alla razza numero %d.",
			race_number );
		exit( EXIT_FAILURE );
	}

	if ( race_number < 0 || race_number >= MAX_RACE )
	{
		send_log( fp, LOG_FREAD, "fread_race: numero della razza errato: %d", race_number );
		exit( EXIT_FAILURE );
	}

	/* Controlla che i valori della tabella dei codici siano uguali al maschile caricato */
	if ( str_cmp(race->name_male, code_name(NULL, race_number, CODE_RACE)) )
	{
		send_log( fp, LOG_FREAD, "fread_race: nome maschile della razza %s non corrispondende al nome di codice della tabella razza %s",
			race->name_male, code_name(NULL, race_number, CODE_RACE) );
/*		exit( EXIT_FAILURE ); */
	}

	if ( HAS_BIT(race->flags, RACEFLAG_PLAYABLE) && !HAS_BIT(race->lang_natural, LANGUAGE_COMMON) && !HAS_BIT(race->lang_learnable, LANGUAGE_COMMON) )
		send_log( fp, LOG_FREAD, "fread_race: la razza giocabile %s non ha il linguaggio comune settato come naturale o apprendibile, poco friendly", race->name_male );

	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		if ( race->attr_plus[x] < -20 || race->attr_plus[x] > 20 )
		{
			send_log( fp, LOG_FREAD, "fread_race: bonus di attributo errato: %d", race->attr_plus[x] );
			exit( EXIT_FAILURE );
		}
	}

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		if ( !HAS_BIT(race->flags, RACEFLAG_PLAYABLE) )	continue;
		if ( table_language[x]->valid == TRUE )			continue;

		if ( HAS_BIT(race->lang_natural, x) || HAS_BIT(race->lang_learnable, x) )
		{
			send_log( fp, LOG_FREAD, "fread_race: la razza giocabile %s conosce o può apprendere una lingua non valida: %s",
				race->name_male, code_name(fp, x, CODE_LANGUAGE) );
		}
	}

	table_race[race_number] = race;
	if ( HAS_BIT(race->flags, RACEFLAG_PLAYABLE) )
		top_race_play++;
}


/*
 * Carica e controlla tutte le razze
 */
void load_races( void )
{
	int x;

	for ( x = 0;  x < MAX_RACE;  x++ )
		table_race[x] = NULL;

	fread_list( RACE_LIST, "RACE", fread_race, TRUE );

	/*
	 * Controlla le razze
	 */
	if ( top_race_play < 1 || top_race_play > MAX_RACE )
	{
		send_log( NULL, LOG_LOAD, "check_all_races: numero di razze giocabili troppo basso o troppo alto: %d",
			top_race_play );
		exit( EXIT_FAILURE );
	}

	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		if ( !table_race[x] )
		{
			send_log( NULL, LOG_LOAD, "check_all_races: razza numero %d non caricata", x );
			exit( EXIT_FAILURE );
		}
	}
}


/*
 * Libera dalla memoria tutte le razze
 */
void free_all_races( void )
{
	int x;

	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		int y;

		DISPOSE( table_race[x]->name_male );
		DISPOSE( table_race[x]->name_fema );
		free_synonym( table_race[x]->color );
		destroybv( table_race[x]->flags );
		destroybv( table_race[x]->affected );
		destroybv( table_race[x]->resist );
		destroybv( table_race[x]->suscept );
		destroybv( table_race[x]->lang_natural );
		destroybv( table_race[x]->lang_learnable );
		destroybv( table_race[x]->attacks );
		destroybv( table_race[x]->defenses );
		DISPOSE( table_race[x]->tonality_sound );
		for ( y = 0;  y < MAX_WEARLOC;  y++ )
			DISPOSE( table_race[x]->wear_name[y] );
		DISPOSE( table_race[x] );
	}
}


/*
 * Controlla se l'argomento passato sia un nome di una razza
 * Se lo è ritorna la razza altrimenti NULL
 */
RACE_DATA *is_name_race( const char *argument, bool exact )
{
	int x;

	if ( !VALID_STR(argument) )
		return NULL;

	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		if ( !str_cmp(argument, table_race[x]->name_male) )
			return table_race[x];
		if ( !str_cmp(argument, table_race[x]->name_fema) )
			return table_race[x];
	}

	if ( exact )
		return NULL;

	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		if ( !str_prefix(argument, table_race[x]->name_male) )
			return table_race[x];
		if ( !str_prefix(argument, table_race[x]->name_fema) )
			return table_race[x];
	}

	return NULL;
}

/*
 * Da convertire nella code_num quando ci sarà il sistema gramm. maschile/femminile (GR)
 */
int get_race_num( const char *type, bool exact )
{
	RACE_DATA  *race;
	int			x;

	race = is_name_race( type, exact );
	if ( !race )
		return -1;

	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		if ( race == table_race[x] )
			return x;
	}

	return -1;
}


/*
 * Acquisisce la razza di ch
 */
char *get_race( CHAR_DATA *ch, bool color )
{
	static char buf[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_race: ch è NULL" );
		return (color) ? "&wSconosciuta" : "Sconosciuta";
	}

	/* (RR) in realtà bisognerebbe dividere il test per mob e per pg, visto che le razze
	 *	son diverse, controllare anche il playable o no */
	if ( ch->race < 0 && ch->race >= MAX_RACE )
	{
		send_log( NULL, LOG_BUG, "get_race: valore razza errato per il pg %s: %d", ch->name, ch->race );
		return (color) ? "&wSconosciuta" : "Sconosciuta";
	}

	if ( color )
		sprintf( buf, "&%c", get_clr(table_race[ch->race]->color->cat) );
	else
		buf[0] = '\0';

	/* (RR) (GR) cambiarla nella funzione che cerca il nome in base al sesso */
	if ( ch->sex == SEX_FEMALE )
		strcat( buf, table_race[ch->race]->name_fema );
	else
		strcat( buf, table_race[ch->race]->name_male );

	if ( color )
		return strcat( buf, "&w" );
	else
		return buf;
}


/*
 * Comando Admin che mostra le razze
 */
DO_RET do_showrace( CHAR_DATA *ch, char *argument )
{
	RACE_DATA  *race;
	int			x;
	int			ct;

	set_char_color( AT_PLAIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: showrace <race>\r\n" );
		send_to_char( ch, "Le razze in verde chiaro sono giocabili\r\n\r\n" );

		ct = 0;
		for ( x = 0;  x < MAX_RACE;  x++ )
		{
			ct++;

			pager_printf(ch, "%3d] &%c%-20s &w",
				x, (HAS_BIT(table_race[x]->flags, RACEFLAG_PLAYABLE)) ? 'G' : 'w', table_race[x]->name_male );

			if ( (ct%3) == 0 || ct == 0 )
				send_to_pager( ch, "\r\n" );
		}
		send_to_pager( ch, "\r\n" );
		return;
	}

	race = is_name_race( argument, FALSE );
	if ( !race )
	{
		send_to_char( ch, "No such race.\r\n" );
		return;
	}

	ch_printf( ch, "RACE: %s/%s\r\n", race->name_male, race->name_fema );

	send_to_char( ch, "Bonus e malus razziale degli attributi\r\n." );
	ch_printf( ch, "Forza: %-2d\tResistenza: %-2d\tCostituzione: %-2d\r\n",
		race->attr_plus[ATTR_STR], race->attr_plus[ATTR_RES], race->attr_plus[ATTR_CON] );
	ch_printf( ch, "Intelligenza: %-2d\tConcentrazione: %-2d\tVolontà: %-2d\r\n",
		race->attr_plus[ATTR_INT], race->attr_plus[ATTR_COC], race->attr_plus[ATTR_WIL] );
	ch_printf( ch, "Agilità: %-2d\tRiflessi: %-2d\tVelocità: %-2d\r\n",
		race->attr_plus[ATTR_AGI], race->attr_plus[ATTR_RES], race->attr_plus[ATTR_SPE] );
	ch_printf( ch, "Spiritualità: %-2d\tMigrazione: %-2d\tAssorbimento: %-2d\r\n",
		race->attr_plus[ATTR_SPI], race->attr_plus[ATTR_MIG], race->attr_plus[ATTR_ABS] );
	ch_printf( ch, "Empatia: %-2d\tBellezza: %-2d\tComando: %-2d\r\n",
		race->attr_plus[ATTR_EMP], race->attr_plus[ATTR_BEA], race->attr_plus[ATTR_LEA] );

	send_to_char( ch, "Bonus e malus razziale dei sensi." );
	ch_printf( ch, "Vista: %-2d\tUdito: %-2d\tOlfatto:%-2d\r\n",
		race->sense_plus[SENSE_SIGHT], race->sense_plus[SENSE_HEARING], race->sense_plus[SENSE_SMELL] );
	ch_printf( ch, "Gusto: %-2d\tTatto: %-2d\tSesto Senso:%-2d\r\n",
		race->sense_plus[SENSE_TASTE], race->sense_plus[SENSE_TOUCH], race->sense_plus[SENSE_SIXTH] );

	ch_printf( ch, "Limite punteggi:" );
	ch_printf( ch, "Life: %-3d   Mana: %-3d   Move: %-3d   Soul: %-3d\r\n",
		race->points_limit[POINTS_LIFE],
		race->points_limit[POINTS_MANA],
		race->points_limit[POINTS_MOVE],
		race->points_limit[POINTS_SOUL] );

	ch_printf( ch, "AC: %-d  Race Align %d Align Min/Max: %-4d/%-4d Mult: %-d%%\r\n",
		race->ac_plus, race->align_start, race->min_align, race->max_align, race->exp_multiplier );

	send_to_char( ch, "Altezza, peso e tono" );
	ch_printf( ch, "Altezza: %3d cm.\tPeso: %3d kg.\tTonalità: %3d Suono: %s\r\n",
		race->height,
		race->weight,
		race->tonality,
		race->tonality_sound );

	ch_printf( ch, "HungerMod: %d\tThirstMod: %d\r\n", race->hunger_mod, race->thirst_mod );

	ch_printf( ch, "Affected by: %s\r\n", code_bit(NULL, race->affected, CODE_AFFECT) );
	ch_printf( ch, "Resistant to: %s\r\n", code_bit(NULL, race->resist, CODE_RIS) );
	ch_printf( ch, "Susceptible to: %s\r\n", code_bit(NULL, race->suscept, CODE_RIS) );

	ch_printf( ch, "Lanci di salvataggio: (breath) %d  (para & petri) %d  (poison & death) %d  (spell & staff) %d  (wands & rod) %d\r\n",
		race->saving_throw[SAVETHROW_BREATH],
		race->saving_throw[SAVETHROW_PARA_PETRI],
		race->saving_throw[SAVETHROW_POISON_DEATH],
		race->saving_throw[SAVETHROW_SPELL_STAFF],
		race->saving_throw[SAVETHROW_ROD_WANDS] );
/* save vs weapon */

	/* da (GG) */
/*	ch_printf( ch, "Innate: %s\r\n", code_bit(NULL, race->attacks, ATTACK_FLAGS) ); */
}
