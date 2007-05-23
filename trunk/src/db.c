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
 >                       Modulo gestione Database                           <
\****************************************************************************/

#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>		/* DLSym */
#include <stdarg.h>
#include <unistd.h>

#include "mud.h"
#include "adminhost.h"
#include "admin.h"
#include "affect.h"
#include "alias.h"
#include "animation.h"
#include "archetype.h"
#include "ban.h"
#include "bank.h"
#include "build.h"
#include "calendar.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "dream.h"
#include "economy.h"
#include "editor.h"
#include "fread.h"
#include "game_chess.h"
#include "help.h"
#include "herb.h"
#include "hint.h"
#include "interpret.h"
#include "liquid.h"
#include "locker.h"
#include "lyric.h"
#include "movement.h"
#include "mprog.h"
#include "nanny.h"
#include "news.h"
#include "note.h"
#include "morph.h"
#include "question.h"
#include "quote.h"
#include "reboot.h"
#include "reserved.h"
#include "reset.h"
#include "room.h"
#include "save.h"
#include "shop.h"
#include "special.h"
#include "timer.h"
#include "watch.h"
#include "web.h"


/*
 * Variabili esterne
 */
OBJ_DATA		   *extracted_obj_queue;
CHAR_DATA_EXTRACT  *extracted_char_queue;

CHAR_DATA		   *first_char = NULL;
CHAR_DATA		   *last_char = NULL;
CHAR_DATA		   *first_player = NULL;
CHAR_DATA		   *last_player = NULL;
CHAR_DATA		   *first_offline = NULL;
CHAR_DATA		   *last_offline = NULL;

MOB_PROTO_DATA	   *first_mob_proto = NULL;
MOB_PROTO_DATA	   *last_mob_proto = NULL;

OBJ_DATA		   *first_object_proto = NULL;
OBJ_DATA		   *last_obj_proto = NULL;
OBJ_DATA		   *first_object = NULL;
OBJ_DATA		   *last_object = NULL;

int					top_offline = 0;	/* Numero di file save caricati */
int					cur_qobjs;			/* Semi esterna */
int					cur_qchars;			/* Semi esterna */
int					mobs_loaded;
int					objects_loaded;
int					objects_physical;
VNUM				last_pkroom;

bool				fBootDb;



/* weaponry */
int		gsn_pugilism;
int		gsn_long_blades;
int		gsn_short_blades;
int		gsn_flexible_arms;
int		gsn_talonous_arms;
int		gsn_bludgeons;
int		gsn_missile_weapons;

/* thief */
int		gsn_detrap;
int		gsn_backstab;
int		gsn_circle;
int		gsn_dodge;
int		gsn_hide;
int		gsn_peek;
int		gsn_pick_lock;
int		gsn_sneak;
int		gsn_steal;
int		gsn_gouge;
int		gsn_poison_weapon;

/* thief & warrior */
int		gsn_disarm;
int		gsn_enhanced_damage;
int		gsn_kick;
int		gsn_parry;
int		gsn_rescue;
int		gsn_second_attack;
int		gsn_third_attack;
int		gsn_fourth_attack;
int		gsn_fifth_attack;
int		gsn_dual_wield;
int		gsn_punch;
int		gsn_bash;
int		gsn_stun;
int		gsn_bashdoor;
int		gsn_grip;
int		gsn_berserk;
int		gsn_hitall;
int		gsn_tumble;

/* Per vampiri */
int		gsn_feed;
int		gsn_bloodlet;
int		gsn_broach;
int		gsn_mistwalk;	/* GG */

/* Altri   */
int		gsn_aid;
int		gsn_track;
int		gsn_search;
int		gsn_dig;
int		gsn_mount;
int		gsn_bite;
int		gsn_claw;
int		gsn_sting;
int		gsn_tail;
int		gsn_scribe;
int		gsn_brew;
int		gsn_climb;
int		gsn_cook;
int		gsn_scan;
int		gsn_slice;

/* spells */
int		gsn_aqua_breath;
int		gsn_blindness;
int		gsn_charm_person;
int		gsn_curse;
int		gsn_invis;
int		gsn_mass_invis;
int		gsn_poison;
int		gsn_sleep;
int		gsn_possess;
int		gsn_fireball;
int		gsn_chill_touch;
int		gsn_lightning_bolt;


/* for searching */
int		gsn_first_spell;
int		gsn_first_skill;
int		gsn_first_weapon;
int		gsn_top_sn;

/* For styles?  Trying to rebuild from some kind of accident here. */
int		gsn_style_evasive;
int		gsn_style_defensive;
int		gsn_style_normal;
int		gsn_style_aggressive;
int		gsn_style_berserk;


/*
 * Variabili
 */
AREA_DATA  *first_area	= NULL;
AREA_DATA  *last_area	= NULL;
AREA_DATA  *first_build	= NULL;
AREA_DATA  *last_build	= NULL;

int			top_area;
int			top_ed;
int			top_exit;
int			top_mob_proto;
int			top_obj_proto;
VNUM		top_room;


/*
 * Sort areas by name alphanumercially
 */
void sort_area_by_name( AREA_DATA *pArea, bool proto )
{
	AREA_DATA *temp_area;

	if ( !pArea )
	{
		send_log( NULL, LOG_BUG, "sort_area_by_name: NULL pArea" );
		return;
	}

	for ( temp_area = (proto) ? first_build : first_area;  temp_area;  temp_area = temp_area->next )
	{
		if ( strcmp(pArea->name, temp_area->name) < 0 )
		{
			if ( proto )
				INSERT( pArea, temp_area, first_build, next, prev );
			else
				INSERT( pArea, temp_area, first_area, next, prev );
			break;
		}
	}

	if ( !temp_area )
	{
		if ( proto )
			LINK( pArea, first_build, last_build, next, prev );
		else
			LINK( pArea, first_area, last_area, next, prev );
	}
}


/*
 * Carica e inizializza una nuova sezione header di area.
 * Se proto è TRUE significa che è un'area da inserire nella lista first_build-last_build
 * (FF) in realtà mi pare che le building list non utilizzino init_area, sarebbe il caso di accorpare un po'
 */
AREA_DATA *init_area( MUD_FILE *fp )
{
	AREA_DATA	*pArea;
	char		*filename;
	int			 x;

	CREATE( pArea, AREA_DATA, 1 );

	pArea->name						= str_dup( "" );
	/* Ricava il nome del file da fp->path */
	filename = strlstr( fp->path, "/" );
	pArea->filename					= str_dup( filename );
	pArea->author					= str_dup( "" );
	pArea->version					= 0;
	pArea->age						= 15;
	for ( x= 0;  x < MAX_RESETMSG;  x++ )
		pArea->resetmsg[x]			= str_dup( "" );

	CREATE( pArea->color, SYNO_DATA, 1 );
	pArea->color->cat				= AT_BLACK;
	pArea->color->syn				= str_dup( code_name(NULL, AT_BLACK, CODE_COLOR) );

	pArea->economy_high				= 0;
	pArea->economy_low				= 0;
	pArea->nplayer					= 0;
	pArea->level_low				= 0;
	pArea->level_high				= 0;
#ifdef T2_ALFA
	pArea->nation					= NATION_NONE;	/* (GG) */
#endif
	pArea->vnum_low					= 0;
	pArea->vnum_high				= 0;
	pArea->spell_limit				= 0;
	pArea->first_reset				= NULL;
	pArea->last_reset				= NULL;

	/* Initializza i dati per il tempo metereologico */
	CREATE( pArea->weather, WEATHER_DATA, 1 );
	pArea->weather->temp			= 0;
	pArea->weather->precip			= 0;
	pArea->weather->wind			= 0;
	pArea->weather->temp_vector		= 0;
	pArea->weather->precip_vector	= 0;
	pArea->weather->wind_vector		= 0;
	pArea->weather->climate_temp	= 2;
	pArea->weather->climate_precip	= 2;
	pArea->weather->climate_wind	= 2;
	pArea->weather->first_neighbor	= NULL;
	pArea->weather->last_neighbor	= NULL;

	top_area++;

	return pArea;
}


/*
 * Funzione di conversione dal range degli attributi dello SMAUG a quello del BARD
 */
int convert_d20_to_d100( const int stat )
{
	if		( stat <=  0 )	return 1;
	else if ( stat <=  1 )	return number_range(   1,   3 );	/* fascia 1° (step di 3)*/
	else if ( stat <=  2 )	return number_range(   4,   6 );
	else if ( stat <=  3 )	return number_range(   7,   9 );
	else if ( stat <=  4 )	return number_range(  10,  12 );
	else if ( stat <=  5 )	return number_range(  13,  15 );
	else if ( stat <=  6 )	return number_range(  16,  18 );
	else if ( stat <=  7 )	return number_range(  19,  23 );	/* fascia 2° (step di 5)*/
	else if ( stat <=  8 )	return number_range(  24,  28 );
	else if ( stat <=  9 )	return number_range(  29,  33 );
	else if ( stat <= 10 )	return number_range(  34,  38 );
	else if ( stat <= 11 )	return number_range(  39,  43 );
	else if ( stat <= 12 )	return number_range(  44,  50 );	/* fascia 3° (step di 7)*/
	else if ( stat <= 13 )	return number_range(  51,  57 );
	else if ( stat <= 14 )	return number_range(  58,  62 );	/* fascia 4° (step di 5)*/
	else if ( stat <= 15 )	return number_range(  63,  68 );
	else if ( stat <= 16 )	return number_range(  69,  73 );
	else if ( stat <= 17 )	return number_range(  74,  78 );
	else if ( stat <= 18 )	return number_range(  79,  83 );
	else if ( stat <= 19 )	return number_range(  84,  87 );	/* fascia 5° (step di 3)*/
	else if ( stat <= 21 )	return number_range(  88,  90 );
	else if ( stat <= 23 )	return number_range(  91,  93 );
	else if ( stat <= 24 )	return number_range(  94,  96 );
	else					return number_range(  97, 100 );
}


/*
 * Clean out a mobile (index) (leave list pointers intact )
 * (FF) futura clean_mobile(), come clean_obj
 */
void clean_mobile_index( MOB_PROTO_DATA *mob )
{
	DISPOSE( mob->name );
	DISPOSE( mob->short_descr );
	DISPOSE( mob->long_descr  );
	DISPOSE( mob->description );

	mob->spec_fun	= NULL;
	mob->pShop		= NULL;
	mob->rShop		= NULL;

	free_mudprog( mob->first_mudprog );

	mob->count			 = 0;
	mob->killed			 = 0;
	mob->sex			 = 0;
	mob->level			 = 0;
	CLEAR_BITS( mob->act );
	CLEAR_BITS( mob->affected_by );
	mob->alignment		 = 0;
	mob->ac				 = 0;
	mob->hitnodice		 = 0;
	mob->hitsizedice	 = 0;
	mob->hitplus		 = 0;
	mob->damnodice		 = 0;
	mob->damsizedice	 = 0;
	mob->damplus		 = 0;
	mob->gold			 = 0;
	mob->exp			 = 0;
	mob->position		 = 0;
	mob->defposition	 = 0;
	mob->height			 = 0;
	mob->weight			 = 0;
/*	mob->vnum			 = 0; */
	CLEAR_BITS( mob->attacks );
	CLEAR_BITS( mob->defenses );
}


/*
 * Ripulisce un oggetto (indicizzato e non) senza
 *	rimuoverlo dalla lista o dalla tabella hash
 * Ovvero non azzera il vnum, nè il puntatore al prototipo
 */
void clean_object( OBJ_DATA *obj )
{
	AFFECT_DATA			*paf;
	AFFECT_DATA			*paf_next;
	EXTRA_DESCR_DATA	*ed;
	EXTRA_DESCR_DATA	*ed_next;
	MPROG_ACT_LIST		*mpact;
	int					 x;

	obj->area = NULL;

	DISPOSE( obj->name );
	DISPOSE( obj->short_descr );
	DISPOSE( obj->long_descr );
	DISPOSE( obj->action_descr );
	DISPOSE( obj->owner );

	for ( paf = obj->first_affect;  paf;  paf = paf_next )
	{
		paf_next = paf->next;
		DISPOSE( paf );
		top_affect--;
	}
	obj->first_affect	= NULL;
	obj->last_affect	= NULL;

	for ( ed = obj->first_extradescr;  ed;  ed = ed_next )
	{
		ed_next = ed->next;
		DISPOSE( ed->description );
		DISPOSE( ed->keyword );
		DISPOSE( ed );
		top_ed--;
	}
	obj->first_extradescr	= NULL;
	obj->last_extradescr	= NULL;

	obj->type		= 0;
	CLEAR_BITS( obj->extra_flags );
	CLEAR_BITS( obj->magic_flags );
	CLEAR_BITS( obj->wear_flags );
	obj->count		= 0;
	obj->weight		= 0;
	obj->cost		= 0;
	obj->wear_loc	= 0;
	obj->timer		= 0;
	CLEAR_BITS( obj->layers );

	/* Ripulisce le varie strutture per i tipi di oggetto */
	for ( x = 1;  x < MAX_OBJTYPE;  x++ )		/* x = 1 perché salti OBJTYPE_NONE */
		free_values( obj, x );

	free_mudprog( obj->first_mudprog );
	obj->first_mudprog	= NULL;
	obj->last_mudprog	= NULL;

	/* Non dovrebbe averne in memoria di act, ma non si sa mai */
	while ( (mpact = obj->mpact) )
	{
		obj->mpact = mpact->next;
		DISPOSE( mpact->buf );
		DISPOSE( mpact );
	}
	obj->mpactnum = 0;
}


/*
 * clean out a room (leave list pointers intact )
 */
void clean_room( ROOM_DATA *room )
{
	EXTRA_DESCR_DATA *ed;
	EXTRA_DESCR_DATA *ed_next;
	EXIT_DATA		 *pexit;
	EXIT_DATA		 *pexit_next;

	DISPOSE( room->description );
	DISPOSE( room->name );

	for ( ed = room->first_extradescr;  ed;  ed = ed_next )
	{
		ed_next = ed->next;
		DISPOSE( ed->description );
		DISPOSE( ed->keyword );
		DISPOSE( ed );
		top_ed--;
	}

	room->first_extradescr	= NULL;
	room->last_extradescr	= NULL;

	for ( pexit = room->first_exit;  pexit;  pexit = pexit_next )
	{
		pexit_next = pexit->next;
		DISPOSE( pexit->keyword );
		DISPOSE( pexit->description );
		DISPOSE( pexit );
		top_exit--;
	}
	room->first_exit = NULL;
	room->last_exit = NULL;
	CLEAR_BITS( room->flags );
	room->sector	= 0;
	room->light		= 0;
}


/*
 * Load a mob section.
 */
