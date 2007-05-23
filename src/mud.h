#ifndef MUD_H
#define MUD_H


/*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*\
 >									PRINCIPALE FILE HEADER DEL MUD								 <
\*HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH*/


/* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == *\
								 		INCLUDE STANDARD
\* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == */

/*
 * Gli include del mud si trovano più sotto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>



/* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == *\
								 		TIPI DI STRUTTURE
\* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == */


/*
 * Definizioni delle strutture globali generiche
 */
typedef struct	affect_data				AFFECT_DATA;
typedef struct	alias_type				ALIAS_DATA;
typedef struct	area_data				AREA_DATA;
typedef struct	bitvector_data			BITVECTOR;
typedef struct	board_data				BOARD_DATA;
typedef struct	calendar_data			CALENDAR_DATA;
typedef struct	char_data				CHAR_DATA;
typedef struct	char_data_extracted		CHAR_DATA_EXTRACT;
typedef struct	char_morph				CHAR_MORPH;
typedef struct	code_data				CODE_DATA;
typedef struct	command_data			COMMAND_DATA;
typedef struct	clan_data				CLAN_DATA;
typedef struct	descriptor_data			DESCRIPTOR_DATA;
typedef struct	dream_data				DREAM_DATA;
typedef struct	ear_part				EAR_PART;
typedef struct	editor_data				EDITOR_DATA;
typedef struct	exit_data				EXIT_DATA;
typedef struct	extra_descr_data		EXTRA_DESCR_DATA;
typedef struct	eye_part				EYE_PART;
typedef struct	fighting_data			FIGHT_DATA;
typedef struct	hair_part				HAIR_PART;
typedef struct	hunt_hate_fear			HHF_DATA;
typedef struct	ignore_data				IGNORE_DATA;
typedef struct	killed_data				KILLED_DATA;
typedef struct	lyric_data				LYRIC_DATA;
#ifdef T2_MCCP
typedef struct	mccp_data				MCCP_DATA;
#endif
typedef struct	member_data				MEMBER_DATA;
typedef struct	mob_proto_data			MOB_PROTO_DATA;
typedef struct	mud_prog_data			MPROG_DATA;
typedef struct	mud_prog_act_list		MPROG_ACT_LIST;
typedef struct	morph_data				MORPH_DATA;
typedef struct	mud_file				MUD_FILE;
typedef struct	neighbor_data			NEIGHBOR_DATA;
typedef struct	note_data				NOTE_DATA;
typedef struct	object_data				OBJ_DATA;
typedef struct	object_data				OBJ_PROTO_DATA;
typedef struct	olc_data				OLC_DATA;
typedef struct	pg_data					PG_DATA;
typedef struct	pentagram_data			PENTAGRAM_DATA;
typedef struct	repairshop_data			REPAIR_DATA;
typedef struct	reset_data				RESET_DATA;
typedef struct	room_data				ROOM_DATA;
typedef struct	shop_data				SHOP_DATA;
typedef struct	skill_data				SKILL_DATA;
typedef struct	smaug_affect			SMAUG_AFF;
typedef struct	synonym_data			SYNO_DATA;
typedef struct	timer_data				TIMER;
typedef struct	values_light_data		VAL_LIGHT_DATA;
typedef struct	values_scroll_data		VAL_SCROLL_DATA;
typedef struct	values_scroll_data		VAL_POTION_DATA;	/* potion si rifà a scroll di base */
typedef struct	values_scroll_data		VAL_PILL_DATA;		/* anche pill si rifà a scroll */
typedef struct	values_wand_data		VAL_WAND_DATA;
typedef struct	values_weapon_data		VAL_WEAPON_DATA;
typedef struct	values_treasure_data	VAL_TREASURE_DATA;
typedef struct	values_armor_data		VAL_ARMOR_DATA;
typedef struct	values_furniture_data	VAL_FURNITURE_DATA;
typedef struct	values_trash_data		VAL_TRASH_DATA;
typedef struct	values_container_data	VAL_CONTAINER_DATA;
typedef struct	values_drinkcon_data	VAL_DRINKCON_DATA;
typedef struct	values_key_data			VAL_KEY_DATA;
typedef struct	values_food_data		VAL_FOOD_DATA;
typedef	struct	values_money_data		VAL_MONEY_DATA;
typedef	struct	values_pen_data			VAL_PEN_DATA;
typedef	struct	values_boat_data		VAL_BOAT_DATA;
typedef	struct	values_corpse_data		VAL_CORPSE_DATA;
typedef	struct	values_pipe_data		VAL_PIPE_DATA;
typedef	struct	values_herb_data		VAL_HERB_DATA;
typedef	struct	values_lever_data		VAL_LEVER_DATA;
typedef	struct	values_trap_data		VAL_TRAP_DATA;
typedef	struct	values_paper_data		VAL_PAPER_DATA;
typedef	struct	values_salve_data		VAL_SALVE_DATA;
typedef	struct	values_sheath_data		VAL_SHEATH_DATA;
typedef struct	weather_data			WEATHER_DATA;



/* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == *\
								 		DEFINIZIONI GLOBALI
\* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == */


/*
 * Short scalar types
 */
#if !defined( FALSE )
#	define FALSE	0
#endif

#if !defined( TRUE )
#	define TRUE		1
#endif

#if !defined( BERR )
#	define BERR		255
#endif

#if !defined( ALL )
#	define ALL		32677	/* Serve ad indicare che il valore è sia TRUE che FALSE */
#endif

typedef		unsigned int	bool;
typedef		int				ch_ret;
typedef		int				obj_ret;
typedef		int				VNUM;


/*
 * Con questi tre typedef si completa il lavoro fatto dalla e per la dlsym sulle funzioni dei
 *	comandi, così da poter evitare tutta la sfilza di DECLARE_DO_FUN, DECLARE_SPELL_FUN
 *	e DECLARE_SPEC_FUN
 */
