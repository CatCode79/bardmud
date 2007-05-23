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


#include "mud.h"
#include "db.h"
#include "fread.h"
#include "interpret.h"
#include "values.h"
#include "liquid.h"
#include "room.h"


/*
 * Tabella parsing dei tipi di oggetto.
 * Enumerazione.
 * (RR) da tradurre
 */
const CODE_DATA code_objtype[] =
{
	{ OBJTYPE_NONE,				"OBJTYPE_NONE",				"nessuno"				},
	{ OBJTYPE_LIGHT,			"OBJTYPE_LIGHT",			"luce"					},
	{ OBJTYPE_SCROLL,			"OBJTYPE_SCROLL",			"pergamena"				},
	{ OBJTYPE_WAND,				"OBJTYPE_WAND",				"bacchetta"				},
	{ OBJTYPE_STAFF,			"OBJTYPE_STAFF",			"verga"					},
	{ OBJTYPE_WEAPON,			"OBJTYPE_WEAPON",			"arma"					},
	{ OBJTYPE_FIREWEAPON,		"OBJTYPE_FIREWEAPON",		"_arma da fuoco"		},
	{ OBJTYPE_MISSILE,			"OBJTYPE_MISSILE",			"_missile"				},
	{ OBJTYPE_TREASURE,			"OBJTYPE_TREASURE",			"tesoro"				},
	{ OBJTYPE_ARMOR,			"OBJTYPE_ARMOR",			"armatura"				},
	{ OBJTYPE_POTION,			"OBJTYPE_POTION",			"pozione"				},
	{ OBJTYPE_WORN,				"OBJTYPE_WORN",				"_rovinato"				},
	{ OBJTYPE_FURNITURE,		"OBJTYPE_FURNITURE",		"arredamento"			},
	{ OBJTYPE_TRASH,			"OBJTYPE_TRASH",			"spazzatura"			},
	{ OBJTYPE_OLDTRAP,			"OBJTYPE_OLDTRAP",			"_vecchiatrappola"		},
	{ OBJTYPE_CONTAINER,		"OBJTYPE_CONTAINER",		"contenitore"			},
	{ OBJTYPE_NOTE,				"OBJTYPE_NOTE",				"_nota"					},
	{ OBJTYPE_DRINK_CON,		"OBJTYPE_DRINK_CON",		"contenitore liquidi"	},
	{ OBJTYPE_KEY,				"OBJTYPE_KEY",				"chiave"				},
	{ OBJTYPE_FOOD,				"OBJTYPE_FOOD",				"cibo"					},
	{ OBJTYPE_MONEY,			"OBJTYPE_MONEY",			"moneta"				},
	{ OBJTYPE_PEN,				"OBJTYPE_PEN",				"penna"					},
	{ OBJTYPE_BOAT,				"OBJTYPE_BOAT",				"barca"					},
	{ OBJTYPE_CORPSE_MOB,		"OBJTYPE_CORPSE_MOB",		"cadavere"				},
	{ OBJTYPE_CORPSE_PG,		"OBJTYPE_CORPSE_PG",		"cadavere_pg"			},
	{ OBJTYPE_FOUNTAIN,			"OBJTYPE_FOUNTAIN",			"fontana"				},
	{ OBJTYPE_PILL,				"OBJTYPE_PILL",				"pillola"				},
	{ OBJTYPE_BLOOD,			"OBJTYPE_BLOOD",			"sangue"				},
	{ OBJTYPE_BLOODSTAIN,		"OBJTYPE_BLOODSTAIN",		"pozza di sangue"		},
	{ OBJTYPE_SCRAPS,			"OBJTYPE_SCRAPS",			"rottame"				},
	{ OBJTYPE_PIPE,				"OBJTYPE_PIPE",				"pipa"					},
	{ OBJTYPE_HERB_CON,			"OBJTYPE_HERB_CON",			"contenitore erbe"		},
	{ OBJTYPE_HERB,				"OBJTYPE_HERB",				"erba"					},
	{ OBJTYPE_INCENSE,			"OBJTYPE_INCENSE",			"incenso"				},
	{ OBJTYPE_FIRE,				"OBJTYPE_FIRE",				"fuoco"					},
	{ OBJTYPE_BOOK,				"OBJTYPE_BOOK",				"libro"					},
	{ OBJTYPE_SWITCH,			"OBJTYPE_SWITCH",			"interruttore"			},
	{ OBJTYPE_LEVER,			"OBJTYPE_LEVER",			"leva"					},
	{ OBJTYPE_PULLCHAIN,		"OBJTYPE_PULLCHAIN",		"catena"				},
	{ OBJTYPE_BUTTON,			"OBJTYPE_BUTTON",			"bottone"				},
	{ OBJTYPE_DIAL,				"OBJTYPE_DIAL",				"orologio"				},
	{ OBJTYPE_RUNE,				"OBJTYPE_RUNE",				"runa"					},
	{ OBJTYPE_RUNEPOUCH,		"OBJTYPE_RUNEPOUCH",		"borsa di rune"			},
	{ OBJTYPE_MATCH,			"OBJTYPE_MATCH",			"fiammifero"			},
	{ OBJTYPE_TRAP,				"OBJTYPE_TRAP",				"trappola"				},
	{ OBJTYPE_MAP,				"OBJTYPE_MAP",				"mappa"					},
	{ OBJTYPE_PORTAL,			"OBJTYPE_PORTAL",			"portale"				},
	{ OBJTYPE_PAPER,			"OBJTYPE_PAPER",			"carta"					},
	{ OBJTYPE_TINDER,			"OBJTYPE_TINDER",			"acciarino"				},
	{ OBJTYPE_LOCKPICK,			"OBJTYPE_LOCKPICK",			"attrezzo da scasso"	},
	{ OBJTYPE_SPIKE,			"OBJTYPE_SPIKE",			"chiodo"				},
	{ OBJTYPE_DISEASE,			"OBJTYPE_DISEASE",			"malattia"				},
	{ OBJTYPE_OIL,				"OBJTYPE_OIL",				"olio"					},
	{ OBJTYPE_FUEL,				"OBJTYPE_FUEL",				"carburante"			},
	{ OBJTYPE_EMPTY1,			"OBJTYPE_EMPTY1",			"_vuoto1"				},
	{ OBJTYPE_EMPTY2,			"OBJTYPE_EMPTY2",			"_vuoto2"				},
	{ OBJTYPE_MISSILE_WEAPON,	"OBJTYPE_MISSILE_WEAPON",	"lanciamissili"			},
	{ OBJTYPE_PROJECTILE,		"OBJTYPE_PROJECTILE",		"proiettile"			},
	{ OBJTYPE_QUIVER,			"OBJTYPE_QUIVER",			"faretra"				},
	{ OBJTYPE_SHOVEL,			"OBJTYPE_SHOVEL",			"vanga"					},
	{ OBJTYPE_SALVE,			"OBJTYPE_SALVE",			"pomata"				},
	{ OBJTYPE_COOK,				"OBJTYPE_COOK",				"cibo cotto"			},
	{ OBJTYPE_KEYRING,			"OBJTYPE_KEYRING",			"portachiavi"			},
	{ OBJTYPE_ODOR,				"OBJTYPE_ODOR",				"odore"					},
	{ OBJTYPE_CHANCE,			"OBJTYPE_CHANCE",			"portafortuna"			},
	{ OBJTYPE_DRESS,			"OBJTYPE_DRESS",			"abito"					},
	{ OBJTYPE_STATUE,			"OBJTYPE_STATUE",			"statua"				},
	{ OBJTYPE_SHEATH,			"OBJTYPE_SHEATH",			"fodero"				}
#ifdef T2_CHESS
	,
	{ OBJTYPE_CHESS,			"OBJTYPE_CHESS",			"scacchiera"			}
#endif
};
const int max_check_objtype = sizeof(code_objtype)/sizeof(CODE_DATA);


/*
 * Tabella dei tipi di luce
 */
const CODE_DATA code_light[] =
{
	{ LIGHT_FIRE,		"LIGHT_FIRE",		"fuoco"		},
	{ LIGHT_SUN,		"LIGHT_SUN",		"sole"		},
	{ LIGHT_ELECTRICAL,	"LIGHT_ELECTRICAL",	"elettrica"	},
	{ LIGHT_MAGICAL,	"LIGHT_MAGICAL",	"magica"	},
	{ LIGHT_SACRED,		"LIGHT_SACRED",		"sacra"		}
};
const int max_check_light = sizeof(code_light)/sizeof(CODE_DATA);


/*
 * Tabella parsing sulle flag dei contenitori
 */
const CODE_DATA code_container[] =
{
	{ CONTAINER_CLOSEABLE,	"CONTAINER_CLOSEABLE",	"closeable"	},
	{ CONTAINER_PICKPROOF,	"CONTAINER_PICKPROOF",	"pickproof"	},
	{ CONTAINER_CLOSED,		"CONTAINER_CLOSED",		"closed"	},
	{ CONTAINER_LOCKED,		"CONTAINER_LOCKED",		"locked"	},
	{ CONTAINER_EATKEY,		"CONTAINER_EATKEY",		"eatkey"	}
};
const int max_check_container = sizeof(code_container)/sizeof(CODE_DATA);


/*
 * Tabella di parsing per i trigger flag per le leve.
 */
const CODE_DATA code_trig[] =
{
	{ TRIG_UP,			"TRIG_UP",			"up"			},
	{ TRIG_UNLOCK,		"TRIG_UNLOCK",		"unlock"		},
	{ TRIG_LOCK,		"TRIG_LOCK",		"lock"			},
	{ TRIG_D_NORTH,		"TRIG_D_NORTH",		"d_north"		},
	{ TRIG_D_SOUTH,		"TRIG_D_SOUTH",		"d_south"		},
	{ TRIG_D_EAST,		"TRIG_D_EAST",		"d_east"		},
	{ TRIG_D_WEST,		"TRIG_D_WEST",		"d_west"		},
	{ TRIG_D_UP,		"TRIG_D_UP",		"d_up"			},
	{ TRIG_D_DOWN,		"TRIG_D_DOWN",		"d_down"		},
	{ TRIG_DOOR,		"TRIG_DOOR",		"door"			},
	{ TRIG_CONTAINER,	"TRIG_CONTAINER",	"container"		},
	{ TRIG_OPEN,		"TRIG_OPEN",		"open"			},
	{ TRIG_CLOSE,		"TRIG_CLOSE",		"close"			},
	{ TRIG_PASSAGE,		"TRIG_PASSAGE",		"passage"		},
	{ TRIG_OLOAD,		"TRIG_OLOAD",		"oload"			},
	{ TRIG_MLOAD,		"TRIG_MLOAD",		"mload"			},
	{ TRIG_TELEPORT,	"TRIG_TELEPORT",	"teleport"		},
	{ TRIG_TELEPORTALL,	"TRIG_TELEPORTALL",	"teleportall"	},
	{ TRIG_TELEPORTPLUS,"TRIG_TELEPORTPLUS","teleportplus"	},
	{ TRIG_DEATH,		"TRIG_DEATH",		"death"			},
	{ TRIG_CAST,		"TRIG_CAST",		"cast"			},
	{ TRIG_FAKEBLADE,	"TRIG_FAKEBLADE",	"fakeblade"		},
	{ TRIG_RAND4,		"TRIG_RAND4",		"rand4"			},
	{ TRIG_RAND6,		"TRIG_RAND6",		"rand6"			},
	{ TRIG_TRAPDOOR,	"TRIG_TRAPDOOR",	"trapdoor"		},
	{ TRIG_ANOTHEROOM,	"TRIG_ANOTHEROOM",	"anotherroom"	},
	{ TRIG_USEDIAL,		"TRIG_USEDIAL",		"usedial"		},
	{ TRIG_ABSOLUTEVNUM,"TRIG_ABSOLUTEVNUM","absolutevnum"	},
	{ TRIG_SHOWROOMDESC,"TRIG_SHOWROOMDESC","showroomdesc"	},
	{ TRIG_AUTORETURN,	"TRIG_AUTORETURN",	"autoreturn"	}
};
const int max_check_trig = sizeof(code_trig)/sizeof(CODE_DATA);


/*
 * Tabella parsing sulle flag delle trappole.
 */
const CODE_DATA code_trapflag[] =
{
	{ TRAPFLAG_ROOM,		"TRAPFLAG_ROOM",		"room"		},
	{ TRAPFLAG_OBJ,			"TRAPFLAG_OBJ",			"obj"		},
	{ TRAPFLAG_ENTER_ROOM,	"TRAPFLAG_ENTER_ROOM",	"enter"		},
	{ TRAPFLAG_LEAVE_ROOM,	"TRAPFLAG_LEAVE_ROOM",	"leave"		},
	{ TRAPFLAG_OPEN,		"TRAPFLAG_OPEN",		"open"		},
	{ TRAPFLAG_CLOSE,		"TRAPFLAG_CLOSE",		"close"		},
	{ TRAPFLAG_GET,			"TRAPFLAG_GET",			"get"		},
	{ TRAPFLAG_PUT,			"TRAPFLAG_PUT",			"put"		},
	{ TRAPFLAG_PICK,		"TRAPFLAG_PICK",		"pick"		},
	{ TRAPFLAG_UNLOCK,		"TRAPFLAG_UNLOCK",		"unlock"	},
	{ TRAPFLAG_N,			"TRAPFLAG_N",			"north"		},	/* (CC) spostare questi in cima così da gestire le trappole di trap_doo[] con le direzioni magari */
	{ TRAPFLAG_S,			"TRAPFLAG_S",			"south"		},
	{ TRAPFLAG_E,			"TRAPFLAG_E",			"east"		},
	{ TRAPFLAG_W,			"TRAPFLAG_W",			"west"		},
	{ TRAPFLAG_U,			"TRAPFLAG_U",			"up"		},
	{ TRAPFLAG_D,			"TRAPFLAG_D",			"down"		},
	{ TRAPFLAG_EXAMINE,		"TRAPFLAG_EXAMINE",		"examine"	},
	{ TRAPFLAG_NE,			"TRAPFLAG_NE",			"northeast"	},
	{ TRAPFLAG_NW,			"TRAPFLAG_NW",			"northwest"	},
	{ TRAPFLAG_SE,			"TRAPFLAG_SE",			"southeast"	},
	{ TRAPFLAG_SW,			"TRAPFLAG_SW",			"southwest"	}
};
const int max_check_trapflag = sizeof(code_trapflag)/sizeof(CODE_DATA);


/*
 * Comando Admin che permette di visualizzare l'elenco del tipo di oggetto suddiviso nelle aree
 */