void fread_mobile( AREA_DATA *tarea, MUD_FILE *fp )
{
	MOB_PROTO_DATA *pMobIndex;
	int				x;

	if ( !tarea )
	{
		send_log( fp, LOG_FREAD, "fread_mobile: sezione #AREA non trovata" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	for ( ; ; )
	{
		VNUM	vnum;
		char	letter;
		bool	oldmob;
		bool	tmpBootDb;

		/*
		 * Carattere identificativo del vnum
		 */
		letter = fread_letter( fp );
		if ( letter != '#' )
		{
			send_log( fp, LOG_FREAD, "fread_mobile: # non trovato" );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;
		}

		/*
		 * Lettura vnum mobile
		 */
		vnum = fread_number( fp );
		if ( vnum == 0 )
			break;

		tmpBootDb = fBootDb;
		fBootDb = FALSE;
		if ( get_mob_index(fp, vnum) )
		{
			if ( tmpBootDb )
			{
				send_log( fp, LOG_FREAD, "fread_mobile: vnum #%d duplicato", vnum );
				exit( EXIT_FAILURE );
			}
			else
			{
				pMobIndex = get_mob_index( fp, vnum );
				send_log( fp, LOG_FREAD, "fread_mobile: pulizia del mobile #%d", vnum );
				clean_mobile_index( pMobIndex );
				oldmob = TRUE;
			}
		}
		else
		{
			oldmob = FALSE;
			CREATE( pMobIndex, MOB_PROTO_DATA, 1 );
		}

		fBootDb = tmpBootDb;

		pMobIndex->vnum = vnum;
		pMobIndex->area = tarea;

		/*
		 * Inizializza alcune variabili
		 */
		pMobIndex->pShop			= NULL;
		pMobIndex->rShop			= NULL;

		/*
		 * Lettura Nomi e Descrizioni del Mob
		 */
		pMobIndex->name			= fread_string( fp );
		pMobIndex->short_descr	= fread_string( fp );
		pMobIndex->long_descr	= fread_string( fp );
		strip_crnl_end( pMobIndex->long_descr );	/* toglie la nuova linea come vuole lo stile bard */

		pMobIndex->description		= fread_string( fp );

		/*
		 * Prima riga dopo la descrizione
		 */
		pMobIndex->act				= fread_old_bitvector( fp, CODE_MOB );		/* (FF) spostarla nella struttura del mob */
		if ( pMobIndex->vnum == 21436 )	/* (FF) visto che la fread_old_bitvector è bacata sulla lettura de &, faccio così per settare la 33° flag */
			SET_BIT( pMobIndex->act, MOB_LOCKER );
		pMobIndex->affected_by		= fread_old_bitvector( fp, CODE_AFFECT );
		pMobIndex->alignment		= fread_number( fp );
		letter						= fread_letter( fp );	/* (CC) da togliere, saranno tutti complex */

		/*
		 * Seconda riga dopo la descrizione
		 */
		pMobIndex->level			= fread_number( fp );
									  fread_number( fp );
		pMobIndex->ac				= fread_number( fp );
		pMobIndex->hitnodice		= fread_number( fp );
		/* 'd'	*/					  fread_letter( fp );
		pMobIndex->hitsizedice		= fread_number( fp );
		/* '+'	*/					  fread_letter( fp );
		pMobIndex->hitplus			= fread_number( fp );
		pMobIndex->damnodice		= fread_number( fp );
		/* 'd'	*/					  fread_letter( fp );
		pMobIndex->damsizedice		= fread_number( fp );
		/* '+'	*/					  fread_letter( fp );
		pMobIndex->damplus			= fread_number( fp );

		/*
		 * Terza riga dopo la descrizione
		 */
		if ( HAS_BIT(table_race[pMobIndex->race]->flags, RACEFLAG_NOMONEY) )
									  fread_number( fp );
		else
			pMobIndex->gold			= fread_number( fp ) / 4;
									  fread_number( fp );

		/*
		 * Quarta riga dopo la descrizione
		 */
		pMobIndex->position			= fread_number( fp );
		if ( pMobIndex->position < 100 )
		{
			switch ( pMobIndex->position )
			{
			  default:
			  case  0:
			  case  1:
			  case  2:
			  case  3:
			  case  4:								break;
			  case  5:	pMobIndex->position =  6;	break;
			  case  6:	pMobIndex->position =  8;	break;
			  case  7:	pMobIndex->position =  9;	break;
			  case  8:	pMobIndex->position = 12;	break;
			  case  9:	pMobIndex->position = 13;	break;
			  case 10:	pMobIndex->position = 14;	break;
			  case 11:	pMobIndex->position = 15;	break;
			}
		}
		else
			pMobIndex->position -= 100;

		pMobIndex->defposition		= fread_number( fp );
		if ( pMobIndex->defposition < 100 )
		{
			switch ( pMobIndex->defposition )
			{
			  default:
			  case  0:
			  case  1:
			  case  2:
			  case  3:
			  case  4:									break;
			  case  5:	pMobIndex->defposition =  6;	break;
			  case  6:	pMobIndex->defposition =  8;	break;
			  case  7:	pMobIndex->defposition =  9;	break;
			  case  8:	pMobIndex->defposition = 12;	break;
			  case  9:	pMobIndex->defposition = 13;	break;
			  case 10:	pMobIndex->defposition = 14;	break;
			  case 11:	pMobIndex->defposition = 15;	break;
			}
		}
		else
			pMobIndex->defposition -= 100;

		/* Acquisisce il sesso del mob, converte i valori 2 (neutral) in maschio */
		pMobIndex->sex			= fread_number( fp );
		--pMobIndex->sex;
		if ( pMobIndex->sex < 0 || pMobIndex->sex > 1 )
			pMobIndex->sex = SEX_MALE;

		if ( letter != 'S' && letter != 'C' )
		{
			send_log( fp, LOG_FREAD, "fread_mobile: vnum %d: letter '%c' not S or C.", vnum, letter );
			exit( EXIT_FAILURE );
		}

		if ( letter == 'C' )	/* Complex mob */
		{
			/*
			 * Prima riga del complex mob
			 */
			for ( x = 0;  x < MAX_ATTR;  x++ )
			{
				switch ( x )
				{
				  case 0:	pMobIndex->attr_perm[ATTR_STR] = fread_number( fp );	break;		/* forza */
				  case 1:	pMobIndex->attr_perm[ATTR_INT] = fread_number( fp );	break;		/* intelligenza */
				  case 2:	pMobIndex->attr_perm[ATTR_WIL] = fread_number( fp );	break;		/* saggezza->volontà */
				  case 3:	pMobIndex->attr_perm[ATTR_AGI] = fread_number( fp );	break;		/* destrezza */
				  case 4:	pMobIndex->attr_perm[ATTR_CON] = fread_number( fp );	break;		/* costituzione */
				  case 5:	pMobIndex->attr_perm[ATTR_BEA] = fread_number( fp );	break;		/* carisma->bellezza */
				  case 6:	pMobIndex->attr_perm[ATTR_MIG] = fread_number( fp );	break;		/* ex-fortuna */
				  default:
					pMobIndex->attr_perm[x] = number_range( 11, 15 );	/* (CC) da convertire per ottenere mob con le 15 caratteristiche */
					break;
				}
				if ( pMobIndex->attr_perm[x] < 0 || pMobIndex->attr_perm[x] > 25 )
					send_log( fp, LOG_FREAD, "fread_mobile: attr errato: %d", pMobIndex->attr_perm[x] );
				pMobIndex->attr_perm[x] = convert_d20_to_d100( pMobIndex->attr_perm[x] );
			}

			for ( x = 0;  x < MAX_SENSE;  x++ )
				pMobIndex->sense_perm[x]= number_range( DEF_MIN_ATTR, DEF_MAX_ATTR );

			/*
			 * Seconda riga del complex mob
			 */
			for ( x = 0;  x < MAX_SAVETHROW;  x++ )
				pMobIndex->saving_throw[x] = fread_number( fp );	/* (RR) e che test se po' fa? che limit cià? */

			/*
			 * Terza riga del complex mob
			 */
			pMobIndex->race			= fread_number( fp );
			pMobIndex->class		= fread_number( fp );

			/* Per altezza e peso */
			pMobIndex->height		= fread_number( fp );
			/* Converte in centimetri */
			if ( !HAS_BIT(tarea->flags, AREAFLAG_WEIGHTHEIGHT) )
				pMobIndex->height	= inch_to_centimetre( pMobIndex->height );
			/* Se l'altezza è zero gli dà l'altezza media razziale */
			if ( pMobIndex->height == 0 )
			{
				pMobIndex->height = number_range( table_race[pMobIndex->race]->height * (1.00 - MAX_FACTOR_HEIGHT/2),
								 				  table_race[pMobIndex->race]->height * (1.00 + MAX_FACTOR_HEIGHT/2) );
			}

			pMobIndex->weight		= fread_number( fp );
			/* Converte in grammi */
			if ( !HAS_BIT(tarea->flags, AREAFLAG_WEIGHTHEIGHT) )
				pMobIndex->weight	= once_to_gram( pMobIndex->weight );	/* converte in grammi */
			/* Se il peso è zero gli dà l'altezza media razziale */
			if ( pMobIndex->weight == 0 )
			{
				pMobIndex->weight = number_range( table_race[pMobIndex->race]->weight * (1.00 - MAX_FACTOR_WEIGHT/2),
								 				  table_race[pMobIndex->race]->weight * (1.00 + MAX_FACTOR_WEIGHT/2) );
			}

			fread_number( fp );			/* pMobIndex->speaks	= (CC) = fread_old_bitvector( fp, CODE_LANGUAGE );*/
			pMobIndex->speaking		= LANGUAGE_COMMON;	/* Inutile da caricare (CC) */
			fread_number( fp );
			if ( pMobIndex->vnum == 10394 )	/* (TT) fino a che non si fa bene il sistema lingue con la (CC) */
			{
				SET_BIT( pMobIndex->speaks, LANGUAGE_COMMON );
				SET_BIT( pMobIndex->speaks, LANGUAGE_ELVEN );
				SET_BIT( pMobIndex->speaks, LANGUAGE_DWARVEN );
				SET_BIT( pMobIndex->speaks, LANGUAGE_PIXIE );
				SET_BIT( pMobIndex->speaks, LANGUAGE_OGRE );
				SET_BIT( pMobIndex->speaks, LANGUAGE_ORCISH );
				SET_BIT( pMobIndex->speaks, LANGUAGE_TROLLISH );
				SET_BIT( pMobIndex->speaks, LANGUAGE_RODENT );
				SET_BIT( pMobIndex->speaks, LANGUAGE_INSECTOID );
				SET_BIT( pMobIndex->speaks, LANGUAGE_MAMMAL );
				SET_BIT( pMobIndex->speaks, LANGUAGE_REPTILE );
				SET_BIT( pMobIndex->speaks, LANGUAGE_DRAGON );
				SET_BIT( pMobIndex->speaks, LANGUAGE_DEATH );
				SET_BIT( pMobIndex->speaks, LANGUAGE_GOBLIN );
				SET_BIT( pMobIndex->speaks, LANGUAGE_ANCIENT );
				SET_BIT( pMobIndex->speaks, LANGUAGE_HALFLING );
				SET_BIT( pMobIndex->speaks, LANGUAGE_GITH );
				SET_BIT( pMobIndex->speaks, LANGUAGE_GNOME );
				SET_BIT( pMobIndex->speaks, LANGUAGE_DEMON );
				SET_BIT( pMobIndex->speaks, LANGUAGE_GNOLLISH );
				SET_BIT( pMobIndex->speaks, LANGUAGE_DROW );
				SET_BIT( pMobIndex->speaks, LANGUAGE_CANINE );
				SET_BIT( pMobIndex->speaks, LANGUAGE_FELINE );
				SET_BIT( pMobIndex->speaks, LANGUAGE_AVIS );
				SET_BIT( pMobIndex->speaks, LANGUAGE_MARINE );
				SET_BIT( pMobIndex->speaks, LANGUAGE_TREELEAF );
			}

			pMobIndex->numattacks	= fread_number( fp );	/* (RR) che test ci vorrà mai? */


			/*
			 * Quarta riga del Complex Mob
			 */
			pMobIndex->hitroll		= fread_number( fp );
			pMobIndex->damroll		= fread_number( fp );
			pMobIndex->xflags		= fread_old_bitvector( fp, CODE_PART );
			pMobIndex->resistant	= fread_old_bitvector( fp, CODE_RIS );
			pMobIndex->immune		= fread_old_bitvector( fp, CODE_RIS );
			pMobIndex->susceptible	= fread_old_bitvector( fp, CODE_RIS );
			pMobIndex->attacks		= fread_old_bitvector( fp, CODE_ATTACK );
			pMobIndex->defenses		= fread_old_bitvector( fp, CODE_DEFENSE );
		}
		else
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
				pMobIndex->attr_perm[x] = number_range( DEF_MIN_ATTR, DEF_MAX_ATTR );

			for ( x = 0;  x < MAX_SENSE;  x++ )
				pMobIndex->sense_perm[x]= number_range( DEF_MIN_ATTR, DEF_MAX_ATTR );

			pMobIndex->race				=  0;
			pMobIndex->class			=  CLASS_FIGHTER;
			pMobIndex->numattacks		=  0;

			CLEAR_BITS( pMobIndex->xflags );
			CLEAR_BITS( pMobIndex->resistant );
			CLEAR_BITS( pMobIndex->immune );
			CLEAR_BITS( pMobIndex->susceptible );
			CLEAR_BITS( pMobIndex->attacks  );
			CLEAR_BITS( pMobIndex->defenses );
		}

		letter = fread_letter( fp );
		if ( letter == '>' )
		{
			ungetc( letter, fp->file );
			fread_mudprog( fp, &pMobIndex->first_mudprog, &pMobIndex->last_mudprog, FALSE );
		}
		else
		{
			ungetc( letter, fp->file );
		}

#ifdef DEBUG_LOAD
		/* Controlla i valori letti del mob */
		check_mobile( pMobIndex, fp );
#endif

		if ( !oldmob )
		{
			LINK( pMobIndex, first_mob_proto, last_mob_proto, next, prev );
			top_mob_proto++;
		}
	} /* chiude il for */
}


/*
 * Load an obj section (prototipo e non (FF))
 */
void fread_object( AREA_DATA *tarea, MUD_FILE *fp, bool proto )
{
	OBJ_DATA	*obj;
	char		 letter;
	char		*ln;
	int			 x[5];

	if ( !tarea )
	{
		send_log( fp, LOG_FREAD, "fread_object: sezione #AREA non trovata." );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	for ( ; ; )
	{
		VNUM	vnum;
		bool	tmpBootDb;
		bool	oldobj;

		letter				= fread_letter( fp );
		if ( letter != '#' )
		{
			send_log( fp, LOG_FREAD, "fread_object: # non trovato." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;
		}

		/*
		 * Lettura vnum oggetto
		 */
		vnum				= fread_number( fp );
		if ( vnum == 0 )
			break;

		tmpBootDb = fBootDb;
		fBootDb = FALSE;
		if ( get_obj_proto(fp, vnum) )
		{
			if ( tmpBootDb )
			{
				send_log( fp, LOG_FREAD, "fread_object: vnum #%d duplicato.", vnum );
				exit( EXIT_FAILURE );
			}
			else
			{
				obj = get_obj_proto( fp, vnum );
				send_log( fp, LOG_FREAD, "fread_object: pulizia dell'oggetto #%d", vnum );
				clean_object( obj );
				oldobj = TRUE;
			}
		}
		else
		{
			oldobj = FALSE;
			CREATE( obj, OBJ_DATA, 1 );
		}

		fBootDb = tmpBootDb;

		/* Associazione della flag di prototipo */
		obj->prototype = proto;

		obj->vnum	= vnum;
		obj->area	= tarea;

		/*
		 * Lettura Moni e Descrizioni dell'Oggetto
		 */
		obj->name			= fread_string( fp );
		obj->short_descr	= fread_string( fp );
		obj->long_descr		= fread_string( fp );
		strip_crnl_end( obj->long_descr );	/* toglie la nuova linea come vuole lo stile bard */
		obj->action_descr	= fread_string( fp );

		/*
		 * Prima riga degli oggetti dopo le stringhe
		 */
		obj->type			= fread_number( fp );
		obj->extra_flags	= fread_old_bitvector( fp, CODE_OBJFLAG );

		ln = fread_line( fp );
		x[0] = x[1] = 0;
		sscanf( ln, "%d %d", &x[0], &x[1] );

		obj->wear_flags	= convert_bitvector( x[0] );
		obj->layers		= convert_bitvector( x[1] );		/* (TT) che test metterci? */

		/*
		 * Seconda riga degli oggetti dopo le stringhe, i value
		 */
		fread_values_area( tarea, fp, obj );

		/*
		 * Terza riga degli oggetti dopo le stringhe
		 */
		obj->weight = fread_number( fp );
		if ( !HAS_BIT(tarea->flags, AREAFLAG_WEIGHTHEIGHT) )
			obj->weight	= once_to_gram( obj->weight );	/* Converte in grammi */
		obj->weight		= UMAX( 1, obj->weight );
		switch ( obj->vnum )	/* (TT) (RR) temporaneo */
		{
		  case 10333:	obj->weight = 100;	break;
		  case 2:		obj->weight = 1;	break;
		  case 3:		obj->weight = 1;	break;
		}
		obj->cost			= fread_number( fp );
							  fread_number( fp );		/* ex-rent */

		if ( tarea->version == 1 )
		{
			switch ( obj->type )
			{
			  case OBJTYPE_SCROLL:
				obj->scroll->sn1 = skill_lookup( fread_word(fp) );
				obj->scroll->sn2 = skill_lookup( fread_word(fp) );
				obj->scroll->sn3 = skill_lookup( fread_word(fp) );
				break;

			  case OBJTYPE_POTION:
				obj->potion->sn1 = skill_lookup( fread_word(fp) );
				obj->potion->sn2 = skill_lookup( fread_word(fp) );
				obj->potion->sn3 = skill_lookup( fread_word(fp) );
				break;

			  case OBJTYPE_PILL:
				obj->pill->sn1 = skill_lookup( fread_word(fp) );
				obj->pill->sn2 = skill_lookup( fread_word(fp) );
				obj->pill->sn3 = skill_lookup( fread_word(fp) );
				break;

			  case OBJTYPE_WAND:
			  case OBJTYPE_STAFF:
				obj->wand->sn = skill_lookup( fread_word(fp) );
				break;

			  case OBJTYPE_SALVE:
				obj->salve->sn1 = skill_lookup( fread_word(fp) );
				obj->salve->sn2 = skill_lookup( fread_word(fp) );
				break;
			}
		}

		for ( ; ; )
		{
			letter = fread_letter( fp );

			if ( letter == 'A' )
			{
				AFFECT_DATA *paf;

				CREATE( paf, AFFECT_DATA, 1 );
				paf->type		= -1;
				paf->duration	= -1;
				paf->location	= fread_number( fp );

				if ( paf->location == APPLY_WEAPONSPELL
				  || paf->location == APPLY_WEARSPELL
				  || paf->location == APPLY_REMOVESPELL
				  || paf->location == APPLY_STRIPSN
				  || paf->location == APPLY_RECURRINGSPELL )
				{
					paf->modifier		= slot_lookup( fread_number(fp) );
				}
				else
					paf->modifier		= fread_number( fp );

				LINK( paf, obj->first_affect, obj->last_affect, next, prev );
				top_affect++;
				/* (RR) Fare una funzione check_ per gli affect */
			}
			else if ( letter == 'E' )
			{
				EXTRA_DESCR_DATA *ed;

				CREATE( ed, EXTRA_DESCR_DATA, 1 );
				ed->keyword			= fread_string( fp );
				ed->description		= fread_string( fp );
				LINK( ed, obj->first_extradescr, obj->last_extradescr, next, prev );
				top_ed++;
			}
			else if ( letter == '>' )
			{
				ungetc( letter, fp->file );
				fread_mudprog( fp, &obj->first_mudprog, &obj->last_mudprog, FALSE );
			}
			else
			{
				ungetc( letter, fp->file );
				break;
			}
		} /* chiude il for */

		/*
		 * Translate spell "slot numbers" to internal "skill numbers."
		 */
		if ( tarea->version == 0 )
		{
			switch ( obj->type )
			{
			  case OBJTYPE_SCROLL:
				obj->scroll->sn1 = slot_lookup( obj->scroll->sn1 );
				obj->scroll->sn2 = slot_lookup( obj->scroll->sn2 );
				obj->scroll->sn3 = slot_lookup( obj->scroll->sn3 );
				break;

			  case OBJTYPE_POTION:
				obj->potion->sn1 = slot_lookup( obj->potion->sn1 );
				obj->potion->sn2 = slot_lookup( obj->potion->sn2 );
				obj->potion->sn3 = slot_lookup( obj->potion->sn3 );
				break;

			  case OBJTYPE_PILL:
				obj->pill->sn1 = slot_lookup( obj->pill->sn1 );
				obj->pill->sn2 = slot_lookup( obj->pill->sn2 );
				obj->pill->sn3 = slot_lookup( obj->pill->sn3 );
				break;

			  case OBJTYPE_WAND:
			  case OBJTYPE_STAFF:
				obj->wand->sn	= slot_lookup( obj->wand->sn );
				break;

			  case OBJTYPE_SALVE:
				obj->salve->sn1 = slot_lookup( obj->salve->sn1 );
				obj->salve->sn2 = slot_lookup( obj->salve->sn2 );
				break;
			}
		}

#ifdef DEBUG_LOAD
		check_object( obj, fp );
#endif

		if ( !oldobj )
		{
			LINK( obj, first_object_proto, last_obj_proto, next, prev );
			top_obj_proto++;
		}
	} /* chiude il for */
}


/*
 * Load a room section.
 */
void fread_room( AREA_DATA *tarea, MUD_FILE *fp )
{
	ROOM_DATA *pRoom;
	char	  *ln;

	if ( !tarea )
	{
		send_log( fp, LOG_FREAD, "fread_room: sezione #AREA non trovata." );
		exit( EXIT_FAILURE );
	}

	for ( ; ; )
	{
		VNUM	vnum;
		char	letter;
		int		door;
		bool	tmpBootDb;
		bool	oldroom;
		int		i,
				x[5];

		letter = fread_letter( fp );
		if ( letter != '#' )
		{
			send_log( fp, LOG_FREAD, "fread_room: # non trovato." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;
		}

		vnum = fread_number( fp );
		if ( vnum == 0 )
			break;

		tmpBootDb = fBootDb;
		fBootDb = FALSE;
		if ( get_room_index(fp, vnum) != NULL )
		{
			if ( tmpBootDb )
			{
				send_log( fp, LOG_FREAD, "fread_room: vnum %d duplicato.", vnum );
				exit( EXIT_FAILURE );
			}
			else
			{
				pRoom = get_room_index( fp, vnum );
				send_log( fp, LOG_BUILD, "Cleaning room: %d", vnum );
				clean_room( pRoom );
				oldroom = TRUE;
			}
		}
		else
		{
			oldroom = FALSE;
			CREATE( pRoom, ROOM_DATA, 1 );
			pRoom->first_person		= NULL;
			pRoom->last_person		= NULL;
			pRoom->first_content	= NULL;
			pRoom->last_content		= NULL;
		}

		fBootDb						= tmpBootDb;

		/*
		 * Inizializza dei valori
		 */
		pRoom->first_extradescr	= NULL;
		pRoom->last_extradescr	= NULL;
		pRoom->area				= tarea;
		pRoom->light			= 0;
		pRoom->first_exit		= NULL;
		pRoom->last_exit		= NULL;

		/* vnum stanza */
		pRoom->vnum				= vnum;

		/*
		 * Stringhe di nome e di descrizione stanza
		 */
		pRoom->name				= fread_string( fp );
		pRoom->description		= fread_string( fp );

		/*
		 * Prima riga della stanza dopo le stringhe di descrizione
		 */
		x[0] = x[1] = x[2] = x[3] = x[4] = x[5] = 0;

		fread_number( fp );		/* Non acquisisce il primo numero */

		pRoom->flags = fread_old_bitvector( fp, CODE_ROOM );

		ln = fread_line( fp );
		sscanf( ln, "%d %d %d %d",
			&x[2], &x[3], &x[4], &x[5] );

		pRoom->sector		= x[2];
		pRoom->tele_delay	= x[3];
		pRoom->tele_vnum	= x[4];
		pRoom->tunnel		= x[5];

		for ( ; ; )
		{
			letter = fread_letter( fp );

			/* S indica che la sezione stanza viene chiusa */
			if ( letter == 'S' )
				break;

			/* Legge un'uscita */
			if ( letter == 'D' )
			{
				EXIT_DATA *pexit;
				int		   locks;

				door = fread_number( fp );
				if ( door < 0 || door >= MAX_DIR )
				{
					send_log( fp, LOG_FREAD, "fread_rooms: numero di porta errato: %d.", door );

					if ( fBootDb )
						exit( EXIT_FAILURE );
					return;
				}
				else
				{
					pexit				= make_exit( pRoom, NULL, door );
					pexit->description	= fread_string( fp );
					pexit->keyword		= fread_string( fp );

					ln = fread_line( fp );
					x[0] = x[1] = x[2] = x[3] = x[4] = x[5] = 0;
					sscanf( ln, "%d %d %d %d %d %d",
						&x[0], &x[1], &x[2], &x[3], &x[4], &x[5] );

					locks			= x[0];
					pexit->key		= x[1];
					pexit->vnum		= x[2];
					pexit->vdir		= door;
					pexit->distance	= x[3];
					pexit->pulltype	= x[4];
					pexit->pull		= x[5];

					switch ( locks )
					{
					  default:
						pexit->flags = convert_bitvector( locks );
						break;
					  case 1:
						pexit->flags = meb( EXIT_ISDOOR );
						break;
					  case 2:
						pexit->flags = multimeb( EXIT_ISDOOR, EXIT_PICKPROOF, -1 );
						break;
					}
				}
			}
			else if ( letter == 'E' )
			{
				EXTRA_DESCR_DATA *ed;

				CREATE( ed, EXTRA_DESCR_DATA, 1 );
				ed->keyword		= fread_string( fp );
				ed->description	= fread_string( fp );
				LINK( ed, pRoom->first_extradescr, pRoom->last_extradescr, next, prev );
				top_ed++;
			}
			else if ( letter == '>' )
			{
				ungetc( letter, fp->file );
				fread_mudprog( fp, &pRoom->first_mudprog, &pRoom->last_mudprog, FALSE );
			}
			else
			{
				send_log( fp, LOG_FREAD, "fread_room: flag '%c' errata, deve essere 'D', 'E', 'S' o '>'", letter );
				if ( fBootDb )
					exit( EXIT_FAILURE );
				return;
			}
		} /* chiude il for */

#ifdef DEBUG_LOAD
		check_room( pRoom, fp );
#endif

		if ( !oldroom )
		{
			i				   = vnum % MAX_KEY_HASH;
			pRoom->next		   = room_index_hash[i];
			room_index_hash[i] = pRoom;
			top_room++;
		}
	} /* chiude il for */
}


/*
 * Load spec proc declarations.
 */
void fread_specials( MUD_FILE *fp )
{
	for ( ; ; )
	{
		MOB_PROTO_DATA *pMobIndex;
		char letter;

		switch ( letter = fread_letter(fp) )
		{
		  default:
			send_log( fp, LOG_FREAD, "fread_specials: letter '%c' not *MS.", letter );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;

		  case 'S':
			return;

		  case '*':
			break;

		  case 'M':
		  {
			char *temp;

			pMobIndex = get_mob_index( fp, fread_number(fp) );
			if ( !pMobIndex )
			{
				send_log( NULL, LOG_FREAD, "fread_specials: 'M': Invalid mob vnum!" );
				break;
			}

			temp = fread_word( fp );
			pMobIndex->spec_fun = specfun_lookup( temp );

			if ( pMobIndex->spec_fun == NULL )
			{
				send_log( fp, LOG_FREAD, "fread_specials: 'M': vnum %d.", pMobIndex->vnum );
				pMobIndex->spec_funname = NULL;
			}
			else
			{
				DISPOSE( pMobIndex->spec_funname );
				pMobIndex->spec_funname = str_dup( temp );
			}
		  }
		  break;
		}
		fread_to_eol( fp );
	}
}


/*
 * Go through all areas, and set up initial economy based on mob levels and gold
 */
void init_economy( void )
{
	AREA_DATA	   *tarea;
	MOB_PROTO_DATA *mob;
	int				idx,
					gold,
					rng;

	for ( tarea = first_area;  tarea;  tarea = tarea->next )
	{
		/* skip area if they already got some gold */
		if ( tarea->economy_high > 0 || tarea->economy_low > 10000 )
			continue;
		rng = tarea->level_high - tarea->level_low;

		if ( rng )
			rng /= 2;
		else
			rng = 25;

		gold = rng * rng * 50000;
		boost_economy( tarea, gold );

		for ( idx = tarea->vnum_low;  idx < tarea->vnum_high;  idx++ )
		{
			if ( (mob = get_mob_index(NULL, idx)) != NULL )
				boost_economy( tarea, mob->gold*10 );
		}
	}
}


/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
	ROOM_DATA	*pRoom;
	EXIT_DATA	*pexit;
	EXIT_DATA	*pexit_next;
	EXIT_DATA	*rev_exit;
	int			 x;

	for ( x = 0;  x < MAX_KEY_HASH;  x++ )
	{
		for ( pRoom  = room_index_hash[x];
			pRoom;
			pRoom  = pRoom->next )
		{
			bool fexit;

			fexit = FALSE;
			for ( pexit = pRoom->first_exit;  pexit;  pexit = pexit_next )
			{
				pexit_next = pexit->next;
				pexit->rvnum = pRoom->vnum;
				if ( pexit->vnum <= 0
				  || (pexit->to_room=get_room_index(NULL, pexit->vnum)) == NULL )
				{
					send_log( NULL, LOG_BUILD, "fix_exits: stanza %d, l'uscita %s è collegata a un vnum errato (%d), cancellazione dell'uscita.",
						pRoom->vnum,
						code_name(NULL, pexit->vdir, CODE_DIR),
						pexit->vnum );
					extract_exit( pRoom, pexit );
				}
				else
				{
					fexit = TRUE;
				}
			}
			if ( !fexit )
				SET_BIT( pRoom->flags, ROOM_NOMOB );
		}
	}

	/* Set all the rexit pointers */
	for ( x = 0;  x < MAX_KEY_HASH;  x++ )
	{
		for ( pRoom = room_index_hash[x];  pRoom;  pRoom = pRoom->next )
		{
			for ( pexit = pRoom->first_exit;  pexit;  pexit = pexit->next )
			{
				if ( pexit->to_room && !pexit->rexit )
				{
					rev_exit = get_exit_to( pexit->to_room, table_dir[pexit->vdir].rev_dir, pRoom->vnum );
					if ( rev_exit )
					{
						pexit->rexit	= rev_exit;
						rev_exit->rexit	= pexit;
					}
				}
			}
		}
	}
}


/*
 * Get diku-compatable exit by number.
 */
EXIT_DATA *get_exit_number( ROOM_DATA *room, int xit )
{
	EXIT_DATA	*pexit;
	int			 count;

	count = 0;
	for ( pexit = room->first_exit;  pexit;  pexit = pexit->next )
	{
		if ( ++count == xit )
			return pexit;
	}

	return NULL;
}


/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA **xit1, EXIT_DATA **xit2 )
{
	int d1, d2;

	d1 = (*xit1)->vdir;
	d2 = (*xit2)->vdir;

	if ( d1 < d2 )
		return -1;

	if ( d1 > d2 )
		return 1;

	return 0;
}


void sort_exits( ROOM_DATA *room )
{
	EXIT_DATA *pexit;
	EXIT_DATA *exits[MAX_ROOM_EXITS];
	int		   x;
	int		   nexits;

	nexits = 0;
	for ( pexit = room->first_exit;  pexit;  pexit = pexit->next )
	{
		exits[nexits++] = pexit;
		if ( nexits >= MAX_ROOM_EXITS )
		{
			send_log( NULL, LOG_BUG, "sort_exits: più di %d uscite nella stanza %d dell'area %s.",
				nexits, room->vnum, room->area->name );
			return;
		}
	}

	qsort( &exits[0], nexits, sizeof( EXIT_DATA * ), (int(*) (const void *, const void *)) exit_comp );

	for ( x = 0; x < nexits; x++ )
	{
		if ( x > 0 )
			exits[x]->prev	= exits[x-1];
		else
		{
			exits[x]->prev	= NULL;
			room->first_exit	= exits[x];
		}

		if ( x >= (nexits - 1) )
		{
			exits[x]->next	= NULL;
			room->last_exit	= exits[x];
		}
		else
		{
			exits[x]->next	= exits[x+1];
		}
	}
}


void randomize_exits( ROOM_DATA *room, int maxdir )
{
	EXIT_DATA  *pexit;
	int			nexits;
	int			d0;
	int			d1;
	int			count;
	int			door;
	int			vdirs[MAX_ROOM_EXITS];

	nexits = 0;
	for ( pexit = room->first_exit;  pexit;  pexit = pexit->next )
		vdirs[nexits++] = pexit->vdir;

	for ( d0 = 0;  d0 < nexits;  d0++ )
	{
		if ( vdirs[d0] > maxdir )
			continue;

		count = 0;
		while ( vdirs[(d1 = number_range(d0, nexits-1))] > maxdir || ++count > 5 );
		{
			if ( vdirs[d1] > maxdir )
				continue;
		}

		door		= vdirs[d0];
		vdirs[d0]	= vdirs[d1];
		vdirs[d1]	= door;
	}

	count = 0;

	for ( pexit = room->first_exit;  pexit;  pexit = pexit->next )
		pexit->vdir = vdirs[count++];

	sort_exits( room );
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *make_mobile( MOB_PROTO_DATA *pMobIndex )
{
	CHAR_DATA *mob;
	int		   i;

	if ( !pMobIndex )
	{
		send_log( NULL, LOG_BUG, "make_mobile: pMobIndex è NULL." );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return NULL;
	}

	CREATE( mob, CHAR_DATA, 1 );
	init_char( mob, TRUE );
	mob->pIndexData			= pMobIndex;

	mob->editor				= NULL;
	mob->name				= str_dup( pMobIndex->name );
	mob->short_descr		= str_dup( pMobIndex->short_descr );
	mob->long_descr			= str_dup( pMobIndex->long_descr  );
	mob->description		= str_dup( pMobIndex->description );
	mob->spec_fun			= pMobIndex->spec_fun;
	if ( pMobIndex->spec_funname )
		mob->spec_funname	= str_dup( pMobIndex->spec_funname );
	mob->level				= number_fuzzy( pMobIndex->level );
	mob->level				= URANGE( 1, pMobIndex->level, LVL_LEGEND );
	SET_BITS( mob->act, pMobIndex->act );

	if ( HAS_BIT_ACT(mob, MOB_INCOGNITO) )
		mob->mobinvis		= mob->level;

	SET_BITS( mob->affected_by, pMobIndex->affected_by );
	mob->alignment			= pMobIndex->alignment;
	mob->sex				= pMobIndex->sex;

	if ( pMobIndex->ac )
		mob->armor			= pMobIndex->ac;
	else
		mob->armor			= interpolate( get_level(mob), 100, -100 );		/* qui non divido per due il livello */

	if ( !pMobIndex->hitnodice )
	{
		/* (TT) da testare, forse cambiare il + number_range con un check della costituzione del mob */
		/* Qui non divido il fattore di livello, lo tengo per fare le cose un po' più toste per il pg */
		mob->points_max[POINTS_LIFE] = get_level(mob) * 8 +
			number_range( get_level(mob) * get_level(mob)/4, get_level(mob) * get_level(mob) );
	}
	else
	{
		mob->points_max[POINTS_LIFE] = pMobIndex->hitnodice
			* number_range( 1, pMobIndex->hitsizedice ) + pMobIndex->hitplus;
	}

	mob->points[POINTS_LIFE]		= mob->points_max[POINTS_LIFE];
	/* lets put things back the way they used to be! */
	mob->gold				= pMobIndex->gold;
	mob->exp				= pMobIndex->exp;
	mob->position			= pMobIndex->position;
	mob->defposition		= pMobIndex->defposition;
	mob->barenumdie			= pMobIndex->damnodice;
	mob->baresizedie		= pMobIndex->damsizedice;
	mob->hitplus			= pMobIndex->hitplus;
	mob->damplus			= pMobIndex->damplus;

	for ( i = 0;  i < MAX_ATTR;  i++ )
		mob->attr_perm[i]	= pMobIndex->attr_perm[i];
	for ( i = 0;  i < MAX_SENSE;  i++ )
		mob->sense_perm[i]	= pMobIndex->sense_perm[i];

	mob->hitroll			= pMobIndex->hitroll;
	mob->damroll			= pMobIndex->damroll;
	mob->race				= pMobIndex->race;
	mob->class				= pMobIndex->class;
	SET_BITS( mob->xflags, pMobIndex->xflags );

	for ( i = 0;  i < MAX_SAVETHROW;  i++ )
		mob->saving_throw[i] = pMobIndex->saving_throw[i];

	mob->height				= pMobIndex->height;
	mob->weight				= pMobIndex->weight;
	SET_BITS( mob->resistant, pMobIndex->resistant );
	SET_BITS( mob->immune, pMobIndex->immune );
	SET_BITS( mob->susceptible, pMobIndex->susceptible );
	SET_BITS( mob->attacks, pMobIndex->attacks );
	SET_BITS( mob->defenses, pMobIndex->defenses );
	mob->numattacks			= pMobIndex->numattacks;
	SET_BITS( mob->speaks, pMobIndex->speaks );
	mob->speaking			= pMobIndex->speaking;

	/*
	 * Perhaps add this to the index later.
	 */
	CLEAR_BITS( mob->no_affected_by );
	CLEAR_BITS( mob->no_resistant );
	CLEAR_BITS( mob->no_immune );
	CLEAR_BITS( mob->no_susceptible );

	/*
	 * Insert in list.
	 */
	LINK( mob, first_char, last_char, next, prev );
	pMobIndex->count++;
	mobs_loaded++;
	return mob;
}


/*
 * Create an instance of an object
 */
OBJ_DATA *make_object( OBJ_PROTO_DATA *pObjProto, int level )
{
	OBJ_DATA *obj;

	if ( !pObjProto )
	{
		send_log( NULL, LOG_BUG, "make_object: pObjProto è NULL" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return NULL;
	}

	if ( pObjProto->prototype == FALSE )
	{
		send_log( NULL, LOG_BUG, "make_object: pObjProto non è un prototipo di oggetto" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return NULL;
	}

	if ( pObjProto->pObjProto )
	{
		send_log( NULL, LOG_BUG, "make_object: pObjProto anche se non è un prototipo di oggetto non ha NULL il puntatore al prototipo" );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return NULL;
	}

	CREATE( obj, OBJ_DATA, 1 );
	/* Copia l'oggetto indicizzato in quello non indicizzato */
	copy_object( pObjProto, obj );

	/* Dopo aver copiato l'oggetto reinizializza i valori chiave */
	obj->vnum		= pObjProto->vnum;
	obj->pObjProto	= pObjProto;
	obj->prototype	= FALSE;
	obj->in_room	= NULL;
	obj->level		= level;
	obj->wear_loc	= -1;
	obj->count		= 1;	/* (??) in make_mobile count viene messo a 0, qui a 1 */
	cur_obj_serial	= UMAX( (cur_obj_serial + 1 ) & ((1<<30)-1), 1 );
	obj->serial		= obj->pObjProto->serial = cur_obj_serial;
	DISPOSE( obj->owner );
	obj->owner		= str_dup( "" );

	/*
	 * Inizializza alcuni valori riguardanti le diverse tipologie di oggetto
	 */
	if ( obj->food )
	{
		/*
		 * optional food condition (rotting food)
		 * value1 is the max condition of the food
		 * value4 is the optional initial condition
		 */
		if ( obj->food->init_cond )
			obj->timer = obj->food->init_cond;
		else
			obj->timer = obj->food->condition;
	}
	else if ( obj->pipe )	/* La pipa è anche un contenitore */
	{
		init_values( obj, OBJTYPE_CONTAINER );
		obj->container->capacity	= obj->pipe->capacity;
		obj->container->key			= 0;
		obj->container->durability	= 12;
		obj->container->type		= OBJTYPE_HERB;
		obj->container->flags		= 0;
	}
	else if ( obj->salve )
	{
		obj->salve->level	= number_fuzzy( obj->salve->level );
		obj->salve->delay	= number_fuzzy( obj->salve->delay );
	}
	else if ( obj->scroll )
	{
		obj->scroll->level	= number_fuzzy( obj->scroll->level );
	}
	else if ( obj->wand )
	{
		obj->wand->level		= number_fuzzy( obj->wand->level );
		obj->wand->max_charges	= number_fuzzy( obj->wand->max_charges );
		if ( obj->wand->curr_charges > obj->wand->curr_charges )
			obj->wand->curr_charges = obj->wand->max_charges;
	}
	else if ( obj->weapon )
	{
		if ( obj->weapon->dice_number == 0 )
			obj->weapon->dice_number = number_fuzzy( number_fuzzy(level * 1/4 + 2) );	/* max 27 */

		if ( obj->weapon->dice_size == 0 )
			obj->weapon->dice_size	 = number_fuzzy( number_fuzzy(level * 3/4 + 6) );	/* max 81 */

/*		obj->weapon->dice_size *= obj->weapon->dice_number; */

		if ( obj->weapon->condition == 0 )
			obj->weapon->condition = INIT_WEAPON_CONDITION;
	}
	else if ( obj->armor )
	{
		if ( obj->armor->ac == 0 )
			obj->armor->ac	= number_fuzzy( level * 1/4 + 2 );

		if ( obj->armor->ac_orig == 0 )
			obj->armor->ac_orig = obj->armor->ac;
	}
	else if ( obj->money )
	{
		obj->money->qnt = obj->cost;
		if ( obj->money->qnt == 0 )
			obj->money->qnt = 1;
	} /* chiude lo switch */

	LINK( obj, first_object, last_object, next, prev );
	pObjProto->count++;
	objects_loaded++;
	objects_physical++;

	return obj;
}


/*
 * Inizializza una nuova struttura char_data
 */
void init_char( CHAR_DATA *ch, bool mob )
{
	int		x;

	ch->spec_funname	= str_dup( "" );
	ch->alloc_ptr		= NULL;
	ch->name			= str_dup( "" );
	ch->short_descr		= str_dup( "" );
	ch->long_descr		= str_dup( "" );
	ch->description		= str_dup( "" );

	CLEAR_BITS( ch->affected_by );
	CLEAR_BITS( ch->xflags );

	ch->race			= RACE_HUMAN;
	ch->class			= CLASS_FIGHTER;
	ch->height			= DEF_HEIGHT;
	ch->weight			= DEF_WEIGHT;
	ch->num_fighting	= 0;			/* Serve altrimenti la space_to_fight non funziona la prima volta che un pg combatte */

	for ( x = 0;  x < MAX_ATTR;  x++ )
		ch->attr_perm[x] = number_range( DEF_MIN_ATTR, DEF_MAX_ATTR );

	for ( x = 0;  x < MAX_SENSE;  x++ )
		ch->sense_perm[x] = number_range( DEF_MIN_ATTR, DEF_MAX_ATTR );

	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		ch->points[x]		= table_points[x].start;
		ch->points_max[x]	= table_points[x].start;
	}

	ch->armor			= 100;
	ch->position		= POSITION_STAND;

	/* (FF) qui aggiungere speaks mettendo le lingue con cui può parlare di default, se poi il mob index ne avrà qualcuna allora le cancellerà prendendo l'altro valore, quindi togliere le procedure in lancguage.c in questo senso */
	if ( IS_PG(ch) )
		ch->speaking	= LANGUAGE_COMMON;
	else
		ch->speaking	= get_lang_speakable( ch, ch );

	ch->barenumdie		= 1;
	ch->baresizedie		= 4;
	ch->substate		= 0;
	ch->tempnum			= 0;

	if ( mob )
	{
		ch->pg = NULL;
	}
}


/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
	OBJ_DATA		*obj;
	AFFECT_DATA 	*paf;
	TIMER			*timer;
	MPROG_ACT_LIST	*mpact;
	MPROG_ACT_LIST	*mpact_next;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "free_char: ch è NULL." );
		return;
	}

	if ( ch->desc )
		send_log( NULL, LOG_BUG, "free_char: char still has descriptor." );

	if ( ch->morph )
		free_char_morph( ch->morph );

	while ( (obj = ch->last_carrying) != NULL )
		free_object( obj );

	while ( (paf = ch->last_affect) != NULL )
		affect_remove( ch, paf );

	while ( (timer = ch->first_timer) != NULL )
		extract_timer( ch, timer );

	if ( ch->editor )
		stop_editing(ch);

	DISPOSE( ch->name		 );
	DISPOSE( ch->short_descr );
	DISPOSE( ch->long_descr	 );
	DISPOSE( ch->description );

	stop_hunting( ch );
	stop_hating ( ch );
	stop_fearing( ch );
	free_fight  ( ch );

	if ( ch->pnote )
		free_note( ch->pnote );

	if ( ch->pg )
	{
		IGNORE_DATA *ignore;

		DISPOSE( ch->pg->pwd );
		DISPOSE( ch->pg->title );
		DISPOSE( ch->pg->bio );
		DISPOSE( ch->pg->afk_message );
		DISPOSE( ch->pg->msg_login_you );
		DISPOSE( ch->pg->msg_login_room );
		DISPOSE( ch->pg->msg_logout_you );
		DISPOSE( ch->pg->msg_logout_room );
		DISPOSE( ch->pg->objective );
		DISPOSE( ch->pg->music_file );
		DISPOSE( ch->pg->msg_slay_victim );
		DISPOSE( ch->pg->msg_slay_room );
		DISPOSE( ch->pg->host );
		DISPOSE( ch->pg->personality );
		DISPOSE( ch->pg->helled_by );
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
		DISPOSE( ch->pg->subprompt );
		DISPOSE( ch->pg->permission_action );

		free_eye( ch->pg->eye );
		free_hair( ch->pg->hair );
		
		free_aliases( ch );

		for ( ignore = ch->pg->first_ignored;  ignore;  ignore = ch->pg->first_ignored )
		{
			UNLINK( ignore, ch->pg->last_ignored, ch->pg->first_ignored, next, prev );
			DISPOSE( ignore->name );
			DISPOSE( ignore );
		}

		DISPOSE( ch->pg );
	} /* chiude l'if */

	for ( mpact = ch->mpact;  mpact;  mpact = mpact_next )
	{
		mpact_next = mpact->next;
		DISPOSE( mpact->buf );
		DISPOSE( mpact		);
	}

	DISPOSE( ch );
}


/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_PROTO_DATA *get_mob_index( MUD_FILE *fp, VNUM vnum )
{
	MOB_PROTO_DATA *pMobIndex;

	if ( vnum < 1 || vnum >= MAX_VNUM )
	{
		send_log( fp, LOG_BUG, "get_mob_index: vnum passato errato: %d", vnum );
		vnum = 1;
	}

	for ( pMobIndex = first_mob_proto;  pMobIndex;  pMobIndex = pMobIndex->next )
	{
		if ( pMobIndex->vnum == vnum )
			return pMobIndex;
	}

	if ( fBootDb )
		send_log( fp, LOG_BUG, "get_mob_index: bad vnum %d.", vnum );

	return NULL;
}


/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_PROTO_DATA *get_obj_proto( MUD_FILE *fp, VNUM vnum )
{
	OBJ_PROTO_DATA *pObj;

	if ( vnum < 1 || vnum >= MAX_VNUM )
	{
		send_log( fp, LOG_BUG, "get_obj_proto: vnum passato errato: %d", vnum );
		return NULL;
	}

	for ( pObj = first_object_proto;  pObj;  pObj = pObj->next )
	{
		if ( !pObj->prototype )
		{
			send_log( fp, LOG_BUG, "get_obj_proto: l'oggetto %d non è segnato come prototipo!", vnum );
			continue;
		}

		if ( pObj->vnum == vnum )
			return pObj;
	}

	if ( fBootDb )
		send_log( fp, LOG_BUG, "get_obj_proto: oggetto con vnum %d inesistente", vnum );

	return NULL;
}


/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_DATA *get_room_index( MUD_FILE *fp, VNUM vnum )
{
	ROOM_DATA *pRoom;

	if ( vnum < 1 || vnum >= MAX_VNUM )
	{
		send_log( fp, LOG_BUG, "get_room_index: vnum passato errato: %d", vnum );
		return NULL;
	}

	for ( pRoom = room_index_hash[vnum%MAX_KEY_HASH];  pRoom;  pRoom = pRoom->next )
	{
		if ( pRoom->vnum == vnum )
			return pRoom;
	}

	if ( fBootDb )
		send_log( fp, LOG_BUG, "get_room_index: bad vnum %d.", vnum );

	return NULL;
}


/*
 * Generate a random door.
 */
int number_door( void )
{
	int door;

	while ( (door = number_mm( ) & (16-1)) > 9 )
		;

	return door;
}


/*
 * Roll some dice.
 */
int dice( int number, int size )
{
	int idice;
	int sum;

	switch ( size )
	{
		case 0:		return 0;
		case 1:		return number;
	}

	for ( idice = 0, sum = 0; idice < number; idice++ )
		sum += number_range( 1, size );

	return sum;
}


/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
	return value_00 + level * (value_32 - value_00) / 32;
}


