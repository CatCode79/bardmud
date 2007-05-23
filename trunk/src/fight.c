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
 >						Modulo di combattimento & morte						<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "calendar.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "gramm.h"
#include "group.h"
#include "interpret.h"
#include "make_obj.h"
#include "movement.h"
#include "mprog.h"
#include "msp.h"
#include "object_interact.h"
#include "morph.h"
#include "room.h"
#include "save.h"
#include "timer.h"


/*
 * Variabili locali
 */
OBJ_DATA *used_weapon;		/* Used to figure out which weapon later */


/*
 * Tabella dei codici sulle tipologie di attacco
 */
const CODE_DATA code_attack[] =
{
	{ ATTACK_BITE,			"ATTACK_BITE",			"bite"			},
	{ ATTACK_CLAWS,			"ATTACK_CLAWS",			"claws"			},
	{ ATTACK_TAIL,			"ATTACK_TAIL",			"tail"			},
	{ ATTACK_STING,			"ATTACK_STING",			"sting"			},
	{ ATTACK_PUNCH,			"ATTACK_PUNCH",			"punch"			},
	{ ATTACK_KICK,			"ATTACK_KICK",			"kick"			},
	{ ATTACK_TRIP,			"ATTACK_TRIP",			"trip"			},
	{ ATTACK_BASH,			"ATTACK_BASH",			"bash"			},
	{ ATTACK_STUN,			"ATTACK_STUN",			"stun"			},
	{ ATTACK_GOUGE,			"ATTACK_GOUGE",			"gouge"			},
	{ ATTACK_BACKSTAB,		"ATTACK_BACKSTAB",		"backstab"		},
	{ ATTACK_FEED,			"ATTACK_FEED",			"feed"			},
	{ ATTACK_DRAIN,			"ATTACK_DRAIN",			"drain"			},
	{ ATTACK_FIREBREATH,	"ATTACK_FIREBREATH",	"firebreath"	},
	{ ATTACK_FROSTBREATH,	"ATTACK_FROSTBREATH",	"frostbreath"	},
	{ ATTACK_ACIDBREATH,	"ATTACK_ACIDBREATH",	"acidbreath"	},
	{ ATTACK_LIGHTNBREATH,	"ATTACK_LIGHTNBREATH",	"lightnbreath"	},
	{ ATTACK_GASBREATH,		"ATTACK_GASBREATH",		"gasbreath"		},
	{ ATTACK_POISON,		"ATTACK_POISON",		"poison"		},
	{ ATTACK_NASTYPOISON,	"ATTACK_NASTYPOISON",	"nastypoison"	},
	{ ATTACK_GAZE,			"ATTACK_GAZE",			"gaze"			},
	{ ATTACK_BLINDNESS,		"ATTACK_BLINDNESS",		"blindness"		},
	{ ATTACK_CAUSESERIOUS,	"ATTACK_CAUSESERIOUS",	"causeserious"	},
	{ ATTACK_EARTHQUAKE,	"ATTACK_EARTHQUAKE",	"earthquake"	},
	{ ATTACK_CAUSECRITICAL,	"ATTACK_CAUSECRITICAL",	"causecritical"},
	{ ATTACK_CURSE,			"ATTACK_CURSE",			"curse"			},
	{ ATTACK_FLAMESTRIKE,	"ATTACK_FLAMESTRIKE",	"flamestrike"	},
	{ ATTACK_HARM,			"ATTACK_HARM",			"harm"			},
	{ ATTACK_FIREBALL,		"ATTACK_FIREBALL",		"fireball"		},
	{ ATTACK_COLORSPRAY,	"ATTACK_COLORSPRAY",	"colorspray"	},
	{ ATTACK_WEAKEN,		"ATTACK_WEAKEN",		"weaken"		},
	{ ATTACK_SPIRALBLAST,	"ATTACK_SPIRALBLAST",	"spiralblast"	}
};
const int max_check_attack = sizeof(code_attack)/sizeof(CODE_DATA);


/*
 * Tabella codici sulle tipologie di difesa
 */
const CODE_DATA code_defense[] =
{
	{ DEFENSE_PARRY,		"DEFENSE_PARRY",		"parry"			},
	{ DEFENSE_DODGE,		"DEFENSE_DODGE",		"dodge"			},
	{ DEFENSE_HEAL,			"DEFENSE_HEAL",			"heal"			},
	{ DEFENSE_CURELIGHT,	"DEFENSE_CURELIGHT",	"curelight"		},
	{ DEFENSE_CURESERIOUS,	"DEFENSE_CURESERIOUS",	"cureserious"	},
	{ DEFENSE_CURECRITICAL,	"DEFENSE_CURECRITICAL",	"curecritical"	},
	{ DEFENSE_DISPELMAGIC,	"DEFENSE_DISPELMAGIC",	"dispelmagic"	},
	{ DEFENSE_DISPELEVIL,	"DEFENSE_DISPELEVIL",	"dispelevil"	},
	{ DEFENSE_SANCTUARY,	"DEFENSE_SANCTUARY",	"sanctuary"		},
	{ DEFENSE_FIRESHIELD,	"DEFENSE_FIRESHIELD",	"fireshield"	},
	{ DEFENSE_SHOCKSHIELD,	"DEFENSE_SHOCKSHIELD",	"shockshield"	},
	{ DEFENSE_SHIELD,		"DEFENSE_SHIELD",		"shield"		},
	{ DEFENSE_BLESS,		"DEFENSE_BLESS",		"bless"			},
	{ DEFENSE_STONESKIN,	"DEFENSE_STONESKIN",	"stoneskin"		},
	{ DEFENSE_TELEPORT,		"DEFENSE_TELEPORT",		"teleport"		},
	{ DEFENSE_MONSUM1,		"DEFENSE_MONSUM1",		"monsum1"		},
	{ DEFENSE_MONSUM2,		"DEFENSE_MONSUM2",		"monsum2"		},
	{ DEFENSE_MONSUM3,		"DEFENSE_MONSUM3",		"monsum3"		},
	{ DEFENSE_MONSUM4,		"DEFENSE_MONSUM4",		"monsum4"		},
	{ DEFENSE_DISARM,		"DEFENSE_DISARM",		"disarm"		},
	{ DEFENSE_ICESHIELD,	"DEFENSE_ICESHIELD",	"iceshield"		},
	{ DEFENSE_GRIP,			"DEFENSE_GRIP",			"grip"			},
	{ DEFENSE_TRUESIGHT,	"DEFENSE_TRUESIGHT",	"truesight"		},
	{ DEFENSE_ACIDMIST,		"DEFENSE_ACIDMIST",		"acidmist"		},
	{ DEFENSE_VENOMSHIELD,	"DEFENSE_VENOMSHIELD",	"venomshield"	}
};
const int max_check_defense = sizeof(code_defense)/sizeof(CODE_DATA);


/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	if ( !used_weapon )
		return FALSE;

	obj = get_eq_char( ch, WEARLOC_WIELD );
	if ( obj && used_weapon == obj && HAS_BIT(obj->extra_flags, OBJFLAG_POISONED) )
		return TRUE;

	obj = get_eq_char( ch, WEARLOC_DUAL );
	if ( obj && used_weapon == obj && HAS_BIT(obj->extra_flags, OBJFLAG_POISONED) )
		return TRUE;

	return FALSE;
}


/*
 * Hunting, hating and fearing code
 */
#if 0
/* (FF) mai utilizzata, commentata per ora, forse in futuro servirà */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( !ch->hunting || !victim || ch->hunting->who != victim )
		return FALSE;

    if ( VALID_STR(ch->hunting->name) && VALID_STR(victim->name)
      && str_cmp(ch->hunting->name, victim->name) )
    {
		return FALSE;
	}

	return TRUE;
}
#endif


bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( !ch->hating || !victim || ch->hating->who != victim )
		return FALSE;

    if ( VALID_STR(ch->hating->name) && VALID_STR(victim->name)
      && str_cmp(ch->hating->name, victim->name) )
    {
		return FALSE;
	}

	return TRUE;
}


bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( !ch->fearing || !victim || ch->fearing->who != victim )
		return FALSE;

    if ( VALID_STR(ch->fearing->name) && VALID_STR(victim->name)
      && str_cmp(ch->fearing->name, victim->name) )
    {
		return FALSE;
	}

	return TRUE;
}


void stop_hunting( CHAR_DATA *ch )
{
	if ( ch->hunting )
	{
		DISPOSE( ch->hunting->name );
		DISPOSE( ch->hunting );
		ch->hunting = NULL;
	}
}


void stop_hating( CHAR_DATA *ch )
{
	if ( ch->hating )
	{
		DISPOSE( ch->hating->name );
		DISPOSE( ch->hating );
		ch->hating = NULL;
	}
}


void stop_fearing( CHAR_DATA *ch )
{
	if ( ch->fearing )
	{
		DISPOSE( ch->fearing->name );
		DISPOSE( ch->fearing );
		ch->fearing = NULL;
	}
}


void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( ch->hunting )
		stop_hunting( ch );

	CREATE( ch->hunting, HHF_DATA, 1 );
	ch->hunting->name = str_dup( victim->name );
	ch->hunting->who  = victim;
}


void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( ch->hating )
		stop_hating( ch );

	CREATE( ch->hating, HHF_DATA, 1 );
	ch->hating->name = str_dup( victim->name );
	ch->hating->who  = victim;
}


void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( ch->fearing )
		stop_fearing( ch );

	CREATE( ch->fearing, HHF_DATA, 1 );
	ch->fearing->name = str_dup( victim->name );
	ch->fearing->who  = victim;
}


/*
 * Calcola un fattore di massa virtuale di un PG.
 * Si basa sulla sua altezza ed il suo peso (FF) e la sua taglia.
 * Il fattore massa serve per calcolare quanta gente può combattere contro di una persona.
 * Tendenzialmente tutti i calcoli sono stati tarati per ottenere un numero basso
 *	e non realistico, questo per evitare combattimenti di massa, troppo pesanti per tutto
 *	(giocatore e server).
 */
int compute_mass( CHAR_DATA *ch )
{
	int mass;		/* numero ritornato che rappresenta la massa virtuale */
 	int modifier;	/* modificatore di calcolo della forma */

	modifier = 10;	/* (RR) temporanea finché la FORM non è stata definita */
	if ( ch->weight > ch->height )	/* (??) ma che cavolo intendevo fare qui? O_o  devo aver sbagliato */
		mass = ( (ch->weight /*/ 1000*/) / ch->height ) * modifier;
	else
		mass = modifier;

	return mass;
}


/*
 * Ritorna vero se l'attaccante trova spazio per combattere contro la vittima.
 * (FF) Probabile che gli debba anche passare come argomento in futuro il tipo di attacco, per variare tra duelli corpo a corpo e con armi da lancio o simili.
 */
bool space_to_fight( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int			mass_ch;		/* massa d'offessa dell'attaccante */
	int			mass_victim;	/* massa offendibile della vittima */
	CHAR_DATA  *rch,
			   *rch_next;		/* servono per scorrere nella lista dei pg nella stanza */

	if ( victim->num_fighting == 0 )
	{
		return TRUE;
	}
	else if ( victim->num_fighting < 0 )	/* test scaramantico */
	{
		send_log( NULL, LOG_BUG, "max_fight: num_figthing è minore di zero: %d", victim->num_fighting );
		victim->num_fighting = 0;
		return TRUE;
	}

	/* Calcola la massa d'offesa dell'attaccante.
	 * Viene diviso per 4, un numero basso che indicherebbe le 4 direzioni
	 *	principali, è basso, vero, ma così risulta un valore grosso nella divisione
	 *	da utilizzare per calcoli sucessivi ed avere così pochi "posti di combattimento". */
	send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: peso %s: %d\r\n", ch->name, ch->weight );
	send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: altezza %s: %d\r\n", ch->name, ch->height );
	mass_ch = compute_mass( ch ) / 4;

	/* Calcola la massa totale della vittima */
	send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: peso %s: %d\r\n", victim->name, victim->weight );
	send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: altezza %s: %d\r\n", victim->name, victim->height );
	mass_victim = compute_mass( victim );

	/* Riproporziona la massa offendibile della vittima se essa si trova in un tunnel;
	 *	per semplicità, diamo per scontato che vi sia spazio solo per 2 direzioni d'attacco */
	if ( HAS_BIT(victim->in_room->flags, ROOM_TUNNEL) )
		mass_victim = (mass_victim / 4) * 2;	/* divide per il numero stabilito per il calcolo della mass_ch */

	send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: la tua massa di attacco è %d\r\n", mass_ch );
	send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: la massa offendibile totale della vittima è %d\r\n", mass_victim );

	for ( rch = ch->in_room->first_person;  rch;  rch = rch_next )
	{
		int n_fight = 0;

		rch_next = rch->next_in_room;

		/* Per ogni PG che combatte contro la vittima diminuisce lo spazio disponibile ad una offesa */
		if ( (rch != victim								)
/*		  && (rch != ch									) direi inutile */
		  && (rch->fighting								)
		  && (who_fighting(rch->fighting->who) == victim) )
		{
			n_fight++;

			send_log( NULL, LOG_SPACEFIGHT, "compute_mass: peso %s: %d\r\n", rch->name, ch->weight );
			send_log( NULL, LOG_SPACEFIGHT, "compute_mass: altezza %s: %d\r\n", rch->name, ch->height );
			mass_victim -= compute_mass( rch ) / 4;
		}

		/* Se ha già addosso il numero massimo supportabile di attaccanti allora non c'è spazio */
		if ( n_fight > MAX_FIGHT )
			return FALSE;

		send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: numero di combattimenti addosso a %s è %d\r\n", victim->name, n_fight );
		send_log( NULL, LOG_SPACEFIGHT, "space_to_fight: la massa offendibile residua di %s è %d\r\n", victim->name, mass_victim );
	}

	/* Se la massa offendibile della vittima è maggiore di quella dell'attaccante allora può combattere */
	if ( mass_victim > mass_ch )	/* (FF) anche se sarebbe bello lasciare la possibilità di gestire un posto in più e dare una probabilità di ferimento in mischia vs tutti */
    	return TRUE;

	return FALSE;
}


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 *
 * Note:  This function also handles some non-violence updates.
 */
