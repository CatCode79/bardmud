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


#include <dirent.h>

#include "mud.h"
#include "calendar.h"
#include "db.h"
#include "fread.h"
#include "interpret.h"
#include "mprog.h"
#include "room.h"
#include "save.h"


/*
 * Definizioni
 */
#define	STORAGE_DIR		AREA_DIR "/storages/"	/* directory delle stanze magazzino */


/*
 * Variabili
 */
static OBJ_DATA	   *rgObjNest[MAX_NEST];	/* per gli storage */


/*
 * Tabella con tutte le informazioni di settore di una stanza
 */
const struct sector_type table_sector[] =
{
/*		id					code					name					back fore invert	symb	pass   loss   R    G    B	 tile */
	{ {	SECTOR_INSIDE,		"SECTOR_INSIDE",		"interno"			},	" ", "x", " ",		" ",	FALSE,	1/*,	  0,   0,   0,	"w.gif"*/ },
	{ {	SECTOR_CITY,		"SECTOR_CITY",			"città"				},	" ", "Y", " ",		":",	TRUE,	2/*,	255, 128,  64,	"w.gif"*/ },
	{ {	SECTOR_FIELD,		"SECTOR_FIELD",			"pianura"			},	" ", "G", " ",		"+",	TRUE,	2/*,	141, 215,   1,	"w.gif"*/ },
	{ {	SECTOR_FOREST,		"SECTOR_FOREST",		"foresta"			},	" ", "g", " ",		"+",	TRUE,	3/*,	  0, 108,  47,	"w.gif"*/ },
	{ {	SECTOR_HILLS,		"SECTOR_HILLS",			"collina"			},	" ", "O", " ",		"^^",	TRUE,	4/*,	140, 102,  54,	"w.gif"*/ },
	{ {	SECTOR_MOUNTAIN,	"SECTOR_MOUNTAIN",		"montagna"			},	" ", "w", " ",		"^^",	TRUE,	6/*,	152, 152, 152,	"w.gif"*/ },
	{ {	SECTOR_WATER_SWIM,	"SECTOR_WATER_SWIM",	"acqua"				},	" ", "C", " ",		"~",	TRUE,	4/*,	 89, 242, 251,	"w.gif"*/ },
	{ {	SECTOR_WATER_NOSWIM,"SECTOR_WATER_NOSWIM",	"acqua mossa"		},	" ", "B", " ",		"~",	TRUE,	10/*,	 67, 114, 251,	"w.gif"*/ },
	{ {	SECTOR_UNDERWATER,	"SECTOR_UNDERWATER",	"sott'acqua"		},	" ", "x", " ",		"?",	FALSE,	7/*,	  0,   0,   0,	"w.gif"*/ },
	{ {	SECTOR_AIR,			"SECTOR_AIR",			"aria"				},	" ", "x", " ",		"?",	FALSE,	1/*,	  0,   0,   0,	"w.gif"*/ },
	{ {	SECTOR_DESERT,		"SECTOR_DESERT",		"deserto"			},	" ", "Y", " ",		"~",	TRUE,	5/*,	241, 228, 145,	"w.gif"*/ },
	{ {	SECTOR_DUNNO,		"SECTOR_DUNNO",			"duna"				},	" ", "Y", " ",		"~",	TRUE,	6/*,	241, 228, 125,	"w.gif"*/ },
	{ {	SECTOR_OCEANFLOOR,	"SECTOR_OCEANFLOOR",	"fondale"			},	" ", "x", " ",		"?",	FALSE,	6/*,	  0,   0,   0,	"w.gif"*/ },
	{ {	SECTOR_UNDERGROUND,	"SECTOR_UNDERGROUND",	"sottoterra"		},	" ", "x", " ",		"?",	FALSE,	4/* 	  0,   0,   0,	"w.gif"*/ },
	{ {	SECTOR_LAVA,		"SECTOR_LAVA",			"lava"				},	" ", "R", " ",		":",	FALSE,	2/*,	245,  37,  29,	"w.gif"*/ },
	{ {	SECTOR_SWAMP,		"SECTOR_SWAMP",			"palude"			},	" ", "g", " ",		"~",	TRUE,	3/*,	218, 176,  56,	"w.gif"*/ }
};
const int max_check_sector = sizeof(table_sector)/sizeof(struct sector_type);


/*
 * Tabella codici sulle flag di uscita
 */
