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


/* Modulo per la gestione delle parti del corpo. */

#include "mud.h"
#include "calendar.h"
#include "fread.h"
#include "interpret.h"
#include "nanny.h"
#include "partcorp.h"


/*
 * Tabella riguardante le parti del corpo
 * Da spostare tutti gli oggetti vnum qui con le vari descrizioni
 */
const struct part_type table_part[] =
{
	{ { PART_HEAD,			"PART_HEAD",		"" },	12,		"La testa di $n viene tagliata dal suo collo e rotola per terra!",
																"La testa mozzata si stacca dal collo di $n."										},
	{ { PART_ARMS,			"PART_ARMS",		"" },	14,		"La gamba sinistra di $n è stata recisa!",
																"Un braccio mozzato si stacca dal corpo senza vita di $n."							},
	{ { PART_LEGS,			"PART_LEGS",		"" },	15,		"La gamba destra di $n è stata recisa!",
																"Una gamba mozzata si stacca dal corpo senza vita di $n."							},
	{ { PART_HEART,			"PART_HEART",		"" },	13,		"Il cuore $n di è stato strappato fuori dal petto!",
																"Il cuore di $n gli cade da un buco nel petto."										},
	{ { PART_BRAINS,		"PART_BRAINS",		"" },	44,		"Il cervello di $n schizza fuori dal suo stesso cranio!",
																"Il cervello di $n esce grottescamente dal suo cranio."								},
	{ { PART_GUTS,			"PART_GUTS",		"" },	16,		"Le budella di $n si riversano fuori dal suo ventre!",
																"Le budella di $n gli fuoriescono da un taglio all'addome."							},
	{ { PART_HANDS,			"PART_HANDS",		"" },	45,		"La mano di $n viene recisa e cade a terra!",
																"Una mano mozzata si stacca dal corpo sanza vita di $n."							},
	{ { PART_FEET,			"PART_FEET",		"" },	46,		"Un piede di $n viene amputato!",
																"Un piede mozzato si stacca dal corpo senza vita di $n."							},
	{ { PART_FINGERS,		"PART_FINGERS",		"" },	47,		"Un dito di $n è stato reciso dalla sua mano!",
																"Un dito mozzato si stacca dal corpo senza vita di $n."								},
	{ { PART_EAR,			"PART_EAR",			"" },	48,		"L'orecchio destro di $n è stato brutalmente reciso!",
																"Un orecchio mozzato si stacca dal corpo senza vita di $n."							},
	{ { PART_EYE,			"PART_EYE",			"" },	49,		"L'occhio sinistro di $n viene colpito e il bulbo schizza fuori!",
																"Un occhio fuoriesce dall'orbita di $n."											},
	{ { PART_LONG_TONGUE,	"PART_LONG_TONGUE",	"" },	50,		"La lingua di $n viene tagliata violentemente!",
																"La lingua si stacca dalla bocca del corpo senza vita di $n."						},
	{ { PART_EYESTALKS,		"PART_EYESTALKS",	"" },	51,		"Un tentacolo oculare di $n viene reciso e sbattuto via!",
																"Un'antenna mozzata si stacca dal corpo senza vita di $n."							},
	{ { PART_TENTACLES,		"PART_TENTACLES",	"" },	52,		"Un tentacolo di $n viene reciso e sbattuto via!",
																"Un tentacolo mozzato si stacca dal corpo senza vita di $n."						},
	{ { PART_FINS,			"PART_FINS",		"" },	53,		"Una pinna di $n viene recisa e tagliata via!",
																"Una pinna si stacca dal corpo senza vita di $n."									},
	{ { PART_WINGS,			"PART_WINGS",		"" },	54,		"Un'ala di $n viene tagliata via!",
																"Un'ala mozzata si stacca dal corpo senza vita di $n."								},
	{ { PART_TAIL,			"PART_TAIL",		"" },	55,		"La coda di $n viene recisa in un lampo! Si dibatte ancora per qualche istante!",
																"La coda mozzata si stacca dal corpo senza vita di $n."								},
	{ { PART_SCALES,		"PART_SCALES",		"" },	56,		"Una scaglia cade dal corpo di $n!",
																"Una scaglia cade dal corpo di $n."													},
	{ { PART_CLAWS,			"PART_CLAWS",		"" },	59,		"Un artiglio si stacca da $n!",
																"Un artiglio mozzato si stacca dal corpo senza vita di $n."							},
	{ { PART_FANGS,			"PART_FANGS",		"" },	87,		"Le zanne di $n vengono estirpate dalla sua bocca!",
																"Una zanna si stacca dalla bocca del corpo senza vita di $n."						},
	{ { PART_HORNS,			"PART_HORNS",		"" },	58,		"Un corno viene strappato via dal corpo di $n!",
																"Un corno si stacca dal corpo di $n."												},
	{ { PART_TUSKS,			"PART_TUSKS",		"" },	57,		"Una zanna viene tirata via dal corpo di $n!",
																"Una zanna si stacca dal corpo senza vita di $n."									},
	{ { PART_TAILATTACK,	"PART_TAILATTACK",	"" },	55,		"La coda di $n viene recisa dal suo corpo!",
																"La coda mozzata si stacca dal corpo senza vita di $n."								},
	{ { PART_SHARPSCALES,	"PART_SHARPSCALES",	"" },	85,		"Una scaglia ondulata viene via dal corpo di $n!",
																"La cresta cade dal corpo senza vita di $n."										},
	{ { PART_BEAK,			"PART_BEAK",		"" },	84,		"Il becco di $n viene via dal suo corpo!",
																"Il becco mozzato si stacca dal corpo senza vita di $n."							},
	{ { PART_HAUNCH,		"PART_HAUNCH",		"" },	86,		"Le anche di $n vengono strappate via dal suo corpo.",
																"L'anca mozzata si stacca dal corpo senza vita di $n."								},
	{ { PART_HOOVES,		"PART_HOOVES",		"" },	83,		"Uno zoccolo viene via dal corpo di $n.",
																"Uno zoccolo mozzato si stacca dal corpo senza vita di $n."							},
	{ { PART_PAWS,			"PART_PAWS",		"" },	82,		"Un artiglio viene via dal corpo di $n.",
																"Una zampa mozzata si stacca dal corpo senza vita di $n."							},
	{ { PART_FORELEGS,		"PART_FORELEGS",	"" },	81,		"Una zampa vien via dal corpo di $n.",
																"Una zampa anteriore si stacca dal corpo senza vita di $n."							},
	{ { PART_FEATHERS,		"PART_FEATHERS",	"" },	80,		"Qualche piuma cade dal corpo di $n..",
											"Alcune penne cadono dal corpo senza vita di $n."									},
};
const int max_check_part = sizeof(table_part)/sizeof(struct part_type);


