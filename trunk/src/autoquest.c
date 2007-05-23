/*
0 'GLORY RATES~
All prices are for adding affects/stats to an item or weapon, excepting the
"Sex Change" which is a permanent altering of one's player file.
&W
&W+1 Manapoint            &G5 Glory | &WWeight to 1            &G125 Glory
&W+1 Hitpoint            &G10 Glory | &WGlow/Hum Removal         &G5 Glory
&W+1 Stat               &G100 Glory | &WRename/Rekey            &G20 Glory
&W+1 Hitroll            &G125 Glory | &W1 Practice              &G50 Glory
&W+1 Damroll            &G175 Glory | &WSex Change              &G10 Glory
&W-1 Save               &G175 Glory | &WMinor Affect           &G300 Glory
&W-1 AC (lowers ac)      &G50 Glory | &WResistance             &G750 Glory
&W+1 AC (base obj AC)   &G130 Glory | &WAff_Protect            &G500 Glory
&WObject edit(per line)   &G5 Glory | &W
*/

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
#include "admin.h"
#include "archetype.h"
#include "db.h"
#include "nanny.h"
#include "race.h"
#include "room.h"


/*
 * Calcola il costo della gloria
 * Maggiore è la migrazione e minore sarà il costo dei punti gloria
 * Maggiore è il livello e maggiore sarà il costo
 */
int get_oneirism_cost( CHAR_DATA *ch, int glory )
{
	int migration;

	if ( !ch )
	{
		send_log( NULL, LOG_AUTOQUEST, "get_oneirism_cost: ch è NULL" );
		return glory;
	}

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_AUTOQUEST, "get_oneirism_cost: ch è il mob #%d",
			ch->pIndexData->vnum );
		return glory;
	}

	if ( glory < 0 || glory > 100000 )
	{
		send_log( NULL, LOG_AUTOQUEST, "get_oneirism_cost: costo in gloria di base del onirismo errato: %d", glory );
		return glory;
	}

	migration = get_curr_attr( ch, ATTR_MIG );

	/* Se ha la migrazione bassa aumenta il costo, viceversa se la ha alta la diminuisce */
	if ( migration < 30 )
	{
		
		glory += ( glory * (30-migration) ) / 100;	/* aumenta il costo in percentuale */
	}
	else if ( migration > 70 )
	{
		glory -= ( glory * (migration-70) ) / 100;	/* diminuisce il costo in percentuale */
	}

	/* Più il livello del pg è alto più ne aumenta il costo */
	glory += glory * ( (get_level(ch)/10) / 100 );

	send_log( NULL, LOG_AUTOQUEST, "get_oneirism_cost: costo onirismo per %s: %d", ch->name, glory );
	return glory;
}


/*
 * Delirio onirico astrale che permette di spendere i punti di gloria
 *	distribuendoli dove si vuole
 * (FF) sarebbero da rivedere le stringhe di fallimento gloria e consumo gloria di tutte
 *	le tipologie e farle più fantasiose, scenico-teatrali, e con le act
 * (FF) Fare l'arma
 */
