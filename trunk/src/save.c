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


/*****************************************************************************\
 >				Modulo di salvataggio e caricamento del PG					 <
\*****************************************************************************/


#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mud.h"
#include "affect.h"
#include "alias.h"
#include "build.h"
#include "clan.h"
#include "db.h"
#include "dream.h"
#include "fread.h"
#include "herb.h"
#include "mprog.h"
#include "nanny.h"
#include "morph.h"
#include "room.h"
#include "save.h"
#include "timer.h"
#include "values.h"


/*
 * Array to keep track of equipment temporarily
 */
OBJ_DATA   *save_equipment[MAX_WEARLOC][MAX_LAYERS];
CHAR_DATA  *quitting_char;
CHAR_DATA  *loading_char;
CHAR_DATA  *saving_char;

int			file_ver;

#define		VERSION_PLAYER 1


/*
 * Array of containers read for proper re-nesting of objects
 */
static OBJ_DATA *rgObjNest[MAX_NEST];


/*
 * Comando Save
 */
DO_RET do_save( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
		return;

	if ( HAS_BIT(ch->in_room->area->flags, AREAFLAG_NOSAVE) )
	{
		send_to_char( ch, "Non posso ricordarmi e tornare viaggiando nei Piani in questo luogo.\r\n" );
		return;
	}

	if ( get_level(ch) < sysdata.save_level )
	{
		send_to_char( ch, "&BAffinché il mio nome venga scritto nella memoria di questo mondo dovrò fare un po' di più.\r\n" );
		return;
	}

	update_aris( ch );		/* aggiorna gli affect e i RIS del pg */
	save_player( ch );
	saving_char = NULL;

	send_to_char( ch, "Ok, mi ricorderò di questo posto.\r\n" );
}


/*
 * Un-equip character before saving to ensure proper
 *	stats are saved in case of changes to or removal of EQ.
 */
void de_equip_char( CHAR_DATA *ch )
{
	OBJ_DATA *obj;
	int		  x;
	int		  y;

	for ( x = 0;  x < MAX_WEARLOC;  x++ )
	{
		for ( y = 0;  y < MAX_LAYERS;  y++ )
			save_equipment[x][y] = NULL;
	}

	for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
	{
		if ( obj->wear_loc > -1 && obj->wear_loc < MAX_WEARLOC )
		{
		    if ( get_level(ch) >= obj->level )
		    {
				for ( x = 0;  x < MAX_LAYERS;  x++ )
				{
					if ( save_equipment[obj->wear_loc][x] == NULL )
					{
						save_equipment[obj->wear_loc][x] = obj;
						break;
					}
				}

				if ( x == MAX_LAYERS )
				{
					send_log( NULL, LOG_SAVE, "%s had on more than %d layers of clothing in one location (%d): %s",
						ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
				}
			}
			else
			{
				send_log( NULL, LOG_SAVE, "%s had on %s: get_level(ch) = %d  obj->level = %d",
					ch->name, obj->name, get_level(ch), obj->level );
			}

			unequip_char( ch, obj );
		}
	}
}


/*
 * Re-equip character
 */
void re_equip_char( CHAR_DATA *ch )
{
	int		x;
	int		y;

	for ( x = 0;  x < MAX_WEARLOC;  x++ )
	{
		for ( y = 0;  y < MAX_LAYERS;  y++ )
		{
			if ( save_equipment[x][y] == NULL )
				continue;	/* (TT) era break ma non mi torna */

			equip_char( ch, save_equipment[x][y], x );
			save_equipment[x][y] = NULL;
		}
	}
}


/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, MUD_FILE *fp )
{
	AFFECT_DATA *paf;
	ALIAS_DATA	*pal;
	SKILL_DATA	*skill = NULL;
	int			 x;
	int			 pos;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "fwrite_char: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_BUG, "fwrite_char: ch è un mob" );
		return;
	}

	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fwrite_char: fp è NULL" );
		return;
	}

	fprintf( fp->file, "#PLAYER\n" );

	for ( paf = ch->first_affect;  paf;  paf = paf->next )
	{
		if ( paf->type >= 0 && (skill = get_skilltype(paf->type)) == NULL )
			continue;

		if ( paf->type >= 0 && paf->type < TYPE_RACIAL )
		{
			fprintf( fp->file, "AffectData	  '%s' %3d %3d %s %s~\n",
				skill->name,
				paf->duration,
				paf->modifier,
				code_str(fp, paf->location, CODE_APPLY),
				code_bit(fp, paf->bitvector, CODE_AFFECT) );
		}
		else
		{
			fprintf( fp->file, "Affect       %3d %3d %3d %s %s~\n",
				paf->type,
				paf->duration,
				paf->modifier,
				code_str(fp, paf->location, CODE_APPLY),
				code_bit(fp, paf->bitvector, CODE_AFFECT) );
		}
	}
	if ( !IS_EMPTY(ch->affected_by) )
		fprintf( fp->file, "AffectedBy   %s~\n", code_bit(fp, ch->affected_by, CODE_AFFECT) );
	if ( !IS_EMPTY(ch->no_affected_by) )
		fprintf( fp->file, "NoAffectedBy %s~\n", code_bit(fp, ch->no_affected_by, CODE_AFFECT) );

	for ( pal = ch->pg->first_alias;  pal;  pal = pal->next )
	{
		if ( !VALID_STR(pal->name) )	continue;
		if ( !VALID_STR(pal->cmd) )		continue;

		fprintf( fp->file, "Alias        %s~ %s~\n", pal->name, pal->cmd );
	}

	fprintf( fp->file, "Alignment    %d\n", ch->alignment );
	fprintf( fp->file, "Archetype    %d\n", ch->pg->archetype );
	fprintf( fp->file, "Armor        %d\n", ch->armor );

	/* Stampa i valori degli attributi modulari */
	fputs( "*           ", fp->file );
	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		fprintf( fp->file, " %s", code_str(fp, x, CODE_ATTR) );
		if ( (x+1) % 3 == 0 )
			fputs( " ", fp->file );
	}
	fputs( "\n", fp->file );
	fputs( "AttrMod     ", fp->file );
	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		fprintf( fp->file, " %3d", ch->attr_mod[x] );
		if ( (x+1) % 3 == 0 )
			fputs( " ", fp->file );
	}
	fputs( "\n", fp->file );

	/* Stampa i valori degli attributi permanenti */
	fputs( "*           ", fp->file );
	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		fprintf( fp->file, " %s", code_str(fp, x, CODE_ATTR) );
		if ( (x+1) % 3 == 0 )
			fputs( " ", fp->file );
	}
	fputs( "\n", fp->file );
	fputs( "AttrPerm    ", fp->file );
	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		fprintf( fp->file, " %3d", ch->attr_perm[x] );
		if ( (x+1) % 3 == 0 )
			fputs( " ", fp->file );
	}
	fputs( "\n", fp->file );

	if ( VALID_STR(ch->pg->bio) )
		fprintf( fp->file, "Bio          %s~\n", ch->pg->bio );
	if ( VALID_STR(ch->pg->personality) )
		fprintf( fp->file, "Character    %s~\n", ch->pg->personality );
	fprintf( fp->file, "Class        %s\n", code_str(fp, ch->class, CODE_CLASS) );

	if ( ch->pg->clan && ch->pg->member )
		fprintf( fp->file, "Clan      %s~\n", ch->pg->clan->name );

	/* Save color values - Samson 9-29-98 */
	fprintf( fp->file, "MaxColors    %d\n", MAX_AT_COLORS );
	fprintf( fp->file, "Colors      " );
	for ( x = 0; x < MAX_AT_COLORS ; x++ )
		fprintf( fp->file, " %d", ch->pg->colors[x] );
	fprintf( fp->file, "\n" );

	/* Stampa i valori della condizione */
	fputs( "Condition  ", fp->file );
	for ( x = 0;  x < MAX_CONDITION;  x++ )
		fprintf( fp->file, "  %s %d", code_str(fp, x, CODE_CONDITION), ch->pg->condition[x] );
	fputs( "\n", fp->file );

	fprintf( fp->file, "CreationDate %d\n",			(int) ch->pg->creation_date );
	fprintf( fp->file, "DamRoll      %d\n",			ch->damroll );
	if ( !IS_EMPTY(ch->pg->deaf) )
		fprintf( fp->file, "Deaf         %s~\n",	code_bit(fp, ch->pg->deaf, CODE_CHANNEL) );
	if ( VALID_STR(ch->description) )
		fprintf( fp->file, "Description  %s~\n",	ch->description	);
	if ( ch->pg->dream )
	{
		fprintf( fp->file, "Dream        %s\n",		ch->pg->dream->name );
		fprintf( fp->file, "DreamPart    %d\n",		ch->pg->dream_part );
	}
	fprintf( fp->file, "Exp          %d\n",		ch->exp );
	if ( ch->pg->eye )
	{
		fprintf( fp->file, "Eye\n" );
		fwrite_eye( ch->pg->eye, fp );
	}
	else
	{
		send_log( fp, LOG_SAVE, "fwrite_char: struttura occhio nulla!" );
	}

#ifdef T2_ALFA
	if ( !IS_EMPTY(ch->enflags) )
		fprintf( fp->file, "EnFlags      %s~\n",		code_bit(fp, ch->enflags, CODE_ENFLAG) );
