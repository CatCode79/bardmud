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


/* Modulo relativo alle armi */

#include "mud.h"
#include "fread.h"


char *messages_s_blade[24] =
{
	"manchi",
	"graffi lievemente",
	"graffi",
	"graffi profondamente",
	"colpisci leggermente",
	"colpisci",
	"colpisci",
	"colpisci violentemente",
	"colpisci molto violentemente",
	"ferisci leggermente",
	"ferisci",
	"ferisci gravemente",
	"ferisci molto gravemente",
	"ferisci pesantemente",
	"tagli",
	"devasti",
	"trucidi",
	"stermini",
	"MUTILI",
	"SFIGURI",
	"MASSACRI",
	"SVENTRI",
	"* OBLITERI *",
	"*** ANNIENTI ***"
};

char *messages_p_blade[24] =
{
	"manca",
	"graffia lievemente",
	"graffia",
	"graffia profondamente",
	"colpisce leggermente",
	"colpisce",
	"colpisce",
	"colpisce violentemente",
	"colpisce molto violentemente",
	"ferisce leggermente",
	"ferisce",
	"ferisce gravemente",
	"ferisce molto gravemente",
	"ferisce pesantemente",
	"taglia",
	"devasta",
	"trucida",
	"stermina",
	"MUTILA",
	"SFIGURA",
	"MASSACRA",
	"SVENTRA",
	"* OBLITERA *",
	"*** ANNIENTA ***"
};

char *messages_s_blunt[24] =
{
	"manchi",
	"pesti leggermente",
	"pesti",
	"pesti violentemente",
	"percuoti leggermente",
	"percuoti",
	"percuoti",
	"percuoti violentemente",
	"percuoti molto violentemente",
	"fracassi leggermente",
	"fracassi",
	"fracassi violentemente",
	"fracassi molto violentemente",
	"fracassi in pieno",
	"frantumi",
	"devasti",
	"trucidi",
	"stermini",
	"MUTILI",
	"SFIGURI",
	"MASSACRI",
	"POLVERIZZI",
	"* OBLITERI *",
	"*** ANNIENTI ***"
};

char *messages_p_blunt[24] =
{
	"manca",
	"pesta leggermente",
	"pesta",
	"pesta violentemente",
	"percuote leggermente",
	"percuote",
	"percuote",
	"percuote violentemente",
	"percuote molto violentemente",
	"fracassa leggermente",
	"fracassa",
	"fracassa violentemente",
	"fracassa molto violentemente",
	"fracassa in pieno",
	"frantuma",
	"devasta",
	"trucida",
	"stermina",
	"MUTILA",
	"SFIGURA",
	"MASSACRA",
	"POLVERIZZA",
	"* OBLITERA *",
	"*** ANNIENTA ***"
};

char *messages_s_generic[24] =
{
	"manchi",
	"graffi leggermente",
	"graffi",
	"graffi profondamente",
	"colpisci leggermente",
	"colpisci",
	"colpisci",
	"colpisci violentemente",
	"colpisci molto violentemente",
	"ferisci leggermente",
	"ferisci",
	"ferisci gravemente",
	"ferisci molto gravemente",
	"dilani",
	"devasti",
	"stermini",
	"demolisci",
	"MUTILI",
	"MASSACRI",
	"POLVERIZZI",
	"DISTRUGGI",
	"* OBLITERI *",
	"*** ANNIENTI ***",
	"**** DEVASTI ****"
};

char *messages_p_generic[24] =
{
	"manca",
	"graffia leggermente",
	"graffia",
	"graffia profondamente",
	"colpisce leggermente",
	"colpisce",
	"colpisce",
	"colpisce violentemente",
	"colpisce molto violentemente",
	"ferisce leggermente",
	"ferisce",
	"ferisce gravemente",
	"ferisce molto gravemente",
	"dilania",
	"devasta",
	"stermina",
	"demolisce",
	"MUTILA",
	"MASSACRA",
	"POLVERIZZA",
	"DISTRUGGE",
	"* OBLITERA *",
	"*** ANNIENTA ***",
	"**** DEVASTA ****"
};


/*
 * Tabella con le informazioni realtive ai vari danni possibili delle armi
 */