/*
 * Function to delete a room index
 * Called from do_rdelete in build.c
 */
void free_room_proto( ROOM_DATA *room )
{
	ROOM_DATA			*prev;
	ROOM_DATA			*limbo = get_room_index( NULL, VNUM_ROOM_LIMBO );
	OBJ_DATA			*o;
	CHAR_DATA			*ch;
	EXTRA_DESCR_DATA	*ed;
	EXIT_DATA			*ex;
	MPROG_ACT_LIST		*mpact;
	int					 hash;

	if ( !room )
	{
		send_log( NULL, LOG_BUG, "free_room_proto: la variabile room passata è NULL" );
		return;
	}

	while ( (ch = room->first_person) )
	{
		if ( IS_PG(ch) )
		{
			char_from_room( ch );
			char_to_room( ch, limbo );
		}
		else
		{
			extract_char( ch, TRUE );
		}
	}

	while ( (o = room->first_content) != NULL )
		free_object( o );

	while ( (ed = room->first_extradescr) != NULL )
	{
		room->first_extradescr = ed->next;
		DISPOSE(ed->keyword);
		DISPOSE(ed->description);
		DISPOSE(ed);
		--top_ed;
	}

	while ( (ex = room->first_exit) != NULL )
		extract_exit( room, ex );

	while ( (mpact = room->mpact) != NULL )
	{
		room->mpact = mpact->next;
		DISPOSE( mpact->buf );
		DISPOSE( mpact );
	}

	free_mudprog( room->first_mudprog );

	DISPOSE( room->name );
	DISPOSE( room->description );

	hash = room->vnum % MAX_KEY_HASH;
	if ( room == room_index_hash[hash] )
	{
		room_index_hash[hash] = room->next;
	}
	else
	{
		for ( prev = room_index_hash[hash];  prev;  prev = prev->next )
		{
			if ( prev->next == room )
				break;
		}

		if ( prev )
			prev->next = room->next;
		else
			send_log( NULL, LOG_BUG, "free_room_proto: room %d not in hash bucket %d", room->vnum, hash );
	}

	DISPOSE( room );
	--top_room;
}