#endif
	if ( !IS_EMPTY(ch->pg->flags) )
	{
		/* Prima di scrivere le flag per sicurezza toglie quelle con trust superiore a quella del pg */
		for ( x = 0;  x < MAX_PLAYER;  x++ )
		{
			if ( table_player[x].trust <= ch->pg->trust )
				continue;
			REMOVE_BIT( ch->pg->flags, x );
		}
		fprintf( fp->file, "Flags        %s~\n",		code_bit(fp, ch->pg->flags, CODE_PLAYER) );
	}
	fprintf( fp->file, "Glory        %d\n",			ch->pg->glory );
	fprintf( fp->file, "Gold         %d\n",			ch->gold );

	if ( ch->pg->hair )
	{
		fprintf( fp->file, "Hair\n" );
		fwrite_hair( ch->pg->hair, fp );
	}
	else
	{
		send_log( fp, LOG_SAVE, "fwrite_char: struttura capelli nulla!" );
	}

	fprintf( fp->file, "Height       %d\n",			ch->height );
	if ( ch->pg->release_date )
		fprintf( fp->file, "Helled       %d %s~\n", (int) ch->pg->release_date, ch->pg->helled_by );
	fprintf( fp->file, "Hitroll      %d\n",			ch->hitroll );

	if ( fBootDb && VALID_STR(ch->desc->host_old) )	/* il check fBootDb serve per le conversioni dei file save tramite load_offlines */
		fprintf( fp->file, "Host         %s\n",		ch->desc->host_old );
	else
		fprintf( fp->file, "Host         %s\n",		get_host(ch) );

	fprintf( fp->file, "IllegalPK    %d\n",			ch->pg->illegal_pk );
	if ( !IS_EMPTY(ch->immune) )
		fprintf( fp->file, "Immune       %s~\n",	code_bit(fp, ch->immune, CODE_RIS) );
	if ( !IS_EMPTY(ch->no_immune) )
		fprintf( fp->file, "NoImmune     %s~\n",	code_bit(fp, ch->immune, CODE_RIS) );
	/* (FF) Cambiarlo da vnum a razze (o aggiungere la info), così da poter ricavare eventuali title ch tipo: Ammazza-Troll */
	for ( x = 0;  x < MAX_KILLTRACK;  x++ )
	{
		if ( ch->pg->killed[x].vnum == 0 )
			break;

		fprintf( fp->file, "Killed       %d %d\n",	ch->pg->killed[x].vnum, ch->pg->killed[x].count );
	}
	fprintf( fp->file, "Speaking     %s\n",			code_str(fp, ch->speaking, CODE_LANGUAGE) );
	fprintf( fp->file, "Level        %d\n",			ch->level );
	if ( VALID_STR(ch->long_descr) )
		fprintf( fp->file, "LongDescr    %s\n", ch->long_descr );
	if ( ch->mental_state != -10 )
		fprintf( fp->file, "MentalState	 %d\n",		ch->mental_state	);
	fprintf( fp->file, "MDeaths      %d\n",			ch->pg->mdeaths	);
	fprintf( fp->file, "MKills       %d\n",			ch->pg->mkills	);

	if ( VALID_STR(ch->pg->msg_login_you) )
		fprintf( fp->file, "MsgLoginYou   %s~\n",	ch->pg->msg_login_you );
	if ( VALID_STR(ch->pg->msg_login_room) )
		fprintf( fp->file, "MsgLoginRoom  %s~\n",	ch->pg->msg_login_room );
	if ( VALID_STR(ch->pg->msg_logout_you) )
		fprintf( fp->file, "MsgLogoutYou  %s~\n",	ch->pg->msg_logout_you );
	if ( VALID_STR(ch->pg->msg_logout_room) )
		fprintf( fp->file, "MsgLogoutRoom %s~\n",	ch->pg->msg_logout_room );

	if ( VALID_STR(ch->pg->msg_slay_victim) )
		fprintf( fp->file, "MsgSlayVictim %s~\n",	ch->pg->msg_slay_victim );
	if ( VALID_STR(ch->pg->msg_slay_room) )
		fprintf( fp->file, "MsgSlayRoom %s~\n",		ch->pg->msg_slay_room );

	if ( ch->pg->min_snoop != TRUST_PLAYER )
		fprintf( fp->file, "MinSnoop     %s\n",		code_str(fp, ch->pg->min_snoop, CODE_TRUST) );
	fprintf( fp->file, "MobInvis     %d\n",			ch->mobinvis );
	fprintf( fp->file, "Name         %s~\n",		ch->name );
#ifdef T2_SAVECONVERT
	fprintf( fp->file, "Logon        %d\n",			(int) ch->pg->logon_old );	/* serve per mantenere la vecchia data di login alla riscrittura del file (BB) ma merda non va'... */
#else
	fprintf( fp->file, "Logon        %d\n",			(int) ch->pg->logon );
#endif
	if ( VALID_STR(ch->pg->objective) )
		fprintf( fp->file, "Objective    %s~\n",		ch->pg->objective );
	if ( ch->pg && ch->pg->pagerlen != DEF_LEN_PAGER )
		fprintf( fp->file, "Pagerlen     %d\n",		ch->pg->pagerlen );
	fprintf( fp->file, "Password     %s~\n",		ch->pg->pwd );
	fprintf( fp->file, "PDeaths      %d\n",			ch->pg->pdeaths	);
	fprintf( fp->file, "PKills       %d\n",			ch->pg->pkills	);
	fprintf( fp->file, "Played       %d\n",			ch->pg->played + (int) (current_time - ch->pg->logon) );

	/* Stampa i valori dei punti attuali. */
	fputs( "Points     ", fp->file );
	for ( x = 0;  x < MAX_POINTS;  x++ )
		fprintf( fp->file, "  %s %5d", code_str(fp, x, CODE_POINTS), ch->points[x] );
	fputs( "\n", fp->file );

	/* Stampa i valori dei punti massimi. */
	fputs( "PointsMax  ", fp->file );
	for ( x = 0;  x < MAX_POINTS;  x++ )
		fprintf( fp->file, "  %s %5d", code_str(fp, x, CODE_POINTS), ch->points_max[x] );
	fputs( "\n", fp->file );

	/* Toglie le posizioni di combattimento. */
	pos = ch->position;
	if ( pos == POSITION_BERSERK
	  || pos == POSITION_AGGRESSIVE
	  || pos == POSITION_FIGHT
	  || pos == POSITION_DEFENSIVE
	  || pos == POSITION_EVASIVE	)
	{
		pos = POSITION_STAND;
	}
	fprintf( fp->file, "Position     %s\n",			code_str(fp, pos, CODE_POSITION) );

	fprintf( fp->file, "Practice     %d\n",			ch->pg->practice );
	if ( VALID_STR(ch->pg->prompt) )
		fprintf( fp->file, "Prompt       %s~\n",	ch->pg->prompt );
	if ( VALID_STR(ch->pg->fprompt) )
		fprintf( fp->file, "FPrompt      %s~\n",	ch->pg->fprompt	);
	fprintf( fp->file, "Protocol         %s~\n",	code_bit(fp, ch->pg->protocols, CODE_PROTOCOL) );	/* (FF) bisognerebbe aggiungere un check che tolga le flag per soli admin in fase di saving ai pg con trust player */
	fprintf( fp->file, "QuestGlory   %d\n",			ch->pg->glory_accum );
	fprintf( fp->file, "Race         %s\n",			code_str(fp, ch->race, CODE_RACE) );
#ifdef T2_WEB
	if ( ch->pg->report != 0 )
		fprintf( fp->file, "Report       %d\n",		ch->pg->report	);
#endif
	if ( !IS_EMPTY(ch->resistant) )
		fprintf( fp->file, "Resistant    %s~\n",	code_bit(fp, ch->resistant, CODE_RIS) );
	if ( !IS_EMPTY(ch->no_resistant) )
		fprintf( fp->file, "NoResistant	 %s~\n",	code_bit(fp, ch->no_resistant, CODE_RIS) );
	fprintf( fp->file, "RightHand    %s\n",			print_bool(ch->right_hand) );

	if ( !ch->in_room )
		ch->in_room = get_room_index( NULL, VNUM_ROOM_TEMPLE );
	if ( ch->in_room == get_room_index(NULL, VNUM_ROOM_LIMBO) && ch->was_in_room )
		fprintf( fp->file, "Room         %d\n", ch->was_in_room->vnum );
	else
		fprintf( fp->file, "Room         %d\n", ch->in_room->vnum );

	/* Stampa i valori dei tiri di salvataggio */
	fputs( "SavingThrow", fp->file );
	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		fprintf( fp->file, "  %s %d", code_str(fp, x, CODE_SAVETHROW), ch->saving_throw[x] );
	fputs( "\n", fp->file );

	/* Stampa i valori dei sensi modulari */
	fputs( "SenseMod   ", fp->file );
	for ( x = 0;  x < MAX_SENSE;  x++ )
		fprintf( fp->file, "  %s %d", code_str(fp, x, CODE_SENSE), ch->sense_mod[x] );
	fputs( "\n", fp->file );

	/* Stampa i valori dei sensi permanenti */
	fputs( "SensePerm  ", fp->file );
	for ( x = 0;  x < MAX_SENSE;  x++ )
		fprintf( fp->file, "  %s %d", code_str(fp, x, CODE_SENSE), ch->sense_perm[x] );
	fputs( "\n", fp->file );

	fprintf( fp->file, "Sex          %s\n",			code_str(fp, ch->sex, CODE_SEX) );
	if ( VALID_STR(ch->short_descr) )
		fprintf( fp->file, "ShortDescr   %s",		ch->short_descr );
	fprintf( fp->file, "Style        %s\n",			code_str(fp, ch->style, CODE_STYLE) );
	if ( VALID_STR(ch->pg->surname) )
		fprintf( fp->file, "Surname      %s~\n",	ch->pg->surname );
	if ( !IS_EMPTY(ch->susceptible) )
		fprintf( fp->file, "Suscept	     %s~\n",	code_bit(fp, ch->susceptible, CODE_RIS) );
	if ( !IS_EMPTY(ch->no_susceptible) )
		fprintf( fp->file, "NoSuscept    %s~\n",	code_bit(fp, ch->no_susceptible, CODE_RIS) );
	if ( VALID_STR(ch->pg->title) )
		fprintf( fp->file, "Title        %s~\n",	ch->pg->title );
	if ( get_timer(ch, TIMER_KILLED) > 0 )
		fprintf( fp->file, "TimerKilled  %d\n",		get_timer(ch, TIMER_KILLED) );
	if ( get_timer(ch, TIMER_PKILLED) > 0 )
		fprintf( fp->file, "TimerPKilled %d\n",		get_timer(ch, TIMER_PKILLED) );
	fprintf( fp->file, "Trust        %s\n",			code_str(fp, get_trust(ch), CODE_TRUST) );
	fprintf( fp->file, "Version      %d\n",			VERSION_PLAYER );
	fprintf( fp->file, "Weight       %.3f\n",			((float) ch->weight) / 1000 );	/* (TT) */
	if ( ch->wimpy != 0 )
		fprintf( fp->file, "Wimpy        %d\n",		ch->wimpy );
	if ( ch->pg->zodiac != -1 )
		fprintf( fp->file, "Zodiac       %d\n",		ch->pg->zodiac );

	if ( ch->pg->trust >= TRUST_NEOMASTER )	/* non utilizza IS_ADMIN altrimenti con la flag PLAYER_NOADMIN attiva salta questa parte */
	{
		fprintf( fp->file, "Incognito    %d\n",		ch->pg->incognito );

		if ( ch->pg->vnum_range_low && ch->pg->vnum_range_high )
			fprintf( fp->file, "VnumRange    %d %d\n", ch->pg->vnum_range_low, ch->pg->vnum_range_high );

		if ( !IS_EMPTY( ch->pg->msglog) )
		{
			/* Rimuove le flag di msglog che non si dovrebbero avere */
			for ( x = 0;  x < MAX_LOG;  x++ )
			{
				if ( table_log[x].trust <= ch->pg->trust )
					continue;
				REMOVE_BIT( ch->pg->msglog, x );
			}
			fprintf( fp->file, "MsgLog       %s~\n", code_bit(fp, ch->pg->msglog, CODE_LOG) );
		}
	}

	/* (FF) appena capita cambiare i nomi da Skill a LearnSkill per esempio */
	for ( x = 1;  x < top_sn;  x++ )
	{
		if ( VALID_STR(table_skill[x]->name) && ch->pg->learned_skell[x] > 0 )
		{
			switch ( table_skill[x]->type )
			{
			  default:
				fprintf( fp->file, "Skill        %d '%s'\n",
					ch->pg->learned_skell[x], table_skill[x]->name );
				break;
			  case SKILLTYPE_SPELL:
				fprintf( fp->file, "Spell        %d '%s'\n",
					ch->pg->learned_skell[x], table_skill[x]->name );
				break;
			  case SKILLTYPE_WEAPON:
				fprintf( fp->file, "Weapon       %d '%s'\n",
					ch->pg->learned_skell[x], table_skill[x]->name );
				break;
			}
		}
	}

	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
	{
		if ( ch->pg->learned_lang[x] > 0 )
		{
			fprintf( fp->file, "LearnedLang  %s %d\n",
				code_str(fp, x, CODE_LANGUAGE),
				ch->pg->learned_lang[x] );
		}
	}

	fprintf( fp->file, "End  * Plr\n\n" );
}


