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
 >			Modulo per la gestione dei livelli e della experienza			<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "calendar.h"
#include "db.h"
#include "experience.h"
#include "fight.h"
#include "group.h"
#include "interpret.h"
#include "mprog.h"
#include "msp.h"
#include "room.h"
#include "save.h"


/*
 * Calculate roughly how much experience a character is worth
 */
int get_exp_worth( CHAR_DATA *ch )
{
	int experience;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_exp_worth: ch è NULL" );
		return 5;
	}

	experience  = get_level(ch)/2 * get_level(ch)/2 * get_level(ch)/2 * 5;
	experience += ch->points[POINTS_LIFE];
	experience -= (ch->armor-50) * 2;
	experience += ( ch->barenumdie * ch->baresizedie + get_damroll(ch) ) * 50;
	experience += get_hitroll(ch) * get_level(ch)/2 * 10;

	if ( HAS_BIT(ch->affected_by, AFFECT_SANCTUARY) )		experience += experience * 1.5;
	if ( HAS_BIT(ch->affected_by, AFFECT_FIRESHIELD) )		experience += experience * 1.2;
	if ( HAS_BIT(ch->affected_by, AFFECT_SHOCKSHIELD) )		experience += experience * 1.2;

	experience = URANGE( 5, experience, 5000 );
	return experience;
}


int get_exp_base( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_exp_base: ch è NULL" );
		return 200;
	}

	if ( IS_MOB(ch) )
		return 200;

	if ( !HAS_BIT(table_class[ch->class]->flags, CLASSFLAG_PLAYABLE) )
		return 200;

	return table_class[ch->class]->exp_base;
}


/*
 * Ritorna quanta esperienza è richiesta per un certo livello.
 */
int exp_level( CHAR_DATA *ch, int level )
{
	CHAR_DATA  *och;
	int			points;
	int			perc_race = 0;	/* Percentuale delle razze giocate nel mud */
	int			perc_class = 0;	/* Percentuale delle classi giocate nel mud */
	int			total = 0;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "exp_level: ch è NULL" );
		return 1000000;
	}

	if ( level <= 0 && level > LVL_LEGEND )
	{
		send_log( NULL, LOG_BUG, "exp_level: livello errato: %d", level );
		return 1000000;
	}

	for ( och = first_offline;  och;  och = och->next )
	{
		/* Salta i pg che hanno giocato poco per non inficiare troppo nel risultato con gli inattivi */
		if ( och->level < LVL_NEWBIE/2 )	continue;
		if ( get_hours_played(och) < 24 )	continue;

		total++;

		/* Calcola prima quante classi e razze sono utilizzate */
		if ( och->class == ch->class )
			perc_class++;
		if ( och->race == ch->race )
			perc_race++;
	}

	/* E ora la relativa percentuale */
	if ( perc_class != 0 )
		perc_class	*= 100 / total;
	if ( perc_race != 0 )
		perc_race	*= 100 / total;

	level++;

	points = (level * level * 3) - (level * 9);
	points *= get_exp_base( ch );

	/* Moltiplica prima la pecentuale con meno elementi, così da non appesantire troppo il risultato,
	 *	difatti la percentuale relativa a più elementi è mediamente più bassa */
	if ( top_class_play < top_race_play )
	{
		if ( perc_class != 0 )
			points *= perc_class / 100;
		if ( perc_race != 0 )
			points *= perc_race / 100;
	}
	else
	{
		if ( perc_race != 0 )
			points *= perc_race / 100;
		if ( perc_class != 0 )
			points *= perc_class / 100;
	}

	return points;
}


/*
 * Funzione di avanzamento livello.
 */