typedef		void		DO_RET;		/* Per tutte quelle funzioni definite in commands.dat e skils.dat */
typedef		ch_ret		SPELL_RET;	/* Per tutte quelle funzione che sono definite in spells.dat */
typedef		bool		SPEC_RET;	/* Per tutte quelle funzione che sono definite in specials.dat */


/*
 * Definizioni di tipi di funzione
 */
typedef		DO_RET		DO_FUN	 ( CHAR_DATA *ch, char *argument );					/* Comando */
typedef		ch_ret		SPELL_FUN( int sn, int level, CHAR_DATA *ch, void *vo );	/* Incantesimo */
typedef		bool		SPEC_FUN ( CHAR_DATA *ch );									/* Procedura speciale */
typedef		void		FREAD_FUN( MUD_FILE *fp );									/* Lettura file per la fread_section */


/*
 * Parametri di gestione stringa e memoria
 */
#define MAX_KEY_HASH		(2048 * 2)
#define MSL					(4096 * 2)	/* MAX_STRING_LENGTH	buf */
#define MIL					 1024		/* MAX_INPUT_LENGTH		arg */
#define MLL					 80			/* MAX_LINE_LENGTH		line */
#define MAX_INBUF_SIZE		(1024 * 4)


/* Definizione del messaggio di un comando o skill non trovata */
#define NOFOUND_MSG		"Huh?\r\n"



/* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == *\
					ENUMERAZIONI E STRUTTURE GENERICHE CHE SERVONO AGLI INCLUDE DEL MUD
\* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == */


/*
 * Servono per la gestione delle risposte ai vari comandi con richieste che abbisognano di permesso
 */
typedef enum
{
	ANSWER_MAYBE,	/* La risposta ancora non si conosce */
	ANSWER_NO,		/* La risposta è no */
	ANSWER_YES,		/* La risposta è sì */
	MAX_ANSWER
} answer_types;

/*
 * Enumerazione degli allineamenti
 */
typedef enum
{
	ALIGN_NONE = -1,
	ALIGN_GOOD,
	ALIGN_NEUTRAL,
	ALIGN_EVIL,
	MAX_ALIGN
} alignment_types;


/*
 * Enumerazione degli attributi
 */
typedef enum
{
	/* Fisico */
	ATTR_STR,		/* Forza			*/
	ATTR_RES,		/* Resistenza		*/	/* MOVE	*/
	ATTR_CON,		/* Costituzione		*/	/* LIFE	*/
	/* Ragione */
	ATTR_INT,		/* Intelligenza		*/	/* da cambiare in ragione, aggiungere la memoria */
	ATTR_COC,		/* Concentrazione	*/
	ATTR_WIL,		/* Volontà			*/
	/* Destrezza */
	ATTR_AGI,		/* Agilità			*/
	ATTR_REF,		/* Riflessi			*/
	ATTR_SPE,		/* Velocità			*/
	/* Interiore */
	ATTR_SPI,		/* Spiritualità		*/	/* SOUL	*/
	ATTR_MIG,		/* Migrazione		*/
	ATTR_ABS,		/* Assorbimento		*/	/* MANA	*/
	/* Carisma */
	ATTR_EMP,		/* Empatia			*/
	ATTR_BEA,		/* Bellezza			*/
	ATTR_LEA,		/* Comando			*/
	MAX_ATTR
} attribute_types;


/*
 * Enumerazione dei punti.
 */
typedef enum
{
	POINTS_LIFE,		/* Vita		 */
	POINTS_MOVE,		/* Movimento */
	POINTS_MANA,		/* Mana		 */
	POINTS_SOUL,		/* Anima	 */
	MAX_POINTS
} points_types;


/*
 * Enumerazione dei sensi.
 */
typedef enum
{
	SENSE_SIGHT,	/* Vista		*/
	SENSE_HEARING,	/* Udito		*/
	SENSE_SMELL,	/* Olfatto		*/
	SENSE_TASTE,	/* Gusto		*/
	SENSE_TOUCH,	/* Tatto		*/
	SENSE_SIXTH,	/* Sesto senso	*/
	MAX_SENSE
} sense_types;


/*
 * Condizioni
 */
typedef enum
{
	CONDITION_DRUNK,
	CONDITION_FULL,
	CONDITION_THIRST,
	CONDITION_BLOODTHIRST,
	MAX_CONDITION
} condition_types;


/*
 * Tipo di sesso dei pg e dei mob.
 */
typedef enum
{
	SEX_NONE = -1,
	SEX_MALE,
	SEX_FEMALE,
	MAX_SEX
} sex_types;


/*
 * Enumerazione dei gradi di fiducia degli amministratori
 */
typedef enum
{
	TRUST_PLAYER,		/* Fiducia dei giocatori non amministratori */
	TRUST_NEOMASTER,	/* Comandi trust da amminsitratore di quest ridotti */
	TRUST_MASTER,		/* Comandi trust da Amministratore di Quest (Game Master) */
	TRUST_NEOBUILDER,	/* Comandi per la creazione di entità prototipo e di aree assegnate */
	TRUST_BUILDER,		/* Comandi trust da TRUST_MASTER e da Amministratore Aree senza restrizioni */
	TRUST_IMPLE,		/* Comandi trust da TRUST_BUILDER e da Amministratore del Mud (Implementor o Root) */
	MAX_TRUST
} trustes_types;


/*
 * Enumerazione dei tiri di salvezza.
 */
typedef enum
{
	SAVETHROW_NONE = -1,
	SAVETHROW_POISON_DEATH,
	SAVETHROW_ROD_WANDS,
	SAVETHROW_PARA_PETRI,
	SAVETHROW_BREATH,
	SAVETHROW_SPELL_STAFF,
	MAX_SAVETHROW
	/* 6 save types */
} save_types;