const CODE_DATA code_exit[] =
{
	{ EXIT_ISDOOR,		"EXIT_ISDOOR",		"isdoor"		},
	{ EXIT_CLOSED,		"EXIT_CLOSED",		"closed"		},
	{ EXIT_LOCKED,		"EXIT_LOCKED",		"locked"		},
	{ EXIT_SECRET,		"EXIT_SECRET",		"secret"		},
	{ EXIT_SWIM,		"EXIT_SWIM",		"swim"			},
	{ EXIT_PICKPROOF,	"EXIT_PICKPROOF",	"pickproof"		},
	{ EXIT_FLY,			"EXIT_FLY",			"fly"			},
	{ EXIT_CLIMB,		"EXIT_CLIMB",		"climb"			},
	{ EXIT_DIG,			"EXIT_DIG",			"dig"			},
	{ EXIT_EATKEY,		"EXIT_EATKEY",		"eatkey"		},
	{ EXIT_NOPASSDOOR,	"EXIT_NOPASSDOOR",	"nopassdoor"	},
	{ EXIT_HIDDEN,		"EXIT_HIDDEN",		"hidden"		},
	{ EXIT_PASSAGE,		"EXIT_PASSAGE",		"passage"		},
	{ EXIT_PORTAL,		"EXIT_PORTAL",		"portal"		},
	{ EXIT_RES1,		"EXIT_RES1",		"r1"			},
	{ EXIT_RES2,		"EXIT_RES2",		"r2"			},
	{ EXIT_xCLIMB,		"EXIT_xCLIMB",		"can_climb"		},
	{ EXIT_xENTER,		"EXIT_xENTER",		"can_enter"		},
	{ EXIT_xLEAVE,		"EXIT_xLEAVE",		"can_leave"		},
	{ EXIT_xAUTO,		"EXIT_xAUTO",		"auto"			},
	{ EXIT_NOFLEE,		"EXIT_NOFLEE",		"noflee"		},
	{ EXIT_xSEARCHABLE,	"EXIT_xSEARCHABLE",	"searchable"	},
	{ EXIT_BASHED,		"EXIT_BASHED",		"bashed"		},
	{ EXIT_BASHPROOF,	"EXIT_BASHPROOF",	"bashproof"		},
	{ EXIT_NOMOB,		"EXIT_NOMOB",		"nomob"			},
	{ EXIT_WINDOW,		"EXIT_WINDOW",		"window"		},
	{ EXIT_xLOOK,		"EXIT_xLOOK",		"can_look"		},
	{ EXIT_ISBOLT,		"EXIT_ISBOLT",		"isbolt"		},
	{ EXIT_BOLTED,		"EXIT_BOLTED",		"bolted"		}
};
const int max_check_exit = sizeof(code_exit)/sizeof(CODE_DATA);


/*
 * Tabella parsing sulle flag di stanza.
 * Bitvector.
 */
const CODE_DATA code_room[] =
{
	{ ROOM_DARK,			"ROOM_DARK",			"dark"			},
	{ ROOM_DEATH,			"ROOM_DEATH",			"death"			},
	{ ROOM_NOMOB,			"ROOM_NOMOB",			"nomob"			},
	{ ROOM_INDOORS,			"ROOM_INDOORS",			"indoors"		},
	{ ROOM_LAWFUL,			"ROOM_LAWFUL",			"lawful"		},
	{ ROOM_NEUTRAL,			"ROOM_NEUTRAL",			"neutral"		},
	{ ROOM_CHAOTIC,			"ROOM_CHAOTIC",			"chaotic"		},
	{ ROOM_NOMAGIC,			"ROOM_NOMAGIC",			"nomagic"		},
	{ ROOM_TUNNEL,			"ROOM_TUNNEL",			"tunnel"		},
	{ ROOM_PRIVATE,			"ROOM_PRIVATE",			"private"		},
	{ ROOM_SAFE,			"ROOM_SAFE",			"safe"			},
	{ ROOM_SOLITARY,		"ROOM_SOLITARY",		"solitary"		},
	{ ROOM_MASCOTTESHOP,	"ROOM_MASCOTTESHOP",	"mascotteshop"	},
	{ ROOM_NORECALL,		"ROOM_NORECALL",		"norecall"		},
	{ ROOM_DONATION,		"ROOM_DONATION",		"donation"		},
	{ ROOM_NODROPALL,		"ROOM_NODROPALL",		"nodropall"		},
	{ ROOM_SILENCE,			"ROOM_SILENCE",			"silence"		},
	{ ROOM_LOGSPEECH,		"ROOM_LOGSPEECH",		"logspeech"		},
	{ ROOM_NODROP,			"ROOM_NODROP",			"nodrop"		},
	{ ROOM_STOREROOM,		"ROOM_STOREROOM",		"storeroom"		},
	{ ROOM_NOSUMMON,		"ROOM_NOSUMMON",		"nosummon"		},
	{ ROOM_NOASTRAL,		"ROOM_NOASTRAL",		"noastral"		},
	{ ROOM_TELEPORT,		"ROOM_TELEPORT",		"teleport"		},
	{ ROOM_TELESHOWDESC,	"ROOM_TELESHOWDESC",	"teleshowdesc"	},
	{ ROOM_NOFLOOR,			"ROOM_NOFLOOR",			"nofloor"		},
	{ ROOM_NOSUPPLICATE,	"ROOM_NOSUPPLICATE",	"nosupplicate"	},
	{ ROOM_ARENA,			"ROOM_ARENA",			"arena"			},
	{ ROOM_TERRACE,			"ROOM_TERRACE",			"terrace"		},
	{ ROOM_NOMISSILE,		"ROOM_NOMISSILE",		"nomissile"		},
	{ ROOM_BFS_MARK,		"ROOM_BFS_MARK",		"bfs_mark"		},
	{ ROOM_RENT,			"ROOM_RENT",			"rent"			},
	{ ROOM_PROTOTYPE,		"ROOM_PROTOTYPE",		"prototype"		},
	{ ROOM_HELL,			"ROOM_HELL",			"hell"			},
#ifdef T2_ALFA
	{ ROOM_FLYING,			"ROOM_FLYING",			"flying"		},
	{ ROOM_NO_LANDING,		"ROOM_NO_LANDING",		"nolanding"		}
#endif
};
const int max_check_room = sizeof(code_room)/sizeof(CODE_DATA);