/*
 * Tabella parsing sulla tipologia di occhi.
 * Enumerazione.
 */
const CODE_DATA code_eye[] =
{
	{ EYE_NORMAL,	"EYE_NORMAL",	"normali"		},
	{ EYE_NOPUPIL,	"EYE_NOPUPIL",	"senza pupilla"	},
	{ EYE_MIRROR,	"EYE_MIRROR",	"come specchi"	},
	{ EYE_SERPENT,	"EYE_SERPENT",	"da serpente"	}
};
const int max_check_eye = sizeof(code_eye)/sizeof(CODE_DATA);


/*
 * Tabella parsing sulle tipologie di capelli.
 * Enumerazione,
 */
const CODE_DATA code_hairtype[] =
{
	{ HAIRTYPE_SLEEK,	"HAIRTYPE_SLEEK",	"lisci"			},
	{ HAIRTYPE_WAVY,	"HAIRTYPE_WAVY",	"mossi"			},
	{ HAIRTYPE_WAVY2,	"HAIRTYPE_WAVY2",	"ondulati"		},
	{ HAIRTYPE_CURLY,	"HAIRTYPE_CURLY",	"ricci"			},
	{ HAIRTYPE_FRIZZY,	"HAIRTYPE_FRIZZY",	"crespi"		},
	{ HAIRTYPE_RUFFLED,	"HAIRTYPE_RUFFLED",	"arruffati"		},
	{ HAIRTYPE_BALD,	"HAIRTYPE_BALD",	"calvo"			}
};
const int max_check_hairtype = sizeof(code_hairtype)/sizeof(CODE_DATA);