/*
 * Struttura che salva le info di digitazione e tempo di comandi e skill
 */
struct timerset
{
	unsigned int	num_uses;
	struct timeval	total_time;
	struct timeval	min_time;
	struct timeval	max_time;
};


/*
 * Struttura per una più potente gestione delle descrizioni dinamiche, la categoria è una
 *	tipologia definita, mentre il sinonimo è un aggettivo o sostantivo alternativo a quello
 *	relativo alla categori.
 * Per esempio se la struttura si riferisce ai capelli, come categoria potremmo avere HAIR_SLEEK,
 *	cioè lisci, e come sinonimo invece "dritto"; questo potenzia di molto la varietà nelle
 *	descrizioni dinamiche dei pg e di altro.
 * L'importante è che il sinonimo sia in sintonia con la categoria.
 */
struct synonym_data
{
	int		cat;		/* categoria a cui l'elemento appartiene */
	char   *syn;		/* stringa sinonimo descrivente l'elemento */
};



/* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == *\
												INCLUDE DEL MUD
\* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == */

#include "bitvector.h"  	/* Il supporto bitvector */
#include "code.h"			/* Contiene tutte le funzioni per la gestione dei "codici", utilizzate spesso */
#include "channel.h"		/* la send_log è dappertutto quindi si trova qui */
#include "class.h"			/* MAX_CLASS in tables.h */
#include "experience.h"		/* LVL_LEGEND in giro per i file *.h, come const.h */
#include "const.h"			/* MAX_SKILL in player.h */
#include "color.h"			/* MAX_AT_COLORS in mud.h */
#include "comm.h"			/* la send_to_char è dappertutto, quindi si trova qui */
#include "fight.h"			/* MAX_KILL_TRACK in mud.h */
#include "object_handler.h"	/* Contiene codice sulla gestione riguardo gli oggetti soprattutto, chiamato spesso */
#include "handler_ch.h"		/* Contiene codice riguardante la gestione dei mob e pg chiamate spesso */
#include "language.h"		/* MAX_LANGUAGE in mud.h */
#include "memory.h"			/* Utilizzata spesso nel mud */
#include "object_handler.h"	/* Serve per MAX_WEARLOC in race.h */
#include "player.h"			/* La struttura ch->pg c'è un po' dappertutto */
#include "race.h"			/* MAX_RACE in skills.h */

#ifdef ALL_HEADER
#	include "admin.h"
#	include "affect.h"
#	include "build.h"
#	include "calendar.h"
#	include "clan.h"
#	include "command.h"
#	include "db.h"
#	include "economy.h"
#	include "editor.h"
#	include "gramm.h"
#	include "group.h"
#	include "information.h"
#	include "interpret.h"
#	include "values.h"
#	include "make_obj.h"
#	include "mapper.h"
#	include "mccp.h"
#	include "movement.h"
#	include "msp.h"
#	include "mxp.h"
#	include "mprog.h"
#	include "nanny.h"
#	include "object_interact.h"
#	include "olc.h"
#	include "morph.h"
#	include "reset.h"
#	include "room.h"
#	include "save.h"
#	include "shop.h"
#endif

#include "partcorp.h"		/* Serve per MAX_PART in race.h */
#include "skills.h"
#include "social.h"
#include "magic.h"
#include "sysdata.h"
#include "timer.h"
#include "update.h"
#include "utility.h"
#include "values.h"
#include "weapon.h"
#include "weather.h"



/* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == *\
									SEZIONE RIGUARDANTE I FILE
\* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == */


/*
 * Struttura file per gestire meglio tutto il sistema di messaggi di log.
 */
struct mud_file
{
	FILE   *file;
	char   *path;
};


/*
 * Chiude un file e gli dona un puntatore NULL per sicurezza.
 */
#if !defined( FCLOSE )
#	define FCLOSE( fp )			\
	{							\
		if ( fp )				\
		{						\
			fclose( (fp) );		\
			(fp) = NULL;		\
		}						\
	}
#endif

/*
 * Chiude un file di tipologia log
 */
#if !defined( MUD_FCLOSE )
#	define MUD_FCLOSE( fp )														\
	{																			\
		if ( (fp) )																\
		{																		\
			DISPOSE( (fp)->path );												\
			FCLOSE( (fp)->file );												\
		}																		\
		else																	\
		{																		\
			send_log( NULL, LOG_BUG, "MUD_FCLOSE: puntatore fp nullo: %s %d",	\
				__FILE__, __LINE__ );											\
		}																		\
	}
#endif


/* File */
#define EXE_FILE		"../src/bard"		/* Path dell'eseguibile */

/* Dir di "primo livello" */
#define AREA_DIR		"../area/"			/* Dir delle aree di BARD */
#define PLAYER_DIR		"../player/"		/* Dir del salvataggio dei file PG */
#define SYSTEM_DIR		"../system/"		/* Dir principale con molti file i sistema */
#define TABLES_DIR		"../tables/"		/* Dir con i dati di molte tabelle del Mud */



/* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == *\
						  			VALORI DI INTERESSE PER LE AREE
\* == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == */


/*
 * Area flags
 */