void update_violence( void )
{
	CHAR_DATA	*ch;
	CHAR_DATA	*lst_ch;
	CHAR_DATA	*victim;
	CHAR_DATA	*rch;
	CHAR_DATA	*rch_next;
	ch_ret		 retcode;
	int			 attacktype;
	int			 cnt;
	static int	 pulse = 0;


	lst_ch = NULL;
	pulse = (pulse+1) % 100;

	for ( ch = last_char;  ch;  lst_ch = ch, ch = gch_prev )
	{
		set_cur_char( ch );

		if ( ch == first_char && ch->prev )
		{
			send_log( NULL, LOG_BUG, "update_violence: first_char->prev != NULL... corretto." );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;

		if ( gch_prev && gch_prev->next != ch )
		{
			send_log( NULL, LOG_BUG, "update_violence: %s->prev->next doesn't point to ch.  Short-cutting here", ch->name );
			send_log( NULL, LOG_WARNING, "admtalk --> Prepararsi al peggio! <--" );
			ch->prev = NULL;
			gch_prev = NULL;
		}

		/*
		 * See if we got a pointer to someone who recently died...
		 * if so, either the pointer is bad... or it's a player who
		 * "died", and is back at the healer...
		 * Since he/she's in the char_list, it's likely to be the later...
		 * and should not already be in another fight already
		 */
		if ( char_died(ch) )
			continue;

		/*
		 * See if we got a pointer to some bad looking data...
		 */
		if ( !ch->in_room || !ch->name )
		{
			send_log( NULL, LOG_NORMAL, "update_violence: bad ch record!  (Shortcutting.)" );
			send_log( NULL, LOG_NORMAL, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
				(int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );

			if ( lst_ch )
			{
				send_log( NULL, LOG_NORMAL, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
					(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
			}
			else
			{
				send_log( NULL, LOG_NORMAL, "lst_ch: NULL" );
			}

			gch_prev = NULL;

			continue;
		}

		/*
		 * Experience gained during battle deceases as battle drags on
		 */
		if ( ch->fighting )
		{
			if ( (++ch->fighting->duration%24) == 0 )
				ch->fighting->xp = ((ch->fighting->xp * 9) / 10);
		}

		/* check for exits moving players around (FF) pensare se non sia il caso di staccarlo in un update a parte */
		retcode = pullcheck( ch, pulse );
		if ( retcode == rCHAR_DIED || char_died(ch) )
			continue;

		/* Let the battle begin! */
		victim = who_fighting( ch );
		if ( !victim )
			continue;

		if ( HAS_BIT(ch->affected_by, AFFECT_PARALYSIS) )
			continue;

		retcode = rNONE;

		if ( HAS_BIT(ch->in_room->flags, ROOM_SAFE) )
		{
			send_log( NULL, LOG_NORMAL, "update_violence: %s fighting %s in a SAFE room.", ch->name, victim->name );
			stop_fighting( ch, TRUE );
		}
		else if ( is_awake(ch) && ch->in_room == victim->in_room )
		{
			retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
		}
		else
		{
			stop_fighting( ch, FALSE );
		}

		if ( char_died(ch) )
			continue;

		if ( retcode == rCHAR_DIED || (victim = who_fighting(ch)) == NULL )
			continue;

		/*
		 *  Mob triggers. Added some victim death checks, because it IS possible..
		 */
		rprog_rfight_trigger( ch );
		if ( char_died(ch) || char_died(victim) )
			continue;

		mprog_hitprcnt_trigger( ch, victim );
		if ( char_died(ch) || char_died(victim) )
			continue;

		mprog_fight_trigger( ch, victim );
		if ( char_died(ch) || char_died(victim) )
			continue;

		/*
		 * NPC special attack flags
		 */
		if ( IS_MOB(ch) )
		{
			if ( !IS_EMPTY(ch->attacks) )
			{
				attacktype = -1;

				if ( 35 + get_level(ch)/8 >= number_percent() )
				{
					cnt = 0;

					for ( ; ; )
					{
						if ( cnt++ > 10 )
						{
							attacktype = -1;
							break;
						}
						attacktype = number_range( 7, MAX_ATTACK-1 );
						if ( HAS_BIT(ch->attacks, attacktype) )
							break;
					}

					switch ( attacktype )
					{
					  case ATTACK_BASH:
						send_command( ch, "bash", CO );
						retcode = global_retcode;
						break;

					  case ATTACK_STUN:
						send_command( ch, "stun", CO );
						retcode = global_retcode;
						break;

					  case ATTACK_GOUGE:
						send_command( ch, "gouge", CO );

						retcode = global_retcode;
						break;

					 case ATTACK_FEED:
						send_command( ch, "feed", CO );
						retcode = global_retcode;
						break;

					  case ATTACK_DRAIN:
						retcode = spell_energy_drain( skill_lookup("energy drain"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_FIREBREATH:
						retcode = spell_fire_breath( skill_lookup("fire breath"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_FROSTBREATH:
						retcode = spell_frost_breath( skill_lookup("frost breath"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_ACIDBREATH:
						retcode = spell_acid_breath( skill_lookup("acid breath"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_LIGHTNBREATH:
						retcode = spell_lightning_breath( skill_lookup("lightning breath"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_GASBREATH:
						retcode = spell_gas_breath( skill_lookup("gas breath"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_SPIRALBLAST:
						retcode = spell_spiral_blast( skill_lookup("spiral blast"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_POISON:
						retcode = spell_poison( gsn_poison, get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_NASTYPOISON:
/*						retcode = spell_nasty_poison( skill_lookup("nasty poison")/2, get_level(ch), ch, victim ); */
						break;

					  case ATTACK_GAZE:
/*						retcode = spell_gaze( skill_lookup("gaze"), get_level(ch)/2, ch, victim ); */
						break;

					  case ATTACK_BLINDNESS:
						retcode = spell_blindness( gsn_blindness, get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_CAUSESERIOUS:
						retcode = spell_cause_serious( skill_lookup("cause serious"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_EARTHQUAKE:
						retcode = spell_earthquake( skill_lookup("earthquake"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_CAUSECRITICAL:
						retcode = spell_cause_critical( skill_lookup("cause critical"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_CURSE:
						retcode = spell_curse( skill_lookup("curse"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_FLAMESTRIKE:
						retcode = spell_flamestrike( skill_lookup("flamestrike"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_HARM:
						retcode = spell_harm( skill_lookup("harm"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_FIREBALL:
						retcode = spell_fireball( skill_lookup("fireball"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_COLORSPRAY:
						retcode = spell_colour_spray( skill_lookup("colour spray"), get_level(ch)/2, ch, victim );
						break;

					  case ATTACK_WEAKEN:
						retcode = spell_weaken( skill_lookup("weaken"), get_level(ch)/2, ch, victim );
						break;
					}
					if ( attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)) )
						continue;
				} /* chiude l'if */
			} /* chiude l'if */

			/*
			 * NPC special defense flags
			 */
			if ( !IS_EMPTY(ch->defenses) )
			{
				attacktype = -1;
				if ( (get_level(ch)/8) + 55 > number_percent() )
				{
					cnt = 0;

					for ( ; ; )
					{
						if ( cnt++ > 10 )
						{
							attacktype = -1;
							break;
						}
						attacktype = number_range( 2, MAX_DEFENSE-1 );
						if ( HAS_BIT( ch->defenses, attacktype ) )
							break;
					}

					switch ( attacktype )
					{
					  case DEFENSE_CURELIGHT:
						/* A few quick checks in the cure ones so that
						 *	a) less spam and
						 *	b) we don't have mobs looking stupider than normal by healing
						 *	   themselves when they aren't even being hit (although that
						 *	   doesn't happen TOO often */

						if ( ch->points[POINTS_LIFE] < ch->points_max[POINTS_LIFE] )
						{
							act( AT_MAGIC, "$n allevia le sue ferite con dei frettolosi medicamenti.", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("cure light"), get_level(ch)/2, ch, ch );
						}
						break;

					  case DEFENSE_CURESERIOUS:
						if ( ch->points[POINTS_LIFE] < ch->points_max[POINTS_LIFE] )
						{
							act( AT_MAGIC, "$n lenisce le sue ferite con dei frettolosi medicamenti.", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("cure serious"), get_level(ch)/2, ch, ch );
						}
						break;

					  case DEFENSE_CURECRITICAL:
						if ( ch->points[POINTS_LIFE] < ch->points_max[POINTS_LIFE] )
						{
							act( AT_MAGIC, "$n lenisce le sue ferite con dei medicamenti.", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("cure critical"), get_level(ch)/2, ch, ch );
						}
						break;

					  case DEFENSE_HEAL:
						if ( ch->points[POINTS_LIFE] < ch->points_max[POINTS_LIFE] )
						{
							act( AT_MAGIC, "$n cura le sue ferite con dei medicamenti.", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("heal"), get_level(ch)/2, ch, ch );
						}
						break;

					  case DEFENSE_DISPELMAGIC:
						if ( victim->first_affect )
						{
							act( AT_MAGIC, "$n sibila un'inquietante cantilena arcana..", ch, NULL, NULL, TO_ROOM );
							retcode = spell_dispel_magic( skill_lookup("dispel magic"), get_level(ch)/2, ch, victim );
						}
						break;

					  case DEFENSE_DISPELEVIL:
						act( AT_MAGIC, "$n muta la voce in un sussurro fatto di sillabe arcane..", ch, NULL, NULL, TO_ROOM );
						retcode = spell_dispel_evil( skill_lookup("dispel evil"), get_level(ch)/2, ch, victim );
						break;

					  case DEFENSE_TELEPORT:
						retcode = spell_teleport( skill_lookup("teleport"), get_level(ch)/2, ch, ch );
						break;

					  case DEFENSE_SHOCKSHIELD:
						if ( !HAS_BIT(ch->affected_by, AFFECT_SHOCKSHIELD) )
						{
							act( AT_MAGIC, "$n modula la sua voce in un canto al vento..", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("shockshield"), get_level(ch)/2, ch, ch );
						}
						else
							retcode = rNONE;
						break;

					  case DEFENSE_VENOMSHIELD:
						if ( !HAS_BIT(ch->affected_by, AFFECT_VENOMSHIELD) )
						{
							act( AT_MAGIC, "$n sillaba oscuri incanti tessendoli con la voce..", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("venomshield"), get_level(ch)/2, ch, ch );
						}
						else
							retcode = rNONE;
						break;

					  case DEFENSE_ACIDMIST:
						if ( !HAS_BIT(ch->affected_by, AFFECT_ACIDMIST) )
						{
							act( AT_MAGIC, "$n pronuncia a mezza voce velenosi incanti..", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("acidmist"), get_level(ch)/2, ch, ch );
						}
						else
							retcode = rNONE;
						break;

					  case DEFENSE_FIRESHIELD:
						if ( !HAS_BIT(ch->affected_by, AFFECT_FIRESHIELD) )
						{
							act( AT_MAGIC, "$n sibila all'aere.. e lingue di fuoco accompagnano la sua voce..", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("fireshield"), get_level(ch)/2, ch, ch );
						}
						else
							retcode = rNONE;
						break;

					  case DEFENSE_ICESHIELD:
						if ( !HAS_BIT(ch->affected_by, AFFECT_ICESHIELD) )
						{
							act( AT_MAGIC, "$n sibila al vento.. e cristalli di ghiaccio accompagnano la sua voce..", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("iceshield"), get_level(ch)/2, ch, ch );
						}
						else
							retcode = rNONE;
						break;

					  case DEFENSE_TRUESIGHT:
						if ( !HAS_BIT(ch->affected_by, AFFECT_TRUESIGHT) )
							retcode = spell_smaug( skill_lookup("true"), get_level(ch)/2, ch, ch );
						else
							retcode = rNONE;
						break;

					  case DEFENSE_SANCTUARY:
						if ( !HAS_BIT(ch->affected_by, AFFECT_SANCTUARY) )
						{
							act( AT_MAGIC, "$n canta soavemente inebriando l'aere di una mistica cantilena..", ch, NULL, NULL, TO_ROOM );
							retcode = spell_smaug( skill_lookup("sanctuary"), get_level(ch)/2, ch, ch );
						}
						else
							retcode = rNONE;
						break;
					}
					if ( attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)) )
						continue;
				} /* chiude l'if */
			} /* chiude l'if */
		} /* chiude l'if */

		/*
		 * Fun for the whole family!
		 */
		for ( rch = ch->in_room->first_person;  rch;  rch = rch_next )
		{
			rch_next = rch->next_in_room;

			/*
			 *   Group Fighting Styles Support:
			 *   If ch is tanking
			 *   If rch is using a more aggressive style than ch
			 *   Then rch is the new tank   -h
			 */
/*			  &&( is_same_group(ch, rch)	  ) */

			if ( (IS_PG(ch) && IS_PG(rch))
			  && rch != ch
			  && rch->fighting
			  && who_fighting(rch->fighting->who) == ch
			  && (IS_MOB(rch->fighting->who) && !HAS_BIT_ACT(rch->fighting->who, MOB_AUTONOMOUS))
			  && rch->style < ch->style )
			{
				rch->fighting->who->fighting->who = rch;
			}

			if ( is_awake(rch) && !rch->fighting )
			{
				/*
				 * PC's auto-assist others in their group. (TT) pensare se sia il caso di tenerlo
				 */
				if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
				{
					if ( ((IS_PG(rch) && rch->desc)
					  || HAS_BIT(rch->affected_by, AFFECT_CHARM))
					  && is_same_group(ch, rch)
					  && !is_safe(rch, victim, TRUE) )
					{
						multi_hit( rch, victim, TYPE_UNDEFINED );
					}
					continue;
				}

				/*
				 * NPC's assist NPC's of same type or 12.5% chance regardless. (TT) testare se la frequenza di assist è buona
				 */
				if ( IS_MOB(rch) && !HAS_BIT(rch->affected_by, AFFECT_CHARM)
				  && !HAS_BIT_ACT(rch, MOB_NOASSIST) )
				{
					if ( char_died(ch) )
						break;

					if ( rch->pIndexData == ch->pIndexData
					  || number_bits(3) == 0 )
					{
						CHAR_DATA *vch;
						CHAR_DATA *target;
						int		   number;

						target = NULL;
						number = 0;

						for ( vch = ch->in_room->first_person;  vch;  vch = vch->next )
						{
							if ( !can_see(rch, vch) )			continue;
							if ( !is_same_group(vch, victim) )	continue;
							if ( number_range(0, number) != 0 )	continue;

							if ( vch->mount && vch->mount == rch )
							{
								target = NULL;
							}
							else
							{
								target = vch;
								number++;
							}
						}

						if ( target )
							multi_hit( rch, target, TYPE_UNDEFINED );
					}
				} /* chiude l'if */
			} /* chiude l'if */
		} /* chiude il for */
	} /* chiude il for */
}


/*
 * Calculate damage based on resistances, immunities and suceptibilities
 */
int ris_damage( CHAR_DATA *ch, int dam, int ris )
{
	int modifier;

	modifier = 10;
	if ( HAS_BIT(ch->immune, ris)  && !HAS_BIT(ch->no_immune, ris) )
		modifier -= 10;

	if ( HAS_BIT(ch->resistant, ris) && !HAS_BIT(ch->no_resistant, ris) )
		modifier -= 2;

	if ( HAS_BIT(ch->susceptible, ris) && !HAS_BIT(ch->no_susceptible, ris) )
	{
		if ( IS_MOB(ch) && HAS_BIT(ch->immune, ris) )
			modifier += 0;
		else
			modifier += 2;
	}

	if ( modifier <= 0 )
		return -1;

	if ( modifier == 10 )
		return dam;

	return (dam * modifier) / 10;
}


/*
 * Weapon types
 */
int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
	int bonus;

	bonus = 0;
	*gsn_ptr = -1;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "weapon_prof_bonus_check: ch è NULL" );
		return 0;
	}

	if ( !wield )
	{
		send_log( NULL, LOG_BUG, "weapon_prof_bonus_check: wield è NULL" );
		return 0;
	}

	if ( !wield->weapon )
	{
		send_log( NULL, LOG_BUG, "weapon_prof_bonus_check: %s (#%d) non è un'arma",
			wield->short_descr, wield->vnum );
		return 0;
	}

	if ( IS_PG(ch) && get_level(ch) > LVL_NEWBIE/2 )
	{
		switch ( wield->weapon->damage1 )
		{
		  default:					*gsn_ptr = -1;						break;

		  case DAMAGE_HIT:
		  case DAMAGE_SUCTION:
		  case DAMAGE_BITE:
		  case DAMAGE_BLAST:		*gsn_ptr = gsn_pugilism;			break;

		  case DAMAGE_SLASH:
		  case DAMAGE_SLICE:		*gsn_ptr = gsn_long_blades;			break;

		  case DAMAGE_PIERCE:
		  case DAMAGE_STAB:			*gsn_ptr = gsn_short_blades;		break;

		  case DAMAGE_WHIP:			*gsn_ptr = gsn_flexible_arms;		break;

		  case DAMAGE_CLAW:			*gsn_ptr = gsn_talonous_arms;		break;

		  case DAMAGE_POUND:
		  case DAMAGE_CRUSH:		*gsn_ptr = gsn_bludgeons;			break;

		  case DAMAGE_BOLT:
		  case DAMAGE_ARROW:
		  case DAMAGE_DART:
		  case DAMAGE_STONE:
		  case DAMAGE_PEA:			*gsn_ptr = gsn_missile_weapons;		break;
		}

		if ( *gsn_ptr != -1 )
			bonus = (int) ( (knows_skell(ch, *gsn_ptr)-50) / 10 );
	}

	return bonus;
}


/*
 * Calculate the tohit bonus on the object and return RIS values.
 */
int obj_hitroll( OBJ_DATA *obj )
{
	AFFECT_DATA *paf;
	int			 tohit = 0;

	for ( paf = obj->pObjProto->first_affect;  paf;  paf = paf->next )
	{
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	}

	for ( paf = obj->first_affect;  paf;  paf = paf->next )
	{
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	}

	return tohit;
}


/*
 * Offensive shield level modifier
 */
int off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int lvl;

	if ( IS_PG(ch) )          /* players get much less effect */
	{
		lvl = UMAX( 1, (get_level(ch)/2 - 10) / 2 );
		if ( number_percent() + (get_level(victim)/2 - lvl) < 40 )
		{
			if ( can_pkill(ch, NULL) && can_pkill(victim, NULL) )
				return get_level(ch) / 2;
			else
				return lvl;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		lvl = get_level(ch) / 4;
		if ( number_percent() + (get_level(victim)/2 - lvl) < 70 )
			return lvl;
		else
			return 0;
	}
}


/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	OBJ_DATA   *wield;
	int			victim_ac;
	int			thac0;
	int			thac0_00;
	int			thac0_32;
	int			plusris;
	int			dam;
	int			diceroll;
	int			attacktype, cnt;
	int			prof_bonus;
	int			prof_gsn = -1;
	ch_ret		retcode = rNONE;
	static bool dual_flip = FALSE;

	/*
	 * Can't beat a dead char!
	 * Guard against weird room-leavings.
	 */
	if ( victim->position == POSITION_DEAD || ch->in_room != victim->in_room )
		return rVICT_DIED;

#ifdef T2_ALFA
	/* (TT) */
	/* Le anime non possono iniziare un combattimento */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SPIRIT) )
		return rNONE;
	if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_SPIRIT) )
		return rNONE;
	/* Se si è stati uccisi da meno di 10 minuti allora non si viene attaccati */
	if ( IS_PG(victim) && (get_timer(victim, TIMER_KILLED) > 0 || get_timer(victim, TIMER_PKILLED) > 0) )
		return rNONE;
#endif

	used_weapon = NULL;
	/*
	 * Figure out the weapon doing the damage
	 * Dual wield support -- switch weapons each attack
	 */
	if ( (wield = get_eq_char(ch, WEARLOC_DUAL)) != NULL )
	{
		if ( !dual_flip )
		{
			dual_flip = TRUE;
			wield = get_eq_char( ch, WEARLOC_WIELD );
		}
		else
			dual_flip = FALSE;
	}
	else
		wield = get_eq_char( ch, WEARLOC_WIELD );

	used_weapon = wield;

	if ( wield )
		prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
	else
		prof_bonus = 0;


	/* Gestisce gli attacchi di un mob */
	if ( IS_MOB(ch)
	  && ch->fighting		/* make sure fight is already started */
	  && dt == TYPE_UNDEFINED
	  && !IS_EMPTY(ch->attacks) )
	{
		cnt = 0;
		for ( ; ; )
		{
			attacktype = number_range( 0, 6 );
			if ( HAS_BIT(ch->attacks, attacktype) )
				break;

			if ( cnt++ > 16 )
			{
				attacktype = -1;
				break;
			}
		}

		if ( attacktype == ATTACK_BACKSTAB )
			attacktype = -1;
		if ( wield && number_percent() > 25 )
			attacktype = -1;
		if ( !wield && number_percent() > 50 )
			attacktype = -1;

		switch ( attacktype )
		{
		  default:
			break;
		  case ATTACK_BITE:
			send_command( ch, "bite", CO );
			retcode = global_retcode;
			break;
		  case ATTACK_CLAWS:
			send_command( ch, "claw", CO );
			retcode = global_retcode;
			break;
		  case ATTACK_TAIL:
			send_command( ch, "tail", CO );
			retcode = global_retcode;
			break;
		  case ATTACK_STING:
			send_command( ch, "sting", CO );
			retcode = global_retcode;
			break;
		  case ATTACK_PUNCH:
			send_command( ch, "punch", CO );
			retcode = global_retcode;
			break;
		  case ATTACK_KICK:
			send_command( ch, "kick", CO );
			retcode = global_retcode;
			break;
		  case ATTACK_TRIP:
			attacktype = 0;
			break;
		}
		if ( attacktype >= 0 )
			return retcode;
	}

	if ( dt == TYPE_UNDEFINED )
	{
		dt = TYPE_HIT;
		if ( wield && wield->type == OBJTYPE_WEAPON )
			dt += wield->weapon->damage1;
	}

    /*
     * Calculate to-hit-armor-class-0 versus armor.
	 * Zebeid fliped class thac (TT)
     */
	if ( IS_MOB(ch) )
	{
		thac0_00 = table_class[ch->class]->thac0_00;
		thac0_32 = table_class[ch->class]->thac0_32;
	}
	else
	{
		thac0_00 = 0;
		thac0_32 = 0;
	}

	thac0 = interpolate( get_level(ch)/2, thac0_00, thac0_32 ) - get_hitroll( ch );
	victim_ac = UMAX( -19, (int) (get_ac(victim) / 10) );

	/* if you can't see what's coming... */
	if ( wield && !can_see_obj(victim, wield, FALSE) )
		victim_ac += 1;

	if ( !can_see(ch, victim) )
		victim_ac -= 4;

	/*
	 * "learning" between combatants.  Takes the intelligence difference,
	 * and multiplies by the times killed to make up a learning bonus
	 * given to whoever is more intelligent
	 * (basically the more intelligent one "learns" the other's fighting style)
	 */
	if ( ch->fighting && ch->fighting->who == victim )
	{
		int times = ch->fighting->timeskilled;

		if ( times )
		{
			int intdiff = get_curr_attr( ch, ATTR_INT )/4 - get_curr_attr( victim, ATTR_INT )/4;

			if ( intdiff != 0 )
				victim_ac += (intdiff * times) / 10;
		}
	}

	/* Weapon proficiency bonus */
	victim_ac += prof_bonus;

	/*
	 * The moment of excitement!
	 */
	while ( (diceroll = number_bits(5)) >= 20 )
		;

	if ( diceroll == 0
	  || (diceroll != 19 && diceroll < thac0 - victim_ac) )
	{
		/* Physical hit with no damage (hit natural AC) - Seven */
		if ( diceroll < thac0 - victim_ac
		  && diceroll >= thac0 - (get_ac_natural(victim)/10 + table_race[victim->race]->ac_plus/10 + 10)
		  && diceroll != 19
		  && diceroll != 0 )
		{
			char *verb_ch;
			char * verb_vict;

			switch ( number_range(0, 7) )
			{
			  default:	/* il caso più comune */
				verb_ch = "assorbito";
				verb_vict = "assorbe";
				break;
			  case 0:
				verb_ch = "deviato";
				verb_vict = "devia";
				break;
			  case 1:
				verb_ch = "fermato";
				verb_vict = "ferma";
				break;
			  case 2:
				verb_ch = "interrotto";
				verb_vict = "interrompe";
				break;
			  case 3:
				verb_ch = "frenato";
				verb_vict = "frena";
				break;
			  case 4:
				verb_ch = "bloccato";
				verb_vict = "blocca";
				break;
			}

			if ( prof_gsn != -1 )
				learn_skell( ch, prof_gsn, FALSE );

/*			if ( victim->class == CLASS_MAGE )
			{
				act( AT_PLAIN, "Your attack is halted by a wall of energy.", ch, NULL, victim, TO_CHAR );
				act( AT_PLAIN, "$n's attack is stopped by your energy field.", ch, NULL, victim, TO_VICT );
			}
			else*/
			{
				act( AT_HIT, "Il mio attacco è $t dall'armatura di $N.", ch, verb_ch, victim, TO_CHAR );
				act( AT_HITME, "La mia armatura $t l'attacco di $n.", ch, verb_vict, victim, TO_VICT );
			}
			if ( !victim->fighting && victim->in_room == ch->in_room )
				set_fighting( victim, ch );
			tail_chain( );
			return rNONE;
		}
		else
		{
			/* Miss. */
			if ( prof_gsn != -1 )
				learn_skell( ch, prof_gsn, FALSE );
			damage( ch, victim, 0, dt );
			tail_chain( );
			return rNONE;
		}
	}

#if 0
	/*
	 * The moment of excitement!
	 */
	while ( ( diceroll = number_bits( 5 ) ) >= 20 )
		;

	if ( diceroll == 0
	  || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
	{
		/* Miss. */
		if ( prof_gsn != -1 )
			learn_skell( ch, prof_gsn, FALSE );

		damage( ch, victim, 0, dt );
		tail_chain( );
		return rNONE;
	}
#endif

	/*
	 * Hit.
	 * Calc damage.
	 */
	if ( !wield || !wield->weapon )
		dam = number_range( ch->barenumdie, ch->baresizedie * ch->barenumdie ) + ch->damplus;
	else
		dam = number_range( wield->weapon->dice_number, wield->weapon->dice_size*wield->weapon->dice_number );

	/*
	 * Bonuses.
	 */
	dam += get_damroll( ch );

	if ( prof_bonus )
		dam += prof_bonus / 4;

	/*
	 * Calculate Damage Modifiers from Victim's Fighting Style
	 */
	if		( victim->position == POSITION_BERSERK	  )		dam = 1.2  * dam;
	else if ( victim->position == POSITION_AGGRESSIVE )		dam = 1.1  * dam;
	else if ( victim->position == POSITION_DEFENSIVE  )		dam = 0.85 * dam;
	else if ( victim->position == POSITION_EVASIVE	  )		dam = 0.8  * dam;

	/*
	 * Calculate Damage Modifiers from Attacker's Fighting Style
	 */
	if		( ch->position == POSITION_BERSERK		 )		dam = 1.2  * dam;
	else if ( ch->position == POSITION_AGGRESSIVE	 )		dam = 1.1  * dam;
	else if ( ch->position == POSITION_DEFENSIVE	 )		dam = 0.85 * dam;
	else if ( ch->position == POSITION_EVASIVE		 )		dam = 0.8  * dam;

	if ( IS_PG(ch) && ch->pg->learned_skell[gsn_enhanced_damage] > 0 )
	{
		dam += (int) (dam * knows_skell(ch, gsn_enhanced_damage) / 120);
		learn_skell( ch, gsn_enhanced_damage, TRUE );
	}

	if ( !is_awake(victim) )	/* (FF) farlo diverso a seconda della posizione */
		dam *= 2;

	if ( dt == gsn_backstab )
		dam *= 2 + URANGE( 2, get_level(ch)/2 - (get_level(victim)/8), 30 ) / 8;

	if ( dt == gsn_circle )
		dam *= 2 + URANGE( 2, get_level(ch)/2 - (get_level(victim)/8), 30 ) / 16;

	if ( dam <= 0 )
		dam = 1;

	plusris = 0;

	if ( wield )
	{
		if ( HAS_BIT(wield->extra_flags, OBJFLAG_MAGIC) )
			dam = ris_damage( victim, dam, RIS_MAGIC );
		else
			dam = ris_damage( victim, dam, RIS_NONMAGIC );

		/* Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll */
		plusris = obj_hitroll( wield );
	}
	else
		dam = ris_damage( victim, dam, RIS_NONMAGIC );

	/* check for RIS_PLUSx */
	if ( dam )
	{
		int x, res, imm, sus, mod;

		if ( plusris )
			plusris = RIS_PLUS1 + UMIN( plusris, 7 );

		/* initialize values to handle a zero plusris */
		imm = res = -1;  sus = 1;

		/* find high ris */
		for ( x = RIS_PLUS1; x <= RIS_PLUS6; x++ )
		{
			if ( HAS_BIT(victim->immune, x) )		imm = x;
			if ( HAS_BIT(victim->resistant, x) )	res = x;
			if ( HAS_BIT(victim->susceptible, x) )	sus = x;
		}

		mod = 10;
		if ( imm >= plusris )		mod -= 10;
		if ( res >= plusris )		mod -= 2;
		if ( sus <= plusris )		mod += 2;

		/* check if immune */
		if ( mod <= 0 )				dam = -1;
		if ( mod != 10 )			dam = (dam * mod) / 10;
	}

	if ( prof_gsn != -1 )
	{
		if ( dam > 0 )
			learn_skell( ch, prof_gsn, TRUE );
		else
			learn_skell( ch, prof_gsn, FALSE );
	}

	/* immune to damage */
	if ( dam == -1 )
	{
		if ( dt >= 0 && dt < top_sn )
		{
			SKILL_DATA *skill = table_skill[dt];
			bool	   found = FALSE;

			if ( VALID_STR(skill->imm_char) )
			{
				act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
				found = TRUE;
			}

			if ( VALID_STR(skill->imm_vict) )
			{
				act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
				found = TRUE;
			}

			if ( VALID_STR(skill->imm_room) )
			{
				act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOVICT );
				found = TRUE;
			}

			if ( found )
				return rNONE;
		}
		dam = 0;
	} /* chiude l'if */

	if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
		return retcode;
	if ( char_died(ch) )
		return rCHAR_DIED;
	if ( char_died(victim) )
		return rVICT_DIED;

	retcode = rNONE;
	if ( dam == 0 )
		return retcode;

	/*
	 * Weapon spell support
	 * Each successful hit casts a spell
	 */
	if ( wield
	  && !HAS_BIT(victim->immune, RIS_MAGIC)
	  && !HAS_BIT(victim->in_room->flags, ROOM_NOMAGIC) )
	{
		AFFECT_DATA *aff;

		for ( aff = wield->pObjProto->first_affect;  aff;  aff = aff->next )
		{
			if ( aff->location == APPLY_WEAPONSPELL
			  && is_valid_sn(aff->modifier)
			  && table_skill[aff->modifier]->spell_fun )
			{
				retcode = (*table_skill[aff->modifier]->spell_fun) ( aff->modifier, (wield->level/2 + 3)/3, ch, victim );
			}
		}

		if ( (retcode != rNONE && retcode != rSPELL_FAILED) || char_died(ch) || char_died(victim) )
			return retcode;

		for ( aff = wield->first_affect; aff; aff = aff->next )
		{
			if ( aff->location == APPLY_WEAPONSPELL
			  && is_valid_sn(aff->modifier)
			  && table_skill[aff->modifier]->spell_fun )
			{
				retcode = (*table_skill[aff->modifier]->spell_fun) ( aff->modifier, (wield->level/2 + 3)/3, ch, victim );
			}
		}

		if ( (retcode != rNONE && retcode != rSPELL_FAILED) || char_died(ch) || char_died(victim) )
			return retcode;
	} /* chiude l'if */

	/*
	 * Magic shields that retaliate
	 */
	if ( HAS_BIT(victim->affected_by, AFFECT_FIRESHIELD) && !HAS_BIT(ch->affected_by, AFFECT_FIRESHIELD) )
		retcode = spell_smaug( skill_lookup( "flare" ), off_shld_lvl(victim, ch), victim, ch );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;

    if ( HAS_BIT(victim->affected_by, AFFECT_ICESHIELD) && !HAS_BIT(ch->affected_by, AFFECT_ICESHIELD) )
		retcode = spell_smaug( skill_lookup( "iceshard" ), off_shld_lvl(victim, ch), victim, ch );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;

	if ( HAS_BIT(victim->affected_by, AFFECT_SHOCKSHIELD) && !HAS_BIT(ch->affected_by, AFFECT_SHOCKSHIELD) )
		retcode = spell_smaug( skill_lookup( "torrent" ), off_shld_lvl(victim, ch), victim, ch );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;

	if ( HAS_BIT(victim->affected_by, AFFECT_ACIDMIST) && !HAS_BIT(ch->affected_by, AFFECT_ACIDMIST) )
		retcode = spell_smaug( skill_lookup( "acidshot" ), off_shld_lvl(victim, ch), victim, ch );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;

	if ( HAS_BIT(victim->affected_by, AFFECT_VENOMSHIELD) && !HAS_BIT(ch->affected_by, AFFECT_VENOMSHIELD) )
		retcode = spell_smaug( skill_lookup( "venomshot" ), off_shld_lvl(victim, ch), victim, ch );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;

	tail_chain( );
	return retcode;
}


/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	int		hit_chance;
	int		dual_bonus;
	ch_ret	retcode;

	/* add timer to pkillers */
	if ( IS_PG(ch) && IS_PG(victim) )
	{
		add_timer( ch,     TIMER_RECENTFIGHT, PULSE_VIOLENCE * 11, NULL, 0 );
		add_timer( victim, TIMER_RECENTFIGHT, PULSE_VIOLENCE * 11, NULL, 0 );
	}

	if ( is_attack_supressed(ch) )
		return rNONE;

	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_NOATTACK) )
		return rNONE;

#ifdef T2_ALFA
	/* (TT) */
	/* Le anime non possono iniziare un combattimento */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SPIRIT) )
		return rNONE;
	if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_SPIRIT) )
		return rNONE;
	/* Se si è stati uccisi da meno di 10 minuti allora non si viene attaccati */
	if ( IS_PG(victim) && (get_timer(victim, TIMER_KILLED) > 0 || get_timer(victim, TIMER_PKILLED) > 0) )
		return rNONE;
#endif

	if ( (retcode = one_hit(ch, victim, dt)) != rNONE )
		return retcode;

	if ( who_fighting(ch) != victim || dt == gsn_backstab || dt == gsn_circle )
		return rNONE;

	/* Very high chance of hitting compared to chance of going berserk
	 *	40% or higher is always hit.. don't learn anything here though. */
	hit_chance = IS_MOB(ch)  ?  100  :  (knows_skell(ch, gsn_berserk) * 5/2);
	if ( HAS_BIT(ch->affected_by, AFFECT_BERSERK) && number_percent() < hit_chance )
	{
		if ( (retcode = one_hit(ch, victim, dt)) != rNONE || who_fighting(ch) != victim )
			return retcode;
	}

	if ( get_eq_char(ch, WEARLOC_DUAL) )
	{
		dual_bonus = IS_MOB(ch)  ?  get_level( ch ) / 20  :  knows_skell( ch, gsn_dual_wield ) / 10;
		hit_chance = IS_MOB(ch)  ?  get_level( ch )	/ 2	  :  knows_skell( ch, gsn_dual_wield );

		if ( number_percent( ) < hit_chance )
		{
			learn_skell( ch, gsn_dual_wield, TRUE );
			retcode = one_hit( ch, victim, dt );
			if ( retcode != rNONE || who_fighting(ch) != victim )
				return retcode;
		}
		else
			learn_skell( ch, gsn_dual_wield, FALSE );
	}
	else
		dual_bonus = 0;

	/* Guadagno degli stili durante il combattimento, probabilità bassa perché questa funzione viene usata spesso */
	if ( IS_PG(ch) && number_range(0, 5) == 0 )
	{
		if ( ch->style == STYLE_EVASIVE )
		{
			if ( number_percent() > knows_skell(ch, gsn_style_evasive) )
				learn_skell( ch, gsn_style_evasive, FALSE );
		}
		else if ( ch->style == STYLE_DEFENSIVE )
		{
			if ( number_percent() > knows_skell(ch, gsn_style_defensive) )
				learn_skell( ch, gsn_style_defensive, FALSE );
		}
		else if ( ch->style == STYLE_FIGHTING )
		{
			if ( number_percent() > knows_skell(ch, gsn_style_normal) )
				learn_skell( ch, gsn_style_normal, FALSE );
		}
		else if ( ch->style == STYLE_AGGRESSIVE )
		{
			if ( number_percent() > knows_skell(ch, gsn_style_aggressive) )
				learn_skell( ch, gsn_style_aggressive, FALSE );
		}
		else if ( ch->style == STYLE_BERSERK )
		{
			if ( number_percent() > knows_skell(ch, gsn_style_berserk) )
				learn_skell( ch, gsn_style_berserk, FALSE );
		}
	}

	if ( ch->points[POINTS_MOVE] < get_level(ch)/2 )
		dual_bonus = -20;

	/*
	 * NPC predetermined number of attacks
	 */
	if ( IS_MOB(ch) && ch->numattacks > 0 )
	{
		for ( hit_chance = 0;  hit_chance < ch->numattacks;  hit_chance++ )
		{
			retcode = one_hit( ch, victim, dt );
			if ( retcode != rNONE || who_fighting(ch) != victim )
				return retcode;
		}
		return retcode;
	}

	hit_chance = IS_MOB(ch)  ?  get_level(ch)/2  :  (int) ( (knows_skell(ch, gsn_second_attack) + dual_bonus) / 1.5 );
	if ( number_percent( ) < hit_chance )
	{
		learn_skell( ch, gsn_second_attack, TRUE );

		retcode = one_hit( ch, victim, dt );
		if ( retcode != rNONE || who_fighting(ch) != victim )
			return retcode;
	}
	else
	{
		learn_skell( ch, gsn_second_attack, FALSE );
	}

	hit_chance = IS_MOB(ch)  ?  get_level(ch)/2  :  (int) ( (knows_skell(ch, gsn_third_attack) + (dual_bonus*1.5)) / 2 );
	if ( number_percent( ) < hit_chance )
	{
		learn_skell( ch, gsn_third_attack, TRUE );
		retcode = one_hit( ch, victim, dt );

		if ( retcode != rNONE || who_fighting(ch) != victim )
			return retcode;
	}
	else
	{
		learn_skell( ch, gsn_third_attack, FALSE );
	}

	hit_chance = IS_MOB(ch) ? get_level(ch)/2  :  (int) ( (knows_skell(ch, gsn_fourth_attack)+(dual_bonus*2))/3 );
	if ( number_percent( ) < hit_chance )
	{
		learn_skell( ch, gsn_fourth_attack, TRUE );

		retcode = one_hit( ch, victim, dt );
		if ( retcode != rNONE || who_fighting(ch) != victim )
			return retcode;
	}
	else
	{
		learn_skell( ch, gsn_fourth_attack, FALSE );
	}

	hit_chance = IS_MOB(ch)  ?  get_level(ch)/2  :  (int) ( (knows_skell(ch, gsn_fifth_attack) + (dual_bonus*3)) / 4 );
	if ( number_percent( ) < hit_chance )
	{
		learn_skell( ch, gsn_fifth_attack, TRUE );

		retcode = one_hit( ch, victim, dt );
		if ( retcode != rNONE || who_fighting(ch) != victim )
			return retcode;
	}
	else
	{
		learn_skell( ch, gsn_fifth_attack, FALSE );
	}

	retcode = rNONE;

	hit_chance = IS_MOB(ch)  ?  (int) (get_level(ch) / 4)  :  0;
	if ( number_percent( ) < hit_chance )
		retcode = one_hit( ch, victim, dt );

	if ( retcode == rNONE )
	{
		int move;

		if ( !HAS_BIT(ch->affected_by, AFFECT_FLYING) && !HAS_BIT(ch->affected_by, AFFECT_FLOATING) )
			move = encumbrance( ch, table_sector[ch->in_room->sector].move_loss );
		else
			move = encumbrance( ch, 1 );

		if ( ch->points[POINTS_MOVE] )
			ch->points[POINTS_MOVE] = UMAX( 0, ch->points[POINTS_MOVE] - move );
	}

	return retcode;
}


/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */
ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield, OBJ_DATA *projectile, int dist )
{
	int		victim_ac;
	int		thac0;
	int		thac0_00;
	int		thac0_32;
	int		plusris;
	int		dam;
	int		diceroll;
	int		prof_bonus;
	int		prof_gsn = -1;
	int		proj_bonus;
	int		dt;
	ch_ret	retcode;

	if ( !projectile )
		return rNONE;

	if ( projectile->type == OBJTYPE_PROJECTILE
	  || projectile->type == OBJTYPE_WEAPON)
	{
		dt = TYPE_HIT + projectile->weapon->damage1;
		proj_bonus = number_range( projectile->weapon->dice_number, projectile->weapon->dice_size*projectile->weapon->dice_number );
	}
	else
	{
		dt = TYPE_UNDEFINED;
		proj_bonus = number_range(1, URANGE(2, get_obj_weight(projectile)/1000, 100) );	/* (RR) (TT) da testare e ritarare per il peso */
	}

	/*
	 * Can't beat a dead char!
	 */
	if ( victim->position == POSITION_DEAD || char_died(victim) )
	{
		free_object(projectile);
		return rVICT_DIED;
	}

	if ( wield )
		prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
	else
		prof_bonus = 0;

	if ( dt == TYPE_UNDEFINED )
	{
		dt = TYPE_HIT;
		if ( wield && wield->type == OBJTYPE_MISSILE_WEAPON )
			dt += wield->weapon->damage1;
	}

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if ( IS_MOB(ch) )
	{
		thac0_00 = 0;
		thac0_32 = 0;
	}
	else
	{
		thac0_00 = 0;
		thac0_32 = 0;
	}

	thac0 = interpolate( get_level(ch)/2, thac0_00, thac0_32 ) - get_hitroll(ch) + (dist*2);
	victim_ac = UMAX( -19, (int) (get_ac(victim) / 10) );

	/* if you can't see what's coming... */
	if ( !can_see_obj(victim, projectile, FALSE) )
		victim_ac += 1;
	if ( !can_see(ch, victim) )
		victim_ac -= 4;

	/* Weapon proficiency bonus */
	victim_ac += prof_bonus;

	/*
	 * The moment of excitement!
	 */
	while ( (diceroll = number_bits(5)) >= 20 )
		;

	if ( diceroll == 0
	  || (diceroll != 19 && diceroll < thac0 - victim_ac) )
	{
		/* Miss. */
		if ( prof_gsn != -1 )
			learn_skell( ch, prof_gsn, FALSE );

		/* Do something with the projectile */
		if ( number_percent() < 50 )
			free_object(projectile);
		else
		{
			if ( projectile->in_obj )
				obj_from_obj(projectile);

			if ( projectile->carried_by )
				obj_from_char(projectile);

			obj_to_room(projectile, victim->in_room);
		}
		damage( ch, victim, 0, dt );
		tail_chain( );
		return rNONE;
	}

	/*
	 * Hit. Calc damage.
	 */
	if ( !wield )
		dam = proj_bonus;
	else
		dam = number_range( wield->weapon->dice_number, wield->weapon->dice_size*wield->weapon->dice_number) + (proj_bonus / 10 );

	/*
	 * Bonuses.
	 */
	dam += get_damroll(ch);

	if ( prof_bonus )
		dam += prof_bonus / 4;

	/*
	 * Calculate Damage Modifiers from Victim's Fighting Style
	 */
	if		( victim->position == POSITION_BERSERK	  )		dam = 1.2  * dam;
	else if ( victim->position == POSITION_AGGRESSIVE )		dam = 1.1  * dam;
	else if ( victim->position == POSITION_DEFENSIVE  )		dam =  .85 * dam;
	else if ( victim->position == POSITION_EVASIVE	  )		dam =  .8  * dam;

	if ( IS_PG(ch) && ch->pg->learned_skell[gsn_enhanced_damage] > 0 )
	{
		dam += (int) ( dam * knows_skell(ch, gsn_enhanced_damage) / 120 );
		learn_skell( ch, gsn_enhanced_damage, TRUE );
	}

	if ( !is_awake(victim) )
		dam *= 2;

	if ( dam <= 0 )
		dam = 1;

	plusris = 0;

	if ( HAS_BIT(projectile->extra_flags, OBJFLAG_MAGIC) )
		dam = ris_damage( victim, dam, RIS_MAGIC );
	else
		dam = ris_damage( victim, dam, RIS_NONMAGIC );

	/*
	 * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll
	 */
	if ( wield )
		plusris = obj_hitroll( wield );

	/* check for RIS_PLUSx */
	if ( dam )
	{
		int x,
			res,
			imm,
			sus,
			mod;

		if ( plusris )
			plusris = RIS_PLUS1 + UMIN( plusris, 7 );

		/* initialize values to handle a zero plusris */
		imm = -1;
		res = -1;
		sus =  1;

		/* find high ris */
		for ( x = RIS_PLUS1;  x <= RIS_PLUS6;  x++ )
		{
			if ( HAS_BIT(victim->immune, x) )		imm = x;
			if ( HAS_BIT(victim->resistant, x) )	res = x;
			if ( HAS_BIT(victim->susceptible, x) )	sus = x;
		}

		mod = 10;
		if ( imm >= plusris )		mod -= 10;
		if ( res >= plusris )		mod -= 2;
		if ( sus <= plusris )		mod += 2;

		/* check if immune */
		if ( mod <= 0 )				dam = -1;
		if ( mod != 10 )			dam = (dam * mod) / 10;
	}

	if ( prof_gsn != -1 )
	{
		if ( dam > 0 )
			learn_skell( ch, prof_gsn, TRUE );
		else
			learn_skell( ch, prof_gsn, FALSE );
	}

	/* immune to damage */
	if ( dam == -1 )
	{
		if ( dt >= 0 && dt < top_sn )
		{
			SKILL_DATA *skill = table_skill[dt];
			bool	   found = FALSE;

			if ( VALID_STR(skill->imm_char) )
			{
				act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
				found = TRUE;
			}

			if ( VALID_STR(skill->imm_vict) )
			{
				act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
				found = TRUE;
			}

			if ( VALID_STR(skill->imm_room) )
			{
				act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOVICT );
				found = TRUE;
			}

			if ( found )
			{
				if ( number_percent() < 50 )
					free_object(projectile);
				else
				{
					if ( projectile->in_obj )
						obj_from_obj(projectile);
					if ( projectile->carried_by )
						obj_from_char(projectile);
					obj_to_room(projectile, victim->in_room);
				}
				return rNONE;
			}
		} /* chiude l'if */
		dam = 0;
    } /* chiude l'if */

	if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
	{
		free_object(projectile);
		return retcode;
	}

	if ( char_died(ch) )
	{
		free_object(projectile);
		return rCHAR_DIED;
	}

	if ( char_died(victim) )
	{
		free_object(projectile);
		return rVICT_DIED;
	}

	retcode = rNONE;
	if ( dam == 0 )
	{
		if ( number_percent() < 50 )
			free_object(projectile);
		else
		{
			if ( projectile->in_obj )
				obj_from_obj(projectile);
			if ( projectile->carried_by )
				obj_from_char(projectile);
			obj_to_room(projectile, victim->in_room);
		}
		return retcode;
	}

	/* Weapon spells */
	if ( wield
	  && !HAS_BIT(victim->immune, RIS_MAGIC)
	  && !HAS_BIT(victim->in_room->flags, ROOM_NOMAGIC) )
	{
		AFFECT_DATA *aff;

		for ( aff = wield->pObjProto->first_affect;  aff;  aff = aff->next )
		{
			if ( aff->location == APPLY_WEAPONSPELL
			  && is_valid_sn(aff->modifier)
			  && table_skill[aff->modifier]->spell_fun )
			{
				retcode = (*table_skill[aff->modifier]->spell_fun) ( aff->modifier, (wield->level/2 + 3)/3, ch, victim );
			}
		}

		if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		{
			free_object( projectile );
			return retcode;
		}

		for ( aff = wield->first_affect; aff; aff = aff->next )
		{
			if ( aff->location == APPLY_WEAPONSPELL
			  && is_valid_sn(aff->modifier)
			  && table_skill[aff->modifier]->spell_fun )
			{
				retcode = (*table_skill[aff->modifier]->spell_fun) ( aff->modifier, (wield->level/2 + 3)/3, ch, victim );
			}
		}

		if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		{
			free_object( projectile );
			return retcode;
		}
    }

	free_object( projectile );

	tail_chain( );
	return retcode;
}


/*
 * Added code to produce different messages based on weapon type
 * Added better bug message so you can track down the bad dt's
 */
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, OBJ_DATA *obj )
{
	ROOM_DATA	*was_in_room;
	SKILL_DATA	*skill = NULL;
	const char	*vs;				/* Messaggio di danno alla prima persona */
	const char	*vp;				/* Messaggio di danno alla terza persona */
	char		*article;			/* Articolo per la stringa del danno */
	char		*attack;			/* stringa del danno */
	char		 buf_room[256];		/* messaggio di danno a tutti tranne a colui che attacca e a colui che si difende */
	char		 buf_ch[256];		/* messaggio di danno a colui che attacca */
	char		 buf_victim[256];	/* messaggio di danno a colui che si difende */
	char		 buf_admin[256];	/* messaggio di danno numerico per gli imple */
	char		 punct;
	int			 dampc;
	int			 d_index;
	int			 w_index;

	if ( dam == 0 )
		dampc = 0;
	else
		dampc = ( (dam * 1000) / victim->points_max[POINTS_LIFE]) + ( 50 - ((victim->points[POINTS_LIFE] * 50) / victim->points_max[POINTS_LIFE]) );

	if ( ch->in_room != victim->in_room )
	{
		was_in_room = ch->in_room;
		char_from_room( ch );
		char_to_room( ch, victim->in_room );
	}
	else
		was_in_room = NULL;

	/* Get the weapon index */
	if ( dt > 0 && dt < top_sn )
		w_index = 0;
	else if ( dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE )
		w_index = dt - TYPE_HIT;
	else
	{
		send_log( NULL, LOG_BUG, "dam_message: bad dt %d from %s in %d.", dt, ch->name, ch->in_room->vnum );
		dt = TYPE_HIT;
		w_index = 0;
	}

	/* get the damage index */
	if		( dampc ==	 0 )	d_index =  0;
	else if ( dampc <	 0 )	d_index =  1;
	else if ( dampc <= 100 )	d_index =  1 +  dampc	  / 10;
	else if ( dampc <= 200 )	d_index = 11 + (dampc-100)/ 20;
	else if ( dampc <= 900 )	d_index = 16 + (dampc-200)/100;
	else						d_index = 23;

	/* Lookup the damage message */
	vs = table_damage[w_index].message_s[d_index];
	vp = table_damage[w_index].message_p[d_index];

	punct = (dampc <= 30)  ?  '.'  :  '!';

	if ( dt >=0 && dt < top_sn )
		skill = table_skill[dt];

	if ( dt == TYPE_HIT )
	{
		sprintf( buf_room,	 "$n %s $N%c",  vp, punct );
		sprintf( buf_ch,	 "%s $N%c", vs, punct );
		sprintf( buf_victim, "$n mi %s%c", vp, punct );
	}
	else if ( dt > TYPE_HIT && is_wielding_poisoned(ch) )
	{
		char	msg_pois[MIL];

		if ( dt < TYPE_HIT + MAX_DAMAGE )
		{
			attack = code_name(NULL, dt - TYPE_HIT, CODE_DAMAGE );
		}
		else
		{
			send_log( NULL, LOG_BUG, "dam_message: bad dt %d from %s in %d.", dt, ch->name, ch->in_room->vnum );
			dt = TYPE_HIT;
			attack = code_name( NULL, DAMAGE_HIT, CODE_DAMAGE );
		}

		if ( table_damage[dt].gender == 'F' )
			strcpy( msg_pois, "avvelenata" );
		else
			strcpy( msg_pois, "avvelenato" );

		/* Prima il messaggio alla stanza */
		article = get_article( attack, (table_damage[dt].gender == 'F') ? ARTICLE_FEMININE : -1, -1 );
		sprintf( buf_room, "%s %s di $n %s $N%c", article, msg_pois, vp, punct );

		/* Poi il messaggio all'attaccante */
		article = get_article( attack, ARTICLE_POSSESSIVE, (table_damage[dt].gender == 'F') ? ARTICLE_FEMININE : -1, -1 );
		sprintf( buf_ch, "%s %s %s $N%c", article, msg_pois, vp, punct );

		/* Poi il messaggio alla vittima */
		article = get_article( attack, (table_damage[dt].gender == 'F') ? ARTICLE_FEMININE : -1, -1 );
		sprintf( buf_victim, "%s %s di $n mi %s%c", article, msg_pois, vp, punct );
	}
	else
	{
		if ( skill )
		{
			attack = skill->noun_damage;
			if ( dam == 0 )
			{
				bool found = FALSE;

				if ( VALID_STR(skill->miss_char) )
				{
					act( AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR );
					found = TRUE;
				}

				if ( VALID_STR(skill->miss_vict) )
				{
					act( AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT );
					found = TRUE;
				}

				if ( VALID_STR(skill->miss_room) )
				{
					if (strcmp( skill->miss_room,"supress" ) )
						act( AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOVICT );

					found = TRUE;
				}

				if ( found )		/* miss message already sent */
				{
					if ( was_in_room )
					{
						char_from_room( ch );
						char_to_room( ch, was_in_room );
					}
					return;
				}
			}
			else
			{
				if ( VALID_STR(skill->hit_char) )
					act( AT_HIT, skill->hit_char, ch, NULL, victim, TO_CHAR );
				if ( VALID_STR(skill->hit_vict) )
					act( AT_HITME, skill->hit_vict, ch, NULL, victim, TO_VICT );
				if ( VALID_STR(skill->hit_room) )
					act( AT_ACTION, skill->hit_room, ch, NULL, victim, TO_NOVICT );
			}
		}
		else if ( dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE )
		{
			if ( obj )
				attack = obj->short_descr;
			else
				attack = code_name( NULL, dt - TYPE_HIT, CODE_DAMAGE );
		}
		else
		{
			send_log( NULL, LOG_BUG, "dam_message: bad dt %d from %s in %d.", dt, ch->name, ch->in_room->vnum );
			dt = TYPE_HIT;
			attack = code_name( NULL, DAMAGE_HIT, CODE_DAMAGE );
		}

		/* Prima il messaggio alla stanza */
		article = get_article( attack, (table_damage[dt].gender == 'F') ? ARTICLE_FEMININE : -1, -1 );
		sprintf( buf_room, "%s di $n %s $N%c", article, vp, punct );

		/* Poi il messaggio all'attaccante */
		article = get_article( attack, ARTICLE_POSSESSIVE, (table_damage[dt].gender == 'F') ? ARTICLE_FEMININE : -1, -1 );
		sprintf( buf_ch, "%s %s $N%c", article, vp, punct );

		/* Poi il messaggio alla vittima */
		article = get_article( attack, (table_damage[dt].gender == 'F') ? ARTICLE_FEMININE : -1, -1 );
		sprintf( buf_victim, "%s di $n mi %s%c", article, vp, punct );
	}

	/* Inizializza la stringa per il danno numerico */
	sprintf( buf_admin, " [%d]", dam );

	act( AT_ACTION, buf_room, ch, NULL, victim, TO_NOVICT );

	if ( IS_ADMIN(ch) )
		strcat( buf_ch, buf_admin );
	act( AT_HIT, buf_ch, ch, NULL, victim, TO_CHAR );

	if ( IS_ADMIN(ch) )
		strcat( buf_victim, buf_admin );	/* (TT) (bb) ma funziona? */
	act( AT_HITME, buf_victim, ch, NULL, victim, TO_VICT );

	if ( was_in_room )
	{
		char_from_room(ch);
		char_to_room( ch, was_in_room );
	}
}


int align_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
	int		newalign;

	if ( !gch )
	{
		send_log( NULL, LOG_BUG, "align_compute: gch passato è NULL" );
		return gch->alignment;;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "align_compute: victim passata è NULL" );
		return gch->alignment;;
	}

	if ( gch->morph )
		return gch->alignment;

#ifdef T2_ARENA
	/* Se il pg si trovava in arena l'allineamento non varia */
	if ( in_arena(gch) )
		return gch->alignment;
#endif

	if ( is_align(gch, ALIGN_GOOD) || is_align(gch, ALIGN_EVIL) )
	{
		if		( is_align(victim, ALIGN_GOOD) )	newalign = gch->alignment-1;
		else if ( is_align(victim, ALIGN_EVIL) )	newalign = gch->alignment+2;
		else										newalign = gch->alignment;
	}
	else
	{
		if		( is_align(victim, ALIGN_GOOD) )	newalign = gch->alignment-1;
		else if ( is_align(victim, ALIGN_EVIL) )	newalign = gch->alignment+1;
		else										newalign = gch->alignment;
	}

	newalign = URANGE( -1000, newalign, 1000 );
	return newalign;
}


/*
 * Just verify that a corpse looting is legal
 */
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim )
{
	/* anyone can loot mobs */
	if ( IS_MOB(victim) )
		return TRUE;

	/* non-charmed mobs can loot anything */
	if ( IS_MOB(ch) && !ch->master )
		return TRUE;

#ifdef T2_ALFA
    /* Se il pg è un cacciatore di taglie e la vittima un criminale, allora può lootare */
	if ( IS_PG(ch) && IS_PG(victim)
	  && HAS_BIT_PLR(ch, PLAYER_HEADHUNTER)
	  && (HAS_BIT_PLR(victim, PLAYER_KILLER) || HAS_BIT_PLR(victim, PLAYER_THIEF)) )
	{
		return TRUE;
	}
#endif

	return FALSE;
}


/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int		x;

	/* NPC's are fair game. */
	if ( IS_MOB(victim) )
	{
		if ( IS_PG(ch) )
		{
			if ( ch->pg->clan )
				ch->pg->clan->mkills++;

			ch->pg->mkills++;
			ch->in_room->area->mkills++;
		}

		return;
	}


	/*
	 * If you kill yourself nothing happens.
	 */
	if ( ch == victim || IS_ADMIN(ch) )
		return;

#ifdef T2_ARENA
	/* Any character in the arena is ok to kill.
	 * Added pdeath and pkills here */
	if ( in_arena(ch) )
	{
		if ( IS_PG(ch) && IS_PG(victim) )
		{
			ch->pg->pkills++;
			victim->pg->pdeaths++;
		}

		return;
	}
#endif

	/*
	 * So are killers and thieves.
	 */
	if ( HAS_BIT_PLR(victim, PLAYER_KILLER)
	  || HAS_BIT_PLR(victim, PLAYER_THIEF) )
	{
		if ( IS_PG(ch) )
		{
			if ( ch->pg->clan )
				ch->pg->clan->pkills++;

			ch->pg->pkills++;
			ch->in_room->area->pkills++;
		}
		return;
	}

    /* Not of same clan? Go ahead and kill!!! */
	if ( IS_PG(ch) && IS_PG(victim)
	  && (!ch->pg->clan
	   || !victim->pg->clan
	   || ch->pg->clan != victim->pg->clan) )
	{
		if ( ch->pg->clan )
			ch->pg->clan->pkills++;

		ch->pg->pkills++;
		for ( x = 0;  x < MAX_POINTS;  x++ )
			ch->points[x] = ch->points_max[x];

		if ( ch->pg )
			ch->pg->condition[CONDITION_BLOODTHIRST] = get_level(ch)/2 + 10;

		update_position( victim );
		if ( victim != ch )
		{
			act( AT_MAGIC, "Scariche azzurre di energia si sollevano dal corpo penetrando $n.", ch, victim->name, NULL, TO_ROOM );
			act( AT_MAGIC, "Scariche azzurre di energia si sollevano dal corpo penetrandoti.", ch, victim->name, NULL, TO_CHAR );
		}

		if ( victim->pg->clan )
			victim->pg->clan->pdeaths++;

		victim->pg->pdeaths++;
		add_timer( victim, TIMER_PKILLED, SPIRIT_TIME, NULL, 0 );
		WAIT_STATE( victim, PULSE_VIOLENCE * 3 );
		return;
	} /* chiude l'if */

	/*
	 * Charm-o-rama.
	 */
	if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		if ( !ch->master )
		{
			send_log( NULL, LOG_BUG, "check_killer: %s bad AFFECT_CHARM", IS_MOB(ch)  ?  ch->short_descr  :  ch->name );
			affect_strip( ch, gsn_charm_person );
			REMOVE_BIT( ch->affected_by, AFFECT_CHARM );
			return;
		}
/*		stop_follower( ch ); */

		if ( ch->master )
			check_killer( ch->master, victim );

		return;
	}

	/*
	 * NPC's are cool of course (as long as not charmed).
	 * Hitting yourself is cool too (bleeding).
	 * So is being admin (Alander's idea).
	 * And current killers stay as they are.
	 */
	if ( IS_MOB(ch) )
	{
		if ( IS_PG(victim) )
		{
			if ( victim->pg->clan )
				victim->pg->clan->mdeaths++;

			victim->pg->mdeaths++;
			victim->in_room->area->mdeaths++;

			add_timer( victim, TIMER_KILLED, SPIRIT_TIME, NULL, 0 );
		}
		return;
	}

	if ( ch->pg->clan )
		ch->pg->clan->illegal_pk++;
	ch->pg->illegal_pk++;
	ch->in_room->area->illegal_pk++;

	if ( IS_PG(victim) )
	{
		if ( victim->pg->clan )
			victim->pg->clan->pdeaths++;

		victim->pg->pdeaths++;
		victim->in_room->area->pdeaths++;
	}

	if ( HAS_BIT_PLR(ch, PLAYER_KILLER) )
		return;

	set_char_color( AT_WHITE, ch );
	send_to_char( ch, "Una strana sensazione ti pervade.. e un tremito ti attraversa..\r\n" );
	set_char_color( AT_ADMIN, ch );
	send_to_char( ch, "Una voce tuona nella tua testa.. Il tuo crimine ti segna ora indelebilmente!\r\n" );
	set_char_color( AT_WHITE, ch );
	send_to_char( ch, "Senti come se la tua anima si stesse disvelando al mondo intero..\r\n" );
	SET_BIT( ch->pg->flags, PLAYER_KILLER );

	if ( HAS_BIT_PLR(ch, PLAYER_ATTACKER) )
		REMOVE_BIT( ch->pg->flags, PLAYER_ATTACKER );

	save_player( ch );
}


/*
 * See if an attack justifies a ATTACKER flag.
 */
void check_attacker( CHAR_DATA *ch, CHAR_DATA *victim )
{
	/*
	 * NPC's are fair game.
	 * So are killers and thieves.
	 */
	if ( IS_MOB(victim)
	  || (IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_KILLER))
	  || (IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_THIEF)) )
	{
		return;
	}

	/* deadly char check */
	if ( can_pkill(ch, NULL) && can_pkill(victim, NULL) )
		return;

	/*
	 * Charm-o-rama.
	 */
	if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		if ( !ch->master )
		{
			send_log( NULL, LOG_BUG, "check_attacker: %s bad AFFECT_CHARM", IS_MOB(ch)  ?  ch->short_descr  :  ch->name );
			affect_strip( ch, gsn_charm_person );
			REMOVE_BIT( ch->affected_by, AFFECT_CHARM );
			return;
		}

		/* Won't have charmed mobs fighting give the master an attacker flag.
		 * The killer flag stays in, and I'll put something in do_murder. */
/*		SET_BIT( ch->master->pg->flags, PLAYER_ATTACKER ); */
/*		stop_follower( ch ); */

		return;
	}

	/*
	 * NPC's are cool of course (as long as not charmed).
	 * Hitting yourself is cool too (bleeding).
	 * Gli amministratori possono attaccare.
	 * sì può attaccare se ci si trova in arena
	 * And current killers stay as they are.
	 */
	if ( IS_MOB(ch)
	  || ch == victim
	  || IS_ADMIN(ch)
#ifdef T2_ARENA
	  || in_arena(ch)
#endif
	  || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ATTACKER))
	  || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_KILLER)) )
	{
		return;
	}

	SET_BIT( ch->pg->flags, PLAYER_ATTACKER );
	save_player( ch );
}


