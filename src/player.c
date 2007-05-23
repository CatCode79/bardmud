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
 >		Comandi per i personali settaggi/statistiche dei giocatori			<
\****************************************************************************/

#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "mud.h"
#include "affect.h"
#include "archetype.h"
#include "calendar.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "editor.h"
#include "gramm.h"
#include "group.h"
#include "information.h"
#include "interpret.h"
#include "mccp.h"
#include "md5.h"
#include "msp.h"
#include "mxp.h"
#include "nanny.h"
#include "object_handler.h"
#include "object_interact.h"
#include "morph.h"
#include "punition.h"
#include "room.h"
#include "save.h"


/*
 * (GR) beccare il termine in femminile o maschile con la funzione futura grammaticale
 */
char *get_attr_alias( CHAR_DATA *ch, int attr )
{
	int		value;

	value = get_curr_attr( ch, attr );
	/* Segue la scala di valori che crea la get_skill_alias */
	switch ( attr )
	{
	  case ATTR_STR:		case ATTR_RES:	case ATTR_CON:
	  case ATTR_INT:		case ATTR_COC:	case ATTR_WIL:
	  case ATTR_AGI:		case ATTR_SPE:	case ATTR_SPI:
	  case ATTR_MIG:		case ATTR_EMP:	case ATTR_BEA:
		if		( value >= 90 ) 	return "&RMOSTRUOSA";
		else if ( value >= 75 )		return "&GPerfetta";
		else if ( value >= 60 )		return " &YOttima";
		else if ( value >= 45 )		return " &wBuona";
		else if ( value >= 30 )		return " &cMedia";
		else if ( value >= 15 )		return " &OBassa";
		else if ( value >   0 )		return " &zMisera";
		else
		{
			send_log( NULL, LOG_BUG, "get_attr_alias: valore errato di %s a %s: %d",
				code_name(NULL, attr, CODE_ATTR), ch->name, value );
			return "(errore)";
		}
		break;

	  case ATTR_REF:		case ATTR_ABS:		case ATTR_LEA:
		if		( value >= 90 ) 	return "&RMOSTRUOSO";
		else if ( value >= 75 )		return "&GPerfetto";
		else if ( value >= 60 )		return " &YOttimo";
		else if ( value >= 45 )		return " &wBuono";
		else if ( value >= 30 )		return " &cMedio";
		else if ( value >= 15 )		return " &OBasso";
		else if ( value >   0 )		return " &zMisero";
		else
		{
			send_log( NULL, LOG_BUG, "get_attr_alias: valore errato di %s a %s: %d",
				code_name(NULL, attr, CODE_ATTR), ch->name, value );
			return "(errore)";
		}
		break;
	}

	send_log( NULL, LOG_BUG, "get_attr_alias: attributo passato non trovato: %d", value );
	return "(errore)";
}


/*
 * Invia l'etichetta descrivente la potenza del senso sulla base del valore attuale
 * (GR) beccare il termine in femminile o maschile con la funzione futura grammaticale
 */
char *get_sense_alias( CHAR_DATA *ch, int sense )
{
	int		value;

	value = get_curr_sense( ch, sense );
	/* Segue la scala di valori che crea la get_skill_alias */
	switch ( sense )
	{
	  case SENSE_SIGHT:
		if		( value >= 90 ) 	return "&RMOSTRUOSA";
		else if ( value >= 75 )		return "&GPerfetta";
		else if ( value >= 60 )		return " &YOttima";
		else if ( value >= 45 )		return " &wBuona";
		else if ( value >= 30 )		return " &cMedia";
		else if ( value >= 15 )		return " &OBassa";
		else if ( value >   0 )		return " &zMisera";
		else
		{
			send_log( NULL, LOG_BUG, "get_sense_alias: valore errato di %s a %s: %d",
				code_name(NULL, sense, CODE_SENSE), ch->name, value );
			return "(errore)";
		}
		break;
	  case SENSE_HEARING:		case SENSE_SMELL:
	  case SENSE_TASTE:		case SENSE_TOUCH:
	  case SENSE_SIXTH:
		if		( value >= 90 ) 	return "&RMOSTRUOSO";
		else if ( value >= 75 )		return "&GPerfetto";
		else if ( value >= 60 )		return " &YOttimo";
		else if ( value >= 45 )		return " &wBuono";
		else if ( value >= 30 )		return " &cMedio";
		else if ( value >= 15 )		return " &OBasso";
		else if ( value >   0 )		return " &zMisero";
		else
		{
			send_log( NULL, LOG_BUG, "get_sense_alias: valore errato di %s a %s: %d",
				code_name(NULL, sense, CODE_SENSE), ch->name, value );
			return "(errore)";
		}
		break;
	}

	send_log( NULL, LOG_BUG, "get_sense_alias: senso passato non trovato: %d", value );
	return "(errore)";
}


char *get_style_name( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_style_name: ch è NULL" );
		return "&wnormale";
	}

	switch ( ch->style )
	{
	  default:					return "&wNormale";
	  case STYLE_EVASIVE:		return "&CEvasivo";
	  case STYLE_DEFENSIVE:		return "&GDifensivo";
	  case STYLE_AGGRESSIVE:	return "&YAggressivo";
	  case STYLE_BERSERK:		return "&GBerserk";
	}
}


/*
 * Score che vedono i pg e gli amministratori, informazioni divise.
 */
void score_handler( CHAR_DATA *ch, CHAR_DATA* victim, char *argument )
{
	AFFECT_DATA	*paf;
	char		 column1[MSL];
	char		 column2[MIL];
	char		 column3[MIL];
	char		 arg[MIL];
	char		 csex;			/* Carattere 'a' o 'o' dipende dal sesso del pg */
	int			 x;

	csex = gramm_ao( victim );			/* Prepara il carattere che utilizza nelle stringhe per il genere di alcune parole */

	set_char_color( AT_SCORE, ch );

	/* --- Nome, cognome, articolo, razza, class --- */
	{
		char	surname[MIL];
		char	class_name[MIL];

		surname[0] = '\0';
		if ( IS_PG(victim) && VALID_STR(victim->pg->surname) )
			sprintf( surname, " %s", victim->pg->surname );

		strcpy( class_name, get_class(victim, TRUE) );
		strcpy( class_name, get_article(class_name, ARTICLE_INDETERMINATE, (victim->sex == SEX_FEMALE) ? ARTICLE_FEMININE : -1, -1) );

		pager_printf( ch,
			"&wIl mio nome è &W%s&w%s, sono %s%s %s&w e ho &W%d&w anni.\r\n",	/* (FF) da togliere l'età appena viene fatto il sistema di compleanni-vecchiaia */
			get_name(victim),
			surname,
			(IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_SPIRIT)) ? "un'anima di " : "",
			class_name,
			get_race(victim, TRUE),
			(IS_MOB(victim)) ? 0 : get_age(victim, TIME_YEAR) );
	}

	/* Archetipo */
	if ( IS_PG(victim) )
	{
		char	archetype[MIL];

		strcpy( archetype, get_archetype(victim) );
		strcpy( archetype, get_article(archetype, ARTICLE_INDETERMINATE, (victim->sex == SEX_FEMALE) ? ARTICLE_FEMININE : -1, -1) );

		pager_printf( ch, "Il mio carattere segue l'archetipo di %s\r\n", archetype );
	}

#ifdef T2_ALFA
	/* --- Luogo di nascita, data, età --- */	/* (RR) */
	pager_printf( ch, "&wSono nat%c a &W%s &wil &Wxx mese xxxx&w, ho &W%d&w anni.\r\n",
		csex, MUD_NAME, get_age(victim, TIME_YEAR) );

	/* --- Visualizza il segno zodiacale se il pg lo conosce --- */
	if ( IS_PG(victim) && victim->pg->zodiac != -1 )
		pager_printf( ch, "&wLa mia costellazione è &Wla MuCCHa PAzZa&w!!\r\n" );