typedef enum
{
	AREAFLAG_NOPKILL,		/* Nell'area non si può effettuare dell pkilling */
	AREAFLAG_FREEKILL,		/* Nell'area si può effettuare del pkilling senza restrizioni */
	AREAFLAG_NOTELEPORT,	/* Nell'area non vi ci si può teleportare */
	AREAFLAG_SPELL_LIMIT,
	AREAFLAG_NOSAVE,		/* L'area è un tutorial, i progressi non verranno salvati, ATTENZIONE: è una flag da utilizare solo per aree isolate dal resto del mud */
	AREAFLAG_HELL,			/* L'area è l'aldilà, per i pg morti o condannati con il comando hell */
	AREAFLAG_ARENA,			/* L'area è una arena */
	AREAFLAG_NOPOINTLOSS,	/* Non vengono persi punti di vita, mana e movimento per le principali cose */
	AREAFLAG_CITY,			/* Indica che l'area è una città */
	AREAFLAG_WEIGHTHEIGHT,	/* Area convertia con peso in grammi e altezza in centimetri */
	AREAFLAG_TAGGED,		/* Le descrizioni delle stanze verranno convertite con la change_command_tag */
	AREAFLAG_SECRET,		/* L'area non viene visualizzata nella lista del comando areas */
	AREAFLAG_WILD,			/* L'area è una Wild */
	AREAFLAG_LOADED,		/* L'area in costruzione è stata caricata */
	AREAFLAG_DELETED,		/* L'area in costruzione è stata chiusa */
	MAX_AREAFLAG
} area_flags;


/*
 * Well known mob virtual numbers.
 * Definiti in #MOBILES.
 * (FF) Non penso che questi qui ci saranno in futuro su T2 sarebbero da togliere almeno alcuni.
 */
#define VNUM_MOB_SUPERMOB			3
#define VNUM_MOB_ANIMATED_CORPSE	5
#define VNUM_MOB_POLY_WOLF			10
#define VNUM_MOB_POLY_MIST			11
#define VNUM_MOB_POLY_BAT			12
#define VNUM_MOB_POLY_HAWK			13
#define VNUM_MOB_POLY_CAT			14
#define VNUM_MOB_POLY_DOVE			15
#define VNUM_MOB_POLY_FISH			16
#define VNUM_MOB_VAMPIRE			80
#define VNUM_MOB_CITYGUARD			3060
#define VNUM_MOB_IDENTIFY			21001	/* Mob che identifica */


/*
 * ACT bits for mobs
 * Used in #MOBILES
 */
typedef enum
{
	MOB_IS_MOB,				/* Auto set for mobs				(CC) da togliere */
	MOB_SENTINEL,			/* Stays in one room				(CC) stay_room e poi aggiungere anche STAY_PLANE per il sistema dei piani, e anche stay_sector */
	MOB_SCAVENGER,			/* Picks up objects					*/
	MOB_SING,				/* Il mob canta delle liriche		(CC) occhio che questa prima era la MOB_R3 bisogna aggiungere dei check per la conversione */
	MOB_MORGUE,				/* Il mob recupera cadaveri ai pg dietro compenso */	/* (CC) occhio che questo prima era MOB_R4 */
	MOB_AGGRESSIVE,			/* Attacks PC's						*/
	MOB_STAY_AREA,			/* Won't leave area					*/
	MOB_WIMPY,				/* Flees when hurt (CC) fare una variabile come il pg	*/
	MOB_MASCOTTE,			/* Auto set for mascottes			*/
	MOB_TRAIN,				/* Can train PC's (CC) togliere		*/
	MOB_PRACTICE,			/* Can practice PC's				*/
	MOB_CARRY,				/* Può trasportare molto (GG) ex-ACT_IMMORTAL, da controllare	*/
	MOB_DEADLY,				/* Has a deadly poison				*/
	MOB_POLYSELF,
	MOB_META_AGGR,			/* Attacks other mobs				*/
	MOB_GUARDIAN,			/* Protects master					*/
	MOB_RUNNING,			/* Hunts quickly					*/
	MOB_NOWANDER,			/* Doesn't wander (GG) pare che serve per farlo star fermo almeno fino a che non venga attaccato	*/
	MOB_MOUNTABLE,			/* Can be mounted					*/
	MOB_MOUNTED,			/* Is mounted						*/
	MOB_SCHOLAR,			/* Can teach languages				*/
	MOB_SECRETIVE,			/* actions aren't seen				*/
	MOB_HARDHAT,			/* Immune to falling item damage	*/
	MOB_INCOGNITO,			/* Like wizinvis					*/
	MOB_NOASSIST,			/* Doesn't assist mobs				*/
	MOB_AUTONOMOUS,			/* Doesn't auto switch tanks		*/
	MOB_PACIFIST,			/* Doesn't ever fight				*/
	MOB_NOATTACK,			/* No physical attacks				*/
	MOB_ANNOYING,			/* Other mobs will attack			*/
	MOB_DAY,				/* Ex-ACT di statshield (CC) Only out during the day */
	MOB_NIGHT,				/* Ex-ACT di prototype (CC) Only out during the night */
	MOB_BANKER,				/* Il mob è un bancario				*/
	MOB_LOCKER,				/* Il mob gestisce gli armadietti di sicurezza */
	MAX_MOB
} mob_flags;


typedef enum
{
	TRAPTYPE_POISON_GAS = 1,
	TRAPTYPE_POISON_DART,
	TRAPTYPE_POISON_NEEDLE,
	TRAPTYPE_POISON_DAGGER,
	TRAPTYPE_POISON_ARROW,
	TRAPTYPE_BLINDNESS_GAS,
	TRAPTYPE_SLEEPING_GAS,
	TRAPTYPE_FLAME,
	TRAPTYPE_EXPLOSION,
	TRAPTYPE_ACID_SPRAY,
	TRAPTYPE_ELECTRIC_SHOCK,
	TRAPTYPE_BLADE,
	TRAPTYPE_SEX_CHANGE,
	/* 13 tipi di trappole */
	MAX_TRAPTYPE
} traps_types;


/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
/* (RR) in futuro molti di questi oggetti verranno tolti o moificati per T2 */
#define VNUM_OBJ_MONEY_ONE			2
#define VNUM_OBJ_MONEY_SOME			3