/*
 * Write an object and its contents
 */
void fwrite_object( CHAR_DATA *ch, OBJ_DATA *obj, MUD_FILE *fp, int iNest, int os_type )
{
	EXTRA_DESCR_DATA *ed_proto;		/* extra descr di un oggetto prototipo */
	EXTRA_DESCR_DATA *ed;			/* extra descr di un oggetto */
	AFFECT_DATA		 *paf_proto;	/* affect di un oggetto prototipo */
	AFFECT_DATA		 *paf;			/* affect di un oggetto */
	MPROG_DATA		 *mprog_proto;	/* mud progs di un oggetto prototipo */
	MPROG_DATA		 *mprog;		/* mud progs di un oggetto */
	int				  wear;
	int				  wear_loc;
	int				  x;
	bool			  owrite;

	/*
	 * Slick recursion to write lists backwards,
	 *  so loading them will load in forwards order
	 */
	if ( obj->prev_content && os_type == OS_CARRY )
		fwrite_object( ch, obj->prev_content, fp, iNest, OS_CARRY );

	owrite = TRUE;

	if ( iNest >= MAX_NEST )
	{
		send_log( NULL, LOG_SAVE, "fwrite_object: iNest hit MAX_NEST %d per l'oggetto %s (#%d) al pg %s",
			iNest, obj->name, obj->vnum, (ch) ? ch->name : "sconosciuto" );
		owrite = FALSE;
	}

	/*
	 * Castrate storage characters
	 * (FF) Verrà tolto perché il futuro sistema di livelli oggetto non avrà bisogno di questo check
	 */
	if ( ch && obj->level > get_level(ch) && os_type == OS_CARRY )
		owrite = FALSE;

	/* Non salva le chiavi che il pg ha con sé */
	if ( obj->type == OBJTYPE_KEY && !HAS_BIT(obj->extra_flags, OBJFLAG_CLANOBJECT) )
		owrite = FALSE;

	/* Catch deleted objects */
	if ( obj_extracted(obj) )
	{
		send_log( NULL, LOG_SAVE, "fwrite_object: oggetto %s (#%d) estratto",
			obj->short_descr, obj->vnum );
		owrite = FALSE;
	}

	/* Munch magic flagged containers for now  */
	if ( obj->type == OBJTYPE_CONTAINER && HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
		TOGGLE_BIT( obj->extra_flags, OBJFLAG_MAGIC );

	if ( owrite )
	{
		switch ( os_type )
		{
		  case OS_CARRY:	fputs( "#OBJECT\n", fp->file );		break;
		  case OS_CORPSE:	fputs( "#CORPSE\n", fp->file );		break;	/* Corpse saving */
		};

		fprintf( fp->file, "Vnum         %d\n",		obj->vnum );
		if ( iNest != 0 )
			fprintf( fp->file, "Nest         %d\n",		iNest );
		if ( obj->count > 1 )
			fprintf( fp->file, "Count        %d\n",		obj->count );
		if ( obj->prototype || str_cmp(obj->name, obj->pObjProto->name) )
			fprintf( fp->file, "Name         %s~\n",	obj->name );
		if ( obj->prototype || str_cmp(obj->short_descr, obj->pObjProto->short_descr) )
			fprintf( fp->file, "ShortDescr   %s~\n",	obj->short_descr );
		if ( obj->prototype || str_cmp(obj->long_descr, obj->pObjProto->long_descr) )
			fprintf( fp->file, "Description  %s~\n",	obj->long_descr );
		if ( obj->prototype || str_cmp(obj->action_descr, obj->pObjProto->action_descr) )
			fprintf( fp->file, "ActionDesc   %s~\n",	obj->action_descr );
		if ( os_type == OS_CORPSE && obj->in_room )
			fprintf( fp->file, "Room         %d\n",		obj->in_room->vnum );
		if ( obj->prototype || !SAME_BITS(obj->extra_flags, obj->pObjProto->extra_flags) )
			fprintf( fp->file, "ExtraFlags   %s~\n",	code_bit(fp, obj->extra_flags, CODE_OBJFLAG) );
		if ( obj->prototype || !SAME_BITS(obj->wear_flags, obj->pObjProto->wear_flags) )
			fprintf( fp->file, "WearFlags    %s~\n",	code_bit(fp,obj->wear_flags, CODE_OBJWEAR) );

		wear_loc = -1;
		for ( wear = 0;  wear < MAX_WEARLOC;  wear++ )
		{
			for ( x = 0;  x < MAX_LAYERS;  x++ )
			{
				if ( save_equipment[wear][x] == NULL )
					break;

				if ( obj == save_equipment[wear][x] )
				{
					wear_loc = wear;
					break;
				}
			}
		}

		if ( wear_loc != -1 )
			fprintf( fp->file, "WearLoc	  %d\n",	wear_loc );
		if ( obj->prototype || obj->type != obj->pObjProto->type )
			fprintf( fp->file, "ItemType	 %d\n",	obj->type );
		if ( obj->prototype || obj->weight != obj->pObjProto->weight )
			fprintf( fp->file, "Weight	     %.3f\n", ((float) obj->weight) / 1000 );
		if ( obj->level != 0 )
			fprintf( fp->file, "Level		 %d\n",	obj->level );
		if ( obj->timer != 0 )
			fprintf( fp->file, "Timer		 %d\n",	obj->timer );
		if ( obj->prototype || obj->cost != obj->pObjProto->cost )
			fprintf( fp->file, "Cost		 %d\n",	obj->cost );

		fwrite_values( fp, obj );

		/* Salva gli affect da rimuovere dall'oggetto non prototipo confrontandolo con il suo prototipo */
		if ( !obj->prototype )
		{
			for ( paf_proto = obj->pObjProto->first_affect;  paf_proto;  paf_proto = paf_proto->next )
			{
				bool found = FALSE;

				for ( paf = obj->first_affect;  paf;  paf = paf->next )
				{
					if ( is_same_affect(paf_proto, paf, TRUE) )
						found = TRUE;
				}

				if ( !found )
				{
					if ( paf_proto->type < 0 || paf_proto->type >= top_sn )
						fprintf( fp->file, "NoAffect  %d", paf_proto->type );
					else
						fprintf( fp->file, "NoAffectData '%s'", table_skill[paf_proto->type]->name );
	
					fprintf( fp->file, " %d %d %d %d\n",
						paf_proto->duration,
						( (paf_proto->location == APPLY_WEAPONSPELL
						|| paf_proto->location == APPLY_WEARSPELL
						|| paf_proto->location == APPLY_REMOVESPELL
						|| paf_proto->location == APPLY_STRIPSN
						|| paf_proto->location == APPLY_RECURRINGSPELL)
						&& is_valid_sn(paf_proto->modifier))  ?  table_skill[paf_proto->modifier]->slot  :  paf_proto->modifier,
						paf_proto->location,
						print_bitvector(paf_proto->bitvector) );
				}
			}
		}

		/* Ora invece confronta gli affect modificati o nuovi dell'oggetto e li salva.
		 *	Se invece l'oggetto è un prototipo salva tutti gli affect */
		for ( paf = obj->first_affect;  paf;  paf = paf->next )
		{
			bool found = FALSE;

			if ( !obj->prototype )
			{
				for ( paf_proto = obj->pObjProto->first_affect;  paf_proto;  paf_proto = paf_proto->next )
				{
					if ( is_same_affect(paf_proto, paf, TRUE) )
						found = TRUE;
				}
			}

			if ( obj->prototype || !found )
			{
				if ( paf->type < 0 || paf->type >= top_sn )
					fprintf( fp->file, "Affect  %d", paf->type );
				else
					fprintf( fp->file, "AffectData '%s'", table_skill[paf->type]->name );

				fprintf( fp->file, " %d %d %d %d\n",
					paf->duration,
					( (paf->location == APPLY_WEAPONSPELL
					|| paf->location == APPLY_WEARSPELL
					|| paf->location == APPLY_REMOVESPELL
					|| paf->location == APPLY_STRIPSN
					|| paf->location == APPLY_RECURRINGSPELL)
					&& is_valid_sn(paf->modifier))  ?  table_skill[paf->modifier]->slot  :  paf->modifier,
					paf->location,
					print_bitvector(paf->bitvector) );
			}
		}

		/* Salva prima le extra che devono essere tolte, sulla base del prototipo,
		 *	una volta che viene ricaricato l'oggetto */
		if ( !obj->prototype )
		{
			for ( ed_proto = obj->pObjProto->first_extradescr;  ed_proto;  ed_proto = ed_proto->next )
			{
				bool found = FALSE;
	
				for ( ed = obj->first_extradescr;  ed;  ed = ed->next )
				{
					if ( !str_cmp(ed_proto->keyword, ed->keyword) && !str_cmp(ed_proto->description, ed->description) )
						found = TRUE;
				}
	
				if ( found == FALSE )
					fprintf( fp->file, "NoExtraDescr %s~\n", ed_proto->keyword );
			}
		}

		/* Al contrario ora cerca tra le extra dell'oggetto non prototipo per cercare quelle che deve salvare perché nuove o modificate;
		 *	a meno che non sia un oggetto prototipo in questo caso le salva tutte */
		for ( ed = obj->first_extradescr;  ed;  ed = ed->next )
		{
			bool found = FALSE;
	
			if ( !obj->prototype )
			{
				for ( ed_proto = obj->pObjProto->first_extradescr;  ed_proto;  ed_proto = ed_proto->next )
				{
					if ( !str_cmp(ed->keyword, ed_proto->keyword) && !str_cmp(ed->description, ed_proto->description) )
						found = TRUE;
				}
			}

			if ( obj->prototype || found == FALSE )
				fprintf( fp->file, "ExtraDescr   %s~\n%s~\n", ed->keyword, ed->description );
		}

		/* Cerca tra i mudprog del prototipo per segnarsi quelli da togliere modificati o inesistenti */
		if ( !obj->prototype )
		{
			for ( mprog_proto = obj->pObjProto->first_mudprog;  mprog_proto;  mprog_proto = mprog_proto->next )
			{
				bool found = FALSE;
	
				for ( mprog = obj->first_mudprog;  mprog;  mprog = mprog->next )
				{
					if ( is_same_mudprog(mprog_proto, mprog) )
						found = TRUE;
				}

				if ( !found )
				{
					fprintf( fp->file, "NoMudProg %s %s~\n",
						code_name(fp, mprog_proto->type, CODE_MPTRIGGER),
						mprog_proto->arglist );
				}
			}
		}

		/* Cerca ora invece tra i mudprog dell'oggetto e si salva se non esistenti o modificati rispetto al suo prototipo.
		 *	Oppure li salva tutti se si tratta di un oggetto prototipo. */
		for ( mprog = obj->first_mudprog;  mprog;  mprog = mprog->next )
		{
			bool found = FALSE;
	
			if ( !obj->prototype )
			{
				for ( mprog_proto = obj->pObjProto->first_mudprog;  mprog_proto;  mprog_proto = mprog_proto->next )
				{
					if ( is_same_mudprog(mprog_proto, mprog) )
						found = TRUE;
				}
			}

			if ( obj->prototype || found == FALSE )
			{
				fputs( "MudProg ", fp->file );
				fwrite_mudprog( fp, mprog );
			}
		}

		fprintf( fp->file, "End  * Obj\n\n" );
	}

	/* Salva il contenuto dell'oggetto, se ne ha, aumentando il nesting */
	if ( obj->first_content )
		fwrite_object( ch, obj->last_content, fp, iNest+1, OS_CARRY );
}


/*
 * This will write one mobile structure pointed to be fp.
 */
void fwrite_mobile( MUD_FILE *fp, CHAR_DATA *mob )
{
	if ( !fp )
	{
		send_log( NULL, LOG_FWRITE, "fwrite_mobile: fp passato è NULL" );
		return;
	}

	if ( !mob )
	{
		send_log( NULL, LOG_FWRITE, "fwrite_mobile: mob passato è NULL" );
		return;
	}

	if ( IS_PG(mob) )
	{
		send_log( NULL, LOG_FWRITE, "fwrite_mobile: mob passato è un pg: %s", mob->name );
		return;
	}

	fprintf( fp->file, "#MOBILE\n" );
	fprintf( fp->file, "Vnum	    %d\n", mob->pIndexData->vnum );

	if ( mob->in_room )
	{
		fprintf( fp->file, "Room        %d\n",
			(mob->in_room == get_room_index(fp, VNUM_ROOM_LIMBO) && mob->was_in_room)  ?
			mob->was_in_room->vnum  :  mob->in_room->vnum );
	}
	if ( str_cmp(mob->name, mob->pIndexData->name) )
		fprintf( fp->file, "Name	    %s~\n", mob->name );
	if ( str_cmp(mob->short_descr, mob->pIndexData->short_descr) )
		fprintf( fp->file, "Short	    %s~\n", mob->short_descr );
	if ( str_cmp(mob->long_descr, mob->pIndexData->long_descr) )
		fprintf( fp->file, "Long	    %s~\n", mob->long_descr );
	if ( str_cmp(mob->description, mob->pIndexData->description) )
		fprintf( fp->file, "Description %s~\n", mob->description );

	fprintf( fp->file, "Position    %s\n",	code_str(fp, mob->position, CODE_POSITION) );
#ifdef T2_ALFA
	if ( !IS_EMPTY(mon->enflags) && !SAME_BITS(mob->enflags, mob->pIndexData->enflags) )
		fprintf( fp->file, "EnFlags     %s~\n",	code_bit(fp, mob->enflags, CODE_ENFLAG) );
#endif
	REMOVE_BIT( mob->act, MOB_MOUNTED );	/* Così permette di riutilizzare i mascotte mounted al login */
	if ( !IS_EMPTY(mob->act) && !SAME_BITS(mob->act, mob->pIndexData->act) )
		fprintf( fp->file, "Flags       %s~\n",	code_bit(fp, mob->act, CODE_MOB) );

/* Might need these later
	de_equip_char( mob );
	re_equip_char( mob );
*/
	if ( mob->first_carrying )
		fwrite_object( mob, mob->last_carrying, fp, 0, OS_CARRY );

	fprintf( fp->file, "End  * Mob\n" );
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *  some of the infrastructure is provided.
 */
void save_player( CHAR_DATA *ch )
{
	MUD_FILE   *fp;
	char		strsave[MIL];
	char		strback[MIL];
	char		strtemp[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_SAVE, "save_player: ch è NULL." );
		return;
	}

	if ( IS_MOB(ch) || get_level(ch) < sysdata.save_level )
		return;

	/* Se si trova in un'area in cui non si può salvare */
	if ( ch->in_room && ch->in_room->area && HAS_BIT(ch->in_room->area->flags, AREAFLAG_NOSAVE) )
		return;

	saving_char = ch;

	if ( ch->desc && ch->desc->original )
		ch = ch->desc->original;

	de_equip_char( ch );

	ch->pg->save_time = current_time;
	sprintf( strsave, "%s%s", PLAYER_DIR, ch->name );

	/* Backup of each pfile on quitting */
	if ( quitting_char == ch )
	{
		sprintf( strback, "%s%s", BACKUP_DIR, ch->name );
		if ( rename(strsave, strback) != 0 )
		{
			send_log( NULL, LOG_SAVE, "save_player: errore nella rinominazione da %s a %s: %s",
				strsave, strback, strerror(errno) );
		}
	}

	/*
	 * Save admin stats, level & vnums for do_vnums command
	 */
	if ( IS_ADMIN(ch) )
	{
		fp = mud_fopen( ADMIN_DIR, ch->name, "w", TRUE );
		if ( !fp )
			return;

		fprintf( fp->file, "Trust        %s\n", code_str(fp, ch->pg->trust, CODE_TRUST) );

		if ( ch->pg->vnum_range_low && ch->pg->vnum_range_high )
			fprintf( fp->file, "VnumRange    %d %d\n", ch->pg->vnum_range_low, ch->pg->vnum_range_high );

		MUD_FCLOSE( fp );
	}

	sprintf( strtemp, "%s%s.tmp", PLAYER_DIR, ch->name );
	fp = mud_fopen( "", strtemp, "w", TRUE );
	if ( fp )
	{
		bool ferr;

		fwrite_char( ch, fp );

		if ( ch->morph )
			fwrite_morph_data( ch, fp );

		if ( ch->first_carrying )
			fwrite_object( ch, ch->last_carrying, fp, 0, OS_CARRY );

		if ( sysdata.mascotte_save && ch->pg->mascotte )
			fwrite_mobile( fp, ch->pg->mascotte );

		fprintf( fp->file, "#END\n" );
		ferr = ferror( fp->file );

		MUD_FCLOSE( fp );

		if ( ferr )
		{
			send_log( NULL, LOG_SAVE, "save_player: error writing temp file for %s (errno: %s) -- not copying",
				strsave, strerror(errno) );
		}
		else
		{
			if ( rename(strtemp, strsave) != 0 )
			{
				send_log( NULL, LOG_SAVE, "save_player: errore nella rinominazione da %s a %s: %s",
					strtemp, strsave, strerror(errno) );
			}
		}
	}

	if ( quitting_char != ch )
	re_equip_char( ch );

	quitting_char = NULL;
	saving_char   = NULL;
}


/*
 * Inizializza le stringhe non acquisite della struttura del pg
 */
void init_pg_strings( PG_DATA *pg )
{
	if ( !pg->pwd )				pg->pwd				= str_dup( "" );
	if ( !pg->surname )			pg->surname			= str_dup( "" );
	if ( !pg->title )			pg->title			= str_dup( "" );
	if ( !pg->bio )				pg->bio				= str_dup( "" );
	if ( !pg->afk_message )		pg->afk_message		= str_dup( "" );
	if ( !pg->msg_login_you )	pg->msg_login_you	= str_dup( "" );
	if ( !pg->msg_login_room )	pg->msg_login_room	= str_dup( "" );
	if ( !pg->msg_logout_you )	pg->msg_logout_you	= str_dup( "" );
	if ( !pg->msg_logout_room )	pg->msg_logout_room	= str_dup( "" );
	if ( !pg->objective )		pg->objective		= str_dup( "" );
	if ( !pg->music_file )		pg->music_file		= str_dup( "" );
	if ( !pg->msg_slay_victim )	pg->msg_slay_victim	= str_dup( "" );
	if ( !pg->msg_slay_room )	pg->msg_slay_room	= str_dup( "" );
	if ( !pg->personality )		pg->personality		= str_dup( "" );
	if ( !pg->helled_by )		pg->helled_by		= str_dup( "" );
	if ( !pg->prompt )			pg->prompt			= str_dup( "" );
	if ( !pg->fprompt )			pg->fprompt			= str_dup( "" );
	if ( !pg->subprompt )		pg->subprompt		= str_dup( "" );
}


/*
 * Inizializza una nuova struttura PG_DATA.
 */
void init_pg( PG_DATA *pg )
{
	int x;

	/* Valori plr di default */
	pg->flags			= multimeb( PLAYER_BLANK, PLAYER_COMBINE, PLAYER_DOUBLELANG, PLAYER_PROMPT, PLAYER_COLOR, PLAYER_INTRO, PLAYER_AUTOEXIT, PLAYER_HINT, PLAYER_MAPPER, -1 );

#ifdef T2_MSP
	pg->protocols		= multimeb( PROTOCOL_MSP, PROTOCOL_MUSIC, PROTOCOL_SOUND, PROTOCOL_COMBAT, -1 );
#else
#endif
#ifdef T2_MCCP
	SET_BIT( pg->protocols, PROTOCOL_MCCP );
#endif
#ifdef T2_MXP
	SET_BIT( pg->protocols, PROTOCOL_MXP );
#endif

	pg->condition[CONDITION_THIRST]		=  48;
	pg->condition[CONDITION_FULL]		=  48;
	pg->condition[CONDITION_BLOODTHIRST] =  10;

	pg->pagerlen					= -1;	/* disattivato */
	pg->incognito					=  0;
	pg->charmies					=  0;
	for ( x = 0;  x < MAX_SKILL;  x++ )
		pg->learned_skell[x] = 0;
	for ( x = 0;  x < MAX_LANGUAGE;  x++ )
		pg->learned_lang[x] = 0;
	pg->release_date	= 0;
	pg->helled_by		= NULL;
	pg->pagerlen		= DEF_LEN_PAGER;

	pg->hair			= init_hair( pg );
	pg->eye				= init_eye( pg );

	pg->logon			= current_time;
	pg->practice		= 0;
	pg->archetype		= -1;		/* Nessuno */
	pg->counter_quit	= 0;		/* Contatore alla rovescia per il quit */
	pg->dream			= NULL;
	pg->dream_part		= -1;
#ifdef T2_WEB
	pg->report			= 0;
#endif
	pg->first_alias		= NULL;
	pg->last_alias		= NULL;
	pg->host			= NULL;
}


/*
 * Read in a char.
 */
void fread_player( CHAR_DATA *ch, MUD_FILE *fp, bool preload )
{
	char   *word;
	int		x;
	int		killcnt;
	int		max_colors = 0; /* Color code */
	file_ver = 0;
	killcnt  = 0;

	/* Setup color values in case player has none set - Samson */
	reset_colors( ch );

	for ( ; ; )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_BUG, "fread_player: fine del file prematura nella lettura di un file save" );
			break;
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Affect") || !str_cmp(word, "AffectData") )
		{
			AFFECT_DATA *paf;

			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}

			CREATE( paf, AFFECT_DATA, 1 );

			if ( !str_cmp(word, "Affect") )
				paf->type	= fread_number( fp );
			else
			{
				int sn;
				char *sname = fread_word( fp );

				if ( (sn = skill_lookup(sname)) < 0 )
				{
					if ( (sn = herb_lookup(sname)) < 0 )
						send_log( fp, LOG_SAVE, "fread_player: unknown skill." );
					else
						sn += TYPE_HERB;
				}
				paf->type = sn;
			}

			paf->duration =													fread_number( fp );
			paf->modifier =													fread_number( fp );
			paf->location =													code_code( fp, fread_word(fp), CODE_APPLY );

			if ( paf->location == APPLY_WEAPONSPELL
			  || paf->location == APPLY_WEARSPELL
			  || paf->location == APPLY_REMOVESPELL
			  || paf->location == APPLY_STRIPSN
			  || paf->location == APPLY_RECURRINGSPELL )
			{
				paf->modifier = slot_lookup( paf->modifier );
			}

			paf->bitvector	=												fread_code_bit( fp, CODE_AFFECT );
			LINK( paf, ch->first_affect, ch->last_affect, next, prev );
		}
		else if ( !str_cmp(word, "AffectedBy"	) )		ch->affected_by =		fread_code_bit( fp, CODE_AFFECT );
		else if ( !str_cmp(word, "NoAffectedBy"	) )		ch->no_affected_by =	fread_old_bitvector( fp, CODE_AFFECT );
		else if ( !str_cmp(word, "Alias"		) )
		{
			ALIAS_DATA *pal;

			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}

			CREATE( pal, ALIAS_DATA, 1 );

			pal->name	= fread_string( fp );
			pal->cmd	= fread_string( fp );

			LINK( pal, ch->pg->first_alias, ch->pg->last_alias, next, prev );
		}
		else if ( !str_cmp(word, "Alignment"	) )		ch->alignment =			fread_number( fp );
		else if ( !str_cmp(word, "Armor"		) )		ch->armor =				fread_number( fp );
		else if ( !str_cmp(word, "Archetype"	) )		ch->pg->archetype =		fread_number( fp );
		else if ( !str_cmp(word, "AttrMod"		) )
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
				ch->attr_mod[x] =												fread_number( fp );
		}
		else if ( !str_cmp(word, "AttrPerm"	) )
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
				ch->attr_perm[x] =												fread_number( fp );
			/* check che serve a castrare soprattutto gli admin che si comportano troppo da pwp */
			while ( check_total_attr(ch) <= 0 )
			{
				int num;
	
				num = number_range( 0, MAX_ATTR );
				if ( ch->attr_perm[num] <= 15 )
					continue;
				ch->attr_perm[num]--;
			}
		}
		else if ( !str_cmp(word, "Bio"			) )		ch->pg->bio =			fread_string( fp );
		else if ( !str_cmp(word, "Character"	) )		ch->pg->personality =		fread_string( fp );
		else if ( !str_cmp(word, "Clan"			) )	/* Temporary measure */
		{
			char	*temp;

			temp =																fread_string( fp );
			ch->pg->clan = get_clan( temp, TRUE );
			if ( !preload && VALID_STR(temp) && !ch->pg->clan )
				ch_printf( ch, "Il Clan %s a cui ero legato ora non esiste più.\r\n", temp );
		}
		else if ( !str_cmp(word, "Class"		) )		ch->class =				code_code(fp, fread_word(fp), CODE_CLASS );
		else if ( !str_cmp(word, "MaxColors"	) )		max_colors =			fread_number( fp );
		else if ( !str_cmp(word, "Colors"		) )
		{
			for ( x = 0; x < max_colors; x++ )
				ch->pg->colors[x] =													fread_number( fp );
		}
		else if ( !str_cmp(word, "Condition") )
		{
			for ( x = 0;  x < 4;  x++ )
			{
				fread_word( fp );
				ch->pg->condition[x] =											fread_number( fp );
			}
		}
		else if ( !str_cmp(word, "CreationDate" ) )		ch->pg->creation_date =	fread_number( fp );
		else if ( !str_cmp(word, "DamRoll"		) )		ch->damroll =			fread_number( fp );
		else if ( !str_cmp(word, "Deaf"			) )		ch->pg->deaf =			fread_code_bit( fp, CODE_CHANNEL );
		else if ( !str_cmp(word, "Description"	) )		ch->description =		fread_string( fp );
		else if ( !str_cmp(word, "Dream"		) )		ch->pg->dream =			get_dream( fread_word(fp), TRUE );
		else if ( !str_cmp(word, "DreamPart"	) )		ch->pg->dream_part =	fread_number( fp );
		else if ( !str_cmp(word, "End"			) )
		{
			if ( !preload )
			{
				if ( !ch->desc || !ch->desc->host_old )
				{
					send_log( fp, LOG_SAVE, "fread_player: Host non acquisito." );
					if ( ch->desc )
					{
						DISPOSE( ch->desc->host_old );
						ch->desc->host_old = str_dup( "<non si sa>" );
					}
				}

				if ( ch->pg->logon_old == 0 )
					send_log( fp, LOG_SAVE, "fread_player: Logon non acquisito." );
			}

			/* Ricava le informazioni del membro dal clan del pg */
			if ( ch->pg->clan )
			{
				ch->pg->member = get_clan_member( ch->name, ch->pg->clan, TRUE );
				if ( !ch->pg->member )
					ch_printf( ch, "Non faccio più parte del Clan %s.\r\n", ch->pg->clan->name );
			}

			init_pg_strings( ch->pg );

			if ( IS_ADMIN(ch) )
			{
				/* Carica il pg Admin imponendo la visibilità nel who, per renderli partecipi alla vita del mud */
				ch->pg->incognito = 0;
				if ( HAS_BIT_PLR(ch, PLAYER_INVISWHO) )
					REMOVE_BIT( ch->pg->flags, PLAYER_INVISWHO );
				ch->pg->incognito = get_level( ch );
				if ( !HAS_BIT_PLR(ch, PLAYER_INVISWHO) )
					SET_BIT( ch->pg->flags, PLAYER_INVISWHO );
				assign_area( ch );
			}

			return;
		}
		else if ( !str_cmp(word, "Eye"			) )		ch->pg->eye	=			fread_eye( fp );
		else if ( !str_cmp(word, "Exp"			) )		ch->exp =				fread_long( fp );