/*
 * Tabella parsing sulla acconciatura dei capelli.
 * Enumerazione.
 */
const CODE_DATA code_hairstyle[] =
{
	{ HAIRSTYLE_NONE,		"HAIRSTYLE_NONE",		"nessuno"	},
	{ HAIRSTYLE_LOOSE,		"HAIRSTYLE_LOOSE",		"sciolti"	},
	{ HAIRSTYLE_PONYTAIL,	"HAIRSTYLE_PONYTAIL",	"coda"    	},
	{ HAIRSTYLE_CHIGNON,	"HAIRSTYLE_CHIGNON",	"chignon"	},
	{ HAIRSTYLE_FORELOCK,	"HAIRSTYLE_FORELOCK",	"ciuffo"	},
	{ HAIRSTYLE_BANG,		"HAIRSTYLE_BANG",		"frangetta"	},
	{ HAIRSTYLE_BOB,		"HAIRSTYLE_BOB",		"caschetto"	},
	{ HAIRSTYLE_PLAIT,		"HAIRSTYLE_PLAIT",		"treccia"	}
};
const int max_check_hairstyle = sizeof(code_hairstyle)/sizeof(CODE_DATA);


/*
 * Controlla se il pg possiede quella parte del corpo.	(ex macro omonima)
 */
bool HAS_BODYPART( CHAR_DATA *ch, int part )
{
	if ( !IS_EMPTY((ch)->xflags) && HAS_BIT((ch)->xflags, (part)) )
		return TRUE;

	return FALSE;
}


/*
 * Crea un occhio sulla base delle info di un pg
 * (RR) Da rifare con le restrizioni razziali.
 */
EYE_PART *init_eye( PG_DATA *pg )
{
	EYE_PART	*eye;

	CREATE( eye, EYE_PART, 1 );

	/* Colore dell'iride */
	CREATE( eye->color, SYNO_DATA, 1 );
	eye->color->cat			= AT_BLACK;
	eye->color->syn			= str_dup( code_name(NULL, AT_BLACK, CODE_COLOR) );

	/* Colore della pupilla  */
	CREATE( eye->pupil_color, SYNO_DATA, 1 );
	eye->pupil_color->cat	= AT_BLACK;
	eye->pupil_color->syn	= str_dup( code_name(NULL, AT_BLACK, CODE_COLOR) );

	/* Colore della luminosità, magica o meno, dell'occhio, NULL inizialmente, prende il colore occhi */
	CREATE( eye->bright_color, SYNO_DATA, 1 );
	eye->bright_color->cat	= AT_NONE;
	eye->bright_color->syn	= str_dup( code_name(NULL, AT_NONE, CODE_COLOR) );

	/* Tipo di occhio */
	CREATE( eye->type, SYNO_DATA, 1 );
	eye->type->cat			= EYE_NORMAL;
	eye->type->syn			= str_dup( code_name(NULL, EYE_NORMAL, CODE_EYE) );

	return eye;
}


/*
 * Crea i capelli sulla base delle info di un pg
 * (RR) Da rifare con le restrizioni razziali.
 */