#define VNUM_OBJ_CORPSE_NPC			10
#define VNUM_OBJ_CORPSE_PC			11
#define VNUM_OBJ_SEVERED_HEAD		12
#define VNUM_OBJ_TORN_HEART			13
#define VNUM_OBJ_SLICED_ARM			14
#define VNUM_OBJ_SLICED_LEG			15
#define VNUM_OBJ_SPILLED_GUTS		16
#define VNUM_OBJ_BLOOD				17
#define VNUM_OBJ_BLOODSTAIN			18
#define VNUM_OBJ_SCRAPS				19

#define VNUM_OBJ_MUSHROOM			20
#define VNUM_OBJ_LIGHT_BALL			21
#define VNUM_OBJ_SPRING				22

#define VNUM_OBJ_SKIN				23
#define VNUM_OBJ_SLICE				24
#define VNUM_OBJ_SHOPPING_BAG		25

#define VNUM_OBJ_BLOODLET			26

#define VNUM_OBJ_FIRE				30
#define VNUM_OBJ_TRAP				31
#define VNUM_OBJ_PORTAL				32

#define VNUM_OBJ_BLACK_POWDER		33
#define VNUM_OBJ_SCROLL_SCRIBING	34
#define VNUM_OBJ_FLASK_BREWING		35
#define VNUM_OBJ_NOTE				36


/* Academy eq */
#define VNUM_OBJ_SCHOOL_MACE		10315
#define VNUM_OBJ_SCHOOL_DAGGER		10312
#define VNUM_OBJ_SCHOOL_SWORD		10313
#define VNUM_OBJ_SCHOOL_VEST		10308
#define VNUM_OBJ_SCHOOL_SHIELD		10310
#define VNUM_OBJ_SCHOOL_BANNER		10311
#define VNUM_OBJ_SCHOOL_GUIDE		10333


/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
/* (RR) in futuro alcune di queste room salteranno e non serviranno per T2, controllare per esempio la 6 e la 1026 in do_unhell */
#define VNUM_ROOM_LIMBO			    2
#define VNUM_ROOM_POLY			    3
#define VNUM_ROOM_HELL				8
#define VNUM_ROOM_TUTORIAL		  700
#define VNUM_ROOM_CHAT			 1200
#define VNUM_ROOM_TEMPLE		21001
#define VNUM_ROOM_ALTAR			21194
#define VNUM_ROOM_SCHOOL		10300
#define VNUM_ROOM_HALLOFFALLEN	21195
#define VNUM_ROOM_ASHITAKA		32772	/* Un prato di lavanda */

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *
 *							VALORI DI INTERESSE PER LE AREE						 *
 *								(Fine della sezione)							 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */



/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
	rNONE,
	rCHAR_DIED,
	rVICT_DIED,
	rBOTH_DIED,
	rCHAR_QUIT,
	rVICT_QUIT,
	rBOTH_QUIT,
	rSPELL_FAILED,
	rOBJ_SCRAPPED,
	rOBJ_EATEN,
	rOBJ_EXPIRED,
	rOBJ_TIMER,
	rOBJ_SACCED,
	rOBJ_QUAFFED,
	rOBJ_USED,
	rOBJ_EXTRACTED,
	rOBJ_DRUNK,
	rCHAR_IMMUNE,
	rVICT_IMMUNE,
	rCHAR_AND_OBJ_EXTRACTED = 128,
	rERROR = 255,
	rSTOP				/* Serve per il run nella move_char() */
	/* 21 tipi di return */
} ret_types;


/*
 * Posizioni
 */
typedef enum
{
	POSITION_DEAD,
	POSITION_MORTAL,
	POSITION_INCAP,
	POSITION_STUN,
	POSITION_SLEEP,
	POSITION_BERSERK,
	POSITION_REST,
#ifdef T2_ALFA
	POSITION_KNEE,		/* inginocchiato (GG) questa ha valenza per i chierici, se si trovano in questa posizione i loro cast funzionano meglio */
#endif
	POSITION_AGGRESSIVE,
	POSITION_SIT,
#if T2_ALFA
	POSITION_CROUCHED	/* accucciato (GG) nulla di speciale credo, l'ho creata così tanto per */
#endif
	POSITION_FIGHT,
	POSITION_DEFENSIVE,
	POSITION_EVASIVE,
	POSITION_STAND,
	POSITION_MOUNT,
	POSITION_SHOVE,
	POSITION_DRAG,
	MAX_POSITION
} positions;


/*
 * Stili
 */
typedef enum
{
	STYLE_BERSERK,
	STYLE_AGGRESSIVE,
	STYLE_FIGHTING,
	STYLE_DEFENSIVE,
	STYLE_EVASIVE,
	MAX_STYLE
} styles;


/*
 * Struttura di una Descrizione Extra per stanze e oggetti (FF) e mob
 */
struct extra_descr_data
{
	EXTRA_DESCR_DATA *next;			/* Precedente nella lista		*/
	EXTRA_DESCR_DATA *prev;			/* Sucessiva nella lista		*/
	char			 *keyword;		/* Parole chiavi per guardarla	*/
	char			 *description;	/* Quello che si legge			*/
};


/*
 * Prototype for a mob
 * This is the in-memory version of #MOBILES
 */