#ifdef T2_ALFA
		else if ( !str_cmp(word, "EnFlags"		) )		ch->enflags =			fread_code_bit( fp, CODE_ENFLAG );
#endif
		else if ( !str_cmp(word, "Flags"		) )		ch->pg->flags =			fread_code_bit( fp, CODE_PLAYER );
		else if ( !str_cmp(word, "Glory"		) )		ch->pg->glory =			fread_number( fp );
		else if ( !str_cmp(word, "Gold"			) )		ch->gold =				fread_number( fp );
		else if ( !str_cmp(word, "Hair"			) )		ch->pg->hair =			fread_hair( fp );
		else if ( !str_cmp(word, "Height"		) )		ch->height =			fread_number( fp );
		else if ( !str_cmp(word, "Helled"		) )
		{
			ch->pg->release_date =												fread_number( fp );
			ch->pg->helled_by	 =												fread_string( fp );
		}
		else if ( !str_cmp(word, "HitRoll"		) )		ch->hitroll =			fread_number( fp );
		else if ( !str_cmp(word, "Host"			) )
		{
			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}
			else
			{
				ch->desc->host_old =									str_dup( fread_word(fp) );
			}
		}
		else if ( !str_cmp(word, "Incognito"	 ) )	ch->pg->incognito =		fread_number( fp );
		else if ( !str_cmp(word, "IllegalPK"	 ) )	ch->pg->illegal_pk =	fread_number( fp );
		else if ( !str_cmp(word, "Immune"		 ) )	ch->immune =			fread_code_bit( fp, CODE_RIS );
		else if ( !str_cmp(word, "MsgLog"		 ) )	ch->pg->msglog =		fread_code_bit( fp, CODE_LOG );
		else if ( !str_cmp(word, "NoImmune"		 ) )	ch->no_immune =			convert_bitvector( fread_number(fp) );
		else if ( !str_cmp(word, "Killed"		 ) )
		{
			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}

			if ( killcnt >= MAX_KILLTRACK )
			{
				send_log( fp, LOG_SAVE, "fread_player: killcnt è maggiore o uguale a MAX_KILLTRACK, %d >= %d", killcnt, MAX_KILLTRACK );
			}
			else
			{
				ch->pg->killed[killcnt].vnum	=									fread_number( fp );
				ch->pg->killed[killcnt++].count =									fread_number( fp );
			}
		}
		else if ( !str_cmp(word, "Speaking"		 ) )	ch->speaking =				code_code( fp, fread_word(fp), CODE_LANGUAGE );
		else if ( !str_cmp(word, "Level"		 ) )	ch->level =					fread_number( fp );
		else if ( !str_cmp(word, "Logon"		 ) )	ch->pg->logon_old =			fread_number( fp );
		else if ( !str_cmp(word, "LongDescr"	 ) )	ch->long_descr =			fread_string( fp );
		else if ( !str_cmp(word, "Mentalstate"	 ) )	ch->mental_state =			fread_number( fp );
		else if ( !str_cmp(word, "MDeaths"		 ) )	ch->pg->mdeaths =			fread_number( fp );
		else if ( !str_cmp(word, "MKills"		 ) )	ch->pg->mkills =			fread_number( fp );
		else if ( !str_cmp(word, "MsgLoginYou"   ) )	ch->pg->msg_login_you =		fread_string( fp );
		else if ( !str_cmp(word, "MsgLoginRoom"  ) )	ch->pg->msg_login_room =	fread_string( fp );
		else if ( !str_cmp(word, "MsgLogoutYou"  ) )	ch->pg->msg_logout_you =	fread_string( fp );
		else if ( !str_cmp(word, "MsgLogoutRoom" ) )	ch->pg->msg_logout_room =	fread_string( fp );
		else if ( !str_cmp(word, "MsgSlayRoom"	 ) )	ch->pg->msg_slay_room =		fread_string( fp );
		else if ( !str_cmp(word, "MsgSlayVictim" ) )	ch->pg->msg_slay_victim =	fread_string( fp );
		else if ( !str_cmp(word, "MsgLogoutRoom" ) )	ch->pg->msg_slay_room =		fread_string( fp );
		else if ( !str_cmp(word, "MinSnoop"		 ) )	ch->pg->min_snoop =			fread_number( fp );
		else if ( !str_cmp(word, "MobInvis"		 ) )	ch->mobinvis =				fread_number( fp );
		else if ( !str_cmp(word, "Name"			 ) )
		{
			char	*name;
			name = fread_string( fp );
			if ( str_cmp(name, ch->name) )
				send_log( NULL, LOG_BUG, "fread_player: nome inviato(%s) diverso da quello letto(%s)", name, ch->name );
		}
		else if ( !str_cmp(word, "Pagerlen"		 ) )	ch->pg->pagerlen =			fread_number( fp );
		else if ( !str_cmp(word, "Password"		 ) )	ch->pg->pwd =				fread_string( fp );
		else if ( !str_cmp(word, "PDeaths" 		 ) )	ch->pg->pdeaths =			fread_number( fp );
		else if ( !str_cmp(word, "PKills"		 ) )	ch->pg->pkills =			fread_number( fp );
		else if ( !str_cmp(word, "Played"		 ) )	ch->pg->played =			fread_number( fp );
		else if ( !str_cmp(word, "Points"		 ) )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
			{
				fread_word( fp );
				ch->points[x] =													fread_number( fp );
			}
		}
		else if ( !str_cmp(word, "PointsMax"	) )
		{
			for ( x = 0;  x < MAX_POINTS;  x++ )
			{
				fread_word( fp );
				ch->points_max[x] =												fread_number( fp );
			}
			/* check per castra admin pwp e pg che sfruttano o hanno bachi */
			while ( check_total_points(ch) <= 0 )
			{
				int num;
	
				num = number_range( 0, MAX_POINTS );
				if ( ch->points_max[num] <= 100 )
					continue;
				ch->points_max[num]--;
			}
		}
		else if ( !str_cmp(word, "Position"		) )		ch->position =			code_code( fp, fread_word(fp), CODE_POSITION );
		else if ( !str_cmp(word, "Practice"		) )		ch->pg->practice =		fread_number( fp );
		else if ( !str_cmp(word, "Prompt"		) )		ch->pg->prompt =		fread_string( fp );
		else if ( !str_cmp(word, "FPrompt"		) )		ch->pg->fprompt =		fread_string( fp );
		else if ( !str_cmp(word, "Objective"	) )		ch->pg->objective =		fread_string( fp );
		else if ( !str_cmp(word, "Protocol"		) )		ch->pg->protocols =		fread_code_bit( fp, CODE_PROTOCOL );
		else if ( !str_cmp(word, "QuestGlory"	) )		ch->pg->glory_accum=fread_number( fp );
		else if ( !str_cmp(word, "Race"			) )		ch->race =				code_code( fp, fread_word(fp), CODE_RACE );