HAIR_PART *init_hair( PG_DATA *pg )
{
	HAIR_PART	*hair;

	CREATE( hair, HAIR_PART, 1 );

	/* Colore dei capelli */
	CREATE( hair->color, SYNO_DATA, 1 );
	hair->color->cat		= AT_BLACK;
	hair->color->syn		= str_dup( code_name(NULL, AT_BLACK, CODE_COLORHAIR) );

	/* Colore della tinta, inizialmente NULL, prende il colore dei capelli */
	CREATE( hair->dye, SYNO_DATA, 1 );
	hair->dye->cat			= AT_NONE;
	hair->dye->syn			= str_dup( code_name(NULL, AT_NONE, CODE_COLORHAIR) );

	/* Tipo di capelli */
	CREATE( hair->type, SYNO_DATA, 1 );
	hair->type->cat			= HAIRTYPE_SLEEK;
	hair->type->syn			= str_dup( code_name(NULL, HAIRTYPE_SLEEK, CODE_HAIRTYPE) );

	/* Acconciatura dei capelli, inizialmente NULL, prende il tipo di capelli */
	CREATE( hair->style, SYNO_DATA, 1 );
	hair->style->cat		= HAIRSTYLE_NONE;
	hair->style->syn		= str_dup( code_name(NULL, HAIRSTYLE_NONE, CODE_HAIRTYPE) );

	/* Lunghezza dei capelli */
	hair->length	= number_range( 1, 40 );	/* cm */

	return hair;
}


/*
 * Legge da un file una struttura occhio.
 */
EYE_PART *fread_eye( MUD_FILE *fp )
{
	EYE_PART	*eye;

	CREATE( eye, EYE_PART, 1 );

	eye->color			= fread_synonym( fp, CODE_COLOR );
	eye->pupil_color	= fread_synonym( fp, CODE_COLOR );
	eye->bright_color	= fread_synonym( fp, CODE_COLOR );
	eye->type			= fread_synonym( fp, CODE_EYE );

	return eye;
}


/*
 * Scrive su un file una struttura occhio.
 */
void fwrite_eye( EYE_PART *eye, MUD_FILE *fp )
{
	fwrite_synonym( fp, eye->color, CODE_COLOR );
	fwrite_synonym( fp, eye->pupil_color, CODE_COLOR );
	fwrite_synonym( fp, eye->bright_color, CODE_COLOR );
	fwrite_synonym( fp, eye->type, CODE_EYE );
}


/*
 * Libera una struttura di occhio
 */
void free_eye( EYE_PART *eye )
{
	DISPOSE( eye->color->syn );
	DISPOSE( eye->color );

	DISPOSE( eye->pupil_color->syn );
	DISPOSE( eye->pupil_color );

	DISPOSE( eye->type->syn );
	DISPOSE( eye->type );		

	DISPOSE( eye );
}


/*
 * Legge da un file una struttura capelli.
 */
HAIR_PART *fread_hair( MUD_FILE *fp )
{
	HAIR_PART	*hair;

	CREATE( hair, HAIR_PART, 1 );

	hair->color		= fread_synonym( fp, CODE_COLORHAIR );
	hair->dye		= fread_synonym( fp, CODE_COLORHAIR );
	hair->type		= fread_synonym( fp, CODE_HAIRTYPE );
	hair->style		= fread_synonym( fp, CODE_HAIRSTYLE );
	hair->length	= fread_number( fp );

	return hair;
}


/*
 * Scrive su un file una struttura occhio.
 */
void fwrite_hair( HAIR_PART *hair, MUD_FILE *fp )
{
	fwrite_synonym( fp, hair->color, CODE_COLORHAIR );
	fwrite_synonym( fp, hair->dye, CODE_COLORHAIR );
	fwrite_synonym( fp, hair->type, CODE_HAIRTYPE );
	fwrite_synonym( fp, hair->style, CODE_HAIRSTYLE );
	fprintf( fp->file, "    %d\n", hair->length );
}


/*
 * Libera una struttura dei capelli
 */