/*
 * Inflict damage from a hit. This is one damn big function.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	CHAR_DATA  *gch /*, *lch  */;
	OBJ_DATA   *damobj;
	char		filename[MIL];
	int			dameq;
	int			maxdam;
	int			dampmod;
	int			init_gold;
	int			new_gold;
	int			gold_diff;
	ch_ret		retcode;
	bool		loot;

	retcode = rNONE;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "damage: ch è NULL." );
		return rERROR;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "damage: victim è NULL." );
		return rVICT_DIED;
	}

	if ( victim->position == POSITION_DEAD )
		return rVICT_DIED;

	/*
	 * Check damage types for RIS
	 */
	if ( dam && dt != TYPE_UNDEFINED )
	{
		if		( is_spelldamage(dt, SPELLDAMAGE_FIRE) )						dam = ris_damage( victim, dam, RIS_FIRE );
		else if ( is_spelldamage(dt, SPELLDAMAGE_COLD) )						dam = ris_damage( victim, dam, RIS_COLD );
		else if ( is_spelldamage(dt, SPELLDAMAGE_ACID) )						dam = ris_damage( victim, dam, RIS_ACID );
		else if ( is_spelldamage(dt, SPELLDAMAGE_ELECTRICITY) )					dam = ris_damage( victim, dam, RIS_ELECTRICITY );
		else if ( is_spelldamage(dt, SPELLDAMAGE_ENERGY) )						dam = ris_damage( victim, dam, RIS_ENERGY );
		else if ( is_spelldamage(dt, SPELLDAMAGE_DRAIN) )						dam = ris_damage( victim, dam, RIS_DRAIN );
		else if ( is_spelldamage(dt, SPELLDAMAGE_POISON) || dt == gsn_poison )	dam = ris_damage( victim, dam, RIS_POISON );
		else if ( dt == (TYPE_HIT+DAMAGE_POUND) || dt == (TYPE_HIT+DAMAGE_CRUSH )
		  ||	  dt == (TYPE_HIT+DAMAGE_STONE) || dt == (TYPE_HIT+DAMAGE_PEA   ) )	dam = ris_damage( victim, dam, RIS_BLUNT );
		else if ( dt == (TYPE_HIT+DAMAGE_STAB ) || dt == (TYPE_HIT+DAMAGE_PIERCE)
		  ||	  dt == (TYPE_HIT+DAMAGE_BITE ) || dt == (TYPE_HIT+DAMAGE_BOLT  )
		  ||	  dt == (TYPE_HIT+DAMAGE_DART ) || dt == (TYPE_HIT+DAMAGE_ARROW ) )	dam = ris_damage( victim, dam, RIS_PIERCE );
		else if ( dt == (TYPE_HIT+DAMAGE_SLICE) || dt == (TYPE_HIT+DAMAGE_SLASH )
		  ||	  dt == (TYPE_HIT+DAMAGE_WHIP ) || dt == (TYPE_HIT+DAMAGE_CLAW  ) )	dam = ris_damage( victim, dam, RIS_SLASH );

		if ( dam == -1 )
		{
			if ( dt >= 0 && dt < top_sn )
			{
				bool	   found = FALSE;
				SKILL_DATA *skill = table_skill[dt];

				if ( VALID_STR(skill->imm_char) )
				{
					act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
					found = TRUE;
				}

				if ( VALID_STR(skill->imm_vict) )
				{
					act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
					found = TRUE;
				}

				if ( VALID_STR(skill->imm_room) )
				{
					act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOVICT );
					found = TRUE;
				}

				if ( found )
					return rNONE;
			}
			dam = 0;
		} /* chiude l'if */
	} /* chiude l'if */

	/*
	 * Precautionary step mainly to prevent people in Hell from finding a way out.
	 */
	if ( HAS_BIT(victim->in_room->flags, ROOM_SAFE) )
		dam = 0;

	if ( (dam > 0 || number_range(0,1)) && IS_MOB(victim) && ch != victim )
	{
		if ( IS_MOB(victim) && !HAS_BIT_ACT(victim, MOB_SENTINEL ) )
		{
			if ( victim->hunting )
			{
				if ( victim->hunting->who != ch )
				{
					DISPOSE( victim->hunting->name );
					victim->hunting->name = str_dup( ch->name );
					victim->hunting->who  = ch;
				}
			}
			else if ( IS_MOB(victim) && !HAS_BIT_ACT(victim, MOB_PACIFIST) )
			{
				start_hunting( victim, ch );
			}
		}

		if ( victim->hating )
		{
			if ( victim->hating->who != ch )
			{
				DISPOSE( victim->hating->name );
				victim->hating->name = str_dup( ch->name );
				victim->hating->who  = ch;
			}
		}
		else if ( IS_MOB(victim) && !HAS_BIT_ACT(victim, MOB_PACIFIST) )
			start_hating( victim, ch );
	}

	/*
	 * Stop up any residual loopholes.
	 */
	if ( dt == gsn_backstab )
		maxdam = get_level( ch ) * 40;
	else
		maxdam = get_level( ch ) * 20;

	if ( dam > maxdam && !IS_ADMIN(ch) )
	{
		send_log( NULL, LOG_BUG, "damage: %d more than %d points! ** %s (lvl %d) -> %s **",
			dam, maxdam, ch->name, ch->level, victim->name );
		dam = maxdam;
	}

	if ( victim != ch )
	{
		/*
		 * Certain attacks are forbidden. Most other attacks are returned.
		 */
		if ( is_safe(ch, victim, TRUE) )
			return rNONE;

		check_attacker( ch, victim );

		if ( victim->position > POSITION_STUN )
		{
			if ( !victim->fighting && victim->in_room == ch->in_room )
				set_fighting(victim, ch);

			if ( IS_MOB(victim) && victim->fighting )
				victim->position = POSITION_FIGHT;
			else if ( victim->fighting )
			{
				switch ( victim->style )
				{
				  case STYLE_EVASIVE:		victim->position = POSITION_EVASIVE;		break;
				  case STYLE_DEFENSIVE:		victim->position = POSITION_DEFENSIVE;	break;
				  case STYLE_AGGRESSIVE:	victim->position = POSITION_AGGRESSIVE;	break;
				  case STYLE_BERSERK:		victim->position = POSITION_BERSERK;		break;
				  default:					victim->position = POSITION_FIGHT;		break;
				}
			}
		}

		if ( victim->position > POSITION_STUN )
		{
			if ( !ch->fighting && victim->in_room == ch->in_room )
				set_fighting( ch, victim );

			/*
			 * If victim is charmed, ch might attack victim's master.
			 */
			if ( IS_MOB(ch) && IS_MOB(victim)
			  && HAS_BIT(victim->affected_by, AFFECT_CHARM)
			  && victim->master
			  && victim->master->in_room == ch->in_room
			  && number_bits(3) == 0 )
			{
				stop_fighting( ch, FALSE );
				retcode = multi_hit( ch, victim->master, TYPE_UNDEFINED );
				return retcode;
			}
		}


		/* More charm stuff */
		if ( victim->master == ch )
			stop_follower( victim );


		/*
		 * Se uno del gruppo ha attaccato un pg allora quelli del gruppo attaccante
		 *	se hanno la flag PLAYER_PKGROUP disattiva escono dal gruppo
		 */
		if ( IS_PG(ch) && IS_PG(victim) )
		{
			/* Disband from same group first */
			if ( is_same_group(ch, victim) )
			{
				/* Messages to char and master handled in stop_follower */
				act( AT_ACTION, "$n esce dal gruppo di $N.",
					(ch->leader == victim)  ?  victim		   :  ch, NULL,
					(ch->leader == victim)  ?  victim->master  :  ch->master,
					TO_NOVICT );

				if ( ch->leader == victim )
					stop_follower( victim );
				else
					stop_follower( ch );
			}

			/* Toglie i pg senza flag PLAYER_PKGROUP dal gruppo di ch */
			for ( gch = ch->in_room->first_person;  gch;  gch = gch->next_in_room )
			{
				if ( IS_MOB(gch) )				continue;
				if ( gch == ch )				continue;
				if ( !is_same_group(gch, ch) )	continue;

				if ( !HAS_BIT_PLR(gch, PLAYER_PKGROUP) )
				{
					act( AT_ACTION, "$n esce dal gruppo di $N.", ch, NULL, gch->master, TO_NOVICT );
					stop_follower( gch );
				}
			}

			/* Toglie i pg senza flag PLAYER_PKGROUP dal gruppo di ch */
			for ( gch = victim->in_room->first_person;  gch;  gch = gch->next_in_room )
			{
				if ( IS_MOB(gch) )					continue;
				if ( gch == victim )				continue;
				if ( !is_same_group(gch, victim) )	continue;

				if ( !HAS_BIT_PLR(gch, PLAYER_PKGROUP) )
				{
					act( AT_ACTION, "$n esce dal gruppo di $N.", gch, NULL, gch->master, TO_NOVICT );
					stop_follower( gch );
				}
			}
		} /* chiude l'if */


		/*
		 * Inviso attacks ... not.
		 */
		if ( HAS_BIT(ch->affected_by, AFFECT_INVISIBLE) )
		{
			affect_strip( ch, gsn_invis );
			affect_strip( ch, gsn_mass_invis );
			REMOVE_BIT( ch->affected_by, AFFECT_INVISIBLE );
			act( AT_MAGIC, "$n torna visibile.. dal nulla.", ch, NULL, NULL, TO_ROOM );
		}

		/* Take away Hide */
		if ( HAS_BIT(ch->affected_by, AFFECT_HIDE) )
			REMOVE_BIT( ch->affected_by, AFFECT_HIDE );

		/*
		 * Damage modifiers.
		 */

		if ( HAS_BIT(victim->affected_by, AFFECT_SANCTUARY) )
			dam /= 2;

		if ( HAS_BIT(victim->affected_by, AFFECT_PROTECT) && is_align(ch, ALIGN_EVIL) )
			dam -= (int) (dam / 4);

		if ( dam < 0 )
			dam = 0;

		/*
		 * Check for disarm, trip, parry, dodge and tumble.
		 */
		if ( dt >= TYPE_HIT && ch->in_room == victim->in_room )
		{
			if ( IS_MOB(ch)
			  && HAS_BIT(ch->defenses, DEFENSE_DISARM)
			  && get_level(ch) >= LVL_NEWBIE
			  && number_percent() < get_level(ch) / 6 )	/* Was 4 try this */
			{
				disarm( ch, victim );
			}

			if ( IS_MOB(ch)
			  && HAS_BIT(ch->attacks, ATTACK_TRIP)
			  && get_level(ch) >= LVL_NEWBIE/2
			  && number_percent() < get_level(ch) / 4 )
			{
				trip( ch, victim );
			}

			if ( check_parry (ch, victim) )		return rNONE;
			if ( check_dodge (ch, victim) )		return rNONE;
			if ( check_tumble(ch, victim) )		return rNONE;
		}

		/*
		 * Check control panel settings and modify damage
		 */
		if ( IS_MOB(ch) )
		{
			if ( IS_MOB(victim) )
				dampmod = sysdata.dam_mob_vs_mob;
			else
				dampmod = sysdata.dam_mob_vs_plr;
		}
		else
		{
			if ( IS_MOB(victim) )
				dampmod = sysdata.dam_plr_vs_mob;
			else
				dampmod = sysdata.dam_plr_vs_plr;
		}

		if ( dampmod > 0 )
			dam = (dam * dampmod) / 100;

		dam_message( ch, victim, dam, dt, NULL );
	} /* chiude l'if */

	/*
	 * Code to handle equipment getting damaged, and also support
	 * bonuses/penalties for having or not having equipment where hit
	 */
	if ( dam > 10 && dt != TYPE_UNDEFINED )
	{
		/* get a random body eq part */
		dameq  = number_range(WEARLOC_LIGHT, WEARLOC_EYES);
		damobj = get_eq_char(victim, dameq);

		if ( damobj )
		{
			if ( dam > get_obj_resistance(damobj) && number_bits(1) == 0 )
			{
				set_cur_obj( damobj );
				damage_obj( damobj );
			}
			dam -= 5;  /* add a bonus for having something to block the blow */
		}
		else
			dam += 5;  /* add penalty for bare skin! */
	}

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	if ( IS_MOB(victim) )
	{
		victim->points[POINTS_LIFE] -= dam;
	}
	else
	{
		/* In pratica si prendono un poco di danno in più a seconda del livello:
		 *	lev   1		* 1,001		ovvero lo 0,1% in più
		 *	lev  50		* 1,05		ovvero il 5% in più
		 *	lev 100		* 1,1		ovvero il 10% in più */
		victim->points[POINTS_LIFE] -= dam + ( dam * (get_level(ch)/1000) );
	}

	/* calcolo dei px per il danno */
	exp_for_damage( ch, victim, dam, dt );

	if ( dam > 0 && dt > TYPE_HIT
	  && !HAS_BIT(victim->affected_by, AFFECT_POISON)
	  && is_wielding_poisoned(ch)
	  && !HAS_BIT(victim->immune, RIS_POISON)
	  && !compute_savthrow(get_level(ch)/2, victim, SAVETHROW_POISON_DEATH) )
	{
		AFFECT_DATA *aff;

		CREATE( aff, AFFECT_DATA, 1 );
		aff->type		 = gsn_poison;
		aff->duration	 = 60;
		aff->location	 = APPLY_STR;
		aff->modifier	 = -2;
		aff->bitvector = meb( AFFECT_POISON );
		affect_join( victim, aff );
		victim->mental_state = URANGE( 20, victim->mental_state+2, 100 );
	}

	update_position( victim );

	switch ( victim->position )
	{
	  case POSITION_MORTAL:
		act( AT_DYING, "$n è mortalmente ferit$x.. morirà presto se non aiutat$x",
			victim, NULL, NULL, TO_ROOM );
		act( AT_DANGER, "Le ferite sul tuo corpo sono mortali. Hai bisogno di cure se vuoi sopravvivere.",
			victim, NULL, NULL, TO_CHAR );
		break;

	  case POSITION_INCAP:
		act( AT_DYING, "$n non può più muoversi a causa delle ferite.. e morirà presto se non aiutat$x.",
			victim, NULL, NULL, TO_ROOM );
		act( AT_DANGER, "Non riesci più a muoverti.. e la morte ti raggiungerà presto se nessuno ti aiuta.",
			victim, NULL, NULL, TO_CHAR );
		break;

	  case POSITION_STUN:
		if ( !HAS_BIT(victim->affected_by, AFFECT_PARALYSIS) )
		{
			act( AT_ACTION, "$n è stordit$x.. ma potrebbe riaversi.",
				victim, NULL, NULL, TO_ROOM );
			act( AT_HURT, "Non riesci a tornare in te.. ma forse potresti farcela.",
				victim, NULL, NULL, TO_CHAR );
		}
		break;

	  case POSITION_DEAD:
		if ( dt >= 0 && dt < top_sn )
		{
			SKILL_DATA *skill = table_skill[dt];

			if ( VALID_STR(skill->die_char) )
				act( AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR );
			if ( VALID_STR(skill->die_vict) )
				act( AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT );
			if ( VALID_STR(skill->die_room) )
				act( AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOVICT );
		}
		act( AT_DEAD, "$n è mort$x!!", victim, 0, 0, TO_ROOM );
		act( AT_DEAD, "Un vortice di orrore e dolore ti assale.. Avresti mai immaginato così.. la morte?\r\n", victim, 0, 0, TO_CHAR );
		break;

	  default:
		/*
		 * Victim mentalstate affected, not attacker -- oops ;)
		 */
		if ( dam > victim->points_max[POINTS_LIFE]/5 )
		{
			act( AT_HURT, "Fa davvero MALE!", victim, 0, 0, TO_CHAR );
			if ( number_bits(3) == 0 )
				worsen_mental_state( victim, 1 );
		}

		if ( victim->points[POINTS_LIFE] < victim->points[POINTS_LIFE]/5 )
		{
			act( AT_DANGER, "Il tuo stesso sangue ti ricopre.. brucia ovunque!",
				victim, 0, 0, TO_CHAR );
			if ( number_bits(2) == 0 )
				worsen_mental_state( victim, 1 );
		}
		break;
	}

	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if ( !is_awake(victim)		/* lets make NPC's not slaughter PC's */
	  && !HAS_BIT(victim->affected_by, AFFECT_PARALYSIS) )
	{
		if ( victim->fighting
		  && victim->fighting->who->hunting
		  && victim->fighting->who->hunting->who == victim )
		{
			stop_hunting( victim->fighting->who );
		}

		if ( victim->fighting
		  && victim->fighting->who->hating
		  && victim->fighting->who->hating->who == victim )
		{
			stop_hating( victim->fighting->who );
		}

		if ( IS_PG(victim) && IS_MOB(ch) )
			stop_fighting( victim, TRUE );
		else
			stop_fighting( victim, FALSE );
	}

	/*
	 * Payoff for killing things.
	 */
	if ( victim->position == POSITION_DEAD )
	{
		group_gain( ch, victim );

		if ( IS_PG(victim) )
		{
			char log_buf[MSL];

			/* Distingue l'output inviato diviso da uccisore mob e uccisore player */
			if ( IS_MOB(ch) )
				sprintf( log_buf, "dal mob %s (#%d)", ch->short_descr, ch->pIndexData->vnum );
			else
				sprintf( log_buf, "dal pg %s (lev %d)", ch->name, ch->level );

			if ( ch == victim )
			{
				send_log( NULL, LOG_PK, "%s (liv %d) è mort%c a %s (#%d)",
					victim->name,
					victim->level,
					gramm_ao(victim),
					victim->in_room->name,
					victim->in_room->vnum );
			}
			else
			{
				send_log( NULL, LOG_PK, "%s (liv %d) è stat%s %s a %s (#%d)",
					victim->name,
					victim->level,
					(victim->sex == SEX_FEMALE) ? "a uccisa" : "o ucciso",
					log_buf,
					victim->in_room->name,
					victim->in_room->vnum );
			}

			if ( IS_PG(ch) && !IS_ADMIN(ch) && victim != ch
			  && ch->pg->clan && victim->pg->clan )
			{
				if ( victim->pg->clan->name != ch->pg->clan->name )
				{
					sprintf( filename, "%s%s.victories", CLAN_DIR, ch->pg->clan->name );
					sprintf( log_buf, "%-12s %-12s %-14s %s",
						ch->name,
						victim->name,
						victim->pg->clan->name,
						ch->in_room->area->name );
					append_to_file( filename, log_buf );
				}
				else
				{
					sprintf( filename, "%s%s.defeats", CLAN_DIR, ch->pg->clan->name );
					sprintf( log_buf, "%-12s %-12s %-14s %s",
						victim->name,
						ch->name,
						victim->pg->clan->name,
						ch->in_room->area->name );
					append_to_file( filename, log_buf );
				}
			}
		}
		else if ( IS_PG(ch) )		/* keep track of mob vnum killed */
		{
			add_kill( ch, victim );
		}

		check_killer( ch, victim );

		if ( ch->in_room == victim->in_room )
			loot = legal_loot( ch, victim );
		else
			loot = FALSE;

		set_cur_char( victim );
		raw_kill( ch, victim );
		victim = NULL;

		if ( IS_PG(ch) && loot )
		{
			/* Autogold */
			if ( HAS_BIT_PLR(ch, PLAYER_AUTOGOLD) )
			{
				init_gold = ch->gold;
				/* l'Autosplit viene gestito dalla get */
				send_command( ch, "get monete cadavere", CO );
				new_gold = ch->gold;
				gold_diff = ( new_gold - init_gold );
			}

			if ( HAS_BIT_PLR(ch, PLAYER_AUTOLOOT) && victim != ch )	/* prevent nasty obj problems */
				send_command( ch, "get all cadavere", CO );
			else
				send_command( ch, "look in cadavere", CO );

			if ( HAS_BIT_PLR(ch, PLAYER_AUTOSAC) )
				send_command( ch, "sacrifice cadavere", CO );
		}
		save_player( ch );
		return rVICT_DIED;
	}

	if ( victim == ch )
		return rNONE;

	/*
	 * Take care of link dead people.
	 */
	if ( IS_PG(victim) && !victim->desc && !HAS_BIT_PLR(victim, PLAYER_AUTORECALL) )
	{
		if ( number_range(0, victim->wait) == 0 )
		{
			send_command( victim, "recall", CO );
			return rNONE;
		}
	}

	/*
	 * Wimp out?
	 */
	if ( IS_MOB(victim) && dam > 0 )
	{
		if ( ((IS_MOB(victim) && HAS_BIT_ACT(victim, MOB_WIMPY)) && number_bits(1) == 0
		  && victim->points[POINTS_LIFE] < victim->points[POINTS_LIFE]/2)
		  || (HAS_BIT(victim->affected_by, AFFECT_CHARM) && victim->master
		  && victim->master->in_room != victim->in_room) )
		{
			start_fearing( victim, ch );
			stop_hunting( victim );
			send_command( victim, "flee", CO );
		}
    }

	if ( IS_PG(victim)
	  && victim->points[POINTS_LIFE] > 0
	  && victim->wimpy != 0			/* <- caso in cui il wimpy è disattivato */
	  && get_percent(victim->points[POINTS_LIFE],victim->points_max[POINTS_LIFE]) <= victim->wimpy
	  && victim->wait == 0 )
	{
		send_command( victim, "flee", CO );
	}

	tail_chain( );
	return rNONE;
}