#endif

	/* --- Titolo, se ne possiede uno --- */
	if ( IS_PG(victim) && VALID_STR(victim->pg->title) )
		pager_printf( ch, "&wSono conosciut%c come%s.\r\n", csex, victim->pg->title );

	/* Obiettivo */
	if ( IS_PG(victim) && VALID_STR(victim->pg->objective) )
		pager_printf( ch, "Il mio obiettivo è %s.", victim->pg->objective );

	/* --- Riga separatrice --- */
	send_to_pager( ch, "&g--------------------------------------------------------------------------------\r\n" );

	/* --- Forza, Parte del Corpo 0, Colore capelli-Tinta capelli --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Str   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_STR], victim->attr_mod[ATTR_STR], get_curr_attr(victim, ATTR_STR) );
	else
		sprintf( column1, " Forza        %-9.9s", get_attr_alias(victim, ATTR_STR) );

	if ( IS_MOB(victim) )
		sprintf( column3, "Colore capelli    Nessuno" );
	else
	{
		sprintf( column3, "%s    &%c%s",
			(victim->pg->hair->dye->cat == AT_NONE) ? "Colore capelli"		: "Tinta capelli ",
			(victim->pg->hair->dye->cat == AT_NONE) ? get_clr(victim->pg->hair->color->cat) : get_clr(victim->pg->hair->dye->cat),
			(victim->pg->hair->dye->cat == AT_NONE) ? victim->pg->hair->color->syn : victim->pg->hair->dye->syn );
	}

	pager_printf( ch, "&R%s &g| %s &g| &w%s\r\n",
		column1,
		score_partcorp(victim, 0),
		column3 );

	/* --- Resistenza, Parte del Corpo 1, Tipo di capelli-Acconciatura capelli --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Res   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_RES], victim->attr_mod[ATTR_RES], get_curr_attr(victim, ATTR_RES) );
	else
		sprintf( column1, " Resistenza   %-9.9s", get_attr_alias(victim, ATTR_RES) );

	if ( IS_MOB(victim) )
		sprintf( column3, "Tipo di capelli   Nessuno" );
	else
	{
		sprintf( column3, "%s   &W%s",
			(victim->pg->hair->style->cat == HAIRSTYLE_NONE) ? "Tipo di capelli"	: "Acconciatura   ",
			(victim->pg->hair->style->cat == HAIRSTYLE_NONE) ? victim->pg->hair->type->syn  : victim->pg->hair->style->syn );
	}

	pager_printf( ch, "&R%s &g| %s &g| &w%s\r\n",
		column1,
		score_partcorp(victim, 1),
		column3 );

	/* --- Costituzione, Parte del Corpo 2, Lunghezza capelli --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Con   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_CON], victim->attr_mod[ATTR_CON], get_curr_attr(victim, ATTR_CON) );
	else
		sprintf( column1, " Costituzione %-9.9s", get_attr_alias(victim, ATTR_CON) );

	if ( IS_PG(victim) && victim->pg->hair && victim->pg->hair->type->cat == HAIRTYPE_BALD )
		column3[0] = '\0';
	else
		sprintf( column3, " &wLunghezza capelli &W%d &wcm", (IS_MOB(victim)) ? -1 : victim->pg->hair->length );

	pager_printf( ch, "&R%s &g| %s &g|%s\r\n",
		column1,
		score_partcorp(victim, 2),
		column3 );

	/* --- Intelligenza, Parte del Corpo 3, Colore occhi-Luminosità occhi --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Int   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_INT], victim->attr_mod[ATTR_INT], get_curr_attr(victim, ATTR_INT) );
	else
		sprintf( column1, " Intelligenza %-9.9s", get_attr_alias(victim, ATTR_INT) );

	if ( IS_MOB(victim) )
		sprintf( column3, "Colore Occhi    Nessuno" );
	else
	{
		sprintf( column3, "%s  &%c%s",
			(victim->pg->eye->bright_color->cat == AT_NONE) ? "Colore occhi    "	   : "Luminosità occhi",
			(victim->pg->eye->bright_color->cat == AT_NONE) ? get_clr(victim->pg->eye->color->cat) : get_clr(victim->pg->eye->bright_color->cat),
			(victim->pg->eye->bright_color->cat == AT_NONE) ? victim->pg->eye->color->syn : victim->pg->eye->bright_color->syn );
	}

	pager_printf( ch, "&B%s &g| %s &g| &w%s\r\n",
		column1,
		score_partcorp(victim, 3),
		column3 );

	/* --- Concentrazione, Parte del Corpo 4, Mano principale --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Coc   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_COC], victim->attr_mod[ATTR_COC], get_curr_attr(victim, ATTR_COC) );
	else
		sprintf( column1, " Concentrazio %-9.9s", get_attr_alias(victim, ATTR_COC) );

	pager_printf( ch, "&B%s &g| %s &g| &wMano principale   &W%s\r\n",
		column1,
		score_partcorp(victim, 4),
		(victim->right_hand) ? "destra" : "sinistra" );

	/* --- Volontà, Parte del Corpo 5, --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Wil   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_WIL], victim->attr_mod[ATTR_WIL], get_curr_attr(victim, ATTR_WIL) );
	else
		sprintf( column1, " Volontà      %-9.9s", get_attr_alias(victim, ATTR_WIL) );

	pager_printf( ch, "&B%s &g| %s &g| \r\n",
		column1,
		score_partcorp(victim, 5) );

	/* --- Agilità, Parte del Corpo 6, --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Agi   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_AGI], victim->attr_mod[ATTR_AGI], get_curr_attr(victim, ATTR_AGI) );
	else
		sprintf( column1, " Agilità      %-9.9s", get_attr_alias(victim, ATTR_AGI) );

	pager_printf( ch, "&G%s &g| %s &g| \r\n",
		column1,
		score_partcorp(victim, 6) );

	/* --- Riflessi, Parte del Corpo 7, --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Ref   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_REF], victim->attr_mod[ATTR_REF], get_curr_attr(victim, ATTR_REF) );
	else
		sprintf( column1, " Riflessi     %-9.9s", get_attr_alias(victim, ATTR_REF) );

	pager_printf( ch, "&G%s &g| %s &g| \r\n",
		column1,
		score_partcorp(victim, 7) );

	/* --- Velocità, Parte del Corpo 8, --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Spe   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_SPE], victim->attr_mod[ATTR_SPE], get_curr_attr(victim, ATTR_SPE) );
	else
		sprintf( column1, " Velocità     %-9.9s", get_attr_alias(victim, ATTR_SPE) );

	pager_printf( ch, "&G%s &g| %s &g|\r\n",
		column1,
		score_partcorp(victim, 8) );

	/* --- Spiritualità, Parte del Corpo 9, --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Spi   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_SPI], victim->attr_mod[ATTR_SPI], get_curr_attr(victim, ATTR_SPI) );
	else
		sprintf( column1, " Spiritualità %-9.9s", get_attr_alias(victim, ATTR_SPI) );

	pager_printf( ch, "&C%s &g| %s &g| \r\n",
		column1,
		score_partcorp(victim, 9) );

	/* --- Migrazione, Linea Separatrice, Linea Separatrice --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Mig   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_MIG], victim->attr_mod[ATTR_MIG], get_curr_attr(victim, ATTR_MIG) );
	else
		sprintf( column1, " Migrazione   %-9.9s", get_attr_alias(victim, ATTR_MIG) );

	pager_printf( ch, "&C%s &g|-------------------+-------------------------------------\r\n",
		column1 );

	/* --- Assorbimento, Punti Vita, Stato Mentale --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Abs   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_ABS], victim->attr_mod[ATTR_ABS], get_curr_attr(victim, ATTR_ABS) );
	else
		sprintf( column1, " Assorbimento %-9.9s", get_attr_alias(victim, ATTR_ABS) );

	if ( IS_ADMIN(ch) )
		sprintf( column2, "&RVita  %7s&w/&R%5d", get_points_colored(ch, victim, POINTS_LIFE, FALSE), victim->points_max[POINTS_LIFE] );
	else if ( IS_PG(victim) && !HAS_BIT_PLR(victim, PLAYER_SPIRIT) )
		sprintf( column2, "&RVita         %8s", get_points_colored(ch, victim, POINTS_LIFE, FALSE) );
	else
		sprintf( column2, "                     " );

	if ( !IS_ADMIN(ch) )
	{
		if ( victim->position != POSITION_SLEEP )
		{
			switch ( victim->mental_state / 5 )
			{
			  default:	strcpy ( column3, "&zMi sento completamente fuori fase!"		);	break;
			  case -20:	strcpy ( column3, "&zOgni pensiero è un peso. Fa male!"			);	break;
			  case -19: strcpy ( column3, "&zRimango a malapena cosciente.."			);	break;
			  case -18:	strcpy ( column3, "&zUn immenso vuoto mi possiede.."			);	break;
			  case -17:	strcpy ( column3, "&zGli occhi mi si chiudono.."				);	break;
			  case -16:	strcpy ( column3, "&zVedo solo il vuoto.. è buio qui!"			);	break;
			  case -15:	sprintf( column3, "&zSono estremamente assonnat%c", csex		);	break;
			  case -14:	strcpy ( column3, "&zNo! Non ce la faccio ora, non ora.." 		);	break;
			  case -13:	sprintf( column3, "&zMi sento molto demotivat%c", csex			);	break;
			  case -12:	strcpy ( column3, "&zMi si chiudono gli occhi.."				);	break;
			  case -11:	strcpy ( column3, "&zMi sento estremamente stanca"				);	break;
			  case -10:	strcpy ( column3, "&zHo bisogno di riposare.."					);	break;
			  case  -9:	sprintf( column3, "&wMi sento assonnat%c", csex					);	break;
			  case  -8:	strcpy ( column3, "&wNon riesco a concentrarmi.."				);	break;
			  case  -7:	sprintf( column3, "&wMi sento stanc%c", csex					);	break;
			  case  -6:	strcpy ( column3, "&wChe stress.. mi pesa la testa.."			);	break;
			  case  -5:	strcpy ( column3, "&wAvrei bisogno di riposo"					);	break;
			  case  -4:	strcpy ( column3, "&wSospiro.. potrebbe andare peggio"			);	break;
			  case  -3:	sprintf( column3, "&wMi sento leggermente indebolit%c", csex	);	break;
			  case  -2:	strcpy ( column3, "&wMi sento bene"								);	break;
			  case  -1:	strcpy ( column3, "&wMi sento ok"								);	break;
			  case   0:	strcpy ( column3, "&wMi sento in forze"							);	break;
			  case   1:	strcpy ( column3, "&wMi sento meravigliosamente"				);	break;
			  case   2:	strcpy ( column3, "&wMi sento davvero in forze"					);	break;
			  case   3:	sprintf( column3, "&wMi sento pien%c di energie", csex			);	break;
			  case   4:	strcpy ( column3, "&wSono in gran forma"						);	break;
			  case   5:	sprintf( column3, "&wSono sovraeccitat%c", csex					);	break;
			  case   6:	strcpy ( column3, "&wMi sento in splendida forma"				);	break;
			  case   7:	strcpy ( column3, "&wRiesco a concentrarmi bene"				);	break;
			  case   8:	strcpy ( column3, "&wSono in comunione con l'universo"			);	break;
			  case   9:	strcpy ( column3, "&wI miei pensieri sono velocissimi"			);	break;
			  case  10:	strcpy ( column3, "&YSpazio tra i piani con la mente"			);	break;
			  case  11:	strcpy ( column3, "&YHo la testa tra le nuvole"					);	break;
			  case  12:	strcpy ( column3, "&YLo spirito guida mente e corpo"			);	break;
			  case  13:	strcpy ( column3, "&YLa mente è come fuori dal corpo"			);	break;
			  case  14:	strcpy ( column3, "&YNon esiste materia per lo spirito"			);	break;
			  case  15:	strcpy ( column3, "&YLa realtà non ha più senso"				);	break;
			  case  16:	strcpy ( column3, "&YReale e irreale sono due istanti" 			);	break;
			  case  17:	strcpy ( column3, "&YLa differenza tra realtà e fantasia?"		);	break;
			  case  18:	strcpy ( column3, "&YMi sento un'anima vagante"					);	break;
			  case  19:	strcpy ( column3, "&YIo sono spirito"							);	break;
			  case  20:	strcpy ( column3, "&YIo sono il Tutto" 							);	break;
			}
		}
		else if ( victim->mental_state > 45 )
			strcpy ( column3, "&CSogni strani e vividi nel sonno" );
		else if ( victim->mental_state > 25 )
			strcpy ( column3, "&CIl mio sonno è agitato" );
		else if ( victim->mental_state < -35 )
			strcpy ( column3, "&BUn sonno profondo e ristoratore" );
		else if ( victim->mental_state < -25 )
			strcpy ( column3, "&BUn sonno profondo" );
		else
			strcpy( column3, "Shh.. Sto dormendo" );
	}
	else
		sprintf( column3, "&wStato Mentale: &W%3d", victim->mental_state );

	pager_printf( ch, "&C%s &g| %s &g| %s\r\n",
		column1,
		column2,
		column3 );

	/* --- Empatia, Punti Movimento, Fame e Sete e Sobrietà--- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Emp   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_EMP], victim->attr_mod[ATTR_EMP], get_curr_attr(victim, ATTR_EMP) );
	else
		sprintf( column1, " Empatia      %-9.9s", get_attr_alias(victim, ATTR_EMP) );

	if ( IS_ADMIN(ch) )
		sprintf( column2, "&GMovi  %7s&w/&G%5d", get_points_colored(ch, victim, POINTS_MOVE, FALSE), victim->points_max[POINTS_MOVE] );
	else
		sprintf( column2, "&GMovimento    %8s", get_points_colored(ch, victim, POINTS_MOVE, FALSE) );

	if ( IS_PG(victim) )
	{
		if ( IS_ADMIN(ch) )
		{
			sprintf( column3, "&wFame &W%d  &wSete &W%d  &wSobrio &W%d",
				victim->pg->condition[CONDITION_FULL],
				victim->pg->condition[CONDITION_THIRST],
				victim->pg->condition[CONDITION_DRUNK] );
		}
		else
		{
			char sobre[MIL];

			if ( victim->pg->condition[CONDITION_DRUNK] > 10 )
				sprintf( sobre, "&w, sono &Pubriac%c", csex );
			else
				sprintf( sobre, "&w, sono sobri%c", csex );

			if ( victim->pg->condition[CONDITION_THIRST] == 0 )
			{
				if ( victim->pg->condition[CONDITION_FULL] == 0 )
					sprintf( column3, "Muoio di &Ofame&w, di &Csete%s", sobre );
				else
					sprintf( column3, "Muoio di &Csete%s", sobre );
			}
			else
			{
				if ( victim->pg->condition[CONDITION_FULL] == 0 )
					sprintf( column3, "Muoio di &Ofame%s", sobre );
				else
					sprintf( column3, "Non ho fame né sete%s", sobre );
			}
		}
	}
	else
		sprintf( column3, "Mob non hanno fame, sete e sobrietà" );

	pager_printf( ch, "&Y%s &g| %s &g| &w%s\r\n",
		column1,
		column2,
		column3 );

	/* --- Bellezza, Punti Mana, (FF) Sonno e nostalgia di casa --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Bea   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_BEA], victim->attr_mod[ATTR_BEA], get_curr_attr(victim, ATTR_BEA) );
	else
		sprintf( column1, " Bellezza     %-9.9s", get_attr_alias(victim, ATTR_BEA) );

	if ( IS_ADMIN(ch) )
		sprintf( column2, "&CMana  %7s&w/&C%5d", get_points_colored(ch, victim, POINTS_MANA, FALSE), victim->points_max[POINTS_MANA] );
	else
		sprintf( column2, "&CMana         %8s", get_points_colored(ch, victim, POINTS_MANA, FALSE) );


	pager_printf( ch, "&Y%s &g| %s &g|\r\n",
		column1,
		column2 );

	/* --- Comando, punti d'anima, Armatura --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Lea   &W%3d &w+ &W%3d &w= &W%3d", victim->attr_perm[ATTR_LEA], victim->attr_mod[ATTR_LEA], get_curr_attr(victim, ATTR_LEA) );
	else
		sprintf( column1, " Comando      %-9.9s", get_attr_alias(victim, ATTR_LEA) );

	if ( IS_ADMIN(ch) )
		sprintf( column2, "&WAnima %7s&w/&W%5d", get_points_colored(ch, victim, POINTS_SOUL, FALSE), victim->points_max[POINTS_SOUL] );
	else if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_SPIRIT) )
		sprintf( column2, "&CAnima        %8s", get_points_colored(ch, victim, POINTS_SOUL, FALSE) );
	else
		sprintf( column2, "                 " );

	if ( IS_ADMIN(ch) )
		sprintf( column3, "&wAC &W%d", get_ac(victim) );
	else
	{
		if		( get_ac(victim) >=  101 )	sprintf( column3, "&zSono completamente indifes%c", csex	);
		else if ( get_ac(victim) >=   80 )	strcpy ( column3, "&zArmatura inadatta per l'avventura"		);
		else if ( get_ac(victim) >=   55 )	strcpy ( column3, "&zArmatura di pessima qualità"			);
		else if ( get_ac(victim) >=   40 )	strcpy ( column3, "&zArmatura di cattiva qualità"			);
		else if ( get_ac(victim) >=   20 )	strcpy ( column3, "&wArmatura di scarsa qualità"			);
		else if ( get_ac(victim) >=   10 )	strcpy ( column3, "&wL'armatura mi protegge appena"			);
		else if ( get_ac(victim) >=    0 )	strcpy ( column3, "&wL'armatura mi protegge abbastanza"		);
		else if ( get_ac(victim) >=  -10 )	strcpy ( column3, "&wArmatura mi protegge bene"				);
		else if ( get_ac(victim) >=  -20 )	strcpy ( column3, "&CArmatura mi corazza bene"				);
		else if ( get_ac(victim) >=  -40 )	strcpy ( column3, "&CEccellente corazza"					);
		else if ( get_ac(victim) >=  -60 )	strcpy ( column3, "&CArmatura degna di un Cavaliere"		);
		else if ( get_ac(victim) >=  -80 )	strcpy ( column3, "&CArmatura degna di un Barone"			);
		else if ( get_ac(victim) >= -100 )	strcpy ( column3, "&YArmatura degna di un Principe"			);
		else if ( get_ac(victim) >= -200 )	strcpy ( column3, "&YArmatura degna di un Re"				);
		else if ( get_ac(victim) >= -280 )	strcpy ( column3, "&YArmatura degna dell'Imperatore"		);
		else								strcpy ( column3, "&WArmatura degna di una LEGGENDA"		);
	}

	pager_printf( ch, "&Y%s &g| %s &g| %s\r\n",
		column1,
		column2,
		column3 );

	pager_printf( ch, "&g----------------------+-------------------| &wSono liber%c &wda qualsiasi religione\r\n", csex );


	/* --- Vista, HitRoll,  --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Sight &W%3d &w+ &W%3d &w= &W%3d", victim->sense_perm[SENSE_SIGHT], victim->sense_mod[SENSE_SIGHT], get_curr_sense(victim, SENSE_SIGHT) );
	else
		sprintf( column1, " Vista        %-9.9s", get_sense_alias(victim, SENSE_SIGHT) );

	if ( IS_ADMIN(ch) )
	{
		sprintf( column2, "   &W%10d", get_hitroll(victim) );
	}
	else
	{
		if		( get_hitroll(victim) <  1 )	sprintf( column2, "&r*&w------------"	);
		else if ( get_hitroll(victim) <  3 )	sprintf( column2, "&w**&w-----------"	);
		else if ( get_hitroll(victim) <  5 )	sprintf( column2, "&w***&w----------"	);
		else if ( get_hitroll(victim) <  7 )	sprintf( column2, "&Y****&w---------"	);
		else if ( get_hitroll(victim) <  8 )	sprintf( column2, "&Y*****&w--------"	);
		else if ( get_hitroll(victim) < 10 )	sprintf( column2, "&Y******&w-------"	);
		else if ( get_hitroll(victim) < 13 )	sprintf( column2, "&G*******&w------"	);
		else if ( get_hitroll(victim) < 16 )	sprintf( column2, "&G********&w-----"	);
		else if ( get_hitroll(victim) < 20 )	sprintf( column2, "&G*********&w----"	);
		else if ( get_hitroll(victim) < 25 )	sprintf( column2, "&R**********&w---"	);
		else if ( get_hitroll(victim) < 30 )	sprintf( column2, "&R***********&w--"	);
		else if ( get_hitroll(victim) < 35 )	sprintf( column2, "&R************&w-"	);
		else									sprintf( column2, "&C*************"		);
	}

	/* (RR) sarebbe bello cambiare il nome hit in Colpire */
	pager_printf( ch, "&Y%s &g| &wHit %-13s &g| \r\n",
		column1,
		column2 );

	/* --- Udito, DamRoll --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Heari &W%3d &w+ &W%3d &w= &W%3d", victim->sense_perm[SENSE_HEARING], victim->sense_mod[SENSE_HEARING], get_curr_sense(victim, SENSE_HEARING) );
	else
		sprintf( column1, " Udito        %-9.9s", get_sense_alias(victim, SENSE_HEARING) );

	if ( IS_ADMIN(ch) )
		sprintf( column2, "   &W%10d", get_damroll(victim) );
	else
	{
		if		( get_damroll(victim) <  1 )	sprintf( column2, "&r*&w------------" );
		else if ( get_damroll(victim) <  3 )	sprintf( column2, "&w**&w-----------" );
		else if ( get_damroll(victim) <  5 )	sprintf( column2, "&w***&w----------" );
		else if ( get_damroll(victim) <  7 )	sprintf( column2, "&Y****&w---------" );
		else if ( get_damroll(victim) <  8 )	sprintf( column2, "&Y*****&w--------" );
		else if ( get_damroll(victim) < 10 )	sprintf( column2, "&Y******&w-------" );
		else if ( get_damroll(victim) < 13 )	sprintf( column2, "&G*******&w------" );
		else if ( get_damroll(victim) < 16 )	sprintf( column2, "&G********&w-----" );
		else if ( get_damroll(victim) < 20 )	sprintf( column2, "&G*********&w----" );
		else if ( get_damroll(victim) < 25 )	sprintf( column2, "&R**********&w---" );
		else if ( get_damroll(victim) < 30 )	sprintf( column2, "&R***********&w--" );
		else if ( get_damroll(victim) < 35 )	sprintf( column2, "&R************&w-" );
		else									sprintf( column2, "&C*************" );
	}

	/* (RR) sarebbe bello cambiare il nome dam in Danno */
	pager_printf( ch, "&O%s &g| &wDam %13s &g| \r\n",
		column1,
		column2 );

	/* --- Olfatto, Allineamento, Pratiche e Fuga --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Smell &W%3d &w+ &W%3d &w= &W%3d", victim->sense_perm[SENSE_SMELL], victim->sense_mod[SENSE_SMELL], get_curr_sense(victim, SENSE_SMELL) );
	else
		sprintf( column1, " Olfatto      %-9.9s", get_sense_alias(victim, SENSE_SMELL) );

	if ( IS_ADMIN(ch) )
		sprintf( column2, "&W%d", victim->alignment );
	else
		strcpy( column2, get_align_alias(victim->alignment) );

	pager_printf( ch, "&G%s &g| &wAllineam %10.10s &g| &wPratiche &W%3d  &wFuga &W%3d%%\r\n",
		column1,
		column2,
		(IS_PG(victim))  ?  victim->pg->practice  :  -1,
		victim->wimpy );

	/* --- Gusto, Gloria, Monete d'oro --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Taste &W%3d &w+ &W%3d &w= &W%3d", victim->sense_perm[SENSE_TASTE], victim->sense_mod[SENSE_TASTE], get_curr_sense(victim, SENSE_TASTE) );
	else
		sprintf( column1, " Gusto        %-9.9s", get_sense_alias(victim, SENSE_TASTE) );

	pager_printf( ch, "&R%s &g| &wGloria &W%10d &g| &wMonete d'oro &Y%s\r\n",
		column1,
		(IS_PG(victim)) ? victim->pg->glory : 0,
		num_punct(victim->gold) );

	/* --- Tatto, Posizione, --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Touch &W%3d &w+ &W%3d &w= &W%3d", victim->sense_perm[SENSE_TOUCH], victim->sense_mod[SENSE_TOUCH], get_curr_sense(victim, SENSE_TOUCH) );
	else
		sprintf( column1, " Tatto        %-9.9s", get_sense_alias(victim, SENSE_TOUCH) );

	switch ( victim->position )
	{
	  default:
		send_log( NULL, LOG_BUG, "posizione errata numero %d al pg %s.", victim->position, victim->name );
	  	strcpy( column2, "(errore)" );
	  	break;
	  case POSITION_DEAD:		sprintf( column2, "&zMort%c", csex			);		break;
	  case POSITION_MORTAL:		sprintf( column2, "&rFerit%c grave", csex	);		break;
	  case POSITION_INCAP:		sprintf( column2, "&BIncapacitat%c", csex	);		break;
	  case POSITION_STUN:		sprintf( column2, "&cStordit%c", csex		);		break;
	  case POSITION_SLEEP:		strcpy ( column2, "&CDormo"					);		break;
	  case POSITION_BERSERK:		strcpy ( column2, "&RComb berserk"			);		break;
	  case POSITION_REST:		strcpy ( column2, "&gRiposo"				);		break;
	  case POSITION_AGGRESSIVE:	strcpy ( column2, "&RComb &Yaggress."		);		break;
	  case POSITION_SIT:			sprintf( column2, "&GSedut%c", csex			);		break;
	  case POSITION_FIGHT:		strcpy ( column2, "&RCombattendo"			);		break;
	  case POSITION_DEFENSIVE:	strcpy ( column2, "&RComb &Gdifensivo"		);		break;
	  case POSITION_EVASIVE:		strcpy ( column2, "&RComb &Cevasivo"		);		break;
	  case POSITION_STAND:		sprintf( column2, "&wAlzat%c", csex			);		break;
	  case POSITION_MOUNT:		strcpy ( column2, "&OCavalco"				);		break;
	  case POSITION_SHOVE:		strcpy ( column2, "&OSpingo"				);		break;
	  case POSITION_DRAG:		strcpy ( column2, "&OTrascino"				);		break;
	}

	if ( IS_ADMIN(ch) )
	{
		sprintf( column3, "&wLiv &W%d  &wExp &W%d  &wExpTo &W%d",
			victim->level,
			victim->exp,
			exp_level(victim, victim->level+1) - victim->exp );
	}
	else
	{
		int	percent;

		strcpy( column3, "&wEsperienza &C|&W" );
		/* Percentuale del totale dei px per livellare rispetto al totale dei px di quel livello */
		percent = get_percent( victim->exp, exp_level(victim, victim->level+1) );
		percent /= 5;	/* ogni crocetta un 5% */
		/* Prepara la stringa che rappresenta l'avanzamento di exp */
		for ( x = 0;  x <= 20;  x++ )
		{
			if ( x <= percent )
				strcat( column3, "+" );
			else
				strcat( column3, " " );
		}
		strcat( column3, "&C|" );
	}

	pager_printf( ch, "&P%s &g| &wPos &W%s%*s &g| %s\r\n",
		column1,
		column2, 13 - strlen_nocolor(column2), "",
		column3 );

	/* --- Intuito, Stile, --- */
	if ( IS_ADMIN(ch) )
		sprintf( column1, "Sixth &W%3d &w+ &W%3d &w= &W%3d", victim->sense_perm[SENSE_SIXTH], victim->sense_mod[SENSE_SIXTH], get_curr_sense(victim, SENSE_SIXTH) );
	else
		strcpy( column1, "                     " );

	/* Fighting style support. */
	strcpy( column2, get_style_name(victim) );

	pager_printf( ch, "%s &g| &wStile &W%13s &g|\r\n", column1, column2 );

	/* Linea Separatrice */
	send_to_pager( ch, "--------------------------------------------------------------------------------\r\n" );

	/* --- Informazioni riguardanti il morphing ---*/
	if ( victim->morph && victim->morph->morph )
	{
		send_to_pager( ch, "&W----------------------------------------------------------------------------&C\r\n" );
		if ( IS_ADMIN(ch) )
		{
			pager_printf( ch, "Morphed as (%d) %s with a timer of %d.\r\n",
				victim->morph->morph->vnum, victim->morph->morph->short_desc, victim->morph->timer );
		}
		else
			pager_printf( ch, "You are morphed into a %s.\r\n", victim->morph->morph->short_desc );

		send_to_pager( ch, "&W----------------------------------------------------------------------------&C\r\n" );
	}

	if ( IS_ADMIN(ch) && IS_PG(victim) )
	{
		pager_printf( ch, "&wPkills: &W%3d&w  Illegal Pkills (&W%3d&w)  Pdeaths (&W%d&w)\r\n",
			victim->pg->pkills, victim->pg->illegal_pk, victim->pg->pdeaths );
	}

	/* (RR) spostarlo nel comando do_affect */
	one_argument( argument, arg );
	if ( victim->first_affect && (!str_cmp(arg, "affezioni") || !str_cmp(arg, "affects")) )
	{
		SKILL_DATA *sktmp;

		x = 0;
		send_to_pager( ch, "&W----------------------------------------------------------------------------&C\r\n" );
		send_to_pager( ch, "AFFECT DATA:							" );

		for ( paf = victim->first_affect;  paf;  paf = paf->next )
		{
			if ( (sktmp = get_skilltype(paf->type)) == NULL )
				continue;

			if ( !IS_ADMIN(ch) )
			{
				pager_printf( ch, "&C[&W%-34.34s&C]	", sktmp->name );
				if ( x == 0 )
					x = 2;
				if ( (++x%3) == 0 )
					send_to_pager( ch, "\r\n" );
			}
			else
			{
				if ( paf->modifier == 0 )
				{
					pager_printf( ch, "&C[&W%-24.24s;%5d &Crds]	",
					sktmp->name,
					paf->duration );
				}
				else if ( paf->modifier > 999 )
				{
					pager_printf( ch, "&C[&W%-15.15s; %7.7s;%5d &Crds]	",
						sktmp->name,
						code_name(NULL, paf->location, CODE_APPLY),
						paf->duration );
				}
				else
				{
					pager_printf( ch, "&C[&W%-11.11s;%+-3.3d %7.7s;%5d &Crds]    ",
						sktmp->name,
						paf->modifier,
						code_name(NULL, paf->location, CODE_APPLY),
						paf->duration );
				}
				if ( x == 0 )
					x = 1;
				if ( (++x % 2) == 0 )
					send_to_pager( ch, "\r\n" );
			}
		}
	} /* chiude l'if */

	/* Informazioni per amministratori in realtà sarebbero, per comodità lo tengo */
	if ( IS_ADMIN(ch) )
	{
		if ( IS_PG(victim) )
		{
			char	logon_time[MSL];
			char	logon_old_time[MSL];
			char	server_time[MSL];
			char	save_time[MSL];

			/* --- Ora del logon e ora del server --- */
			strcpy( logon_time, friendly_ctime(&victim->pg->logon) );
			strcpy( logon_old_time, friendly_ctime(&victim->pg->logon_old) );
			pager_printf( ch, "&wOra login  &W%21.21s   &wLogin prec &W%21.21s\r\n",
				logon_time, logon_old_time );

			/* --- Ora dell'ultimo logon e ora del salvataggio se esiste */
			strcpy( server_time, friendly_ctime(&current_time) );
			strcpy( save_time, victim->pg->save_time  ?  friendly_ctime(&victim->pg->save_time)  :  "Nessuna ancora." );
			pager_printf( ch, "&wOra boot   &W%21.21s   &wOra Save   &W%21.21s\r\n",
				server_time, save_time );
		}

		/* --- Visualizza se il pg utilizza l'mccp e l'mxp --- */
		if ( IS_PG(victim) )
		{
#ifdef T2_MSP
			pager_printf( ch, "&wMSP: %s&w/%s&w   ",
				(victim->desc && victim->desc->msp)  ?  "&GSI"  :  "&rno",
				(use_msp(victim->desc))  ?  "&GSI"  :  "&rno" );
#endif
#ifdef T2_MCCP
			pager_printf( ch, "&wMCCP: %s&w/%s&w   ",
				(victim->desc && victim->desc->mccp)  ?  "&GSI"  :  "&rno",
				(use_mccp(victim->desc))  ?  "&GSI"  :  "&rno" );
#endif
#ifdef T2_MXP
			pager_printf( ch, "&wMXP: %s&w/%s&w   ",
				(victim->desc && victim->desc->mxp)  ?  "&GSI"  :  "&rno",
				(use_mxp(victim->desc))  ?  "&GSI"  :  "&rno" );
#endif
			pager_printf( ch, "&cIncognito: &W%3d   &wFiducia: &W%s\r\n",
				victim->pg->incognito,
				code_name(NULL, get_trust(victim), CODE_TRUST) );	/* (GR) gestire in futuro e creare nella gramm.c le robe per il suo-sua e maschile e femminile */

			pager_printf( ch, "&wGloria accumulata &W%5d   &wMorti &W%3d   &wUccisioni &W%5d\r\n",
				victim->pg->glory_accum,
				victim->pg->mdeaths,
				victim->pg->mkills );

			if ( victim->pg->build_area )
			{
				pager_printf( ch, "&gVnums  : &R%-5.5d &w- &R%-5.5d\r\n",
					victim->pg->build_area->vnum_low, victim->pg->build_area->vnum_high );
				pager_printf( ch, "&p         Area Loaded &W[%s&W]\r\n",
					(HAS_BIT(victim->pg->build_area->flags, AREAFLAG_LOADED)) ? "&GSI" : "&rno" );
			}
		}
		else
		{
			if ( HAS_BIT_ACT(victim, MOB_INCOGNITO) )
				pager_printf( ch, "You are mobinvis at level %d.\r\n", victim->mobinvis );
		}
	}
}