#ifdef T2_WEB
		else if ( !str_cmp(word, "Report"		) )		ch->pg->report =		fread_number( fp );
#endif
		else if ( !str_cmp(word, "Resistant"	) )		ch->resistant =			fread_code_bit( fp, CODE_RIS );
		else if ( !str_cmp(word, "NoResistant"	) ) 	ch->no_resistant =		fread_code_bit( fp, CODE_RIS );
		else if ( !str_cmp(word, "RightHand"	) )		ch->right_hand =		fread_bool( fp );
		else if ( !str_cmp(word, "Room"			) )
		{
			ch->in_room = get_room_index( fp, fread_number(fp) );
			if ( !ch->in_room )
				ch->in_room = get_room_index( fp, VNUM_ROOM_LIMBO );
		}
		else if ( !str_cmp(word, "SavingThrow"	) )
		{
			for ( x = 0;  x < MAX_SAVETHROW;  x++ )
			{
				fread_word( fp );
				ch->saving_throw[x] =											fread_number( fp );
			}
		}
		else if ( !str_cmp(word, "SenseMod"	) )
		{
			/* Associo dei valori fittizzi nel caso non riesca a leggerli dal file */
			for ( x = 0;  x < MAX_SENSE;  x++ )
			{
				fread_word( fp );
				ch->sense_mod[SENSE_SIGHT] =											fread_number( fp );
			}
		}
		else if ( !str_cmp(word, "SensePerm"	) )
		{
			/* Associo dei valori fittizzi nel caso non riesca a leggerli dal file */
			for ( x = 0;  x < MAX_SENSE;  x++ )
			{
				fread_word( fp );
				ch->sense_perm[x] =												fread_number( fp );
			}
			while ( check_total_sense(ch) <= 0 )
			{
				int num;
	
				num = number_range( 0, MAX_SENSE );
				if ( ch->sense_perm[num] <= 15 )
					continue;
				ch->sense_perm[num]--;
			}
		}
		else if ( !str_cmp(word, "Sex"			) )		ch->sex =				code_code( fp, fread_word(fp), CODE_SEX );
		else if ( !str_cmp(word, "ShortDescr"	) )		ch->short_descr =		fread_string( fp );
		else if ( !str_cmp(word, "Style"		) )		ch->style =				code_code( fp, fread_word(fp), CODE_STYLE );
		else if ( !str_cmp(word, "Surname"		) )		ch->pg->surname =		fread_string( fp );
		else if ( !str_cmp(word, "Suscept"		) )		ch->susceptible =		fread_code_bit( fp, CODE_RIS );
		else if ( !str_cmp(word, "NoSuscept"	) )		ch->no_susceptible =	fread_code_bit( fp, CODE_RIS );
		else if ( !str_cmp(word, "Skill"		) )
		{
			int sn;
			int value;

			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}

			value = fread_number( fp );

			sn = bsearch_skill_exact( fread_word(fp), gsn_first_skill, gsn_first_weapon-1 );
			if ( sn < 0 )
				send_log( fp, LOG_SAVE, "fread_player: unknown skill." );
			else
				ch->pg->learned_skell[sn] = value;
		} /* chiude l'if skill */
		else if ( !str_cmp(word, "Spell"		) )
		{
			int		sn;
			int		value;

			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}

			value = fread_number( fp );
			sn = bsearch_skill_exact( fread_word(fp), gsn_first_spell, gsn_first_skill-1 );
			if ( sn < 0 )
				send_log( fp, LOG_SAVE, "fread_player: unknown spell." );
			else
				ch->pg->learned_skell[sn] = value;
		} /* chiude l'if spell */
		else if ( !str_cmp(word, "LearnedLang"		) )
		{
			int		lang;
			char	*wlang;

			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}

			wlang = fread_word( fp );
			lang = code_code( fp, wlang, CODE_LANGUAGE );
			if ( lang < 0 || lang >= MAX_LANGUAGE )
			{
				send_log( fp, LOG_SAVE, "fread_player: lingua sconosciuta %s per pg %s",
					wlang, ch->name );
			}
			else
				ch->pg->learned_lang[lang] = fread_number( fp );

		} /* chiude l'if Tongue */
		else if ( !str_cmp(word, "Weapon"		) )
		{
			int sn;
			int value;

			if ( preload )
			{
				fread_to_eol( fp );
				continue;
			}

			value = fread_number( fp );

			sn = bsearch_skill_exact( fread_word(fp), gsn_first_weapon, gsn_top_sn-1 );
			if ( sn < 0 )
				send_log( fp, LOG_SAVE, "fread_player: unknown weapon." );
			else
				ch->pg->learned_skell[sn] = value;
		} /* chiude l'if weapon */
		else if ( !str_cmp(word, "TimerKilled"	) )		add_timer( ch , TIMER_KILLED, fread_number(fp), NULL, 0 );
		else if ( !str_cmp(word, "TimerPKilled"	) )		add_timer( ch , TIMER_PKILLED, fread_number(fp), NULL, 0 );
		else if ( !str_cmp(word, "Title"		) )
		{
			ch->pg->title =		fread_string( fp );
			if ( !isspace(ch->pg->title[0]) )
			{
				char	buf[MIL];

				sprintf( buf, " %s", ch->pg->title );
				DISPOSE( ch->pg->title );
				ch->pg->title = str_dup( buf );
			}
		}
		else if ( !str_cmp(word, "Trust"		) )		ch->pg->trust = code_code( fp, fread_word(fp), CODE_TRUST );
		else if ( !str_cmp(word, "VnumRange"	) )
		{
			ch->pg->vnum_range_low	=											fread_number( fp );
			ch->pg->vnum_range_high	=											fread_number( fp );
		}
		else if ( !str_cmp(word, "Version"		) )		file_ver =				fread_number( fp );
		else if ( !str_cmp(word, "Weight"		) )		ch->weight =			fread_float( fp ) * 1000;
		else if ( !str_cmp(word, "Wimpy"		) )		ch->wimpy =				fread_number( fp );
		else if ( !str_cmp(word, "Zodiac"		) )		ch->pg->zodiac =		fread_number( fp );
		else
			send_log( fp, LOG_SAVE, "fread_player: etichetta non trovata: %s", word );
	} /* chiude il for */
}