void advance_level( CHAR_DATA *ch )
{
	int		x;

	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		int bonus;

		/* 20% di non ottenere un bonus */
		if ( number_range(0, 4) == 0 )
			continue;

		/* 10% di ottenere un malus */
		if ( number_range(0, 9) == 0 )
		{
			bonus = number_range( -(get_level(ch)/20), -1 );
send_log( NULL, LOG_MONI, "advance_level: diminuzione casuale di punti: pg %s, liv %d, val %d", ch->name, ch->level, bonus );
			/* Impedisce di diminuirle del livello di punteggio iniziale */
			if ( bonus < 0 && ch->points_max[x] <= table_points[x].start )
				bonus = 0;
		}
		else
		{
			bonus  = get_curr_attr( ch, table_points[x].attr_pri ) / 12;
			bonus += get_curr_attr( ch, table_points[x].attr_sec ) / 12;

			/* Aggiunge il modificatore di punteggio della razza, pari ad un millesimo del massimo potenziale */
			if ( table_race[ch->race]->points_limit[x] > 1000 )
				bonus += table_race[ch->race]->points_limit[x] / 1000;
		}

		/* Controlla il massimale con il limite di punti acquisibile dalla razza */
		if ( check_total_points(ch) <= 0
		  || ch->points_max[x] <= table_points[x].start
		  || ch->points_max[x] >= table_race[ch->race]->points_limit[x] )
		{
			bonus = 0;
		}

		ch->points_max[x] += bonus;

		if		( bonus > 0 )
			send_to_char( ch, "Le mie esperienze si sono rivelate proficue e sto migliorando " );
		else if ( bonus < 0 )
			send_to_char( ch, "Non sta andando molto bene, sto peggiorando " );

		if ( bonus != 0 )
		{
			switch ( x )
			{
			  default:
				send_log( NULL, LOG_BUG, "advance_level: punteggio errato: %d", x );
				send_to_char( ch, "in qualcosa.\r\n" );
				break;
			  case POINTS_LIFE:	send_to_char( ch, "nella resistenza vitale.\r\n" );	break;
			  case POINTS_MOVE:	send_to_char( ch, "nel movimento.\r\n" );			break;
			  case POINTS_MANA:	send_to_char( ch, "nell'energia magica.\r\n" );		break;
			  case POINTS_SOUL:	send_to_char( ch, "nell'anima.\r\n" );				break;
			}
		}
	}

	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		int bonus;

		/* Allena più spesso gli attributi della propria classe */
		if ( number_range(0, (table_class[ch->class]->avarage_attr[x] == 0) ? 3 : 2) == 0 )
			continue;

		bonus = number_range( -1, 2 );

		/* Diminuisce la perdita del senso del 50% */
		if ( bonus == -1 && number_range(0, 1) == 0 )
			bonus = 0;

		/* Controlla se può sommare il bonus agli attributi */
		if ( check_total_attr(ch) <= 0 || ch->attr_perm[x] <= 10 || ch->attr_perm[x] >= MAX_LEARN_ATTR )
			bonus = 0;

		ch->attr_perm[x] += bonus;

		if		( bonus < 0 )
			ch_printf( ch, "Sto peggiorando in %s..\r\n", code_name(NULL, x, CODE_ATTR) );
		else if ( bonus > 0 )
			ch_printf( ch, "Sto migliorando %sin %s!\r\n", (bonus > 1) ? "molto " : "", code_name(NULL, x, CODE_ATTR) );
	}

	for ( x = 0;  x < MAX_SENSE;  x++ )
	{
		int bonus;

		/* I sensi sono più difficili da allenare perché una cosa più innata */
		if ( number_range(0, 1) == 0 )
			continue;

		bonus = number_range( -1, 2 );

		/* Diminuisce la perdita del senso del 50% */
		if ( bonus == -1 && number_range(0, 1) == 0 )
			bonus = 0;

		/* Controlla se può sommare il bonus ai sensi */
		if ( check_total_sense(ch) <= 0 || ch->sense_perm[x] <= 10 || ch->sense_perm[x] >= MAX_LEARN_ATTR )
			bonus = 0;

		ch->sense_perm[x] += bonus;

		if		( bonus < 0 )
			ch_printf( ch, "Sto peggiorando in %s..\r\n", code_name(NULL, x, CODE_SENSE) );
		else if	( bonus > 0 )
			ch_printf( ch, "Sto migliorando %sin %s!\r\n", (bonus > 1) ? "molto " : "", code_name(NULL, x, CODE_SENSE) );
	}

	/* Aumenta le pratiche */
	ch->pg->practice += get_curr_attr( ch, ATTR_INT ) / number_range( 16, 20 );

	/* Aumenta la gloria */
	ch->pg->glory += number_range( 1, get_level(ch)/2 );

#ifdef T2_MSP
	if ( ch->level > 2 )
		send_audio( ch->desc, "level.mid", TO_CHAR );