/*
 * Comando score (ex newscore).
 */
DO_RET do_score( CHAR_DATA *ch, char *argument )
{
	score_handler( ch, ch, argument );
}


/*
 * (FF) Comando monete, conta oro, conta argento, conta rame o altra moneta,
 * conta tutto per un conteggio calmo e preciso (rovista nella tasche)
 */
DO_RET do_money( CHAR_DATA * ch, char *argument )
{
	if ( !VALID_STR(argument) )
	{
		set_char_color( AT_GOLD, ch );
		ch_printf( ch,  "Possiedi %s pezzi d'oro.\r\n", num_punct(ch->gold) );
		return;
	}

	give_money( ch, argument );
}


DO_RET do_remains( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA	*obj;
	CHAR_DATA	*mob;
	char		 buf[MSL];
	int			 cost;
	bool		 found = FALSE;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono chiedere di recuperare i cadaveri.\r\n" );
		return;
	}

	/* Cerca il mob con l'ACT_MORGUE */
	for ( mob = ch->in_room->first_person;  mob;  mob = mob->next_in_room )
	{
		if ( IS_MOB(mob) && HAS_BIT_ACT(mob, MOB_MORGUE) )
			break;
	}

	if ( !mob )
	{
		send_to_char( ch, "Qui non trovo nessuno che mi potrebbe recuperare il mio cadavere.\r\n" );
		return;
	}

	if ( !can_see(mob, ch) )
	{
		sprintf( buf, "say Non riesco a vederti, non posso riconoscere la fisonomia del tuo cadavere." );
		send_command( mob, buf, CO );
		return;
	}

	for ( obj = first_object;  obj;  obj = obj->next )
	{
		char  name[MIL];

		if ( !obj->in_room )					continue;
		if ( obj->vnum != VNUM_OBJ_CORPSE_PC )	continue;

		/* Recupera il nome del cadavere dalla short dell'oggetto */
		one_argument( obj->name, name );

		if ( !str_cmp(ch->name, name) )
		{
			found = TRUE;
			break;		/* quindi ne trova uno per volta nel caso ve ne siano più di uno */
		}
	}

	if ( !found )
	{
		sprintf( buf, "say a %s Non credo che vi siano cadaveri con la tua fisionomia.", ch->name );
		send_command( mob, buf, CO );
		return;
	}

	sprintf( buf, "say a %s Sì, posso trovare il cadavere che corrisponde alla tua descrizione..", ch->name );
	send_command( mob, buf, CO );

	cost = 100;
	cost += ( (ch->level+1)/2 ) * 100;
	cost = compute_cost( ch, mob, cost );

	if ( ch->gold < cost )
	{
		sprintf( buf, "say a %s Vedo che ti manca un po' di moneta per riottenerlo, ti costerà %d.", ch->name, cost );
		send_command( mob, buf, CO );

		return;
	}

	ch_printf( ch, "\r\nMisteriosamente %s se ne va, tornando un po' di tempo dopo con il tuo cadavere!\r\n\r\n", mob->short_descr );

	WAIT_STATE( mob, PULSE_IN_SECOND * 5 );	/* (TT) più o meno un tentativo di mettere una pausa alla caccia del becchino, cmq è una cosa che risolve solo con gli eventi */

	obj_from_room( obj );
	obj = obj_to_char( obj, mob );
	obj->timer = -1;

	/* (FF) qui cambiarla in maniera tale che se il comando ritorna ch_ret di fallimento posi il cadavere */
	sprintf( buf, "give %s %s", ch->name, ch->name );
	send_command( mob, buf, CO );
}