bool is_pacifist( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "ch è NULL" );
		return TRUE;
	}

	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_PACIFIST) )
		return TRUE;

	return FALSE;
}


/*
 * Changed is_safe to have the show_msg boolian.
 * This is so if you don't want to show why you can't kill someone
 *	you can't turn it off. This is useful for things like area attacks.
 */
bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim, bool show_msg )
{
	if ( char_died(victim) || char_died(ch) )
		return TRUE;

	if ( who_fighting(ch) == ch )
		return FALSE;

	if ( !victim )		/* Gonna find this is_safe crash bug */
	{
		send_log( NULL, LOG_BUG, "is_safe: %s opponent does not exist!", ch->name );
		return TRUE;
	}

	if ( !victim->in_room )
	{
		send_log( NULL, LOG_BUG, "is_safe: %s has no physical location!", victim->name );
		return TRUE;
	}

	if ( HAS_BIT(victim->in_room->flags, ROOM_SAFE) )
	{
		if ( show_msg )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Una misteriosa aura protettiva ti impedisce di arrecare del male.\r\n" );
		}
		return TRUE;
	}

#ifdef T2_ARENA
	if ( is_pacifist(ch) && !in_arena(ch) )
#else
	if ( is_pacifist(ch) )
#endif
	{
		if ( show_msg )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( ch, "Ciò è contro le tue stesse regole, non lo farai.\r\n" );
		}
		return TRUE;
    }