#endif

	/* Logga le informazioni sull'avanzamento di livello */
	send_log( NULL, LOG_LEVEL, "%-12s  Liv %-3d  Età %-4d  Room: #%-5d  Razza %s  Classe %s",
		ch->name,
		ch->level,
		get_age(ch, TIME_YEAR),
		(ch->in_room == NULL)  ?  0  :  ch->in_room->vnum,
		get_race(ch, FALSE),
		get_class(ch, FALSE) );

	/* Salva il pg */
	save_player( ch );
}


/*
 * Funzione che gestisce il guadagno di punti esperienza.
 */
void gain_exp( CHAR_DATA *ch, int gain, bool dream )
{
	int	gain_orig;

	/* salva il gain originariamente passato per debugging */
	gain_orig = gain;
	
	if ( IS_MOB(ch) )   /* I mob non guadagnano esperienza */
	    return;
	if ( ch->level == LVL_LEGEND )	/* Fino al livello massimo  */
	    return;
	/* Fino ad ogni step di leggenda, quindi niente get_level() */
	if ( !IS_ADMIN(ch) && (ch->level+1) % LVL_LEGEND == 0 )
	    return;

	/* per-race experience multipliers */
	gain += (gain * table_race[ch->race]->exp_multiplier) / 100;

	/* Punizione ExpHalve, viene dimezzato il guadagno dei px */
	if ( HAS_BIT_PLR(ch, PLAYER_EXPHALVE) )
	{
		if		( gain < 0 )	gain *= 2;
		else if ( gain > 0 )	gain /= 2;
	}

	/* xp cap to prevent any one event from giving enougth xp to gain more than one level */
	gain = UMIN( gain, exp_level(ch, ch->level+2) - 1 );

	if ( IS_ADMIN(ch) )
	{
		set_char_color( AT_ADMIN, ch );
		ch_printf( ch, "[ADM] Ricevo %d punti di esperienza.\r\n", gain );
	}

	send_log( NULL, LOG_EXP, "pg %s guadagna %d (valore originale %d)", ch->name, gain, gain_orig );

	ch->exp = UMAX( 0, ch->exp+gain );

	/* Fa passare di livello, senza messaggio, fino al livello di newbie, poi bisogna passare sognando */
	while ( (ch->level <= LVL_NEWBIE/2 || dream) && ch->exp >= exp_level(ch, ch->level+1) )
	{
		ch->exp = ch->exp - exp_level( ch , ch->level+1 );
		/* il penultimo e l'ultimo livello non li aumenta, cosìcché il pg cresce all'infinito con lo stesso livello */
		if ( ch->level < LVL_LEGEND-1 )
			ch->level++;
		advance_level( ch );
	}
}


/*
 * Funzioncina per la gestione della hole
 */
void exp_death_hole( CHAR_DATA *ch )
{
	int	xp;
	int	spi;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "death_exp_malus: ch passato è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( ch->exp <= 0 )
		return;

	/* Vengono tolti px sulla base di quelli da fare in tutto il livello */
	xp = exp_level( ch, ch->level+1 );
	xp /= 100;

	/* La spiritualità alta diminuisce il malus */
	spi = get_curr_attr( ch, ATTR_SPI );
	if ( spi >= 70 )
		xp /= ( spi / 50 );		/* Se la spiritualità fosse 100 dimezzerebbe di 2 */
	else if ( spi < 30 )
		xp *= 2 - ( spi / 50 );	/* Se la spiritualità fosse di 1 moltiplicherebbe di quasi 2 */

	xp *= sysdata.death_hole_mult/10;
	gain_exp( ch, 0 - xp, FALSE );
}


/*
 * Comando admin che visualizza tutti i punti px necessari per livellare relativi a classe e razza
 */
DO_RET do_experience( CHAR_DATA *ch, char *argument )
{
	char	arg1[MIL];
	char	arg2[MIL];
	int		lvl;
	int		class;
	int		race;
	int		orig_class = ch->class;
	int		orig_race = ch->race;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "ch passato è NULL" );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		ch_printf( ch, "&YSintassi&w: %s <nome classe> <nome razza>", translate_command(ch, "experience") );
		return;
	}

	class	= get_class_num( arg1, FALSE );
	race	= get_race_num( arg2, FALSE );

	/* Modifica temporaneamente la classe e la razza di ch per far funziona la exp_level() */
	ch->class	= class;
	ch->race	= race;

	send_to_pager( ch, "Livello     Esperienza\r\n" );
	for ( lvl = 1;  lvl <= LVL_LEGEND;  lvl++ )
		pager_printf( ch, "%3d -> %3d %9d\r\n", lvl, lvl+1, exp_level(ch, lvl+1) );

	ch->class	= orig_class;
	ch->race	= orig_race;
}