DO_RET do_inventory( CHAR_DATA *ch, char *argument )
{
	set_char_color( AT_RED, ch );
	send_to_char( ch, "Sto trasportando:\r\n" );
#ifdef T2_MXP
	show_list_to_char( ch->first_carrying, ch, TRUE, TRUE, eItemDrop );
#else
	show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
#endif

	ch_printf( ch, "&wSto trasportando &W%d&w oggetti per un peso totale di &W%d&w chili.\r\n",
		ch->carry_number, ch->carry_weight/1000 );
	ch_printf( ch, "Stimo che il mio massimo carico possibile sia di &W%d&w oggetti e &W%d&w chili.\r\n",
		can_carry_number(ch), can_carry_weight(ch)/1000 );
}


/*
 * Elenca l'equipaggiamento vestito.
 * Se si passa come argomento nome di altrui pg elenca l'equip di tale pg.
 */
DO_RET do_equipment( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim = NULL;	/* Indica quale pg si è riferiti */
	OBJ_DATA   *obj;
	int			x;
	bool		found = FALSE;	/* Se falso indica che non sono stati trovati oggetti addosso */
	bool		dark = FALSE;	/* Se vero indica che non sono stati trovati degli oggetti perché la stanza è buia */

	if ( !VALID_STR(argument) )
	{
		victim = ch;
	}
	else
	{
		victim = get_char_room( ch, argument, TRUE );
		if ( !victim )
		{
			ch_printf( ch, "Non vedo nessun %s qui attorno.\r\n", argument );
			return;
		}
	}

	set_char_color( AT_RED, ch );
	ch_printf( ch, "St%c utilizzando:\r\n", (victim == ch) ? 'o' : 'a' );

	set_char_color( AT_OBJECT, ch );

	for ( x = 0;  x < MAX_WEARLOC;  x++ )
	{
		for ( obj = victim->first_carrying;  obj;  obj = obj->next_content )
		{
			if ( obj->wear_loc != x )				continue;

			if ( room_is_dark(ch->in_room) && !can_see_obj_dark(ch, obj, FALSE) )
				dark = TRUE;

			if ( !can_see_obj(ch, obj, FALSE) )		continue;

			/* Se non è umano */
			if ( IS_PG(victim) && (victim->race > 0 && victim->race < MAX_RACE) )
				send_to_char( ch, table_race[victim->race]->wear_name[x] );
			else
				send_to_char( ch, wear_name[x] );	/* (RR) dovrebbero utilizzare tutti la tabella di razza wear */

			if ( can_see_obj(ch, obj, FALSE) )
			{
				int cond = -1;

				switch ( obj->type )
				{
				  case OBJTYPE_ARMOR:
					cond = (int) ( obj->armor->ac * 10/obj->armor->ac_orig );
					if ( !IS_ADMIN(ch) )
						send_to_char( ch, "&C<&R" );
					else
						ch_printf( ch, "&C<&R%3.3d", cond );
					break;

				  case OBJTYPE_WEAPON:
					cond = (int) ( obj->weapon->condition *10/12 );
					if ( !IS_ADMIN(ch) )
						send_to_char( ch, "&C<&R" );
					else
						ch_printf( ch, "&C<&R%3.3d", cond );
					break;
				}

				if ( !IS_ADMIN(ch) )
				{
					if ( cond >= 0 )
					{
						int y;

						for ( y = 1;  y <= 10;  y++ )
						{
							if ( y <= cond )
								send_to_char( ch, "+" );
							else
								send_to_char( ch, " " );
						}
					}
				}
				if ( IS_ADMIN(ch) )
					ch_printf( ch, "&C>&W             %s&w\r\n", format_obj_to_char(obj, ch, TRUE) );
				else if ( obj->type == OBJTYPE_WEAPON || obj->type == OBJTYPE_ARMOR )
					ch_printf( ch, "&C>&W  %s&w\r\n", format_obj_to_char(obj, ch, TRUE) );
				else
					ch_printf( ch, "              %s&w\r\n", format_obj_to_char(obj, ch, TRUE) );
			}
			else
			{
				send_to_char( ch, "                            qualcosa.\r\n" );
			}

			found = TRUE;
		}
	}

	if ( !found )
		send_to_char( ch, "Nulla.\r\n" );

	if ( dark )
		send_to_char( ch, "&zLa stanza è buia, potrei non aver visto tutti gli oggetti.&w\r\n" );

	if ( IS_MOB(ch) || victim == ch )
		return;

	if ( IS_ADMIN(ch) || (number_percent() < knows_skell(ch, gsn_peek)) )
	{
		send_to_char( ch, "\r\nFrughi nel suo inventario:\r\n" );
#ifdef T2_MXP
		show_list_to_char( victim->first_carrying, ch, TRUE, TRUE, eItemNothing );
#else
		show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
#endif
		learn_skell( ch, gsn_peek, TRUE );
	}
	else if ( ch->pg->learned_skell[gsn_peek] > 0 )
		learn_skell( ch, gsn_peek, FALSE );
}


void set_title( CHAR_DATA *ch, char *title )
{
	char	buf[MSL];

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "set_title: ai mob non è permesso settare title." );
		return;
	}

	/* Aggiunge il primo carattere di spazio se già non v'è */
	if ( title[0] != ' ' )
		sprintf( buf, " %s", strcolor_close(title) );
	else
		strcpy( buf, strcolor_close(title) );

	DISPOSE( ch->pg->title );
	ch->pg->title = str_dup( buf );
}