void fread_object_save( CHAR_DATA *ch, MUD_FILE *fp, int os_type )
{
	ROOM_DATA	*room = NULL;
	OBJ_DATA	*obj;
	char		*word;
	int			 iNest;
	int			 x;
	bool		 fNest;
	bool		 fVnum;

	if ( ch )
		room = ch->in_room;

	CREATE( obj, OBJ_DATA, 1 );
	obj->count		=  1;
	obj->wear_loc	= -1;
	obj->weight		=  1;

	fNest	= TRUE;		/* Requiring a Nest 0 is a waste */
	fVnum	= FALSE;
	iNest	= 0;

	for ( ; ; )
	{
		word   = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'			  )									fread_to_eol( fp );
		else if ( !str_cmp(word, "ActionDesc") )		obj->action_descr =		fread_string( fp );
		else if ( !str_cmp(word, "Affect") ||	!str_cmp(word, "AffectData")
		  ||	!str_cmp(word, "NoAffect") || !str_cmp(word, "NoAffectData") )
		{
			AFFECT_DATA *paf;
			int			 pafmod;

			CREATE( paf, AFFECT_DATA, 1 );

			if ( !str_cmp(word, "Affect") )
			{
				paf->type	=													fread_number( fp );
			}
			else
			{
				int sn;

				sn = skill_lookup( fread_word(fp) );
				if ( sn < 0 )
					send_log( fp, LOG_SAVE, "fread_object_save: unknown skill all'oggetto #%d", obj->vnum );
				else
					paf->type = sn;
			}

			paf->duration		=												fread_number( fp );
			pafmod				=												fread_number( fp );
			paf->location		=												fread_number( fp );
			paf->bitvector		=												fread_old_bitvector( fp, CODE_AFFECT );

			if ( !str_cmp(word, "NoAffect") || !str_cmp(word, "NoAffectData") )
			{
				AFFECT_DATA	*aff;

				for ( aff = obj->first_affect;  aff;  aff = aff->next )
				{
					if ( is_same_affect(aff, paf, FALSE) )
						free_affect( aff );
				}
			}

			if ( paf->location == APPLY_WEAPONSPELL
			  || paf->location == APPLY_WEARSPELL
			  || paf->location == APPLY_STRIPSN
			  || paf->location == APPLY_REMOVESPELL
			  || paf->location == APPLY_RECURRINGSPELL )
			{
				paf->modifier		= slot_lookup( pafmod );
			}
			else
				paf->modifier		= pafmod;

			LINK( paf, obj->first_affect, obj->last_affect, next, prev );
		}
		else if ( !str_cmp(word, "Cost"		 ) )	obj->cost		 =			fread_number( fp );
		else if ( !str_cmp(word, "Count"	 ) )	obj->count		 =			fread_number( fp );
		else if ( !str_cmp(word, "Description") )	obj->long_descr =			fread_string( fp );
		else if ( !str_cmp(word, "End"			) )		break;
		else if ( !str_cmp(word, "ExtraFlags" ) )	obj->extra_flags =			fread_code_bit( fp, CODE_OBJFLAG );
		else if ( !str_cmp(word, "ExtraDescr" ) )
		{
			EXTRA_DESCR_DATA *ed;

			CREATE( ed, EXTRA_DESCR_DATA, 1 );
			ed->keyword		=													fread_string( fp );
			ed->description =													fread_string( fp );
			LINK( ed, obj->first_extradescr, obj->last_extradescr, next, prev );
		}
		else if ( !str_cmp(word, "ItemType"		) )		obj->type =			fread_number( fp );
		else if ( !str_cmp(word, "Level"		) )		obj->level =		fread_number( fp );
		else if ( !str_cmp(word, "Name"			) )		obj->name =			fread_string( fp );
		else if ( !str_cmp(word, "Mudprog"		) )		fread_mudprog( fp, &obj->first_mudprog, &obj->last_mudprog, TRUE );
		else if ( !str_cmp(word, "Nest"			) )
		{
			iNest = fread_number( fp );
			if ( iNest < 0 || iNest >= MAX_NEST )
			{
				send_log( fp, LOG_SAVE, "fread_object_save: bad nest %d.", iNest );
				iNest = 0;
				fNest = FALSE;
			}
		}
		else if ( !str_cmp(word, "NoExtraDescr"	) )
		{
			EXTRA_DESCR_DATA	*ed;
			EXTRA_DESCR_DATA	*ed_next;
			char				*keyword;

			keyword = fread_string( fp );

			for ( ed = obj->first_extradescr;  ed;  ed = ed_next )
			{
				ed_next = ed->next;

				if ( !str_cmp(ed->keyword, keyword) )
				{
					UNLINK( ed, obj->first_extradescr, obj->last_extradescr, next, prev );
					DISPOSE( ed->keyword );
					DISPOSE( ed->description );
					DISPOSE( ed );
					top_ed--;
				}
			}
		}
		else if ( !str_cmp(word, "NoMudProg"	) )
		{
			MPROG_DATA	*mprog;
			char		*trigger;

			trigger = fread_string( fp );
			for ( mprog = obj->first_mudprog;  mprog;  mprog = mprog->next )
			{
				if ( !str_cmp(mprog->arglist, trigger) )
					free_mudprog( mprog );
			}
		}
		else if ( !str_cmp(word, "Room"			) )		room =	get_room_index( fp, fread_number(fp) );
		else if ( !str_cmp(word, "ShortDescr"	) )		obj->short_descr =		fread_string( fp );
		else if ( !str_cmp(word, "Timer"		) )		obj->timer =			fread_number( fp );
		else if ( !str_cmp(word, "Values"		) )		fread_values( fp, obj );
		else if ( !str_cmp(word, "Vnum") )
		{
			VNUM vnum;

			vnum =																fread_number( fp );
			obj->pObjProto = get_obj_proto( fp, vnum );
			if ( !obj->pObjProto )
			{
				fVnum = FALSE;
				send_log( fp, LOG_SAVE, "fread_object_save: bad vnum %d.", vnum );
			}
			else
			{
				fVnum = TRUE;
				copy_object( obj->pObjProto, obj );
				obj->prototype = FALSE;
			}
		}
        /* (TT) temporaneamente li legge, alla prox conversione tolgo tutti i WearFlags ~ */
		else if ( !str_cmp(word, "WearFlags") )
		{
			obj->wear_flags =		fread_code_bit( fp, CODE_OBJWEAR );
			if ( IS_EMPTY(obj->wear_flags) )
				SET_BITS( obj->wear_flags, obj->pObjProto->wear_flags );
		}
		else if ( !str_cmp(word, "WearLoc"  ) )		obj->wear_loc =			fread_number( fp );
		else if ( !str_cmp(word, "Weight"   ) )		obj->weight =			fread_float( fp ) * 1000;
		else
		{
			send_log( fp, LOG_SAVE, "fread_object_save: etichetta non trovata per la parola %s.", word );
			fread_to_eol( fp );

			clean_object( obj );
			return;
		}
	} /* chiude il for */

	if ( !fNest || !fVnum )
	{
		if ( obj->name )
			send_log( fp, LOG_SAVE, "fread_object_save: %s incomplete object.", obj->name );
		else
			send_log( fp, LOG_SAVE, "fread_object_save: incomplete object." );

		clean_object( obj );

		return;
	}

	LINK( obj, first_object, last_object, next, prev );
	obj->pObjProto->count += obj->count;

	if ( !obj->serial )
	{
		cur_obj_serial = UMAX( (cur_obj_serial + 1) & ((1<<30)-1), 1 );
		obj->serial = obj->pObjProto->serial = cur_obj_serial;
	}

	if ( fNest )
		rgObjNest[iNest] = obj;

	objects_loaded += obj->count;
	++objects_physical;

	/* Corpse saving */
	if ( os_type == OS_CORPSE )
	{
		if ( !room )
		{
			send_log( fp, LOG_SAVE, "fread_object_save: Corpse without room" );
			room = get_room_index( NULL, VNUM_ROOM_LIMBO );
		}

		/* Give the corpse a timer if there isn't one */
		if ( obj->timer < 1 )
			obj->timer = 40;
		if ( room->vnum == VNUM_ROOM_HALLOFFALLEN && obj->first_content )
			obj->timer = -1;

		obj = obj_to_room( obj, room );
	}
	else if ( iNest == 0 || rgObjNest[iNest] == NULL )
	{
		int wear_loc = obj->wear_loc;
		int slot = -1;
		bool reslot = FALSE;

		obj->wear_loc = -1;

		if ( wear_loc > -1 && wear_loc < MAX_WEARLOC )
		{
			for ( x = 0;  x < MAX_LAYERS;  x++ )
			{
				if ( save_equipment[wear_loc][x] == NULL )
				{
					save_equipment[wear_loc][x] = obj;
					slot = x;
					reslot = TRUE;
					break;
				}
			}

			if ( x == MAX_LAYERS )
				send_log( fp, LOG_SAVE, "fread_object_save: too many layers %d", wear_loc );
		}
		else
		{
			obj->wear_loc = -1;
		}

		obj = obj_to_char( obj, ch );

		if ( reslot && slot != -1 )
			save_equipment[wear_loc][slot] = obj;
	}
	else
	{
		if ( rgObjNest[iNest-1] )
		{
			split_obj( rgObjNest[iNest-1], 1 );
			obj = obj_to_obj( obj, rgObjNest[iNest-1] );
		}
		else
		{
			send_log( fp, LOG_SAVE, "fread_object_save: nest layer missing %d", iNest-1 );
		}
	}

	if ( fNest )
		rgObjNest[iNest] = obj;
}