DO_RET do_oneirism( CHAR_DATA *ch, char *argument )
{
	char	arg1[MIL];
	char	arg2[MIL];
	int		num;
	int		cost;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I Mob non possono andare in delirio onirico astrale.\r\n" );
		return;
	}

	if ( !ch->pg )
	{
		send_log( NULL, LOG_BUG, "do_oneirism: struttura pg NULL per il giocatore %s", ch->name );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Posso entrare in delirio onirico astrale per:\r\n" );

		send_to_char( ch, "&gattributo&w <nome attributo> +/-\r\n" );
		send_to_char( ch, "&gsenso&w <nome senso> +/-\r\n" );
		send_to_char( ch, "&gpunteggio&w <nome punteggio> +/-\r\n" );
#ifdef T2_ALFA
		/* Visto che ci pensano le practice a fare questo non mi pare il caso di aggiungerle */
		send_to_char( ch, "&gabilità&w <nome>\r\n" );
		send_to_char( ch, "&gincantesimo&w <nome>\r\n" );
		send_to_char( ch, "&glingua&w <nome>\r\n" );
#endif
		send_to_char( ch, "&galtezza&w +/-\r\n" );
		send_to_char( ch, "&gpeso&w +/-\r\n" );
		if ( ch->race != RACE_LIZARDMAN )
		{
			send_to_char( ch, "&gcapelli&w lunghezza +/-\r\n" );
			send_to_char( ch, "&gcapelli&w colore <nome colore>\r\n" );
			send_to_char( ch, "&gcapelli&w tipo <nome tipo>\r\n" );
		}
		send_to_char( ch, "&gocchi&w <nome colore>\r\n" );
#ifdef T2_ALFA
		send_to_char( ch, "&gpelle&w <nome colore>\r\n" );
		if ( ch->race == RACE_LIZARDMAN )
			send_to_char( ch, "&gpelle2&w <nome colore>\r\n" );
		send_to_char( ch, "&gallineamento&w +/-\r\n" );
		send_to_char( ch, "&ganni&w +/-\r\n" );
		send_to_char( ch, "&garma&w\r\n" );
#endif
		send_to_char( ch, "&garchetipo&w <nome archetipo>\r\n" );
		send_to_char( ch, "&gmano&w\r\n" );
		send_to_char( ch, "&gvestizione&w\r\n" );
		send_to_char( ch, "&gspoglie&w\r\n" );

		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !str_prefix(arg1, "attributo") )
	{
		num = code_num( NULL, arg2, CODE_ATTR );
		if ( !VALID_STR(arg2) || num == -1 )
		{
			send_to_char( ch, "Quale attributo dovrei modificare?\r\n" );
			code_all( CODE_ATTR, FALSE );
			return;
		}

		if ( argument[0] != '+' && argument[0] != '-' )
		{
			send_to_char( ch, "Devo aumentare l'attributo o diminuirlo?.\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 40 );
		if ( ch->pg->glory < cost )
		{
			ch_printf( ch, "Cerco di andare in trance, ma la mia gloria è troppo bassa per poter aumentare l'attributo %s.\r\n", code_name(NULL, num, CODE_ATTR) );	/* (GR) */
			return;
		}

		if ( argument[0] == '+' )
		{
			if ( ch->attr_perm[num] >= MAX_LEARN_ATTR )
			{
				ch_printf( ch, "Cerco di aumentare l'attributo %s, ma ho già raggiunto il mio limite.\r\n", code_name(NULL, num, CODE_ATTR) );	/* (GR) */
				return;
			}

			if ( check_total_attr(ch) <= 0 )
			{
				ch_printf( ch, "Cerco di aumentare l'attributo %s, ma più di così non posso migliorarmi negli attributi.\r\n", code_name(NULL, num, CODE_ATTR) );	/* (GR) */
				return;
			}
		}

		if ( argument[0] == '-' && ch->attr_perm[num] < 15 )
		{
			ch_printf( ch, "Cerco di diminuire l'attributo %s, ma ho raggiunto il mio minimo.\r\n", code_name(NULL, num, CODE_ATTR) );	/* (GR) */
			return;
		}

		ch_printf( ch, "Entro in trance di delirio onirico astrale e riesco a %s la %s!\r\n",
			(argument[0] == '+') ? "aumentare" : "diminuire",
			code_name(NULL, num, CODE_ATTR) );	/* (GR) */
		ch->pg->glory -= cost;
		if ( argument[0] == '+' )
			ch->attr_perm[num]++;
		else
			ch->attr_perm[num]--;
		return;
	}

	if ( !str_prefix(arg1, "senso") )
	{
		num = code_num( NULL, arg2, CODE_SENSE );
		if ( !VALID_STR(arg2) || num == -1 )
		{
			ch_printf( ch, "Quale senso dovrei modificare?\r\n" );
			code_all( CODE_SENSE, FALSE );
			return;
		}

		if ( argument[0] != '+' && argument[0] != '-' )
		{
			send_to_char( ch, "Devo aumentare il senso o diminuirlo?.\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 30 );
		if ( ch->pg->glory < cost )
		{
			ch_printf( ch, "Cerco di andare in trance, ma la mia gloria è troppo bassa per poter aumentare il senso %s.\r\n", code_name(NULL, num, CODE_SENSE) );	/* (GR) */
			return;
		}

		if ( argument[0] == '+' )
		{
			if ( ch->sense_perm[num] >= MAX_LEARN_SKELL )
			{
				ch_printf( ch, "Cerco di aumentare il senso %s, ma ho già raggiunto il mio limite.\r\n", code_name(NULL, num, CODE_SENSE) );	/* (GR) */
				return;
			}

			if ( check_total_sense(ch) <= 0 )
			{
				ch_printf( ch, "Cerco di aumentare il senso %s, ma più di così non posso migliorarmi nei sensi.\r\n", code_name(NULL, num, CODE_ATTR) );	/* (GR) */
				return;
			}
		}

		if ( argument[0] == '-' && ch->sense_perm[num] < 15 )
		{
			ch_printf( ch, "Cerco di diminuire il senso %s, ma ho raggiunto il mio minimo.\r\n", code_name(NULL, num, CODE_ATTR) );	/* (GR) */
			return;
		}

		ch_printf( ch, "Entro in trance di delirio onirico astrale e riesco a %s il senso %s!\r\n",
			(argument[0] == '+') ? "aumentare" : "diminuire",
			code_name(NULL, num, CODE_SENSE) );	/* (GR) */
		ch->pg->glory -= cost;
		if ( argument[0] == '+' )
			ch->sense_perm[num]++;
		else
			ch->sense_perm[num]--;
		return;
	}

	if ( !str_prefix(arg1, "punteggio") )
	{
		num = code_num( NULL, arg2, CODE_POINTS );
		if ( !VALID_STR(arg2) || num == -1 )
		{
			ch_printf( ch, "Quale punteggio dovrei modificare?.\r\n" );
			code_all( CODE_POINTS, FALSE );
			return;
		}

		if ( argument[0] != '+' && argument[0] != '-' )
		{
			send_to_char( ch, "Devo aumentare il punteggio o diminuirlo?.\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 60 );
		if ( ch->pg->glory < cost )
		{
			ch_printf( ch, "Cerco di andare in trance, ma la mia gloria è troppo bassa per poter aumentare il punteggio %s.\r\n", code_name(NULL, num, CODE_POINTS) );
			return;
		}

		if ( argument[0] == '+' )
		{
			/* (FF) Per ora tutte le razze lo hanno a 5000, sarà da caratterizzare */
			if ( ch->points_max[num] >= table_race[ch->race]->points_limit[num]
			  || check_total_points(ch) <= 0 )
			{
				ch_printf( ch, "Non posso aumentare più di così il punteggio %s, sono al limite.\r\n", code_name(NULL, num, CODE_POINTS) );		/* (GR) */
				return;
			}
		}

		if ( argument[0] == '-' && ch->points_max[num] < table_points[num].start )
		{
			ch_printf( ch, "Cerco di diminuire il punteggio %s, ma ho raggiunto il mio minimo.\r\n", code_name(NULL, num, CODE_ATTR) );	/* (GR) */
			return;
		}

		ch_printf( ch, "Entro in trance di delirio onirico astrale e riesco a %s il punteggio %s!\r\n",
			(argument[0] == '+') ? "aumentare" : "diminuire",
			code_name(NULL, num, CODE_POINTS) );	/* (GR) */
		ch->pg->glory -= cost;
		if ( argument[0] == '+' )
			ch->points_max[num] += 5;
		else
			ch->points_max[num] -= 5;
		return;
	}

#ifdef T2_ALFA
	/* Non lo attivo perché già ci pensa il practice a farlo, mi sembra inutile */
	if ( !is_name_prefix(arg1, "abilità incantesimo") )
	{
		char	type[MIL];

		strcpy( type, (arg1[0] == 'a') ? "a abilità" : "o incantesimo" );
		num = skill_lookup( arg2 );

		if ( !VALID_STR(arg2) || !is_valid_sn(num) )
		{
			ch_printf( ch, "Temo che quest%s sia inesistente.\r\n", stype );
			return;
		}

		if ( get_level(ch) < table_skill[num]->skill_level[ch->class] )
		{
			ch_printf( ch, "Quest%s non la posso ancora imparare.\r\n", stype );
			return;
		}

		if ( ch->pg->learned_skell[num] <= 0 )
		{
			ch_printf( ch, "Non posso imparare quest%s, devo prima impararlo almeno un po'.\r\n", stype );
			return;
		}

		if ( ch->pg->learned_skell[num] >= MAX_LEARN_SKELL )
		{
			ch_printf( ch, "Conosco già a sufficenza quest%s, non potrei assorbire di più.\r\n", stype );
			return;
		}

		if ( SPELL_FLAG(table_skill[num], SKELL_SECRETSKILL) )
		{
			ch_printf( ch, "Mi è impossibile imparare quest%s!\r\n", stype );
			return;
		}

		cost = get_oneirism_cost( ch, 50 );
		if ( ch->pg->glory < cost )
		{
			ch_printf( ch, "Non ho abbastanza gloria per poter apprendere maggiormente riguardo a %s\r\n", table_skill[num]->name );
			return;
		}

		ch_printf( ch, "Entro in trance di delirio onirico astrale e riesco ad aumentare %s\r\n", table_skill[num]->name );
		ch->pg->glory -= cost;
		ch->pg->learned_skell[num] += 5;
		URANGE( 0, ch->pg->learned_skell[num], MAX_LEARN_SKELL );
		return;
	}

	if ( !str_prefix(arg1, "lingua") )
	{

		return;
	}
#endif

	if ( !str_prefix(arg1, "altezza") )
	{
		if ( arg2[0] != '+' && arg2[0] != '-' )
		{
			send_to_char( ch, "Devo aumentare la mia altezza o diminuirla?.\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 5 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter modificare la mia altezza.\r\n" );
			return;
		}

		/* Valori massimi e minimi uguali a quelli della nanny */
		if ( arg2[0] == '+' && ch->height >= table_race[ch->race]->height * (1.00 + MAX_FACTOR_HEIGHT) )
		{
			send_to_char( ch, "Non posso aumentare la mia altezza più di così.\r\n" );
			return;
		}

		if ( arg2[0] == '-' && ch->height <= table_race[ch->race]->height * (1.00 - MAX_FACTOR_WEIGHT) )
		{
			send_to_char( ch, "Non posso diminuire la mia altezza più di così.\r\n" );
			return;
		}

		ch_printf( ch, "Entro in trance di delirio onirico astrale e %s la mia altezza.\r\n",
			(arg2[0] == '+') ? "aumento" : "diminuisco" );
		ch->pg->glory -= cost;
		if ( arg2[0] == '+' )
			ch->height++;
		else
			ch->height--;
		return;
	}

	if ( !str_prefix(arg1, "peso") )
	{
		if ( arg2[0] != '+' && arg2[0] != '-' )
		{
			send_to_char( ch, "Devo aumentare il mio peso o diminuirlo?\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 15 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter modificare il mio peso.\r\n" );
			return;
		}

		/* Valori massimi e minimi uguali a quelli della nanny */
		if ( arg2[0] == '+' && ch->weight >= table_race[ch->race]->weight * (1.00 + MAX_FACTOR_WEIGHT) )
		{
			send_to_char( ch, "Non posso aumentare il mio peso più di così.\r\n" );
			return;
		}

		if ( arg2[0] == '-' && ch->weight <= table_race[ch->race]->weight * (1.00 - MAX_FACTOR_WEIGHT) )
		{
			send_to_char( ch, "Non posso diminuire il mio peso più di così.\r\n" );
			return;
		}

		ch_printf( ch, "Entro in trance di delirio onirico astrale e %s il mio peso.\r\n",
			(arg2[0] == '+') ? "aumento" : "diminuisco" );
		ch->pg->glory -= cost;
		if ( arg2[0] == '+' )
			ch->weight += 1000;
		else
			ch->weight -= 1000;
		return;
	}

	if ( !str_prefix(arg1, "capelli") )
	{
		if ( ch->race == RACE_LIZARDMAN )
		{
			send_to_char( ch, "Inutile, sulla mia pelle di rettile non crescono i capelli.\r\n" );
			return;
		}

		if ( !ch->pg->hair )
		{
			send_log( NULL, LOG_BUG, "do_oneirism: struttura capelli NULL per il pg %s", ch->name );
			return;
		}

		if ( !str_prefix(arg2, "lunghezza") )
		{
			if ( argument[0] != '+' && argument[0] != '-' )
			{
				send_to_char( ch, "Devo aumentare la lunghezza dei miei capelli o farli più corti?\r\n" );
				return;
			}

			cost = get_oneirism_cost( ch, 5 );
			if ( ch->pg->glory < cost )
			{
				send_to_char( ch, "Non ho abbastanza gloria per poter modificare la lunghezza dei miei capelli.\r\n" );
				return;
			}

			ch_printf( ch, "Entro in trance di delirio onirico astrale e %s la lunghezza dei miei capelli.\r\n",
				(argument[0] == '+') ? "aumento" : "diminuisco" );
			ch->pg->glory -= cost;
			if ( argument[0] == '+' )
				ch->pg->hair->length++;
			else
				ch->pg->hair->length--;
			return;
		}

		if ( !str_prefix(arg2, "colore") )
		{
			num = code_num( NULL, argument, CODE_COLORHAIR );
			if ( !VALID_STR(argument) || num == -1 )
			{
				ch_printf( ch, "Con quale colore?\r\n" );
				code_all( CODE_COLORHAIR, FALSE );
				return;
			}

			cost = get_oneirism_cost( ch, 10 );
			if ( ch->pg->glory < cost )
			{
				send_to_char( ch, "Non ho abbastanza gloria per poter modificare il colore dei miei capelli.\r\n" );
				return;
			}

			send_to_char( ch, "Entro in trance di delirio onirico astrale cambiandomi il colore dei capelli.\r\n" );
			ch->pg->glory -= cost;

			ch->pg->hair->color->cat = num;
			DISPOSE( ch->pg->hair->color->syn );
			ch->pg->hair->color->syn = str_dup( code_name(NULL, num, CODE_COLORHAIR) );

			return;
		}

		if ( !str_prefix(arg2, "tipo") )
		{
			num = code_num( NULL, argument, CODE_HAIRTYPE );
			if ( !VALID_STR(argument) || num == -1 )
			{
				ch_printf( ch, "Con quale tipo di capelli?\r\n" );
				code_all( CODE_HAIRTYPE, FALSE );
				return;
			}

			cost = get_oneirism_cost( ch, 10 );
			if ( ch->pg->glory < cost )
			{
				send_to_char( ch, "Non ho abbastanza gloria per poter modificare il tipo dei miei capelli.\r\n" );
				return;
			}

			send_to_char( ch, "Entro in trance di delirio onirico astrale cambiandomi il tipo di capelli.\r\n" );
			ch->pg->glory -= cost;

			ch->pg->hair->type->cat = num;
			DISPOSE( ch->pg->hair->type->syn );
			ch->pg->hair->type->syn = str_dup( code_name(NULL, num, CODE_HAIRTYPE) );

			return;
		}

		send_to_char( ch, "Che cosa dovrei modificare dei miei capelli?\r\n" );
		return;
	}

	if ( !str_prefix(arg1, "occhi") )
	{
		strcat( arg2, argument );	/* Cosìcché anche gli argomenti tipo "occhi blu" funzionano */

		num = code_num( NULL, arg2, CODE_COLOR );
		if ( !VALID_STR(arg2) || num == -1 )
		{
			ch_printf( ch, "Con quale colore?\r\n" );
			code_all( CODE_COLOR, FALSE );
			return;
		}

		cost = get_oneirism_cost( ch, 10 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter modificare il colore dei miei occhi.\r\n" );
			return;
		}

		send_to_char( ch, "Entro in trance di delirio onirico astrale cambiandomi il colore degli occhi.\r\n" );
		ch->pg->glory -= cost;

		ch->pg->eye->color->cat = num;
		DISPOSE( ch->pg->eye->color->syn );
		ch->pg->eye->color->syn = str_dup( code_name(NULL, num, CODE_COLOR) );

		return;
	}

#ifdef T2_ALFA
	if ( !str_prefix(arg1, "pelle") )
	{
		num = code_num( NULL, arg2, CODE_COLOR );
		if ( !VALID_STR(arg2) || num == -1 )
		{
			ch_printf( ch, "Con quale colore?\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 10 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter modificare il colore della mia pelle.\r\n" );
			return;
		}

		send_to_char( ch, "Entro in trance di delirio onirico astrale cambiandomi il colore della pelle.\r\n" );
		ch->pg->glory -= cost;
		ch->pg->skin->color = num;
		return;
	}

	if ( !str_prefix(arg1, "pelle2") )
	{
		num = code_num( NULL, arg2, CODE_COLOR );
		if ( !VALID_STR(arg2) || num == -1 )
		{
			ch_printf( ch, "Con quale colore?\r\n" );
			code_all( CODE_COLOR, FALSE );
			return;
		}

		if ( ch->race != RACE_THEPA )
		{
			send_to_char( ch, "Non ho la pelle maculata come un rettile.\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 10 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter modificare il colore della mia pelle.\r\n" );
			return;
		}

		send_to_char( ch, "Entro in trance di delirio onirico astrale cambiandomi il colore della pelle.\r\n" );
		ch->pg->glory -= cost;
		ch->pg->skin2->color = num;
		return;
	}
#endif

	if ( !str_prefix(arg1, "archetipo") )
	{
		num = get_archetype_num( arg2 );
		if ( !VALID_STR(arg2) || num == -1 )
		{
			send_to_char( ch, "Non credo che sia un archetipo con cui possa trovarmi a mio agio.\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 40 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter ritornare in un luogo sicuro.\r\n" );
			return;
		}

		send_to_char( ch, "Entro in trance di delirio onirico astrale per cambiare il mio archetipo.\r\n" );
		ch->pg->glory -= cost;
		ch->pg->archetype = num;
		return;
	}

	if ( !str_prefix(arg1, "mano") )
	{
		cost = get_oneirism_cost( ch, 20 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter cambiare il tipo di mano principale.\r\n" );
			return;
		}

		send_to_char( ch, "Entro in trance di delirio onirico astrale per cambiare il tipo di mano con cui utilizzo le cose.\r\n" );
		ch->pg->glory -= cost;
		ch->right_hand = !ch->right_hand;
		return;
	}

	if ( !str_prefix(arg1, "vestizione") )
	{
		if ( get_level(ch) > LVL_NEWBIE )
		{
			send_to_char( ch, "Non posso rivestirmi dell'equipaggiamento di base ormai non mi servirebbe a molto.\r\n" );
			return;
		}

		cost = get_oneirism_cost( ch, 20 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per portare a termine un'altra vestizione.\r\n" );
			return;
		}

		ch->pg->glory -= cost;
		equip_newbieset( ch );
		return;
	}

	if ( !str_prefix(arg1, "spoglie") )
	{
		OBJ_PROTO_DATA	*pObj;
		OBJ_DATA		*obj;
		char			 buf[MSL];
		bool			 found = FALSE;

		cost = get_oneirism_cost( ch, 30 );
		if ( ch->pg->glory < cost )
		{
			send_to_char( ch, "Non ho abbastanza gloria per poter percepire ove si trovano le mie spoglie.\r\n" );
			return;
		}

		ch->pg->glory -= cost;

		/* Recupera la short di un cadavere di pg per usarla nella ricerca */
		pObj = get_obj_proto( NULL, VNUM_OBJ_CORPSE_PC );
		sprintf( buf, pObj->short_descr, ch->name );
		
		send_to_char( ch, "Una visione rivela ove si trovano le mie spoglie..\r\n" );
		for ( obj = first_object;  obj;  obj = obj->next )
		{
			if ( !obj->in_room )								continue;
			if ( !obj->pObjProto->vnum == VNUM_OBJ_CORPSE_PC )	continue;
		
			if ( !str_cmp(buf, obj->short_descr) )
			{
				found = TRUE;
				ch_printf( ch, "Si trova a %s e durerà ancora per %d minuti circa.\r\n",
					obj->in_room->name, obj->timer );
			}
		}
		
		if ( !found )
			send_to_pager( ch, "Non percepisco la presenza materiale di nessuna mia spoglia.\r\n" );

		return;
	}
}
