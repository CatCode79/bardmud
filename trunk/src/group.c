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
 >							Modulo Gestione Gruppo							<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "command.h"
#include "db.h"
#include "group.h"
#include "handler_ch.h"
#include "interpret.h"
#include "msp.h"
#include "room.h"


/*
 * Comando gruppo; Overhauled.
 * (RR) da rifare meno informativa e più rpg, come se uno desse una occhiata attorno
 *	per vedere se vi siano qualche gruppo in giro, questo nel caso che non faccia parte
 *	di nessun gruppo.
 */
DO_RET do_group( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	/* Impedisce ai mob di fare leader dei gruppi */
	if ( IS_MOB(ch) )
		return;

#ifdef T2_ALFA
	if ( count_group(ch) == 0 )
	{
		send_to_char( ch, "Non fai parte di nessun gruppo.\r\n" );
		return;
	}
#endif

	if ( !VALID_STR(argument) )
	{
		CHAR_DATA *gch;
		CHAR_DATA *leader;

		leader = ch->leader  ?  ch->leader  :  ch;
		set_char_color( AT_DGREEN, ch );
		/* (FF) Cambiarle dando queste info: razza, stato(vita in percentuale), arma, style */
		ch_printf( ch, "\r\nGruppo di %-12s  Razza          Stile      Mente  Vita  Movi  Mana%s\r\n",
			PERS(leader, ch),
			IS_ADMIN(ch) ? "  Anima" : "" );

		for ( gch = first_char;  gch;  gch = gch->next )
		{
			int		x;

			if ( !is_same_group(gch, ch) )
				continue;

			set_char_color( AT_DGREEN, ch );
			if ( HAS_BIT(gch->affected_by, AFFECT_POSSESS) )	/* reveal no information */
			{
				ch_printf( ch, "%-12s            %-14s %-10s %-6s %-5s %-5s %-5s%s\r\n",
					capitalize(PERS(gch, ch)),
					"?????",
					"?????",
					"?????",
					"?????",
					"?????",
					"?????",
					IS_ADMIN(ch) ? "  ?????" : "" );
			}

			set_char_color( AT_GREEN, ch );
			/* Nome, Colore+Razza, Stile */
			ch_printf( ch, "%-12s            %-14s&w %-10s ",
				PERS(gch, ch),
				get_race(gch, TRUE),
				get_style_name(gch) );

			/* Mente */
			ch_printf( ch, "%s   ",
				gch->mental_state >  80  ?  "&G++++"	:
				gch->mental_state >  60  ?  "&w=&G+++"	:
				gch->mental_state >  40  ?  "&w==&G++"	:
				gch->mental_state >	 20	 ?	"&w===&G+"	:
				gch->mental_state > -20  ?  "&w===="	:
				gch->mental_state > -40  ?  "&R-&w==="	:
				gch->mental_state > -60  ?  "&R--&w=="	:
				gch->mental_state > -80  ?  "&R---&w="	:
										    "&R----" );

			for ( x = 0;  x < MAX_POINTS;  x++ )
			{
				if ( x != POINTS_SOUL || (x == POINTS_SOUL && IS_ADMIN(ch)) )
				{
					/* Punti -- bisogna abbondare nell'allineamento perché conta
					 *	anche i colori, per i player 2 in più perché conta la colorazione
					 *	della percentuale &w% */
					if ( IS_ADMIN(ch) )
						ch_printf( ch, "%-7s ", get_points_colored(ch, gch, x, FALSE) );
					else
						ch_printf( ch, "%-9s ", get_points_colored(ch, gch, x, FALSE) );
				}
			}

			send_to_char( ch, "\r\n" );
		} /* chiude il for */
		return;
	} /* chiude l'if */

	if ( !str_cmp(argument, "sciogliere") || !str_cmp(argument, "disband") )
	{
		CHAR_DATA *gch;
		int		   count = 0;

		if ( ch->leader || ch->master )
		{
			send_to_char( ch, "Non posso sciogliere un gruppo se sto seguendo qualcuno.\r\n" );
			return;
		}

		for ( gch = first_char;  gch;  gch = gch->next )
		{
			if ( is_same_group(ch, gch) && (ch != gch) )
			{
				gch->leader = NULL;
				gch->master = NULL;
				count++;
				send_to_char( gch, "Il mio gruppo si è sciolto.\r\n" );
			}
		}

		if ( count == 0 )
			send_to_char( ch, "Non ho nessun membro nel gruppo da sciogliere.\r\n" );
		else
			send_to_char( ch, "Il mio gruppo si è sciolto.\r\n" );

		return;
	} /* chiude l'if */

	if ( !str_cmp_all(argument, FALSE) )
	{
		CHAR_DATA	*rch;

		int count = 0;
		for ( rch = ch->in_room->first_person;  rch;  rch = rch->next_in_room )
		{
			if ( ch == rch )				continue;
			if ( IS_MOB(rch) )				continue;
			if ( !can_see(ch, rch) )		continue;
			if ( rch->master != ch )		continue;
			if ( ch->master )				continue;
			if ( ch->leader )				continue;
			if ( is_same_group(rch, ch) )	continue;

			if ( abs(get_level(ch) - get_level(rch)) <= sysdata.lvl_group_diff )
			{
				rch->leader = ch;
				count++;
			}
		}

		if ( count == 0 )
		{
			send_to_char( ch, "Non sono un membro idoneo al gruppo.\r\n" );
		}
		else
		{
			act( AT_ACTION, "$n fa gruppo con i seguaci di $x.", ch, NULL, NULL, TO_ROOM );
			send_to_char( ch, "Organizzo un gruppo con i miei seguaci.\r\n" );
		}
		return;
	} /* chiude l'if */

	victim = get_char_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non lo trovo qui.\r\n" );
		return;
	}

	if ( ch->master || (ch->leader && ch->leader != ch) )
	{
		send_to_char( ch, "Sto seguendo qualcun'altro!\r\n" );
		return;
	}

	if ( victim->master != ch && ch != victim )
	{
		act( AT_PLAIN, "$N non mi sta seguendo.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( victim == ch )
	{
		act( AT_PLAIN, "Non posso fare gruppo con me stess$x.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( is_same_group(victim, ch) && ch != victim )
	{
		victim->leader = NULL;
		act( AT_ACTION, "$n rimuove $N dal suo gruppo.", ch, NULL, victim, TO_NOVICT );
		act( AT_ACTION, "$n mi rimuove dal suo gruppo.", ch, NULL, victim, TO_VICT	  );
		act( AT_ACTION, "Rimuovo $N dal mio gruppo.",	 ch, NULL, victim, TO_CHAR	  );
		return;
	}

	victim->leader = ch;
	act( AT_ACTION, "$N entra nel gruppo di $n.",	ch, NULL, victim, TO_NOVICT );
	act( AT_ACTION, "Entro nel gruppo di $n.",		ch, NULL, victim, TO_VICT	 );
	act( AT_ACTION, "$N entra nel mio gruppo.",		ch, NULL, victim, TO_CHAR	 );
}


/*
 * Ritorna quanti membri ci sono nel gruppo in giro per il mud.
 * Nel conteggio non è contato ch.
 */
int count_group( CHAR_DATA *ch )
{
	CHAR_DATA	*gch;
	int			 number = 0;

	for ( gch = first_char;  gch;  gch = gch->next )
	{
		if ( !is_same_group(ch, gch) )
			continue;
		if ( ch != gch )
			number++;
	}

	return number;
}


/*
 * Ritorna quanti vi sono del gruppo nella stanza in cui ch si trova.
 * Nel conteggio non è contato ch.
 */
int count_group_room( CHAR_DATA *ch )
{
	CHAR_DATA	*gch;
	int			 number = 0;

	for ( gch = ch->in_room->first_person;  gch;  gch = gch->next_in_room )
	{
		if ( !is_same_group(ch, gch) )
			continue;
		if ( ch != gch )
			number++;
	}

	return number;
}


/*
 * Comando Dividi
 */
DO_RET do_split( CHAR_DATA *ch, char *argument )
{
	char		buf[MSL];
	CHAR_DATA  *gch;
	int			members;
	int			amount;
	int			share;
	int			extra;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Che somma dovrei dividere?\r\n" );
		return;
	}

	amount = atoi( argument );
	if ( amount < 0 )
	{
		send_to_char( ch, "Come faccio a dividere una somma negativa??\r\n" );
		return;
	}

	if ( amount == 0 )
	{
		send_to_char( ch, "Come faccio a suddividere il nulla in più parti??\r\n" );
		return;
	}

	if ( ch->gold < amount )
	{
		send_to_char( ch, "Non ho così tante monete.\r\n" );
		return;
	}

	members = count_group_room( ch );
	members++;	/* aggiunge ch */

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_AUTOSPLIT) && members < 2 )
		return;

	if ( members < 2 )
	{
		send_to_char( ch, "Faccio prima a tenermele tutte.\r\n" );
		return;
	}

	share = amount / members;
	extra = amount % members;

	if ( share == 0 )
	{
		send_to_char( ch, "Sono proprio uno spilorcio..\r\n" );
		return;
	}

	ch->gold -= amount;
	ch->gold += share + extra;

	set_char_color( AT_GOLD, ch );
	ch_printf( ch, "Divido %d monete d'oro. La mia quota è di %d monete.\r\n", amount, share + extra );

	sprintf( buf, "$n divido %d monete d'oro. La mia quota è di %d monete.", amount, share );

	for ( gch = ch->in_room->first_person;  gch;  gch = gch->next_in_room )
	{
		if ( gch != ch && is_same_group(gch, ch) )
		{
			act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
#ifdef T2_MSP
			send_audio( gch->desc, "money.wav", TO_CHAR );
#endif
			gch->gold += share;
		}
	}
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
	if ( ach->leader )
		ach = ach->leader;

	if ( bch->leader )
		bch = bch->leader;

	return ach == bch;
}