/*
 * This will read one mobile structure pointer to by fp.
 */
CHAR_DATA *fread_mobile_save( MUD_FILE *fp )
{
	CHAR_DATA	*mob = NULL;
	char		*word;
	int			 inroom = 0;
	ROOM_DATA *pRoom = NULL;

	word = feof( fp->file )  ?  "End"  :  fread_word( fp );

	if ( !str_cmp(word, "Vnum") )
	{
		VNUM vnum;

		vnum = fread_number( fp );
		mob = make_mobile( get_mob_index(fp, vnum) );
		if ( !mob )
		{
			for ( ; ; )
			{
				word = feof( fp->file )  ?  "End"  :  fread_word( fp );
				/* So we don't get so many bug messages when something messes up */
				if ( !str_cmp(word, "End") )
					break;
			}
			send_log( fp, LOG_SAVE, "fread_mobile_save: Nessun index data per il vnum %d", vnum );
			return NULL;
		}
	}
	else
	{
		for ( ; ; )
		{
			word = feof( fp->file )  ?  "End"  :  fread_word( fp );
			/* So we don't get so many bug messages when something messes up. */
			if ( !str_cmp(word, "End") )
				break;
		}
		extract_char( mob, TRUE );
		send_log( fp, LOG_SAVE, "fread_mobile_save: Vnum non trovato." );
		return NULL;
	}

	for ( ; ; )
	{
		word   = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'			  )								fread_to_eol( fp );
		else if ( !str_cmp(word, "#OBJECT")	  )								fread_object_save( mob, fp, OS_CARRY );
		else if ( !str_cmp(word, "End" ) )
		{
			if ( inroom == 0 )
				inroom = VNUM_ROOM_TEMPLE;

			pRoom = get_room_index( NULL, inroom );
			if ( !pRoom )
				pRoom = get_room_index( NULL, VNUM_ROOM_TEMPLE );

			char_to_room( mob, pRoom );
			return mob;
		}
		else if ( !str_cmp(word, "Description") )	mob->description =		fread_string( fp );
		else if ( !str_cmp(word, "Flags"	  ) )	mob->act =				fread_code_bit( fp, CODE_MOB );
		else if ( !str_cmp(word, "Long"		  ) )	mob->long_descr  =		fread_string( fp );
		else if ( !str_cmp(word, "Name"		  ) )	mob->name		 =		fread_string( fp );
		else if ( !str_cmp(word, "Position"	  ) )	mob->position	 =		code_code( fp, fread_word(fp), CODE_POSITION );
		else if ( !str_cmp(word, "Room"		  ) )	inroom			 =		fread_number( fp );
		else if ( !str_cmp(word, "Short"	  ) )	mob->short_descr =		fread_string( fp );
		else
			send_log( fp, LOG_SAVE, "fread_mobile_save: etichetta non trovata per la parola %s.", word );
	} /* chiude il for */

	return NULL;
}