DO_RET do_title( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
		return;

	set_char_color( AT_SCORE, ch );
	if ( HAS_BIT_PLR(ch, PLAYER_NOTITLE) )
	{
		set_char_color( AT_PLAIN, ch );
		send_to_char( ch, "Non voglio nessun nuovo titolo, sono sempre assurdi!\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		if ( !VALID_STR(ch->pg->title) )
		{
			send_to_char( ch, "Quale titolo potrei avere?\r\n" );
		}
		else
		{
			send_to_char( ch, "Per dimenticarmi del mio titolo devo pensare a:\r\n" );
			ch_printf	( ch, "%s dimentica\r\n", translate_command(ch, "title") );
		}

		return;
	}

	if ( !str_cmp(argument, "dimentica") )
	{
		DISPOSE( ch->pg->title );
		ch->pg->title = str_dup( "" );
		send_to_char( ch, "Per ora cercherò di dimenticare il mio titolo.\r\n" );
		return;
	}

	if ( strlen_nocolor(argument) > MAX_LEN_TITLE-1 )
	{
		send_to_char( ch, "Troppo lungo! Non me lo ricorderei proprio!\r\n" );
		return;
	}

	smash_tilde( argument );	/* (bb) ed anche smash % direi, e anche rimuovere i colori doppi */
	set_title( ch, argument );
	send_log( NULL, LOG_TITLE, "do_title: %s setta il title a: %s", ch->name, argument );
	send_to_char( ch, "Bene! Chissà che il mio nome e titolo un giorno non verranno cantati dai bardi.\r\n" );
}


/*
 * Comando per settare il cognome o luogo di nascita del pg
 */
DO_RET do_surname( CHAR_DATA *ch, char *argument )
{
	char	*pc;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_surname: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono avere un cognome!\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		if ( !VALID_STR(ch->pg->title) )
		{
			send_to_char( ch, "Quale sarà il mio cognome? Che vuoto di memoria!\r\n" );
		}
		else
		{
			send_to_char( ch, "Per dimenticarmi del mio cognome devo pensare a:\r\n" );
			ch_printf	( ch, "%s dimentica\r\n", translate_command(ch, "surname") );
		}
		return;
	}

	if ( strlen_nocolor(argument) < 2 )
	{
		send_to_char( ch, "Così il mio cognome sarebbe troppo corto per essere significativo..\r\n" );
		return;
	}

	if ( strlen_nocolor(argument) > 16 )
	{
		send_to_char( ch, "Così il mio cognome sarebbe troppo lungo, non esageriamo!\r\n" );
		return;
	}

	if ( !str_cmp(argument, "dimentica") )
	{
		DISPOSE( ch->pg->surname );
		ch->pg->surname = str_dup( "" );
		send_to_char( ch, "Per ora cercherò di dimenticare il mio cognome.\r\n" );
		return;
	}

	for ( pc = argument;  VALID_STR(pc);  pc++ )
	{
		if ( !isalpha(*pc) && *pc != '\'' && !is_accent(*pc) )
		{
			send_to_char( ch, "Non posso avere un cognome di questo genere!\r\n" );
			return;
		}
	}

	smash_tilde( argument );	/* (bb) ed anche smash % direi, e anche rimuovere i colori doppi */
	ch_printf( ch, "D'ora in poi il mio cognome sarà: %s", argument );
	DISPOSE( ch->pg->surname );
	ch->pg->surname = str_dup( argument );
}


/*
 * Comando per personalizzare la propria descrizione
 * (FF) sarebbe bello unire i vari comandi di editing tra di loro
 * (FF) idem per la descrizione del mascotte, però devo dire che se si aggiunge la victim nelle funzioni DO_RET la cosa può funzionare
 */
DO_RET do_description( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Monsters are too dumb to do that!\r\n" );
		return;
	}

	if ( !ch->desc )
	{
		send_log( NULL, LOG_BUG, "do_description: no descriptor" );
		return;
	}

	switch ( ch->substate )
	{
	  default:
		send_log( NULL, LOG_BUG, "do_description: illegal substate" );
		return;

	  case SUB_RESTRICTED:
		send_to_char( ch, "You cannot use this command from within another command.\r\n" );
		return;

	  case SUB_NONE:
		ch->substate = SUB_PERSONAL_DESCR;
		ch->dest_buf = ch;
		start_editing( ch, ch->description );
		return;

	  case SUB_PERSONAL_DESCR:
		DISPOSE( ch->description );
		ch->description = copy_buffer( ch );
		stop_editing( ch );
		return;
	}
}


/*
 * Comando mascotte per gestire alcune azioni sui cuccioli
 */
DO_RET do_mascotte( CHAR_DATA *ch, char *argument )
{
	char	arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_whistle: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono avere Cuccioli.\r\n" );
		return;
	}

	if ( !ch->pg->mascotte )
	{
		send_to_char( ch, "Non posseggo nessun cucciolo al momento.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		char	buf[MIL];

		send_to_char( ch, "Ecco come posso interagire con il mio cucciolo:\r\n" );
		send_to_char( ch, "richiama\r\n" );
		send_to_char( ch, "abbandona\r\n" );
		if ( sysdata.mascotte_restring )
		{
			send_to_char( ch, "cambia nome  <frase>\r\n" );
			send_to_char( ch, "cambia corta <frase>\r\n" );
			send_to_char( ch, "cambia lunga <frase>\r\n" );
			send_to_char( ch, "cambia descr <frase>\r\n" );
		}
#ifdef T2_ALFA
		send_to_char( ch, "affida <nomepg>\r\n" );
		send_to_char( ch, "regala <nomepg>\r\n" );
#endif

		sprintf( buf, "look %s", ch->pg->mascotte->name );
		send_command( ch, "look %s", CO );

		return;
	}

	argument = one_argument( argument, arg );
	if ( !str_cmp(arg, "richiama") )
	{
		if ( ch->in_room->area == ch->pg->mascotte->in_room->area )
		{
			act( AT_ACTION, "$N sentendo il mio richiamo torna in fretta da me.", ch, NULL, ch->pg->mascotte, TO_CHAR );
			act( AT_ACTION, "$N sentendo il richiamo del suo padrone torna in fretta.", ch, NULL, ch->pg->mascotte, TO_ROOM );

			char_from_room( ch->pg->mascotte );
			char_to_room( ch->pg->mascotte, ch->in_room );
		}
		else
		{
			send_to_char( ch, "Il tuo cucciolo non sembra essere nei paraggi.\r\n" );
		}
	}

	if ( !str_cmp(arg, "abbandona") )
	{
		act( AT_ACTION, "E' dura, ma abbandono $N al suo destino.", ch, NULL, ch->pg->mascotte, TO_CHAR );
		act( AT_ACTION, "$n abbandona il suo cucciolo $N.", ch, NULL, ch->pg->mascotte, TO_ROOM );
		stop_follower( ch->pg->mascotte );
	}

	if ( sysdata.mascotte_restring )
	{
		if ( !str_cmp(arg, "cambia") )
		{
			char	arg2[MIL];
	
			argument = one_argument( argument, arg2 );
			if ( is_name_prefix(arg2, "nome name nomi keyword keywords") )
			{
				DISPOSE( ch->pg->mascotte->name );
				ch->pg->mascotte->name = str_dup( argument );
			}
			if ( is_name_prefix(arg2, "corta short") )
			{
				DISPOSE( ch->pg->mascotte->short_descr );
				ch->pg->mascotte->short_descr = str_dup( argument );
			}
			if ( is_name_prefix(arg2, "lunga long") )
			{
				DISPOSE( ch->pg->mascotte->long_descr );
				ch->pg->mascotte->long_descr = str_dup( argument );
			}
			if ( is_name_prefix(arg2, "descrizione description") )
			{
				if ( !ch->desc )
				{
					send_log( NULL, LOG_BUG, "do_mascotte: il descrittore è NULL" );
					return;
				}
			
				switch ( ch->substate )
				{
				  default:
					send_log( NULL, LOG_BUG, "do_description: illegal substate" );
					return;
			
				  case SUB_RESTRICTED:
					send_to_char( ch, "You cannot use this command from within another command.\r\n" );
					return;
			
				  case SUB_NONE:
					ch->substate = SUB_PERSONAL_DESCR;
					ch->dest_buf = ch;
					start_editing( ch, ch->pg->mascotte->description );
					return;
			
				  case SUB_PERSONAL_DESCR:
					DISPOSE( ch->pg->mascotte->description );
					ch->pg->mascotte->description = copy_buffer( ch );
					stop_editing( ch );
					return;
				}
			}
		}
	}

	send_command( ch, "mascotte", CO );
}


/*
 * Setta la descrizione psicologica del pg
 */
DO_RET do_character( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Monsters are too dumb to do that!\r\n" );
		return;
	}

	if ( ch->desc == NULL )
	{
		send_log( NULL, LOG_BUG, "do_description: no descriptor" );
		return;
	}

	switch ( ch->substate )
	{
	  default:
		send_log( NULL, LOG_BUG, "do_description: illegal substate" );
		return;

	  case SUB_RESTRICTED:
		send_to_char( ch, "You cannot use this command from within another command.\r\n" );
		return;

	  case SUB_NONE:
		ch->substate = SUB_PERSONAL_CHARACTER;
		ch->dest_buf = ch;
		start_editing( ch, ch->pg->personality );
		return;

	  case SUB_PERSONAL_CHARACTER:
		DISPOSE( ch->pg->personality );
		ch->pg->personality = copy_buffer( ch );
		stop_editing( ch );
		return;
	}
}


/*
 * Comando che permette al giocatore di scrivere la propria biografia.
 */
DO_RET do_biography( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Mobs cannot set a bio.\r\n" );
		return;
	}

	if ( ch->desc == NULL )
	{
		send_log( NULL, LOG_BUG, "do_bio: no descriptor" );
		return;
	}

	switch ( ch->substate )
	{
	  default:
		send_log( NULL, LOG_BUG, "do_bio: illegal substate" );
		return;

	  case SUB_RESTRICTED:
		send_to_char( ch, "You cannot use this command from within another command.\r\n" );
		return;

	  case SUB_NONE:
		ch->substate = SUB_PERSONAL_BIO;
		ch->dest_buf = ch;
		start_editing( ch, ch->pg->bio );
		return;

	  case SUB_PERSONAL_BIO:
		DISPOSE( ch->pg->bio );
		ch->pg->bio = copy_buffer( ch );
		stop_editing( ch );
		return;
	}
}


/*
 * Permette di personalizzare alcuni messaggi riguardanti il player
 */
DO_RET do_messages( CHAR_DATA *ch, char *argument )
{
	char	arg1[MIL];
	char	arg2[MIL];

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono creare dei messaggi personalizzati.\r\n" );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(argument) )
	{
		send_to_char( ch, "Puoi modificare i messaggi del tuo personaggio di:\r\n" );
		send_to_char( ch, "&glogin&w tu\r\n" );
		send_to_char( ch, "&glogin&w stanza\r\n" );
		send_to_char( ch, "&glogout&w tu\r\n" );
		send_to_char( ch, "&glogout&w stanza\r\n\r\n" );
#ifdef T2_ALFA
		send_to_char( ch, "morte tu\r\n" );
		send_to_char( ch, "morte stanza\r\n" );
		send_to_char( ch, "uccisione tu\r\n" );
		send_to_char( ch, "uccisione stanza\r\n" );
		send_to_char( ch, "portale1 tu\r\n" );
		send_to_char( ch, "portale1 stanza\r\n" );
		send_to_char( ch, "portale2 tu\r\n" );
		send_to_char( ch, "portale2 stanza\r\n\r\n" );
#endif

		/* (RR) da spostarli in una sezione di help apposita con maggiori info ed esempi */
		send_to_char( ch, "&gLogin&w riguarda i messaggi di entrata in gioco.\r\n" );
		send_to_char( ch, "&gLogout&w riguarda i messaggi di uscita in gioco.\r\n" );
		send_to_char( ch, "L'argomento tu riguarda i messaggi da inviare a te.\r\n" );
		send_to_char( ch, "L'argomento stanza riguarda i messaggi da agli altri.\r\n" );
		send_to_char( ch, "Miraccomando, create messaggi con un minimo di \"buon gusto rpg-fantasy\"\r\n\r\n" );

		if ( VALID_STR(ch->pg->msg_login_you) )
			ch_printf( ch, "Login tu: %s\r\n", ch->pg->msg_login_you );
		if ( VALID_STR(ch->pg->msg_login_room) )
			ch_printf( ch, "Login stanza: %s\r\n", ch->pg->msg_login_room );
		if ( VALID_STR(ch->pg->msg_logout_you) )
			ch_printf( ch, "Logout tu: %s\r\n", ch->pg->msg_logout_you );
		if ( VALID_STR(ch->pg->msg_logout_room) )
			ch_printf( ch, "Logout stanza: %s\r\n", ch->pg->msg_logout_room );

		return;
	}

	if ( strchr(argument, '%') )
	{
		send_to_char( ch, "Messaggio contenente uno o più caretteri non validi: %" );
		return;
	}

	if ( !str_cmp(arg1, "login") )
	{
		if ( !str_cmp(arg2, "tu") )
		{
			DISPOSE( ch->pg->msg_login_you );
			ch->pg->msg_login_you = str_dup( argument );
			return;
		}

		if ( !str_cmp(arg2, "stanza") )
		{
			DISPOSE( ch->pg->msg_login_room );
			ch->pg->msg_login_room = str_dup( argument );
			return;
		}

		send_to_char( ch, "Tipo di obiettivo del messaggio errato.\r\n" );
		return;
	}

	if ( !str_cmp(arg1, "logout") )
	{
		if ( !str_cmp(arg2, "tu") )
		{
			DISPOSE( ch->pg->msg_logout_you );
			ch->pg->msg_logout_you = str_dup( argument );
			return;
		}

		if ( !str_cmp(arg2, "stanza") )
		{
			DISPOSE( ch->pg->msg_logout_room );
			ch->pg->msg_logout_room = str_dup( argument );
			return;
		}

		send_to_char( ch, "Tipo di obiettivo del messaggio errato.\r\n" );
		return;
	}

	send_command( ch, "messages", CO );
}


/*
 * Comando che permette di inserire l'obiettivo che il pg vuole
 */