DO_RET do_itemtype( CHAR_DATA *ch, char *argument )
{
	AREA_DATA	*area;				/* Area in cui si cerca */
	OBJ_DATA	*obj;				/* Oggetto */
	char		 arg[MIL];			/* Argomento indicante il tipo di oggetto */
	int			 type;				/* Numero di tipo oggetto cercato */
	bool		 inmud = FALSE;		/* Se vero cerca tra gli oggetti caricati nel mud */
	bool		 primary = FALSE;	/* Se vero cerca solo gli oggetti con quel tipo principale */
	bool		 secondary = FALSE;	/* Se vero cerca solo gli oggetti con quel tipo secondario */

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "ch passato è NULL" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		char *cmd;

		cmd = translate_command( ch, "itemtype" );
		pager_printf( ch, "&YSintassi&w:  %s <tipo oggetto> (inmud)\r\n", cmd );
		pager_printf( ch, "&YSintassi&w:  %s <tipo oggetto> primary (inmud)\r\n", cmd );
		pager_printf( ch, "&YSintassi&w:  %s <tipo oggetto> secondary (inmud)\r\n", cmd );

		send_to_pager( ch, "\r\n&WTipi di oggetti&w:\r\n" );
		send_to_pager( ch, code_all(CODE_OBJTYPE, FALSE) );

		return;
	}

	/* Cerca il tipo di oggetto */
	argument = one_argument( argument, arg );
	type = code_num( NULL, arg, CODE_OBJTYPE );
	if ( type == -1 )
	{
		send_to_char( ch, "Tipo di oggetto non valido.\r\n" );
		return;
	}

	if ( nifty_is_name_prefix(argument, "inmud nelmud") )
		inmud = TRUE;
	if ( nifty_is_name_prefix(argument, "primary primario principale") )
		primary = TRUE;
	if ( nifty_is_name_prefix(argument, "secondary secondario") )
		secondary = TRUE;

	if ( primary && secondary )
	{
		send_to_char( ch, "Non si possono inviare contemporaneamente le opzioni di ricerca primary e secondary.\r\n" );
		return;
	}

	send_to_pager( ch, "\r\n" );

	for ( area = first_area;  area;  area = area->next )
	{
		bool found;

		found = FALSE;
		for ( obj = inmud ? first_object : first_object_proto;  obj;  obj = obj->next )
		{
			bool stop;

			if ( obj->vnum < area->vnum_low )
				continue;
			if ( obj->vnum > area->vnum_high )
				continue;

			stop = FALSE;
			/* Salta gli oggetti che non hanno la funzione del tipo scelto */
			if ( !primary && !secondary )
			{
				switch ( type )
				{
				  default:
					send_log( NULL, LOG_BUG, "tipo d'oggetto %d mancante allo switch !primary && !secondary", type );
					return;
				  case OBJTYPE_LIGHT:		if ( !obj->light )		stop = TRUE;	break;
				  case OBJTYPE_SCROLL:		if ( !obj->scroll )		stop = TRUE;	break;
				  case OBJTYPE_WAND:		if ( !obj->wand )		stop = TRUE;	break;
				  case OBJTYPE_STAFF:		if ( !obj->wand )		stop = TRUE;	break;
				  case OBJTYPE_WEAPON:		if ( !obj->weapon )		stop = TRUE;	break;
				  case OBJTYPE_FIREWEAPON:	if ( !obj->weapon )		stop = TRUE;	break;
				  case OBJTYPE_TREASURE:	if ( !obj->treasure )	stop = TRUE;	break;
				  case OBJTYPE_ARMOR:		if ( !obj->armor )		stop = TRUE;	break;
				  case OBJTYPE_POTION:		if ( !obj->potion )		stop = TRUE;	break;
				  case OBJTYPE_FURNITURE:	if ( !obj->furniture )	stop = TRUE;	break;
				  case OBJTYPE_TRASH:		if ( !obj->trash )		stop = TRUE;	break;
				  case OBJTYPE_CONTAINER:	if ( !obj->container )	stop = TRUE;	break;
				  case OBJTYPE_DRINK_CON:	if ( !obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_KEY:			if ( !obj->key )		stop = TRUE;	break;
				  case OBJTYPE_FOOD:		if ( !obj->food )		stop = TRUE;	break;
				  case OBJTYPE_MONEY:		if ( !obj->money )		stop = TRUE;	break;
				  case OBJTYPE_PEN:			if ( !obj->pen )		stop = TRUE;	break;
				  case OBJTYPE_BOAT:		if ( !obj->boat )		stop = TRUE;	break;
				  case OBJTYPE_CORPSE_MOB:	if ( !obj->corpse )		stop = TRUE;	break;
				  case OBJTYPE_CORPSE_PG:	if ( !obj->corpse )		stop = TRUE;	break;
				  case OBJTYPE_FOUNTAIN:	if ( !obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_PILL:		if ( !obj->pill )		stop = TRUE;	break;
				  case OBJTYPE_BLOOD:		if ( !obj->drinkcon )	stop = TRUE;	break;	/* (TT) ma è drink con? */
				  case OBJTYPE_BLOODSTAIN:	if ( !obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_PIPE:		if ( !obj->pipe )		stop = TRUE;	break;
				  case OBJTYPE_HERB_CON:	if ( !obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_HERB:		if ( !obj->herb )		stop = TRUE;	break;
				  case OBJTYPE_SWITCH:		if ( !obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_LEVER:		if ( !obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_PULLCHAIN:	if ( !obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_BUTTON:		if ( !obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_DIAL:		if ( !obj->lever )		stop = TRUE;	break;	/* (TT) ma ha la lever? */
				  case OBJTYPE_TRAP:		if ( !obj->trap )		stop = TRUE;	break;
				  case OBJTYPE_PAPER:		if ( !obj->paper )		stop = TRUE;	break;
				  case OBJTYPE_PROJECTILE:	if ( !obj->weapon )		stop = TRUE;	break;
				  case OBJTYPE_QUIVER:		if ( !obj->container )	stop = TRUE;	break;
				  case OBJTYPE_SALVE:		if ( !obj->salve )		stop = TRUE;	break;
				  case OBJTYPE_COOK:		if ( !obj->food )		stop = TRUE;	break;
				  case OBJTYPE_KEYRING:		if ( !obj->container )	stop = TRUE;	break;
				  case OBJTYPE_DRESS:		if ( !obj->armor )		stop = TRUE;	break;
				  case OBJTYPE_SHEATH:		if ( !obj->sheath )		stop = TRUE;	break;
				}
			}
			else if ( primary )
			{
				switch ( type )
				{
				  default:
					send_log( NULL, LOG_BUG, "tipo d'oggetto %d mancante allo switch primary", type );
					return;
				  case OBJTYPE_LIGHT:		if ( obj->type != OBJTYPE_LIGHT )		stop = TRUE;	break;
				  case OBJTYPE_SCROLL:		if ( obj->type != OBJTYPE_SCROLL )		stop = TRUE;	break;
				  case OBJTYPE_WAND:		if ( obj->type != OBJTYPE_WAND )		stop = TRUE;	break;
				  case OBJTYPE_STAFF:		if ( obj->type != OBJTYPE_STAFF )		stop = TRUE;	break;
				  case OBJTYPE_WEAPON:		if ( obj->type != OBJTYPE_WEAPON )		stop = TRUE;	break;
				  case OBJTYPE_FIREWEAPON:	if ( obj->type != OBJTYPE_FIREWEAPON )	stop = TRUE;	break;
				  case OBJTYPE_TREASURE:	if ( obj->type != OBJTYPE_TREASURE )	stop = TRUE;	break;
				  case OBJTYPE_ARMOR:		if ( obj->type != OBJTYPE_ARMOR )		stop = TRUE;	break;
				  case OBJTYPE_POTION:		if ( obj->type != OBJTYPE_POTION )		stop = TRUE;	break;
				  case OBJTYPE_FURNITURE:	if ( obj->type != OBJTYPE_FURNITURE )	stop = TRUE;	break;
				  case OBJTYPE_TRASH:		if ( obj->type != OBJTYPE_TRASH )		stop = TRUE;	break;
				  case OBJTYPE_CONTAINER:	if ( obj->type != OBJTYPE_CONTAINER )	stop = TRUE;	break;
				  case OBJTYPE_DRINK_CON:	if ( obj->type != OBJTYPE_DRINK_CON )	stop = TRUE;	break;
				  case OBJTYPE_KEY:			if ( obj->type != OBJTYPE_KEY )			stop = TRUE;	break;
				  case OBJTYPE_FOOD:		if ( obj->type != OBJTYPE_FOOD )		stop = TRUE;	break;
				  case OBJTYPE_MONEY:		if ( obj->type != OBJTYPE_MONEY )		stop = TRUE;	break;
				  case OBJTYPE_PEN:		if ( obj->type != OBJTYPE_PEN )		stop = TRUE;	break;
				  case OBJTYPE_BOAT:		if ( obj->type != OBJTYPE_BOAT )		stop = TRUE;	break;
				  case OBJTYPE_CORPSE_MOB:	if ( obj->type != OBJTYPE_CORPSE_MOB )	stop = TRUE;	break;
				  case OBJTYPE_CORPSE_PG:	if ( obj->type != OBJTYPE_CORPSE_PG )	stop = TRUE;	break;
				  case OBJTYPE_FOUNTAIN:	if ( obj->type != OBJTYPE_FOUNTAIN )	stop = TRUE;	break;
				  case OBJTYPE_PILL:		if ( obj->type != OBJTYPE_PILL )		stop = TRUE;	break;
				  case OBJTYPE_BLOOD:		if ( obj->type != OBJTYPE_BLOOD )		stop = TRUE;	break;	/* (TT) ma è drink con? */
				  case OBJTYPE_BLOODSTAIN:	if ( obj->type != OBJTYPE_BLOODSTAIN )	stop = TRUE;	break;
				  case OBJTYPE_PIPE:		if ( obj->type != OBJTYPE_PIPE )		stop = TRUE;	break;
				  case OBJTYPE_HERB_CON:	if ( obj->type != OBJTYPE_HERB_CON )	stop = TRUE;	break;
				  case OBJTYPE_HERB:		if ( obj->type != OBJTYPE_HERB )		stop = TRUE;	break;
				  case OBJTYPE_SWITCH:		if ( obj->type != OBJTYPE_SWITCH )		stop = TRUE;	break;
				  case OBJTYPE_LEVER:		if ( obj->type != OBJTYPE_LEVER )		stop = TRUE;	break;
				  case OBJTYPE_PULLCHAIN:	if ( obj->type != OBJTYPE_PULLCHAIN )	stop = TRUE;	break;
				  case OBJTYPE_BUTTON:		if ( obj->type != OBJTYPE_BUTTON )		stop = TRUE;	break;
				  case OBJTYPE_DIAL:		if ( obj->type != OBJTYPE_DIAL )		stop = TRUE;	break;	/* (TT) ma ha la lever? */
				  case OBJTYPE_TRAP:		if ( obj->type != OBJTYPE_TRAP )		stop = TRUE;	break;
				  case OBJTYPE_PAPER:		if ( obj->type != OBJTYPE_PAPER )		stop = TRUE;	break;
				  case OBJTYPE_PROJECTILE:	if ( obj->type != OBJTYPE_PROJECTILE )	stop = TRUE;	break;
				  case OBJTYPE_QUIVER:		if ( obj->type != OBJTYPE_QUIVER )		stop = TRUE;	break;
				  case OBJTYPE_SALVE:		if ( obj->type != OBJTYPE_SALVE )		stop = TRUE;	break;
				  case OBJTYPE_COOK:		if ( obj->type != OBJTYPE_COOK )		stop = TRUE;	break;
				  case OBJTYPE_KEYRING:		if ( obj->type != OBJTYPE_KEYRING )		stop = TRUE;	break;
				  case OBJTYPE_DRESS:		if ( obj->type != OBJTYPE_DRESS )		stop = TRUE;	break;
				  case OBJTYPE_SHEATH:		if ( obj->type != OBJTYPE_SHEATH )		stop = TRUE;	break;
				}
			}
			else	/* altrimenti il check è secondary a TRUE */
			{
				switch ( type )
				{
				  default:
					send_log( NULL, LOG_BUG, "tipo d'oggetto %d mancante allo switch dell'else", type );
					return;
				  case OBJTYPE_LIGHT:		if ( obj->type == OBJTYPE_LIGHT ||		!obj->light )		stop = TRUE;	break;
				  case OBJTYPE_SCROLL:		if ( obj->type == OBJTYPE_SCROLL ||		!obj->scroll )		stop = TRUE;	break;
				  case OBJTYPE_WAND:		if ( obj->type == OBJTYPE_WAND ||		!obj->wand )		stop = TRUE;	break;
				  case OBJTYPE_STAFF:		if ( obj->type == OBJTYPE_STAFF ||		!obj->wand )		stop = TRUE;	break;
				  case OBJTYPE_WEAPON:		if ( obj->type == OBJTYPE_WEAPON ||		!obj->weapon )		stop = TRUE;	break;
				  case OBJTYPE_FIREWEAPON:	if ( obj->type == OBJTYPE_FIREWEAPON ||	!obj->weapon )		stop = TRUE;	break;
				  case OBJTYPE_TREASURE:	if ( obj->type == OBJTYPE_TREASURE ||	!obj->treasure )	stop = TRUE;	break;
				  case OBJTYPE_ARMOR:		if ( obj->type == OBJTYPE_ARMOR ||		!obj->armor )		stop = TRUE;	break;
				  case OBJTYPE_POTION:		if ( obj->type == OBJTYPE_POTION ||		!obj->potion )		stop = TRUE;	break;
				  case OBJTYPE_FURNITURE:	if ( obj->type == OBJTYPE_FURNITURE ||	!obj->furniture )	stop = TRUE;	break;
				  case OBJTYPE_TRASH:		if ( obj->type == OBJTYPE_TRASH ||		!obj->trash )		stop = TRUE;	break;
				  case OBJTYPE_CONTAINER:	if ( obj->type == OBJTYPE_CONTAINER ||	!obj->container )	stop = TRUE;	break;
				  case OBJTYPE_DRINK_CON:	if ( obj->type == OBJTYPE_DRINK_CON ||	!obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_KEY:			if ( obj->type == OBJTYPE_KEY ||		!obj->key )			stop = TRUE;	break;
				  case OBJTYPE_FOOD:		if ( obj->type == OBJTYPE_FOOD ||		!obj->food )		stop = TRUE;	break;
				  case OBJTYPE_MONEY:		if ( obj->type == OBJTYPE_MONEY ||		!obj->money )		stop = TRUE;	break;
				  case OBJTYPE_PEN:		if ( obj->type == OBJTYPE_PEN ||		!obj->pen )		stop = TRUE;	break;
				  case OBJTYPE_BOAT:		if ( obj->type == OBJTYPE_BOAT ||		!obj->boat )		stop = TRUE;	break;
				  case OBJTYPE_CORPSE_MOB:	if ( obj->type == OBJTYPE_CORPSE_MOB ||	!obj->corpse )		stop = TRUE;	break;
				  case OBJTYPE_CORPSE_PG:	if ( obj->type == OBJTYPE_CORPSE_PG ||	!obj->corpse )		stop = TRUE;	break;
				  case OBJTYPE_FOUNTAIN:	if ( obj->type == OBJTYPE_FOUNTAIN ||	!obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_PILL:		if ( obj->type == OBJTYPE_PILL ||		!obj->pill )		stop = TRUE;	break;
				  case OBJTYPE_BLOOD:		if ( obj->type == OBJTYPE_BLOOD ||		!obj->drinkcon )	stop = TRUE;	break;	/* (TT) ma è drink con? */
				  case OBJTYPE_BLOODSTAIN:	if ( obj->type == OBJTYPE_BLOODSTAIN ||	!obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_PIPE:		if ( obj->type == OBJTYPE_PIPE ||		!obj->pipe )		stop = TRUE;	break;
				  case OBJTYPE_HERB_CON:	if ( obj->type == OBJTYPE_HERB_CON ||	!obj->drinkcon )	stop = TRUE;	break;
				  case OBJTYPE_HERB:		if ( obj->type == OBJTYPE_HERB ||		!obj->herb )		stop = TRUE;	break;
				  case OBJTYPE_SWITCH:		if ( obj->type == OBJTYPE_SWITCH ||		!obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_LEVER:		if ( obj->type == OBJTYPE_LEVER ||		!obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_PULLCHAIN:	if ( obj->type == OBJTYPE_PULLCHAIN ||	!obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_BUTTON:		if ( obj->type == OBJTYPE_BUTTON ||		!obj->lever )		stop = TRUE;	break;
				  case OBJTYPE_DIAL:		if ( obj->type == OBJTYPE_DIAL ||		!obj->lever )		stop = TRUE;	break;	/* (TT) ma ha la lever? */
				  case OBJTYPE_TRAP:		if ( obj->type == OBJTYPE_TRAP ||		!obj->trap )		stop = TRUE;	break;
				  case OBJTYPE_PAPER:		if ( obj->type == OBJTYPE_PAPER ||		!obj->paper )		stop = TRUE;	break;
				  case OBJTYPE_PROJECTILE:	if ( obj->type == OBJTYPE_PROJECTILE ||	!obj->weapon )		stop = TRUE;	break;
				  case OBJTYPE_QUIVER:		if ( obj->type == OBJTYPE_QUIVER ||		!obj->container )	stop = TRUE;	break;
				  case OBJTYPE_SALVE:		if ( obj->type == OBJTYPE_SALVE ||		!obj->salve )		stop = TRUE;	break;
				  case OBJTYPE_COOK:		if ( obj->type == OBJTYPE_COOK ||		!obj->food )		stop = TRUE;	break;
				  case OBJTYPE_KEYRING:		if ( obj->type == OBJTYPE_KEYRING ||	!obj->container )	stop = TRUE;	break;
				  case OBJTYPE_DRESS:		if ( obj->type == OBJTYPE_DRESS ||		!obj->armor )		stop = TRUE;	break;
				  case OBJTYPE_SHEATH:		if ( obj->type == OBJTYPE_SHEATH ||		!obj->sheath )		stop = TRUE;	break;
				}
			}
			if ( stop )
				continue;

			if ( !found )
			{
				found = TRUE;
				pager_printf( ch, "&w- Oggetti con funzione di &W%s&w nell'area &W%s&w\r\n", code_name(NULL, type, CODE_OBJTYPE), area->name );
				send_to_pager( ch, "&gVnum  Short Descr                              Tipo originale&w\r\n" );
			}

			/* (TT) check aggiuntivi per armi e armature per poterle bilanciare meglio */
			if ( type == OBJTYPE_ARMOR && obj->armor )
			{
				pager_printf( ch, "%5d %-40.40s %15.15s %3d %3d\r\n",
					obj->vnum,
					obj->short_descr,
					code_name(NULL, obj->type, CODE_OBJTYPE),
					obj->level,
					obj->armor->ac_orig );
			}
			else if ( type == OBJTYPE_WEAPON && obj->weapon )
			{
				pager_printf( ch, "%5d %-40.40s %15.15s %3d %3d %3d\r\n",
					obj->vnum,
					obj->short_descr,
					code_name(NULL, obj->type, CODE_OBJTYPE),
					obj->level,
					obj->weapon->dice_number,
					obj->weapon->dice_size );
			}
			else
			{
				pager_printf( ch, "%5d %-40.40s %15.15s% 3d\r\n",
					obj->vnum,
					obj->short_descr,
					code_name(NULL, obj->type, CODE_OBJTYPE),
					obj->level );
			}
		}

		if ( found )
			send_to_pager( ch, "\r\n" );
	}
}


/*
 * Inizializza i value di un tipo di oggetto passato
 */
void init_values( OBJ_DATA *obj, const int type )
{
	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "init_values: obj è NULL" );
		return;
	}

	switch ( type )
	{
	  default:
	  case OBJTYPE_NONE:
		send_log( NULL, LOG_BUG, "init_values: typo passato errato: %d", type );
		return;

	  /* Questi tipi di oggetti non serve inizializzarli quindi esce */
	  case OBJTYPE_WORN:
  	  case OBJTYPE_TRASH:
	  case OBJTYPE_OLDTRAP:
	  case OBJTYPE_NOTE:
	  case OBJTYPE_SCRAPS:
	  case OBJTYPE_INCENSE:
	  case OBJTYPE_FIRE:
	  case OBJTYPE_BOOK:
	  case OBJTYPE_DIAL:
	  case OBJTYPE_RUNE:
	  case OBJTYPE_RUNEPOUCH:
	  case OBJTYPE_MATCH:
	  case OBJTYPE_MAP:
	  case OBJTYPE_PORTAL:
	  case OBJTYPE_TINDER:
	  case OBJTYPE_LOCKPICK:
	  case OBJTYPE_SPIKE:
	  case OBJTYPE_DISEASE:
	  case OBJTYPE_OIL:
	  case OBJTYPE_FUEL:
	  case OBJTYPE_EMPTY1:
	  case OBJTYPE_EMPTY2:
	  case OBJTYPE_SHOVEL:
	  case OBJTYPE_ODOR:
	  case OBJTYPE_CHANCE:
	  case OBJTYPE_STATUE:
#ifdef T2_CHESS
	  case OBJTYPE_CHESS:
#endif
		return;

	  case OBJTYPE_LIGHT:
		if ( obj->light )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->light, VAL_LIGHT_DATA, 1 );
		obj->light->intensity			= 0;
		obj->light->type				= LIGHT_FIRE;
		obj->light->hours				= 0;
		obj->light->act_light			= str_dup( "" );
		obj->light->act_light_room		= str_dup( "" );
		obj->light->act_dark			= str_dup( "" );
		obj->light->act_dark_room		= str_dup( "" );
		obj->light->act_consume			= str_dup( "" );
		obj->light->act_consume_room	= str_dup( "" );
		return;

	  case OBJTYPE_SCROLL:
		if ( obj->scroll )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->scroll, VAL_SCROLL_DATA, 1 );
		obj->scroll->level				= 0;
		obj->scroll->sn1				= -1;
		obj->scroll->sn2				= -1;
		obj->scroll->sn3				= -1;
		obj->scroll->act_use			= str_dup( "" );
		obj->scroll->act_use_room		= str_dup( "" );
		return;

	  case OBJTYPE_POTION:
		if ( obj->potion )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->potion, VAL_POTION_DATA, 1 );
		obj->potion->level				= 0;
		obj->potion->sn1				= -1;
		obj->potion->sn2				= -1;
		obj->potion->sn3				= -1;
		obj->potion->act_use			= str_dup( "" );
		obj->potion->act_use_room		= str_dup( "" );
		return;

	  case OBJTYPE_PILL:
		if ( obj->pill )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->pill, VAL_PILL_DATA, 1 );
		obj->pill->level				= 0;
		obj->pill->sn1					= -1;
		obj->pill->sn2					= -1;
		obj->pill->sn3					= -1;
		obj->pill->act_use				= str_dup( "" );
		obj->pill->act_use_room			= str_dup( "" );
		return;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		if ( obj->wand )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->wand, VAL_WAND_DATA, 1 );
		obj->wand->level				= 0;
		obj->wand->max_charges			= 0;
		obj->wand->curr_charges			= 0;
		obj->wand->sn					= -1;
		obj->wand->act_zap				= str_dup( "" );
		obj->wand->act_zap_room			= str_dup( "" );
		obj->wand->act_consume			= str_dup( "" );
		obj->wand->act_consume_room		= str_dup( "" );
		return;

	  case OBJTYPE_WEAPON:
	  case OBJTYPE_FIREWEAPON:
	  case OBJTYPE_MISSILE_WEAPON:
	  case OBJTYPE_PROJECTILE:
		if ( obj->weapon )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->weapon, VAL_WEAPON_DATA, 1 );
		obj->weapon->condition			= 0;
		obj->weapon->dice_number		= 0;
		obj->weapon->dice_size			= 0;
		obj->weapon->type				= WEAPON_DAGGER;
		obj->weapon->damage1			= DAMAGE_NONE;
		obj->weapon->damage2			= DAMAGE_NONE;
		obj->weapon->lvl_disarm			= 0;
		obj->weapon->act_wield			= str_dup( "" );
		obj->weapon->act_wield_room		= str_dup( "" );
		return;

	  case OBJTYPE_TREASURE:
		if ( obj->treasure )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->treasure, VAL_TREASURE_DATA, 1 );
		obj->treasure->type				= 0;
		obj->treasure->condition		= 0;
		return;

	  case OBJTYPE_DRESS:
	  case OBJTYPE_ARMOR:
		if ( obj->armor )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->armor, VAL_ARMOR_DATA, 1 );
		obj->armor->ac					= 0;
		obj->armor->ac_orig				= 0;
		obj->armor->type				= 0;
		obj->armor->act_wear			= str_dup( "" );
		obj->armor->act_wear_room		= str_dup( "" );
		return;

	  case OBJTYPE_FURNITURE:
	  {
		int x;

		if ( obj->furniture )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->furniture, VAL_FURNITURE_DATA, 1 );
		for ( x = 0;  x < MAX_POINTS;  x++ )
			obj->furniture->regain[x]		= 0;
	  }
		return;

	  case OBJTYPE_CONTAINER:
	  case OBJTYPE_QUIVER:
	  case OBJTYPE_KEYRING:
		if ( obj->container )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->container, VAL_CONTAINER_DATA, 1 );
		obj->container->capacity		= 0;
		obj->container->key				= 0;
		obj->container->durability		= 0;
		obj->container->type			= OBJTYPE_NONE;
		obj->container->flags			= 0;
		obj->container->act_open		= str_dup( "" );
		obj->container->act_open_room	= str_dup( "" );
		obj->container->act_close		= str_dup( "" );
		obj->container->act_close_room	= str_dup( "" );
		return;

	  case OBJTYPE_DRINK_CON:
	  case OBJTYPE_BLOOD:
	  case OBJTYPE_BLOODSTAIN:
	  case OBJTYPE_FOUNTAIN:
	  case OBJTYPE_HERB_CON:
		if ( obj->drinkcon )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->drinkcon, VAL_DRINKCON_DATA, 1 );
		obj->drinkcon->capacity			= 0;
		obj->drinkcon->curr_quant		= 0;
		obj->drinkcon->liquid			= 0;
		obj->drinkcon->poison			= 0;
		obj->drinkcon->fountain			= FALSE;
		obj->drinkcon->act_drink		= str_dup( "" );
		obj->drinkcon->act_drink_room	= str_dup( "" );
		return;

	  case OBJTYPE_KEY:
		if ( obj->key )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->key, VAL_KEY_DATA, 1 );
		obj->key->save					= FALSE;
		obj->key->act_unlock			= str_dup( "" );
		obj->key->act_unlock_room		= str_dup( "" );
		obj->key->act_lock				= str_dup( "" );
		obj->key->act_lock_room			= str_dup( "" );
		return;

	  case OBJTYPE_FOOD:
	  case OBJTYPE_COOK:
		if ( obj->food )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->food, VAL_FOOD_DATA, 1 );
		obj->food->hours				= 0;
		obj->food->condition			= 0;
		obj->food->dam_cook				= 0;
		obj->food->poison				= 0;
		obj->food->init_cond			= 0;
		obj->food->flags				= 0;
		obj->food->act_eat				= str_dup( "" );
		obj->food->act_eat_room			= str_dup( "" );
		return;

	  case OBJTYPE_MONEY:
		if ( obj->money )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->money, VAL_MONEY_DATA, 1 );
		obj->money->qnt					= 0;
		obj->money->qnt_recycle			= 0;
		obj->money->type				= 0;
		return;

	  case OBJTYPE_PEN:
		if ( obj->pen )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->pen, VAL_PEN_DATA, 1 );
		obj->pen->ink					= 0;
		obj->pen->act_write				= str_dup( "" );
		obj->pen->act_write_room		= str_dup( "" );
		return;

	  case OBJTYPE_BOAT:
		if ( obj->boat )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->boat, VAL_BOAT_DATA, 1 );
		obj->boat->rooms				= 0;
		obj->boat->type					= 0;
		obj->boat->act_travel			= str_dup( "" );
		obj->boat->act_travel_room		= str_dup( "" );
		return;

	  case OBJTYPE_CORPSE_MOB:
	  case OBJTYPE_CORPSE_PG:
		if ( obj->corpse )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->corpse, VAL_CORPSE_DATA, 1 );
		obj->corpse->time				= 0;
		obj->corpse->tick				= 0;
		obj->corpse->level				= 0;
		obj->corpse->looted				= 0;
		obj->corpse->pg					= FALSE;
		return;

	  case OBJTYPE_PIPE:
		if ( obj->pipe )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->pipe, VAL_PIPE_DATA, 1 );
		obj->pipe->capacity				= 0;
		obj->pipe->draws				= 0;
		obj->pipe->herb					= 0;
		obj->pipe->flags				= 0;
		obj->pipe->act_smoke			= str_dup( "" );
		obj->pipe->act_smoke_room		= str_dup( "" );
		return;

	  case OBJTYPE_HERB:
		if ( obj->herb )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->herb, VAL_HERB_DATA, 1 );
		obj->herb->charges				= 0;
		obj->herb->hnum					= 0;
		return;

	  case OBJTYPE_SWITCH:
	  case OBJTYPE_LEVER:
	  case OBJTYPE_PULLCHAIN:
	  case OBJTYPE_BUTTON:
		if ( obj->lever )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->lever, VAL_LEVER_DATA, 1 );
		obj->lever->flags				= 0;
		obj->lever->vnum_sn				= 0;
		obj->lever->vnum				= 0;
		obj->lever->vnum_val			= 0;
		obj->lever->act_use				= str_dup( "" );
		obj->lever->act_use_room		= str_dup( "" );
		return;

	  case OBJTYPE_TRAP:
		if ( obj->trap )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->trap, VAL_TRAP_DATA, 1 );
		obj->trap->charges				= 0;
		obj->trap->type					= 0;
		obj->trap->level				= 0;
		obj->trap->flags				= 0;
		obj->trap->act_activate			= str_dup( "" );
		obj->trap->act_activate_room	= str_dup( "" );
		return;

	  case OBJTYPE_PAPER:
		if ( obj->paper )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->paper, VAL_PAPER_DATA, 1 );
		obj->paper->text				= 0;
		obj->paper->subject				= 0;
		obj->paper->to					= 0;
		obj->paper->type				= 0;
		return;

	  case OBJTYPE_SALVE:
		if ( obj->salve )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->salve, VAL_SALVE_DATA, 1 );
		obj->salve->level				= 0;
		obj->salve->charges				= 0;
		obj->salve->max_charges			= 0;
		obj->salve->delay				= 0;
		obj->salve->sn1					= 0;
		obj->salve->sn2					= 0;
		obj->salve->act_apply			= str_dup( "" );
		obj->salve->act_apply_room		= str_dup( "" );
		return;

	  case OBJTYPE_SHEATH:
		if ( obj->sheath )
		{
			send_log( NULL, LOG_BUG, "init_values: oggetto %d già con tipologia di %s",
				obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
			return;
		}

		CREATE( obj->sheath, VAL_SHEATH_DATA, 1 );
		obj->sheath->weapon_type		= WEAPON_DAGGER;
		obj->sheath->vnum				= 0;
		obj->sheath->flags				= 0;
		obj->sheath->act_open			= str_dup( "" );
		obj->sheath->act_open_room		= str_dup( "" );
		obj->sheath->act_close			= str_dup( "" );
		obj->sheath->act_close_room		= str_dup( "" );
		return;
	};
}