struct mob_proto_data
{
	MOB_PROTO_DATA *next;
	MOB_PROTO_DATA *prev;
	SPEC_FUN	   *spec_fun;
	SHOP_DATA	   *pShop;
	REPAIR_DATA	   *rShop;
	MPROG_DATA	   *first_mudprog;
	MPROG_DATA	   *last_mudprog;
	AREA_DATA	   *area;
	char 		   *name;
	char 		   *short_descr;
	char 		   *long_descr;
	char		   *description;
	char		   *spec_funname;
	VNUM			vnum;
	int				count;
	int				killed;
	int				sex;
	int				level;
	BITVECTOR	   *act;
	BITVECTOR	   *affected_by;
	int				alignment;
	int				ac;
	int				hitnodice;
	int				hitsizedice;
	int				hitplus;
	int				damnodice;
	int				damsizedice;
	int				damplus;
	int				numattacks;
	int				gold;
	int				exp;
	BITVECTOR	   *xflags;
	BITVECTOR	   *immune;
	BITVECTOR	   *resistant;
	BITVECTOR	   *susceptible;
	BITVECTOR	   *attacks;
	BITVECTOR	   *defenses;
	BITVECTOR	   *speaks;				/* Indica quali lingue il mob conosce	*/
	int				speaking;			/* Indica con quale lingua il mob parla, se -1 allora parla in una lingua a scelta tra le sue native */
	int				position;
	int				defposition;
	int				height;
	int				weight;
	int				race;
	int				class;
	int				hitroll;
	int				damroll;
	int				attr_perm[MAX_ATTR];
	int				sense_perm[MAX_SENSE];	/* (GG) */
	int				saving_throw[MAX_SAVETHROW];
};


struct char_data_extracted
{
	CHAR_DATA_EXTRACT *next;
	CHAR_DATA		  *ch;
	ROOM_DATA		  *room;
	ch_ret			   retcode;
	bool			   extract;
};


/*
 * One character (PC or MOB)
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 *	spare_ptr and tempnum?  Seems a little redundant)
 */
struct char_data
{
	CHAR_DATA		 *next;
	CHAR_DATA		 *prev;
	CHAR_DATA		 *next_player;
	CHAR_DATA		 *prev_player;
	CHAR_DATA		 *next_in_room;
	CHAR_DATA		 *prev_in_room;
#ifdef T2_ALFA
	EXTRA_DESCR_DATA *first_extradescr;			/* (GG) */
	EXTRA_DESCR_DATA *last_extradescr;			/* (GG) sistema di descr e sotto descr */
#endif
	CHAR_DATA		 *master;
	CHAR_DATA		 *leader;
	FIGHT_DATA		 *fighting;
	CHAR_DATA		 *reply;
	CHAR_DATA		 *switched;
	CHAR_DATA		 *mount;
	HHF_DATA		 *hunting;
	HHF_DATA		 *fearing;
	HHF_DATA		 *hating;
	SPEC_FUN		 *spec_fun;
	LYRIC_DATA		 *lyric;					/* Lirica che il mob sta cantando */	/* (CC) All'atto del caricamento il mob avrà un campo con tanti nomi di lirica, questi possono essere uno o più di uno, se nessuno il mob canterà tutte le canzoni non segrete che la sua lingua e la razza glielo permettono */
	PENTAGRAM_DATA	 *pentagram;				/* Riga di pentagramma in cui il mob è arrivato a cantare */
	int				  rhythm;					/* Numero di pulse mancanti per l'invio del prossimo pentagramma */
	char			 *spec_funname;
	MPROG_ACT_LIST	 *mpact;
	int				  mpactnum;
	MOB_PROTO_DATA	 *pIndexData;
	DESCRIPTOR_DATA	 *desc;
	AFFECT_DATA		 *first_affect;
	AFFECT_DATA		 *last_affect;
	NOTE_DATA		 *pnote;
	OBJ_DATA		 *first_carrying;
	OBJ_DATA		 *last_carrying;
	ROOM_DATA		 *in_room;
	ROOM_DATA		 *was_in_room;
	PG_DATA			 *pg;
	DO_FUN			 *last_cmd;
	DO_FUN			 *prev_cmd;					/* mapping									*/
	void			 *dest_buf;					/* This one is to assign to differen things */
	char			 *alloc_ptr;				/* Must str_dup and free this one 			*/
	void			 *spare_ptr;
	int				  tempnum;
	EDITOR_DATA		 *editor;
	TIMER			 *first_timer;
	TIMER			 *last_timer;
	int				  timer;
	CHAR_MORPH		 *morph;
	char			 *name;
	char			 *short_descr;
	char			 *long_descr;
	char			 *description;
	int				  num_fighting;
	int				  substate;
	int				  sex;
	int				  class;
	int				  race;
	int				  level;
	int				  wait;
	bool			  right_hand;				/* mano principale del pg, TRUE se destra FALSE per sinistra */
	int				  attr_mod [MAX_ATTR];
	int				  attr_perm[MAX_ATTR];
	int				  sense_mod [MAX_SENSE];	/* (RR) probabilmente per questi devo creare una funzione tipo la get_curr_attr */
	int				  sense_perm[MAX_SENSE];
	int				  points[MAX_POINTS];
	int				  points_max[MAX_POINTS];
	int				  numattacks;
	int				  gold;
	int				  exp;
	BITVECTOR		 *act;
	BITVECTOR		 *affected_by;
	BITVECTOR		 *no_affected_by;
#ifdef T2_ALFA
	BITVECTOR		 *enflags;					/* Flag di entità, ovvero quelle che possono essere settate sia per mob che per pg */
#endif
	int				  carry_weight;
	int				  carry_number;
	BITVECTOR		 *xflags;
	BITVECTOR		 *no_immune;
	BITVECTOR		 *no_resistant;
	BITVECTOR		 *no_susceptible;
	BITVECTOR		 *immune;
	BITVECTOR		 *resistant;
	BITVECTOR		 *susceptible;
	BITVECTOR		 *attacks;
	BITVECTOR		 *defenses;
	BITVECTOR		 *speaks;					/* Indica quali lingue il mob conosce	*/
	int				  speaking;					/* Indica con quale lingua il pg parla	*/
	int				  saving_throw[MAX_SAVETHROW];
	int				  alignment;
	int				  barenumdie;
	int				  baresizedie;
	int				  hitroll;
	int				  damroll;
	int				  hitplus;
	int				  damplus;
	int				  position;
	int				  defposition;
	int				  style;
	int				  height;					/* (GG) Altezza	in cm			*/
	int				  weight;					/* (GG) Peso		in grammi	*/
	int				  armor;
	int				  wimpy;
	int				  mental_state;				/* simplified		*/
	int				  emotional_state;			/* simplified		*/
	VNUM			  retran;					/* VNUM dell'ultima stanza prima di essere trasferiti ad un'altra (FF) cercare di spostarla della struttura pg_data */
	VNUM			  regoto;					/* VNUM dell'ultima stanza dove ci si è trasferiti con il goto	  (FF) cercare di spostarla della struttura pg_data */
	int				  mobinvis;					/* Livello di Mobinvis (TT) da controllare che non sia mischiato con la trust */
	int				  cmd_recurse;				/* Conta la ricorsione di un comando alias */
};