DO_RET do_objective( CHAR_DATA *ch, char *argument )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_objective: ch è NULL" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		if ( VALID_STR(ch->pg->objective) )
		{
			send_to_char( ch, "Per rimuovere il proprio obiettivo devi dimenticarlo:\r\n" );
			ch_printf	( ch, "%s dimentica", translate_command(ch, "objective") );
		}
		else
		{
			send_to_char( ch, "Quale obiettivo vorresti perseguire?" );
		}
		return;
	}

	if ( !str_cmp(argument, "dimentica") )
	{
		if ( VALID_STR(ch->pg->objective) )
			DISPOSE( ch->pg->objective );
		else
			send_to_char( ch, "Non ho nessuno obiettivo da dimenticare per il momento.\r\n" );
		return;
	}

	if ( strlen_nocolor(argument) > 60 )
	{
		send_to_char( ch, "Il mio obiettivo è troppo lungo devo cercare di esprimerlo in maniera più succinta.\r\n" );
		return;
	}

	DISPOSE( ch->pg->objective );
	ch->pg->objective = str_dup( argument );
	send_to_char( ch, "D'ora in avanti avrò un mio obiettivo da seguire!\r\n" );
}

/* (FF) fare il comando permessi per gestire le automazioni riguardo il si e no, oppure permettere sempre all'ultimo a cui è stato detto di sì di fare un'azione, oppure sempre la stessa, e forse creare sistemi con lista di nomi da permettere o da escludere a priori, magari differenziata per comando... argh vado troppo in là....... */

/*
 * Comandi per gestire i permessi di base
 */
void do_yes( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	char		 arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_yes: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(ch->pg->permission_action) )
	{
		send_log( NULL, LOG_BUG, "do_yes: azione del permesso non valida, inviata da %s, ricevuta da %s.\r\n",
			ch->pg->permission_sender ? ch->pg->permission_sender->name : "sconosciuto", ch->name );
		return;
	}

	victim = ch->pg->permission_sender;
	strcpy( arg, ch->pg->permission_action );

	/* Ora che abbiamo acquisito i valori temporanei dell'azione li si pulisce */
	ch->pg->permission_sender = NULL;
	DISPOSE( ch->pg->permission_action );

	if ( !victim )
	{
		send_to_char( ch, "Nessuno mi ha chiesto qualcosa.\r\n" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_log( NULL, LOG_BUG, "do_no: victim è un mob: %s #%d",
			victim->short_descr, victim->pIndexData->vnum );
		return;
	}

	if ( char_died(victim) || victim->in_room != ch->in_room )
	{
		ch_printf( ch, "%s se n'è andato.\r\n", PERS(victim, ch) );
		return;
	}

	act( AT_SAY, "$n dice di sì a $N.", ch, NULL, victim, TO_ROOM );
	act( AT_SAY, "$n mi dice di sì.", ch, NULL, victim, TO_VICT );
	act( AT_SAY, "Dico di sì a $N.", ch, NULL, victim, TO_CHAR );

	victim->pg->permission_answer = TRUE;
	victim->pg->permission_receiver = ch;
	interpret( victim, arg );
	if ( victim )	/* Check nel caso che venga estratto nell'azione */
		victim->pg->permission_answer = FALSE;
}

void do_no( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_no: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(ch->pg->permission_action) )
	{
		send_log( NULL, LOG_BUG, "do_no: azione del permesso non valida, inviata da %s, ricevuta da %s.\r\n",
			ch->pg->permission_sender ? ch->pg->permission_sender->name : "sconosciuto", ch->name );
		return;
	}

	victim = ch->pg->permission_sender;

	/* Rimuove l'offerta */
	ch->pg->permission_sender = NULL;
	DISPOSE( ch->pg->permission_action );

	if ( !victim )
	{
		send_to_char( ch, "Nessuno mi ha chiesto qualcosa.\r\n" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_log( NULL, LOG_BUG, "do_no: victim è un mob: %s #%d",
			victim->short_descr, victim->pIndexData->vnum );
		return;
	}

	if ( char_died(victim) || victim->in_room != ch->in_room )
	{
		ch_printf( ch, "%s se n'è andato.\r\n", PERS(victim, ch) );
		return;
	}

	act( AT_SAY, "$n dice di no a $N.", ch, NULL, victim, TO_ROOM );
	act( AT_SAY, "$n mi dice di no.", ch, NULL, victim, TO_VICT );
	act( AT_SAY, "Dico di no a $N", ch, NULL, victim, TO_CHAR );

	victim->pg->permission_answer = FALSE;
	victim->pg->permission_receiver = NULL;
}


/*
 * Funzione che gestisce il comando prompt e fprompt
 */
DO_RET prompt_handler( CHAR_DATA *ch, char *argument, bool fprompt )
{
	char arg[MIL];

	set_char_color( AT_GREY, ch );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono modificare il proprio prompt.\r\n" );
		return;
	}

	smash_tilde( argument );
	one_argument( argument, arg );

	if ( !VALID_STR(arg) || is_name(arg, "mostra display") )
	{
		set_char_color( AT_GREY, ch );
		send_to_char( ch, "Lista dei prompt di default, impostali digitanto &Gprompt&w <&gtipo di prompt&w>:\r\n" );
		send_to_char( ch, "default\r\n" );
		send_to_char( ch, "classico\r\n" );
		send_to_char( ch, "classico2\r\n" );
		if ( IS_ADMIN(ch) )
			send_to_char( ch, "admin\r\n" );
		send_to_char( ch, "combattente\r\n" );
		send_to_char( ch, "mago\r\n" );
		send_to_char( ch, "ladro\r\n\r\n" );

		ch_printf( ch, "Digita '&G%s&w prompt' per informazioni su come cambiare il tuo prompt.\r\n\r\n", translate_command(ch, "help") );

		ch_printf( ch, "Il tuo corrente prompt%s è:\r\n", fprompt ? " di combattimento" : "" );
		set_char_color( AT_WHITE, ch );
		if ( (fprompt && VALID_STR(ch->pg->fprompt)) || (!fprompt && VALID_STR(ch->pg->prompt)) )
			ch_printf_nocolor( ch, "%s\r\n", fprompt ? ch->pg->fprompt : ch->pg->prompt );
		else
			send_to_char( ch, "Quello di default.\r\n" );

		return;
	}

	send_to_char( ch, "Sostituzione del vecchio prompt:\r\n" );
	set_char_color( AT_WHITE, ch );
	if ( (fprompt && VALID_STR(ch->pg->fprompt)) || (!fprompt && VALID_STR(ch->pg->prompt)) )
		ch_printf_nocolor( ch, "%s\r\n", fprompt ? ch->pg->fprompt : ch->pg->prompt );
	else
		send_to_char( ch, "Quello di default.\r\n" );

	if ( strlen_nocolor(argument) > 100 )
	{
		send_to_char( ch, "Prompt troppo lungo.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "default") )
	{
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
	}
	else if ( is_name(arg, "classic classico") )
	{
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
		ch->pg->prompt	= str_dup( "&B%k &c%n: &WPf:&R%l &WMn:&C%m &WMv:&G%v&w " );
		ch->pg->fprompt	= str_dup( "&c%n &WPf:&R%l &WMn:&C%m &WMv:&G%v &pTank:&R%c &pMob:&R%C&w " );
	}
	else if ( is_name(arg, "classic2 classico2") )
	{
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
		ch->pg->prompt	= str_dup( "&B%k &c%n: &WPf:&R%l &WMn:&C%m &WMv:&G%v &WAm:&b%o&w" );
		ch->pg->fprompt	= str_dup( "&c%n: &WPf:&R%l &WMn:&C%m &WMv:&G%v &pTank:&R%c &pMob:&Y(%N)&R%C&w" );
	}
	else if ( is_name(arg, "amministratore admin") && IS_ADMIN(ch) )
	{
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
		ch->pg->prompt	= str_dup( "&c%k&g%n: %l &RVita &w| %v &GMovi &w| %m &CMana&w " );
		ch->pg->fprompt	= str_dup( "&c%k&g%n: %l &RVita &w| %v &GMovi &w| %m &CMana&w (&R%c &wvs &R%C&w) " );
	}
	else if ( is_name(arg, "combattente guerriero tank warrior fighter") )
	{
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
		ch->pg->prompt	= str_dup( "&B%k &c%n: &WPf:&R%l &WMv:&G%v&w &WOro:&Y%g&w " );
		ch->pg->fprompt	= str_dup( "&c%n: &WPf:&R%l &WMv:&G%v &WStile:&x%S &pTank:&R%c &pMob:&Y(%N)&R%C&w " );
	}
	else if ( is_name(arg, "mago caster mage") )
	{
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
		ch->pg->prompt	= str_dup( "&B%k &c%n: &WPf:&R%l &WMn:&C%m &WMv:&G%v&w &WInvis:&b%A&w " );
		ch->pg->fprompt	= str_dup( "&c%n &WPf:&R%l &WMn:&C%m &pTank:&R%c &pMob:&Y(%N)&R%C&w " );
	}
	else if ( is_name(arg, "ladro thief") )
	{
		DISPOSE( ch->pg->prompt );
		DISPOSE( ch->pg->fprompt );
		ch->pg->prompt	= str_dup( "&B%k &c%n: &WPf:&R%l &WMn:&C%m &WMv:&G%v&w &WOro:&Y%g &WTime:&b%T&w " );
		ch->pg->fprompt	= str_dup( "&c%n &WPf:&R%l &WMv:&G%v &WStile:&x%S &pTank:&R%c &pMob:&R%C&w " );
	}
	else
	{
		if ( fprompt )
		{
			DISPOSE( ch->pg->fprompt );
			ch->pg->fprompt = str_dup( argument );
		}
		else
		{
			DISPOSE( ch->pg->prompt );
			ch->pg->prompt = str_dup( argument );
		}
	}
}

/*
 * Comando prompt, configurazione del prompt normalmente visualizzato
 */
DO_RET do_prompt( CHAR_DATA *ch, char *argument )
{
	prompt_handler( ch, argument, FALSE );
}

/*
 * Comando fprompt, configurazione del prompt visualizzato nei combattimenti.
 */
DO_RET do_fprompt( CHAR_DATA *ch, char *argument )
{
	prompt_handler( ch, argument, TRUE );
}


/*
 * Permette al pg di distruggere il salvataggio
 */
DO_RET do_delete( CHAR_DATA* ch, char *argument )
{
	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Se vuoi cancellare il tuo personaggio devi scrivere la tua password dopo il comando.\r\n" );
		return;
	}

#ifdef NOCRYPT
	if ( strcmp(cryptmd5(argument), ch->pg->pwd) )
#else
	if ( strcmp(cryptmd5(argument), ch->pg->pwd+1) )
#endif
	{
		send_to_char( ch, "Parola chiave errata.\r\n" );
		return;
	}

	puny_destroy( ch, ch, "" );
	send_log( NULL, LOG_WARNING, "%s sta distruggendo il suo personaggio.", ch->name );
}


DO_RET do_password( CHAR_DATA *ch, char *argument )
{
	char   *pArg;
	char   *pwdnew;
	char	arg1[MIL];
	char	arg2[MIL];
	char	cEnd;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non hanno password da poter cambiare.\r\n" );
		return;
	}

	/*
	 * Can't use one_argument here because it smashes case.
	 * So we just steal all its code.  Bleagh.
	 */
	pArg = arg1;
	while ( isspace(*argument) )
	argument++;

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( VALID_STR(argument) )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	pArg = arg2;
	while ( isspace(*argument) )
		argument++;

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( VALID_STR(argument) )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Sintassi: password <scrivi-la-nuova-password> <ripeti-la-nuova-password>.\r\n" );
		return;
	}

	/* This should stop all the mistyped password problems */
	if ( strcmp(arg1, arg2) )
	{
		send_to_char( ch, "Le password non corrispondono, per favore ripeti.\r\n" );
		return;
	}

	pwdnew = check_password( ch, arg2 );
	if ( !pwdnew )
		return;

	DISPOSE( ch->pg->pwd );
	ch->pg->pwd = str_dup( pwdnew );

	save_player( ch );

	if ( ch->pg->trust <= TRUST_MASTER )
		send_log( NULL, LOG_NORMAL, "%s sta modificando la sua password", ch->name );
	else
		send_log( NULL, LOG_NORMAL, "%s sta modificando la sua password - host: %s", ch->name, get_host(ch) );

	send_to_char( ch, "Ok.\r\n" );
}


/*
 * Tabella dei codici sulle flag dei player
 */