#ifdef T2_ARENA
	if ( is_pacifist(victim) && !in_arena(ch) )
#else
	if ( is_pacifist(victim) )
#endif
	{
		if ( show_msg )
		{
			set_char_color( AT_MAGIC, ch );
			ch_printf( ch, "%s non ti combatterà.\r\n", capitalize(victim->short_descr) );
		}
		return TRUE;
	}

	if ( IS_PG(ch) && IS_ADMIN(ch) )
		return FALSE;

	if ( IS_PG(ch) && IS_PG(victim)
	  && ch != victim
	  && HAS_BIT(victim->in_room->area->flags, AREAFLAG_NOPKILL) )
	{
		if ( show_msg )
		{
			set_char_color( AT_ADMIN, ch );
			send_to_char( ch, "Quest'area è Sacra e non sarà turbata da spargimenti di sangue.\r\n" );
		}
		return TRUE;
	}

	if ( IS_MOB(ch) || IS_MOB(victim) )
		return FALSE;

	if ( get_level(ch) < LVL_NEWBIE )
	{
		if ( show_msg )
		{
			set_char_color( AT_WHITE, ch );
			send_to_char( ch, "Hai bisogno di maggiore esperienza.. fai pratica! \r\n" );
		}
		return TRUE;
	}

	if ( get_level(victim) < LVL_NEWBIE )
	{
		if ( show_msg )
		{
			set_char_color( AT_WHITE, ch );
			send_to_char( ch, "Ancora troppo giovane per morire.. vivi e lascia vivere, per ora.\r\n" );
		}
		return TRUE;
	}

	/* Le anime non possono iniziare un combattimento */
	if ( IS_PG(victim) && HAS_BIT_PLR(victim, PLAYER_SPIRIT) )
	{
		if ( show_msg )
		{
			set_char_color( AT_WHITE, ch );	/* (FF) AT_SOUL e gli altri tre AT_points quindi */
			ch_printf( ch, "L'anima impalpabile di %s sembra impossibile da ferire\r\n", victim->name );
		}
		return TRUE;
	}

	if ( get_timer(victim, TIMER_PKILLED) > 0 || get_timer(victim, TIMER_KILLED) > 0 )
	{
		if ( show_msg )
		{
			set_char_color( AT_GREEN, ch );	/* (FF) AT_SOUL */
			send_to_char( ch, "Attendi che riprenda perfettamente contatto con il suo corpo.\r\n" );
		}
		return TRUE;
	}

	if ( get_timer(ch, TIMER_PKILLED) > 0 || get_timer(ch, TIMER_KILLED) > 0 )
	{
		if ( show_msg )
		{
			set_char_color( AT_GREEN, ch );	/* (FF) AT_SOUL */
			send_to_char( ch, "Aspetta che la tua anima riprenda perfettamente contatto con il corpo.\r\n" );
		}
		return TRUE;
	}

	return FALSE;
}