/* Massimo dei messaggi di reset caricabili */
#define MAX_RESETMSG	5

/*
 * Struttura di un'Area
 */
struct area_data
{
	AREA_DATA		*next;				/* Area precedente nella lista			*/
	AREA_DATA		*prev;				/* Area sucessiva nella lista			*/
	AREA_DATA		*next_sort;			/*  */
	AREA_DATA		*prev_sort;			/*  */
	RESET_DATA		*first_reset;		/* Primo reset dell'area				*/
	RESET_DATA		*last_reset;		/* Ultimo reset dell'area				*/
	RESET_DATA		*last_mob_reset;	/*  */
	RESET_DATA		*last_obj_reset;	/*  */
#ifdef T2_ALFA
	NATION_DATA		*nation;			/* Nazione che domina o che ha conquistato l'area */
#endif
	WEATHER_DATA	*weather;			/* Informazioni meteo dell'area			*/
	SYNO_DATA		*color;				/* Colore dell'area, applicato ai capilettera delle descrizioni */
	char			*name;				/* Nome dell'area						*/
	char			*filename;			/* Nome file con cui l'area viene caricata e salvata */
	char			*author;			/* Nome del/i autore/i dell'area		*/
	VNUM			 vnum_low;			/* Vnum minimo dell'area				*/
	VNUM			 vnum_high;			/* Vnum massimo dell'area				*/
	int				 age;				/* Età dell'area, per il reset della stessa (FF) le funzioni dei reset son da spostare da update.c a reset.c secondo me */
	int				 reset_frequency;	/* Frequenza di reset					*/
	char			*resetmsg[MAX_RESETMSG];	/* Messaggi di reset (FF) da pensare se farci una lista */
	int				 version;			/* Numero di versione dell'area			*/
	BITVECTOR		*flags;				/* Flag dell'area, vedere i codici AREAFLAG_ */
	int				 economy_high;		/*  */
	int				 economy_low;		/*  */
	int				 gold_looted;		/* Soldi saccheggiati nell'area			*/
	int				 level_low;			/* Livello minimo indicativo per l'area	*/
	int				 level_high;		/* Livello massimo indicativo per l'area */
	int				 nplayer;			/* Numero dei giocatori nell'area (FF) può servire per un futuro comando di popularity che elenca le aree e la loro "popolazione" */
	int				 max_players;		/* Numero massimo di giocatori presenti nell'area */
	int				 mdeaths;			/* Numero di volte in cui i mob son stati uccisi */
	int				 mkills;			/* Numero di volte in cui i mob hanno ucciso */
	int				 pdeaths;			/* Numero di volte in cui i pg sono morti */
	int				 pkills;			/* Numero di volte in cui i pg hanno ucciso altri pg */
	int				 illegal_pk;		/* Numero di pk illegali nell'area		*/
	int				 spell_limit;		/* Limite degli spell castati nell'area (TT) */
	int				 curr_spell_count;	/* Corrente numero di incantesimi castati nell'area */
};


/*
 * Tipi di obiettivi Target
 */
typedef enum
{
	TARGET_IGNORE,
	TARGET_CHAR_OFFENSIVE,
	TARGET_CHAR_DEFENSIVE,
	TARGET_CHAR_SELF,
	TARGET_OBJ_INV,
	MAX_TARGET
} targets_types;


/*
 * Macro di Utilità
 */
/* Controlla la validità di una stringa */
#define VALID_STR( str )	( (str) && (str)[0] != '\0' )

#define UMIN( a, b )		( (a) < (b)  ?  (a)  :  (b) )
#define UMAX( a, b )		( (a) > (b)  ?  (a)  :  (b) )

#ifdef T2_ALFA
/* Inutilizzate per ora, usarle al caso */
#	define UMIN3( a, b, c )	( (a) < (b)  ?  (a) < (c) ? (a) : (c)  :  (b) < (c) ? (b) : (c) )
#	define UMAX3( a, b, c )	( (a) > (b)  ?  (a) > (c) ? (a) : (c)  :  (b) > (c) ? (b) : (c) )
#endif

#define URANGE( a, b, c )	( (b) < (a)  ?  (a)  :  ((b) > (c) ? (c) : (b)) )

#define ASSIGN_GSN( gsn, skill )														\
	do																					\
	{																					\
		if ( ((gsn) = skill_lookup((skill))) == -1 )									\
			send_log( NULL, LOG_BUG, "ASSIGN_GSN: Skill %s non trovata.", (skill) );	\
	} while (0)

#define CHECK_SUBRESTRICTED(ch)																		\
	do																								\
	{																								\
		if ( (ch)->substate == SUB_RESTRICTED )														\
		{																							\
			ch_printf( ch, "Non puoi utilizzare questo comando assieme ad un altro comando.\r\n" );	\
			return;																					\
		}																							\
	} while (0)


/*
 * Macro per la gestione delle doppie liste
 */