const struct player_type table_player[] =
{
	{ { PLAYER_ACCENT,		"PLAYER_ACCENT",	"accenti"		},	TRUST_PLAYER,	"Visualizzi gli accenti \"corretti\"",
																					"Visualizzi gli accenti \"con l'apostofo\""	},
	{ { PLAYER_BLANK,		"PLAYER_BLANK",		"blank"			},	TRUST_PLAYER,	"Mette una linea vuota prima del tuo prompt",
																					"Evita di mettere una linea vuota prima del tuo prompt"	},
	{ { PLAYER_BRIEF,		"PLAYER_BRIEF",		"breve"			},	TRUST_PLAYER,	"Vedi solo descrizioni brevi",
																					"Vedi le descrizioni in maniera estesa"	},
	{ { PLAYER_COMBINE,		"PLAYER_COMBINE",	"combina"		},	TRUST_PLAYER,	"Vedi le liste di oggetti raggruppando quelli uguali",
																					"Vedi le liste di oggetti senza raggruppare quelli uguali"	},
	{ { PLAYER_COLOR,		"PLAYER_COLOR",		"colori"		},	TRUST_PLAYER,	"Visualizzi i colori",
																					"Non visualizzi i colori"	},
	{ { PLAYER_HINT,		"PLAYER_HINT",		"suggerimenti"	},	TRUST_PLAYER,	"Visualizzi ogni tanto dei suggerimenti di gioco",
																					"Non visualizzi i suggerimenti di gioco"	},
	{ { PLAYER_LOOKEQUIP,	"PLAYER_LOOKEQUIP",	"guardaequip"	}, 	TRUST_PLAYER,	"Visualizzi l'equipaggiamento altrui con il comando guarda\\look",
																					"Visualizzi l'equipaggiamento altrui tramite il comando equip"	},
	{ { PLAYER_MAPPER,		"PLAYER_MAPPER",	"mappa"			},	TRUST_PLAYER,	"Visualizzi la mappa a fianco della descrizione delle stanze",
																					"Non visualizzi la mappa a fianco della descrizione delle stanze"	},
	{ { PLAYER_PKGROUP,		"PLAYER_PKGROUP",	"pkgruppo"		},	TRUST_PLAYER,	"Aiuti se uno del tuo gruppo attacca un giocatore",
																					"Non aiuti se uno del tuo gruppo attacca un giocatore"	},
	{ { PLAYER_PROMPT,		"PLAYER_PROMPT",	"prompt"		},	TRUST_PLAYER,	"Visualizzi il prompt",
																					"Non visualizza il prompt"	},
	{ { PLAYER_PROMPTBAR,	"PLAYER_PROMPTBAR",	"barreprompt"	},	TRUST_PLAYER,	"Visualizzi le barre al posto delle percentuali del prompt",
																					"Non visualizza le barre al posto delle percentuali dei punti"	},
	{ { PLAYER_AUTOEXIT,	"PLAYER_AUTOEXIT",	"autouscite"	},	TRUST_PLAYER,	"Vedi automaticamente una lista di uscite della stanza",
																					"Vedi le uscite della stanza su una riga dopo la descrizione"	},
	{ { PLAYER_AUTOGOLD,	"PLAYER_AUTOGOLD",	"automonete"	},	TRUST_PLAYER,	"Raccogli automaticamente i soldi dai cadaveri delle creature",
																					"Non raccogli automaticamente i soldi dai cadaveri delle creature"	},
	{ { PLAYER_AUTOSPLIT,	"PLAYER_AUTOSPLIT",	"autodividi"	},	TRUST_PLAYER,	"Dividi automaticamente i soldi con il gruppo",
																					"Non dividi automaticamente i soldi con il gruppo"	},
	{ { PLAYER_AUTOLOOT,	"PLAYER_AUTOLOOT",	"autoprendi"	},	TRUST_PLAYER,	"Saccheggi automaticamente i cadaveri delle creature uccise",
																					"Non saccheggi i cadaveri delle creature uccise"	},
	/* (FF) forse cambiarlo in autoflee? o aggiungere anche quello? (TT) (bb) ma poi ora il return in combat funziona se si è in link death? */
	{ { PLAYER_AUTORECALL,	"PLAYER_AUTORECALL","autoritorna"	},	TRUST_PLAYER,	"Ritorni automaticamente se ti sconnetti durante un combattimento",
																					"Non ritorni automaticamente se ti sconnetti durante un combattimento"	},
	{ { PLAYER_AUTOSAC,		"PLAYER_AUTOSAC",	"autosacrif"	},	TRUST_PLAYER,	"Sacrifichi automaticamente i cadaveri delle creature uccise",
																					"Non sacrifichi i cadaveri delle creature uccise"	},
	{ { PLAYER_INTRO,		"PLAYER_INTRO",		"intro"			},	TRUST_PLAYER,	"Visualizzi le animazioni entrando nel Mud",
																					"Non visualizzi le animazioni entrando nel Mud"	},
	{ { PLAYER_ITALIAN,		"PLAYER_ITALIAN",	"italiano"		},	TRUST_PLAYER,	"Utilizzi i comandi in italiano",
																					"Utilizzi i comandi in inglese"	},
	{ { PLAYER_DOUBLELANG,	"PLAYER_DOUBLELANG","doppialingua"	},	TRUST_PLAYER,	"Cerca i comandi su tutti e due i set di lingua",
																					"Non utilizzi la ricerca su tutti e due i set di lingua"	},
	{ { PLAYER_TELNETGA,	"PLAYER_TELNETGA",	"telnetga"		},	TRUST_PLAYER,	"Ricevi una sequenza telnet GA",
																					"Non ricevi una sequenza telnet GA"	},
	{ { PLAYER_ADMSIGHT,	"PLAYER_ADMSIGHT",	"admsight"		},	TRUST_NEOMASTER,"Puoi vedere nel buio, l'invisibile il nascosto e altro..",
																					"Visuale di amministratore disattivata"	},
	{ { PLAYER_ALLEXITS,	"PLAYER_ALLEXITS",	"allexits"		},	TRUST_NEOMASTER,"Visualizzi tutte le uscite e le keyword delle stesse",
																					"Visualizzi solo alcuni tipi di uscite e nessuna keyword"	},
	{ { PLAYER_ECHOIN,		"PLAYER_ECHOIN",	"echoin"		},	TRUST_NEOMASTER,"Invii gli aecho e recho nella stanza o area ove ti trovi",
																					"Non invii gli aecho e recho nella stanza o area ove ti trovi"	},
	{ { PLAYER_FLAG,		"PLAYER_FLAG",		"flag"			},	TRUST_NEOMASTER,"Visualizzi informazioni sulle flag delle entità del Mud",
																					"Non visualizzi informazioni sulle flag delle entità del Mud"	},
	{ { PLAYER_INVISWHO,	"PLAYER_INVISWHO",	"inviswho"		},	TRUST_NEOMASTER,"Sei invisibile nel who ai giocatori",
																					"Non sei invisibile nel who ai giocatori"	},
	{ { PLAYER_LASTCMD,		"PLAYER_LASTCMD",	"lastcmd"		},	TRUST_NEOMASTER,"Visualizzi l'ultimo comando inviato prima del messaggi di log",
																					"Non visualizzi l'ultimo comando inviato prima del messaggi di log"	},
	/* Permette di vedere il mud come un pg anche se si è admin */
	{ { PLAYER_NOADMIN,		"PLAYER_NOADMIN",	"noadmin"		},	TRUST_NEOMASTER,"Molte funzionalità di amministratore sono disattive",
																					"Possiedi tutte le funzionalità di un amministratore"	},
	{ { PLAYER_NOWAIT,		"PLAYER_NOWAIT",	"nowait"		},	TRUST_NEOMASTER,"Wait disattivato, nessun ritardo tra un comando e un altro",
																					"Wait normale, ritardo classico tra un comando e un altro"	},
	{ { PLAYER_VNUM,		"PLAYER_VNUM",		"vnum"			},	TRUST_NEOMASTER,"Visualizzi i vnum accanto alle entità del Mud",
																					"Non visualizzi i vnum accanto alle entità del Mud"	},
	{ { PLAYER_AFK,			"PLAYER_AFK",		"afk"			},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_AFK2,		"PLAYER_AFK2",		"afk2"			},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_DIAGNOSE,	"PLAYER_DIAGNOSE",	"diagnose"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_IDLE,		"PLAYER_IDLE",		"idle"			},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_LOG,			"PLAYER_LOG",		"log"			},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_WATCH,		"PLAYER_WATCH",		"watch"			},	TRUST_PLAYER,	"",
																					""	},	/* Bisogna far evitare che chi è watchato tra gli admin veda questa flag */
	{ { PLAYER_SPIRIT,		"PLAYER_SPIRIT",	"spirit"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_ATTACKER,	"PLAYER_ATTACKER",	"attacker"		},	TRUST_PLAYER,	"",
																					""	},
#ifdef T2_ALFA
	{ { PLAYER_HEADHUNTER,	"PLAYER_HEADHUNTER","headhunter"	},	TRUST_PLAYER,	"",
																					""	},
#endif                      
	{ { PLAYER_KILLER,		"PLAYER_KILLER",	"killer"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_OUTCAST,		"PLAYER_OUTCAST",	"outcast"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_THIEF,		"PLAYER_THIEF",		"thief"			},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_DENY,		"PLAYER_DENY",		"deny"			},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_EXPHALVE,	"PLAYER_EXPHALVE",	"exphalve"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_FREEZE,		"PLAYER_FREEZE",	"freeze"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_LITTERBUG,	"PLAYER_LITTERBUG",	"litterbug"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_NOEMOTE,		"PLAYER_NOEMOTE",	"noemote"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_NOSOCIAL,	"PLAYER_NOSOCIAL",	"nosocial"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_NOTELL,		"PLAYER_NOTELL",	"notell"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_NOTITLE,		"PLAYER_NOTITLE",	"notitle"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_NUISANCE,	"PLAYER_NUISANCE",	"nuisance"		},	TRUST_PLAYER,	"",
																					""	},
	{ { PLAYER_SILENCE,		"PLAYER_SILENCE",	"silence"		},	TRUST_PLAYER,	"",
																					""	}
};
const int max_check_player = sizeof(table_player)/sizeof(struct player_type);


/*
 * Comando per la configurazione generica delle impostazioni.
 */
DO_RET do_config( CHAR_DATA *ch, char *argument )
{
	int		x;

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		send_to_pager( ch, "\r\n&GConfigurazioni.\r\n" );
		pager_printf ( ch, "&gPer attivare una opzione utilizza &w'%s + <opzione>'\r\n",
			translate_command(ch, "config") );
		pager_printf ( ch, "&gPer disattivare una opzione utilizza &w'%s - <opzione>'\r\n",
			translate_command(ch, "config") );

		for ( x = 0;  x < MAX_PLAYER;  x++ )
		{
			if ( table_player[x].trust > get_trust(ch) )
				continue;

			if ( x == PLAYER_ACCENT )	send_to_pager( ch, "\r\n&GVisualizzazione:&w\r\n" );
			if ( x == PLAYER_AUTOEXIT )	send_to_pager( ch, "\r\n&GAutomazioni:&w\r\n" );
			if ( x == PLAYER_INTRO )	send_to_pager( ch, "\r\n&GVarie:&w\r\n" );
			if ( x == PLAYER_ADMSIGHT )	send_to_pager( ch, "\r\n&GAmministratori:&w\r\n" );

			if ( x == PLAYER_AFK )
				break;

			if ( HAS_BIT_PLR(ch, x) )
			{
				pager_printf( ch, "[&G+&w] &W%-12s &w%s\r\n",
					strupper(table_player[x].code.name),
					table_player[x].active );
			}
			else
			{
				pager_printf( ch, "[&R-&w] &W%-12s &w%s\r\n",
					table_player[x].code.name,
					table_player[x].deactive );
			}
		}

		if ( ch->pg->pagerlen == -1 )
		{
			send_to_pager( ch, "&w\r\nLunghezza pager: &Wdisattivato&w\r\n" );
		}
		else
		{
			pager_printf( ch, "&w\r\nLunghezza pager: &W%d&w  (configurabile con il comando %s)\r\n",
				ch->pg->pagerlen, translate_command(ch, "pager") );
		}

		/* (FF) carattere del target */
	}
	else
	{
		char	arg[MIL];
		int		bit = -1;
		bool	fSet;

		argument = one_argument( argument, arg );

		if		( arg[0] == '+' )	fSet = TRUE;
		else if ( arg[0] == '-' )	fSet = FALSE;
		else
		{
			send_to_char( ch, "&GConfigura -opzione o +opzione?&w\r\n" );
			return;
		}

		/* Se c'è lo spazio tra +/- e l'opzione allora ricontrolla il valore arg */
		if ( strlen(arg) == 1 )
			one_argument( argument, arg );
		else
			strcpy( arg, arg+1 );

		for ( x = 0;  x < MAX_PLAYER;  x++ )
		{
			if ( table_player[x].trust > get_trust(ch) )
				continue;

			if ( x == PLAYER_AFK )
				break;

			if ( !str_prefix(arg, table_player[x].code.name) )
				bit = x;
		}

		if ( bit == -1 )
		{
			send_to_char( ch, "&GOpzione errata.&w\r\n" );
			return;
		}

		if ( fSet )
		{
			SET_BIT( ch->pg->flags, bit );
			send_to_char( ch, "&GOpzione abilitata.&w\r\n" );
		}
		else
		{
			REMOVE_BIT( ch->pg->flags, bit );
			send_to_char( ch, "&GOpzione disabilitata.&w\r\n" );
		}
	} /* chiude l'else */
}


/*
 * Codici flag per i protocolli
 */