/*
 * Libera la struttura di un tipo determinato di oggetto passato
 * Serve sia per oggetti prototipo che non
 */
void free_values( OBJ_DATA *obj, const int type )
{
	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "free_itemtype: obj è NULL" );
		return;
	}

	switch ( type )
	{
	  default:
	  case OBJTYPE_NONE:
		send_log( NULL, LOG_BUG, "free_values: oggetto (#%d) di tipo errato: %s",
			obj->vnum, code_str(NULL, type, CODE_OBJTYPE) );
		return;

	  /* Questi tipi di oggetti non serve liberare la memoria quindi esce */
	  case OBJTYPE_WORN:
	  case OBJTYPE_TRASH:
	  case OBJTYPE_OLDTRAP:
	  case OBJTYPE_NOTE:
	  case OBJTYPE_SCRAPS:
	  case OBJTYPE_INCENSE:
	  case OBJTYPE_FIRE:
	  case OBJTYPE_BOOK:
	  case OBJTYPE_DIAL:
	  case OBJTYPE_RUNE:
	  case OBJTYPE_RUNEPOUCH:
	  case OBJTYPE_MATCH:
	  case OBJTYPE_MAP:
	  case OBJTYPE_PORTAL:
	  case OBJTYPE_TINDER:
	  case OBJTYPE_LOCKPICK:
	  case OBJTYPE_SPIKE:
	  case OBJTYPE_DISEASE:
	  case OBJTYPE_OIL:
	  case OBJTYPE_FUEL:
	  case OBJTYPE_EMPTY1:
	  case OBJTYPE_EMPTY2:
	  case OBJTYPE_SHOVEL:
	  case OBJTYPE_ODOR:
	  case OBJTYPE_CHANCE:
	  case OBJTYPE_STATUE:
#ifdef T2_CHESS
	  case OBJTYPE_CHESS:
#endif
		return;

	  case OBJTYPE_LIGHT:
		if ( !obj->light )
			return;
		DISPOSE( obj->light->act_light );
		DISPOSE( obj->light->act_light_room );
		DISPOSE( obj->light->act_dark );
		DISPOSE( obj->light->act_dark_room );
		DISPOSE( obj->light->act_consume );
		DISPOSE( obj->light->act_consume_room );
		DISPOSE( obj->light );
		return;

	  case OBJTYPE_SCROLL:
		if ( !obj->scroll )
			return;
		DISPOSE( obj->scroll->act_use );
		DISPOSE( obj->scroll->act_use_room );
		DISPOSE( obj->scroll );
		return;

	  case OBJTYPE_POTION:
		if ( !obj->potion )
			return;
		DISPOSE( obj->potion->act_use );
		DISPOSE( obj->potion->act_use_room );
		DISPOSE( obj->potion );
		return;

	  case OBJTYPE_PILL:
		if ( !obj->pill )
			return;
		DISPOSE( obj->pill->act_use );
		DISPOSE( obj->pill->act_use_room );
		DISPOSE( obj->pill );
		return;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		if ( !obj->wand )
			return;
		DISPOSE( obj->wand->act_zap );
		DISPOSE( obj->wand->act_zap_room );
		DISPOSE( obj->wand->act_consume );
		DISPOSE( obj->wand->act_consume_room );
		DISPOSE( obj->wand );
		return;

	  case OBJTYPE_WEAPON:
	  case OBJTYPE_FIREWEAPON:
	  case OBJTYPE_MISSILE:	/* (TT) */
	  case OBJTYPE_PROJECTILE:
	  case OBJTYPE_MISSILE_WEAPON:
		if ( !obj->weapon )
			return;
		DISPOSE( obj->weapon->act_wield );
		DISPOSE( obj->weapon->act_wield_room );
		DISPOSE( obj->weapon );
		return;

	  case OBJTYPE_TREASURE:
		if ( !obj->treasure )
			return;
		DISPOSE( obj->treasure );
		return;

	  case OBJTYPE_DRESS:
	  case OBJTYPE_ARMOR:
		if ( !obj->armor )
			return;
		DISPOSE( obj->armor->act_wear );
		DISPOSE( obj->armor->act_wear_room );
		DISPOSE( obj->armor );
		return;

	  case OBJTYPE_FURNITURE:
		if ( !obj->furniture )
			return;
		DISPOSE( obj->furniture );
		return;

	  case OBJTYPE_CONTAINER:
	  case OBJTYPE_QUIVER:
	  case OBJTYPE_KEYRING:
		if ( !obj->container )
			return;
		DISPOSE( obj->container->act_open );
		DISPOSE( obj->container->act_open_room );
		DISPOSE( obj->container->act_close );
		DISPOSE( obj->container->act_close_room );
		DISPOSE( obj->container );
		return;

	  case OBJTYPE_DRINK_CON:
  	  case OBJTYPE_FOUNTAIN:
	  case OBJTYPE_BLOOD:
	  case OBJTYPE_BLOODSTAIN:	/* (TT) */
	  case OBJTYPE_HERB_CON:
		if ( !obj->drinkcon )
			return;
		DISPOSE( obj->drinkcon->act_drink );
		DISPOSE( obj->drinkcon->act_drink_room );
		DISPOSE( obj->drinkcon );
		return;

	  case OBJTYPE_KEY:
		if ( !obj->key )
			return;
		DISPOSE( obj->key->act_unlock );
		DISPOSE( obj->key->act_unlock_room );
		DISPOSE( obj->key->act_lock );
		DISPOSE( obj->key->act_lock_room );
		DISPOSE( obj->key );
		return;

	  case OBJTYPE_FOOD:
	  case OBJTYPE_COOK:
		if ( !obj->food )
			return;
		DISPOSE( obj->food->act_eat );
		DISPOSE( obj->food->act_eat_room );
		DISPOSE( obj->food );
		return;

	  case OBJTYPE_MONEY:
		if ( !obj->money )
			return;
		DISPOSE( obj->money );
		return;

	  case OBJTYPE_PEN:
		if ( !obj->pen )
			return;
		DISPOSE( obj->pen->act_write );
		DISPOSE( obj->pen->act_write_room );
		DISPOSE( obj->pen );
		return;

	  case OBJTYPE_BOAT:
		if ( !obj->boat )
			return;
		DISPOSE( obj->boat->act_travel );
		DISPOSE( obj->boat->act_travel_room );
		DISPOSE( obj->boat );
		return;

	  case OBJTYPE_CORPSE_MOB:
	  case OBJTYPE_CORPSE_PG:
		if ( !obj->corpse )
			return;
		DISPOSE( obj->corpse );
		return;

	  case OBJTYPE_PIPE:
		if ( !obj->pipe )
			return;
		DISPOSE( obj->pipe->act_smoke );
		DISPOSE( obj->pipe->act_smoke_room );
		DISPOSE( obj->pipe );
		return;

	  case OBJTYPE_HERB:
		if ( !obj->herb )
			return;
		DISPOSE( obj->herb );
		return;

	  case OBJTYPE_SWITCH:
	  case OBJTYPE_LEVER:
	  case OBJTYPE_PULLCHAIN:
	  case OBJTYPE_BUTTON:
		if ( !obj->lever )
			return;
		DISPOSE( obj->lever->act_use );
		DISPOSE( obj->lever->act_use_room );
		DISPOSE( obj->lever );
		return;

	  case OBJTYPE_TRAP:
		if ( !obj->trap )
			return;
		DISPOSE( obj->trap->act_activate );
		DISPOSE( obj->trap->act_activate_room );
		DISPOSE( obj->trap );
		return;

	  /* (FF) mi sa che usero il note, così paper è senza struttura, note questo, e scroll deriva dal paper, come anche le pagine del futuro book */
	  case OBJTYPE_PAPER:
		if ( !obj->paper )
			return;
		DISPOSE( obj->paper );
		return;

	  case OBJTYPE_SALVE:
		if ( !obj->salve )
			return;
		DISPOSE( obj->salve->act_apply );
		DISPOSE( obj->salve->act_apply_room );
		DISPOSE( obj->salve );
		return;

	  case OBJTYPE_SHEATH:
		if ( !obj->sheath )
			return;
		DISPOSE( obj->sheath->act_open );
		DISPOSE( obj->sheath->act_open_room );
		DISPOSE( obj->sheath->act_close );
		DISPOSE( obj->sheath->act_close_room );
		DISPOSE( obj->sheath );
		return;
	}
}