/*
 * Utilizzato per evitare i loop da follow.
 * Capitano quando qualcuno segue in cerchio un'altro
 *	attraverso un'uscita che conduce nella stessa stanza.
 * (ve ne sono alcune in zone di aree-maze)
 */
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *tmp;

	for ( tmp = victim;  tmp;  tmp = tmp->master )
	{
		if ( tmp == ch )
			return TRUE;
	}

	return FALSE;
}


/*
 * Comando Congeda
 */
DO_RET do_dismiss( CHAR_DATA *ch, char *argument )
{
	char	   arg[MIL];
	CHAR_DATA *victim;

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Devo congedare chi?\r\n" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Quella persona non la vedo qui.\r\n" );
		return;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_CHARM)
	  && IS_MOB(victim)
	  && victim->master == ch )
	{
		stop_follower( victim );
		stop_hating  ( victim );
		stop_hunting ( victim );
		stop_fearing ( victim );
		act( AT_ACTION, "$n congeda $N.", ch, NULL, victim, TO_NOVICT );
		act( AT_ACTION, "Congedo $N.", ch, NULL, victim, TO_CHAR );
	}
	else
	{
		send_to_char( ch, "Non posso congedarlo.\r\n" );
	}
}


/*
 * Comando Segui
 */
DO_RET do_follow( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo seguire chi?\r\n" );
		return;
	}

	victim = get_char_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non lo vedo qui.\r\n" );
		return;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) && ch->master )
	{
		act( AT_PLAIN, "Seguo più volentieri $N!", ch, NULL, ch->master, TO_CHAR );
		return;
	}

	if ( victim == ch )
	{
		if ( !ch->master )
		{
			send_to_char( ch, "Il mio unico sentiero sono i miei passi.\r\n" );
			return;
		}
		stop_follower( ch );
		return;
	}

	if ( circle_follow(ch, victim) )
	{
		send_to_char( ch, "Seguirsi a vicenda non è una buona idea..\r\n" );
		return;
	}

	if ( ch->master )
		stop_follower( ch );

	add_follower( ch, victim );
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "add_follower: ch è NULL" );
		return;
	}

	if ( ch->master )
	{
		send_log( NULL, LOG_BUG, "add_follower: master non è NULL." );
		return;
	}

	ch->master		= master;
	ch->leader		= NULL;

	if ( IS_MOB(ch) && IS_PG(master) )
	{
		/* Supporto per salvare i mascotte */
		if ( HAS_BIT_ACT(ch, MOB_MASCOTTE) )
			master->pg->mascotte = ch;

		master->pg->charmies++;	/* mascotte e charmati, tutti assieme */
	}

	/* Se il master può vedere il suo seguitore e ha abbastanza fiducia */
	if ( IS_ADMIN(master) )
		ch_printf( master, "[ADM] %s ti segue.\r\n", ch->name );

	act( AT_ACTION, "Seguo $N.",  ch, NULL, master, TO_CHAR );
}