/*
 * Carica un file di magazzino
 */
void fread_storage( MUD_FILE *fp )
{
	ROOM_DATA	*room = NULL;
	OBJ_DATA	*tobj;
	OBJ_DATA	*tobj_next;
	char		*name;
	int			 iNest;
	VNUM		 vnum;

	name = name_from_file( fp->path );
	if ( !is_number(name) )
	{
		send_log( fp, LOG_BUG, "fread_storage: il nome del file non è formato da un vnum valido: %s", name );
		exit( EXIT_FAILURE );
	}

	vnum = atoi( name );
	room = get_room_index( fp, vnum );
	if ( !room )
	{
		send_log( fp, LOG_FREAD, "fread_storeroom: numero di vnum stanza magazzino errato %d", vnum );
		exit( EXIT_FAILURE );
	}
	rset_supermob( room );

	for ( iNest = 0;  iNest < MAX_NEST;  iNest++ )
		rgObjNest[iNest] = NULL;

	for ( ; ; )
	{
		char *word;
		char  letter;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_storage: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		letter = fread_letter( fp );
		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if ( letter != '#' )
		{
			send_log( fp, LOG_FREAD, "fread_storage: # non trovato" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( !strcmp(word, "OBJECT") )
		{
			fread_object_save( supermob, fp, OS_CARRY );
		}
		else if ( !strcmp(word, "END") )	/* Data la struttura particolare del file *.storage chiamato tramite fread_section ci vogliono due #END per una lettura corretta del file */
		{
			break;
		}
		else
		{
			send_log( fp, LOG_FREAD, "fread_storage: sezione errata %s", word );
			exit( EXIT_FAILURE );
		}
	}

	for ( tobj = supermob->first_carrying;  tobj;  tobj = tobj_next )
	{
		tobj_next = tobj->next_content;

		obj_from_char( tobj );
		obj_to_room( tobj, room );
	}

	release_supermob( );
}


/*
 * Carica tutti i magazzini.
 */
void load_storages( void )
{
	DIR			  *dp;
	struct dirent *dentry;

	dp = opendir( STORAGE_DIR );
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
		{
			MUD_FILE	*fp;
			char		 filename[MIL];

			if ( !str_suffix(".storage", dentry->d_name) )
			{
				sprintf( filename, "%s%s", STORAGE_DIR, dentry->d_name );
				fp = mud_fopen( "", filename, "r", TRUE );
				if ( fp )
					fread_storage( fp );
				MUD_FCLOSE( fp );
			}
		}
		dentry = readdir( dp );
	}
	closedir( dp );
}


/*
 * Salva gli oggetti contenuti nella stanza di un magazzino.
 */
void fwrite_storage( CHAR_DATA *ch )
{
	MUD_FILE *fp;
	char	  filename[MIL];
	int		  templvl;
	OBJ_DATA *contents;

	if ( !ch )
	{
		send_log( NULL, LOG_FWRITE, "fwrite_clan_storeroom: Null ch pointer!" );
		return;
	}

	sprintf( filename, "%d.storage", ch->in_room->vnum );
	fp = mud_fopen( STORAGE_DIR, filename, "w", TRUE );
	if ( !fp )
		return;

	templvl = get_level( ch );
	ch->level = LVL_LEGEND;		/* make sure EQ doesn't get lost */

	contents = ch->in_room->last_content;
	if ( contents && !HAS_BIT(contents->extra_flags, OBJFLAG_CLANOBJECT) )
		fwrite_object( ch, contents, fp, 0, OS_CARRY );
	fputs( "#END\n", fp->file );

	ch->level = templvl;

	MUD_FCLOSE( fp );
}


/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_DATA *pRoom )
{
	if ( !pRoom )
	{
		send_log( NULL, LOG_BUG, "room_is_dark: NULL pRoom" );
		return TRUE;
	}

	if ( pRoom->light > 0 )
		return FALSE;

	if ( HAS_BIT(pRoom->flags, ROOM_DARK) )
		return TRUE;

	if ( pRoom->sector == SECTOR_INSIDE
	  || pRoom->sector == SECTOR_CITY )
	{
		return FALSE;
	}

	if ( calendar.sunlight == SUN_SET
	  || calendar.sunlight == SUN_DARK )
	{
		return TRUE;
	}

	return FALSE;
}