/*
 * Comando advance.
 * Aumenta o diminuisce il livello.
 */
DO_RET do_advance( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg[MIL];
	int		   level;
	int		   x;

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) || !is_number(argument) )
	{
		send_to_char( ch, "&YSintassi&w: advance <nomepg> <livello>\r\n" );
		ch_printf( ch, "             <livello> da 1 a %d\r\n", LVL_LEGEND );
		return;
	}

	victim = get_player_mud( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo giocatore non c'è.\r\n" );
		return;
	}

	/* You can demote yourself but not someone else at your own level. */
	if ( get_trust(ch) <= get_trust(victim) && ch != victim )
	{
		send_to_char( ch, "Non puoi fare ciò.\r\n" );
		return;
	}

	level = atoi( argument );
	if ( level < 1 || level > LVL_LEGEND )
	{
		send_command( ch, "advance", CO );
		return;
	}

	if ( level == victim->level )
	{
		ch_printf( ch, "%s is already level %d.\r\n", victim->name, level );
		return;
	}

	set_char_color( AT_WHITE, victim );

	/* Lower level:
	 *  Currently, an adm can lower another adm.
	 *  Can't lower adms >= your trust (other than self).
	 */
	if ( level < victim->level )
	{
		set_char_color( AT_ADMIN, victim );

		ch_printf	( ch, "Demoting %s from level %d to level %d!\r\n", victim->name, victim->level, level );
		send_to_char( victim, "Senti come se qualcosa che ti stesse risucchiando l'anima..\r\n" );

		victim->level	= level;
		victim->exp		= 0;

		return;
	}

	ch_printf	( ch, "Aumenti il livello di %s da %d a %d!\r\n", victim->name, victim->level, level );
	send_to_char( victim, "Percepisci un cambiamento del tuo io, un netto miglioramento..\r\n" );

	for ( x = victim->level;  x < level;  x++ )
	{
		victim->level++;
		advance_level( victim );
	}
	victim->exp	= 0;

	save_player( victim );
}


/*
 * Calculate how much XP gch should gain for killing victim.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
	int		align;
	int		xp;
	int		xp_ratio;
	int     xp_sub;

	xp = ( get_exp_worth(victim) * URANGE(0, (get_level(victim)/2 - get_level(gch)/2) + 10, 13) ) / 10;
	align = gch->alignment - victim->alignment;

	/* Bonus for attacking opposite alignment */
	if ( align >  950 || align < -950 )
		xp *= 5/4;

	/* Penalty for attacking same alignment */
	else if ( align < 50 && align > -50 )
		xp /= 2;

	xp = number_range( (xp*3) >> 2, (xp*5) >> 2 );

	/* get 1/2 exp for players */
	if ( IS_PG(victim) )
	{
		xp /= 2;
	}
	else if ( IS_PG(gch) )
	{
		/* reduce exp for killing the same mob repeatedly */
		int times = times_killed( gch, victim );

		if ( times >= 20 )
		{
			xp = 0;
		}
		else if ( times )
		{
			xp = (xp * (20-times)) / 20;

			if ( times > 15 )
				xp /= 3;
			else
				if ( times > 10 )

			xp >>= 1;
		}
	}

	/*
	 * semi-intelligent experienced player vs. novice player xp gain
	 *	"bell curve"ish xp mod
	 *	based on time played vs. level
	 */
	if ( IS_PG(gch) && get_level(gch) > LVL_NEWBIE/2 )
	{
		xp_ratio = (int) gch->pg->played / gch->level;

		if		( xp_ratio > 20000 )	xp = (xp*5) >> 2;	/* 5/4  */
		else if ( xp_ratio < 16000 )	xp = (xp*3) >> 2;	/* 3/4  */
		else if ( xp_ratio < 10000 )	xp >>= 1;			/* 1/2  */
		else if ( xp_ratio <  5000 )	xp >>= 2;			/* 1/4  */
		else if ( xp_ratio <  3500 )	xp >>= 3;			/* 1/8  */
		else if ( xp_ratio <  2000 )	xp >>= 4;			/* 1/16 */
	}

#ifdef T2_ARENA
	/* in arena non si prendono px */
	if ( in_arena(gch) )
		xp = 0;