void stop_follower( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "stop_follower: ch è NULL." );
		return;
	}

	if ( !ch->master )
	{
		send_log( NULL, LOG_BUG, "stop_follower: master è NULL." );
		return;
	}

	if ( IS_MOB(ch) && IS_PG(ch->master) )
	{
		if ( ch->master->pg && ch->master->pg->mascotte		/* (bb) qui quando si raddoppia il pet può segfaultare */
		  && ch->master->pg->mascotte == ch )
		{
			ch->master->pg->mascotte = NULL;
		}
		ch->master->pg->charmies--;	/* mascotte e charmati, tutti assieme */
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		REMOVE_BIT( ch->affected_by, AFFECT_CHARM );
		affect_strip( ch, gsn_charm_person );
	}

	/* Se il master può vedere il suo seguitore ed ha abbastanza fiducia */
	if ( IS_ADMIN(ch->master) )
		ch_printf( ch->master, "[ADM] %s smette di seguirti.\r\n", ch->name );

	act( AT_ACTION, "Smetto di seguire $N.", ch, NULL, ch->master, TO_CHAR );

	ch->master = NULL;
	ch->leader = NULL;
}


void die_follower( CHAR_DATA *ch )
{
	CHAR_DATA *fch;

	if ( ch->master )
		stop_follower( ch );

	ch->leader = NULL;

	for ( fch = first_char;  fch;  fch = fch->next )
	{
		if ( fch->master == ch )
			stop_follower( fch );

		if ( fch->leader == ch )
			fch->leader = fch;
	}
}