/*
 * Set position of a victim.
 */
void update_position( CHAR_DATA *victim )
{
	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "update_position: victim è NULL." );
		return;
	}

	if ( victim->points[POINTS_LIFE] > 0 )
	{
		if ( victim->position <= POSITION_STUN )
			victim->position = POSITION_STAND;

		/* Chissà come mai è stata tolta nel codie atlantideo */
		if ( HAS_BIT(victim->affected_by, AFFECT_PARALYSIS) )
			victim->position = POSITION_STUN;
		return;
	}

	if ( IS_MOB(victim) || victim->points[POINTS_LIFE] <= -10 )
	{
		if ( victim->mount )
		{
			act( AT_ACTION, "$n cade da $N.", victim, NULL, victim->mount, TO_ROOM );
			REMOVE_BIT( victim->mount->act, MOB_MOUNTED );
			victim->mount = NULL;
		}
		victim->position = POSITION_DEAD;
		return;
	}

	if 		( victim->points[POINTS_LIFE] <= -6 )	victim->position = POSITION_MORTAL;
	else if ( victim->points[POINTS_LIFE] <= -3 )	victim->position = POSITION_INCAP;
	else										victim->position = POSITION_STUN;

	if ( victim->position > POSITION_STUN && HAS_BIT(victim->affected_by, AFFECT_PARALYSIS) )
		 victim->position = POSITION_STUN;

	if ( victim->mount )
	{
		act( AT_ACTION, "$n cade priv$x di coscienza da $N.", victim, NULL, victim->mount, TO_ROOM );
		REMOVE_BIT( victim->mount->act, MOB_MOUNTED );
		victim->mount = NULL;
	}
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	FIGHT_DATA *fight;

	if ( ch->fighting )
	{
		send_log( NULL, LOG_BUG, "set_fighting: %s -> %s (already fighting %s)",
			ch->name, victim->name, ch->fighting->who->name );
		return;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_SLEEP) )
		affect_strip( ch, gsn_sleep );