const struct protocol_type table_protocol[] =
{
#ifdef T2_MSP
	{ { PROTOCOL_MSP,		"PROTOCOL_MSP",		"msp"		},	"Protocollo sonoro MSP attivo",
																"Protocollo sonoro MSP disattivo" },
	{ { PROTOCOL_MUSIC,		"PROTOCOL_MUSIC",	"musica"	},	"Musiche attive",
																"Musiche disattive" },
	{ { PROTOCOL_SOUND,		"PROTOCOL_SOUND",	"suoni"		},	"Suoni attivo",
																"Suoni disattivi" },
	{ { PROTOCOL_SURROUND,	"PROTOCOL_SURROUND","surround"	},	"Modalità sonora surround attiva",
																"Modalità sonora surround disattiva" },
	{ { PROTOCOL_COMBAT,	"PROTOCOL_COMBAT",	"combat"	},	"Sonoro del combattimento attivo",
																"Sonoro del combattimento disattivo" },
	{ { PROTOCOL_BEEP,		"PROTOCOL_BEEP",	"beep"		},	"Segnale di tell ricevuti attivo",
																"Segnale di tell ricevuti disattivo" }
#endif                      
#ifdef T2_MCCP
	,                       
	{ { PROTOCOL_MCCP,		"PROTOCOL_MCCP",	"mccp"		},	"Protocollo di compressione MCCP attivo",
																"Protocollo di compressione MCCP disattivo" }
#endif                      
#ifdef T2_MXP
	,                       
	{ { PROTOCOL_MXP,		"PROTOCOL_MXP",		"mxp"		},	"Protocollo MXP attivo",
																"Protocollo MXP disattivo" }
#endif
};
const int max_check_protocol = sizeof(table_protocol)/sizeof(struct protocol_type);


/*
 * Comando per cambiare le opzioni riguardanti i protocolli
 */
DO_RET do_protocols( CHAR_DATA *ch, char *argument )
{
	int x;

	if ( IS_MOB(ch) )
		return;

	if ( !ch->desc )
		return;

	if ( !VALID_STR(argument) )
	{
		send_to_pager( ch, "\r\n&GProtocolli:\r\n" );
		pager_printf ( ch, "&gPer attivare una opzione utilizza &W'%s + <opzione>'\r\n",
			translate_command(ch, "protocol") );
		pager_printf ( ch, "&gPer disattivare una opzione utilizza &W'%s - <opzione>'&w\r\n\r\n",
			translate_command(ch, "protocol") );

#ifdef T2_MSP
		pager_printf( ch, "Il tuo Client %ssupporta il protocollo sonoro MSP\r\n", (!ch->desc->msp) ? "non " : "" );
#endif
#ifdef T2_MCCP
		pager_printf( ch, "Il tuo Client %ssupporta il protocollo di compressione MCCP\r\n", (!ch->desc->mccp) ? "non " : "" );
#endif
#ifdef T2_MXP
		pager_printf( ch, "Il tuo Client %ssupporta il protocollo MXP\r\n", (!ch->desc->mxp) ? "non " : "" );
#endif
		pager_printf( ch, "Le seguenti opzioni possono essere modificate a piacere.\r\n"
			"ma se il vostro client non supporta il protocollo relativo non succederà nulla.\r\n\r\n" );

		for ( x = 0;  x < MAX_PROTOCOL;  x++ )
		{
			if ( HAS_BIT(ch->pg->protocols, x) )
			{
				pager_printf( ch, "[&G+&w] &W%-12s &w%s\r\n",
					strupper(table_protocol[x].code.name),
					table_protocol[x].active );
			}
			else
			{
				pager_printf( ch, "[&R-&w] &W%-12s &w%s\r\n",
					table_protocol[x].code.name,
					table_protocol[x].deactive );
			}
		}
	}
	else
	{
		char	arg[MIL];
		int		bit = -1;
		bool	fSet;

		argument = one_argument( argument, arg );

		if		( arg[0] == '+' )	fSet = TRUE;
		else if ( arg[0] == '-' )	fSet = FALSE;
		else
		{
			ch_printf( ch, "&G%s -opzione o +opzione?&w\r\n", translate_command(ch, "protocol") );
			return;
		}

		/* Se c'è lo spazio tra +/- e l'opzione allora ricontrolla il valore arg */
		if ( strlen(arg) == 1 )
			one_argument( argument, arg );
		else
			strcpy( arg, arg+1 );

		for ( x = 0;  x < MAX_PROTOCOL; x++ )
		{
			if ( !str_prefix(arg, table_protocol[x].code.name) )	/* (BB) non funziona */
				bit = x;
		}

		if ( bit == -1 )
		{
			send_to_char( ch, "&GOpzione errata.&w\r\n" );
			return;
		}

#ifdef T2_MXP
		/* Resetta i menù MXP secondo il linguaggio scelto dal pg */
		if ( bit == PROTOCOL_MXP )
			mxp_toggle_language( ch->desc );
#endif

		if ( fSet )
		{
			SET_BIT( ch->pg->protocols, bit );
			send_to_char( ch, "&GOpzione abilitata.&w\r\n" );
		}
		else
		{
			REMOVE_BIT( ch->pg->protocols, bit );
			send_to_char( ch, "&GOpzione disabilitata.&w\r\n" );
		}
	}
}


DO_RET do_pause( CHAR_DATA *ch, char *argument )
{
	int	num;

	if ( IS_MOB(ch) )
		return;

	set_char_color( AT_NOTE, ch );
	if ( !VALID_STR(argument) )
	{
		char	cmd[MIL];

		strcpy( cmd, translate_command(ch, "pause") );

		ch_printf( ch, "&YSintassi&w: %s on\r\n", cmd );
		ch_printf( ch, "          %s off\r\n", cmd );
		ch_printf( ch, "          %s <numero linee>\r\n\r\n", cmd );
		if ( ch->pg->pagerlen == -1 )
			send_to_char( ch, "Attualmente la pausa per lo scorrimento è disattivata.\r\n" );
		else
			ch_printf( ch, "Attualmente la pausa per lo scorrimento delle linee è settata a %d.\r\n", ch->pg->pagerlen );
		return;
	}

	if ( is_name(argument, "si on attiva") )
	{
		/* Se il pager è disattivato lo setta a default */
		if ( ch->pg->pagerlen == -1 )
		{
			ch->pg->pagerlen = DEF_LEN_PAGER;
			ch_printf( ch, "Lunghezza pager impostata a default: %d linee.\r\n", DEF_LEN_PAGER );
		}
		else
		{
			ch_printf( ch, "Lunghezza pager già attiva a %d linee", ch->pg->pagerlen );
		}
		return;
	}

	if ( is_name(argument, "no off disattiva") )
	{
		/* Se il pager è attivato lo disattiva */
		if ( ch->pg->pagerlen != -1 )
		{
			ch->pg->pagerlen = -1;
			send_to_char( ch, "Pager disattivato.\r\n" );
		}
		else
		{
			send_to_char( ch, "Il pager è già disattivo." );
		}
		return;
	}

	if ( !is_number(argument) )
	{
		send_to_char( ch, "A quante linee vuoi posizionare la pausa per il pager?\r\n" );
		return;
	}

	num = atoi( argument );
	if ( num < 7 )
	{
		send_to_char( ch, "Non puoi impostare la pausa del pager a meno di 7 linee.\r\n" );
		return;
	}

	if ( num > 80 )
	{
		send_to_char( ch, "Non puoi impostare la pausa del pager a più di 80 linee.\r\n" );
		return;
	}

	ch->pg->pagerlen = num;
	ch_printf( ch, "Pausa del pager settata alla %d linea.\r\n", ch->pg->pagerlen );
}



/***************************************************
 * Ecco le funzioni del sistema di player offline:
 ***************************************************/

bool remove_offline( CHAR_DATA *ch )
{
	CHAR_DATA *soffline;

	for ( soffline = first_offline; soffline; soffline = soffline->next )
	{
		if ( !str_cmp(ch->name, soffline->name) )
		{
			UNLINK( soffline, first_offline, last_offline, next, prev );
			DISPOSE( soffline->desc );
			free_char( soffline );
			top_offline--;
			return TRUE;
		}
	}

	send_log( NULL, LOG_BUG, "remove_offline: Impossibile trovare offline di (%s)", ch->name );

	return FALSE;
}


/*
 * Deve essere stato appena salvato prima di chiamare questa procedura, carica un offline da file.
 */
void add_offline( CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;

	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->connected = CON_GET_NAME;

	if ( load_player(d, ch->name, TRUE) )
	{
		LINK( d->character, first_offline, last_offline, next, prev );
		top_offline++;
		return;
	}

	/* Se arriva qui è perchè probabilmente non ha mai salvato prima
	 *	quindi non è detto che sia un baco */
	send_log( NULL, LOG_WARNING, "add_offline: Impossibile aprire il file del pg (%s)", ch->name );
}


/*
 * Sostituisce il vecchio offline con il nuovo, da chiamare dopo la save,
 * se non trova vecchio offline aggiunge.
 * Invece di scambiare la struttura ch con quella di offline la ricarica di nuovo,
 *	questo per evitare vari problemi
 */
void refresh_offline( CHAR_DATA *ch )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA		*soffline;

	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->connected = CON_GET_NAME;

	if ( load_player(d, ch->name, TRUE) )
	{
		/* Prima di tutto slinko la vecchia, poi fuori linko la nuova */
		for ( soffline = first_offline;  soffline;  soffline = soffline->next )
		{
			if ( !str_cmp(ch->name, soffline->name) )
			{
				UNLINK( soffline, first_offline, last_offline, next, prev );
				break;
			}
		}
	}
	else
	{
		send_log( NULL, LOG_BUG, "refresh_offline: load_player del personaggio (%s) ritorna FALSE", ch->name );
		return;
	}

	LINK( d->character, first_offline, last_offline, next, prev );	/* Lo fa in ogni caso, anche se non lo ha trovato */

	if ( soffline )
	{
		DISPOSE( soffline->desc );	/* Dovrebbe bastare un dispose perchè non passa per init_descriptor() */
		free_char( soffline );
	}
	else
	{
		send_log( NULL, LOG_BUG, "refresh_offline: Vecchio offline di (%s) non trovato", ch->name );
		top_offline++;
	}
}


/*
 * Carica tutti i pg all'avvio del mud per gestirli permanentemente
 */
void load_offlines( void )
{
	struct dirent *dir;
	DIR			  *pdir;
	bool		   loaded;

	/* Apertura della directory */
	pdir = opendir( PLAYER_DIR );
	if ( !pdir )
	{
		send_log( NULL, LOG_LOAD, "load_offlines: impossibile trovare la directory %s", PLAYER_DIR );
		exit( EXIT_FAILURE );
	}

	/* Apre tutti i file save dei pg nella lista dei char */
	while ( (dir = readdir(pdir)) )
	{
		DESCRIPTOR_DATA *d;

		if ( dir->d_ino )
		{
			/* Se il nome del file non è valido lo salta, capita con file
			 *	di backup degli editor di linux */
			if ( strchr(dir->d_name, '.') )
				continue;

			/* Salta le directory */
			if ( !strcmp(dir->d_name, "admin")
			  || !strcmp(dir->d_name, "backup")
			  || !strcmp(dir->d_name, "corpses")
			  || !strcmp(dir->d_name, "deleted")
			  || !strcmp(dir->d_name, "accounts")
			  || !strcmp(dir->d_name, "lockers")
			  || !strcmp(dir->d_name, "notes")
			  || dir->d_name[0] == '\'' )	/* (BB) serve come check ad un baco che è da correggere nel login dei pg, bisogna fare dei test inserendo pg con nomi apostrofati per capire */
			{
				continue;
			}

			CREATE( d, DESCRIPTOR_DATA, 1 );

#ifdef T2_SAVECONVERT
			loaded = load_player( d, dir->d_name, FALSE );
#else
			loaded = load_player( d, dir->d_name, TRUE );
#endif
			if ( !loaded )
				send_log( NULL, LOG_LOAD, "load_offlines: file %s non caricato", dir->d_name );
			else
			{
#ifdef T2_ALFA
/* (TT) da testare prima di fare cazzate e cancellare file a caso.. */
				/* Controlla se il file save del pg sia da buttare */
				int month_now;
				int month_logon;

				month_now = current_time.month+1;
				month_logon = d->character->logon.month+1;

				if ( month_logon > month_logon )	/* current_time è un anno dopo */
					month_now += 12;

				if ( month_now - month_logon > sysdata.purge_after_moth
				  && (sysdata.purge_min_hour > get_hours_played(d->character)
				  ||  sysdata.purge_min_level > d->character->level) )
				{
					if ( unlink(d->character->name) == -1 )
					{
						send_log( NULL, LOG_BUG, "load_offline: errore nella rimozione del pfile %s",
							d->character->name );
					}
				}
#endif
			}
#ifdef T2_SAVECONVERT
			save_player( d->character );
#endif
			LINK( d->character, first_offline, last_offline, next, prev );
			top_offline++;
		}
	}

	closedir( pdir );
}
