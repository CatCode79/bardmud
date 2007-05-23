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
 >						Modulo di aggiornamento regolare					<
\****************************************************************************/


#include "mud.h"
#include "affect.h"
#include "animation.h"
#include "bank.h"
#include "calendar.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "dream.h"
#include "economy.h"
#include "gramm.h"
#include "help.h"
#include "herb.h"
#include "hint.h"
#include "interpret.h"
#include "lyric.h"
#include "movement.h"
#include "mprog.h"
#include "msp.h"
#include "nanny.h"
#include "morph.h"
#include "reset.h"
#include "room.h"
#include "save.h"
#include "timer.h"
#include "track.h"


/*
 * Variabili globali
 */
CHAR_DATA *gch_prev;
OBJ_DATA  *gobj_prev;

CHAR_DATA *timechar;

char *corpse_descrs[] =
{
	"Il cadavere di %s è ormai all'ultimo stadio di decadimento.",
	"Il cadavere di %s pullula di vermi e parassiti.",
	"Il cadavere di %s infesta l'aria con un terribile olezzo.",
	"Il cadavere di %s è attaccato da un nugolo di mosche.",
	"Il cadavere di %s giace qui."
};


/*
 * Funzioni di rigenerazione punti.
 * (FF) metterla nella update_points che i nomi delle funzioni gains_points sennò conflitta
 */
int points_gain( CHAR_DATA *ch, int point )
{
	int		gain;

	if ( IS_MOB(ch) )
	{
		gain = get_level(ch) * 5 / 4;
	}
	else
	{
		/* (RR) (TT) bisogna calcolare un po' di casistiche di esempio per vedere se il bilanciamento quadra */
		gain = URANGE( table_points[point].gain_base, get_level(ch)/table_points[point].div, LVL_LEGEND/table_points[point].div );

		switch ( ch->position )
		{
		  case POSITION_DEAD:		return  0;
		  case POSITION_MORTAL:		return -4;
		  case POSITION_INCAP:		return -2;
		  case POSITION_STUN:		return  2;
		  case POSITION_SLEEP:		gain += (get_curr_attr(ch, table_points[point].attr_pri)/3
		  							   + get_curr_attr(ch, table_points[point].attr_sec)/5) * table_points[point].gain_sleep;	break;
		  case POSITION_REST:		gain += (get_curr_attr(ch, table_points[point].attr_pri)/3
		  							   + get_curr_attr(ch, table_points[point].attr_sec)/5) * table_points[point].gain_rest;	break;
		  case POSITION_SIT:			gain += (get_curr_attr(ch, table_points[point].attr_pri)/3
		  								+get_curr_attr(ch, table_points[point].attr_sec)/5) * table_points[point].gain_sit;	break;
		}

#if T2_VAMPIRE
		/* (FF) calcolo per la malattia vampirica*/
		if ( IS_VAMPIRE(ch) && point == POINTS_LIFE )
		{
			if ( ch->pg->condition[CONDITION_BLOODTHIRST] <= 1 )
				gain /= 2;
			else if ( ch->pg->condition[CONDITION_BLOODTHIRST] >= get_level(ch)/2 + 8 )
				gain *= 2;

			if ( IS_OUTSIDE(ch) )
			{
				switch ( calendar.sunlight )
				{
				  case SUN_RISE:
				  case SUN_SET:
					gain /= 2;
					break;
				  case SUN_LIGHT:
					gain /= 4;
					break;
				}
			}
		}
#endif

		if ( ch->pg->condition[CONDITION_FULL]   == 0 )		gain /= 3;
		if ( ch->pg->condition[CONDITION_THIRST] == 0 )		gain /= 3;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_POISON) )			gain /= 6;

	return UMIN( gain, ch->points_max[point] - ch->points[point] );
}


void update_points( void )
{
	CHAR_DATA *ch;

	for ( ch = last_char;  ch;  ch = gch_prev )
	{
		if ( ch == first_char && ch->prev )
		{
			send_log( NULL, LOG_BUG, "update_points: first_char->prev != NULL... fixed" );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;
		set_cur_char( ch );

		if ( gch_prev && gch_prev->next != ch )
		{
			send_log( NULL, LOG_BUG, "update_points: ch->prev->next != ch" );
			return;
		}

		if ( ch->position >= POSITION_STUN )
		{
			int x;

			for ( x = 0;  x < MAX_POINTS;  x++ )
			{
				if ( ch->points[x]  < ch->points_max[x] )
					ch->points[x]  += points_gain( ch, x );
			}
		}

		if ( ch->position == POSITION_STUN )
			update_position( ch );
	}
}


void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
	int		condition;
	ch_ret	retcode = rNONE;

	if ( value == 0 || IS_MOB(ch) || IS_ADMIN(ch) )
		return;

	condition = ch->pg->condition[iCond];
#ifdef T2_VAMPIRE
	if ( iCond == CONDITION_BLOODTHIRST )
		ch->pg->condition[iCond] = URANGE( 0, condition + value, get_level(ch)/2 + 10 );
	else
#endif
		ch->pg->condition[iCond] = URANGE( 0, condition + value, 48 );

	if ( ch->pg->condition[iCond] == 0 )
	{
		switch ( iCond )
		{
		  case CONDITION_FULL:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_HUNGRY, ch );
			send_to_char( ch, "Sto languendo per la FAME!\r\n" );
			act( AT_HUNGRY, "$n sta morendo di fame!", ch, NULL, NULL, TO_ROOM);

			if ( number_bits(1) == 0 )
				worsen_mental_state( ch, 1 );

			retcode = damage( ch, ch, 1, TYPE_UNDEFINED );
			break;

		  case CONDITION_THIRST:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_THIRSTY, ch );
			send_to_char( ch, "Mi sto indebolendo per la SETE!\r\n" );
			act( AT_THIRSTY, "$n sta morendo di sete!", ch, NULL, NULL, TO_ROOM );

			worsen_mental_state( ch, 2 );

			retcode = damage( ch, ch, 2, TYPE_UNDEFINED );
			break;

		  case CONDITION_BLOODTHIRST:
			set_char_color( AT_DRED, ch );
			send_to_char( ch, "Il bisogno di sangue mi stringe la gola!\r\n" );
			act( AT_DRED, "$n patisce la mancanza di sangue!", ch, NULL, NULL, TO_ROOM );

			worsen_mental_state( ch, 2 );

			retcode = damage( ch, ch, ch->points_max[POINTS_LIFE]/20, TYPE_UNDEFINED );
			break;

		  case CONDITION_DRUNK:
			if ( condition != 0 )
			{
				set_char_color( AT_SOBER, ch );
				send_to_char( ch, "La tua sete è scemata.\r\n" );
			}
			retcode = rNONE;
			break;

		  default:
			send_log( NULL, LOG_BUG, "gain_condition: tipo di condizione non valida: %d", iCond );
			retcode = rNONE;
			break;
		} /* chiude lo switch */
	} /* chiude l'if */

	if ( retcode != rNONE )
		return;

	if ( ch->pg->condition[iCond] == 1 )
	{
		switch ( iCond )
		{
		  default:
			send_log( NULL, LOG_BUG, "gain_condition: tipo di condizione non valida: %d", iCond );
			break;

		  case CONDITION_FULL:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_HUNGRY, ch );
			act( AT_HUNGRY, "Sono davvero molto affamat$x.", ch, NULL, NULL, TO_CHAR );
			act( AT_HUNGRY, "Avverti lo stomaco di $n che brontola.", ch, NULL, NULL, TO_ROOM );

			if ( number_bits(1) == 0 )
				worsen_mental_state( ch, 1 );
			break;

		  case CONDITION_THIRST:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_THIRSTY, ch );
			send_to_char( ch, "La mia sete è insostenibile.\r\n" );
			act( AT_THIRSTY, "$n è in preda all'arsura.", ch, NULL, NULL, TO_ROOM );

			worsen_mental_state( ch, 1 );
			break;

		  case CONDITION_BLOODTHIRST:
			set_char_color( AT_DRED, ch );
			send_to_char( ch, "Il mio bisogno di sangue sta diventando intollerabile!\r\n" );
			act( AT_DRED, "$n percepisci un sinistro bagliore negli occhi di su$x ...", ch, NULL, NULL, TO_ROOM );

			worsen_mental_state( ch, 1 );
			break;

		  case CONDITION_DRUNK:
			if ( condition != 0 )
				act( AT_SOBER, "Mi sento lievemente ristorat$x.\r\n", ch, NULL, NULL, TO_CHAR );
			break;
		} /* chiude lo switch */
	} /* chiude l'if */


	if ( ch->pg->condition[iCond] == 2 )
	{
		switch ( iCond )
		{
		  case CONDITION_FULL:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_HUNGRY, ch );
			send_to_char( ch, "Ho fame.\r\n" );
			break;

		  case CONDITION_THIRST:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_THIRSTY, ch );
			send_to_char( ch, "Ho sete.\r\n" );
			break;

		  case CONDITION_BLOODTHIRST:
			set_char_color( AT_DRED, ch );
			send_to_char( ch, "Ho bisogno di sangue, urgentemente.\r\n" );
			break;
		}
	}

	if ( ch->pg->condition[iCond] == 3 )
	{
		switch ( iCond )
		{
		  case CONDITION_FULL:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_HUNGRY, ch );
			send_to_char( ch, "Avverto un languorino allo stomaco.\r\n" );
			break;

		  case CONDITION_THIRST:
#ifdef T2_VAMPIRE
			if ( ch->class != CLASS_VAMPIRE )
#endif
			set_char_color( AT_THIRSTY, ch );
			send_to_char( ch, "Ho proprio voglia di qualcosa di fresco da bere.\r\n" );
			break;

		  case CONDITION_BLOODTHIRST:
			set_char_color( AT_DRED, ch );
			send_to_char( ch, "I miei denti vibrano violentemente..\r\n" );
			break;
		}
	}
}