/*
 * Legge la linea dei value di un oggetto da un file di area
 * (FF) farla con passaggio di type di oggetto dopo la lettura della etichetta
 */
void fread_values_area( AREA_DATA *tarea, MUD_FILE *fp, OBJ_DATA *obj )
{
	char   *ln;
	int		x[6];

	ln = fread_line( fp );
	x[0] = x[1] = x[2] = x[3] = x[4] = x[5] = 0;
	sscanf( ln, "%d %d %d %d %d %d",
		&x[0], &x[1], &x[2], &x[3], &x[4], &x[5] );

	switch ( obj->type )
	{
	  default:
	  case OBJTYPE_NONE:
		send_log( NULL, LOG_BUG, "fread_values_area: tipo di oggetto (#%d) passato errato: %s",
			obj->vnum, code_str(fp, obj->type, CODE_OBJTYPE) );
		return;

	  /* Questi tipi di oggetti non serve liberare la memoria quindi esce */
	  case OBJTYPE_WORN:
  	  case OBJTYPE_TRASH:
	  case OBJTYPE_OLDTRAP:
	  case OBJTYPE_NOTE:
	  case OBJTYPE_KEY:
	  case OBJTYPE_BOAT:
	  case OBJTYPE_SCRAPS:
	  case OBJTYPE_INCENSE:
	  case OBJTYPE_FIRE:
	  case OBJTYPE_BOOK:
	  case OBJTYPE_DIAL:
	  case OBJTYPE_RUNE:
	  case OBJTYPE_RUNEPOUCH:
	  case OBJTYPE_MATCH:
	  case OBJTYPE_MAP:
	  case OBJTYPE_PORTAL:
	  case OBJTYPE_TINDER:
	  case OBJTYPE_LOCKPICK:
	  case OBJTYPE_SPIKE:
	  case OBJTYPE_DISEASE:
	  case OBJTYPE_OIL:
	  case OBJTYPE_FUEL:
	  case OBJTYPE_EMPTY1:
	  case OBJTYPE_EMPTY2:
	  case OBJTYPE_SHOVEL:
	  case OBJTYPE_ODOR:
	  case OBJTYPE_CHANCE:
	  case OBJTYPE_STATUE:
	  case OBJTYPE_SHEATH:
#ifdef T2_CHESS
	  case OBJTYPE_CHESS:
#endif
		return;

	  case OBJTYPE_LIGHT:
		if ( !obj->light )
			init_values( obj, OBJTYPE_LIGHT );

		obj->light->hours			= x[2];
		return;

	  case OBJTYPE_SCROLL:
		if ( !obj->scroll )
			init_values( obj, OBJTYPE_SCROLL );

		obj->scroll->level			= x[0];
		obj->scroll->sn1			= x[1];
		obj->scroll->sn2			= x[2];
		obj->scroll->sn3			= x[3];
		return;

	  case OBJTYPE_POTION:
		if ( !obj->potion )
			init_values( obj, OBJTYPE_POTION );

		obj->potion->level			= x[0];
		obj->potion->sn1			= x[1];
		obj->potion->sn2			= x[2];
		obj->potion->sn3			= x[3];
		return;

	  case OBJTYPE_PILL:
		if ( !obj->pill )
			init_values( obj, OBJTYPE_PILL );

		obj->pill->level			= x[0];
		obj->pill->sn1				= x[1];
		obj->pill->sn2				= x[2];
		obj->pill->sn3				= x[3];
		return;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
		if ( !obj->wand )
			init_values( obj, OBJTYPE_WAND );

		obj->wand->level			= x[0];
		obj->wand->max_charges		= x[1];
		obj->wand->curr_charges		= x[2];
		obj->wand->sn				= x[3];
		return;

	  case OBJTYPE_WEAPON:
	  case OBJTYPE_FIREWEAPON:
	  case OBJTYPE_MISSILE:
	  case OBJTYPE_MISSILE_WEAPON:
	  case OBJTYPE_PROJECTILE:
		if ( !obj->weapon )
			init_values( obj, OBJTYPE_WEAPON );

		obj->weapon->condition		= x[0];
		obj->weapon->dice_number	= x[1];
		obj->weapon->dice_size		= x[2];
		obj->weapon->damage1		= x[3];
		return;

	  case OBJTYPE_TREASURE:
		if ( !obj->treasure )
			init_values( obj, OBJTYPE_TREASURE );

		obj->treasure->type			= x[0];
		obj->treasure->condition	= x[1];
		return;

	  case OBJTYPE_DRESS:
	  case OBJTYPE_ARMOR:
		if ( !obj->armor )
			init_values( obj, OBJTYPE_ARMOR );

		obj->armor->ac				= x[0];
		obj->armor->ac_orig			= x[1];
		return;

	  case OBJTYPE_FURNITURE:
		if ( !obj->furniture )
			init_values( obj, OBJTYPE_FURNITURE );

		obj->furniture->regain[POINTS_LIFE]= x[0];
		obj->furniture->regain[POINTS_MOVE]= x[1];
		obj->furniture->regain[POINTS_MANA]= x[2];
		obj->furniture->regain[POINTS_SOUL]= x[3];
		return;

	  case OBJTYPE_CONTAINER:
	  case OBJTYPE_QUIVER:
	  case OBJTYPE_KEYRING:
		if ( !obj->container )
			init_values( obj, OBJTYPE_CONTAINER );

		obj->container->capacity	= x[0];
		obj->container->flags		= x[1];
		obj->container->key			= x[2];
		obj->container->durability	= x[3];

		/* Converte da once a grammi */
		if ( tarea && !HAS_BIT(tarea->flags, AREAFLAG_WEIGHTHEIGHT) )
			obj->container->capacity = once_to_gram( obj->container->capacity );
		return;

	  case OBJTYPE_DRINK_CON:
  	  case OBJTYPE_FOUNTAIN:
	  case OBJTYPE_BLOOD:
	  case OBJTYPE_BLOODSTAIN:
	  case OBJTYPE_HERB_CON:
		if ( !obj->drinkcon )
				init_values( obj, OBJTYPE_DRINK_CON );

		obj->drinkcon->capacity		= x[0];
		obj->drinkcon->curr_quant	= x[1];
		obj->drinkcon->liquid		= x[2];
		obj->drinkcon->poison		= x[3];

		/* Converte da once a grammi */
		if ( tarea && !HAS_BIT(tarea->flags, AREAFLAG_WEIGHTHEIGHT) )
		{
  			obj->drinkcon->capacity		= once_to_gram( obj->drinkcon->capacity );
			obj->drinkcon->curr_quant	= once_to_gram( obj->drinkcon->curr_quant );
		}
		return;

	  case OBJTYPE_COOK:
	  case OBJTYPE_FOOD:
		if ( !obj->food )
			init_values( obj, OBJTYPE_FOOD );

		obj->food->hours			= x[0];
		obj->food->condition		= x[1];
		obj->food->poison			= x[3];
		obj->food->init_cond		= x[4];
		return;

	  case OBJTYPE_MONEY:
		if ( !obj->money )
			init_values( obj, OBJTYPE_MONEY );

		obj->money->qnt				= x[0];
		return;

	  /* Basta inizializzarla per far andare le mail */
	  case OBJTYPE_PEN:
		if ( !obj->pen )
			init_values( obj, OBJTYPE_PEN );
		return;

	  case OBJTYPE_CORPSE_MOB:
	  case OBJTYPE_CORPSE_PG:
		if ( !obj->corpse )
			init_values( obj, OBJTYPE_CORPSE_MOB );

		obj->corpse->time			= x[2];
		obj->corpse->tick			= x[3];
		obj->corpse->level			= x[4];
		obj->corpse->looted			= x[5];
		return;

	  case OBJTYPE_PIPE:
		if ( !obj->pipe )
			init_values( obj, OBJTYPE_PIPE );

		obj->pipe->capacity			= x[0];
		obj->pipe->draws			= x[1];
		obj->pipe->herb				= x[2];
		obj->pipe->flags			= x[3];
		return;

	  case OBJTYPE_HERB:
		if ( !obj->herb )
			init_values( obj, OBJTYPE_HERB );

		obj->herb->charges			= x[1];
		obj->herb->hnum				= x[2];
		return;

	  case OBJTYPE_SWITCH:
	  case OBJTYPE_LEVER:
	  case OBJTYPE_PULLCHAIN:
	  case OBJTYPE_BUTTON:
		if ( !obj->lever )
			init_values( obj, OBJTYPE_LEVER );

		obj->lever->flags			= x[0];
		obj->lever->vnum_sn			= x[1];
		obj->lever->vnum			= x[2];
		obj->lever->vnum_val		= x[3];
		return;

	  case OBJTYPE_TRAP:
		if ( !obj->trap )
			init_values( obj, OBJTYPE_TRAP );

		obj->trap->charges			= x[0];
		obj->trap->type				= x[1];
		obj->trap->level			= x[2];
		obj->trap->flags			= x[3];
		return;

	  /* Basta inizializzarla per fare funzionare il sistema delle mail */
	  case OBJTYPE_PAPER:
		if ( !obj->paper )
			init_values( obj, OBJTYPE_PAPER );
		return;

	  case OBJTYPE_SALVE:
		if ( !obj->salve )
			init_values( obj, OBJTYPE_SALVE );

		obj->salve->level			= x[0];
		obj->salve->charges			= x[1];
		obj->salve->max_charges		= x[2];
		obj->salve->delay			= x[3];
		obj->salve->sn1				= x[4];
		obj->salve->sn2				= x[5];
		return;
	}
}


/*
 * Legge i values di un oggetto da un file di save
 * Se type è -1 allora legge tutti i
 */
void fread_values( MUD_FILE *fp, OBJ_DATA *obj )
{
	int		type;

	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fread_values: fp è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "fread_values: obj è NULL" );
		return;
	}

	type = code_code( fp, fread_word(fp), CODE_OBJTYPE );
	switch ( type )
	{
	  default:
		send_log( fp, LOG_FREAD, "fread_values: oggetto (#%d) di tipo: %s", obj->vnum, code_str(fp, type, CODE_OBJTYPE) );
		break;

	  case OBJTYPE_LIGHT:
		if ( obj->light )
			free_values( obj, OBJTYPE_LIGHT );

		init_values( obj, OBJTYPE_LIGHT );

		obj->light->intensity			= fread_number( fp );
		obj->light->type				= code_code( fp, fread_word(fp), CODE_LIGHT );
		obj->light->hours				= fread_number( fp );
		obj->light->act_light			= fread_string( fp );
		obj->light->act_light_room		= fread_string( fp );
		obj->light->act_dark			= fread_string( fp );
		obj->light->act_dark_room		= fread_string( fp );
		obj->light->act_consume			= fread_string( fp );
		obj->light->act_consume_room	= fread_string( fp );
		break;

	  case OBJTYPE_SCROLL:
		if ( obj->scroll )
			free_values( obj, OBJTYPE_SCROLL );

		init_values( obj, OBJTYPE_SCROLL );

		obj->scroll->level				= fread_number( fp );

		obj->scroll->sn1				= skill_lookup( fread_word(fp) );
		obj->scroll->sn2				= skill_lookup( fread_word(fp) );
		obj->scroll->sn3				= skill_lookup( fread_word(fp) );
		obj->scroll->act_use			= fread_string( fp );
		obj->scroll->act_use_room		= fread_string( fp );
		break;

	  case OBJTYPE_POTION:
		if ( obj->potion )
			free_values( obj, OBJTYPE_POTION );

		init_values( obj, OBJTYPE_POTION );

		obj->potion->level				= fread_number( fp );
		obj->potion->sn1				= skill_lookup( fread_word(fp) );
		obj->potion->sn2				= skill_lookup( fread_word(fp) );
		obj->potion->sn3				= skill_lookup( fread_word(fp) );
		obj->potion->act_use			= fread_string( fp );
		obj->potion->act_use_room		= fread_string( fp );
		break;

	  case OBJTYPE_PILL:
		if ( obj->pill )
			free_values( obj, OBJTYPE_PILL );

		init_values( obj, OBJTYPE_PILL );

		obj->pill->level				= fread_number( fp );
		obj->pill->sn1					= skill_lookup( fread_word(fp) );
		obj->pill->sn2					= skill_lookup( fread_word(fp) );
		obj->pill->sn3					= skill_lookup( fread_word(fp) );
		obj->pill->act_use				= fread_string( fp );
		obj->pill->act_use_room			= fread_string( fp );
		break;

	  case OBJTYPE_WAND:
		if ( obj->wand )
			free_values( obj, OBJTYPE_WAND );

		init_values( obj, OBJTYPE_WAND );

		obj->wand->level				= fread_number( fp );
		obj->wand->max_charges			= fread_number( fp );
		obj->wand->curr_charges			= fread_number( fp );
		obj->wand->sn					= skill_lookup( fread_word(fp) );
		obj->wand->act_zap				= fread_string( fp );
		obj->wand->act_zap_room			= fread_string( fp );
		obj->wand->act_consume			= fread_string( fp );
		obj->wand->act_consume_room		= fread_string( fp );
		break;

	  case OBJTYPE_WEAPON:
		if ( obj->weapon )
			free_values( obj, OBJTYPE_WEAPON );

		init_values( obj, OBJTYPE_WEAPON );

		obj->weapon->condition			= fread_number( fp );
		obj->weapon->dice_number		= fread_number( fp );
		obj->weapon->dice_size			= fread_number( fp );
		obj->weapon->type				= code_code( fp, fread_word(fp), CODE_WEAPON );
		obj->weapon->damage1			= code_code( fp, fread_word(fp), CODE_DAMAGE );
		obj->weapon->damage2			= code_code( fp, fread_word(fp), CODE_DAMAGE );
		obj->weapon->lvl_disarm			= fread_number( fp );
		obj->weapon->act_wield			= fread_string( fp );
		obj->weapon->act_wield_room		= fread_string( fp );
		break;

	  case OBJTYPE_TREASURE:
		if ( obj->treasure )
			free_values( obj, OBJTYPE_TREASURE );

		init_values( obj, OBJTYPE_TREASURE );

		obj->treasure->type				= fread_number( fp );
		obj->treasure->condition		= fread_number( fp );
		break;

	  case OBJTYPE_ARMOR:
		if ( obj->armor )
			free_values( obj, OBJTYPE_ARMOR );

		init_values( obj, OBJTYPE_ARMOR );

		obj->armor->ac					= fread_number( fp );
		obj->armor->ac_orig				= fread_number( fp );
		obj->armor->type				= fread_number( fp );
		obj->armor->act_wear			= fread_string( fp );
		obj->armor->act_wear_room		= fread_string( fp );
		break;

	  case OBJTYPE_FURNITURE:
	  {
		int x;

		if ( obj->furniture )
			free_values( obj, OBJTYPE_FURNITURE );

		init_values( obj, OBJTYPE_FURNITURE );

		for ( x = 0;  x < MAX_POINTS;  x++ )
			obj->furniture->regain[x]	= fread_number( fp );
	  }
	  break;

	  case OBJTYPE_CONTAINER:
		if ( obj->container )
			free_values( obj, OBJTYPE_CONTAINER );

		init_values( obj, OBJTYPE_CONTAINER );

		obj->container->capacity		= fread_number( fp );
		obj->container->key				= fread_number( fp );
		obj->container->durability		= fread_number( fp );
		obj->container->type			= code_code( fp, fread_word(fp), CODE_OBJTYPE );
		obj->container->flags			= fread_code_bitint( fp, CODE_CONTAINER );
		obj->container->act_open		= fread_string( fp );
		obj->container->act_open_room	= fread_string( fp );
		obj->container->act_close		= fread_string( fp );
		obj->container->act_close_room	= fread_string( fp );
		break;

	  case OBJTYPE_DRINK_CON:
		if ( obj->drinkcon )
			free_values( obj, OBJTYPE_DRINK_CON );

		init_values( obj, OBJTYPE_DRINK_CON );

		obj->drinkcon->capacity			= fread_number( fp );
		obj->drinkcon->curr_quant		= fread_number( fp );
		obj->drinkcon->liquid			= fread_number( fp );
		obj->drinkcon->poison			= fread_number( fp );
		obj->drinkcon->fountain			= fread_bool( fp );
		obj->drinkcon->act_drink		= fread_string( fp );
		obj->drinkcon->act_drink_room	= fread_string( fp );
		break;

	  case OBJTYPE_KEY:
		if ( obj->key )
			free_values( obj, OBJTYPE_KEY );

		init_values( obj, OBJTYPE_KEY );

		obj->key->save					= fread_bool( fp );
		obj->key->act_unlock			= fread_string( fp );
		obj->key->act_unlock_room		= fread_string( fp );
		obj->key->act_lock				= fread_string( fp );
		obj->key->act_lock_room			= fread_string( fp );
		break;

	  case OBJTYPE_FOOD:
		if ( obj->food )
			free_values( obj, OBJTYPE_FOOD );

		init_values( obj, OBJTYPE_FOOD );

		obj->food->hours				= fread_number( fp );
		obj->food->condition			= fread_number( fp );
		obj->food->dam_cook				= fread_number( fp );
		obj->food->poison				= fread_number( fp );
		obj->food->init_cond			= fread_number( fp );
		obj->food->flags				= fread_number( fp );	/* (GG) */
		obj->food->act_eat				= fread_string( fp );
		obj->food->act_eat_room			= fread_string( fp );
		break;

	  case OBJTYPE_MONEY:
		if ( obj->money )
			free_values( obj, OBJTYPE_MONEY );

		init_values( obj, OBJTYPE_MONEY );

		obj->money->qnt					= fread_number( fp );
		obj->money->qnt_recycle			= fread_number( fp );
		obj->money->type				= fread_number( fp );
		break;

	  case OBJTYPE_PEN:
		if ( obj->pen )
			free_values( obj, OBJTYPE_PEN );

		init_values( obj, OBJTYPE_PEN );

		obj->pen->ink					= fread_number( fp );
		obj->pen->act_write				= fread_string( fp );
		obj->pen->act_write_room		= fread_string( fp );
		break;

	  case OBJTYPE_BOAT:
		if ( obj->boat )
			free_values( obj, OBJTYPE_BOAT );

		init_values( obj, OBJTYPE_BOAT );

		obj->boat->rooms				= fread_number( fp );
		obj->boat->type					= fread_number( fp );
		obj->boat->act_travel			= fread_string( fp );
		obj->boat->act_travel_room		= fread_string( fp );
		break;

	  case OBJTYPE_CORPSE_MOB:
		if ( obj->corpse )
			free_values( obj, OBJTYPE_CORPSE_MOB );

		init_values( obj, OBJTYPE_CORPSE_MOB );

		obj->corpse->time				= fread_number( fp );
		obj->corpse->tick				= fread_number( fp );
		obj->corpse->level				= fread_number( fp );
		obj->corpse->looted				= fread_number( fp );
		obj->corpse->pg					= fread_bool( fp );
		break;

	  case OBJTYPE_PIPE:
		if ( obj->pipe )
			free_values( obj, OBJTYPE_PIPE );

		init_values( obj, OBJTYPE_PIPE );

		obj->pipe->capacity				= fread_number( fp );
		obj->pipe->draws				= fread_number( fp );
		obj->pipe->herb					= fread_number( fp );
		obj->pipe->flags				= fread_code_bitint( fp, CODE_PIPE );
		obj->pipe->act_smoke			= fread_string( fp );
		obj->pipe->act_smoke_room		= fread_string( fp );
		break;

	  case OBJTYPE_HERB:
		if ( obj->herb )
			free_values( obj, OBJTYPE_HERB );

		init_values( obj, OBJTYPE_HERB );

		obj->herb->charges				= fread_number( fp );
		obj->herb->hnum					= fread_number( fp );
		break;

	  case OBJTYPE_LEVER:
		if ( obj->lever )
			free_values( obj, OBJTYPE_LEVER );

		init_values( obj, OBJTYPE_LEVER );

		obj->lever->flags				= fread_code_bitint( fp, CODE_TRIG );
		obj->lever->vnum_sn				= fread_number( fp );
		obj->lever->vnum				= fread_number( fp );
		obj->lever->vnum_val			= fread_number( fp );
		obj->lever->act_use				= fread_string( fp );
		obj->lever->act_use_room		= fread_string( fp );
		break;

	  case OBJTYPE_TRAP:
		if ( obj->trap )
			free_values( obj, OBJTYPE_TRAP );

		init_values( obj, OBJTYPE_TRAP );

		obj->trap->charges				= fread_number( fp );
		obj->trap->type					= fread_number( fp );
		obj->trap->level				= fread_number( fp );
		obj->trap->flags				= fread_code_bitint( fp, CODE_TRAPFLAG );
		obj->trap->act_activate			= fread_string( fp );
		obj->trap->act_activate_room	= fread_string( fp );
		break;

	  case OBJTYPE_PAPER:
		if ( obj->paper )
			free_values( obj, OBJTYPE_PAPER );

		init_values( obj, OBJTYPE_PAPER );

		obj->paper->text				= fread_number( fp );
		obj->paper->subject				= fread_number( fp );
		obj->paper->to					= fread_number( fp );
		obj->paper->type				= fread_number( fp );
		break;

	  case OBJTYPE_SALVE:
		if ( obj->salve )
			free_values( obj, OBJTYPE_SALVE );

		init_values( obj, OBJTYPE_SALVE );

		obj->salve->level				= fread_number( fp );
		obj->salve->charges				= fread_number( fp );
		obj->salve->max_charges			= fread_number( fp );
		obj->salve->delay				= fread_number( fp );
		obj->salve->sn1					= skill_lookup( fread_word(fp) );
		obj->salve->sn2					= skill_lookup( fread_word(fp) );
		obj->salve->act_apply			= fread_string( fp );
		obj->salve->act_apply_room		= fread_string( fp );
		break;

	  case OBJTYPE_SHEATH:
		if ( obj->sheath )
			free_values( obj, OBJTYPE_SHEATH );

		init_values( obj, OBJTYPE_SHEATH );

		obj->sheath->weapon_type		= fread_number( fp );
		obj->sheath->vnum				= fread_number( fp );
		obj->sheath->flags				= fread_number( fp );	/* (GG) */
		obj->sheath->act_open			= fread_string( fp );
		obj->sheath->act_open_room		= fread_string( fp );
		obj->sheath->act_close			= fread_string( fp );
		obj->sheath->act_close_room		= fread_string( fp );
		break;
	}
}


