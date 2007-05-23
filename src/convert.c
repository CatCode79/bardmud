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


#ifdef T2_CNV_SMAUG

/* Modulo di conversione Bard */

#include "mud.h"
#include "fread.h"


/*
 * Converte e scrive in un file skill_cnv.dat la nuova struttura di skill.
 * Mancano: SkillAdept e RaceAdept
 */
void convert_skill_from_smaug( const char *filename, SKILL_DATA *skill )
{
	SMAUG_AFF  *aff;
	MUD_FILE   *fp;
	int			modifier;

	if ( !skill )
	{
		send_log( NULL, LOG_CNV, "convert_skill: skill è NULL." );
		return;
	}

	fp = mud_fopen( "", filename, "a", TRUE );
	if ( !fp )
		return;

	switch ( skill->type )
	{
	  default:			fprintf( fp->file, "#SKILL\n" );	break;
	  case SKILL_SPELL:	fprintf( fp->file, "#SPELL\n" );	break;
	  case SKILL_HERB:	fprintf( fp->file, "#HERB\n" );		break;
	}

	fprintf( fp->file, "Name         %s~\n",	skill->name );
	fprintf( fp->file, "Type         %s\n",		code_str(fp, skill->type, CODE_SKILLTYPE) );
	fprintf( fp->file, "FunName      %s\n",		skill->fun_name );
	/* fread_bit (TT) attenzione ad info e flags collegamento con le flags e il BV11 e le OLD_SF */
	fprintf( fp->file, "Info         %s~\n",	code_bit(fp, skill->info, CODE_SKELL_FLAG) );
	fprintf( fp->file, "Flags        %s~\n",	code_bit(fp, skill->flags, CODE_SKELL_FLAG) );
	/* fread_word */
	fprintf( fp->file, "Target       %s\n",		code_str(fp, skill->target, CODE_TARGET) );
	/* Non converte a +100 le posizioni, fread_word */
	fprintf( fp->file, "MinPos       %s\n",		code_str(fp, skill->min_position, CODE_POSITION) );
	fprintf( fp->file, "Seconds      %d\n",		skill->pulse );
	fprintf( fp->file, "Difficulty   %d\n",		skill->difficulty );

	if ( skill->type == SKILL_SPELL )
	{
		fprintf( fp->file, "Intention    %s\n",		code_str(fp, skill->intention, CODE_INTENTION) );
		fprintf( fp->file, "Components  %s~\n",		skill->components );
		/* fread_ext */
		if ( !IS_EMPTY(skill->spell_sector) )
			fprintf( fp->file, "SpellSector  %s~\n",	code_bit(fp, skill->spell_sector, CODE_SECTOR) );
	}

	/* fread_word */
	if ( skill->saves )
		fprintf( fp->file, "Saves        %s\n",		code_str(fp, skill->saves, CODE_SAVETHROW) );
	/* (FF) Sarebbe da modificare con il nome della funzione dell'incantesimo o skill nelle aree al posto del numero, cmq capire bene a che serve */
	if ( skill->slot )
		fprintf( fp->file, "Slot         %d\n",		skill->slot );
	if ( skill->min_mana )
		fprintf( fp->file, "Mana         %d\n",		skill->min_mana );
	if ( skill->range )
		fprintf( fp->file, "Range        %d\n",		skill->range );
	/* (FF) trovare un modo elastico per convertirla in stringa o codice di parsing in futuro */
	if ( VALID_STR(skill->clan) )
		fprintf( fp->file, "Clan         %d\n",		skill->clan );
	if ( skill->participants )
		fprintf( fp->file, "Participants %d\n",		skill->participants );
	if ( VALID_STR(skill->teachers) )
		fprintf( fp->file, "Teachers     %s~\n",	skill->teachers );

	if ( VALID_STR(skill->hit_area) )
		fprintf( fp->file, "HitArea      %s~\n",	skill->hit_area );
	if ( VALID_STR(skill->hit_around) )
		fprintf( fp->file, "HitAround    %s~\n",	skill->hit_around );
	if ( VALID_STR(skill->hit_char) )
		fprintf( fp->file, "HitChar      %s~\n",	skill->hit_char );
	if ( VALID_STR(skill->hit_vict) )
		fprintf( fp->file, "HitVict      %s~\n",	skill->hit_vict );
	if ( VALID_STR(skill->hit_room) )
		fprintf( fp->file, "HitRoom      %s~\n",	skill->hit_room );
	if ( VALID_STR(skill->hit_dest) )
		fprintf( fp->file, "HitDest      %s~\n",	skill->hit_dest );

	if ( VALID_STR(skill->hit_area) )
		fprintf( fp->file, "MastArea     %s~\n",	skill->mast_area );
	if ( VALID_STR(skill->hit_around) )
		fprintf( fp->file, "MastAround   %s~\n",	skill->mast_around );
	if ( VALID_STR(skill->hit_char) )
		fprintf( fp->file, "MastChar     %s~\n",	skill->mast_char );
	if ( VALID_STR(skill->hit_vict) )
		fprintf( fp->file, "MastVict     %s~\n",	skill->mast_vict );
	if ( VALID_STR(skill->hit_room) )
		fprintf( fp->file, "MastRoom     %s~\n",	skill->mast_room );
	if ( VALID_STR(skill->hit_dest) )
		fprintf( fp->file, "MastDest     %s~\n",	skill->mast_dest );

	if ( VALID_STR(skill->hit_area) )
		fprintf( fp->file, "CritArea     %s~\n",	skill->crit_area );
	if ( VALID_STR(skill->hit_around) )
		fprintf( fp->file, "CritAround   %s~\n",	skill->crit_around );
	if ( VALID_STR(skill->hit_char) )
		fprintf( fp->file, "CritChar     %s~\n",	skill->crit_char );
	if ( VALID_STR(skill->hit_vict) )
		fprintf( fp->file, "CritVict     %s~\n",	skill->crit_vict );
	if ( VALID_STR(skill->hit_room) )
		fprintf( fp->file, "CritRoom     %s~\n",	skill->crit_room );
	if ( VALID_STR(skill->hit_dest) )
		fprintf( fp->file, "CritDest     %s~\n",	skill->crit_dest );

	if ( VALID_STR(skill->miss_area) )
		fprintf( fp->file, "MissArea     %s~\n",	skill->miss_area );
	if ( VALID_STR(skill->miss_around) )
		fprintf( fp->file, "MissAround   %s~\n",	skill->miss_around );
	if ( VALID_STR(skill->miss_char) )
		fprintf( fp->file, "MissChar     %s~\n",	skill->miss_char );
	if ( VALID_STR(skill->miss_vict) )
		fprintf( fp->file, "MissVict     %s~\n",	skill->miss_vict );
	if ( VALID_STR(skill->miss_room) )
		fprintf( fp->file, "MissRoom     %s~\n",	skill->miss_room );

	/* (RR) da fare */
	if ( VALID_STR(skill->die_area) )
		fprintf( fp->file, "DieArea      %s~\n",	skill->die_area );
	if ( VALID_STR(skill->die_around) )
		fprintf( fp->file, "DieAround    %s~\n",	skill->die_around );
	if ( VALID_STR(skill->die_char) )
		fprintf( fp->file, "DieChar      %s~\n",	skill->die_char );
	if ( VALID_STR(skill->die_vict) )
		fprintf( fp->file, "DieVict      %s~\n",	skill->die_vict );
	if ( VALID_STR(skill->die_room) )
		fprintf( fp->file, "DieRoom      %s~\n",	skill->die_room );

	if ( VALID_STR(skill->imm_area) )
		fprintf( fp->file, "ImmArea      %s~\n",	skill->imm_area );
	if ( VALID_STR(skill->imm_around) )
		fprintf( fp->file, "ImmAround    %s~\n",	skill->imm_around );
	if ( VALID_STR(skill->imm_char) )
		fprintf( fp->file, "ImmChar      %s~\n",	skill->imm_char );
	if ( VALID_STR(skill->imm_vict) )
		fprintf( fp->file, "ImmVict      %s~\n",	skill->imm_vict );
	if ( VALID_STR(skill->imm_room) )
		fprintf( fp->file, "ImmRoom      %s~\n",	skill->imm_room );

	fprintf( fp->file, "DamMsg       %s~\n",		skill->noun_damage );

	if ( VALID_STR(skill->msg_off) )
		fprintf( fp->file, "WearOff      %s~\n",	skill->msg_off );

	if ( VALID_STR(skill->dice) )
		fprintf( fp->file, "Dice         %s~\n",	skill->dice );
	if ( skill->value )
		fprintf( fp->file, "Value        %d\n",		skill->value );

#ifdef T2_SKILL
	fprintf( fp->file, "PreSkill1    %s",			skill->pre_skill1 );
	if ( VALID_STR(skill->pre_skill2) )
		fprintf( fp->file, "PreSkill2   %s",		skill->pre_skill2 );
	if ( VALID_STR(skill->pre_skill3) )
		fprintf( fp->file, "PreSkill3   %s",		skill->pre_skill3 );
#endif

	for ( aff = skill->affects;  aff;  aff = aff->next )
	{
		/* fread_word per la lettura dell'aff->location */
		fprintf( fp->file, "Affect       '%s' %s ",	aff->duration, code_str(fp, aff->location, CODE_APPLY) );

		modifier = atoi( aff->modifier );
		if ( (aff->location == APPLY_WEAPONSPELL
		  ||  aff->location == APPLY_WEARSPELL
		  ||  aff->location == APPLY_REMOVESPELL
		  ||  aff->location == APPLY_STRIPSN
		  ||  aff->location == APPLY_RECURRINGSPELL)
		  && is_valid_sn(modifier) )
			fprintf( fp->file, "'%d' ", table_skill[modifier]->slot );
		else
			fprintf( fp->file, "'%s' ", aff->modifier );

		fprintf( fp->file, "%s~\n", code_bit(fp, aff->bitvector, CODE_AFFECT) );
	}

	fprintf( fp->file, "End\n\n" );

	/* (bb) Ricordarsi poi che per ogni file convertito bisogna aggiungere #END alla fine, bisogna farlo a mano */

	MUD_FCLOSE( fp );
}


#if T2_ALFA
void convert_mobile_smaug( COMMAND_DATA *cmd )
{
	MUD_FILE *list;

	list = mud_fopen( AREA_DIR, "lista.mob", "a", TRUE );
	if ( !list )
		return;

	fprintf( list->file, "#MOBILE\r\n" );
	fprintf( list->file, "Vnum        %d\r\n",		pMobIndex->vnum );
	fprintf( list->file, "Name        %s~\r\n",		pMobIndex->name );
	fprintf( list->file, "ShortDescr  %s~\r\n",		pMobIndex->short_descr );
	fprintf( list->file, "LongDescr   %s~\r\n",		pMobIndex->long_descr );
	fprintf( list->file, "Race        %s\r\n",		code_str(fp, pMobIndex->race, CODE_RACE) );
	fprintf( list->file, "End\r\n\r\n" );

	MUD_FCLOSE( list );
}

#endif	/* T2_ALFA */


#endif	/* T2_CNV_SMAUG */