void update_condition( void )
{
	CHAR_DATA *ch;

	for ( ch = last_char;  ch;  ch = gch_prev )
	{
		if ( ch == first_char && ch->prev )
		{
			send_log( NULL, LOG_BUG, "update_coindition: first_char->prev != NULL... fixed" );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;
		set_cur_char( ch );

		if ( gch_prev && gch_prev->next != ch )
		{
			send_log( NULL, LOG_BUG, "update_condition: ch->prev->next != ch" );
			return;
		}

		/* (FF) check sulla migrazione, se alta il pg prende un po' sussistenza dai piani */
		/* (FF) check sull'assorbimento per le classi magiche, se alto prendono un po' sussistenza dal mana */

		gain_condition( ch, CONDITION_DRUNK, -1 );
		gain_condition( ch, CONDITION_FULL,  -1 + table_race[ch->race]->hunger_mod );

#ifdef T2_VAMPIRE
		/* (FF) futura implementazione della malattia di vampiro */
		if ( IS_DISEADED(ch, VAMPIRISM) && get_level(ch) > LVL_NEWBIE )
		{
			if ( calendar.hour < 21 && calendar.hour > 5 )
				gain_condition( ch, CONDITION_BLOODTHIRST, -1 );
		}
#endif
		if ( ch->in_room )
		{
			switch ( ch->in_room->sector )
			{
			  default:
				gain_condition( ch, CONDITION_THIRST, -1 + table_race[ch->race]->thirst_mod );
				break;
			  case SECTOR_DESERT:
				gain_condition( ch, CONDITION_THIRST, -3 + table_race[ch->race]->thirst_mod );	/* (RR) differenziarli in base alla stazza razziale */
				break;
			  case SECTOR_UNDERWATER:
			  case SECTOR_OCEANFLOOR:
				if ( number_bits(1) == 0 )
					gain_condition( ch, CONDITION_THIRST, -1 + table_race[ch->race]->thirst_mod );
				break;
			}
		}
	}
}


/* (RR) rifarla con altri effetti.
 * Put this in a seperate function so it isn't called three times per tick
 * This was added after a suggestion from Cronel
 */
void check_alignment( CHAR_DATA *ch )
{
	/* Race alignment restrictions */
	if ( ch->alignment < table_race[ch->race]->min_align )
	{
		set_char_color( AT_DRED, ch );
/*		send_to_char( ch, "Le tue azioni sono incompatibili con gli ideali della tua razza. Sei nei guai." ); */	/* (FF) */
	}

	if ( ch->alignment > table_race[ch->race]->max_align )
	{
		set_char_color( AT_DRED, ch );
/*		send_to_char( ch, "Le tue azioni sono incompatibili con gli ideali della tua razza. Sei nei guai..." ); */	/* (FF) */
	}

	/* (FF) diminuire il mental state mano a mano, soprattutto se di base sarebbe una razza buona */
}


/*
 * drunk randoms
 * (Made part of update_mobile)
 */
void drunk_randoms( CHAR_DATA *ch )
{
	CHAR_DATA *rvch = NULL;
	CHAR_DATA *vch;
	int		   drunk;
	int		   position;

	if ( IS_MOB(ch) || ch->pg->condition[CONDITION_DRUNK] <= 0 )
		return;

	if ( number_percent( ) < 30 )
		return;

	drunk	 = ch->pg->condition[CONDITION_DRUNK];
	position = ch->position;
	ch->position = POSITION_STAND;

	/* (RR) molti di questi sociali ora non esistono più */
	if		( number_percent() < drunk  / 10 )		check_social( ch, "burp",	"", FALSE, FALSE );
	else if ( number_percent() < drunk  / 10 )		check_social( ch, "hiccup", "", FALSE, FALSE );
	else if ( number_percent() < drunk  / 10 )		check_social( ch, "drool",	"", FALSE, FALSE );
	else if ( number_percent() < drunk  / 10 )		check_social( ch, "fart",	"", FALSE, FALSE );
	else if ( number_percent() < drunk  /  9
	  && drunk > get_curr_attr(ch, ATTR_CON) /  6 )
	{
		for ( vch = ch->in_room->first_person;  vch;  vch = vch->next_in_room )
		{
			if ( number_percent() < 10 )
				rvch = vch;
		}
		check_social( ch, "puke", (rvch  ?  rvch->name  :  ""), FALSE, FALSE );
	}

	ch->position = position;
}


/*
 * Random hallucinations for those suffering from an overly high mentalstate
 * (Hats off to Albert Hoffman's "problem child")
 */
void hallucinations( CHAR_DATA *ch )
{
	char t[MSL];

	if ( ch->mental_state < 30 )
		return;

	if ( number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) != 0 )
		return;

	switch ( number_range(1, UMIN(42, (ch->mental_state+5) * 2/5)) )	/* (TT) */
	{
	  default:
	  case  1:	strcpy ( t, "Mi sento senza forze, ho bisogno di riposo ma ancora non riesco a sedere..\r\n" );					break;
	  case  2:	strcpy ( t, "Mi sento pieno di energia, non riesco a stare fermo!\r\n" );										break;
	  case  3:	strcpy ( t, "Un fastidioso formicolio mi sta intorpidendo le membra.. \r\n" );									break;
	  case  4:	strcpy ( t, "Mi sento pizzicare dappertutto.\r\n" );															break;
	  case  5:	strcpy ( t, "La mia pelle brulica!\r\n" );																		break;
	  case  6:	strcpy ( t, "Qualcosa sta strisciando sulla mia pelle.\r\n" );													break;
	  case  7:	strcpy ( t, "Sta accadendo qualcosa di strano..\r\n" );															break;
	  case  8:	strcpy ( t, "Senti che c'è qualcosa di terribilmente sbagliato..\r\n" );										break;
	  case  9:	strcpy ( t, "Quelle piccole fate maledette! Continuano a ridere di me!\r\n" );									break;
	  case 10:	strcpy ( t, "Quei dannati piccoli folletti non smettono di ridere di me!\r\n" );								break;
	  case 12:	strcpy ( t, "Non mi sto sbagliando, quello che sento è il pianto di tua madre..\r\n" );							break;
	  case 13:	strcpy ( t, "Riesco a sentire i lamenti degli spiriti..\r\n" );													break;
	  case 14:	strcpy ( t, "Mi sembra di essere già stato qui..\r\n" );														break;
	  case 15:	strcpy ( t, "Sono già stato qui o no? Non ne sono sicuro..\r\n" );												break;
	  case 16:	strcpy ( t, "Reminescenze di dolorosa infanzia balenano rapide susseguendosi nella mia mente.\r\n" );			break;
	  case 17:	strcpy ( t, "Mi ritornano in mente dolorosi ricordi dell'infanzia.\r\n" );										break;
	  case 18:	strcpy ( t, "Una voca profonda mi ha appena chiamato per nome..\r\n" );											break;
	  case 19:	strcpy ( t, "Sento qualcuno chiamarmi per nome da molto lontano..\r\n" );										break;
	  case 20:	strcpy ( t, "La mia testa pulsa e batte violentemente. Non riesco nemmeno a pensare.\r\n" );					break;
	  case 21:	strcpy ( t, "La mente mi pulsa.. Non riesco a pensare chiaramente.\r\n" );										break;
	  case 22:	strcpy ( t, "La terra pare contorcersi su sè stessa in mille pieghe e suoni ovattati..\r\n" );					break;
	  case 23:	strcpy ( t, "La terra! Sembra contorcersi..\r\n" );																break;
	  case 24:	strcpy ( t, "Non ho più la consapevolezza di quanto sia vero e quanto irreale.\r\n" );							break;
	  case 25:	strcpy ( t, "Non riesco più a distinguere tra cosa sia reale e cosa non lo sia.\r\n" );							break;
	  case 26:	strcpy ( t, "E' tutto un sogno.. Oppure no?\r\n" );																break;
	  case 27:	strcpy ( t, "Sto vivendo in un sogno?\r\n" );																	break;
	  case 28:	strcpy ( t, "Mi sembra di sentire delle voci bisbligliare il muo nome.. Sono dappertutto!" );					break;
	  case 29:	strcpy ( t, "Sento la voce dei miei pronipoti. Pregano perché io li protegga.\r\n" );							break;
	  case 30:	strcpy ( t, "Stanno venendo a prendermi.. Mi porteranno via!\r\n" );											break;
	  case 31:	strcpy ( t, "Stanno arrivando per prendermi! Vengono per portarti via..\r\n" );									break;
	  case 32:	strcpy ( t, "Sembra che le mie membra abbiano ritrovato vigore, possenza ed equilibrio.\r\n" );					break;
	  case 33:	strcpy ( t, "Inizio a sentirmi invincibile!\r\n" );																break;
	  case 34:	sprintf( t, "Sono legger$x più dell'aria stessa. I cieli sono a mia disposizione, posso prenderli!\r\n" );		break;
	  case 35:	strcpy ( t, "Sono luce, aria.. i Piani Superiori aspettano solo la mia venuta.\r\n" );							break;
	  case 36:	strcpy ( t, "Tutta la tua vita mi balena davanti agli occhi, veloce come un lampo.. ma.. il mio futuro?\r\n" );	break;
	  case 37:	strcpy ( t, "Rivivo frammenti della mia vita.. e ho visioni sul mio futuro!\r\n" );								break;
	  case 38:	strcpy ( t, "Incredibile! Sono dappertutto e so tutto, sento tutto e conosco ogni cosa!\r\n" );					break;
	  case 39:	strcpy ( t, "Mi trovo in ogni posto e sono ogni cosa.. Conosco tutto e so tutto!\r\n" );						break;
	  case 40:	strcpy ( t, "Mi sento invincibile!\r\n" );																		break;
	  case 41:	strcpy ( t, "Mi sento immortale!\r\n" );																		break;
	  case 42:	strcpy ( t, "Ahhh.. questo è il sapore del Potere di un'Entità Suprema! Vediamo cosa posso fare.\r\n" );		break;
	  case 43:	strcpy ( t, "Ahh! Il potere dell'Entità Suprema.. cosa farci?\r\n" );											break;
	}

	act( AT_PLAIN, t, ch, NULL, NULL, TO_CHAR );
}