/*
 * Scrive su file i values di una tipologia di oggetto
 */
void fwrite_values( MUD_FILE *fp, OBJ_DATA *obj )
{
	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fwrite_values: fp è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "fwrite_values: obj è NULL" );
		return;
	}

	if ( obj->light )
	{
		fprintf( fp->file, "Values %s %d %s %d\n",
			code_str(fp, OBJTYPE_LIGHT, CODE_OBJTYPE),
			obj->light->intensity,
			code_str(fp, obj->light->type, CODE_LIGHT),
			obj->light->hours );
		fprintf( fp->file, "    %s~\n", obj->light->act_light );
		fprintf( fp->file, "    %s~\n", obj->light->act_light_room );
		fprintf( fp->file, "    %s~\n", obj->light->act_dark );
		fprintf( fp->file, "    %s~\n", obj->light->act_dark_room );
		fprintf( fp->file, "    %s~\n", obj->light->act_consume );
		fprintf( fp->file, "    %s~\n", obj->light->act_consume_room );
	}

	if ( obj->scroll )
	{
		fprintf( fp->file, "Values %s %d '%s' '%s' '%s'\n",
			code_str(fp, OBJTYPE_SCROLL, CODE_OBJTYPE),
			obj->scroll->level,
			is_valid_sn(obj->scroll->sn1) ? table_skill[obj->scroll->sn1]->name : "",
			is_valid_sn(obj->scroll->sn2) ? table_skill[obj->scroll->sn2]->name : "",
			is_valid_sn(obj->scroll->sn3) ? table_skill[obj->scroll->sn3]->name : "" );
		fprintf( fp->file, "    %s~\n", obj->scroll->act_use );
		fprintf( fp->file, "    %s~\n", obj->scroll->act_use_room );
	}

	if ( obj->potion )
	{
		fprintf( fp->file, "Values %s %d '%s' '%s' '%s'\n",
			code_str(fp, OBJTYPE_POTION, CODE_OBJTYPE),
			obj->potion->level,
			is_valid_sn(obj->potion->sn1) ? table_skill[obj->potion->sn1]->name : "",
			is_valid_sn(obj->potion->sn2) ? table_skill[obj->potion->sn2]->name : "",
			is_valid_sn(obj->potion->sn3) ? table_skill[obj->potion->sn3]->name : "" );
		fprintf( fp->file, "    %s~\n", obj->potion->act_use );
		fprintf( fp->file, "    %s~\n", obj->potion->act_use_room );
	}

	if ( obj->pill )
	{
		fprintf( fp->file, "Values %s %d '%s' '%s' '%s'\n",
			code_str(fp, OBJTYPE_PILL, CODE_OBJTYPE),
			obj->pill->level,
			is_valid_sn(obj->pill->sn1) ? table_skill[obj->pill->sn1]->name : "",
			is_valid_sn(obj->pill->sn2) ? table_skill[obj->pill->sn2]->name : "",
			is_valid_sn(obj->pill->sn3) ? table_skill[obj->pill->sn3]->name : "" );
		fprintf( fp->file, "    %s~\n", obj->pill->act_use );
		fprintf( fp->file, "    %s~\n", obj->pill->act_use_room );
	}

	if ( obj->wand )
	{
		fprintf( fp->file, "Values %s %d %d %d '%s'\n",
			code_str(fp, OBJTYPE_WAND, CODE_OBJTYPE),
			obj->wand->level,
			obj->wand->max_charges,
			obj->wand->curr_charges,
			is_valid_sn(obj->wand->sn) ? table_skill[obj->wand->sn]->name : "" );
		fprintf( fp->file, "    %s~\n", obj->wand->act_zap );
		fprintf( fp->file, "    %s~\n", obj->wand->act_zap_room );
		fprintf( fp->file, "    %s~\n", obj->wand->act_consume );
		fprintf( fp->file, "    %s~\n", obj->wand->act_consume_room );
	}

	if ( obj->weapon )
	{
		fprintf( fp->file, "Values %s %d %d %d %s %s %s %d\n",
			code_str(fp, OBJTYPE_WEAPON, CODE_OBJTYPE),
			obj->weapon->condition,
			obj->weapon->dice_number,
			obj->weapon->dice_size,
			code_str(fp, obj->weapon->type, CODE_WEAPON),
			code_str(fp, obj->weapon->damage1, CODE_DAMAGE),
			code_str(fp, obj->weapon->damage2, CODE_DAMAGE),
			obj->weapon->lvl_disarm );
		fprintf( fp->file, "    %s~\n", obj->weapon->act_wield );
		fprintf( fp->file, "    %s~\n", obj->weapon->act_wield_room );
	}

	/* Struttura da togliere una volta convertita la condition */
	if ( obj->treasure )
	{
		fprintf( fp->file, "Values %s %d %d\n",
			code_str(fp, OBJTYPE_TREASURE, CODE_OBJTYPE),
			obj->treasure->type,
			obj->treasure->condition );
	}

	/* (CC) %s tipo di armatura, cambiarlo a wear-flags */
	if ( obj->armor )
	{
		fprintf( fp->file, "Values %s %d %d %d\n",
			code_str(fp, OBJTYPE_ARMOR, CODE_OBJTYPE),
			obj->armor->ac,
			obj->armor->ac_orig,
			obj->armor->type );
		fprintf( fp->file, "    %s~\n", obj->armor->act_wear );
		fprintf( fp->file, "    %s~\n", obj->armor->act_wear_room );
	}

	if ( obj->furniture )
	{
		fprintf( fp->file, "Values %s %d %d %d %d\n",
			code_str(fp, OBJTYPE_FURNITURE, CODE_OBJTYPE),
			obj->furniture->regain[POINTS_LIFE],
			obj->furniture->regain[POINTS_MOVE],
			obj->furniture->regain[POINTS_MANA],
			obj->furniture->regain[POINTS_SOUL] );
	}

	if ( obj->container )
	{
		fprintf( fp->file, "Values %s %d %d %d %s\n",
			code_str(fp, OBJTYPE_CONTAINER, CODE_OBJTYPE),
			obj->container->capacity,
			obj->container->key,
			obj->container->durability,
			code_str(fp, obj->container->type, CODE_OBJTYPE) );
		fprintf( fp->file, "    %s~\n", code_bitint(fp, obj->container->flags, CODE_CONTAINER) );
		fprintf( fp->file, "    %s~\n", obj->container->act_open );
		fprintf( fp->file, "    %s~\n", obj->container->act_open_room );
		fprintf( fp->file, "    %s~\n", obj->container->act_close );
		fprintf( fp->file, "    %s~\n", obj->container->act_close_room );
	}

	if ( obj->drinkcon )
	{
		fprintf( fp->file, "Values %s %d %d %d %d %s\n",
			code_str(fp, OBJTYPE_DRINK_CON, CODE_OBJTYPE),
			obj->drinkcon->capacity,
			obj->drinkcon->curr_quant,
			obj->drinkcon->liquid,
			obj->drinkcon->poison,
			fwrite_bool(obj->drinkcon->fountain) );
		fprintf( fp->file, "    %s~\n", obj->drinkcon->act_drink );
		fprintf( fp->file, "    %s~\n", obj->drinkcon->act_drink_room );
	}

	if ( obj->key )
	{
		fprintf( fp->file, "Values %s %s\n",
			code_str(fp, OBJTYPE_KEY, CODE_OBJTYPE),
			fwrite_bool(obj->key->save) );
		fprintf( fp->file, "    %s~\n", obj->key->act_unlock );
		fprintf( fp->file, "    %s~\n", obj->key->act_unlock_room );
		fprintf( fp->file, "    %s~\n", obj->key->act_lock );
		fprintf( fp->file, "    %s~\n", obj->key->act_lock_room );
	}

	if ( obj->food )
	{
		fprintf( fp->file, "Values %s %d %d %d %d %d\n",
			code_str(fp, OBJTYPE_FOOD, CODE_OBJTYPE),
			obj->food->hours,
			obj->food->condition,
			obj->food->dam_cook,
			obj->food->poison,
			obj->food->init_cond );
		fprintf( fp->file, "    %d\n", obj->food->flags );	/* (FF) futura stringa di flag */
		fprintf( fp->file, "    %s~\n", obj->food->act_eat );
		fprintf( fp->file, "    %s~\n", obj->food->act_eat_room );
	}

	if ( obj->money )
	{
		fprintf( fp->file, "Values %s %d %d %d\n",
			code_str(fp, OBJTYPE_MONEY, CODE_OBJTYPE),
			obj->money->qnt,
			obj->money->qnt_recycle,
			obj->money->type );
	}

	if ( obj->pen )
	{
		fprintf( fp->file, "Values %s %d\n",
			code_str(fp, OBJTYPE_PEN, CODE_OBJTYPE),
			obj->pen->ink );
		fprintf( fp->file, "    %s~\n", obj->pen->act_write );
		fprintf( fp->file, "    %s~\n", obj->pen->act_write_room );
	}

	if ( obj->boat )
	{
		fprintf( fp->file, "Values %s %d %d\n",
			code_str(fp, OBJTYPE_BOAT, CODE_OBJTYPE),
			obj->boat->rooms,
			obj->boat->type );
		fprintf( fp->file, "    %s~\n", obj->boat->act_travel );
		fprintf( fp->file, "    %s~\n", obj->boat->act_travel_room );
	}

	if ( obj->corpse )
	{
		fprintf( fp->file, "Values %s %d %d %d %d %s\n",
			code_str(fp, OBJTYPE_CORPSE_MOB, CODE_OBJTYPE),
			obj->corpse->time,
			obj->corpse->tick,
			obj->corpse->level,
			obj->corpse->looted,
			fwrite_bool(obj->corpse->pg) );
	}

	if ( obj->pipe )
	{
		fprintf( fp->file, "Values %s %d %d %d\n",
			code_str(fp, OBJTYPE_PIPE, CODE_OBJTYPE),
			obj->pipe->capacity,
			obj->pipe->draws,
			obj->pipe->herb );
		fprintf( fp->file, "    %s~\n", code_bitint(fp, obj->pipe->flags, CODE_PIPE) );
		fprintf( fp->file, "    %s~\n", obj->pipe->act_smoke );
		fprintf( fp->file, "    %s~\n", obj->pipe->act_smoke_room );
	}

	if ( obj->herb )
	{
		fprintf( fp->file, "Values %s %d %d\n",
			code_str(fp, OBJTYPE_HERB, CODE_OBJTYPE),
			obj->herb->charges,
			obj->herb->hnum );
	}

	if ( obj->lever )
	{
		fprintf( fp->file, "Values %s %s~\n",
			code_str(fp, OBJTYPE_LEVER, CODE_OBJTYPE),
			code_bitint(fp, obj->lever->flags, CODE_TRIG) );
		fprintf( fp->file, "    %d %d %d\n",
			obj->lever->vnum_sn,
			obj->lever->vnum,
			obj->lever->vnum_val );
		fprintf( fp->file, "    %s~\n", obj->lever->act_use );
		fprintf( fp->file, "    %s~\n", obj->lever->act_use_room );
	}

	if ( obj->trap )
	{
		fprintf( fp->file, "Values %s %d %d %d\n",
			code_str(fp, OBJTYPE_TRAP, CODE_OBJTYPE),
			obj->trap->charges,
			obj->trap->type,
			obj->trap->level );
		fprintf( fp->file, "    %s~\n", code_bitint(fp, obj->trap->flags, CODE_TRAPFLAG) );
		fprintf( fp->file, "    %s~\n", obj->trap->act_activate );
		fprintf( fp->file, "    %s~\n", obj->trap->act_activate_room );
	}

	if ( obj->paper )
	{
		fprintf( fp->file, "Values %s %d %d %d %d\n",
			code_str(fp, OBJTYPE_PAPER, CODE_OBJTYPE),
			obj->paper->text,
			obj->paper->subject,
			obj->paper->to,
			obj->paper->type );
	}

	if ( obj->salve )
	{
		fprintf( fp->file, "Values %s %d %d %d %d '%s' '%s'\n",
			code_str(fp, OBJTYPE_SALVE, CODE_OBJTYPE),
			obj->salve->level,
			obj->salve->charges,
			obj->salve->max_charges,
			obj->salve->delay,
			is_valid_sn(obj->salve->sn1) ? table_skill[obj->salve->sn1]->name : "",
			is_valid_sn(obj->salve->sn2) ? table_skill[obj->salve->sn2]->name : "" );
		fprintf( fp->file, "    %s~\n", obj->salve->act_apply );
		fprintf( fp->file, "    %s~\n", obj->salve->act_apply_room );
	}

	if ( obj->sheath )
	{
		fprintf( fp->file, "Values %s %s %d\n",
			code_str(fp, OBJTYPE_SHEATH, CODE_OBJTYPE),
			code_str(fp, obj->sheath->weapon_type, CODE_WEAPON),
			obj->sheath->vnum );
		fprintf( fp->file, "    %d\n", obj->sheath->flags );	/* (FF) flags in stringa con code_bitint */
		fprintf( fp->file, "    %s~\n", obj->sheath->act_open );
		fprintf( fp->file, "    %s~\n", obj->sheath->act_open_room );
		fprintf( fp->file, "    %s~\n", obj->sheath->act_close );
		fprintf( fp->file, "    %s~\n", obj->sheath->act_close_room );
	}
}


/*
 * Copia i value dell'oggetto source in quello di destinazione
 */