#define LINK( link, first, last, next, prev )										\
	do																				\
	{																				\
		if ( !(link) )																\
			send_log( NULL, LOG_BUG, "link è NULL @ %s:%d\n", __FILE__, __LINE__ );	\
		if ( !(first) )																\
		{																			\
			(first)	= (link);														\
			(last)  = (link);														\
		}																			\
		else																		\
			(last)->next	= (link);												\
		(link)->next = NULL;														\
		if ( (first) == (link) )													\
			(link)->prev = NULL;													\
		else																		\
			(link)->prev = (last);													\
		(last) = (link);															\
	} while (0)

#define UNLINK( link, first, last, next, prev )		\
	do												\
	{												\
		if ( !(link)->prev )						\
		{											\
			(first) = (link)->next;					\
			if ( (first) )							\
				(first)->prev = NULL;				\
		}											\
		else										\
			(link)->prev->next = (link)->next;		\
		if ( !(link)->next )						\
		{											\
			(last) = (link)->prev;					\
			if ( (last) )							\
				(last)->next = NULL;				\
		}											\
		else										\
			(link)->next->prev = (link)->prev;		\
	} while (0)

/*  Questa MACRO inserisce la struttura prima di quella specificata in insert */
#define INSERT( link, insert, first, next, prev )									\
	do																				\
	{																				\
		if ( !(link) )																\
			send_log( NULL, LOG_BUG, "link è NULL @ %s:%d\n", __FILE__, __LINE__ );	\
		(link)->prev = (insert)->prev;												\
		if ( !(insert)->prev )														\
			(first) = (link);														\
		else																		\
			(insert)->prev->next = (link);											\
		(insert)->prev = (link);													\
		(link)->next = (insert);													\
	} while (0)

/* Questa macro è difficile da usare st1 DEVE essere **PRIMA** di st2 */
#define EXCHANGE( st1, st2, next, prev, type )	\
	do											\
	{											\
		type *succ, *prec;						\
		succ = st1->next;						\
		if ( succ == st2 )						\
			succ = st1;							\
		prec = st1->prev;						\
		st1->next = st2->next;					\
		st1->prev = st2->prev;					\
		if ( st1->prev == st1 )					\
			st1->prev = st2;					\
		st2->prev = prec;						\
		st2->next = succ;						\
		if ( st1->next )						\
			st1->next->prev = st1;				\
		if ( st1->prev )						\
			st1->prev->next = st1;				\
		if ( st2->next )						\
			st2->next->prev = st2;				\
		if ( st2->prev )						\
			st2->prev->next = st2;				\
		succ = st1;								\
		st1 = st2;								\
		st2 = succ;								\
	} while (0)                                     

#define CHECK_LINKS( first, last, next, prev, type )												\
	do																								\
	{																								\
		type *ptr, *pptr = NULL;																	\
		if ( !(first) && !(last) )																	\
			break;																					\
		if ( !(first) )																				\
		{																							\
			send_log( NULL, LOG_BUG, "CHECK_LINKS: last with NULL first!  %s.",						\
				__STRING(first) );																	\
			for ( ptr = (last);  ptr->prev;  ptr = ptr->prev );										\
			(first) = ptr;																			\
		}																							\
		else if ( !(last) )																			\
		{																							\
			send_log( NULL, LOG_BUG, "CHECK_LINKS: first with NULL last!  %s.",						\
				__STRING(first) );																	\
			for ( ptr = (first);  ptr->next;  ptr = ptr->next );									\
			(last) = ptr;																			\
		}																							\
		if ( (first) )																				\
		{																							\
			for ( ptr = (first);  ptr;  ptr = ptr->next )											\
			{																						\
				if ( ptr->prev != pptr )															\
				{																					\
					send_log( NULL, LOG_BUG, "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",			\
						__STRING(first), ptr, pptr );												\
					ptr->prev = pptr;																\
				}																					\
				if ( ptr->prev && ptr->prev->next != ptr )											\
				{																					\
					send_log( NULL, LOG_BUG, "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",	\
						__STRING(first), ptr, ptr );												\
					ptr->prev->next = ptr;															\
				}																					\
				pptr = ptr;																			\
			}																						\
			pptr = NULL;																			\
		}																							\
		if ( (last) )																				\
		{																							\
			for ( ptr = (last);  ptr;  ptr = ptr->prev )											\
			{																						\
				if ( ptr->next != pptr )															\
				{																					\
					send_log( NULL, LOG_BUG, "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",			\
						__STRING(first), ptr, pptr );												\
					ptr->next = pptr;																\
				}																					\
				if ( ptr->next && ptr->next->prev != ptr )											\
				{																					\
					send_log( NULL, LOG_BUG, "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",	\
						__STRING(first), ptr, ptr );												\
					ptr->next->prev = ptr;															\
				}																					\
				pptr = ptr;																			\
			}																						\
		}																							\
	} while (0)


/*
 * Variabili globali
 */
extern	int					objects_loaded;
extern	int					mobs_loaded;
extern	int					objects_physical;

extern	CHAR_DATA		   *cur_char;
extern	ROOM_DATA		   *cur_room;
extern	bool			 	cur_char_died;
extern	ch_ret		 		global_retcode;

extern	int					cur_obj;
extern	int					cur_obj_serial;
extern	bool				cur_obj_extracted;
extern	obj_ret				global_objcode;

extern	OBJ_DATA		   *extracted_obj_queue;
extern	CHAR_DATA_EXTRACT  *extracted_char_queue;
extern	OBJ_DATA		   *save_equipment[MAX_WEARLOC][MAX_LAYERS];
extern	CHAR_DATA		   *quitting_char;
extern	CHAR_DATA		   *loading_char;
extern	CHAR_DATA		   *saving_char;
extern	OBJ_DATA		   *all_obj;

extern	time_t				current_time;
extern	bool				fLogAll;


#endif  /* MUD_H */