/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void update_mobile( void )
{
	CHAR_DATA *ch;
	EXIT_DATA *pexit;
	int		   door;
	ch_ret	   retcode;

	retcode = rNONE;

	/* Examine all mobs. */
	for ( ch = last_char;  ch;  ch = gch_prev )
	{
		set_cur_char( ch );
		if ( ch == first_char && ch->prev )
		{
			send_log( NULL, LOG_BUG, "update_mobile: first_char->prev != NULL... corretto." );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;

		if ( gch_prev && gch_prev->next != ch )
		{
			send_log( NULL, LOG_BUG, "update_mobile: %s->prev->next non puntano a ch.  Short-cutting here.", ch->name );
			gch_prev = NULL;
			ch->prev = NULL;
			send_command( ch, "admtalk --> Prepararsi al peggio! <--", CO );
		}

		if ( IS_PG(ch) )
		{
			drunk_randoms( ch );
			hallucinations( ch );
			continue;
		}

		/*
		 * Da qui in giù è tutto per i Mob
		 */
#ifdef T2_ALFA
/* Tengo qui tanto per ricordarmi che devo spostarlo nel manuale che verrà
A mob with a 'DAY' flag will only reset/pop during the day. While in the game
during the night, the mob will be extracted with a room message.

A mob with a 'NIGHT' flag will only reset/pop during the night. While in the game
during the day, the mob will be extracted with a room message.

        **[ Building examples/ideas ]**

Imagine a graveyard area, during the day fairly quiet and peaceful, except for a few
rotting zombies shuffling about, rotting body parts falling off etc. Then as night
approaches, scores of undead rise up from the grave, ghouls, wraiths, ghosts..
well you get the picture.. Then as the day arrives, these nocturnal undead flee from
the light of day, making the graveyard fairly safe once again......

or

The town of SomeName during the day the good folks travel about conducting business,
children playing, shopkeeps sell wares etc.. Then as night approaches the town's folk
head home for the night leaving the streets empty. Perhaps a few guards on fire watch
complain about being on the night shift...

Make quests more challenging trying to find a mob out only during a certain time...*/
#endif
		/** Extract Day and Night mobs ** Rythmic **/
		if ( HAS_BIT_ACT(ch, MOB_DAY) && calendar.sunlight ==  SUN_DARK )
		{
			act( AT_ACTION, "$n fugge dall'oscurità.", ch, NULL, NULL, TO_ROOM );
			extract_char( ch, TRUE );
			continue;
		}
		if ( HAS_BIT_ACT(ch, MOB_NIGHT) && calendar.sunlight == SUN_LIGHT )
		{
			act( AT_ACTION, "$n fugge dalla luce.", ch, NULL, NULL, TO_ROOM );
			extract_char( ch, TRUE );
			continue;
		}

		if ( !ch->in_room )							continue;
		if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) )			continue;
		if ( HAS_BIT(ch->affected_by, AFFECT_PARALYSIS) )		continue;

		/* Clean up 'animated corpses' that are not charmed' */
		if ( ch->pIndexData->vnum == VNUM_MOB_ANIMATED_CORPSE && !HAS_BIT(ch->affected_by, AFFECT_CHARM) )
		{
			if ( ch->in_room->first_person )
				act( AT_MAGIC, "$n ritorna alla polvere dalla quale provenne.", ch, NULL, NULL, TO_ROOM );

			extract_char( ch, TRUE );
			continue;
		}

		if ( !HAS_BIT_ACT(ch, MOB_RUNNING)
		  && !HAS_BIT_ACT(ch, MOB_SENTINEL)
		  && !ch->fighting && ch->hunting )
		{
			WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
/* Commented out temporarily to avoid spam
			send_log( NULL, LOG_NORMAL, "%s hunting %s from %s.",
				ch->name, ch->hunting->name, ch->in_room->name );
*/
			hunt_victim( ch );
			continue;
		}

		/* Examine call for special procedure */
		if ( !HAS_BIT_ACT(ch, MOB_RUNNING) && ch->spec_fun )
		{
			if ( (*ch->spec_fun) (ch) )		continue;
			if ( char_died(ch) )			continue;
		}

		if ( ch != cur_char )
		{
			send_log( NULL, LOG_BUG, "update_mobile: ch != cur_char dopo spec_fun" );
			continue;
		}

		/* That's all for sleeping / busy monster */
		if ( ch->position <= POSITION_SLEEP )
			continue;

		if ( HAS_BIT_ACT(ch, MOB_MOUNTED) )
		{
			if ( HAS_BIT_ACT(ch, MOB_AGGRESSIVE) || HAS_BIT_ACT(ch, MOB_META_AGGR) )
				send_command( ch, "emote ringhia e grugnisce rumorosamente.", CO );

			continue;
		}

		if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE)
		  && (HAS_BIT_ACT(ch, MOB_AGGRESSIVE) || HAS_BIT_ACT(ch, MOB_META_AGGR)) )
		{
			if ( number_range(0, 5) == 0 )
				send_command( ch, "emote ringhia, con evidente irritazione..", CO );
		}

		/* MOBprogram random trigger */
		if ( ch->in_room->area->nplayer > 0 )
		{
			mprog_random_trigger( ch );
			if ( char_died(ch) )
				continue;
			if ( ch->position < POSITION_STAND )	/* (FF) pensare se qui, e più sotto, non accettare il POSITION_SIT */
				continue;
		}

		if ( char_died(ch) )				continue;
		if ( char_died(ch) )				continue;
		if ( ch->position < POSITION_STAND )		continue;

		/* Scavenge */
		if ( HAS_BIT_ACT(ch, MOB_SCAVENGER)
		  && ch->in_room->first_content
		  && number_bits(2) == 0 )
		{
			OBJ_DATA *obj;
			OBJ_DATA *obj_best;
			int max;

			max		 = 1;
			obj_best = NULL;

			for ( obj = ch->in_room->first_content;  obj;  obj = obj->next_content )
			{
				if ( HAS_BIT(obj->wear_flags, OBJWEAR_TAKE) && obj->cost > max
				  && !HAS_BIT(obj->extra_flags, OBJFLAG_BURIED) )
				{
					obj_best = obj;
					max		 = obj->cost;
				}
			}

			if ( obj_best )
			{
				obj_from_room( obj_best );
				obj_to_char( obj_best, ch );
				act( AT_ACTION, "$n prende $p.", ch, obj_best, NULL, TO_ROOM );
			}
		}

		/* Wander */
		if ( !HAS_BIT_ACT(ch, MOB_RUNNING)
		  && !HAS_BIT_ACT(ch, MOB_SENTINEL)
		  && (door = number_bits(5)) <= 9
		  && (pexit = get_exit(ch->in_room, door)) != NULL
		  && pexit->to_room
		  && !HAS_BIT(pexit->flags, EXIT_CLOSED)
		  && !HAS_BIT(pexit->to_room->flags, ROOM_NOMOB)
		  && !HAS_BIT(pexit->to_room->flags, ROOM_DEATH)
		  && (!HAS_BIT_ACT(ch, MOB_STAY_AREA) || pexit->to_room->area == ch->in_room->area) )
		{
			retcode = move_char( ch, pexit, 0, FALSE );

			if ( char_died(ch) )					continue;
			if ( retcode != rNONE )					continue;
			if ( HAS_BIT_ACT(ch, MOB_SENTINEL) )	continue;
			if ( ch->position < POSITION_STAND )			continue;	/* If ch changes position due to it's or someother mob's movement via MOBProgs, continue */
			if ( ch->desc && ch->desc->original )	continue;	/* (TT) da provare a vedere se così i mob switchati stanno fermi */
		}

		/* Flee */
		if ( ch->points[POINTS_LIFE] < ch->points_max[POINTS_LIFE]/2
		  && (door = number_bits(4)) <= 9
		  && (pexit = get_exit(ch->in_room,door)) != NULL
		  && pexit->to_room
		  && !HAS_BIT(pexit->flags, EXIT_CLOSED)
		  && !HAS_BIT(pexit->to_room->flags, ROOM_NOMOB) )
		{
			CHAR_DATA *rch;
			bool found;

			found = FALSE;

			for ( rch  = ch->in_room->first_person;  rch;  rch  = rch->next_in_room )
			{
				if ( is_fearing(ch, rch) )
				{
					char	buf[MSL];

					switch ( number_bits(2) )
					{
					  case 0:	sprintf( buf, "yell Stai lontano da me, %s!", rch->name );					break;
					  case 1:	sprintf( buf, "yell Lasciami andare, %s!", rch->name );						break;
					  case 2:	sprintf( buf, "yell %s sta cercando di assassinarmi!! Aiuto!", rch->name );	break;
					  case 3:	sprintf( buf, "yell Qualcuno mi salvi da %s!", rch->name );					break;
					}
					send_command( ch, buf, CO );
					found = TRUE;
					break;
				}
			}

			if ( found )
				retcode = move_char( ch, pexit, 0, FALSE );
		} /* chiude l'if */
	} /* chiude il for */
}