#endif

	/* Level based experience gain cap.
	 * Cannot get more experience for a kill than the
	 *	amount for your current experience level */
	xp = URANGE( 0, xp, exp_level(gch, gch->level+1) );

	/* Aumentano man mano che si avanza di livello i px da sottrarre
	 * E' un semplice calcolo di percentuale diviso 6 */
	xp_sub = (xp * get_level(gch) / 100) / 6;
	if ( xp_sub >= xp )
		return 0;
	else
		return xp = xp - xp_sub;
}


void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *gch;
	CHAR_DATA *gch_next;
	CHAR_DATA *lch;
	int		   xp;
	int		   members;

	/*
	 * Monsters don't get kill xp's or alignment changes.
	 * Dying of mortal wounds or poison doesn't give xp to anyone!
	 */
	if ( IS_MOB(ch) || victim == ch )
		return;

	members = count_group_room( ch );
	members++;	/* aggiunge lo stesso ch */

	if ( members == 0 )
	{
		send_log( NULL, LOG_BUG, "group_gain: la variabile members è 0." );
		members = 1;
	}

	lch = ch->leader  ?  ch->leader  :  ch;

	for ( gch = ch->in_room->first_person;  gch;  gch = gch_next )
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		gch_next = gch->next_in_room;

		if ( !is_same_group(gch, ch) )
			continue;

		if ( get_level(gch) - get_level(lch) > sysdata.lvl_group_diff )
		{
			send_to_char( gch, "Da questo gruppo non ho nulla da imparare.\r\n" );
			continue;
		}

		if ( get_level(gch) - get_level(lch) < -sysdata.lvl_group_diff )
		{
			send_to_char( gch, "Questo gruppo ha troppa esperienza per me.\r\n" );
			continue;
		}

		xp = (int) (xp_compute(gch, victim) * 0.1765) / members;
		if ( !gch->fighting )
			xp /= 2;

		/* Invia un messaggio se prende pochi punti */
		if ( xp < get_level(gch)/4 )
			send_to_char( gch, "Non è certo stata una sfida con cui possa fare esperienza.\r\n" );

		gain_exp( gch, xp, FALSE );

		/* Invia un messaggio che indica che bisogna sognare per passare di livello */
		if ( gch->exp >= exp_level(gch, gch->level+1) )
			send_to_char( gch, "Dovrei dormire per un po' per assimilare ciò che ho imparato.\r\n" );

		/* Dona un po' di gloria a seconda del livello della vittima */
		if ( IS_PG(gch) && get_level(victim) > get_level(gch)+1 )
			gch->pg->glory += victim->level - gch->level;

		gch->alignment = align_compute( gch, victim );

		for ( obj = gch->first_carrying;  obj;  obj = obj_next )
		{
			obj_next = obj->next_content;
			if ( obj->wear_loc == WEARLOC_NONE )
				continue;

			if ( (HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_EVIL) && is_align(gch, ALIGN_EVIL))
			  || (HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_GOOD) && is_align(gch, ALIGN_GOOD)) )
			{
				act( AT_MAGIC, "$p ti colpisce!", gch, obj, NULL, TO_CHAR );
				act( AT_MAGIC, "$n viene colpit$x con $p.", gch, obj, NULL, TO_ROOM );

				obj_from_char( obj );
				obj = obj_to_room( obj, gch->in_room );
				oprog_zap_trigger( gch, obj );	/* mudprogs */
				if ( char_died(gch) )
				{
					save_player( gch );
					break;
				}
			}
		}
	} /* chiude il for */
}


/*
 * Get experience based on % of damage done
 */
void exp_for_damage( CHAR_DATA *ch, CHAR_DATA *victim, const int dam, const int dt )
{
	int	xp_gain;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "exp_for_damage: ch passato è NULL" );
		return;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "exp_for_damage: victim passato è NULL" );
		return;
	}

	if ( dam && ch != victim && IS_PG(ch)
	  && ch->fighting && ch->fighting->xp )
	{
		if ( ch->fighting->who == victim )
			xp_gain = (int) ( ch->fighting->xp * dam ) / victim->points_max[POINTS_LIFE];
		else
			xp_gain = (int) ( xp_compute(ch, victim) * 0.85 * dam ) / victim->points_max[POINTS_LIFE];

		if ( dt == gsn_backstab || dt == gsn_circle )
			xp_gain = 0;
	
		gain_exp( ch, xp_gain, FALSE );
	}
}