void copy_values( OBJ_DATA *source, OBJ_DATA *dest )
{
	if ( !source )
	{
		send_log( NULL, LOG_BUG, "copy_values: l'oggetto source è NULL" );
		return;
	}

	if ( !dest )
	{
		send_log( NULL, LOG_BUG, "copy_values: l'oggetto dest è NULL" );
		return;
	}

	if ( source->light )
	{
		init_values( dest, OBJTYPE_LIGHT );

		dest->light->intensity			= source->light->intensity;
		dest->light->type				= source->light->type;
		dest->light->hours				= source->light->hours;
		dest->light->act_light			= str_dup( source->light->act_light );
		dest->light->act_light_room		= str_dup( source->light->act_light_room );
		dest->light->act_dark			= str_dup( source->light->act_dark );
		dest->light->act_dark_room		= str_dup( source->light->act_dark_room );
		dest->light->act_consume		= str_dup( source->light->act_consume );
		dest->light->act_consume_room	= str_dup( source->light->act_consume_room );
	}

	if ( source->scroll )
	{
		init_values( dest, OBJTYPE_SCROLL );

		dest->scroll->level				= source->scroll->level;
		dest->scroll->sn1				= source->scroll->sn1;
		dest->scroll->sn2				= source->scroll->sn2;
		dest->scroll->sn3				= source->scroll->sn3;
		dest->scroll->act_use			= str_dup( source->scroll->act_use );
		dest->scroll->act_use_room		= str_dup( source->scroll->act_use_room );
	}

	if ( source->potion )
	{
		init_values( dest, OBJTYPE_POTION );

		dest->potion->level				= source->potion->level;
		dest->potion->sn1				= source->potion->sn1;
		dest->potion->sn2				= source->potion->sn2;
		dest->potion->sn3				= source->potion->sn3;
		dest->potion->act_use			= str_dup( source->potion->act_use );
		dest->potion->act_use_room		= str_dup( source->potion->act_use_room );
	}

	if ( source->pill )
	{
		init_values( dest, OBJTYPE_PILL );

		dest->pill->level				= source->pill->level;
		dest->pill->sn1					= source->pill->sn1;
		dest->pill->sn2					= source->pill->sn2;
		dest->pill->sn3					= source->pill->sn3;
		dest->pill->act_use				= str_dup( source->pill->act_use );
		dest->pill->act_use_room		= str_dup( source->pill->act_use_room );
	}

	if ( source->wand )
	{
		init_values( dest, OBJTYPE_WAND );

		dest->wand->level				= source->wand->level;
		dest->wand->max_charges			= source->wand->max_charges;
		dest->wand->curr_charges		= source->wand->curr_charges;
		dest->wand->sn					= source->wand->sn;
		dest->wand->act_zap				= str_dup( source->wand->act_zap );
		dest->wand->act_zap_room		= str_dup( source->wand->act_zap_room );
		dest->wand->act_consume			= str_dup( source->wand->act_consume );
		dest->wand->act_consume_room	= str_dup( source->wand->act_consume_room );
	}

	if ( source->weapon )
	{
		init_values( dest, OBJTYPE_WEAPON );

		dest->weapon->condition			= source->weapon->condition;
		dest->weapon->dice_number		= source->weapon->dice_number;
		dest->weapon->dice_size			= source->weapon->dice_size;
		dest->weapon->type				= source->weapon->type;
		dest->weapon->damage1			= source->weapon->damage1;
		dest->weapon->damage2			= source->weapon->damage2;
		dest->weapon->range				= source->weapon->range;
		dest->weapon->lvl_disarm		= source->weapon->lvl_disarm;
		dest->weapon->act_wield			= str_dup( source->weapon->act_wield );
		dest->weapon->act_wield_room	= str_dup( source->weapon->act_wield_room );
	}

	if ( source->treasure )
	{
		init_values( dest, OBJTYPE_TREASURE );

		dest->treasure->type			= source->treasure->type;
		dest->treasure->condition		= source->treasure->condition;
	}

	if ( source->armor )
	{
		init_values( dest, OBJTYPE_ARMOR );

		dest->armor->ac					= source->armor->ac;
		dest->armor->ac_orig			= source->armor->ac_orig;
		dest->armor->type				= source->armor->type;
		dest->armor->act_wear			= str_dup( source->armor->act_wear );
		dest->armor->act_wear_room		= str_dup( source->armor->act_wear_room );
	}

	if ( source->furniture )
	{
		int x;

		init_values( dest, OBJTYPE_FURNITURE );

		for ( x = 0;  x < MAX_POINTS;  x++ )
			dest->furniture->regain[x]		= source->furniture->regain[x];
	}

	if ( source->container )
	{
		init_values( dest, OBJTYPE_CONTAINER );

		dest->container->capacity			= source->container->capacity;
		dest->container->key				= source->container->key;
		dest->container->durability			= source->container->durability;
		dest->container->type				= source->container->type;
		dest->container->flags				= source->container->flags;
		dest->container->act_open			= str_dup( source->container->act_open );
		dest->container->act_open_room		= str_dup( source->container->act_open_room );
		dest->container->act_close			= str_dup( source->container->act_close );
		dest->container->act_close_room		= str_dup( source->container->act_close_room );
	}

	if ( source->drinkcon )
	{
		init_values( dest, OBJTYPE_DRINK_CON );

		dest->drinkcon->capacity			= source->drinkcon->capacity;
		dest->drinkcon->curr_quant			= source->drinkcon->curr_quant;
		dest->drinkcon->liquid				= source->drinkcon->liquid;
		dest->drinkcon->poison				= source->drinkcon->poison;
		dest->drinkcon->fountain			= source->drinkcon->fountain;
		dest->drinkcon->act_drink			= str_dup( source->drinkcon->act_drink );
		dest->drinkcon->act_drink_room		= str_dup( source->drinkcon->act_drink_room );
	}

	if ( source->key )
	{
		init_values( dest, OBJTYPE_KEY );

		dest->key->save					= source->key->save;
		dest->key->act_unlock			= str_dup( source->key->act_unlock );
		dest->key->act_unlock_room		= str_dup( source->key->act_unlock_room );
		dest->key->act_lock				= str_dup( source->key->act_lock );
		dest->key->act_lock_room		= str_dup( source->key->act_lock_room );
	}

	if ( source->food )
	{
		init_values( dest, OBJTYPE_FOOD );

		dest->food->hours				= source->food->hours;
		dest->food->condition			= source->food->condition;
		dest->food->dam_cook			= source->food->dam_cook;
		dest->food->poison				= source->food->poison;
		dest->food->init_cond			= source->food->init_cond;
		dest->food->flags				= source->food->flags;
		dest->food->act_eat				= str_dup( source->food->act_eat );
		dest->food->act_eat_room		= str_dup( source->food->act_eat_room );
	}

	if ( source->money )
	{
		init_values( dest, OBJTYPE_MONEY );

		dest->money->qnt				= source->money->qnt;
		dest->money->qnt_recycle		= source->money->qnt_recycle;
		dest->money->type				= source->money->type;
	}

	if ( source->pen )
	{
		init_values( dest, OBJTYPE_PEN );

		dest->pen->ink					= source->pen->ink;
		dest->pen->act_write			= str_dup( source->pen->act_write );
		dest->pen->act_write_room		= str_dup( source->pen->act_write_room );
	}

	if ( source->boat )
	{
		init_values( dest, OBJTYPE_BOAT );

		dest->boat->rooms				= source->boat->rooms;
		dest->boat->type				= source->boat->type;
		dest->boat->act_travel			= str_dup( source->boat->act_travel );
		dest->boat->act_travel_room		= str_dup( source->boat->act_travel_room );
	}

	if ( source->corpse )
	{
		init_values( dest, OBJTYPE_CORPSE_MOB );

		dest->corpse->time				= source->corpse->time;
		dest->corpse->tick				= source->corpse->tick;
		dest->corpse->level				= source->corpse->level;
		dest->corpse->looted			= source->corpse->looted;
		dest->corpse->pg				= source->corpse->pg;
	}

	if ( source->pipe )
	{
		init_values( dest, OBJTYPE_PIPE );

		dest->pipe->capacity			= source->pipe->capacity;
		dest->pipe->draws				= source->pipe->draws;
		dest->pipe->herb				= source->pipe->herb;
		dest->pipe->flags				= source->pipe->flags;
		dest->pipe->act_smoke			= str_dup( source->pipe->act_smoke );
		dest->pipe->act_smoke_room		= str_dup( source->pipe->act_smoke_room );
	}

	if ( source->herb )
	{
		init_values( dest, OBJTYPE_HERB );

		dest->herb->charges				= source->herb->charges;
		dest->herb->hnum				= source->herb->hnum;
	}

	if ( source->lever )
	{
		init_values( dest, OBJTYPE_LEVER );

		dest->lever->flags				= source->lever->flags;
		dest->lever->vnum_sn			= source->lever->vnum_sn;
		dest->lever->vnum				= source->lever->vnum;
		dest->lever->vnum_val			= source->lever->vnum_val;
		dest->lever->act_use			= str_dup( source->lever->act_use );
		dest->lever->act_use_room		= str_dup( source->lever->act_use_room );
	}

	if ( source->trap )
	{
		init_values( dest, OBJTYPE_TRAP );

		dest->trap->charges				= source->trap->charges;
		dest->trap->type				= source->trap->type;
		dest->trap->level				= source->trap->level;
		dest->trap->flags				= source->trap->flags;
		dest->trap->act_activate		= str_dup( source->trap->act_activate );
		dest->trap->act_activate_room	= str_dup( source->trap->act_activate_room );
	}

	if ( source->paper )
	{
		init_values( dest, OBJTYPE_PAPER );

		dest->paper->text				= source->paper->text;
		dest->paper->subject			= source->paper->subject;
		dest->paper->to					= source->paper->to;
		dest->paper->type				= source->paper->type;
	}

	if ( source->salve )
	{
		init_values( dest, OBJTYPE_SALVE );

		dest->salve->level				= source->salve->level;
		dest->salve->charges			= source->salve->charges;
		dest->salve->max_charges		= source->salve->max_charges;
		dest->salve->delay				= source->salve->delay;
		dest->salve->sn1				= source->salve->sn1;
		dest->salve->sn2				= source->salve->sn2;
		dest->salve->act_apply			= str_dup( source->salve->act_apply );
		dest->salve->act_apply_room		= str_dup( source->salve->act_apply_room );
	}

	if ( source->sheath )
	{
		init_values( dest, OBJTYPE_SHEATH );

		dest->sheath->weapon_type		= source->sheath->weapon_type;
		dest->sheath->vnum				= source->sheath->vnum;
		dest->sheath->flags				= source->sheath->flags;
		dest->sheath->act_open			= str_dup( source->sheath->act_open );
		dest->sheath->act_open_room		= str_dup( source->sheath->act_open_room );
		dest->sheath->act_close			= str_dup( source->sheath->act_close );
		dest->sheath->act_close_room	= str_dup( source->sheath->act_close_room );
	}
}


/*
 * Confronta i value di un determinato tipo tra due oggetti e ritorna TRUE se uguali
 */