/*
 * Controlla se il pg debba andare in afk.
 */
void check_afk( CHAR_DATA *ch )
{
	if ( IS_MOB(ch) )
		return;

	/* AFK */
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_afk: ch è NULL!" );
		return;
	}

	/* Se ha il descrittore e se sta giocando
	 *	e se ha un idle alto
	 *	e se non è in afk
	 *	e se è admin e non è in afk2 */
	if ( ch->desc
	  && ch->desc->connected == CON_PLAYING
	  && ch->desc->idle > 10 * 60 * PULSE_IN_SECOND
	  && !HAS_BIT_PLR(ch, PLAYER_AFK)
	  && (IS_ADMIN(ch) && !HAS_BIT_PLR(ch, PLAYER_AFK2)) )
	{
		send_command( ch, "afk", CO );
	}
}


/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void update_char( void )
{
	CHAR_DATA   *ch;
	CHAR_DATA   *ch_save;
	int			 save_count = 0;

	ch_save	= NULL;

	for ( ch = last_char;  ch;  ch = gch_prev )
	{
		if ( ch == first_char && ch->prev )
		{
			send_log( NULL, LOG_BUG, "update_char: first_char->prev != NULL, corretto." );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;
		set_cur_char( ch );

		if ( gch_prev && gch_prev->next != ch )
		{
			send_log( NULL, LOG_BUG, "update_char: ch->prev->next != ch" );
			return;
		}

		/*
		 *  Do a room_prog rand check right off the bat
		 *   if ch disappears (rprog might wax npc's), continue
		 */
		if ( IS_PG(ch) )
			rprog_random_trigger( ch );

		if ( char_died(ch) )
			continue;

		if ( IS_MOB(ch) )
			mprog_time_trigger( ch );

		if ( char_died(ch) )
			continue;

		rprog_time_trigger( ch );

		if ( char_died(ch) )
			continue;

		/*
		 * See if player should be auto-saved.
		 */
		if ( IS_PG(ch) && (!ch->desc || ch->desc->connected == CON_PLAYING)
		  && current_time - ch->pg->save_time > (sysdata.save_frequency * 60) )
		{
			ch_save	= ch;
		}
		else
		{
			ch_save	= NULL;
		}

		/* Morph timer expires */
		if ( ch->morph )
		{
			if ( ch->morph->timer > 0 )
			{
				ch->morph->timer--;

				if ( ch->morph->timer == 0 )
					do_unmorph_char( ch );
			}
		}

		if ( IS_PG(ch) )
		{
			OBJ_DATA *obj;

			/* Dopo un tot di tempo rimuove la flag d'anima */
			if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SPIRIT)
			  && get_timer(ch, TIMER_KILLED)  < SPIRIT_TIME/2
			  && get_timer(ch, TIMER_PKILLED) < SPIRIT_TIME/2 )
			{
				REMOVE_BIT( ch->pg->flags, PLAYER_SPIRIT );
				set_char_color( AT_WHITE, ch );	/* (FF) AT_SOUL */
				send_command( ch, "help _MSG_SOUL_TO_CORP_", CO );
			}

			if ( (obj = get_eq_char(ch, WEARLOC_LIGHT)) != NULL
			  && obj->type == OBJTYPE_LIGHT
			  && obj->light->hours > 0 )
			{
				if ( --obj->light->hours == 0 && ch->in_room )
				{
					/* Prima visualizza cosa si sta spegnendo poi diminuisce la luce nella stanza */
					act( AT_ACTION, "$p si spegne.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "$p si spegne.", ch, obj, NULL, TO_CHAR );

					ch->in_room->light -= obj->count;
					if ( ch->in_room->light < 0 )
						ch->in_room->light = 0;

					if ( obj->serial == cur_obj )
						global_objcode = rOBJ_EXPIRED;

					free_object( obj );
				}
			}

			if ( ++ch->timer >= 18 && !is_idle(ch) && !IS_ADMIN(ch) )
			{
				if ( ch->fighting )
					stop_fighting( ch, TRUE );

				act( AT_ACTION, "$n scompare nel vuoto.", ch, NULL, NULL, TO_ROOM );
				send_to_char( ch, "Lentamente mi dissolvo nel vuoto.\r\n" );

				send_log( NULL, LOG_COMM, "update_char: %s va in idle", ch->name );
				save_player( ch );

				SET_BIT( ch->pg->flags, PLAYER_IDLE );
				char_from_room( ch );
				char_to_room( ch, get_room_index(NULL, VNUM_ROOM_LIMBO) );
			}

			/* Gli admin e le anime non soffrono fame e sete */
			if ( !IS_ADMIN(ch)|| !HAS_BIT_PLR(ch, PLAYER_SPIRIT) )
			{
				if ( ch->pg->condition[CONDITION_DRUNK] > 8 )
					worsen_mental_state( ch, ch->pg->condition[CONDITION_DRUNK]/8 );

				/* (FF) Fare altri check per il better_ e worsen_ in questi punti */
				if ( ch->pg->condition[CONDITION_FULL] > 1 )
				{
					switch ( ch->position )
					{
					  case POSITION_SLEEP:		better_mental_state( ch, 4 );	break;
					  case POSITION_REST:		better_mental_state( ch, 3 );	break;
					  case POSITION_SIT:		better_mental_state( ch, 1 );	break;
					  case POSITION_MOUNT:		better_mental_state( ch, 2 );	break;
					  case POSITION_STAND:		better_mental_state( ch, 1 );	break;
					  case POSITION_FIGHT:
					  case POSITION_EVASIVE:
					  case POSITION_DEFENSIVE:
					  case POSITION_AGGRESSIVE:
					  case POSITION_BERSERK:
						if ( number_bits(2) == 0 )
							better_mental_state( ch, 1 );
						break;
					}
				}

				if ( ch->pg->condition[CONDITION_THIRST] > 1 )
				{
					switch ( ch->position )
					{
					  case POSITION_SLEEP:		better_mental_state( ch, 5 );	break;
					  case POSITION_REST:		better_mental_state( ch, 3 );	break;
					  case POSITION_SIT:		better_mental_state( ch, 1 );	break;
					  case POSITION_MOUNT:		better_mental_state( ch, 2 );	break;
					  case POSITION_STAND:		better_mental_state( ch, 1 );	break;
					  case POSITION_FIGHT:
					  case POSITION_EVASIVE:
					  case POSITION_DEFENSIVE:
					  case POSITION_AGGRESSIVE:
					  case POSITION_BERSERK:
						if ( number_bits(2) == 0 )
							better_mental_state( ch, 1 );
						break;
					}
				}
			}

			if ( !IS_ADMIN(ch) )
			{
				check_alignment( ch );	/* Togliere il check anche per le anime? da pensare (FF) */
				check_afk( ch );
			}
		}

		if ( IS_PG(ch) && !IS_ADMIN(ch)
		  && ch->pg->release_date > 0
		  && ch->pg->release_date <= current_time )
		{
			ROOM_DATA *location;

			if ( ch->pg->clan )
				location = ch->pg->clan->recall;
			else
				location = get_room_index( NULL, VNUM_ROOM_TEMPLE );

			if ( !location )
				location = ch->in_room;

			char_from_room( ch );
			char_to_room( ch, location );
			MOBtrigger = FALSE;
			ch_printf( ch, "Gli Dei mi hanno assolt%c dall'inferno e la tua sentenza è sciolta.\r\n",
				gramm_ao(ch) );		/* meglio una ch_printf di una act perché ho timore di rendere TRUE MOBTrigger troppo presto */
			send_command( ch, "look", CO );
			DISPOSE( ch->pg->helled_by );
			ch->pg->helled_by = NULL;
			ch->pg->release_date = 0;
			save_player( ch );
		}

		if ( !char_died(ch) )
		{
			/* Le anime non vengono avvelenate, però sto check non mi piace, l'affect poison non dovrebbe essere preso a priori (FF) da pensare */
			if ( IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_SPIRIT) )
			{
				/*
				 * Careful with the damages here,
				 *  MUST NOT refer to ch after damage taken, without checking
				 *  return code and/or char_died as it may be lethal damage.
				 */
				if ( HAS_BIT(ch->affected_by, AFFECT_POISON) )
				{
					act( AT_POISON, "$n sta tremando, è ammalat$x.", ch, NULL, NULL, TO_ROOM );
					set_char_color( AT_POISON, ch );
					ch_printf( ch, "Sto tremando, sono ammalat%c.", gramm_ao(ch) );
#ifdef T2_MSP
					send_audio( ch->desc, "poison.wav", TO_CHAR );
#endif
					ch->mental_state = URANGE( 20, ch->mental_state
						+ (IS_MOB(ch)  ?  2  :  4), 100 );
					damage( ch, ch, 6, gsn_poison );
				}
				else if ( ch->position == POSITION_INCAP )
					damage( ch, ch, 1, TYPE_UNDEFINED );
				else if ( ch->position == POSITION_MORTAL )
					damage( ch, ch, 4, TYPE_UNDEFINED );
			}

			if ( char_died(ch) )
				continue;

			/*
			 * Recurring spell affect
			 */
			if ( HAS_BIT(ch->affected_by, AFFECT_RECURRING_SPELL) )
			{
				AFFECT_DATA	*paf,
							*paf_next;
				SKILL_DATA	*skill;
				bool found = FALSE, died = FALSE;

				for ( paf = ch->first_affect;  paf;  paf = paf_next )
				{
					paf_next = paf->next;
					if ( paf->location == APPLY_RECURRINGSPELL )
					{
						found = TRUE;
						if ( is_valid_sn(paf->modifier)
						  && (skill = table_skill[paf->modifier]) != NULL
						  && skill->type == SKILLTYPE_SPELL )
						{
							if ( (*skill->spell_fun)(paf->modifier, get_level(ch)/2, ch, ch) == rCHAR_DIED
							  || char_died(ch) )
							{
								died = TRUE;
								break;
							}
						}
					}
				}

				if ( died )
					continue;

				if ( !found )
					REMOVE_BIT(ch->affected_by, AFFECT_RECURRING_SPELL);
			} /* chiude l'if */

			if ( ch->mental_state >= 30 && (IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_SPIRIT)) )
			{
				switch ( (ch->mental_state+5) / 10 )
				{
				  case  3:
					send_to_char( ch, "Sono febbricitante..\r\n" );
					act( AT_ACTION, "$n delira in preda alla febbre.", ch, NULL, NULL, TO_ROOM );
					break;
				  case  4:
					send_to_char( ch, "Non mi sento affatto bene.\r\n" );
					act( AT_ACTION, "$n non sta affatto bene in salute.", ch, NULL, NULL, TO_ROOM );
					break;
				  case  5:
					send_to_char( ch, "Ho decisamente bisogno di aiuto!\r\n" );
					act( AT_ACTION, "Decisamente, $n ha bisogno di aiuto.", ch, NULL, NULL, TO_ROOM );
					break;
				  case  6:
					send_to_char( ch, "Farei bene a cercare un curatore..\r\n" );
					act( AT_ACTION, "Forse è il caso di rintracciare un guaritore per $n, al più presto..", ch, NULL, NULL, TO_ROOM );
					break;
				  case  7:
					send_to_char( ch, "Sto perdendo i sensi, il senso del reale mi sta abbandonando..\r\n" );
					act( AT_ACTION, "Sembra che $n non si renda conto di quanto accade..", ch, NULL, NULL, TO_ROOM );
					break;
				  case  8:
					send_to_char( ch, "Comincio a capire.. Tutto!\r\n" );
					act( AT_ACTION, "$n comincia ad agitarsi in modo del tutto imprevedibile!", ch, NULL, NULL, TO_ROOM );
					break;
				  case  9:
					send_to_char( ch, "Mi sento tutt'uno con l'universo, sì!\r\n" );
					act( AT_ACTION, "$n sta riflettendo, sembra che voglia declamare qualcosa..", ch, NULL, NULL, TO_ROOM );
					break;
				  case 10:
					send_to_char( ch, "La fine s'avvicina, lo sento!\r\n" );
					act( AT_ACTION, "$n sta mormorando qualcosa in una lingua sconosciuta e dimenticata..", ch, NULL, NULL, TO_ROOM );
					break;
				}
			}

			if ( ch->mental_state <= -30 && (IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_SPIRIT)) )
			{
				switch ( (abs(ch->mental_state)+5) / 10 )
				{
				  case 10:
					if ( ch->position > POSITION_SLEEP )
					{
						if ( (ch->position == POSITION_STAND
						  || ch->position < POSITION_FIGHT)
						  && number_percent()+10 < abs(ch->mental_state) )
						{
							send_command( ch, "sleep", CO );
						}
						else
						{
							send_to_char( ch, "La mia consapevolezza sta diminuendo, il senso del reale si allontana.\r\n" );
						}
					}
					break;

				  case  9:
					if ( ch->position > POSITION_SLEEP )
					{
						if ( (ch->position == POSITION_STAND
						  || ch->position < POSITION_FIGHT)
						  && (number_percent()+20) < abs(ch->mental_state) )
						{
							send_command( ch, "sleep", CO );
						}
						else
						{
							send_to_char( ch, "Riesco stentatamente a tenere gli occhi aperti.\r\n" );
						}
					}
					break;

				  case  8:
					if ( ch->position > POSITION_SLEEP )
					{
						if ( ch->position < POSITION_SIT
						  && (number_percent()+30) < abs(ch->mental_state) )
						{
							send_command( ch, "sleep", CO );
						}
						else
						{
							send_to_char( ch, "Mi sento debole e la sonnolenza avanza.\r\n" );
						}
					}
					break;

				  case  7:
					if ( ch->position > POSITION_REST )
						send_to_char( ch, "Lo sconforto mi attanaglia in una morsa stringente..\r\n" );
					break;

				  case  6:
					if ( ch->position > POSITION_REST )
						send_to_char( ch, "Forse va meglio: un senso di calma m'avvolge.\r\n" );
					break;

				  case  5:
					if ( ch->position > POSITION_REST )
						send_to_char( ch, "Un'improvvisa sonnolenza mi pervade. Sto proprio sbadigliando..\r\n" );
					break;

				  case  4:
					if ( ch->position > POSITION_REST )
						send_to_char( ch, "La stanchezza comincia a farsi sentire..\r\n" );
					break;

				  case  3:
					if ( ch->position > POSITION_REST )
						send_to_char( ch, "Forse dovrei riposare un poco.\r\n" );
					break;
				}
			}

			if ( IS_PG(ch) && ch->timer > 36 )
			{
				send_log( NULL, LOG_COMM, "update_char: check timer di idle alto, uscita del pg %s", ch->name );
				send_command( ch, "quit", CO );
			}
			else if ( ch == ch_save && ++save_count < 5 )	/* save max of 10 per tick */
				save_player( ch );
		} /* chiude l'if */
	} /* chiude il for */
}