/*
 * Comando Ordina.
 */
DO_RET do_order( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	char	   arg[MIL];
	bool	   found;
	bool	   fAll;

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo ordinare a chi che cosa?\r\n" );
		return;
	}

	/* Se il pg è affetto a sua volta da charme */
	if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, "Il mio animo è incline all'obbedienza non al comando, in questo momento.\r\n" );
		return;
	}

	if ( !str_cmp_all(arg, FALSE) )
	{
		fAll   = TRUE;
		victim = NULL;
	}
	else
	{
		fAll = FALSE;

		victim = get_char_room( ch, arg, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "Non lo vedo qui.\r\n" );
			return;
		}

		if ( victim == ch )
		{
			send_to_char( ch, "Non mi pare proprio il caso..\r\n" );
			return;
		}

		/* Se non si sta cavalcando la vitima
		 *	e se la vittima non è charmata oppure se non si è il loro maestro */
		if ( !(ch->mount && ch->mount == victim)
		  && (!HAS_BIT(victim->affected_by, AFFECT_CHARM) || victim->master != ch) )
		{
			send_to_char( ch, "La mia autorità non può nulla con la ferrea volontà di chi ho davanti!\r\n" );
			return;
		}
	}

	/* (TT) dovrebbe andare a buon fine cmq visto che argument è passato a interpret, però è da provare */
/*	if ( !str_prefix(argument, "mp") )
	{
		send_to_char( ch, "Non penso proprio che funzionerebbe.\r\n" );
		return;
	}
*/

	found = FALSE;
	for ( och = ch->in_room->first_person;  och;  och = och_next )
	{
		och_next = och->next_in_room;

		if ( !HAS_BIT(och->affected_by, AFFECT_CHARM) )	continue;
		if ( och->master != ch )						continue;

		if ( fAll || och == victim )
		{
			found = TRUE;

			/* Invia il comando secondo la lingua della vittima */
			if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
				send_command( och, argument, IT );
			else
				send_command( och, argument, EN );

			act( AT_ACTION, "$n mi ordina di '$t'.", ch, argument, och, TO_VICT );
		}
	}

	if ( found )
	{
		send_log( NULL, LOG_ORDER, "%s: ordina %s.", ch->name, argument );
		send_to_char( ch, "Ok.\r\n" );
		WAIT_STATE( ch, PULSE_IN_SECOND * 3 );
	}
	else
		send_to_char( ch, "Nessuno mi sta seguendo.\r\n" );
}