/*
 * See comment on free_room_proto
 */
bool free_object_proto( OBJ_PROTO_DATA *oproto )
{
	OBJ_DATA	*obj;
	OBJ_DATA	*obj_next;

	/* Remove references to object index */
	for ( obj = first_object;  obj;  obj = obj_next )
	{
		obj_next = obj->next;
		if ( obj->vnum == oproto->vnum )
			free_object( obj );
	}

	clean_object( oproto );

	UNLINK( oproto, first_object_proto, last_obj_proto, next, prev );

	DISPOSE( oproto );
	top_obj_proto--;
	return TRUE;
}


/*
 * See comment on free_room
 */
bool free_mobile_proto( MOB_PROTO_DATA *pMobIndex )
{
	CHAR_DATA	   *ch;
	CHAR_DATA	   *ch_next;

	for ( ch = first_char;  ch;  ch = ch_next )
	{
		ch_next = ch->next;

		if ( IS_PG(ch) )
			continue;

		if ( ch->pIndexData == pMobIndex )
			extract_char( ch, TRUE );
	}

	free_mudprog( pMobIndex->first_mudprog );

	if ( pMobIndex->pShop )
	{
		UNLINK( pMobIndex->pShop, first_shop, last_shop, next, prev );
		DISPOSE( pMobIndex->pShop );
		--top_shop;
	}

	if ( pMobIndex->rShop )
	{
		UNLINK( pMobIndex->rShop, first_repair, last_repair, next, prev );
		DISPOSE( pMobIndex->rShop );
		--top_repair;
	}

	DISPOSE( pMobIndex->name		);
	DISPOSE( pMobIndex->short_descr	);
	DISPOSE( pMobIndex->long_descr	);
	DISPOSE( pMobIndex->description	);

	UNLINK( pMobIndex, first_mob_proto, last_mob_proto, next, prev );

	DISPOSE( pMobIndex );
	top_mob_proto--;
	return TRUE;
}