void remove_portal( OBJ_DATA *portal )
{
	ROOM_DATA	*fromRoom,
				*toRoom;
	EXIT_DATA	*pexit;
	bool		 found;

	if ( !portal )
	{
		send_log( NULL, LOG_BUG, "remove_portal: portal è NULL." );
		return;
	}

	fromRoom = portal->in_room;
	found = FALSE;

	if ( !fromRoom )
	{
		send_log( NULL, LOG_BUG, "remove_portal: portal->in_room è NULL." );
		return;
	}

	for ( pexit = fromRoom->first_exit;  pexit;  pexit = pexit->next )
	{
		if ( HAS_BIT(pexit->flags, EXIT_PORTAL) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_log( NULL, LOG_BUG, "remove_portal: portale non trovato nella stanza %d.", fromRoom->vnum );
		return;
	}

	if ( pexit->vdir != DIR_SOMEWHERE )
		send_log( NULL, LOG_BUG, "remove_portal: l'uscita nella direzione %d != DIR_SOMEWHERE", pexit->vdir );

	if ( (toRoom = pexit->to_room) == NULL )
		send_log( NULL, LOG_BUG, "remove_portal: toRoom è NULL." );

	extract_exit( fromRoom, pexit );
}


/*
 * Update all objs. This function is performance sensitive.
 */
void update_obj( void )
{
	OBJ_DATA *obj;
	int		  AT_TEMP;

	for ( obj = last_object;  obj;  obj = gobj_prev )
	{
		CHAR_DATA *rch;
		char	  *message;

		if ( obj == first_object && obj->prev )
		{
			send_log( NULL, LOG_BUG, "update_obj: first_object->prev != NULL, corretto" );
			obj->prev = NULL;
		}
		gobj_prev = obj->prev;

		if ( gobj_prev && gobj_prev->next != obj )
		{
			send_log( NULL, LOG_BUG, "update_obj: obj->prev->next != obj" );
			return;
		}
		set_cur_obj( obj );

		if		( obj->carried_by )
			oprog_random_trigger(obj);
		else if ( obj->in_room && obj->in_room->area->nplayer > 0 )
			oprog_random_trigger(obj);

		if ( obj_extracted(obj) )
			continue;

		if ( obj->type == OBJTYPE_PIPE )
		{
			if ( HAS_BITINT(obj->pipe->flags, PIPE_LIT) )
			{
				if ( --obj->pipe->draws <= 0 )
				{
					obj->pipe->draws = 0;
					REMOVE_BITINT( obj->pipe->flags, PIPE_LIT );
				}
				else if ( HAS_BITINT( obj->pipe->flags, PIPE_HOT ) )
				{
					REMOVE_BITINT( obj->pipe->flags, PIPE_HOT );
				}
				else
				{
					if ( HAS_BITINT(obj->pipe->flags, PIPE_GOINGOUT) )
					{
						REMOVE_BITINT( obj->pipe->flags, PIPE_LIT );
						REMOVE_BITINT( obj->pipe->flags, PIPE_GOINGOUT );
					}
					else
						SET_BITINT( obj->pipe->flags, PIPE_GOINGOUT );
				}

				if ( !HAS_BITINT(obj->pipe->flags, PIPE_LIT) )
					SET_BITINT( obj->pipe->flags, PIPE_FULLOFASH );
			}
			else
				REMOVE_BITINT( obj->pipe->flags, PIPE_HOT );
		}


		/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) */
		if ( obj->type == OBJTYPE_CORPSE_PG || obj->type == OBJTYPE_CORPSE_MOB )
		{
			int	timerfrac = UMAX(1, obj->timer - 1);
			if ( obj->type == OBJTYPE_CORPSE_PG )
				timerfrac = (int)(obj->timer / 8 + 1);

			if ( obj->timer > 0 && obj->corpse->time > timerfrac )
			{
				char	 buf[MSL];
				char	 name[MSL];
				char	*bufptr;

				/* (TT) (bb) Alcune volte è capitato ai pg che non copiasse il nome, strano perché dai test pare che sia tutto ok */
				bufptr = one_argument( obj->short_descr, name );
				bufptr = one_argument( bufptr, name );
				bufptr = one_argument( bufptr, name );
				bufptr = one_argument( bufptr, name );

				split_obj( obj, 1 );
				obj->corpse->time = timerfrac;
				sprintf( buf, corpse_descrs[ UMIN(timerfrac-1, 4) ], bufptr );

				DISPOSE( obj->long_descr );
				obj->long_descr = str_dup( buf );
			}
		}

		/* don't let inventory decay */
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVENTORY) )
			continue;

		/* groundrot items only decay on the ground */
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_GROUNDROT) && !obj->in_room )
			continue;

		if ( obj->timer <= 0 || --obj->timer > 0 )
			continue;

		/* if we get this far, object's timer has expired. */
		AT_TEMP = AT_PLAIN;
		switch ( obj->type )
		{
		  default:
			message = "$p svanisce misteriosamente nell'aria.";
			AT_TEMP = AT_PLAIN;
			break;

		  case OBJTYPE_CONTAINER:
			message = "$p si spacca ai lati, distrutto dall'età.";
			AT_TEMP = AT_OBJECT;
			break;

		  case OBJTYPE_PORTAL:
			message = "$p si dissolve in mille bagliori di luce.";
			remove_portal( obj );
			obj->type = OBJTYPE_TRASH;		/* so free_object	 */
			AT_TEMP = AT_MAGIC;			/* doesn't remove_portal */
			break;

		  case OBJTYPE_FOUNTAIN:
			message = "$p zampilla allegramente.";
			AT_TEMP = AT_BLUE;
			break;

		  case OBJTYPE_CORPSE_MOB:
			message = "$p si sgretola in polvere disperdendosi in terra.";
			AT_TEMP = AT_OBJECT;
			break;

		  case OBJTYPE_CORPSE_PG:
			message = "$p viene risucchiato da un vortice di colori brillanti.";
			AT_TEMP = AT_MAGIC;
			break;

		  case OBJTYPE_COOK:
		  case OBJTYPE_FOOD:
			message = "$p è divorato da una torma di vermi.";
			AT_TEMP = AT_HUNGRY;
			break;

		  case OBJTYPE_BLOOD:
			message = "$p gocciola lentamente sul terreno.";
			AT_TEMP = AT_DRED;
			break;

		  case OBJTYPE_BLOODSTAIN:
/* (FF) colore del sangue preso dalla caratteristica de tipo di oggetto, o meglio ancora da quello della razza del mob morto */
			message = "$p si dipana in reticolo vermiglio per poi sparire assorbito dal terreno.";
			AT_TEMP = AT_DRED;
			break;

		  case OBJTYPE_SCRAPS:
			message = "$p si sbriciolano in un istante, polverizzandosi.";
			AT_TEMP = AT_OBJECT;
			break;

		  case OBJTYPE_FIRE:
			message = "$p brucia consumandosi.";
			AT_TEMP = AT_FIRE;
		}

		if ( obj->carried_by )
		{
			act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
		}
		else if ( obj->in_room
		  && ( rch = obj->in_room->first_person ) != NULL
		  && !HAS_BIT(obj->extra_flags, OBJFLAG_BURIED) )
		{
			act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
			act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
		}

		if ( obj->serial == cur_obj )
			global_objcode = rOBJ_EXPIRED;

		free_object( obj );
	} /* chiude il for */
}