#ifdef T2_ALFA
	/* Limit attackers (BB) baco, pare che ad uno gli sia saltato fuori il msg nonostante abbia attaccato lui solo un mob */
	if ( space_to_fight(ch, victim) == FALSE )
	{
		send_to_char( ch, "Combattimento troppo confuso.. Non posso unirmi alla rissa!\r\n" );
		return;
	}
#endif

	CREATE( fight, FIGHT_DATA, 1 );
	fight->who	 = victim;
	fight->xp	 = (int) xp_compute( ch, victim ) * 0.85;
	fight->align = align_compute( ch, victim );

	if ( IS_PG(ch) && IS_MOB(victim) )
		fight->timeskilled = times_killed(ch, victim);

	ch->num_fighting = 1;
	ch->fighting = fight;

	if ( IS_MOB(ch) )
		ch->position = POSITION_FIGHT;
	else
	{
		switch ( ch->style )
		{
		  case STYLE_EVASIVE:		ch->position = POSITION_EVASIVE;		break;
		  case STYLE_DEFENSIVE:		ch->position = POSITION_DEFENSIVE;		break;
		  case STYLE_AGGRESSIVE:	ch->position = POSITION_AGGRESSIVE;		break;
		  case STYLE_BERSERK:		ch->position = POSITION_BERSERK;		break;
		  default:					ch->position = POSITION_FIGHT;
		}
	}

	victim->num_fighting++;
	if ( victim->switched && HAS_BIT(victim->switched->affected_by, AFFECT_POSSESS) )
	{
		send_to_char( victim->switched, "Qualcosa ti disturba!\r\n" );
		send_command( victim->switched, "return", CO );
	}
}


CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "who_fighting: ch è NULL." );
		return NULL;
	}

	if ( !ch->fighting )
		return NULL;

	return ch->fighting->who;
}


void free_fight( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "free_fight: ch è NULL." );
		return;
	}

	if ( ch->fighting )
	{
		if ( !char_died(ch->fighting->who) )
			--ch->fighting->who->num_fighting;
		DISPOSE( ch->fighting );
	}

	ch->fighting = NULL;
	if ( ch->mount )
		ch->position = POSITION_MOUNT;
	else
		ch->position = POSITION_STAND;

	/* Berserk wears off after combat. */
	if ( HAS_BIT(ch->affected_by, AFFECT_BERSERK) )
	{
		affect_strip( ch, gsn_berserk );
		set_char_color( AT_WEAROFF, ch );
		ch_printf( ch, "%s\r\n", table_skill[gsn_berserk]->msg_off );
	}
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
	CHAR_DATA *fch;

	free_fight( ch );
	update_position( ch );

	if ( !fBoth )   /* major short cut here */
		return;

	for ( fch = first_char;  fch;  fch = fch->next )
	{
		if ( who_fighting(fch) == ch )
		{
			free_fight( fch );
			update_position( fch );
#ifdef T2_MSP
			stop_music_combat( fch->desc );
#endif
		}
	}
#ifdef T2_MSP
	stop_music_combat( ch->desc );
#endif
}


void death_cry( CHAR_DATA *ch )
{
	ROOM_DATA	*was_in_room;
	char		*msg;
	EXIT_DATA	*pexit;
	VNUM		 vnum;
	int			 part;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "death_cry: ch è NULL." );
		return;
	}

	vnum = 0;
	msg = NULL;

	switch ( number_range(0, 5) )
	{
	  default: msg = "Un gemito di dolore giunge al tuo udito.";					break;
	  case  0: msg = "$n emette il suo ultimo respiro.. cadendo esanime a terra.";	break;
	  case  1: msg = "$n cade a terra.. mort$x.";									break;
	  case  2: msg = "$n sbarra gli occhi mentre esala il suo ultimo respiro..";	break;
      case  3: msg = "$n cade a terra in un lago di sangue.";						break;
      case  4: msg = "$n si contorce per terra esalando il suo ultimo respiro.";	break;
      case  5:
		if ( IS_EMPTY(ch->xflags) )
	    	break;

		part = number_range( 0, MAX_PART-1 );
		if ( HAS_BODYPART(ch, part) )
		{
			if ( number_range(0, 1) )
				msg  = table_part[part].msg1;
			else
				msg  = table_part[part].msg2;

			vnum = table_part[part].vnum;
			break;
		}
		else
			msg = "Odi un atroce lamento di morte.";	/* (MG) fare altri messaggi di morte generici magari */

		break;
	} /* chiude lo switch */

	act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );

	if ( vnum )
	{
		char	  buf[MSL];
		OBJ_DATA *obj;
		char	 *name;

		if ( !get_obj_proto(NULL, vnum) )
		{
			send_log( NULL, LOG_BUG, "death_cry: vnum non valido." );
			return;
		}

		name	   = IS_MOB(ch)  ?  ch->short_descr  :  ch->name;
		obj		   = make_object( get_obj_proto(NULL, vnum), 0 );
		obj->timer = number_range( 4, 7 );

		if ( HAS_BIT(ch->affected_by, AFFECT_POISON) && obj->food )
			obj->food->poison = 10;	/* (TT) (FF) Teoricamente è food, ma ci sono pezzi di cadavere anche trash e cook e altro */

		sprintf( buf, obj->short_descr, name );
		DISPOSE( obj->short_descr );
		obj->short_descr = str_dup( buf );

		sprintf( buf, obj->long_descr, name );
		DISPOSE( obj->long_descr );
		obj->long_descr = str_dup( buf );

		obj = obj_to_room( obj, ch->in_room );
	}

	if ( IS_MOB(ch) )
		msg = "Odi un atroce lamento di morte..";
	else
		msg = "Odi un atroce lamento di morte. Piccole lacerazioni astrali si formano tutt'attorno..";

	was_in_room = ch->in_room;
	for ( pexit = was_in_room->first_exit;  pexit;  pexit = pexit->next )
	{
		if ( !pexit->to_room )
			continue;

		if ( pexit->to_room != was_in_room )
		{
			ch->in_room = pexit->to_room;
			act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );	/* (FF) qui bisognerebbe fare una procedura apposita con un TO_AROUND */
		}
	}

	ch->in_room = was_in_room;
}


/*
 * Calcola la penalità di una morte sulla base del tipo di morte
 */