bool compare_values( OBJ_DATA *obj1, OBJ_DATA *obj2, int type )
{
	if ( !obj1 )
	{
		send_log( NULL, LOG_BUG, "compare_value: obj1 è NULL" );
		return FALSE;
	}

	if ( !obj2 )
	{
		send_log( NULL, LOG_BUG, "compare_value: obj2 è NULL" );
		return FALSE;
	}

	switch ( type )
	{
	  default:
		send_log( NULL, LOG_BUG, "compare_values: tipo passato errato: %s", code_str(NULL, type, CODE_OBJTYPE) );
		return FALSE;

	  /* Questi tipi di oggetti non hanno value quindi ritorna TRUE */
	  case OBJTYPE_NONE:	/* (bb) dovrebbe essere sotto default, ma non fino a che ci sono oggetti di tipo none */
	  case OBJTYPE_WORN:
	  case OBJTYPE_TRASH:
	  case OBJTYPE_OLDTRAP:
	  case OBJTYPE_NOTE:
	  case OBJTYPE_SCRAPS:
	  case OBJTYPE_INCENSE:
	  case OBJTYPE_FIRE:
	  case OBJTYPE_BOOK:
	  case OBJTYPE_DIAL:
	  case OBJTYPE_RUNE:
	  case OBJTYPE_RUNEPOUCH:
	  case OBJTYPE_MATCH:
	  case OBJTYPE_MAP:
	  case OBJTYPE_PORTAL:
	  case OBJTYPE_TINDER:
	  case OBJTYPE_LOCKPICK:
	  case OBJTYPE_SPIKE:
	  case OBJTYPE_DISEASE:
	  case OBJTYPE_OIL:
	  case OBJTYPE_FUEL:
	  case OBJTYPE_EMPTY1:
	  case OBJTYPE_EMPTY2:
	  case OBJTYPE_SHOVEL:
	  case OBJTYPE_ODOR:
	  case OBJTYPE_CHANCE:
	  case OBJTYPE_STATUE:
#ifdef T2_CHESS
	  case OBJTYPE_CHESS:
#endif
		return TRUE;

	  case OBJTYPE_LIGHT:
	  {
		if ( !obj1->light && !obj2->light )		return TRUE;
		if ( !obj1->light &&  obj2->light )		return FALSE;
		if (  obj1->light && !obj2->light )		return FALSE;

		if ( obj1->light->intensity				!=	obj2->light->intensity		   )	return FALSE;
		if ( obj1->light->type					!=	obj2->light->type			   )	return FALSE;
		if ( obj1->light->hours					!=	obj2->light->hours			   )	return FALSE;
		if ( str_cmp(obj1->light->act_light,		obj2->light->act_light		 ) )	return FALSE;
		if ( str_cmp(obj1->light->act_light_room,	obj2->light->act_light_room	 ) )	return FALSE;
		if ( str_cmp(obj1->light->act_dark,			obj2->light->act_dark		 ) )	return FALSE;
		if ( str_cmp(obj1->light->	act_dark_room,	obj2->light->act_dark_room	 ) )	return FALSE;
		if ( str_cmp(obj1->light->act_consume,		obj2->light->act_consume	 ) )	return FALSE;
		if ( str_cmp(obj1->light->act_consume_room,	obj2->light->act_consume_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_SCROLL:
	  {
		if ( !obj1->scroll && !obj2->scroll )		return TRUE;
		if ( !obj1->scroll &&  obj2->scroll )		return FALSE;
		if (  obj1->scroll && !obj2->scroll )		return FALSE;

		if ( obj1->scroll->level 			!=	 obj2->scroll->level		 )		return FALSE;
		if ( obj1->scroll->sn1	 			!=	 obj2->scroll->sn1			 )		return FALSE;
		if ( obj1->scroll->sn2	 			!=	 obj2->scroll->sn2			 )		return FALSE;
		if ( obj1->scroll->sn3	 			!=	 obj2->scroll->sn3			 )		return FALSE;
		if ( str_cmp(obj1->scroll->act_use,		 obj2->scroll->act_use	   ) )		return FALSE;
		if ( str_cmp(obj1->scroll->act_use_room, obj2->scroll->act_use_room) )		return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_POTION:
	  {
		if ( !obj1->potion && !obj2->potion )		return TRUE;
		if ( !obj1->potion &&  obj2->potion )		return FALSE;
		if (  obj1->potion && !obj2->potion )		return FALSE;

		if ( obj1->potion->level 			!=	 obj2->potion->level		  )		return FALSE;
		if ( obj1->potion->sn1	 			!=	 obj2->potion->sn1			  )		return FALSE;
		if ( obj1->potion->sn2	 			!=	 obj2->potion->sn2			  )		return FALSE;
		if ( obj1->potion->sn3	 			!=	 obj2->potion->sn3			  )		return FALSE;
		if ( str_cmp(obj1->potion->act_use,		 obj2->potion->act_use		) )		return FALSE;
		if ( str_cmp(obj1->potion->act_use_room, obj2->potion->act_use_room ) )		return FALSE;

		return TRUE;
	  }
	  break;


	  case OBJTYPE_PILL:
	  {
		if ( !obj1->pill && !obj2->pill )		return TRUE;
		if ( !obj1->pill &&  obj2->pill )		return FALSE;
		if (  obj1->pill && !obj2->pill )		return FALSE;

		if ( obj1->pill->level				!=	obj2->pill->level		   )	return FALSE;
		if ( obj1->pill->sn1				!=	obj2->pill->sn1			   )	return FALSE;
		if ( obj1->pill->sn2				!=	obj2->pill->sn2			   )	return FALSE;
		if ( obj1->pill->sn3				!=	obj2->pill->sn3			   )	return FALSE;
		if ( str_cmp(obj1->pill->act_use,		obj2->pill->act_use		 ) )	return FALSE;
		if ( str_cmp(obj1->pill->act_use_room,	obj2->pill->act_use_room ) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_WAND:
	  case OBJTYPE_STAFF:
	  {
		if ( !obj1->wand && !obj2->wand )		return TRUE;
		if ( !obj1->wand &&  obj2->wand )		return FALSE;
		if (  obj1->wand && !obj2->wand )		return FALSE;

		if ( obj1->wand->level					!=	obj2->wand->level			  )	 return FALSE;
		if ( obj1->wand->max_charges			!=	obj2->wand->max_charges		  )	 return FALSE;
		if ( obj1->wand->curr_charges			!=	obj2->wand->curr_charges	  )	 return FALSE;
		if ( obj1->wand->sn						!=	obj2->wand->sn				  )	 return FALSE;
		if ( str_cmp(obj1->wand->act_zap,			obj2->wand->act_zap			) )	 return FALSE;
		if ( str_cmp(obj1->wand->act_zap_room,		obj2->wand->act_zap_room	) )	 return FALSE;
		if ( str_cmp(obj1->wand->act_consume,		obj2->wand->act_consume		) )	 return FALSE;
		if ( str_cmp(obj1->wand->act_consume_room,	obj2->wand->act_consume_room) )	 return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_WEAPON:
	  case OBJTYPE_FIREWEAPON:
	  case OBJTYPE_MISSILE:
	  case OBJTYPE_MISSILE_WEAPON:
	  case OBJTYPE_PROJECTILE:
	  {
		if ( !obj1->weapon && !obj2->weapon )		return TRUE;
		if ( !obj1->weapon &&  obj2->weapon )		return FALSE;
		if (  obj1->weapon && !obj2->weapon )		return FALSE;

		if ( obj1->weapon->condition			!=  obj2->weapon->condition		  )	 return FALSE;
		if ( obj1->weapon->dice_number			!=  obj2->weapon->dice_number	  )	 return FALSE;
		if ( obj1->weapon->dice_size			!=  obj2->weapon->dice_size		  )	 return FALSE;
		if ( obj1->weapon->type					!=  obj2->weapon->type			  )	 return FALSE;
		if ( obj1->weapon->damage1				!=  obj2->weapon->damage1		  )	 return FALSE;
		if ( obj1->weapon->damage2				!=  obj2->weapon->damage2		  )	 return FALSE;
		if ( str_cmp(obj1->weapon->act_wield,		obj2->weapon->act_wield		) )	 return FALSE;
		if ( str_cmp(obj1->weapon->act_wield_room,	obj2->weapon->act_wield_room) )	 return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_TREASURE:
	  {
		if ( !obj1->treasure && !obj2->treasure )		return TRUE;
		if ( !obj1->treasure &&  obj2->treasure )		return FALSE;
		if (  obj1->treasure && !obj2->treasure )		return FALSE;

		if ( obj1->treasure->type		!= obj2->treasure->type			)	return FALSE;
		if ( obj1->treasure->condition	!= obj2->treasure->condition	)	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_DRESS:
	  case OBJTYPE_ARMOR:
	  {
		if ( !obj1->armor && !obj2->armor )		return TRUE;
		if ( !obj1->armor &&  obj2->armor )		return FALSE;
		if (  obj1->armor && !obj2->armor )		return FALSE;

		if ( obj1->armor->ac				!=   obj2->armor->ac			 )	return FALSE;
		if ( obj1->armor->ac_orig			!=   obj2->armor->ac_orig		 )	return FALSE;
		if ( obj1->armor->type				!=   obj2->armor->type			 )	return FALSE;
		if ( str_cmp(obj1->armor->act_wear,		 obj2->armor->act_wear	   ) )	return FALSE;
		if ( str_cmp(obj1->armor->act_wear_room, obj2->armor->act_wear_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_FURNITURE:
	  {
		if ( !obj1->furniture && !obj2->furniture )		return TRUE;
		if ( !obj1->furniture &&  obj2->furniture )		return FALSE;
		if (  obj1->furniture && !obj2->furniture )		return FALSE;

		if ( obj1->furniture->regain[POINTS_LIFE]	!= obj2->furniture->regain[POINTS_LIFE] )	return FALSE;
		if ( obj1->furniture->regain[POINTS_MOVE]	!= obj2->furniture->regain[POINTS_MOVE] )	return FALSE;
		if ( obj1->furniture->regain[POINTS_MANA]	!= obj2->furniture->regain[POINTS_MANA] )	return FALSE;
		if ( obj1->furniture->regain[POINTS_SOUL]	!= obj2->furniture->regain[POINTS_SOUL] )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_CONTAINER:
	  case OBJTYPE_QUIVER:
	  case OBJTYPE_KEYRING:
	  {
		if ( !obj1->container && !obj2->container )		return TRUE;
		if ( !obj1->container &&  obj2->container )		return FALSE;
		if (  obj1->container && !obj2->container )		return FALSE;

		if ( obj1->container->capacity				!=  obj2->container->capacity		 )	return FALSE;
		if ( obj1->container->key					!=  obj2->container->key			 )	return FALSE;
		if ( obj1->container->durability			!=  obj2->container->durability		 )	return FALSE;
		if ( obj1->container->type					!=  obj2->container->type			 )	return FALSE;
		if ( obj1->container->flags					!=  obj2->container->flags			 )	return FALSE;
		if ( str_cmp(obj1->container->act_open,			obj2->container->act_open	   ) )	return FALSE;
		if ( str_cmp(obj1->container->act_open_room,	obj2->container->act_open_room ) )	return FALSE;
		if ( str_cmp(obj1->container->act_close,		obj2->container->act_close	   ) )	return FALSE;
		if ( str_cmp(obj1->container->act_close_room,	obj2->container->act_close_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_DRINK_CON:
  	  case OBJTYPE_FOUNTAIN:
	  case OBJTYPE_BLOOD:
	  case OBJTYPE_BLOODSTAIN:
	  case OBJTYPE_HERB_CON:
	  {
		if ( !obj1->drinkcon && !obj2->drinkcon )		return TRUE;
		if ( !obj1->drinkcon &&  obj2->drinkcon )		return FALSE;
		if (  obj1->drinkcon && !obj2->drinkcon )		return FALSE;

		if ( obj1->drinkcon->capacity				!=	obj2->drinkcon->capacity		)	return FALSE;
		if ( obj1->drinkcon->curr_quant				!=	obj2->drinkcon->curr_quant		)	return FALSE;
		if ( obj1->drinkcon->liquid					!=	obj2->drinkcon->liquid			)	return FALSE;
		if ( obj1->drinkcon->poison					!=	obj2->drinkcon->poison			)	return FALSE;
		if ( obj1->drinkcon->fountain				!=	obj2->drinkcon->fountain		)	return FALSE;
		if ( str_cmp(obj1->drinkcon->act_drink,			obj2->drinkcon->act_drink	  ) )	return FALSE;
		if ( str_cmp(obj1->drinkcon->act_drink_room,	obj2->drinkcon->act_drink_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_KEY:
	  {
		if ( !obj1->key && !obj2->key )		return TRUE;
		if ( !obj1->key &&  obj2->key )		return FALSE;
		if (  obj1->key && !obj2->key )		return FALSE;

		if ( obj1->key->save					!=	obj2->key->save				)	return FALSE;
		if ( str_cmp(obj1->key->act_unlock,			obj2->key->act_unlock	  ) )	return FALSE;
		if ( str_cmp(obj1->key->act_unlock_room,	obj2->key->act_unlock_room) )	return FALSE;
		if ( str_cmp(obj1->key->act_lock,			obj2->key->act_lock		  ) )	return FALSE;
		if ( str_cmp(obj1->key->act_lock_room,		obj2->key->act_lock_room  ) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_COOK:
	  case OBJTYPE_FOOD:
	  {
		if ( !obj1->food && !obj2->food )		return TRUE;
		if ( !obj1->food &&  obj2->food )		return FALSE;
		if (  obj1->food && !obj2->food )		return FALSE;

		if ( obj1->food->hours				!=	obj2->food->hours		  )	 return FALSE;
		if ( obj1->food->condition			!=	obj2->food->condition	  )	 return FALSE;
		if ( obj1->food->poison				!=	obj2->food->poison		  )	 return FALSE;
		if ( obj1->food->init_cond			!=	obj2->food->init_cond	  )	 return FALSE;
		if ( obj1->food->flags				!=	obj2->food->flags		  )	 return FALSE;
		if ( str_cmp(obj1->food->act_eat,		obj2->food->act_eat		) )	 return FALSE;
		if ( str_cmp(obj1->food->act_eat_room,	obj2->food->act_eat_room) )	 return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_MONEY:
	  {
		if ( !obj1->money && !obj2->money )		return TRUE;
		if ( !obj1->money &&  obj2->money )		return FALSE;
		if (  obj1->money && !obj2->money )		return FALSE;

		if ( obj1->money->qnt			!= obj2->money->qnt			)	return FALSE;
		if ( obj1->money->qnt_recycle	!= obj2->money->qnt_recycle	)	return FALSE;
		if ( obj1->money->type			!= obj2->money->type		)	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_PEN:
	  {
		if ( !obj1->pen && !obj2->pen )		return TRUE;
		if ( !obj1->pen &&  obj2->pen )		return FALSE;
		if (  obj1->pen && !obj2->pen )		return FALSE;

		if ( obj1->pen->ink					!=	obj2->pen->ink			   )	return FALSE;
		if ( str_cmp(obj1->pen->act_write,		obj2->pen->act_write	 ) )	return FALSE;
		if ( str_cmp(obj1->pen->act_write_room,	obj2->pen->act_write_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_BOAT:
	  {
		if ( !obj1->boat && !obj2->boat )		return TRUE;
		if ( !obj1->boat &&  obj2->boat )		return FALSE;
		if (  obj1->boat && !obj2->boat )		return FALSE;

		if ( obj1->boat->rooms					!=  obj2->boat->rooms			 )	return FALSE;
		if ( str_cmp(obj1->boat->act_travel,		obj2->boat->act_travel	   ) )	return FALSE;
		if ( str_cmp(obj1->boat->act_travel_room,	obj2->boat->act_travel_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_CORPSE_MOB:
	  case OBJTYPE_CORPSE_PG:
	  {
		if ( !obj1->corpse && !obj2->corpse )		return TRUE;
		if ( !obj1->corpse &&  obj2->corpse )		return FALSE;
		if (  obj1->corpse && !obj2->corpse )		return FALSE;

		if ( obj1->corpse->time			!= obj2->corpse->time		)	return FALSE;
		if ( obj1->corpse->tick			!= obj2->corpse->tick		)	return FALSE;
		if ( obj1->corpse->level		!= obj2->corpse->level		)	return FALSE;
		if ( obj1->corpse->looted		!= obj2->corpse->looted		)	return FALSE;
		if ( obj1->corpse->pg			!= obj2->corpse->pg			)	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_PIPE:
	  {
		if ( !obj1->pipe && !obj2->pipe )		return TRUE;
		if ( !obj1->pipe &&  obj2->pipe )		return FALSE;
		if (  obj1->pipe && !obj2->pipe )		return FALSE;

		if ( obj1->pipe->capacity				!=  obj2->pipe->capacity		)	return FALSE;
		if ( obj1->pipe->draws					!=  obj2->pipe->draws			)	return FALSE;
		if ( obj1->pipe->herb					!=  obj2->pipe->herb			)	return FALSE;
		if ( obj1->pipe->flags					!=  obj2->pipe->flags			)	return FALSE;
		if ( str_cmp(obj1->pipe->act_smoke,			obj2->pipe->act_smoke	  ) )	return FALSE;
		if ( str_cmp(obj1->pipe->act_smoke_room,	obj2->pipe->act_smoke_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_HERB:
	  {
		if ( !obj1->herb && !obj2->herb )		return TRUE;
		if ( !obj1->herb &&  obj2->herb )		return FALSE;
		if (  obj1->herb && !obj2->herb )		return FALSE;

		if ( obj1->herb->charges	!= obj2->herb->charges )	return FALSE;
		if ( obj1->herb->hnum		!= obj2->herb->hnum )		return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_SWITCH:
	  case OBJTYPE_LEVER:
	  case OBJTYPE_PULLCHAIN:
	  case OBJTYPE_BUTTON:
	  {
		if ( !obj1->lever && !obj2->lever )		return TRUE;
		if ( !obj1->lever &&  obj2->lever )		return FALSE;
		if (  obj1->lever && !obj2->lever )		return FALSE;

		if ( obj1->lever->flags				!=	obj2->lever->flags		   )	return FALSE;
		if ( obj1->lever->vnum_sn			!=	obj2->lever->vnum_sn	   )	return FALSE;
		if ( obj1->lever->vnum				!=	obj2->lever->vnum		   )	return FALSE;
		if ( obj1->lever->vnum_val			!=	obj2->lever->vnum_val	   )	return FALSE;
		if ( str_cmp(obj1->lever->act_use,		obj2->lever->act_use	 ) )	return FALSE;
		if ( str_cmp(obj1->lever->act_use_room,	obj2->lever->act_use_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_TRAP:
	  {
		if ( !obj1->trap && !obj2->trap )		return TRUE;
		if ( !obj1->trap &&  obj2->trap )		return FALSE;
		if (  obj1->trap && !obj2->trap )		return FALSE;

		if ( obj1->trap->charges				!=	obj2->trap->charges			   )	return FALSE;
		if ( obj1->trap->type					!=	obj2->trap->type			   )	return FALSE;
		if ( obj1->trap->level					!=	obj2->trap->level			   )	return FALSE;
		if ( obj1->trap->flags					!=	obj2->trap->flags			   )	return FALSE;
		if ( str_cmp(obj1->trap->act_activate,		obj2->trap->act_activate	 ) )	return FALSE;
		if ( str_cmp(obj1->trap->act_activate_room,	obj2->trap->act_activate_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_PAPER:
	  {
		if ( !obj1->paper && !obj2->paper )		return TRUE;
		if ( !obj1->paper &&  obj2->paper )		return FALSE;
		if (  obj1->paper && !obj2->paper )		return FALSE;

		if ( obj1->paper->text			!= obj2->paper->text	)		return FALSE;
		if ( obj1->paper->subject		!= obj2->paper->subject	)		return FALSE;
		if ( obj1->paper->to			!= obj2->paper->to		)		return FALSE;
		if ( obj1->paper->type			!= obj2->paper->type	)		return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_SALVE:
	  {
		if ( !obj1->salve && !obj2->salve )		return TRUE;
		if ( !obj1->salve &&  obj2->salve )		return FALSE;
		if (  obj1->salve && !obj2->salve )		return FALSE;

		if ( obj1->salve->level					!=	obj2->salve->level			 )	return FALSE;
		if ( obj1->salve->charges				!=	obj2->salve->charges		 )	return FALSE;
		if ( obj1->salve->max_charges			!=	obj2->salve->max_charges	 )	return FALSE;
		if ( obj1->salve->delay					!=	obj2->salve->delay			 )	return FALSE;
		if ( obj1->salve->sn1					!=	obj2->salve->sn1			 )	return FALSE;
		if ( obj1->salve->sn2					!=	obj2->salve->sn2			 )	return FALSE;
		if ( str_cmp(obj1->salve->act_apply,		obj2->salve->act_apply	   ) )	return FALSE;
		if ( str_cmp(obj1->salve->act_apply_room,	obj2->salve->act_apply_room) )	return FALSE;

		return TRUE;
	  }
	  break;

	  case OBJTYPE_SHEATH:
	  {
		if ( !obj1->sheath && !obj2->sheath )		return TRUE;
		if ( !obj1->sheath &&  obj2->sheath )		return FALSE;
		if (  obj1->sheath && !obj2->sheath )		return FALSE;

		if ( obj1->sheath->weapon_type			!=	obj2->sheath->weapon_type	  )	 return FALSE;
		if ( obj1->sheath->vnum					!=	obj2->sheath->vnum			  )	 return FALSE;
		if ( obj1->sheath->flags				!=	obj2->sheath->flags			  )	 return FALSE;
		if ( str_cmp(obj1->sheath->act_open,		obj2->sheath->act_open		) )	 return FALSE;
		if ( str_cmp(obj1->sheath->act_open_room,	obj2->sheath->act_open_room	) )	 return FALSE;
		if ( str_cmp(obj1->sheath->act_close,		obj2->sheath->act_close		) )	 return FALSE;
		if ( str_cmp(obj1->sheath->act_close_room,	obj2->sheath->act_close_room) )	 return FALSE;

		return TRUE;
	  }
	}
}


/*
 * Visualizza tutte i value di un oggetto
 */
void show_values( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "show_values: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "show_values: obj è NULL" );
		return;
	}

	if ( obj->pObjProto->light || obj->light )
	{
		send_to_pager( ch, "&WLuce       &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Intensity   %-33d %-34d\r\n",
			(obj->pObjProto->light) ? obj->pObjProto->light->intensity : 0,
			(obj->light)			? obj->light->intensity : 0 );
		pager_printf( ch, "Type        %-33d %-34d\r\n",
			(obj->pObjProto->light) ? obj->pObjProto->light->type : 0,	/* (FF) farlo con code table */
			(obj->light)			? obj->light->type : 0 );
		pager_printf( ch, "Hours       %-33d %-34d\r\n",
			(obj->pObjProto->light) ? obj->pObjProto->light->hours : 0,
			(obj->light)			? obj->light->hours : 0 );
	}

	if ( obj->pObjProto->scroll || obj->scroll )
	{
		send_to_pager( ch, "&WScroll     &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Level       %-33d %-34d\r\n",
			(obj->pObjProto->scroll) ? obj->pObjProto->scroll->level : 0,
			(obj->scroll)			 ? obj->scroll->level : 0 );
		pager_printf( ch, "Sn1         %-33s %-34s\r\n",
			(obj->pObjProto->scroll) ? obj->pObjProto->scroll->sn1 == -1 ? "nessuno" : table_skill[obj->pObjProto->scroll->sn1]->name  :  "",
			(obj->scroll)			 ? obj->scroll->sn1 == -1 ? "nessuno" : table_skill[obj->scroll->sn1]->name  :  "" );
		pager_printf( ch, "Sn2         %-33s %-34s\r\n",
			(obj->pObjProto->scroll) ? obj->pObjProto->scroll->sn2 == -1 ? "nessuno" : table_skill[obj->pObjProto->scroll->sn2]->name  :  "",
			(obj->scroll)			 ? obj->scroll->sn2 == -1 ? "nessuno" : table_skill[obj->scroll->sn2]->name  :  "" );
		pager_printf( ch, "Sn3         %-33s %-34s\r\n",
			(obj->pObjProto->scroll) ? obj->pObjProto->scroll->sn3 == -1 ? "nessuno" : table_skill[obj->pObjProto->scroll->sn3]->name  :  "",
			(obj->scroll)			 ? obj->scroll->sn3 == -1 ? "nessuno" : table_skill[obj->scroll->sn3]->name  :  "" );
	}

	if ( obj->pObjProto->potion || obj->potion )
	{
		send_to_pager( ch, "&WPotion     &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Level       %-33d %-34d\r\n",
			(obj->pObjProto->potion) ? obj->pObjProto->potion->level : 0,
			(obj->potion)			 ? obj->potion->level : 0 );
		pager_printf( ch, "Sn1         %-33s %-34s\r\n",
			(obj->pObjProto->potion) ? obj->pObjProto->potion->sn1 == -1 ? "nessuno" : table_skill[obj->pObjProto->potion->sn1]->name  :  0,
			(obj->potion)			 ? obj->potion->sn1 == -1 ? "nessuno" : table_skill[obj->potion->sn1]->name  :  "" );
		pager_printf( ch, "Sn2         %-33s %-34s\r\n",
			(obj->pObjProto->potion) ? obj->pObjProto->potion->sn2 == -1 ? "nessuno" : table_skill[obj->pObjProto->potion->sn2]->name  :  0,
			(obj->potion)			 ? obj->potion->sn2 == -1 ? "nessuno" : table_skill[obj->potion->sn2]->name  :  "" );
		pager_printf( ch, "Sn3         %-33s %-34s\r\n",
			(obj->pObjProto->potion) ? obj->pObjProto->potion->sn3 == -1 ? "nessuno" : table_skill[obj->pObjProto->potion->sn3]->name  :  0,
			(obj->potion)			 ? obj->potion->sn3 == -1 ? "nessuno" : table_skill[obj->potion->sn3]->name  :  "" );
	}

	if ( obj->pObjProto->pill || obj->pill )
	{
		send_to_pager( ch, "&WPill       &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Level       %-33d %-34d\r\n",
			(obj->pObjProto->pill) ? obj->pObjProto->pill->level : 0,
			(obj->pill)			   ? obj->pill->level : 0 );
		pager_printf( ch, "Sn1         %-33s %-34s\r\n",
			(obj->pObjProto->pill) ? obj->pObjProto->pill->sn1 == -1 ? "nessuno" : table_skill[obj->pObjProto->pill->sn1]->name  :  0,
			(obj->pill)			   ? obj->pill->sn1 == -1 ? "nessuno" : table_skill[obj->pill->sn1]->name  :  0 );
		pager_printf( ch, "Sn2         %-33s %-34s\r\n",
			(obj->pObjProto->pill) ? obj->pObjProto->pill->sn2 == -1 ? "nessuno" : table_skill[obj->pObjProto->pill->sn2]->name  :  0,
			(obj->pill)			   ? obj->pill->sn2 == -1 ? "nessuno" : table_skill[obj->pill->sn2]->name  :  0 );
		pager_printf( ch, "Sn3         %-33s %-34s\r\n",
			(obj->pObjProto->pill) ? obj->pObjProto->pill->sn3 == -1 ? "nessuno" : table_skill[obj->pObjProto->pill->sn3]->name  :  0,
			(obj->pill)			   ? obj->pill->sn3 == -1 ? "nessuno" : table_skill[obj->pill->sn3]->name  :  0 );
	}

	if ( obj->pObjProto->wand || obj->wand )
	{
		pager_printf( ch, "&W%-5.5s           &cValue Prototipo                  Value Oggetto\r\n",
			OBJTYPE_WAND ? "Wand" : "Staff" );
		pager_printf( ch, "Level       %-33d %-34d\r\n",
			(obj->pObjProto->wand) ? obj->pObjProto->wand->level : 0,
			(obj->wand)			   ? obj->wand->level : 0 );
		pager_printf( ch, "MaxCharges  %-33d %-34d\r\n",
			(obj->pObjProto->wand) ? obj->pObjProto->wand->max_charges : 0,
			(obj->wand)			   ? obj->wand->max_charges : 0 );
		pager_printf( ch, "CurrCharges %-33d %-34d\r\n",
			(obj->pObjProto->wand) ? obj->pObjProto->wand->curr_charges : 0,
			(obj->wand)			   ? obj->wand->curr_charges : 0 );
		pager_printf( ch, "Sn          %-33s %-34s\r\n",
			(obj->pObjProto->wand) ? obj->pObjProto->wand->sn == -1 ? "nessuno" : table_skill[obj->pObjProto->wand->sn]->name  :  "",
			(obj->wand)			   ? obj->wand->sn == -1 ? "nessuno" : table_skill[obj->wand->sn]->name  :  "" );
	}

	if ( obj->pObjProto->weapon || obj->weapon )
	{
		pager_printf( ch, "&W%-10.10s &cValue Prototipo                  Value Oggetto\r\n",
			OBJTYPE_WEAPON			? "Weapon"  :
			OBJTYPE_MISSILE_WEAPON ? "Missile" :
			OBJTYPE_PROJECTILE		? "Projectile" :
								  "FireWeapon" );
		pager_printf( ch, "Condition   %-33d %-34d\r\n",
			(obj->pObjProto->weapon) ? obj->pObjProto->weapon->condition : 0,
			(obj->weapon)			 ? obj->weapon->condition : 0 );
		pager_printf( ch, "DiceNumber  %-33d %-34d\r\n",
			(obj->pObjProto->weapon) ? obj->pObjProto->weapon->dice_number : 0,
			(obj->weapon)			 ? obj->weapon->dice_number : 0 );
		pager_printf( ch, "DiceSize    %-33d %-34d\r\n",
			(obj->pObjProto->weapon) ? obj->pObjProto->weapon->dice_size : 0,
			(obj->weapon)			 ? obj->weapon->dice_size : 0 );
		pager_printf( ch, "Type        %-33d %-34d\r\n",
			(obj->pObjProto->weapon) ? obj->pObjProto->weapon->type : 0,	/* (FF) table code */
			(obj->weapon)			 ? obj->weapon->type : 0 );
		pager_printf( ch, "Damage1     %-33d %-34d\r\n",
			(obj->pObjProto->weapon) ? obj->pObjProto->weapon->damage1 : 0,
			(obj->weapon)			 ? obj->weapon->damage1 : 0 );
		pager_printf( ch, "Damage2     %-33d %-34d\r\n",
			(obj->pObjProto->weapon) ? obj->pObjProto->weapon->damage2 : 0,
			(obj->weapon)			 ? obj->weapon->damage2 : 0 );
	}

	if ( obj->pObjProto->treasure || obj->treasure )
	{
		send_to_pager( ch, "&WTesoro     &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Type        %-33d %-34d\r\n",
			(obj->pObjProto->treasure) ? obj->pObjProto->treasure->type : 0,
			(obj->treasure)			   ? obj->treasure->type : 0 );
		pager_printf( ch, "Condition   %-33d %-34d\r\n",
			(obj->pObjProto->treasure) ? obj->pObjProto->treasure->condition : 0,
			(obj->treasure)			   ? obj->treasure->condition : 0 );
	}

	if ( obj->pObjProto->armor || obj->armor )
	{
		pager_printf( ch, "&W%-8.8s   &cValue Prototipo                  Value Oggetto\r\n", OBJTYPE_ARMOR ? "Armatura" : "Vestito" );
		pager_printf( ch, "Ac          %-33d %-34d\r\n",
			(obj->pObjProto->armor) ? obj->pObjProto->armor->ac : 0,
			(obj->armor)			? obj->armor->ac : 0 );
		pager_printf( ch, "AcOrig      %-33d %-34d\r\n",
			(obj->pObjProto->armor) ? obj->pObjProto->armor->ac_orig : 0,
			(obj->armor)			? obj->armor->ac_orig : 0 );
		pager_printf( ch, "Type        %-33d %-34d\r\n",
			(obj->pObjProto->armor) ? obj->pObjProto->armor->type : 0,
			(obj->armor)			? obj->armor->type : 0 );
	}

	if ( obj->pObjProto->furniture || obj->furniture )
	{
		send_to_pager( ch, "&WArredo     &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Regain      %-7d %-7d %-7d %-7d  %-7d %-7d %-7d %-7d\r\n",
			(obj->pObjProto->furniture) ? obj->pObjProto->furniture->regain[POINTS_LIFE] : 0,
			(obj->pObjProto->furniture) ? obj->pObjProto->furniture->regain[POINTS_MOVE] : 0,
			(obj->pObjProto->furniture) ? obj->pObjProto->furniture->regain[POINTS_MANA] : 0,
			(obj->pObjProto->furniture) ? obj->pObjProto->furniture->regain[POINTS_SOUL] : 0,
			(obj->furniture)			? obj->furniture->regain[POINTS_LIFE] : 0,
			(obj->furniture)			? obj->furniture->regain[POINTS_MOVE] : 0,
			(obj->furniture)			? obj->furniture->regain[POINTS_MANA] : 0,
			(obj->furniture)			? obj->furniture->regain[POINTS_SOUL] : 0 );
	}

	if ( obj->pObjProto->container || obj->container )
	{
		send_to_pager( ch, "&WContainer  &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Capacity    %-33d %-34d\r\n",
			(obj->pObjProto->container) ? obj->pObjProto->container->capacity : 0,
			(obj->container)			? obj->container->capacity : 0 );
		pager_printf( ch, "Flags       %33s %-34s\r\n",
			(obj->pObjProto->container) ? code_bitint(NULL, obj->pObjProto->container->flags, CODE_CONTAINER) : "",
			(obj->container)			? code_bitint(NULL, obj->container->flags, CODE_CONTAINER) : "" );
		pager_printf( ch, "Key         %-33d %-34d\r\n",
			(obj->pObjProto->container) ? obj->pObjProto->container->key : 0,
			(obj->container)			? obj->container->key : 0 );
		pager_printf( ch, "Durability  %-33d %-34d\r\n",
			(obj->pObjProto->container) ? obj->pObjProto->container->durability : 0,
			(obj->container)			? obj->container->durability : 0 );
	}

	if ( obj->pObjProto->drinkcon || obj->drinkcon )
	{
		send_to_pager( ch, "&WCont liqui &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Capacity    %-33d %-34d\r\n",
			(obj->pObjProto->drinkcon) ? obj->pObjProto->drinkcon->capacity : 0,
			(obj->drinkcon)			   ? obj->drinkcon->capacity : 0 );
		pager_printf( ch, "CurrQuant   %-33d %-34d\r\n",
			(obj->pObjProto->drinkcon) ? obj->pObjProto->drinkcon->curr_quant : 0,
			(obj->drinkcon)			   ? obj->drinkcon->curr_quant : 0 );
		pager_printf( ch, "Liquid      %33s %-34s\r\n",
			(obj->pObjProto->drinkcon) ? table_liquid[obj->pObjProto->drinkcon->liquid].name : "",
			(obj->drinkcon)			   ? table_liquid[obj->drinkcon->liquid].name : "" );
		pager_printf( ch, "Poison      %-33d %-34d\r\n",
			(obj->pObjProto->drinkcon) ? obj->pObjProto->drinkcon->poison : 0,
			(obj->drinkcon)			   ? obj->drinkcon->poison : 0 );
		pager_printf( ch, "Fountain    %33s %-34s\r\n",
			(obj->pObjProto->drinkcon) ?  (obj->pObjProto->drinkcon->fountain) ? "SI" : "no"  : "",
			(obj->drinkcon)			   ?  (obj->drinkcon->fountain) ? "SI" : "no"  : "" );
	}

	if ( obj->pObjProto->key || obj->key )
	{
		send_to_pager( ch, "&WChiave     &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Save        %33s %-34s\r\n",
			(obj->pObjProto->key) ?  (obj->pObjProto->key->save) ? "SI" : "no"  :  "",
			(obj->key)			  ?  (obj->key->save) ? "SI" : "no"  :  "" );
	}

	if ( obj->pObjProto->food || obj->food )
	{
		send_to_pager( ch, "&WCibo       &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Hours       %-33d %-34d\r\n",
			(obj->pObjProto->food) ? obj->pObjProto->food->hours : 0,
			(obj->food)			   ? obj->food->hours : 0 );
		pager_printf( ch, "Condition   %-33d %-34d\r\n",
			(obj->pObjProto->food) ? obj->pObjProto->food->condition : 0,
			(obj->food)			   ? obj->food->condition : 0 );
		pager_printf( ch, "Poison      %-33d %-34d\r\n",
			(obj->pObjProto->food) ? obj->pObjProto->food->poison : 0,
			(obj->food)			   ? obj->food->poison : 0 );
	}

	if ( obj->pObjProto->money || obj->money )
	{
		send_to_pager( ch, "&WMoneta     &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Qnt         %-33d %-34d\r\n",
			(obj->pObjProto->money) ? obj->pObjProto->money->qnt : 0,
			(obj->money)			? obj->money->qnt : 0 );
		pager_printf( ch, "QntRecycle  %-33d %-34d\r\n",
			(obj->pObjProto->money) ? obj->pObjProto->money->qnt_recycle : 0,
			(obj->money)			? obj->money->qnt_recycle : 0 );
		pager_printf( ch, "Type        %-33d %-34d\r\n",
			(obj->pObjProto->money) ? obj->pObjProto->money->type : 0,
			(obj->money)			? obj->money->type : 0 );
	}

	if ( obj->pObjProto->corpse || obj->corpse )
	{
		send_to_pager( ch, "&WCadavere   &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Time        %-33d %-34d\r\n",
			(obj->pObjProto->corpse) ? obj->pObjProto->corpse->time : 0,
			(obj->corpse)			 ? obj->corpse->time : 0 );
		pager_printf( ch, "Pg          %-33s %-34s\r\n",
			(obj->pObjProto->corpse) ? (obj->pObjProto->corpse->pg) ? "SI" : "no"  :  "",
			(obj->corpse)			 ? (obj->corpse->pg) ? "SI" : "no"  :  "" );
	}

	if ( obj->pObjProto->trap || obj->trap )
	{
		send_to_pager( ch, "&WTrappola   &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Charges     %-33d %-34d\r\n",
			(obj->pObjProto->trap) ? obj->pObjProto->trap->charges : 0,
			(obj->trap)			   ? obj->trap->charges : 0 );
		pager_printf( ch, "Type        %-33d %-34d\r\n",
			(obj->pObjProto->trap) ? obj->pObjProto->trap->type : 0,
			(obj->trap)			   ? obj->trap->type : 0 );
		pager_printf( ch, "Level       %-33d %-34d\r\n",
			(obj->pObjProto->trap) ? obj->pObjProto->trap->level : 0,
			(obj->trap)			   ? obj->trap->level : 0 );
		pager_printf( ch, "Flags       %-33s %-34s\r\n",
			(obj->pObjProto->trap) ? code_bitint(NULL, obj->pObjProto->trap->flags, CODE_TRAPFLAG) : "",
			(obj->trap)			   ? code_bitint(NULL, obj->pObjProto->trap->flags, CODE_TRAPFLAG) : "" );
	}

	if ( obj->pObjProto->paper || obj->paper )
	{
		send_to_pager( ch, "&WCarta      &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Testo       %-33d %-34d\r\n",
			(obj->pObjProto->paper) ? obj->pObjProto->paper->text : 0,
			(obj->paper)			? obj->paper->text : 0 );
		pager_printf( ch, "Soggetto    %-33d %-34d\r\n",
			(obj->pObjProto->paper) ? obj->pObjProto->paper->subject : 0,
			(obj->paper)			? obj->paper->subject : 0 );
		pager_printf( ch, "A           %-33d %-34d\r\n",
			(obj->pObjProto->paper) ? obj->pObjProto->paper->to : 0,
			(obj->paper)			? obj->paper->to : 0 );
		pager_printf( ch, "Tipo        %-33d %-34d\r\n",
			(obj->pObjProto->paper) ? obj->pObjProto->paper->type : 0,
			(obj->paper)			? obj->paper->type : 0 );
	}

	if ( obj->pObjProto->salve || obj->salve )
	{
		send_to_pager( ch, "&WPomata      &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Livello     %-33d %-34d\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->level : 0,
			(obj->salve)			? obj->salve->level : 0 );
		pager_printf( ch, "Cariche     %-33d %-34d\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->charges : 0,
			(obj->salve)			? obj->salve->charges : 0 );
		pager_printf( ch, "CaricheMax  %-33d %-34d\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->max_charges : 0,
			(obj->salve)			? obj->salve->max_charges : 0 );
		pager_printf( ch, "Attesa      %-33d %-34d\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->delay : 0,
			(obj->salve)			? obj->salve->delay : 0 );
		pager_printf( ch, "Spell1      %-33s %-34s\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->sn1 == -1 ? "nessuno" : table_skill[obj->pObjProto->salve->sn1]->name :  "",
			(obj->salve)			? obj->salve->sn1 == -1 ? "nessuno" : table_skill[obj->salve->sn1]->name  :  "" );
		pager_printf( ch, "Spell2      %-33s %-34s\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->sn2 == -1 ? "nessuno" : table_skill[obj->pObjProto->salve->sn2]->name :  "",
			(obj->salve)			? obj->salve->sn2 == -1 ? "nessuno" : table_skill[obj->salve->sn2]->name :  "" );
		pager_printf( ch, "ActApply    %-33s %-34s\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->act_apply : "",
			(obj->salve)			? obj->salve->act_apply : "" );
		pager_printf( ch, "ActApplyRoo %-33s %-34s\r\n",
			(obj->pObjProto->salve) ? obj->pObjProto->salve->act_apply_room : "",
			(obj->salve)			? obj->salve->act_apply_room : "" );
	}

	if ( obj->pObjProto->sheath || obj->sheath )
	{
		send_to_pager( ch, "&WFodero     &cValue Prototipo                  Value Oggetto\r\n" );
		pager_printf( ch, "Tipo arma  %-33s %-34s\r\n",
			obj->pObjProto->sheath	? code_str(NULL, obj->pObjProto->sheath->weapon_type, CODE_WEAPON) : "",
			obj->sheath				? code_str(NULL, obj->sheath->weapon_type, CODE_WEAPON) : "" );
		pager_printf( ch, "Vnum ogget %-33d %-34d\r\n",
			obj->pObjProto->sheath	? obj->pObjProto->sheath->vnum : 0,
			obj->sheath				? obj->sheath->vnum : 0 );
		pager_printf( ch, "Flags      %-33d %-34d\r\n",
			obj->pObjProto->sheath	? obj->pObjProto->sheath->flags	: 0,
			obj->sheath				? obj->sheath->flags : 0 );			/* (FF) per ora numero */
		pager_printf( ch, "Open       %-33s %-34s\r\n",
			obj->pObjProto->sheath	? obj->pObjProto->sheath->act_open : "",
			obj->sheath				? obj->sheath->act_open : "" );
		pager_printf( ch, "Open Room  %-33s %-34s\r\n",
			obj->pObjProto->sheath	? obj->pObjProto->sheath->act_open_room : "",
			obj->sheath				? obj->sheath->act_open_room : "" );
		pager_printf( ch, "Close      %-33s %-34s\r\n",
			obj->pObjProto->sheath	? obj->pObjProto->sheath->act_close : "",
			obj->sheath				? obj->sheath->act_close : "" );
		pager_printf( ch, "Close Room %-33s %-34s\r\n",
			obj->pObjProto->sheath	? obj->pObjProto->sheath->act_close_room : "",
			obj->sheath				? obj->sheath->act_close_room : "" );
	}
}