/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	OBJ_DATA  *obj;
	EXIT_DATA *pexit;
	static int cnt = 0;
	int		   door;
	ch_ret	   retcode;

	/* This little counter can be used to handle periodic events */
	cnt = (cnt+1) % SECONDS_PER_TICK;

	for ( ch = first_char;  ch;  ch = ch_next )
	{
		set_cur_char( ch );
		ch_next = ch->next;

		will_fall( ch, 0 );

		if ( char_died(ch) )
			continue;

		if ( IS_MOB(ch) )
		{
			if ( (cnt & 1) )
				continue;

			/* running mobs */
			if ( HAS_BIT_ACT(ch, MOB_RUNNING) )
			{
				if ( !HAS_BIT_ACT(ch, MOB_SENTINEL)
				  && !ch->fighting && ch->hunting )
				{
					WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
					hunt_victim( ch );
					continue;
				}

				if ( ch->spec_fun )
				{
					if ( (*ch->spec_fun) (ch) )
						continue;
					if ( char_died(ch) )
						continue;
				}

				if ( !HAS_BIT_ACT(ch, MOB_SENTINEL)
				  && (door = number_bits(4)) <= 9
				  && (pexit = get_exit(ch->in_room, door)) != NULL
				  && pexit->to_room
				  && !HAS_BIT(pexit->flags, EXIT_CLOSED)
				  && !HAS_BIT(pexit->to_room->flags, ROOM_NOMOB)
				  && !HAS_BIT(pexit->to_room->flags, ROOM_DEATH)
				  && (!HAS_BIT_ACT(ch, MOB_STAY_AREA)
				  || pexit->to_room->area == ch->in_room->area) )
				{
					retcode = move_char( ch, pexit, 0, FALSE );

					if ( char_died(ch) )					continue;
					if ( retcode != rNONE )					continue;
					if ( HAS_BIT_ACT(ch, MOB_SENTINEL) )	continue;
					if ( ch->position < POSITION_STAND )			continue;
					if ( ch->desc && ch->desc->original )	continue;	/* (TT) da provare a vedere se così i mob switchati stanno fermi */
				}
			} /* chiude l'if */
			continue;
		}
		else
		{
			if ( ch->mount && IS_MOB(ch->mount) && ch->in_room != ch->mount->in_room )
			{
				REMOVE_BIT( ch->mount->act, MOB_MOUNTED );
				ch->mount = NULL;
				ch->position = POSITION_SIT;
				act( AT_FALLING, "La tua cavalcatura sparisce e tu cadi rovinosamente..\r\nAhia!\r\n", ch, NULL, NULL, TO_CHAR );
				/* (FF) aggiungerci un po' di bua? */
			}

			if (  ch->in_room
			  && (ch->in_room->sector == SECTOR_UNDERWATER
			  ||  ch->in_room->sector == SECTOR_OCEANFLOOR) )
			{
				if ( !HAS_BIT(ch->affected_by, AFFECT_AQUA_BREATH) )
				{
					int dam;

					dam = number_range( ch->points_max[POINTS_LIFE]/100, ch->points_max[POINTS_LIFE]/50 );
					dam = UMAX( 1, dam );
					if ( number_bits(3) == 0 )
						send_to_char( ch, "Tossisci e ingurgiti acqua nel tentativo disperato di respirare..\r\n" );
					damage( ch, ch, dam, TYPE_UNDEFINED );
				}
			}

			if ( char_died(ch) )
				continue;

			if (  ch->in_room
			  && (ch->in_room->sector == SECTOR_WATER_NOSWIM
			  ||  ch->in_room->sector == SECTOR_WATER_SWIM) )
			{
				if ( !HAS_BIT(ch->affected_by, AFFECT_FLYING)
				  && !HAS_BIT(ch->affected_by, AFFECT_FLOATING)
				  && !HAS_BIT(ch->affected_by, AFFECT_AQUA_BREATH)
				  && !ch->mount )
				{
					for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
					{
						if ( obj->type == OBJTYPE_BOAT )
							break;
					}

					if ( !obj )
					{
						if ( ch->points[POINTS_MOVE] > 0 )
						{
							int mov;

							mov = number_range( ch->points[POINTS_MOVE]/20, ch->points_max[POINTS_MOVE]/5 );
							mov = UMAX( 1, mov );

							if ( ch->points[POINTS_MOVE] - mov < 0 )
								ch->points[POINTS_MOVE] = 0;
							else
								ch->points[POINTS_MOVE] -= mov;
						}
						else if ( !IS_ADMIN(ch) )	/* Gli admin non muoiono soffocati nell'acqua */
						{
							int dam;

							dam = number_range( ch->points_max[POINTS_LIFE]/20, ch->points_max[POINTS_LIFE]/5 );
							dam = UMAX( 1, dam );

							if ( number_bits(3) == 0 )
								act( AT_THIRSTY, "Lottando fino allo stremo, soffoco schiacciat$x da uno spesso muro d'acqua!", ch, NULL, NULL, TO_CHAR );
send_log( NULL, LOG_MONI, "char_chek: soffocamento nell'acqua di %s con danno %d", ch->name, dam );
							damage( ch, ch, dam, TYPE_UNDEFINED );
						}
					}
				}
			}

			/* beat up on link dead players */
			if ( !ch->desc )
			{
				CHAR_DATA *wch, *wch_next;

				for ( wch = ch->in_room->first_person;  wch;  wch = wch_next )
				{
					wch_next = wch->next_in_room;

					if ( IS_PG(wch) )									continue;
					if ( wch->fighting )								continue;
					if ( HAS_BIT(wch->affected_by, AFFECT_CHARM) )		continue;
					if ( !is_awake(wch) )								continue;
					if ( HAS_BIT_ACT(wch, MOB_WIMPY) && is_awake(ch) )	continue;
					if ( !can_see(wch, ch) )							continue;

					if ( is_hating(wch, ch) )
					{
						found_prey( wch, ch );
						continue;
					}

					if ( (!HAS_BIT_ACT(wch, MOB_AGGRESSIVE)
					  &&  !HAS_BIT_ACT(wch, MOB_META_AGGR)) )			continue;
					if ( HAS_BIT_ACT(wch, MOB_MOUNTED) )				continue;
					if ( HAS_BIT(wch->in_room->flags, ROOM_SAFE) )	continue;

					global_retcode = multi_hit( wch, ch, TYPE_UNDEFINED );
				}
			}
		} /* chiude l'else */
	} /* chiude il for */
}