/*
 * Creat a new room for online building o wilderness
 */
ROOM_DATA *make_room_index( VNUM vnum )
{
	ROOM_DATA	*pRoom;
	int			 x;

	if ( vnum < 0 || vnum >= MAX_VNUM )
	{
		send_log( NULL, LOG_BUG, "make_room_index: vnum passato errato: %d", vnum );
		return NULL;
	}

	pRoom = get_room_index( NULL, vnum );
	if ( pRoom )
	{
		send_log( NULL, LOG_BUG, "make_room_index: la stanza di vnum %d è già esistente", vnum );
		return NULL;
	}

	CREATE( pRoom, ROOM_DATA, 1 );

	pRoom->first_person		= NULL;
	pRoom->last_person		= NULL;
	pRoom->first_content	= NULL;
	pRoom->last_content		= NULL;
	pRoom->first_extradescr	= NULL;
	pRoom->last_extradescr	= NULL;
	pRoom->area				= NULL;
	pRoom->vnum				= vnum;
	pRoom->name				= str_dup( "Fluttuando nel Vuoto.." );
	pRoom->sector			= SECTOR_FIELD;
	pRoom->light			= 0;
	pRoom->first_exit		= NULL;
	pRoom->last_exit		= NULL;

	x						= vnum % MAX_KEY_HASH;
	pRoom->next				= room_index_hash[x];
	room_index_hash[x]		= pRoom;

	top_room++;
	return pRoom;
}


/*
 * Create a new prototipe object (for online building)
 * Option to clone an existing index object.
 */
OBJ_PROTO_DATA *make_object_proto( VNUM vnum, VNUM cvnum, char *name )
{
	OBJ_DATA *oproto;
	OBJ_DATA *ocopy;
	char	  buf[MSL];

	if ( cvnum > 0 )
		ocopy = get_obj_proto( NULL, cvnum );
	else
		ocopy = NULL;

	CREATE( oproto, OBJ_DATA, 1 );

	if ( !ocopy )
	{
		sprintf( buf, "A newly created %s", name );
		oproto->short_descr		= str_dup( buf  );
		sprintf( buf, "Some god dropped a newly created %s here.", name );
		oproto->long_descr		= str_dup( buf );
		oproto->action_descr	= str_dup( "" );
		oproto->short_descr[0]	= tolower( oproto->short_descr[0] );
		oproto->long_descr[0]	= toupper( oproto->long_descr[0] );
		oproto->type			= OBJTYPE_TRASH;
		CLEAR_BITS( oproto->extra_flags );
		CLEAR_BITS( oproto->magic_flags );
		CLEAR_BITS( oproto->wear_flags );
		oproto->weight			= 1;
		oproto->cost			= 0;
		oproto->first_affect	= NULL;
		oproto->last_affect		= NULL;
		oproto->first_extradescr	= NULL;
		oproto->last_extradescr	= NULL;
	}
	else
		copy_object( ocopy, oproto );

	/* inizializza alcuni valori qui e non prima, al caso che la copia di ocopy li sovrascriva */
	oproto->vnum		= vnum;
	oproto->pObjProto	= NULL;
	oproto->prototype	= TRUE;
	DISPOSE( oproto->name );
	oproto->name		= str_dup( name );

	oproto->count		= 0;
	LINK( oproto, first_object_proto, last_obj_proto, next, prev );
	top_obj_proto++;

	return oproto;
}


/*
 * Create a new INDEX mobile (for online building)
 * Option to clone an existing index mobile.
 */
MOB_PROTO_DATA *make_mobile_proto( VNUM vnum, VNUM cvnum, char *name )
{
	MOB_PROTO_DATA *pMobIndex, *cMobIndex;
	char			buf[MSL];
	int				x;

	if ( cvnum > 0 )
		cMobIndex = get_mob_index( NULL, cvnum );
	else
		cMobIndex = NULL;

	CREATE( pMobIndex, MOB_PROTO_DATA, 1 );

	pMobIndex->vnum		= vnum;
	pMobIndex->count	= 0;
	pMobIndex->killed	= 0;
	pMobIndex->name		= str_dup( name );

	if ( !cMobIndex )
	{
		sprintf( buf, "A newly created %s", name );
		pMobIndex->short_descr		= str_dup( buf  );
		sprintf( buf, "Some god abandoned a newly created %s here.\r\n", name );
		pMobIndex->long_descr		= str_dup( buf );
		pMobIndex->description		= str_dup( "" );
		pMobIndex->short_descr[0]	= tolower( pMobIndex->short_descr[0] );
		pMobIndex->long_descr[0]	= toupper( pMobIndex->long_descr[0]  );
		pMobIndex->description[0]	= toupper( pMobIndex->description[0] );
		CLEAR_BITS( pMobIndex->act );
		CLEAR_BITS( pMobIndex->affected_by );
		pMobIndex->pShop			= NULL;
		pMobIndex->rShop			= NULL;
		pMobIndex->spec_fun			= NULL;
		pMobIndex->first_mudprog			= NULL;
		pMobIndex->alignment		= 0;
		pMobIndex->level			= 1;
		pMobIndex->ac				= 0;
		pMobIndex->hitnodice		= 0;
		pMobIndex->hitsizedice		= 0;
		pMobIndex->hitplus			= 0;
		pMobIndex->damnodice		= 0;
		pMobIndex->damsizedice		= 0;
		pMobIndex->damplus			= 0;
		pMobIndex->gold				= 0;
		pMobIndex->exp				= 0;

		/* Bug -- So we set it back to constants.. :P.. changed to POSITION_STAND -- */
		pMobIndex->position			= POSITION_STAND;
		pMobIndex->defposition		= POSITION_STAND;
		pMobIndex->sex				= 0;
		for ( x = 0;  x < MAX_ATTR;  x++ )
			pMobIndex->attr_perm[x]	= number_range( DEF_MIN_ATTR, DEF_MAX_ATTR );
		for ( x = 0;  x < MAX_SENSE;  x++ )
			pMobIndex->sense_perm[x]= number_range( DEF_MIN_ATTR, DEF_MAX_ATTR );
		pMobIndex->race				= RACE_HUMAN;
		pMobIndex->class			= CLASS_FIGHTER;
		CLEAR_BITS( pMobIndex->resistant );
		CLEAR_BITS( pMobIndex->immune );
		CLEAR_BITS( pMobIndex->susceptible );
		pMobIndex->numattacks		= 0;
		CLEAR_BITS( pMobIndex->attacks	 );
		CLEAR_BITS( pMobIndex->defenses );
		CLEAR_BITS( pMobIndex->xflags	 );
	}
	else
	{
		pMobIndex->short_descr		= str_dup( cMobIndex->short_descr );
		pMobIndex->long_descr		= str_dup( cMobIndex->long_descr  );
		pMobIndex->description		= str_dup( cMobIndex->description );
		SET_BITS( pMobIndex->act, cMobIndex->act );
		SET_BITS( pMobIndex->affected_by, cMobIndex->affected_by );
		pMobIndex->pShop			= NULL;
		pMobIndex->rShop			= NULL;
		pMobIndex->spec_fun			= cMobIndex->spec_fun;
		pMobIndex->first_mudprog			= NULL;
		pMobIndex->alignment		= cMobIndex->alignment;
		pMobIndex->level			= cMobIndex->level;
		pMobIndex->ac				= cMobIndex->ac;
		pMobIndex->hitnodice		= cMobIndex->hitnodice;
		pMobIndex->hitsizedice		= cMobIndex->hitsizedice;
		pMobIndex->hitplus			= cMobIndex->hitplus;
		pMobIndex->damnodice		= cMobIndex->damnodice;
		pMobIndex->damsizedice		= cMobIndex->damsizedice;
		pMobIndex->damplus			= cMobIndex->damplus;
		pMobIndex->gold				= cMobIndex->gold;
		pMobIndex->exp				= cMobIndex->exp;
		pMobIndex->position			= cMobIndex->position;
		pMobIndex->defposition		= cMobIndex->defposition;
		pMobIndex->sex				= cMobIndex->sex;
		for ( x = 0;  x < MAX_ATTR;  x++ )
			pMobIndex->attr_perm[x]	= cMobIndex->attr_perm[x];
		for ( x = 0;  x < MAX_SENSE;  x++ )
			pMobIndex->sense_perm[x]= cMobIndex->sense_perm[x];
		pMobIndex->race				= cMobIndex->race;
		pMobIndex->class			= cMobIndex->class;
		SET_BITS( pMobIndex->xflags, cMobIndex->xflags );
		SET_BITS( pMobIndex->resistant, cMobIndex->resistant );
		SET_BITS( pMobIndex->immune, cMobIndex->immune );
		SET_BITS( pMobIndex->susceptible, cMobIndex->susceptible );
		pMobIndex->numattacks		= cMobIndex->numattacks;
		SET_BITS( pMobIndex->attacks, cMobIndex->attacks );
		SET_BITS( pMobIndex->defenses, cMobIndex->defenses );
	}

	LINK( pMobIndex, first_mob_proto, last_mob_proto, next, prev );
	top_mob_proto++;

	return pMobIndex;
}