/*
 * Load a char and inventory into a new ch structure.
 */
bool load_player( DESCRIPTOR_DATA *d, char *name, bool preload )
{
	CHAR_DATA	*ch;
	MUD_FILE	*fp;
	struct stat	 fst;
	char		 strsave[MIL];
	int			 x;
	int			 y;
	bool		 found;

	for ( x = 0;  x < MAX_WEARLOC;  x++ )
	{
		for ( y = 0;  y < MAX_LAYERS;  y++ )
			save_equipment[x][y] = NULL;
	}

	CREATE( ch, CHAR_DATA, 1 );
	init_char( ch, FALSE );

	CREATE( ch->pg, PG_DATA, 1 );
	init_pg( ch->pg );

	loading_char = ch;

	ch->name				= str_dup( capitalize(name) );
	ch->pg->host			= str_dup( d->host );
	d->character			= ch;
	ch->desc				= d;
	CLEAR_BITS( ch->no_resistant );
	CLEAR_BITS( ch->no_susceptible );
	CLEAR_BITS( ch->no_immune );
	ch->was_in_room			= NULL;
	CLEAR_BITS( ch->no_affected_by );
	ch->mental_state		= -10;
	ch->mobinvis			=   0;	/* livello zero, mobinvis disattivo */
	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		ch->saving_throw[x] = 0;
	ch->style 				= STYLE_FIGHTING;
	ch->morph				= NULL;

	found = FALSE;
	sprintf( strsave, "%s%s", PLAYER_DIR, ch->name );
	if ( stat(strsave, &fst) != -1 )
	{
		if ( fst.st_size == 0 )
		{
			sprintf( strsave, "%s%s", BACKUP_DIR, ch->name );
			send_to_char( ch, "Restoring your backup player file..." );
		}
		else
		{
			if ( !fBootDb )	/* Per evitare di spamare nell'avvio */
			{
				send_log( NULL, LOG_COMM, "%s player data for: %s (%dKb)",
					preload ? "Preloading" : "Loading",
					ch->name,
					(int) fst.st_size/1024 );
			}
		}
	}
	/* else no player file */

	fp = mud_fopen( "", strsave, "r", FALSE );
	if ( fp )
	{
		int		iNest;

		found = TRUE;	/* il file c'è */

		for ( iNest = 0;  iNest < MAX_NEST;  iNest++ )
			rgObjNest[iNest] = NULL;

		/* Cheat so that bug will show line #'s */
		for ( ; ; )
		{
			char	 letter;
			char	*word;

			letter = fread_letter( fp );
			if ( letter == '*' )
			{
				fread_to_eol( fp );
				continue;
			}

			if ( letter != '#' )
			{
				send_log( fp, LOG_SAVE, "load_player: # not found" );
				break;
			}

			word = fread_word( fp );
			if ( !str_cmp(word, "PLAYER") )
			{
				fread_player( ch, fp, preload );

				if ( preload )
					break;
			}
			else if ( !str_cmp(word, "OBJECT") )		/* Objects	*/
			{
				if ( preload )
					break;

				fread_object_save( ch, fp, OS_CARRY );
			}
			else if ( !str_cmp(word, "MORPHDATA") )	/* Morphs */
			{
				if ( preload )
					break;

				fread_morph_data ( ch, fp );
			}
			else if ( !str_cmp(word, "MOBILE") )
			{
				CHAR_DATA	*mob;

				if ( preload )
					break;

				mob = fread_mobile_save( fp );
				ch->pg->mascotte = mob;
				mob->master = ch;
				SET_BIT( mob->affected_by, AFFECT_CHARM );
			}
			else if ( !str_cmp(word, "END") )	/* Done */
				break;
			else
			{
				send_log( fp, LOG_SAVE, "load_player: bad section" );
				break;
			}
		} /* chiude il for */
		MUD_FCLOSE( fp );
	} /* chiude l'if */

	if ( !found )
	{
		/* (FF) Every characters starts at default board from login...
		 * This board should be read_level == 0 ! */
		ch->short_descr			= str_dup( "" );
		ch->long_descr			= str_dup( "" );
		ch->description			= str_dup( "" );
		ch->editor				= NULL;

		init_pg_strings( ch->pg );
	}
	else
	{
		for ( x = 0;  x < MAX_WEARLOC;  x++ )
		{
			for ( y = 0;  y < MAX_LAYERS;  y++ )
			{
				if ( save_equipment[x][y] == NULL )
					break;

				equip_char( ch, save_equipment[x][y], x );
				save_equipment[x][y] = NULL;
			}
		}
	} /* chiude l'if */

	/* Se il pg era una Spirito quando ha quittato ora gli ridà il timer pieno di spirito
	 * Da notare che invece lo status di metà-spirito con il reload del pg viene perso */
	if ( HAS_BIT_PLR(ch, PLAYER_SPIRIT) )
	{
		/* (FF) (RR) (TT) da fare, me ne sono dimenticato? */
	}

	/* Rebuild affected_by and RIS to catch errors */
	update_aris( ch );
	loading_char = NULL;

	return found;
}


void set_alarm( long seconds )
{
	alarm( seconds );
}


/*
 * Based on last time modified, show when a player was last on
 * (RR) cambiarlo con un check sui pg offline
 */
DO_RET do_last( CHAR_DATA *ch, char *argument )
{
	char	buf [MSL];
	char	name[MIL];
	struct stat fst;

	one_argument( argument, name );
	if ( !VALID_STR(name) )
	{
		send_to_char( ch, "&YSintassi: last <giocatore>   Mostra quando è stato l'ultimo accesso del giocatore\r\n" );
		return;
	}

	strcpy( name, capitalize(name) );
	sprintf( buf, "%s%s", PLAYER_DIR, name );

	if ( stat(buf, &fst) != -1 && check_parse_name(name, FALSE) )
		ch_printf( ch, "%s was last on: %21.21s\r\n", name, friendly_ctime(&fst.st_mtime) );
	else
		ch_printf( ch, "%s was not found.\r\n", name );
}


/*
 * Added support for removeing so we could take out the write_corpses
 * so we could take it out of the save_char function.
 */
void write_corpses( CHAR_DATA *ch, char *name, OBJ_DATA *objrem )
{
	OBJ_DATA *corpse;
	MUD_FILE *fp = NULL;		/* inizializzato a NULL */

	/* Name and ch support so that we dont have to have a char to save their
	 corpses.. (ie: decayed corpses while offline) */
	if ( ch && IS_MOB(ch) )
	{
		send_log( NULL, LOG_SAVE, "write_corpses: writing NPC corpse." );
		return;
	}

	if ( ch && ch->in_room && ch->in_room->area
	  && HAS_BIT(ch->in_room->area->flags, AREAFLAG_NOSAVE) )
	{
		return;
	}

	if ( ch )
		name = ch->name;

	/* Go by vnum, less chance of screwups. */
	for ( corpse = first_object;  corpse;  corpse = corpse->next )
	{
		char	keyword[MIL];

		if ( corpse->vnum != VNUM_OBJ_CORPSE_PC )	continue;
		if ( !corpse->in_room )						continue;

		one_argument( corpse->name, keyword );

		if ( str_cmp(keyword, name) )				continue;
		if ( objrem == corpse )						continue;

		if ( !fp )	/* Se non lo ha già aperto lo apre */
		{
			fp = mud_fopen( CORPSE_DIR, capitalize(name), "w", TRUE );
			if ( !fp )
				return;
		}
		fwrite_object( ch, corpse, fp, 0, OS_CORPSE );
	}

	if ( fp && fp->file )
	{
		fprintf( fp->file, "#END\n\n" );
		MUD_FCLOSE( fp );
	}
	else
	{
		char	buf[127];

		sprintf( buf, "%s%s", CORPSE_DIR, capitalize(name) );
		if ( remove(buf) != 0 )
		{
			if ( errno != ENOENT )
			{
				send_log( NULL, LOG_SAVE, "write_corpses: impossibile rimuovere il file %s (errno: %s)",
					buf, strerror(errno) );
			}
		}
	}
}


void load_corpses( void )
{
	DIR			  *dp;
	struct dirent *de;
	MUD_FILE	  *fp;
	extern int	   falling;

	dp = opendir( CORPSE_DIR );
	if ( !dp )
	{
		send_log( NULL, LOG_SAVE, "load_corpses: impossibile aprire la directory %s", CORPSE_DIR );
		return;
	}

	falling = 1;	/* Arbitrary, must be > 0 though. */
	while ( (de = readdir(dp)) != NULL )
	{
		if ( str_infix(".", de->d_name) )
		{
			send_log( NULL, LOG_SAVE, "Cadavere -> %s\n", de->d_name );
			fp = mud_fopen( CORPSE_DIR, de->d_name, "r", TRUE );
			if ( !fp )
				continue;

			for ( ; ; )
			{
				char letter;
				char *word;

				letter = fread_letter( fp );

				if ( letter == '*' )
				{
					fread_to_eol( fp );
					continue;
				}

				if ( letter != '#' )
				{
					send_log( fp, LOG_SAVE, "load_corpses: # not found." );
					break;
				}

				word = fread_word( fp );
				if		( !str_cmp(word, "CORPSE") )	fread_object_save( NULL, fp, OS_CORPSE );
				else if ( !str_cmp(word, "OBJECT") )	fread_object_save( NULL, fp, OS_CARRY );
				else if ( !str_cmp(word, "END"	) )		break;
				else
				{
					send_log( fp, LOG_SAVE, "load_corpses: bad section." );
					break;
				}
			}
			MUD_FCLOSE( fp );
		} /* chiude l'if */
	} /* chiude il while */
	closedir( dp );
	falling = 0;
}


/*
 * This will write in the saved mobile for a char.
 */
void write_char_mobile( CHAR_DATA *ch , const char *filename )
{
	MUD_FILE 	  *fp;
	CHAR_DATA *mob;

	if ( IS_MOB(ch) || !ch->pg->mascotte )
		return;

	if ( HAS_BIT(ch->in_room->area->flags, AREAFLAG_NOSAVE) )
		return;

	fp = mud_fopen( "", filename, "w", TRUE );
	if ( !fp )
		return;

	mob = ch->pg->mascotte;
	SET_BIT( mob->affected_by, AFFECT_CHARM );
	fwrite_mobile( fp, mob );

	MUD_FCLOSE( fp );
}


/*
 * This will read in the saved mobile for a char.
 */
void read_char_mobile( const char *filename )
{
	MUD_FILE	  *fp;
	CHAR_DATA *mob;

	fp = mud_fopen( "", filename, "r", TRUE );
	if ( !fp )
		return;

	mob = fread_mobile_save( fp );
	MUD_FCLOSE( fp );
}