/*
 * Aggress
 *
 * for each descriptor
 *	 for each mob in room
 *		 aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *  because we don't the mob to just attack the first PC
 *  who leads the party into the room.
 */
void update_aggr( void )
{
	DESCRIPTOR_DATA *d;
	DESCRIPTOR_DATA *dnext;
	CHAR_DATA		*wch;
	CHAR_DATA		*ch;
	CHAR_DATA		*ch_next;
	CHAR_DATA		*vch;
	CHAR_DATA		*vch_next;
	CHAR_DATA		*victim;
	struct act_prog_data *apdtmp;

#ifdef UNDEFD
	/*
	 *  GRUNT!  To do
	 */
	if ( IS_MOB(wch) && wch->mpactnum > 0
	  && wch->in_room->area->nplayer > 0 )
	{
		MPROG_ACT_LIST * tmp_act, *tmp2_act;
		for ( tmp_act = wch->mpact;  tmp_act;  tmp_act = tmp_act->next )
		{
			oprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,
			tmp_act->obj, tmp_act->vo, MPTRIGGER_ACT );
			DISPOSE( tmp_act->buf );
		}

		for ( tmp_act = wch->mpact;  tmp_act;  tmp_act = tmp2_act )
		{
			tmp2_act = tmp_act->next;
			DISPOSE( tmp_act );
		}
		wch->mpactnum = 0;
		wch->mpact	= NULL;
	}
#endif

	/* check mobprog act queue */
	while ( (apdtmp = mob_act_list) != NULL )
	{
		wch = mob_act_list->vo;
		if ( !char_died(wch) && wch->mpactnum > 0 )
		{
			MPROG_ACT_LIST * tmp_act;

			while ( (tmp_act = wch->mpact) != NULL )
			{
				if ( tmp_act->obj && obj_extracted(tmp_act->obj) )
					tmp_act->obj = NULL;

				if ( tmp_act->ch && !char_died(tmp_act->ch) )
				{
					mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
						tmp_act->obj, tmp_act->vo, MPTRIGGER_ACT );
				}
				wch->mpact = tmp_act->next;
				DISPOSE( tmp_act->buf );
				DISPOSE( tmp_act );
			}
			wch->mpactnum = 0;
			wch->mpact	  = NULL;
		}
		mob_act_list = apdtmp->next;
		DISPOSE( apdtmp );
	}

	/*
	 * Just check descriptors here for victims to aggressive mobs
	 * We can check for linkdead victims in char_check
	 */
	for ( d = first_descriptor;  d;  d = dnext )
	{
		dnext = d->next;

		if ( d->connected != CON_PLAYING )	continue;
		if ( (wch = d->character) == NULL )	continue;
		if ( char_died(wch) )				continue;
		if ( IS_MOB(wch) )					continue;
		if ( IS_ADMIN(wch) )				continue;
		if ( !wch->in_room )				continue;

		for ( ch = wch->in_room->first_person;  ch;  ch = ch_next )
		{
			int count;

			ch_next	= ch->next_in_room;

			if ( IS_PG(ch) )									continue;
			if ( ch->fighting )									continue;
			if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) )					continue;
			if ( !is_awake(ch) )								continue;
			if ( HAS_BIT_ACT(ch, MOB_WIMPY) && is_awake(wch) )	continue;
			if ( !can_see(ch, wch) )							continue;

			if ( is_hating(ch, wch) )
			{
				found_prey( ch, wch );
				continue;
			}

			if ( (!HAS_BIT_ACT(ch, MOB_AGGRESSIVE)
			  &&  !HAS_BIT_ACT(ch, MOB_META_AGGR)) )		continue;
			if ( HAS_BIT_ACT(ch, MOB_MOUNTED) )				continue;
			if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )	continue;

			/*
			 * Ok we have a 'wch' player character and a 'ch' npc aggressor.
			 * Now make the aggressor fight a RANDOM pc victim in the room,
			 *   giving each 'vch' an equal chance of selection.
			 *
			 * Depending on flags set, the mob may attack another mob
			 */
			count	= 0;
			victim	= NULL;
			for ( vch = wch->in_room->first_person;  vch;  vch = vch_next )
			{
				vch_next = vch->next_in_room;

				if ( IS_ADMIN(vch) )								continue;
				if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_WIMPY)
				  && is_awake(vch) )								continue;
				if ( !can_see(ch, vch) )							continue;

				if ( (IS_PG(vch)
				  || HAS_BIT_ACT(ch, MOB_META_AGGR)
				  || HAS_BIT_ACT(vch, MOB_ANNOYING)) )
				{
					if ( number_range(0, count) == 0 )
						victim = vch;
					count++;
				}
			}

			if ( !victim )
			{
				send_log( NULL, LOG_BUG, "update_aggr: victim è NULL" );
				continue;
			}

			/* backstabbing mobs */
			if ( IS_MOB(ch) && HAS_BIT(ch->attacks, ATTACK_BACKSTAB) )
			{
				OBJ_DATA *obj;

				if ( ch->mount )										continue;
				if ( (obj = get_eq_char(ch, WEARLOC_WIELD)) == NULL )		continue;
				if ( !obj->weapon )										continue;
				if ( obj->weapon->damage1 != DAMAGE_PIERCE
				  && obj->weapon->damage1 != DAMAGE_STAB )					continue;
				if ( victim->fighting )									continue;
				if ( victim->points[POINTS_LIFE] < victim->points_max[POINTS_LIFE] )	continue;

				check_attacker( ch, victim );
				WAIT_STATE( ch, table_skill[gsn_backstab]->beats * 3 );	/* (WT) */

				if ( !is_awake(victim) || number_percent()+5 < get_level(ch)/2 )
				{
					global_retcode = multi_hit( ch, victim, gsn_backstab );
					continue;
				}
				else
				{
					global_retcode = damage( ch, victim, 0, gsn_backstab );
					continue;
				}
			}
			global_retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
		} /* chiude il for */
	} /* chiude il for */
}