/*
 * Creates a simple exit with no fields filled but rvnum and optionally to_room and vnum.
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_DATA *pRoom, ROOM_DATA *to_room, int door )
{
	EXIT_DATA *pexit;
	EXIT_DATA *texit;
	bool	   broke;

	CREATE( pexit, EXIT_DATA, 1 );
	pexit->vdir		= door;
	pexit->rvnum	= pRoom->vnum;
	pexit->to_room	= to_room;
	pexit->distance	= 1;

	if ( to_room )
	{
		pexit->vnum = to_room->vnum;
		texit = get_exit_to( to_room, table_dir[door].rev_dir, pRoom->vnum );

		if ( texit )	/* assign reverse exit pointers */
		{
			texit->rexit = pexit;
			pexit->rexit = texit;
		}
	}

	broke = FALSE;

	for ( texit = pRoom->first_exit;  texit;  texit = texit->next )
	{
		if ( door < texit->vdir )
		{
			broke = TRUE;
			break;
		}
	}

	if ( !pRoom->first_exit )
		pRoom->first_exit = pexit;
	else
	{
		/* keep exits in incremental order - insert exit into list */
		if ( broke && texit )
		{
			if ( !texit->prev )
				pRoom->first_exit	= pexit;
			else
				texit->prev->next	= pexit;

			pexit->prev	= texit->prev;
			pexit->next	= texit;
			texit->prev	= pexit;
			top_exit++;
			return pexit;
		}
		pRoom->last_exit->next	= pexit;
	}

	pexit->next		 = NULL;
	pexit->prev		 = pRoom->last_exit;
	pRoom->last_exit = pexit;
	top_exit++;

	return pexit;
}


void fix_area_exits( AREA_DATA *tarea )
{
	ROOM_DATA	*pRoom;
	EXIT_DATA	*pexit;
	EXIT_DATA	*rev_exit;
	VNUM		 rnum;
	bool		 fexit;

	for ( rnum = tarea->vnum_low; rnum <= tarea->vnum_high; rnum++ )
	{
		if ( (pRoom = get_room_index(NULL, rnum)) == NULL )
			continue;

		fexit = FALSE;
		for ( pexit = pRoom->first_exit;  pexit;  pexit = pexit->next )
		{
			fexit = TRUE;

			pexit->rvnum = pRoom->vnum;
			if ( pexit->vnum <= 0 )
				pexit->to_room = NULL;
			else
				pexit->to_room = get_room_index( NULL, pexit->vnum );
		}

		if ( !fexit )
			SET_BIT( pRoom->flags, ROOM_NOMOB );
	}

	for ( rnum = tarea->vnum_low;  rnum <= tarea->vnum_high;  rnum++ )
	{
		if ( (pRoom = get_room_index(NULL, rnum)) == NULL )
			continue;

		for ( pexit = pRoom->first_exit;  pexit;  pexit = pexit->next )
		{
			if ( pexit->to_room && !pexit->rexit )
			{
				rev_exit = get_exit_to( pexit->to_room, table_dir[pexit->vdir].rev_dir, pRoom->vnum );
				if ( rev_exit )
				{
					pexit->rexit	= rev_exit;
					rev_exit->rexit	= pexit;
				}
			}
		}
	}
}


/*
 * Legge la sezione di header di un'area
 * Attenzione, solo alla fine della sezione l'area viene linkata
 */
void fread_area( AREA_DATA *tarea, MUD_FILE *fp, bool proto )
{
	if ( !tarea )
	{
		send_log( NULL, LOG_BUG, "fread_area: tarea passata è NULL" );
		return;
	}

	for ( ; ; )
	{
		char		*word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_area: fine del file prematuro nella lettura" );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Name"		 ) )
		{
			tarea->name = fread_string( fp );
			if ( !VALID_STR(tarea->name) )
			{
				send_log( fp, LOG_FREAD, "fread_area: manca il nome dell'area" );
				if ( fBootDb )
					exit( EXIT_FAILURE );
				return;
			}
		}
		else if ( !str_cmp(word, "Author"	 ) )
		{
			if ( tarea->author )
				DISPOSE( tarea->author );

			tarea->author = fread_string( fp );

			if ( !VALID_STR(tarea->author) )
			{
				send_log( fp, LOG_FREAD, "fread_area: manca il nome dell'autore dell'area" );
				if ( fBootDb )
					exit( EXIT_FAILURE );
				return;
			}
		}
		else if ( !str_cmp(word, "Climate"	 ) )
		{
			tarea->weather->climate_temp	=									fread_number( fp );
			tarea->weather->climate_precip	=									fread_number( fp );
			tarea->weather->climate_wind	=									fread_number( fp );
		}
		else if ( !str_cmp(word, "Neighbor"	 ) )
		{
			NEIGHBOR_DATA	*neighbor;

			CREATE( neighbor, NEIGHBOR_DATA, 1 );
			neighbor->name		=												fread_string( fp );
			LINK( neighbor, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
		}
		else if ( !str_cmp(word, "Color"	 ) )	tarea->color =				fread_synonym( fp, CODE_COLOR );
		else if ( !str_cmp(word, "Economy"	 ) )
		{
			tarea->economy_high =												fread_number( fp );
			tarea->economy_low	=												fread_number( fp );
		}
		else if ( !str_cmp(word, "Flags"	 ) )	tarea->flags =				fread_code_bit( fp, CODE_AREAFLAG );
		else if ( !str_cmp(word, "LevelRange") )
		{
			tarea->level_low	=											fread_number( fp );
			tarea->level_high =												fread_number( fp );

			if ( tarea->level_low == 0 || tarea->level_high == 0 )
			{
				send_log( fp, LOG_FREAD, "fread_area: acquisizione campo LevelRange errata, mancante oppure livello/i a zero." );
				if ( fBootDb )
					exit( EXIT_FAILURE );
				return;
			}
		}
		else if ( !str_cmp(word, "ResetFreq" ) )
		{
			tarea->reset_frequency =											fread_number( fp );

			if ( tarea->reset_frequency )
				tarea->age = tarea->reset_frequency;
		}
		else if ( !str_cmp(word, "SpellLimit") )	tarea->spell_limit =		fread_number( fp );
		else if ( !str_cmp(word, "Version"	 ) )	tarea->version =			fread_number( fp );
		else if ( !str_cmp(word, "Vnums"	 ) )
		{
			tarea->vnum_low	 =													fread_number( fp );
			tarea->vnum_high =													fread_number( fp );

			if ( tarea->vnum_low == 0 || tarea->vnum_high == 0
			  || tarea->vnum_low >= tarea->vnum_high )
			{
				send_log( fp, LOG_FREAD, "fread_area: acquisizione campo Vnums errata, mancante oppure vnum settato/i a zero." );
				if ( fBootDb )
					exit( EXIT_FAILURE );
				return;
			}
		}
		else if ( !str_cmp(word, "ResetMsg1" ) )	/* (FF) acquisizione con una sola etichetta prefixxata e il numer acquisito poi? */
		{
			if ( VALID_STR(tarea->resetmsg) )
				DISPOSE( tarea->resetmsg[0] );

			tarea->resetmsg[0] =												fread_string( fp );

			if ( !VALID_STR(tarea->resetmsg[0]) )
			{
				send_log( fp, LOG_FREAD, "fread_area: reset message non acquisito, deve esservene almeno uno." );
				if ( fBootDb )
					exit( EXIT_FAILURE );
				return;
			}
		}
		else if ( !str_cmp(word, "ResetMsg2" ) )
		{
			if ( VALID_STR(tarea->resetmsg) )
				DISPOSE( tarea->resetmsg[1] );

			tarea->resetmsg[1] =												fread_string( fp );
		}
		else if ( !str_cmp(word, "ResetMsg3" ) )
		{
			if ( VALID_STR(tarea->resetmsg) )
				DISPOSE( tarea->resetmsg[2] );

			tarea->resetmsg[2] =												fread_string( fp );
		}
		else if ( !str_cmp(word, "End"		 ) )	break;
		else
			send_log( fp, LOG_FREAD, "fread_area: campo non trovato: %s.", word );
	} /* chiude il for */

	sort_area_by_name( tarea, proto );
}


void load_area_file( AREA_DATA *tarea, MUD_FILE *fp )
{
	if ( !fBootDb && !tarea )
	{
		send_log( NULL, LOG_FREAD, "load_area_file: tarea è NULL." );
		return;
	}

	for ( ; ; )
	{
		char *word;

		if ( fread_letter(fp) != '#' )
		{
			send_log( fp, LOG_FREAD, "load_area_file: # non trovato." );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;
		}

		word = fread_word( fp );

		if ( !str_cmp(word, "AREA") )
		{
			if ( fBootDb )
				tarea = init_area( fp );
			else
				DISPOSE( tarea->name );

			/* L'area viene linkata con le altre solo a fine di questa funzione: */
			fread_area( tarea, fp, FALSE );
		}
		else if ( !str_cmp(word, "END"		  ) )		break;
		else if ( !str_cmp(word, "MOBILES"	  ) )		fread_mobile  ( tarea, fp );
		else if ( !str_cmp(word, "OBJECTS"	  ) )		fread_object  ( tarea, fp, TRUE );
		else if ( !str_cmp(word, "RESETS"	  ) )		fread_reset	  ( tarea, fp );
		else if ( !str_cmp(word, "ROOMS"	  ) )		fread_room	  ( tarea, fp );
		else if ( !str_cmp(word, "SHOPS"	  ) )		load_shops	  ( fp );
		else if ( !str_cmp(word, "REPAIRS"	  ) )		load_repairs  ( fp );
		else if ( !str_cmp(word, "SPECIALS"	  ) )		fread_specials( fp );
		else
		{
			send_log( fp, LOG_FREAD, "load_area_file: Nome di sezione errato." );

			if ( fBootDb )
				exit( EXIT_FAILURE );

			MUD_FCLOSE( fp );
			return;
		}
	} /* chiude il for */

	if ( tarea )
	{
		send_log( NULL, LOG_LOAD, "%-27s Vnum: %5d - %5d",
			tarea->filename, tarea->vnum_low, tarea->vnum_high );

		SET_BIT( tarea->flags, AREAFLAG_LOADED );
	}
	else
	{
		send_log( fp, LOG_FREAD, "load_area_file: l'area di file %s passata è NULL", fp->path );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	MUD_FCLOSE( fp );
}


/*
 * Build list of in_progress areas.  Do not load areas.
 * Define AREA_READ if you want it to build area names rather than reading them out of the area files.
 */
void load_buildlist( void )
{
	DIR			   *dp;
	struct dirent  *dentry;
	AREA_DATA	   *pArea;
	MUD_FILE	   *fp;
	char			buf[MSL];
	char			line[81];
	char			word[81];
	char			temp;
	VNUM			low = 0;
	VNUM			hi = 0;
	bool			badfile = FALSE;

	dp = opendir( ADMIN_DIR );

	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' && str_suffix(".txt", dentry->d_name) )
		{
			fp = mud_fopen( ADMIN_DIR, dentry->d_name, "r", TRUE );
			if ( !fp )
			{
				dentry = readdir( dp );
				continue;
			}

			send_log( NULL, LOG_LOAD, "%s", fp->path );
			badfile = FALSE;

			while ( !feof(fp->file) && !ferror(fp->file) )
			{
				int x1 = 0, x2 = 0;

				word[0] = 0;
				line[0] = 0;

				if ( (temp = fgetc(fp->file)) != EOF )
					ungetc( temp, fp->file );
				else
					break;

				fgets( line, 80, fp->file );
				sscanf( line, "%s %d %d", word, &x1, &x2 );

				if ( !strcmp(word, "Trust") )	/* (RR) Anche nella scrittura del file però deve essere Trust al posto di Level */
				{
					if ( x1 < TRUST_NEOMASTER )
					{
						sprintf( buf, "%s: Admin file with trust %d < %d", dentry->d_name, x1, TRUST_NEOMASTER );
						badfile = TRUE;
					}
				}

				if ( !strcmp(word, "VnumRange") )
				{
					low = x1;
					hi = x2;
				}
			} /* chiude il while */

			MUD_FCLOSE( fp );

			if ( low && hi && !badfile )
			{
				fp = mud_fopen( BUILD_DIR, dentry->d_name, "r", TRUE );
				if ( !fp )
				{
					dentry = readdir( dp );
					continue;
				}

/* Dont always want to read stuff. Aumenterebbe di molto i log */
#ifdef READ_AREA
				strcpy( word, fread_word(fp) );
				if ( word[0] != '#' || strcmp(&word[1], "AREA") )
				{
					send_log( fp, LOG_BUG, "load_buildlist: %s.are: no #AREA found.", dentry->d_name );
					MUD_FCLOSE( fp );
					dentry = readdir( dp );
					continue;
				}
#endif

				CREATE( pArea, AREA_DATA, 1 );
				sprintf( buf, "%s.are", dentry->d_name );
				pArea->author = str_dup( dentry->d_name );
				pArea->filename = str_dup( buf );

#ifdef READ_AREA
				pArea->name = fread_string( fp );
#else
				sprintf( buf, "{PROTO} %s's area in progress", dentry->d_name );
				pArea->name = str_dup( buf );
#endif

				MUD_FCLOSE( fp );
				pArea->vnum_low		= low;
				pArea->vnum_high	= hi;
				pArea->level_low	= -1;
				pArea->level_high	= -1;

				CREATE( pArea->weather, WEATHER_DATA, 1 );
				pArea->weather->temp			= 0;
				pArea->weather->precip			= 0;
				pArea->weather->wind			= 0;
				pArea->weather->temp_vector		= 0;
				pArea->weather->precip_vector	= 0;
				pArea->weather->wind_vector		= 0;
				pArea->weather->climate_temp	= 2;
				pArea->weather->climate_precip	= 2;
				pArea->weather->climate_wind	= 2;
				pArea->weather->first_neighbor	= NULL;
				pArea->weather->last_neighbor	= NULL;

				pArea->first_reset = NULL;
				pArea->last_reset = NULL;
				LINK( pArea, first_build, last_build, next, prev );

				send_log( NULL, LOG_LOAD, "%-25s: Vnum %5d - %-5d\n",
					pArea->filename, pArea->vnum_low, pArea->vnum_high );
			} /* chiude l'if */
		} /* chiude l'if */
		dentry = readdir( dp );
	} /* chiude il while */
	closedir( dp );
}


/*
 * Legge le statistiche del mud da un file
 */
void fread_mudstat( MUD_FILE *fp )
{
	char   *word;

	mudstat.time_max = NULL;
	mudstat.time_max_nomulty = NULL;

	for ( ; ; )
	{
		word = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'					  )										fread_to_eol( fp );
		else if ( !str_cmp(word, "TimeMax"			) )		mudstat.time_max =				fread_string( fp );
		else if ( !str_cmp(word, "AlltimeMax"		) )		mudstat.alltime_max =			fread_positive( fp );
		else if ( !str_cmp(word, "TimeMaxNomulty"	) )		mudstat.time_max_nomulty =		fread_string( fp );
		else if ( !str_cmp(word, "AlltimeMaxNomulty") )		mudstat.alltime_max_nomulty =	fread_positive( fp );
		else if ( !str_cmp(word, "End"				) )		break;
		else
			send_log( fp, LOG_FREAD, "fread_mudstat: nessuna etichetta per %s", word );
	}

	if ( !VALID_STR(mudstat.time_max) )
		mudstat.time_max = str_dup( "(Non Registrato)" );
	if ( !VALID_STR(mudstat.time_max_nomulty) )
		mudstat.time_max = str_dup( "(Non Registrato)" );
}

/*
 * Caricamento delle statistiche del mud
 */
void load_mudstat( void )
{
	mudstat.boot_max_players	= 0;

	fread_section( MUDSTAT_FILE, "MUDSTAT", fread_mudstat, TRUE );
}

/*
 * Scrive le statistiche del mud su file
 */