const struct damage_type table_damage[] =
{
	{ { DAMAGE_HIT,		"DAMAGE_HIT",		"colpo"		 },	'M',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_SLICE,	"DAMAGE_SLICE",		"fendente"	 },	'M',	messages_s_blade,		messages_p_blade	},
	{ { DAMAGE_STAB,	"DAMAGE_STAB",		"pugnalata"	 },	'F',	messages_s_blade,		messages_p_blade	},
	{ { DAMAGE_SLASH,	"DAMAGE_SLASH",		"fendente"	 },	'M',	messages_s_blade,		messages_p_blade	},
	{ { DAMAGE_WHIP,	"DAMAGE_WHIP",		"frustata"	 },	'F',	messages_s_blunt,		messages_p_blunt	},
	{ { DAMAGE_CLAW,	"DAMAGE_CLAW",		"artigliata" },	'F',	messages_s_blade,		messages_p_blade	},
	{ { DAMAGE_BLAST,	"DAMAGE_BLAST",		"esplosione" },	'F',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_POUND,	"DAMAGE_POUND",		"martellata" },	'F',	messages_s_blunt,		messages_p_blunt	},
	{ { DAMAGE_CRUSH,	"DAMAGE_CRUSH",		"mazzata"	 },	'F',	messages_s_blunt,		messages_p_blunt	},
	{ { DAMAGE_GREP,	"DAMAGE_GREP",		"presa"		 },	'F',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_BITE,	"DAMAGE_BITE",		"morso"		 },	'M',	messages_s_blade,		messages_p_blade	},
	{ { DAMAGE_PIERCE,	"DAMAGE_PIERCE",	"impatto"	 },	'M',	messages_s_blade,		messages_p_blade	},
	{ { DAMAGE_SUCTION,	"DAMAGE_SUCTION",	"soffio"	 },	'M',	messages_s_blunt,		messages_p_blunt	},
	{ { DAMAGE_BOLT,	"DAMAGE_BOLT",		"fulmine"	 },	'M',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_ARROW,	"DAMAGE_ARROW",		"freccia"	 },	'F',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_DART,	"DAMAGE_DART",		"dardo"		 },	'M',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_STONE,	"DAMAGE_STONE",		"pietra"	 },	'M',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_PEA,		"DAMAGE_PEA",		"beccata"	 },	'M',	messages_s_generic,		messages_p_generic	},
	{ { DAMAGE_NONE,	"DAMAGE_NONE",		"<errore>"	 },	'M',	NULL,					NULL				}
};
const int max_check_damage = sizeof(table_damage)/sizeof(struct damage_type);


/*
 * Tabella con le informazioni relative ai tipi di arma
 * manca fioretto
 */
const CODE_DATA code_weapon[] =
{
	{ WEAPON_DAGGER,		"WEAPON_DAGGER",		"daga"				},
	{ WEAPON_FALCHION,		"WEAPON_FALCHION",		"falchion"			},	/* è abb simile ad una scimitarra a volte, forse da mettere come esotica */
	{ WEAPON_AXE,			"WEAPON_AXE",			"ascia"				},
	{ WEAPON_SCIMITAR,		"WEAPON_SCIMITAR",		"scimitarra"		},
	{ WEAPON_DIRK,			"WEAPON_DIRK",			"spadino"			},	/* è esagerato, cioè già c'è il pugnale che dovrebbe essere invece una daga, bho */
	{ WEAPON_BASTARD,		"WEAPON_BASTARD",		"spadona"			},	/* opzionale due mani se si sceglie di farla bastarda, altrimenti la bastarda manca */
	{ WEAPON_SHORTSWORD,	"WEAPON_SHORTSWORD",	"spada corta"		},
	{ WEAPON_KNUCKLEDUSTER,	"WEAPON_KNUCKLEDUSTER",	"tirapugni"			},
	{ WEAPON_STAFF,			"WEAPON_STAFF",			"bastone"			},
	{ WEAPON_HAMMER,		"WEAPON_HAMMER",		"martello"			},
	{ WEAPON_MACE,			"WEAPON_MACE",			"mazza"				},
	{ WEAPON_FLAIL,			"WEAPON_FLAIL",			"mazzafrusto"		},
	{ WEAPON_WHIP,			"WEAPON_WHIP",			"frusta"			},
	{ WEAPON_BOLA,			"WEAPON_BOLA",			"bola"				},	/* esotica */
	{ WEAPON_COMPOSITEBOW,	"WEAPON_COMPOSITEBOW",	"arco composito"	},
	{ WEAPON_HEAVYCROSSBOW,	"WEAPON_HEAVYCROSSBOW",	"balestra pesante"	},
	{ WEAPON_CROSSBOW,		"WEAPON_CROSSBOW",		"balestra leggera"	},
	{ WEAPON_LONGBOW,		"WEAPON_LONGBOW",		"arco lungo"		},
	{ WEAPON_SHORTBOW,		"WEAPON_SHORTBOW",		"arco corto"		},
	{ WEAPON_SLING,			"WEAPON_SLING",			"fionda"			},
	{ WEAPON_BATTLEAXE,		"WEAPON_BATTLEAXE",		"ascia da battaglia"},	/* due mani */
	{ WEAPON_LEATHERSTRAP,	"WEAPON_LEATHERSTRAP",	"correggia"			},	/* esotica? */
	{ WEAPON_POLE,			"WEAPON_POLE",			"asta"				},
	{ WEAPON_SWORD2H,		"WEAPON_SWORD2H",		"spada a due mani"	},	/* due mani */
	{ WEAPON_JAVELIN,		"WEAPON_JAVELIN",		"giavellotto"		},
	{ WEAPON_LANCE,			"WEAPON_LANCE",			"lancia"			},	/* due mani, opzionale? no lancia corta e scudo */
};
const int max_check_weapon = sizeof(code_weapon)/sizeof(CODE_DATA);


void fread_weapon( MUD_FILE *fp )
{
	fread_skill( fp ,"WEAPON" );
}


/*
 * Carica e controlla tutte le Weapon
 */
void load_weapons( void )
{
	fread_section( WEAPON_FILE,	"WEAPON", fread_weapon, FALSE );
}