void death_penalty( CHAR_DATA *ch, const int type )
{
	int	x;
	int count;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "death_penalty: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( type == DEATH_TRAP )
	{
		send_log( NULL, LOG_DEATH, "death_penalty: %s cade in una trappola mortale.", ch->name );
		ch->gold = 0;	/* Azzera le monete d'oro */
	}

	/* Diminuisce, come handicap di morte, le skill */
	count = 0;
	for ( x = 0;  x < top_sn;  x++ )
	{
		int val;

		if ( !table_skill[x] )					continue;
		if ( !VALID_STR(table_skill[x]->name) )	continue;
		if ( ch->pg->learned_skell[x] == 0 )	continue;

		/* Maggiore è la spiritualità di un pg e maggiore è la sua forza nel superare i traumi da morte */
		if ( number_range(0, 100) + get_curr_attr(ch, ATTR_SPI) < 101 )
		{
			val = number_range( 0, 3 );
			ch->pg->learned_skell[x] -= val;
			count += val;
		}
	}
	if ( count > 0 )
		send_log( NULL, LOG_DEATH, "death_penalty: %s nella morte perde %d punti in skill", ch->name, count );

	/* Diminuisce i punti */
	count = 0;
	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		int val;

		/* 50 è il tetto minimo per tutti, poi c'è il livello */
		if ( ch->points_max[x] > 50 && ch->points_max[x] > get_level(ch)
		  && number_range(0, 100) + get_curr_attr(ch, ATTR_SPI) < 101 )
		{
			val = number_range( 0, get_level(ch)/5 );
			ch->points_max[x] -= val;
			count += val;
		}
	}
	if ( count > 0 )
		send_log( NULL, LOG_DEATH, "death_penalty: %s nella morte perde %d in punteggi", ch->name, count );

	/* Imposta i punti attuali */
	for ( x = 0;  x < MAX_POINTS;  x++ )
		ch->points[x] = number_range( get_level(ch)/4 + 1, UMAX(1, ch->points_max[x]) );

	/* Diminuisce gli attributi */
	count = 0;
	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		int val;

		/* tetto minimo a 15 */
		if ( ch->attr_perm[x] > 15 && number_range(0, 100) + get_curr_attr(ch, ATTR_SPI) < 101 )
		{
			val = number_range( 0, 2 );
			ch->attr_perm[x] -= val;
			count += val;
		}
	}
	if ( count > 0 )
		send_log( NULL, LOG_DEATH, "death_penalty: %s nella morte perde %d in attributi", ch->name, count );

	/* Diminuisce anche la gloria */
	if ( ch->pg->glory > LVL_LEGEND*2 )
	{
		count = number_range( 0, ch->level*2 );
		ch->pg->glory -= count;
		if ( count > 0 )
			send_log( NULL, LOG_DEATH, "death_penalty: %s nella morte perde %d in gloria", ch->name, count );
	}

	worsen_mental_state( ch, 2 );
	exp_death_hole( ch );
}


void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int	x;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "raw_kill: ch è NULL" );
		return;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "raw_kill: victim è NULL" );
		return;
	}

	stop_fighting( victim, TRUE );

	/* Take care of morphed characters */
	if ( victim->morph )
	{
		do_unmorph_char( victim );
		raw_kill( ch, victim );
		return;
	}

	mprog_death_trigger( ch, victim );
	if ( char_died(victim) )
		return;

/*	death_cry( victim ); */

	rprog_death_trigger( ch, victim );
	if ( char_died(victim) )
		return;

	make_corpse( victim, ch );
	if ( victim->in_room->sector == SECTOR_OCEANFLOOR
	  || victim->in_room->sector == SECTOR_UNDERWATER
	  || victim->in_room->sector == SECTOR_WATER_SWIM
	  || victim->in_room->sector == SECTOR_WATER_NOSWIM )
		act( AT_DRED, "Il sangue di $n cosparge l'acqua intorno del suo colore..", victim, NULL, NULL, TO_ROOM );
	else if ( victim->in_room->sector == SECTOR_AIR )
		act( AT_DRED, "Il sangue di $n gocciola cadendo nell'aria..", victim, NULL, NULL, TO_ROOM );
	else
		make_blood( victim );

	if ( IS_MOB(victim) )
	{
		if ( victim->desc )
		{
			send_to_char( victim, "La vittima in cui eri in switch è morta.\r\n" );
			send_command( victim, "return", CO );
		}

		victim->pIndexData->killed++;
		extract_char( victim, TRUE );
		victim = NULL;
		return;		/* Se è mob esce dalla funzione qui */
	}

	set_char_color( AT_DIEMSG, victim );
	send_command( victim, "help _MSG_DIE_", CO );

#ifdef T2_MSP
	if ( victim->sex == SEX_FEMALE )
		send_audio( victim->desc, "deathm.wav", TO_ROOM );
	else
		send_audio( victim->desc, "deathf.wav", TO_ROOM );
#endif

	extract_char( victim, FALSE );

	/* Flag di anima appena uccisa */
	SET_BIT( victim->pg->flags, PLAYER_SPIRIT );

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "raw_kill: oops! extract_char destroyed pc char" );
		return;
	}

	while ( victim->first_affect )
		affect_remove( victim, victim->first_affect );

	SET_BITS( victim->affected_by, table_race[victim->race]->affected );
	CLEAR_BITS( victim->resistant );
	CLEAR_BITS( victim->susceptible );
	CLEAR_BITS( victim->immune );
	victim->carry_weight	= 0;
	victim->armor			= 100;
	victim->armor		   += table_race[victim->race]->ac_plus;
	SET_BITS( victim->attacks, table_race[victim->race]->attacks );
	SET_BITS( victim->defenses, table_race[victim->race]->defenses );
	for ( x = 0;  x < MAX_ATTR;  x++ )
		victim->attr_mod[x] = 0;
	for ( x = 0;  x < MAX_SENSE;  x++ )
		victim->sense_mod[x] = 0;
	victim->damroll			= 0;
	victim->hitroll			= 0;
	victim->mental_state	= -10;

	/* (FF) sarebbe da lavorarci su questo valore di align */
	victim->alignment		= URANGE( -1000, victim->alignment, 1000 );

	for ( x = 0;  x < MAX_SAVETHROW;  x++ )
		victim->saving_throw[x] = table_race[victim->race]->saving_throw[x];

	victim->position = POSITION_REST;

	death_penalty( victim, DEATH_NORMAL );

	/*
	 * Pardon crimes...
	 */
	if ( HAS_BIT_PLR(victim, PLAYER_KILLER) || HAS_BIT_PLR(victim, PLAYER_THIEF) )
	{
		/* Qui anche se rimuove una flag che non c'era, è lo stesso */
		REMOVE_BIT( victim->pg->flags, PLAYER_KILLER );
		REMOVE_BIT( victim->pg->flags, PLAYER_THIEF );

		send_to_char( victim, "Mi sento più leggero, forse avrò pagato per i miei crimini?\r\n" );
	}

	victim->pg->condition[CONDITION_FULL]	= 12;
	victim->pg->condition[CONDITION_THIRST]	= 12;

	save_player( victim );

	/* Mette una pausa per la morte nei pkilling */
	if ( IS_PG(ch) && IS_PG(victim) )
	{
		send_to_char( victim, "Una forza magica blocca i tuoi movimenti!\r\n" );
		WAIT_STATE( victim, PULSE_IN_SECOND * 10 );
	}
}


/*
 * Gestisce il comando do_kill e do_murder
 * (FF) argument servirà in futuro per colpire una determinata parte del corpo oppure per il multitargeting
 */
DO_RET kill_handler( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "kill_handler: ch è NULL" );
		return;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "kill_handler: victim è NULL" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Colpirsi da soli.. non è una grande idea!\r\n" );
		ch->points[POINTS_LIFE] -= ch->level;	/* si autolesiona */
		return;
	}

	if ( is_safe(ch, victim, TRUE) )
		return;

	if ( HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		if ( ch->master == victim )
		{
			act( AT_PLAIN, "$N è la mia guida.", ch, NULL, victim, TO_CHAR );
			return;
		}
		else if ( IS_PG(victim) )
		{
			if ( ch->master && IS_PG(ch->master) )
				SET_BIT( ch->master->pg->flags, PLAYER_ATTACKER );
		}
	}

	if ( IS_MOB(victim) && victim->morph )
	{
		send_to_char( ch, "E' davvero una strana creatura.. Osservala attentamente prima di attaccarla.\r\n" );
		return;
	}

	if ( ch->position == POSITION_FIGHT
	  || ch->position == POSITION_EVASIVE
	  || ch->position == POSITION_DEFENSIVE
	  || ch->position == POSITION_AGGRESSIVE
	  || ch->position == POSITION_BERSERK )
	{
		send_to_char( ch, "E' quello che stai cercando di fare!\r\n" );	/* (FF) inserire qui il sistema di multitargeting */
		return;
	}
#ifdef T2_MSP
	send_music_combat( ch, victim );
#endif
	WAIT_STATE( ch, PULSE_VIOLENCE );

	if ( IS_PG(victim) )
	{
#ifdef T2_ARENA
		char	buf[MSL];

		if ( in_arena(ch) )
		{
			sprintf( buf, "yell %s Avrai pane per i tuoi denti!", IS_MOB(ch) ? ch->short_descr : ch->name );
			send_command( victim, buf, CO );
		}
		else
#endif
			send_command( victim, "yell Aiuto! Mi stanno attaccando!", CO );

		send_log( NULL, LOG_PK, "%s: murder %s.", ch->name, victim->name );
		is_illegal_pk( ch, victim );
	}

	check_attacker( ch, victim );
	multi_hit( ch, victim, TYPE_UNDEFINED );
}

/*
 * Comando per killare mob
 */
DO_RET do_kill( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Chi vorresti uccidere?\r\n" );
		return;
	}

	victim = get_mob_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non si trova qui.\r\n" );
		return;
	}

	kill_handler( ch, victim, argument );
}

/*
 * Comando per killare pg
 */
DO_RET do_murder( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Chi vorresti uccidere?\r\n" );
		return;
	}

	victim = get_char_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non è qui.\r\n" );
		return;
	}

	kill_handler( ch, victim, argument );
}


/*
 * Ritorna vero se ch può killare victim
 * Se viene passata victim NULL controlla se ch può genericamente killare
 * Prima deve eseguire i check con ritorno a TRUE, poi tutti gli altri
 */
bool can_pkill( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "can_pkill: ch è NULL" );
		return FALSE;
	}

	if ( IS_MOB(ch) )
		return TRUE;

	if ( victim )
	{
		if ( ch == victim )
			return TRUE;
		if ( IS_MOB(victim) )
			return TRUE;

		if ( get_level(victim) < LVL_NEWBIE )
			return FALSE;
		if ( HAS_BIT(victim->in_room->area->flags, AREAFLAG_NOPKILL) )
			return FALSE;
	}

	if ( get_level(ch) < LVL_NEWBIE )
		return FALSE;
	if ( HAS_BIT(ch->in_room->area->flags, AREAFLAG_NOPKILL) )
		return FALSE;

	return TRUE;
}


/* (TT) In skill.c c'è una funzione dal nome is_legal_kill, da vedere a che serve, forse è doppia */
bool is_illegal_pk( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char	buf [MSL];
	char	buf2[MSL];

	if ( IS_PG(victim) && IS_PG(ch)
	  && get_level(ch) - get_level(victim) > 20
#ifdef T2_ARENA
	  && !in_arena(ch)
#endif
	  && ch != victim
	  && !(IS_ADMIN(ch) && IS_ADMIN(victim)) )
	{
		if ( IS_MOB(ch) )
			sprintf( buf, " (%s)", ch->name );
		if ( IS_MOB(victim) )
			sprintf( buf2, " (%s)", victim->name );

		send_log( NULL, LOG_PK, "&W***&rILLEGAL PKILL&W***&w on %s%s in  attempt at %d",
			(IS_MOB(victim))  ?  victim->short_descr  :  victim->name,
			(IS_MOB(victim))  ?  buf2  :  "",
			victim->in_room->vnum );

		last_pkroom = victim->in_room->vnum;

		return TRUE;
	}

	return FALSE;
}


/*
 * Modificata da accettare un valore in percentuale e non la quantità di vita.
 */
DO_RET do_wimpy( CHAR_DATA *ch, char *argument )
{
	int		wimpy;

	set_char_color( AT_YELLOW, ch );

	if ( !VALID_STR(argument) )
	{
		ch->wimpy = 20;
		send_to_char( ch, "Fuggirò se ci sarà pericolo per la mia vita.\r\n" );
		return;
	}

	if ( !is_number(argument) )
	{
		send_to_char( ch, "Non posso pensare di fuggire ad una percentuale di vita non numerica.\r\n" );
		return;
	}

	wimpy = atoi( argument );
	if ( wimpy == 0 )
	{
		ch->wimpy = 0;
		send_to_char( ch, "Non fuggirò di fronte a nulla e nessuno!\r\n" );
		return;
	}

	if ( wimpy > 85 )
	{
		ch->wimpy = 85;
		send_to_char( ch, "Il mio coraggio eccede la mia saggezza.\r\n" );
		return;
	}

#ifdef T2_ALFA
	if ( ch->class == CLASS_SAMURAI && wimpy > 60 )
	{
		ch->wimpy = 60;
		send_to_char( ch, "Dovrei farmela così sotto? Io??\r\n" );
		return;
	}
#endif

	ch->wimpy = wimpy;
	ch_printf( ch, "La mia fuga è pronta quando mi sento al %d percento delle mie possibilità vitali.\r\n", wimpy );
}


DO_RET do_flee( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*was_in;
	ROOM_DATA	*now_in;
	EXIT_DATA	*pexit;
	int			 attempt;
	int			 los;
	int			 door;

	if ( !who_fighting(ch) )
	{
		if ( ch->position == POSITION_FIGHT
		  || ch->position == POSITION_EVASIVE
		  || ch->position == POSITION_DEFENSIVE
		  || ch->position == POSITION_AGGRESSIVE
		  || ch->position == POSITION_BERSERK )
		{
			if ( ch->mount )
				ch->position = POSITION_MOUNT;
			else
				ch->position = POSITION_STAND;
		}
		send_to_char( ch, "Non stai combattendo.\r\n" );
		return;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_BERSERK) )
	{
		send_to_char( ch, "La rabbia pervade il tuo corpo.. non puoi fuggire, non ora.. SANGUE!\r\n" );
		return;
	}

	if ( ch->points[POINTS_MOVE] <= 0 && !stop_lowering_points(ch, POINTS_MOVE) )
	{
		send_to_char( ch, "Troppa stanchezza per scappare!\r\n" );
		return;

	}

	/* No fleeing while more aggressive than normal style or hurt. */
	if ( IS_PG(ch) && ch->position < POSITION_FIGHT )
	{
		send_to_char( ch, "Fuggire? No! Il mio stile è troppo combattivo!\r\n" );
		return;
	}

	if ( IS_MOB( ch ) && ch->position <= POSITION_SLEEP )
		return;

	was_in = ch->in_room;
	for ( attempt = 0; attempt < 8; attempt++ )
	{
		door = number_door( );
		if ( (pexit = get_exit(was_in, door)) == NULL
		  || !pexit->to_room
		  ||  HAS_BIT(pexit->flags, EXIT_NOFLEE)
		  || (HAS_BIT(pexit->flags, EXIT_CLOSED)
		  && !HAS_BIT(ch->affected_by, AFFECT_PASS_DOOR))
		  || (IS_MOB(ch)
		  && HAS_BIT(pexit->to_room->flags, ROOM_NOMOB)) )
		{
			continue;
		}

		affect_strip( ch, gsn_sneak );
		REMOVE_BIT ( ch->affected_by, AFFECT_SNEAK );

		if ( ch->mount && ch->mount->fighting )
			stop_fighting( ch->mount, TRUE );

		move_char( ch, pexit, 0, FALSE );

		if ( (now_in = ch->in_room) == was_in )
			continue;

		ch->in_room = was_in;
		act( AT_FLEE, "$n fugge a gambe levate", ch, NULL, NULL, TO_ROOM );
		ch->in_room = now_in;
		act( AT_FLEE, "$n cerca delle tracce sul terreno.", ch, NULL, NULL, TO_ROOM );

		if ( IS_PG(ch) )
		{
			act( AT_FLEE, "Fuggi a gambe levate!", ch, NULL, NULL, TO_CHAR );

			los = get_level(ch) / 5;
#ifdef T2_ARENA
			if ( in_arena(ch) )
				los = 0;
#endif
			gain_exp( ch, 0 - los, FALSE );
		}

		stop_fighting( ch, TRUE );
#ifdef T2_MSP
		if ( get_percent(ch->points[POINTS_LIFE], ch->points_max[POINTS_LIFE]) <= 5 )
			send_audio( ch->desc, "flee.mid", TO_CHAR );
#endif
		return;
	}

	act( AT_FLEE, "Non riesci a cogliere l'attimo giusto per la fuga!", ch, NULL, NULL, TO_CHAR );

	los = get_level(ch) / 10;
#ifdef T2_ARENA
	if ( in_arena(ch) )
		los = 0;
#endif
	if ( number_bits(3) == 1 )
		gain_exp( ch, 0 - los, FALSE );
}