void save_mudstat( void )
{
	MUD_FILE	*fp;

	fp = mud_fopen( "", MUDSTAT_FILE, "w", TRUE );
	if ( !fp )
		return;

	fprintf( fp->file, "#MUDSTAT\n" );
	fprintf( fp->file, "TimeMax           %s~\n",	mudstat.time_max );
	fprintf( fp->file, "AlltimeMax        %d\n",	mudstat.alltime_max );
	fprintf( fp->file, "TimeMaxNomulty    %s~\n",	mudstat.time_max_nomulty );
	fprintf( fp->file, "AlltimeMaxNomulty %d\n",	mudstat.alltime_max_nomulty );
	fprintf( fp->file, "End\n\n" );

	fprintf( fp->file, "#END\n" );

	MUD_FCLOSE( fp );
}


/*
 * Display vnums currently assigned to areas
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA *ch, VNUM low, VNUM high, bool proto, bool shownl, char *loadst, char *notloadst )
{
	AREA_DATA *pArea;
	AREA_DATA *first_sort;
	int		   count = 0;
	int		   loaded = 0;

	set_char_color( AT_PLAIN, ch );

	if ( proto )
		first_sort = first_build;
	else
		first_sort = first_area;

	for ( pArea = first_sort;  pArea;  pArea = pArea->next_sort )
	{
		if ( HAS_BIT(pArea->flags, AREAFLAG_DELETED) )
			continue;

		if ( pArea->vnum_low < low )
			continue;

		if ( pArea->vnum_high > high )
			break;

		if ( HAS_BIT(pArea->flags, AREAFLAG_LOADED) )
			loaded++;
		else
		{
			if ( !shownl )
				continue;
		}

		pager_printf( ch, "%-15s| Vnums: %5d - %-5d %s\r\n",
			(pArea->filename)  ?  pArea->filename  :  "(invalid)",
			pArea->vnum_low, pArea->vnum_high,
			HAS_BIT(pArea->flags, AREAFLAG_LOADED)  ?  loadst  :  notloadst );

		count++;
	}

	pager_printf( ch, "Areas listed: %d  Loaded: %d\r\n", count, loaded );
}


/*
 * Shows prototype vnums ranges, and if loaded.
 */