void subtract_times( struct timeval *estime_time, struct timeval *start_time )
{
	estime_time->tv_sec -= start_time->tv_sec;
	estime_time->tv_usec -= start_time->tv_usec;
	while ( estime_time->tv_usec < 0 )
	{
		estime_time->tv_usec += 1000000;
		estime_time->tv_sec--;
	}
}


/*
 * Repopulate areas periodically
 */
void update_area( void )
{
	AREA_DATA *pArea;

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		CHAR_DATA *pch;
		int reset_age = (pArea->reset_frequency) ? pArea->reset_frequency : 15;

		if ( reset_age == -1 && pArea->age == -1 )	continue;
		if ( ++pArea->age < reset_age-1 )			continue;

		/*
		 * Check for PC's
		 * Invia solo per il 50% il msg
		 */
		if ( pArea->nplayer > 0 && pArea->age == (reset_age-1) && number_range(0, 1) )
		{
			char buf[MSL];
			int num;

			num = number_range( 0, MAX_RESETMSG-1 );
			if ( VALID_STR(pArea->resetmsg[num]) )
				sprintf( buf, "%s\r\n", pArea->resetmsg[num] );
			else
				strcpy( buf, "Sento dei rumori striduli laggiù..\r\n" );

			for ( pch = first_char;  pch;  pch = pch->next )
			{
				if ( IS_MOB(pch) )									continue;
				if ( !is_awake(pch) )								continue;
				if ( !pch->in_room )								continue;

				if ( pch->in_room->area == pArea )
				{
					set_char_color( AT_RESET, pch );
					send_to_char( pch, buf );
				}
			}
		}

		/*
		 * Check age and reset
		 */
		if ( pArea->nplayer == 0 || pArea->age >= reset_age )
		{
			reset_area( pArea );
			if ( reset_age == -1 )
				pArea->age = -1;
			else
				pArea->age = number_range( 0, reset_age/5 );
		}
	} /* chiude il for */
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
	static int		pulse_ani;
	static int		pulse_area;
	static int		pulse_dt;
	static int		pulse_mobile;
	static int		pulse_violence;
	static int		pulse_char;
	static int		pulse_points;
	static int		pulse_condition;
	static int		pulse_hint;
#ifdef T2_MSP
	static int		pulse_sound;
#endif
	static int		pulse_share;
	static int		pulse_dream;
	static int		pulse_time;
	static int		pulse_hair;
	static int		pulse_weather;
	static int		pulse_mudstat;
	static int		pulse_second;
	struct timeval	start_time;
	struct timeval	etime;

	if ( timechar )
	{
		set_char_color( AT_PLAIN, timechar );
		send_to_char( timechar, "Starting update timer\r\n" );
		gettimeofday( &start_time, NULL );
	}

	if ( --pulse_area <= 0 )
	{
		pulse_area	= number_range( PULSE_AREA/2, PULSE_AREA*2 );
		update_area( );
	}

	if ( --pulse_dt <= 0 )
	{
		pulse_dt = number_range( PULSE_IN_SECOND, PULSE_IN_SECOND * 10 );
		update_dt( );
	}

	if ( --pulse_mobile <= 0 )
	{
		pulse_mobile = PULSE_MOBILE;
		update_mobile( );
	}

	if ( --pulse_violence <= 0 )
	{
		pulse_violence = PULSE_VIOLENCE;
		update_violence( );
	}

	if ( --pulse_char <= 0 )
	{
		pulse_char	 = number_range( PULSE_TICK * 0.75, PULSE_TICK * 1.25 );

		update_char( );
		update_obj( );
	}

	if ( --pulse_points <= 0 )
	{
		pulse_points = number_range( PULSE_TICK * 0.50, PULSE_TICK * 1.00 );

		update_points( );
	}

	if ( --pulse_condition <= 0 )
	{
		pulse_condition = number_range( PULSE_TICK * 1.50, PULSE_TICK * 2.50 );

		update_condition( );
	}

	if ( --pulse_hint <= 0 )
	{
		pulse_hint = number_range( PULSE_IN_SECOND * 60, PULSE_IN_SECOND * 120 );

		update_hint( );
	}

#ifdef T2_MSP
	if ( --pulse_sound <= 0 )
	{
		pulse_sound = number_range( PULSE_IN_SECOND * 60, PULSE_IN_SECOND * 120 );

		update_sound( );
	}
#endif
	if ( --pulse_share <= 0 )
	{
		pulse_share = PULSE_IN_SECOND * SECONDS_IN_HOUR;

		update_share( );
	}

	if ( --pulse_dream <= 0 )
	{
		pulse_dream = number_range( PULSE_IN_SECOND * 20, PULSE_IN_SECOND * 40 );

		update_dream( );
	}

	if ( --pulse_time <= 0 )
	{
		pulse_time = PULSE_IN_SECOND * SECONDS_IN_HOUR;

		update_calendar( );
	}

	if ( --pulse_hair <= 0 )
	{
		pulse_hair = PULSE_IN_SECOND * 60;		/* Ogni minuto reale controlla per la lunghezza dei capelli */

		update_hair( );
	}

	if ( --pulse_weather <= 0 )
	{
		pulse_weather = number_range( PULSE_TICK * 1.50, PULSE_TICK * 2.50 );

		update_weather( );
	}

	if ( --pulse_mudstat <= 0 )
	{
		pulse_mudstat = PULSE_IN_SECOND * 60;
		save_mudstat( );
	}

	if ( --pulse_second <= 0 )
	{
		pulse_second = PULSE_IN_SECOND;

#ifdef T2_ARENA
		update_arena( );
#endif
		update_affect( );
		update_tele( );
		char_check( );
		update_counter_quit( );
		reboot_check( 0 );
	}

	if ( --pulse_ani  <= 0 )
	{
		pulse_ani = PULSE_IN_SECOND / 4;

		update_animation( );
	}

	update_lyric( );
	update_timer( );
	update_aggr( );
	update_obj_act( );
	update_room_act( );

	clean_obj_queue( );		/* dispose of extracted objects		   */
	clean_char_queue( );	/* dispose of dead mobs/quitting chars */

	/* (FF) fare una funziona apposta e spostare tutto anche subtract_timer in intepret()? */
	if ( timechar )
	{
		gettimeofday( &etime, NULL );
		set_char_color( AT_PLAIN, timechar );
		send_to_char( timechar, "Update timing complete.\r\n" );
		subtract_times( &etime, &start_time );
		ch_printf( timechar, "Timing took %ld.%6ld seconds.\r\n", etime.tv_sec, etime.tv_usec );
		timechar = NULL;
	}

	tail_chain( );
}


/*
 * Comando admin che serve a forzare il richiamo di alcune funzioni dell'update_handler
 * (FF) aggiungere altre tipologie di update
 * (FF) fare una lista tipo comando puny per gestire le opzioni con la trust
 */
void do_update( CHAR_DATA *ch, char *argument )
{
	if ( !str_prefix(argument, "share") )
	{
		send_to_char( ch, "Update di azioni.\r\n" );
		update_share( );
	}
	else if ( !str_prefix(argument, "weather") )
	{
		send_to_char( ch, "Update del meteo.\r\n" );
		update_weather( );
	}
	else
	{
		send_to_char( ch, "Funzioni di update possibili da aggiornare:\r\n" );
		send_to_char( ch, "share: Update del valore delle azioni.\r\n" );
		send_to_char( ch, "weather: Update del meteo.\r\n" );

		return;
	}
}