void free_hair( HAIR_PART *hair )
{
	DISPOSE( hair->color->syn );
	DISPOSE( hair->color );

	DISPOSE( hair->dye->syn );
	DISPOSE( hair->dye );

	DISPOSE( hair->type->syn );
	DISPOSE( hair->type );

	DISPOSE( hair->style->syn );
	DISPOSE( hair->style );

	DISPOSE( hair );
}


char *score_partcorp( CHAR_DATA *ch, int line )
{
	static char	buf[35];

	const char *corp_male[] =
	{
		"        _        ",
		"       ( )       ",
		"     ~ -v- ~     ",
		"    //\\   /\\\\    ",
		"   // |   | \\\\   ",
		"   m' |---| 'm   ",
		"      | | |      ",
		"      |o|o|      ",
		"      | | |      ",
		"       - -       "
	};

	const char *corp_female[] =
	{
		"        _        ",
		"       ( )       ",
		"      ~-v-~      ",
		"    //\\   /\\\\    ",
		"   //  | |  \\\\   ",
		"   m' /---\\ 'm   ",
		"      |   |      ",
		"      |o|o|      ",
		"      | | |      ",
		"       - -       "
	};


	const char *corp_horned_male[] =
	{
		"     (  _  )     ",
		"       ( )       ",
		"     ~ -v- ~     ",
		"    //\\   /\\\\    ",
		"   // |   | \\\\   ",
		"   m' |---| 'm   ",
		"      | | |)     ",
		"      |o|o|(     ",
		"      | | | \"    ",
		"       = =       "
	};

	const char *corp_horned_female[] =
	{
		"     (  _  )     ",
		"       ( )       ",
		"      ~-v-~      ",
		"    //\\   /\\\\    ",
		"   //  | |  \\\\   ",
		"   m' /---\\ 'm   ",
		"      |   |)     ",
		"      |o|o|(     ",
		"      | | | \"    ",
		"       = =       "
	};


	const char *corp_tailed_male[] =
	{
		"        _        ",
		"       ( )       ",
		"     ~ -v- ~     ",
		"    //\\   /\\\\    ",
		"   // |   | \\\\   ",
		"   m' |---| 'm   ",
		"      | | |)     ",
		"      |o|o|(     ",
		"      | | | )    ",
		"       - -       "
	};

	const char *corp_tailed_female[] =
	{
		"        _        ",
		"       ( )       ",
		"      ~-v-~      ",
		"    //\\   /\\\\    ",
		"   //  | |  \\\\   ",
		"   m' /---\\ 'm   ",
		"      |   |)     ",
		"      |o|o|(     ",
		"      | | | )    ",
		"       - -       "
	};

	if ( ch->race == RACE_LIZARDMAN || ch->race == RACE_FELAR )
	{
		if ( ch->sex == SEX_FEMALE )
			strcpy( buf, corp_tailed_female[line] );
		else
			strcpy( buf, corp_tailed_male[line] );
	}
	if ( ch->race == RACE_MINOTAUR )
	{
		if ( ch->sex == SEX_FEMALE )
			strcpy( buf, corp_horned_female[line] );
		else
			strcpy( buf, corp_horned_male[line] );
	}
	else
	{
		/* Altrimenti invia il disegnino normale */
		if ( ch->sex == SEX_FEMALE )
			strcpy( buf, corp_female[line] );
		else
			strcpy( buf, corp_male[line] );
	}

	return buf;
}


/*
 * Update di lunghezza dei capelli, sulla base di quanto il pg ha giocato
 */
void update_hair( void )
{
	DESCRIPTOR_DATA *d;

	for ( d = first_descriptor;  d;  d = d->next )
	{
		CHAR_DATA *ch;

		if ( d->connected != CON_PLAYING )	continue;
		if ( d->character == NULL )			continue;

		ch = get_original( d->character );

		/* Ogni 4 volte al mese allunga di un centimetro il capello del pg */
		if ( (ch->pg->played + (current_time - ch->pg->logon)) % (SECONDS_IN_DAY*7) == 0 )
			ch->pg->hair->length++;
	}
}