DO_RET do_vnums( CHAR_DATA *ch, char *argument )
{
	char	arg[MIL];
	VNUM	low;
	VNUM	high;

	argument = one_argument( argument, arg );

	if ( !VALID_STR(arg) || !VALID_STR(argument)
	  || !is_number(arg) || !is_number(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  vnums <low> <high>\r\n" );

		low	 = 1;
		high = MAX_VNUM;
	}
	else
	{
		low	 = atoi( arg );
		high = atoi( argument );
	}

	show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}


/*
 * Shows installed areas, sorted.
 * Mark unloaded areas with an X
 */
DO_RET do_zones( CHAR_DATA *ch, char *argument )
{
	char	arg[MIL];
	VNUM	low;
	VNUM	high;

	argument = one_argument( argument, arg );

	if ( !VALID_STR(arg) || !VALID_STR(argument)
	  || !is_number(arg) || !is_number(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  zones <low> <high>\r\n" );

		low	 = 1;
		high = MAX_VNUM;
	}
	else
	{
		low	 = atoi( arg );
		high = atoi( argument );
	}

	show_vnums( ch, low, high, FALSE, TRUE, "", " X" );
}


/*
 * Show prototype areas, sorted.
 * Only show loaded areas
 */
DO_RET do_newzones( CHAR_DATA *ch, char *argument )
{
	char	arg[MIL];
	VNUM	low;
	VNUM	high;

	argument = one_argument( argument, arg );

	if ( !VALID_STR(arg) || !VALID_STR(argument)
	  || !is_number(arg) || !is_number(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  newzones <low> <high>\r\n" );

		low	 = 1;
		high = MAX_VNUM;
	}
	else
	{
		low	 = atoi( arg );
		high = atoi( argument );
	}

	show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}


/*
 * Check to make sure range of vnums is free
 * (FF) questo comando lo sposterei in build
 */
DO_RET do_checkvnums( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	char	   arg[MSL];
	VNUM	   low_range;
	VNUM	   high_range;
	bool	   area_conflict;

	argument = one_argument( argument, arg );

	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  checkvnums <lownum> <highvnum>\r\n" );
		return;
	}

	low_range  = atoi( arg );
	high_range = atoi( argument );

	if ( low_range < 1 || low_range >= MAX_VNUM )
	{
		send_to_char( ch, "Invalid argument for bottom of range.\r\n" );
		return;
	}

	if ( high_range < 1 || high_range >= MAX_VNUM )
	{
		send_to_char( ch, "Invalid argument for top of range.\r\n" );
		return;
	}

	if ( high_range < low_range )
	{
		send_to_char( ch, "Bottom of range must be below top of range.\r\n" );
		return;
	}

	set_char_color( AT_PLAIN, ch );

	/* Controlla prima le aree ufficiali */
	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		area_conflict = FALSE;

		if ( HAS_BIT(pArea->flags, AREAFLAG_DELETED) )
			continue;

		if ( low_range < pArea->vnum_low && pArea->vnum_low < high_range )
			area_conflict = TRUE;

		if ( low_range < pArea->vnum_high && pArea->vnum_high < high_range )
			area_conflict = TRUE;

		if ( low_range >= pArea->vnum_low && low_range <= pArea->vnum_high )
			area_conflict = TRUE;

		if ( high_range <= pArea->vnum_high && high_range >= pArea->vnum_low )
			area_conflict = TRUE;

		if ( area_conflict )
		{
			ch_printf( ch, "Conflict:%-20s| %5d - %-5d\r\n",
				(VALID_STR(pArea->filename)) ? name_from_file(pArea->filename) : "(invalid)",
				pArea->vnum_low,
				pArea->vnum_high );
		}
	} /* chiude il for */

	/* Dopo controlla le aree buildate */
	for ( pArea = first_build;  pArea;  pArea = pArea->next )
	{
		area_conflict = FALSE;

		if ( HAS_BIT(pArea->flags, AREAFLAG_DELETED) )
			continue;

		if ( low_range < pArea->vnum_low && pArea->vnum_low < high_range )
			area_conflict = TRUE;

		if ( low_range < pArea->vnum_high && pArea->vnum_high < high_range )
			area_conflict = TRUE;

		if ( low_range >= pArea->vnum_low && low_range <= pArea->vnum_high )
			area_conflict = TRUE;

		if ( high_range <= pArea->vnum_high && high_range >= pArea->vnum_low )
			area_conflict = TRUE;

		if ( area_conflict )
		{
			ch_printf( ch, "Conflict:%-20s| %5d - %-5d\r\n",
				(VALID_STR(pArea->filename)) ? name_from_file(pArea->filename) : "(invalid)",
				pArea->vnum_low,
				pArea->vnum_high );
		}
	} /* chiude il for */
}


/*
 * Big mama top level function.
 */
void load_databases( bool fCopyover )
{
	MUD_FILE   *fp_pid;
	int			pid;
	int			wear;
	int			x;

	send_log( NULL, LOG_LOAD, "--------------------[ Inizio Log di Avvio ]-------------------" );

	/* Salva il pid in un file */
	fp_pid = mud_fopen( SYSTEM_DIR, "mud.pid", "w", TRUE );
	if ( !fp_pid )
		exit( EXIT_FAILURE );
	pid = getpid( );
	fprintf( fp_pid->file, "%d", pid );
	MUD_FCLOSE( fp_pid );

	send_log( NULL, LOG_LOAD, "Controllo delle tabelle dei Codici" );
	check_code_table( );

	send_log( NULL, LOG_LOAD, "Inizializzazione del supporto LibDL." );
	/* Open up a handle to the executable's symbol table for later use
	 * when working with commands */
	sysdata.dlHandle = dlopen( NULL, RTLD_LAZY );
	if ( !sysdata.dlHandle )
	{
		send_log( NULL, LOG_LOAD, "dl: Error opening local system executable as handle, please check compile flags." );
		exit( EXIT_FAILURE );
	}

	fBootDb = TRUE;

	send_log( NULL, LOG_LOAD, "Caricamento delle statistiche del Mud" );
	load_mudstat( );

	send_log( NULL, LOG_LOAD, "Caricamento Comandi" );
	load_commands( );

	top_sn = 0;
	send_log( NULL, LOG_LOAD, "Caricamento degli Incantesimi" );
	load_spells( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Abilità" );
	load_skills( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Armi" );
	load_weapons( );

	sort_skill_table( );
	remap_slot_numbers( );	/* deve rimanere dopo il sort */

	gsn_first_spell  = 0;
	gsn_first_skill  = 0;
	gsn_first_weapon = 0;
	gsn_top_sn		 = top_sn;

	for ( x = 0;  x < top_sn;  x++ )
	{
		if		( !gsn_first_spell  && table_skill[x]->type == SKILLTYPE_SPELL  )	gsn_first_spell  = x;
		else if ( !gsn_first_skill  && table_skill[x]->type == SKILLTYPE_SKILL  )	gsn_first_skill  = x;
		else if ( !gsn_first_weapon && table_skill[x]->type == SKILLTYPE_WEAPON )	gsn_first_weapon = x;
	}

	/* I sociali vanno caricati dopo i comandi e dopo le skill */
	send_log( NULL, LOG_LOAD, "Caricamento Sociali" );
	load_socials( );

	send_log( NULL, LOG_LOAD, "Caricamento delle funzioni speciali" );
	load_specfuns( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Lingue" );
	load_languages( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Erbe" );
	load_herbs( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Razze" );
	load_races( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Classi" );
	load_classes( );

#ifdef T2_QUESTION
	send_log( NULL, LOG_LOAD, "Caricamento delle Domande" );
	load_questions( );
#endif

	send_log( NULL, LOG_LOAD, "Caricamento delle Citazioni" );
	load_quotes( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Animazioni" );
	load_animations( );

	send_log( NULL, LOG_LOAD, "Caricamento dei Sogni" );
	load_dreams( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Liriche" );
	load_lyrics( );

	send_log( NULL, LOG_LOAD, "Caricamente delle Novità" );
	load_news( );

	send_log( NULL, LOG_LOAD, "Caricamento Clan" );
	load_clans( );

	/* I Morphs DEVONO essere caricati dopo aver caricato le classi e le razze */
	send_log( NULL, LOG_LOAD, "Caricamento Morphs" );
	load_morphs( );

	mobs_loaded			= 0;
	objects_loaded		= 0;
	objects_physical	= 0;

	first_shop			= NULL;
	last_shop			= NULL;
	first_repair		= NULL;
	last_repair			= NULL;
	first_teleport		= NULL;
	last_teleport		= NULL;
	extracted_obj_queue	= NULL;
	extracted_char_queue= NULL;

	cur_qobjs			= 0;
	cur_qchars			= 0;
	cur_char			= NULL;
	cur_obj				= 0;
	cur_obj_serial		= 0;
	cur_char_died		= FALSE;
	cur_obj_extracted	= FALSE;
	cur_room			= NULL;
	quitting_char		= NULL;
	loading_char		= NULL;
	saving_char			= NULL;
	last_pkroom			= 1;

	/* Prepara la variabile globale per il salvataggio dell'equipaggiamento */
	for ( wear = 0;  wear < MAX_WEARLOC;  wear++ )
	{
		for ( x = 0;  x < MAX_LAYERS;  x++ )
			save_equipment[wear][x] = NULL;
	}

	/*
	 * Inizializza il generatore di numeri casuali.
	 */
	send_log( NULL, LOG_LOAD, "Inizializzazione del generatore numeri casuali" );
	init_mm( );

	/* Fissa il tempo */
	send_log( NULL, LOG_LOAD, "Carica le informazioni del calendario" );
	load_calendar( );

	/* Initializza le informazioni meteo */
	send_log( NULL, LOG_LOAD, "Carica le informazioni meteo" );
	load_meteo( );

	/*
	 * Assign gsn's for skills which need them.
	 */
	send_log( NULL, LOG_LOAD, "Assegnazione gsn" );

	/* Stili */
	ASSIGN_GSN( gsn_style_evasive,		"evasive style"		);
	ASSIGN_GSN( gsn_style_defensive,	"defensive style"	);
	ASSIGN_GSN( gsn_style_normal,		"normal style"		);
	ASSIGN_GSN( gsn_style_aggressive,	"aggressive style"	);
	ASSIGN_GSN( gsn_style_berserk,		"berserk style"		);

	/* Armamenti */
	ASSIGN_GSN( gsn_pugilism,			"pugilism"			);
	ASSIGN_GSN( gsn_long_blades,		"long blades"		);
	ASSIGN_GSN( gsn_short_blades,		"short blades"		);
	ASSIGN_GSN( gsn_flexible_arms,		"flexible arms"		);
	ASSIGN_GSN( gsn_talonous_arms,		"talonous arms"		);
	ASSIGN_GSN( gsn_bludgeons,			"bludgeons"			);
	ASSIGN_GSN( gsn_missile_weapons,	"missile weapons"	);

	/* Ladro */
	ASSIGN_GSN( gsn_detrap,				"detrap"			);
	ASSIGN_GSN( gsn_backstab,			"backstab"			);
	ASSIGN_GSN( gsn_circle,				"circle"			);
	ASSIGN_GSN( gsn_tumble,				"tumble"			);
	ASSIGN_GSN( gsn_dodge,				"dodge"				);
	ASSIGN_GSN( gsn_hide,				"hide"				);
	ASSIGN_GSN( gsn_peek,				"peek"				);
	ASSIGN_GSN( gsn_pick_lock,			"pick lock"			);
	ASSIGN_GSN( gsn_sneak,				"sneak"				);
	ASSIGN_GSN( gsn_steal,				"steal"				);
	ASSIGN_GSN( gsn_gouge,				"gouge"				);
	ASSIGN_GSN( gsn_poison_weapon,		"poison weapon"		);

	/* Ladro e Guerriero */
	ASSIGN_GSN( gsn_disarm,				"disarm"			);
	ASSIGN_GSN( gsn_enhanced_damage,	"enhanced damage"	);
	ASSIGN_GSN( gsn_kick,				"kick"				);
	ASSIGN_GSN( gsn_parry,				"parry"				);
	ASSIGN_GSN( gsn_rescue,				"rescue"			);
	ASSIGN_GSN( gsn_second_attack,		"second attack"		);
	ASSIGN_GSN( gsn_third_attack,		"third attack"		);
	ASSIGN_GSN( gsn_fourth_attack,		"fourth attack"		);
	ASSIGN_GSN( gsn_fifth_attack,		"fifth attack"		);
	ASSIGN_GSN( gsn_dual_wield,			"dual wield"		);
	ASSIGN_GSN( gsn_punch,				"punch"				);
	ASSIGN_GSN( gsn_bash,				"bash"				);
	ASSIGN_GSN( gsn_stun,				"stun"				);
	ASSIGN_GSN( gsn_bashdoor,			"doorbash"			);
	ASSIGN_GSN( gsn_grip,				"grip"				);
	ASSIGN_GSN( gsn_berserk,			"berserk"			);
	ASSIGN_GSN( gsn_hitall,				"hitall"			);

	/* per vampiri */
	ASSIGN_GSN( gsn_feed,				"feed"				);
	ASSIGN_GSN( gsn_bloodlet,			"bloodlet"			);
	ASSIGN_GSN( gsn_broach,				"broach"			);
	ASSIGN_GSN( gsn_mistwalk,			"mistwalk"			);

	/* Altro */
	ASSIGN_GSN( gsn_aid,				"aid"				);
	ASSIGN_GSN( gsn_track,				"track"				);
	ASSIGN_GSN( gsn_search,				"search"			);
	ASSIGN_GSN( gsn_dig,				"dig"				);
	ASSIGN_GSN( gsn_mount,				"mount"				);
	ASSIGN_GSN( gsn_bite,				"bite"				);
	ASSIGN_GSN( gsn_claw,				"claw"				);
	ASSIGN_GSN( gsn_sting,				"sting"				);
	ASSIGN_GSN( gsn_tail,				"tail"				);
	ASSIGN_GSN( gsn_scribe,				"scribe"			);
	ASSIGN_GSN( gsn_brew,				"brew"				);
	ASSIGN_GSN( gsn_climb,				"climb"				);
	ASSIGN_GSN( gsn_cook,				"cook"				);
	ASSIGN_GSN( gsn_scan,				"scan"				);
	ASSIGN_GSN( gsn_slice,				"slice"				);

	/* Incantesimi */
	ASSIGN_GSN( gsn_fireball,			"fireball"			);
	ASSIGN_GSN( gsn_chill_touch,		"chill touch"		);
	ASSIGN_GSN( gsn_lightning_bolt,		"lightning bolt"	);
	ASSIGN_GSN( gsn_aqua_breath,		"aqua breath"		);
	ASSIGN_GSN( gsn_blindness,			"blindness"			);
	ASSIGN_GSN( gsn_charm_person,		"charm person"		);
	ASSIGN_GSN( gsn_curse,				"curse"				);
	ASSIGN_GSN( gsn_invis,				"invis"				);
	ASSIGN_GSN( gsn_mass_invis,			"mass invis"		);
	ASSIGN_GSN( gsn_poison,				"poison"			);
	ASSIGN_GSN( gsn_sleep,				"sleep"				);
	ASSIGN_GSN( gsn_possess,			"possess"			);

	send_log( NULL, LOG_LOAD, "Caricamento degli Help" );
	load_helps( );

	send_log( NULL, LOG_LOAD, "Caricamento dei Suggerimenti" );
	load_hints( );

	/* Read in all the area files. */
	send_log( NULL, LOG_LOAD, "Caricamento delle Aree" );
	fread_list( AREA_LIST, "AREA_FILE", NULL, TRUE );

	/*
	 * Initialize supermob.
	 *	must be done before update_area()
	 */
	init_supermob( );

	/*
	 * Fix up exits.
	 * Declare db booting over.
	 * Reset all areas once.
	 * Load up the notes file.
	 */
	send_log( NULL, LOG_LOAD, "Preparazione Uscite." );
	fix_exits( );

	send_log( NULL, LOG_LOAD, "Caricamento Offlines" );
	load_offlines( );

	fBootDb = FALSE;

	send_log( NULL, LOG_LOAD, "Inizializzazione Economia." );
	init_economy( );

	send_log( NULL, LOG_LOAD, "Carica le informazioni per le Banche" );
	load_share( );

	send_log( NULL, LOG_LOAD, "Carica gli Account bancari" );
	load_accounts( );

	send_log( NULL, LOG_LOAD, "Ricaricamento Aree." );
	update_area( );

	send_log( NULL, LOG_LOAD, "Caricamento dei Magazzini" );
	load_storages( );

#ifdef T2_LOCKER
	send_log( NULL, LOG_LOAD, "Caricamento dei Locker" );
	load_lockers( );
#endif

#ifdef T2_WEB
	send_log( NULL, LOG_LOAD, "Creazione delle Tabelle Web" );
	load_web_tables( );
#endif

	send_log( NULL, LOG_LOAD, "Caricamento Host Amministratori." );
	load_admhosts( );

	send_log( NULL, LOG_LOAD, "Caricamento Buildlist." );
	load_buildlist( );

	send_log( NULL, LOG_LOAD, "Caricamento Bacheche." );
	load_boards( );

	send_log( NULL, LOG_LOAD, "Caricamento delle Note Private." );
	load_player_notes( );

	send_log( NULL, LOG_LOAD, "Caricamento Sorveglianze." );
	load_watchlist( );

	send_log( NULL, LOG_LOAD, "Caricamento Ban." );
	load_bans( );

	send_log( NULL, LOG_LOAD, "Caricamento Nomi Riservati." );
	load_reserved( );

	send_log( NULL, LOG_LOAD, "Caricamento Cadaveri." );
	load_corpses( );

	MOBtrigger = TRUE;

	/* Recupero dal Copyover */
	if ( fCopyover )
		load_copyover( );

	send_log( NULL, LOG_LOAD, "---------------------[ Fine Log di Avvio ]--------------------" );
/*close_databases( );*/
}


/*
 * Chiude tutti i database, allo shutdown del mud, per meglio controllare i memory leak rimasti
 */
void close_databases( void )
{
	AREA_DATA *pArea;

	/* Chiude tutte le aree e tutte le strutture relative ad esse */
	for ( pArea = first_area;  pArea;  pArea = first_area )
		close_area( pArea );
	if ( top_area != 0 )
		send_log( NULL, LOG_BUG, "close_database: top_area diverso da 0 dopo aver liberato tutte le aree" );

	/* Chiude tutte le aree di building */
	for ( pArea = first_build;  pArea;  pArea = first_build )
		close_area( pArea );

	/* Libera gli adminhost */
	free_all_adminhosts( );

	/* Libera tutte le animazioni */
	free_all_animations( );

	/* Libera tutti i ban */
	free_all_bans( );

	/* Libera tutti gli anniversari del calendario */
	free_all_anniversaries( );

#ifdef T2_CHESS
	free_all_chessboards( );
#endif

	/* Libera tutti i clan */
	free_all_clans( );

	/* Libera tutte le classi */
	free_all_classes( );

	/* Libera tutti i comandi */
	free_all_commands( );

	/* Libera tutti i sogni */
	free_all_dreams( );

	/* Libera tutti gli help e gli hint */
	free_all_helps( );
	free_all_hints( );

	/* Libera tutte le erbe */
	free_all_herbs( );

	/* Libera tutte le lingue */
	free_all_languages( );

	/* Libera tutti gli account bancari */
	free_all_accounts( );

#ifdef T2_LOCKER
	/* Libera tutti i locker */
	free_all_lockers( );
#endif

	/* Libera tutte le liriche */
	free_all_lyrics( );

	/* Libera tutte le novità */
	free_all_news( );

	/* Libera tutte le bacheche e tutte le note dei pg */
	free_all_boards( );
	free_all_player_notes( );

	/* Libera tutti i polymorph */
	free_all_morphs( );

#ifdef T2_QUESTION
	/* Libera tutte le domande */
	free_all_questions( );
	free_all_question_handlers( );
#endif

	/* Libera tutte le citazioni */
	free_all_quotes( );

	/* Libera tutte le razze */
	free_all_races( );

	/* Libera tutti i nomi riservati */
	free_all_reserveds( );

	/* Libera tutti i negozi */
	free_all_shops( );
	free_all_repairs( );

	/* Libera tutte le skell */
	free_all_skills( );

	/* Libera tutti i social */
	free_all_socials( );

	/* Libera tutte le funzioni speciali */
	free_all_specfuns( );

	/* Libera tutti i watch */
	free_all_watchs( );

	/* Libera gli oggetti e i char estratti */
	clean_obj_queue( );
	clean_char_queue( );

	/* (FF) Controllare anche i vari top_object_physical e robe simili, anche per i mob, che si siano azzerati */

#ifdef DEBUG_MEMORYLEAK
	/* Dopo aver chiuso tutto stampa i leak rimasti su file */
	show_leaks( NULL, TRUE );
#endif
}


/*
 * Comando admin, mostra quanti entità vengono caricate.
 * (FF) da fare ancora più dettagliata con altri e info MAX_
 * (FF) fare anche il top_mudprog
 */
DO_RET do_memory( CHAR_DATA *ch, char *argument )
{
	set_char_color( AT_PLAIN, ch );

	send_to_char( ch, "\r\n&CEntità caricate per il Mud:  &w(I valori in byte sono totalmente indicativi)\r\n" );
	ch_printf( ch, "&wAree:        &W%5d &w(&c%7d &wbyte)   &wReset:        &W%5d &w(&c%7d &wbyte)\r\n",		top_area, top_area * sizeof(AREA_DATA), top_reset, top_reset * sizeof(RESET_DATA) );
	ch_printf( ch, "&wIndexMob:    &W%5d &w(&c%7d &wbyte)   &wMobili:       &W%5d &w(&c%7d &wbyte)\r\n",		top_mob_proto, top_mob_proto * sizeof(MOB_PROTO_DATA), mobs_loaded, mobs_loaded * sizeof(CHAR_DATA) );
	ch_printf( ch, "&wIndexOgg:    &W%5d &w(&c%7d &wbyte)   &wLoadedOgg:    &W%5d &w(&c%7d &wbyte)\r\n",		top_obj_proto, top_obj_proto * sizeof(OBJ_PROTO_DATA), objects_loaded, objects_loaded * sizeof(OBJ_DATA) );
	ch_printf( ch, "                                    &wPhisicalOgg:  &W%d5 &w(&c%7d &wbyte)\r\n",			objects_physical, objects_physical * sizeof(OBJ_DATA) );
	ch_printf( ch, "&wCurOq's:     &W%5d &w(&c%7d &wbyte)   &wCurCq's:      &W%5d &w(&c%7d &wbyte)\r\n",		cur_qobjs, cur_qobjs * sizeof(OBJ_DATA), cur_qchars, cur_qchars * sizeof(CHAR_DATA) );
	ch_printf( ch, "&wStanze:      &W%5d &w(&c%7d &wbyte)   &wAffezioni:    &W%5d &w(&c%7d &wbyte)\r\n",		top_room, top_room * sizeof(ROOM_DATA), top_affect, top_affect * sizeof(AFFECT_DATA) );
	ch_printf( ch, "&wExtraDescr:  &W%5d &w(&c%7d &wbyte)   &wUscite:       &W%5d &w(&c%7d &wbyte)\r\n",		top_ed, top_ed * sizeof(EXTRA_DESCR_DATA), top_exit, top_exit * sizeof(EXIT_DATA) );
	ch_printf( ch, "&wNegozi:      &W%5d &w(&c%7d &wbyte)   &wRiparatori:   &W%5d &w(&c%7d &wbyte)\r\n",		top_shop, top_shop * sizeof(SHOP_DATA), top_repair, top_repair * sizeof(REPAIR_DATA) );
	ch_printf( ch, "&wComandi It:  &W%5d &w(&c%7d &wbyte)   &wComandi En:   &W%5d &w(&c%7d &wbyte)\r\n",		top_command_it, top_command_it * sizeof(COMMAND_DATA), top_command_en, top_command_en * sizeof(COMMAND_DATA) );
	ch_printf( ch, "&wComandi Fun: &W%5d &w(&c%7d &wbyte)   &wSociali:      &W%5d &w(&c%7d &wbyte)\r\n",		top_cmdfun, top_cmdfun * sizeof(CMDFUN_DATA), top_social, top_social * sizeof(SOCIAL_DATA) );
	ch_printf( ch, "&wSkell:       &W%5d &w(&c%7d &wbyte)   &wLingue:       &W%5d &w(&c%7d &wbyte)\r\n",		top_sn, top_sn * sizeof(SKILL_DATA), MAX_LANGUAGE, MAX_LANGUAGE * sizeof(LANG_DATA) );
	ch_printf( ch, "&wErbe:        &W%5d &w(&c%7d &wbyte)\r\n",													MAX_HERB, MAX_HERB * sizeof(SKILL_DATA) );	/* (FF) in futuro HERB_DATA */
	ch_printf( ch, "&wClassi:      &W%5d &w(&c%7d &wbyte)   &wClassiGiocab: &W%5d &w(&c%7d &wbyte)\r\n",		MAX_CLASS, MAX_CLASS * sizeof(CLASS_DATA), top_class_play, top_class_play * sizeof(CLASS_DATA) );
	ch_printf( ch, "&wRazze:       &W%5d &w(&c%7d &wbyte)   &wRazzeGiocab:  &W%5d &w(&c%7d &wbyte)\r\n\r\n",	MAX_RACE, MAX_RACE * sizeof(RACE_DATA), top_race_play, top_race_play * sizeof(RACE_DATA) );
	ch_printf( ch, "&wAiuti:       &W%5d &w(&c%7d &wbyte)   &wSuggerimenti: &W%5d &w(&c%7d &wbyte)\r\n",		top_help, top_help * sizeof(HELP_DATA), top_hint, top_hint * sizeof(HINT_DATA) );
	ch_printf( ch, "&wAdminHost:   &W%5d &w(&c%7d &wbyte)\r\n",		top_adminhost, get_sizeof_adminhost() );
	ch_printf( ch, "&wAnimazioni:  &W%5d &w(&c%7d &wbyte)\r\n",		top_animation, get_sizeof_animation() );
	ch_printf( ch, "&wArchetipi:   &W%5d &w(&c%7d &wbyte)\r\n",		MAX_ARCHETYPE, get_sizeof_archetype() );
	ch_printf( ch, "&wBacheche:    &W%5d &w(&c%7d &wbyte)\r\n",		top_board, top_board * sizeof(BOARD_DATA) );
#ifdef T2_QUESTION
	ch_printf( ch, "&wDomande:     &W%5d &w(&c%7d &wbyte)\r\n",		top_question, get_sizeof_question() );
#endif
	ch_printf( ch, "&wCitazioni:   &W%5d &w(&c%7d &wbyte)\r\n",		top_quote, top_quote * sizeof(QUOTE_DATA) );
	ch_printf( ch, "&wSogni:       &W%5d &w(&c%7d &wbyte)\r\n",		top_dream, top_dream * sizeof(DREAM_DATA) );
#ifdef T2_LOCKER
	ch_printf( ch, "&wArmadietti:  &W%5d &w(&c%7d &wbyte)\r\n",		top_locker, top_locker * sizeof(LOCKER_DATA) );
#endif

	ch_printf( ch, "&wPotion Val : &W%-16d\r\n",	mudstat.upotion_val );
	ch_printf( ch, "&wScribe/Brew: &W%d/%d\r\n",	mudstat.used_scribed, mudstat.used_brewed );
	ch_printf( ch, "&wPill Val   : &W%-16d\r\n",	mudstat.upill_val );
	ch_printf( ch, "&wGlobal loot: &W%d\r\n\r\n",	mudstat.global_looted );

	ch_printf( ch, "&wPg totale:           &W%5d &w(&c%7d &wbyte)\r\n", top_offline, top_offline * sizeof(CHAR_DATA) );
	ch_printf( ch, "&wPg online:           &W%5d &w(&c%7d &wbyte)\r\n", num_descriptors, num_descriptors * sizeof(CHAR_DATA) );
	ch_printf( ch, "&wPgMaxToday:          &W%5d\r\n",					mudstat.boot_max_players );
	ch_printf( ch, "&wPgMaxEver:           &W%5d\r\n",					mudstat.alltime_max );
	ch_printf( ch, "&wPgMaxEverDate:       &W %s\r\n\r\n",				mudstat.time_max );
	ch_printf( ch, "&wPgMaxEverNoMulty:    &W%5d\r\n",					mudstat.alltime_max_nomulty );
	ch_printf( ch, "&wPgMaxEverDateNoMulty:&W %s\r\n\r\n",				mudstat.time_max_nomulty );

	ch_printf( ch, "&wOutput inviato in questo boot:           %10d", mudstat.total_output );
#ifdef T2_MCCP
	ch_printf( ch, "&wOutput compresso inviato in questo boot: %10d", mudstat.total_output_mccp );
	ch_printf( ch, "&wOutput totale in questo boot:            %10d", mudstat.total_output + mudstat.total_output_mccp );
#endif

	ch_printf( ch, "PID %d\r\n", getpid() );
	send_command( ch, "version", CO );
}
