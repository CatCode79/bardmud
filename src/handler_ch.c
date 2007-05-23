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
 >				Modulo principale di manipolazione struttura				<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "build.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "group.h"
#include "interpret.h"
#include "movement.h"
#include "mprog.h"
#include "msp.h"
#include "nanny.h"
#include "olc.h"
#include "morph.h"
#include "room.h"
#include "timer.h"


/*
 * Variabili locali
 */
CHAR_DATA	*cur_char;
ROOM_DATA	*cur_room;
bool		 cur_char_died;
ch_ret		 global_retcode;


/*
 * Tabella codici della flag di entità, cioè sia per mob che per pg
 */
const CODE_DATA code_enflag[] =
{
	{ ENFLAG_FLYCHANGED,	"ENFLAG_FLYCHANGED",	"flychanged" }
};
const int max_check_enflag = sizeof(code_enflag)/sizeof(CODE_DATA);


/*
 * Controlla se effettivamente il mob-pg sia un mob.	( ex IS_NPC )
 */
bool IS_MOB( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "IS_MOB: ch è NULL" );
		return TRUE;	/* Meglio considerarlo mob? Forse sì */
	}

	/* Se ha la struttura del pg nulla allora è un mob*/
	if ( !ch->pg )
		return TRUE;

	return FALSE;
}


/*
 * Controlla se l'entità è un pg
 */
bool IS_PG( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "IS_PG: ch è NULL" );
		return FALSE;
	}

	/* Se la struttura pg non è NULL allora è un pg */
	if ( ch->pg )
		return TRUE;

	return FALSE;
}


/*
 * Description functions.	( ex macro omonime)
 */
char *PERS( CHAR_DATA *ch, CHAR_DATA *looker )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "PERS: ch è NULL" );
		return "qualcuno";
	}

	if ( !looker )
	{
		send_log( NULL, LOG_BUG, "PERS: looker è NULL" );
		return "qualcuno";
	}

	if ( can_see(looker, ch) )
	{
		if ( IS_MOB(ch) )
			return ch->short_descr;

		return ch->name;
	}

	return "qualcuno";
}

char *MORPHPERS( CHAR_DATA *ch, CHAR_DATA *looker )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "MORPHPERS: ch è NULL" );
		return "qualcuno";
	}

	if ( !looker )
	{
		send_log( NULL, LOG_BUG, "MORPHPERS: looker è NULL" );
		return "qualcuno";
	}

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_BUG, "MORPHPERS: i mob non possono morphare: %s", ch->short_descr );
		return "qualcuno";
	}

	if ( can_see(looker, ch) )
		return ch->morph->morph->short_desc;

	return "qualcuno";
}


bool HAS_BIT_PLR( CHAR_DATA *ch, int bit )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "HAS_BIT_PLR: ch è NULL" );
		return FALSE;
	}

	if ( bit < 0 && bit >= MAX_PLAYER )
	{
		send_log( NULL, LOG_BUG, "HAS_BIT_PLR: bit errato: %d", bit );
		return FALSE;
	}

	ch = get_original( ch );

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_BUG, "HAS_BIT_PLR: ch è un mob" );
		return FALSE;
	}

#if 0
	/* (FF) aggiungere poi man mano le altre PLAYER_ di admin se si farà il test, ora che ci penso per evitare sto casino sarebbe il caso in fase di salvataggio di rimuovere queste flag */
	if ( get_trust(ch) == TRUST_PLAYER
	  && (bit == PLAYER_NOADMIN || bit == PLAYER_FLAG || bit == PLAYER_VNUM || PLAYER_ALLEXITS) )
	{
		send_log( NULL, LOG_BUG, "HAS_BIT_PLR: check di un bit per admin senza trust opportuna per %s", ch->name );
	}
#endif

	if ( HAS_BIT(ch->pg->flags, bit) )
		return TRUE;

	return FALSE;
}


bool HAS_BIT_ACT( CHAR_DATA *ch, int bit )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "HAS_BIT_ACT: ch è NULL" );
		return FALSE;
	}

	/* E' meglio che non entrino qui i pg, e che i controlli sugli stessi vengano
	 *	fatti nel codice prima di questa funzione */
	if ( IS_PG(ch) )
		send_log( NULL, LOG_BUG, "HAS_BIT_ACT: ch è un pg" );

	if ( bit < 0 && bit >= MAX_MOB )
	{
		send_log( NULL, LOG_BUG, "HAS_BIT_ACT: bit errato: %d", bit );
		return FALSE;
	}

	 if ( IS_MOB(ch) && HAS_BIT(ch->act, bit) )
		return TRUE;

	return FALSE;
}


/*
 * Ritorna il nome dell'avventuriero.
 */
char *get_name( CHAR_DATA *ch )
{
	int len;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_name: ch è NULL" );
		return "Sconosciuto";
	}

	if ( IS_MOB(ch) )
		return ch->short_descr;

	len = strlen( ch->name );
	if ( len < MIN_LEN_NAME_PG || len > MAX_LEN_NAME_PG )
	{
		send_log( NULL, LOG_BUG, "get_name: lunghezza del nome del pg %s errata: %d", ch->name, len );
		return "Sconosciuto";
	}

	return ch->name;
}


/*
 * Ritorna vero se il descrittore sta giocando o editando
 */
bool is_in_game( DESCRIPTOR_DATA *d )
{
	if ( d->connected == CON_PLAYING )
		return TRUE;

	if ( is_editing(d) )
		return TRUE;

	if ( is_in_olc(d) )
		return TRUE;

	return FALSE;
}


/*
 * Ritorna il ch originale passando il descrittore.
 */
CHAR_DATA *ch_original( DESCRIPTOR_DATA *d )
{
	if ( !d )
	{
		send_log( NULL, LOG_BUG, "ch_original: d è NULL" );
		return NULL;
	}

	if ( d->original )
		return d->original;

	return d->character;
}


/*
 * Ritorna il ch originale nel caso che sia switchato
 */
CHAR_DATA *get_original( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_original: ch è NULL" );
		return NULL;
	}

	if ( ch->desc && ch->desc->original )
		return ch->desc->original;

	return ch;
}


/*
 * Funzione che controlla se si possiede il grado-fiducia per essere implementor.
 */
bool IS_ADMIN( CHAR_DATA *ch )
{
	int	trust;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "IS_ADMIN: ch è NULL" );
		return FALSE;
	}

	if ( IS_MOB(ch) )
		return FALSE;

	if ( HAS_BIT_PLR(ch, PLAYER_NOADMIN) )
		return FALSE;

	trust = get_trust( ch );
	if ( trust >= TRUST_NEOMASTER && trust <= TRUST_IMPLE )
		return TRUE;

	return FALSE;
}


/*
 * Funzione sul controllo dell'allineamento.
 */
bool is_align( CHAR_DATA *ch, int type )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_align: ch è NULL" );
		return FALSE;
	}

	/* ALIGN_NONE non bisogna passarlo a questa funzione, ma serve per dei check sul sistema di restrizioni */
	if ( type < ALIGN_GOOD && type >= MAX_ALIGN )
	{
		send_log( NULL, LOG_BUG, "is_align: tipo di allineamento errato: %d", type );
		return FALSE;
	}

	if ( ch->alignment < -1000 &&  ch->alignment > 1000 )
	{
		send_log( NULL, LOG_BUG, "is_align: valore allineamento errato per il pg %s: %d",
			ch->name, ch->alignment );
		return FALSE;
	}

	/* (FF) magari aggiungere altri gradi di allineamento */

	if ( ch->alignment >= 350 && type == ALIGN_GOOD )
		return TRUE;

	if ( ch->alignment > -350 && ch->alignment < 350 && type == ALIGN_NEUTRAL )
		return TRUE;

	if ( ch->alignment <= -350 && type == ALIGN_EVIL )
		return TRUE;

	return FALSE;
}

/*
 * Ritorna la stringa corrispondente al valore di allineamento posseduto
 */
char *get_align_alias( const int align )
{
	if ( align < -1000 || align > 1000 )
	{
		send_log( NULL, LOG_BUG, "get_align_alias: align passato errato: %d", align );
		return "";
	}

	if		( align >  900 )	return "&Wangelico";
	else if ( align >  700 )	return "&Cnobile";
	else if ( align >  350 )	return "&Conorabile";
	else if ( align >  100 )	return "&Gamabile";
	else if ( align > -100 )	return "&wneutrale";
	else if ( align > -350 )	return "&zirritante";
	else if ( align > -700 )	return "&rignobile";
	else if ( align > -900 )	return "&rdiabolico";
    else						return "&Rdemoniaco";
}


/*
 * Controlla se il pg è ubriaco
 */
bool is_drunk( CHAR_DATA *ch, int drunk )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_drunk: ch è NULL" );
		return FALSE;
	}

	if ( number_percent() < (ch->pg->condition[CONDITION_DRUNK]*2/drunk) )
		return TRUE;

	return FALSE;
}


/*
 * Ritorna l'attributo corrente del pg
 */
int get_curr_attr( CHAR_DATA *ch, int attribute )
{
	int value;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_curr_attr: ch è NULL" );
		return 30;
	}

	/* Controlla la validità del valore passato */
	if ( attribute < 0 || attribute >= MAX_ATTR )
	{
		send_log( NULL, LOG_BUG, "get_curr_attr: valore attribute passato errato: %d", attribute );
		return 30;
	}

	value = ch->attr_perm[attribute] + ch->attr_mod[attribute];
	value += table_race[ch->race]->attr_plus[attribute];	/* Ci aggiunge i bonus/malus razziale dell'attributo */

	return URANGE( 1, value, MAX_LEARN_ATTR );
}


/*
 * Ritorna il totale possibile per l'aumento degli attributi
 * Se il valore è zero o negativo allora ha già raggiunto il massimo
 */
int check_total_attr( CHAR_DATA *ch )
{
	int		total = 0;	/* totale del pg */
	int		max = 0;	/* massimo di default */
	int		x;			/* contatore */

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_total_attr: ch è NULL" );
		return 0;
	}

	for ( x = 0;  x < MAX_ATTR;  x++ )
		total += ch->attr_perm[x];

	max = 75 * MAX_ATTR;

	/* Se il totale è troppo alto, avverte l'admin */
/*	if ( total-80 > max )
		send_log( NULL, LOG_WARNING, "check_total_attr: troppo alto (%d) per pg %s", total, ch->name );*/

	return max - total;
}


int get_curr_sense( CHAR_DATA *ch, int sense )
{
	int	value;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_sense_curr: ch è NULL" );
		return 30;
	}

	/* Controlla la validità del senso passato */
	if ( sense < 0 || sense >= MAX_SENSE )
	{
		send_log( NULL, LOG_BUG, "get_sense_curr: valore sense del pg %s errato: %d", ch->name, sense );
		return 30;
	}

	value = ch->sense_perm[sense] + ch->sense_mod[sense];
	value += table_race[ch->race]->sense_plus[sense];	/* Ci aggiunge il bonus/malus di razza del senso */

	return URANGE( 1, value, MAX_LEARN_ATTR );
}


/*
 * Ritorna il totale possibile per l'aumento dei sensi
 * Se il valore è zero o negativo allora ha già raggiunto il massimo
 */
int check_total_sense( CHAR_DATA *ch )
{
	int		total = 0;	/* totale del pg */
	int		max = 0;	/* massimo di default */
	int		x;			/* contatore */

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_total_sense: ch è NULL" );
		return 0;
	}

	for ( x = 0;  x < MAX_SENSE;  x++ )
		total += ch->sense_perm[x];

	max = 70 * MAX_SENSE;

	/* Se il totale è troppo alto, avverte l'admin */
/*	if ( total-40 > max )
		send_log( NULL, LOG_WARNING, "check_total_sense: troppo alto (%d) per pg %s", total, ch->name );*/

	return max - total;
}


/*
 * Ritorna il livello del pg dandogli come massimo il valore 100
 * Nei casi in cui si voglia il valore reale bisogna ricavarlo con ch->level
 */
int get_level( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_level: ch è NULL" );
		return 1;
	}

	if ( ch->level <= 0 || ch->level > LVL_LEGEND )
	{
		send_log( NULL, LOG_BUG, "get_level: livello di %s errato: %d", ch->name, ch->level );
		return 1;
	}

	if ( ch->level > LVL_LEGEND )
	{
		if ( IS_MOB(ch) )
			send_log( NULL, LOG_BUG, "get_level: livello maggiore di %d per il mob %d", LVL_LEGEND, ch->pIndexData->vnum );

		return LVL_LEGEND;
	}

	return ch->level;
}


/*
 * Ritorna la trust del pg
 */
int get_trust( CHAR_DATA *ch )
{
	CHAR_DATA *och;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_trust: ch è NULL" );
		return TRUST_PLAYER;
	}

	if ( IS_MOB(ch) )
		return TRUST_PLAYER;

	och = get_original( ch );

	if ( och->pg->trust < 0 || och->pg->trust > TRUST_IMPLE )
	{
		send_log( NULL, LOG_BUG, "get_trust: variabile trust di %s errata: %d", och->name, och->pg->trust );
		return TRUST_PLAYER;
	}

	return och->pg->trust;
}


/*
 * Ritorna la AC naturale del pg
 * Ricordandosi che l'AC più è negativa e più difende
 * (FF) Da aggiungere o fare meglio altre posizioni?
 */
int get_ac_natural( CHAR_DATA *ch )
{
	int	ac;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_ac_natural: ch è NULL" );
		return 0;
	}

	ac = table_attr_bonus[get_curr_attr(ch, ATTR_AGI)].defense;

	switch ( ch->position )
	{
	  case POSITION_MORTAL:
	  case POSITION_INCAP:
	  case POSITION_STUN:
/*	  case POSITION_DREAM: (GG) */
	  case POSITION_SLEEP:
		if ( ac < 0 )
			ac /= 3;
		else
			ac *= 3;
		break;

	  case POSITION_REST:
		if ( ac < 0 )
			ac /= 2;
		else
			ac *= 2;
		break;

	  case POSITION_SIT:
/*	  case POSITION_KNEE: (GG) */
		if ( ac < 0 )
			ac /= 3/2;
		else
			ac *= 2/3;
	}

	/* Qui arriva intatta nei casi di posizione: stand, stili-posizioni, mount, shove e drag
	 *	death vabbè è inutile considerarla :P */
	return ac;
}


/*
 * Ritorna l'armatura del pg.
  */
int get_ac( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_ac: ch è NULL" );
		return 0;
	}

	return ( ch->armor + get_ac_natural(ch) );
}


/*
 * Ritornano l'HitRoll e il DamRoll
 */
int get_hitroll( CHAR_DATA *ch )
{
	int	hit;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_hitroll: ch è NULL" );
		return 0;
	}

	hit = table_attr_bonus[get_curr_attr(ch, ATTR_STR)].tohit;

	/* Calcolo sulle posizioni simile a quello della get_ac_natural (TT) bisognerebbe controllare in fight.c se l'hit e il roll non venissero giù penalizzati per posizioni di combat svantaggiose */
	switch ( ch->position )
	{
	  case POSITION_MORTAL:
	  case POSITION_INCAP:
	  case POSITION_STUN:
	  case POSITION_SLEEP:
		if ( hit <= 0 )
			hit *= 4;
		else
			hit /= 4;
		break;

	  case POSITION_REST:
		if ( hit <= 0 )
			hit *= 3;
		else
			hit /= 3;
		break;

	  case POSITION_SIT:
/*	  case POSITION_KNEE: (GG) */
		if ( hit <= 0 )
			hit *= 2;
		else
			hit /= 2;
		break;
	}

	return ch->hitroll + hit + ( 2-(abs(ch->mental_state)/10) );
}


/*
 * Calcola il dam di un pg
 */
int get_damroll( CHAR_DATA *ch )
{
	int	dam;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_damroll: ch è NULL" );
		return 0;
	}

	dam = table_attr_bonus[get_curr_attr(ch, ATTR_STR)].todam;

	/* Calcolo sulle posizioni simile a quello della get_ac_natural (TT) bisognerebbe controllare in fight.c se l'hit e il roll non venissero giù penalizzati per posizioni di combat svantaggiose */
	switch ( ch->position )
	{
	  case POSITION_MORTAL:
	  case POSITION_INCAP:
	  case POSITION_STUN:
	  case POSITION_SLEEP:
		if ( dam <= 0 )
			dam *= 4;
		else
			dam /= 4;
		break;

	  case POSITION_REST:
		if ( dam <= 0 )
			dam *= 3;
		else
			dam /= 3;
		break;

	  case POSITION_SIT:
/*	  case POSITION_KNEE: (GG) */
		if ( dam <= 0 )
			dam *= 2;
		else
			dam /= 2;
		break;
	}

	if ( ch->mental_state > 5 && ch->mental_state < 15 )
		dam++;

	return ch->damroll + ch->damplus + dam;
}


/*
 * Controlla se il pg è sveglio.
 */
bool is_awake( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_awake: ch è NULL" );
		return TRUE;
	}

	if ( ch->position > POSITION_SLEEP )
		return TRUE;

	return FALSE;
}


/*
 * Controlla che il pg stia volando.
 */
bool is_floating( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_floating: ch è NULL" );
		return FALSE;
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_FLYING) )
		return TRUE;

	if ( HAS_BIT(ch->affected_by, AFFECT_FLOATING) )
		return TRUE;

	return FALSE;
}


/*
 * Retrieve a character's carry capacity.
 * (FF) Devo basarlo sul peso e l'altezza del pg anche, se è più grande può portare più cose
 */
int can_carry_number( CHAR_DATA *ch )
{
	int penalty;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "can_carry_number: ch è NULL" );
		return 6;
	}

	if ( IS_PG(ch) && IS_ADMIN(ch) )
		return get_level( ch ) * 10;

	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_MASCOTTE) )
		return get_level( ch );	/* (TT) ex 0 */

	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_CARRY) )
		return get_level( ch ) * 20;

	penalty = 0;
	if ( get_eq_char(ch, WEARLOC_WIELD)	  )	penalty++;
	if ( get_eq_char(ch, WEARLOC_DUAL)	  )	penalty++;
	if ( get_eq_char(ch, WEARLOC_MISSILE) )	penalty++;
	if ( get_eq_char(ch, WEARLOC_HOLD)	  )	penalty++;
	if ( get_eq_char(ch, WEARLOC_SHIELD)  )	penalty++;

	return URANGE( 8, (get_level(ch)+10)/6 + (get_curr_attr(ch, ATTR_RES)/4 - 6) - penalty, 36 );
}


/*
 * Retrieve a character's carry capacity.
 * (FF) Devo basarlo sul peso e l'altezza del pg anche, se è più grande può portare più cose
 */
int can_carry_weight( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "can_carry_weight: ch è NULL" );
		return 2000;
	}

	if ( IS_PG(ch) && IS_ADMIN(ch) )
		return 10000000;	/* 1 quintale */

	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_MASCOTTE) )
		return 10000;	/* ora 10 kg */

	if ( IS_MOB(ch) && HAS_BIT_ACT(ch, MOB_CARRY) )
		return 1000000;	/* 1 quintale */

	/* Minimo per un pg è 1*2*1000 = 2kg
	 * Massimo 125*2*1000 = 250kg */
	return ( get_curr_attr(ch, ATTR_RES) * 2 * 1000 );	/* 1000 sono per convertire ai grammi */
}


/*
 * Controlla se il pg si trovi all'aperto.
 */
bool is_outside( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_outside: ch è NULL" );
		return FALSE;
	}

	if ( HAS_BIT(ch->in_room->flags, ROOM_INDOORS) )
		return FALSE;

	if ( HAS_BIT(ch->in_room->flags, ROOM_TUNNEL) )
		return FALSE;

	return TRUE;
}


/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
	/* panicked attempt to stop crashes */
	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "can_see: victim è NULL, ch è %s", ch->name );
		return FALSE;
	}

	if ( !ch )
	{
		if ( HAS_BIT(victim->affected_by, AFFECT_INVISIBLE) || HAS_BIT(victim->affected_by, AFFECT_HIDE) )
			return FALSE;
		else
			return TRUE;
	}

	if ( ch == victim )
		return TRUE;

	if ( IS_ADMIN(ch) && HAS_BIT_PLR(ch, PLAYER_ADMSIGHT) )
		return TRUE;

	if ( IS_MOB(victim) && victim->pIndexData->vnum == VNUM_MOB_SUPERMOB && !IS_ADMIN(ch) )
		return FALSE;

	if ( IS_PG(victim) && IS_ADMIN(victim) && victim->pg->incognito >= ch->level )
		return FALSE;

	if ( IS_MOB(victim) && HAS_BIT_ACT(victim, MOB_INCOGNITO) && victim->mobinvis > get_level(ch) )
		return FALSE;

	/* Link-dead over 2 ticks aren't seen by mortals */
	if ( IS_PG(ch) && IS_PG(victim)
	  && !victim->desc && victim->timer > 2
	  && !IS_ADMIN(ch) )
	{
		return FALSE;
	}

	/* The miracle cure for blindness? */
	if ( HAS_BIT(ch->affected_by, AFFECT_TRUESIGHT) )
		return TRUE;

	/* Gli si è spiriti allora vedono */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SPIRIT) )
		return TRUE;

	if ( HAS_BIT(ch->affected_by, AFFECT_BLIND) )
		return FALSE;

	if ( room_is_dark(ch->in_room) && !HAS_BIT(ch->affected_by, AFFECT_INFRARED) )
		return FALSE;

	if ( HAS_BIT(victim->affected_by, AFFECT_INVISIBLE)
	  && !HAS_BIT(ch->affected_by, AFFECT_DETECT_INVIS) )
	{
		return FALSE;
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_HIDE)
	  && !HAS_BIT(ch->affected_by, AFFECT_DETECT_HIDDEN)
	  && !victim->fighting
/*	  && (IS_MOB(ch) ? IS_PG(victim) : IS_MOB(victim))*/ )	/* (TT) tolta che crea casini e basta secondo me */
	{
		return FALSE;
	}

	return TRUE;
}


/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
	OBJ_DATA	*obj;
	AFFECT_DATA *paf;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "char_from_room: ch è NULL" );
		return;
	}

	if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "char_from_room: ch->in_room è NULL per il pg %s", ch->name );
		return;
	}

	if ( IS_PG(ch) )
		--ch->in_room->area->nplayer;

	if ( (obj = get_eq_char(ch, WEARLOC_LIGHT)) != NULL && obj->light
	  && obj->light->hours != 0 && ch->in_room->light > 0 )
	{
		--ch->in_room->light;
	}

	/*
	 * Character's affect on the room
	 */
	for ( paf = ch->first_affect;  paf;  paf = paf->next )
		room_affect( ch->in_room, paf, FALSE );

	/*
	 * Room's affect on the character
	 */
	if ( !char_died(ch) )
	{
		for ( paf = ch->in_room->first_affect;  paf;  paf = paf->next )
			affect_modify(ch, paf, FALSE);

		if ( char_died(ch) )	/* could die from removespell, etc */
			return;
	}

	UNLINK( ch, ch->in_room->first_person, ch->in_room->last_person, next_in_room, prev_in_room );

	ch->was_in_room  = ch->in_room;
	ch->in_room      = NULL;
	ch->next_in_room = NULL;
	ch->prev_in_room = NULL;

	if ( IS_PG(ch) && get_timer(ch, TIMER_SHOVEDRAG) > 0 )
		remove_timer( ch, TIMER_SHOVEDRAG );
}


/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_DATA *pRoom )
{
	OBJ_DATA    *obj;
	AFFECT_DATA *paf;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "char_to_room: ch è NULL" );
		return;
	}

	if ( !pRoom )
	{
		send_log( NULL, LOG_BUG, "char_to_room: %s -> NULL room!  Putting char in limbo (%d)",
			ch->name, VNUM_ROOM_LIMBO );
		/*
		 * This used to just return, but there was a problem with crashing
		 * and I saw no reason not to just put the char in limbo.
		 */
		pRoom = get_room_index( NULL, VNUM_ROOM_LIMBO );
	}

	ch->in_room	= pRoom;
	LINK( ch, pRoom->first_person, pRoom->last_person, next_in_room, prev_in_room );

	if ( IS_PG(ch) )
	{
		if ( pRoom->area && ++pRoom->area->nplayer > pRoom->area->max_players )
			pRoom->area->max_players = pRoom->area->nplayer;
	}

	if ( (obj = get_eq_char(ch, WEARLOC_LIGHT)) != NULL && obj->light
	  && obj->light->hours != 0 && ch->in_room->light < 100 )	/* Giusto per mettere un limite alla luminosità (GG) */
	{
		++pRoom->light;
	}

	/*
	 * Room's effect on the character
	 */
	if ( !char_died(ch) )
	{
		for ( paf = pRoom->first_affect;  paf;  paf = paf->next )
			affect_modify(ch, paf, TRUE);

		if ( char_died(ch) )	/* could die from a wearspell, etc */
			return;
    }

	/*
	 * Character's effect on the room
	 */
	for ( paf = ch->first_affect;  paf;  paf = paf->next )
		room_affect(pRoom, paf, TRUE);

	if ( IS_PG(ch) && HAS_BIT(pRoom->flags, ROOM_SAFE)
	  && get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
	{
		add_timer(ch, TIMER_SHOVEDRAG, PULSE_VIOLENCE * 10, NULL, 0);  /* 30 Seconds */
	}

	/*
	 * Delayed Teleport rooms
	 * Should be the last thing checked in this function
	 */
	if ( HAS_BIT(pRoom->flags, ROOM_TELEPORT) && pRoom->tele_delay > 0 )
	{
		TELEPORT_DATA *tele;

		for ( tele = first_teleport;  tele;  tele = tele->next )
		{
			if ( tele->room == pRoom )
				return;
		}
		CREATE( tele, TELEPORT_DATA, 1 );
		LINK( tele, first_teleport, last_teleport, next, prev );
		tele->room		= pRoom;
		tele->timer		= pRoom->tele_delay;
	}

	if ( !ch->was_in_room )
		ch->was_in_room = ch->in_room;
}


/*
 * Add ch to the queue of recently extracted characters
 */
void queue_extracted_char( CHAR_DATA *ch, bool extract )
{
	CHAR_DATA_EXTRACT *ccd;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "queue_extracted char: ch è NULL" );
		return;
	}

	CREATE( ccd, CHAR_DATA_EXTRACT, 1 );
	ccd->ch				 = ch;
	ccd->room			 = ch->in_room;
	ccd->extract		 = extract;

	if ( ch == cur_char )
		ccd->retcode	 = global_retcode;
	else
		ccd->retcode	 = rCHAR_DIED;

	ccd->next			 = extracted_char_queue;
	extracted_char_queue = ccd;
	cur_qchars++;
}


/*
 * Clean out the extracted character queue
 */
void clean_char_queue( void )
{
	CHAR_DATA_EXTRACT *ccd;

	for ( ccd = extracted_char_queue;  ccd;  ccd = extracted_char_queue )
	{
		extracted_char_queue = ccd->next;
		if ( ccd->extract )
			free_char( ccd->ch );

		DISPOSE( ccd );
		--cur_qchars;
	}
}


/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
	CHAR_DATA	*wch;
	OBJ_DATA	*obj;
	ROOM_DATA   *location;
	REL_DATA	*RQueue;
	REL_DATA	*rq_next;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "extract_char: NULL ch." );
		return;
	}

	if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "extract_char: %s in NULL room.", ch->name ? ch->name : "???" );
		return;
	}

	/* Se il pg morto si trova in un campo di arena gli ridà i punti di vita */
	if ( HAS_BIT(ch->in_room->flags, ROOM_ARENA) )
	{
		int x;

		for ( x = 0;  x > MAX_POINTS;  x++ )
			ch->points[x] = ch->points_max[x];
	}

	if ( ch == supermob )
	{
		send_log( NULL, LOG_BUG, "extract_char: ch == supermob!" );
		return;
	}

	if ( char_died(ch) )
	{
		send_log( NULL, LOG_BUG, "extract_char: %s already died!", ch->name );
		return;
	}

	if ( ch == cur_char )
		cur_char_died = TRUE;

	/* shove onto extraction queue */
	queue_extracted_char( ch, fPull );

	for ( RQueue = first_relation;  RQueue;  RQueue = rq_next )
	{
		rq_next = RQueue->next;
		if ( fPull && RQueue->type == relMRESTRING_ON )
		{
			if ( ch == RQueue->subject )
				((CHAR_DATA *) RQueue->actor)->dest_buf = NULL;
			else if ( ch != RQueue->actor )
				continue;
			UNLINK( RQueue, first_relation, last_relation, next, prev );
			DISPOSE( RQueue );
		}
	}

	if ( gch_prev == ch )
		gch_prev = ch->prev;

	if ( fPull)
		die_follower( ch );

	stop_fighting( ch, TRUE );

	if ( ch->mount )
	{
		REMOVE_BIT( ch->mount->act, MOB_MOUNTED );
		ch->mount = NULL;
		ch->position = POSITION_STAND;
	}

	/*
	 * Check if this NPC was a mount or a mascotte
	 */
	if ( IS_MOB(ch) )
	{
		for ( wch = first_char;  wch;  wch = wch->next )
		{
			if ( wch->mount == ch )
			{
				wch->mount	  = NULL;
				wch->position = POSITION_STAND;

				if ( wch->in_room == ch->in_room )
				{
					act( AT_SOCIAL, "Your faithful mount, $N collapses beneath you...", wch, NULL, ch, TO_CHAR );
					act( AT_SOCIAL, "Sadly you dismount $M for the last time.", wch, NULL, ch, TO_CHAR );
					act( AT_PLAIN, "$n sadly dismounts $N for the last time.", wch, NULL, ch, TO_ROOM );
				}
			}

			if ( wch->pg && wch->pg->mascotte == ch )
			{
				wch->pg->mascotte = NULL;
				if ( wch->in_room == ch->in_room )
					act( AT_SOCIAL, "You mourn for the loss of $N.", wch, NULL, ch, TO_CHAR );
			}
		}
	}
	REMOVE_BIT( ch->act, MOB_MOUNTED );

	while ( (obj = ch->last_carrying) != NULL )
		free_object( obj );

	char_from_room( ch );

	if ( !fPull )
	{
		location = NULL;

		if ( IS_PG(ch) && ch->pg->clan )
			location = ch->pg->clan->recall;

		if ( !location )
			location = get_room_index( NULL, VNUM_ROOM_ALTAR );

		if ( !location )
			location = get_room_index( NULL, 2 );

		char_to_room( ch, location );

		/* Make things a little fancier */
		wch = get_mob_room( ch, "healer", TRUE );		/* (bb) healer... uhm */
		if ( wch )
		{
			char	buf[MSL];

			act( AT_MAGIC, "$n mutters a few incantations, waves su$x hands and points su$x finger.",
				wch, NULL, NULL, TO_ROOM );
			act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
			sprintf( buf, "say a %s Welcome back to the land of the living", ch->name );
			send_command( wch, buf, CO );
		}
		else
		{
			act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
		}

		ch->position = POSITION_REST;
		return;
    } /* chiude l'if */

	if ( IS_MOB(ch) )
	{
		--ch->pIndexData->count;
		--mobs_loaded;
	}

	if ( ch->desc && ch->desc->original )
		send_command( ch, "return", CO );

	for ( wch = first_char;  wch;  wch = wch->next )
	{
		if ( wch->reply == ch )
			wch->reply = NULL;
    }

	UNLINK( ch, first_char, last_char, next, prev );
	if ( IS_PG(ch) )
		UNLINK( ch, first_player, last_player, next_player, prev_player );

	if ( ch->desc )
	{
		if ( ch->desc->character != ch )
		{
			send_log( NULL, LOG_BUG, "extract_char: char's descriptor points to another char" );
		}
		else
		{
			ch->desc->character = NULL;
			close_socket( ch->desc, FALSE );
			ch->desc = NULL;
		}
	}
}


#define ENTITY_CHAR			0	/* sia mob che player */
#define ENTITY_MOB			1
#define ENTITY_PLAYER		2

#define IN_ROOM			0
#define IN_AREA			1
#define IN_MUD			2


/*
 * Funzione con un check ricorrente nella get_entity_location()
 */
bool check_entity_location( CHAR_DATA *ch, CHAR_DATA *victim, bool check_see, const int entity )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_entity_location: ch è NULL" );
		return FALSE;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "check_entity_location: victim è NULL" );
		return FALSE;
	}

	if ( entity < 0 || entity > ENTITY_PLAYER )
	{
		send_log( NULL, LOG_BUG, "check_entity_location: valore di entity passato errato: %d", entity );
		return FALSE;
	}

	if ( check_see && !can_see(ch, victim) )
		return FALSE;

	if ( entity == ENTITY_MOB     &&  IS_PG(victim) )
		return FALSE;
	if ( entity == ENTITY_PLAYER  &&  IS_MOB(victim) )
		return FALSE;

	return TRUE;
}


/*
 * Funzione che ritorna il ch cercato con argument e con search che indica una ricerca
 *	stanza, area o su tutto il mud
 */
CHAR_DATA *get_entity_location( CHAR_DATA *ch, char *argument, bool check_see, const int entity, const int location )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA		*victim;
	char			 arg[MIL];
	int				 number;
	int				 count;
	VNUM			 vnum;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_entity_location: ch è NULL" );
		return NULL;
	}

	if ( entity < 0 || entity > ENTITY_PLAYER )
	{
		send_log( NULL, LOG_BUG, "get_entity_location: valore di entity passato errato: %d", entity );
		return NULL;
	}

	if ( location < 0 || location > IN_MUD )
	{
		send_log( NULL, LOG_BUG, "get_entity_location: valore di location passato errato: %d", location );
		return NULL;
	}

	/* Allow reference by vnum for implementor */
	number = number_argument( argument, arg );
	if ( IS_ADMIN(ch) && is_number(arg) )
		vnum = atoi( arg );
	else
		vnum = -1;

	/* Check the room for an exact match */
	count  = 0;
	for ( victim = ch->in_room->first_person;  victim;  victim = victim->next_in_room )
	{
		if ( !check_entity_location(ch, victim, check_see, entity) )
			continue;

		if ( (nifty_is_name(arg, victim->name)
		  || (IS_MOB(victim) && vnum == victim->pIndexData->vnum)) )
		{
			if ( ++count == number )
				return victim;
		}
	}

	/* Se cerca nell'area o nel mud controlla la lista dei descrittori in maniera esatta */
	if ( location == IN_AREA || location == IN_MUD )
	{
		for ( d = first_descriptor;  d;  d = d->next )
		{
			victim = d->character;

			if ( !is_in_game(d) )		continue;
			if ( !victim )				continue;
			if ( !victim->in_room )		continue;

			if ( !check_entity_location(d->character, victim, check_see, entity) )
				continue;
			if ( location == IN_AREA && victim->in_room->area != ch->in_room->area )
				continue;

			if ( nifty_is_name(arg, victim->name) )
				return victim;
		}

		/* check the area for an exact match */
	    count = 0;
		for ( victim = first_char;  victim;  victim = victim->next )
		{
			if ( location == IN_AREA && victim->in_room->area != ch->in_room->area )
				continue;
			if ( !check_entity_location(ch, victim, check_see, entity) )
				continue;

			if ( (nifty_is_name(arg, victim->name)
			  || (IS_MOB(victim) && vnum == victim->pIndexData->vnum)) )
			{
				if ( ++count == number )
					return victim;
			}
		}
	}

	/* Bail out if looking for a vnum match */
	if ( vnum != -1 )
		return NULL;

	/*
	 * If we didn't find an exact match, run through the list of characters
	 * again looking for prefix matching, ie gu == guard.
	 */
	count  = 0;
	for ( victim = ch->in_room->first_person;  victim;  victim = victim->next_in_room )
	{
		if ( !check_entity_location(ch, victim, check_see, entity) )
			continue;
		if ( !nifty_is_name_prefix(arg, victim->name) )
			continue;

		if ( ++count == number )
			return victim;
    }

	/* Controlla il prefisso dei nomi dellla lista dei descrittori */
	if ( location == IN_AREA || location == IN_MUD )
	{
		for ( d = first_descriptor;  d;  d = d->next )
		{
			victim = d->character;

			if ( !is_in_game(d) )	continue;
			if ( !victim )			continue;
			if ( !victim->in_room )	continue;

			if ( !check_entity_location(d->character, victim, check_see, entity) )
				continue;
			if ( location == IN_AREA && victim->in_room->area != ch->in_room->area )
				continue;

			if ( nifty_is_name_prefix(arg, victim->name) )
				return victim;
		}

		/*
		 * If we didn't find a prefix match in the room, run through the full list
		 * of characters looking for prefix matching, ie gu == guard.
		 */
		count  = 0;
		for ( victim = first_char;  victim;  victim = victim->next )
		{
			if ( location == IN_AREA && victim->in_room->area != ch->in_room->area )
				continue;
			if ( !check_entity_location(ch, victim, check_see, entity) )
				continue;
			if ( !nifty_is_name_prefix(arg, victim->name) )	continue;

			if ( ++count == number )
				return victim;
		}
	}

    return NULL;
}

/* char */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_CHAR, IN_ROOM );
}

CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_CHAR, IN_AREA );
}

CHAR_DATA *get_char_mud( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_CHAR, IN_MUD );
}

/* mob */
CHAR_DATA *get_mob_room( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_CHAR, IN_ROOM );
}

CHAR_DATA *get_mob_area( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_MOB, IN_AREA );
}

CHAR_DATA *get_mob_mud( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_MOB, IN_MUD );
}

/* pg */
CHAR_DATA *get_player_room( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_PLAYER, IN_ROOM );
}

CHAR_DATA *get_player_area( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_PLAYER, IN_AREA );
}

CHAR_DATA *get_player_mud( CHAR_DATA *ch, char *argument, bool check_see )
{
	return get_entity_location( ch, argument, check_see, ENTITY_PLAYER, IN_MUD );
}

#undef ENTITY_CHAR
#undef ENTITY_MOB
#undef ENTITY_PLAYER

#undef IN_ROOM
#undef IN_AREA
#undef IN_MUD


/*
 * Trova nella lista degli offline il pg con il nome passato
 */
CHAR_DATA *get_offline( const char *argument, bool exact )
{
	CHAR_DATA	*ch;

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "get_offline: argument passato non è valido" );
		return NULL;
	}

	for ( ch = first_offline;  ch;  ch = ch->next )
	{
		if ( !str_cmp(ch->name, argument) )
			return ch;
	}

	if ( exact )
		return NULL;

	for ( ch = first_offline;  ch;  ch = ch->next )
	{
		if ( !str_prefix(argument, ch->name) )
			return ch;
	}

	return NULL;
}


/*
 * Set the current global character to ch
 */
void set_cur_char( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "set_cru_char: ch è NULL" );
		return;
	}

	cur_char	   = ch;
	cur_char_died  = FALSE;
	cur_room	   = ch->in_room;
	global_retcode = rNONE;
}


/*
 * Check to see if ch died recently
 */
bool char_died( CHAR_DATA *ch )
{
	CHAR_DATA_EXTRACT *ccd;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "set_cur_char: ch è NULL" );
		return FALSE;
	}

	if ( ch == cur_char && cur_char_died )
		return TRUE;

	for ( ccd = extracted_char_queue;  ccd;  ccd = ccd->next )
	{
		if ( ccd->ch == ch )
			return TRUE;
	}
	return FALSE;
}


/*
 * Ritorna il punteggio di victim in percentuale colorata
 * Se ch è admin lo visualizza in numero
 * victim serve da passare per far funzionare la visualizzazione della score_whois
 */
char *get_points_colored( CHAR_DATA *ch, CHAR_DATA *victim, int point, bool prompt )
{
	static char	colored[MIL];
	char		clr[MIL];
	int			percent;

	percent = get_percent( victim->points[point], victim->points_max[point] );
	if		( percent >= 50 )
		strcpy( clr, (point == POINTS_LIFE) ? "&R" : (point == POINTS_MOVE) ? "&G" : (point == POINTS_MANA) ? "&C" : "&W" );
	else if ( percent >= 25 )
		strcpy( clr, (point == POINTS_LIFE) ? "&r" : (point == POINTS_MOVE) ? "&g" : (point == POINTS_MANA) ? "&c" : "&w" );
	else
		strcpy( clr, (point == POINTS_LIFE) ? "&Y" : (point == POINTS_MOVE) ? "&O" : (point == POINTS_MANA) ? "&B" : "&z" );

	/* Ritorna il punteggio per gli admin e la percentuale per gli utenti */
	if ( IS_ADMIN(ch) )
	{
		sprintf( colored, "%s%d", clr, victim->points[point] );
	}
	else
	{
		if ( prompt && HAS_BIT_PLR(ch, PLAYER_PROMPTBAR) )
		{
			int x;

			strcpy( colored, clr );
			for ( x = 10;  x >= 0;  x-- )
			{
				if ( percent - 10 >= 0 )
				{
					percent -= 10;
					strcat( colored, "|" );
				}
				else
				{
					break;
				}
			}
			if ( percent - 5 >= 0 )
				strcat( colored, "'" );
			strcat( colored, "&w" );
		}
		else
		{
			sprintf( colored, "%s%d&w%%", clr, percent );
		}
	}

	return colored;
}


/*
 * Controlla se gli si abbassano i punti oppure no.
 */
bool stop_lowering_points( CHAR_DATA *ch, const int type )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "stop_lowering_points: ch è NULL " );
		return FALSE;
	}

	if ( IS_MOB(ch) )
		return FALSE;

	if ( IS_ADMIN(ch) )
		return TRUE;

	/* I niubbini muoiono più difficilmente così da aiutarli */
	if ( get_level(ch) < LVL_NEWBIE/2 && type == POINTS_LIFE && number_range(0, 3) == 0 )
		return TRUE;

	if ( HAS_BIT(ch->in_room->area->flags, AREAFLAG_NOPOINTLOSS) )
		return TRUE;

	return FALSE;
}


/*
 * Controlla che il pg non abbia superato il massimo
 * Se ritorna negativo significa che il pg ha superato il proprio limite
 */
int check_total_points( CHAR_DATA *ch )
{
	int		total = 0;	/* totale dei punt del pg */
	int		max = 0;	/* massimo di default */
	int		x;			/* contatore */

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_total_points: ch è NULL" );
		return 0;
	}

	for ( x = 0;  x < MAX_POINTS;  x++ )
	{
		max		+= table_race[ch->race]->points_limit[x];
		total	+= ch->points_max[x];
	}

	max = max * 2/3;

	/* Se il totale è troppo alto, avverte l'admin */
/*	if ( total-500 > max )
		send_log( NULL, LOG_WARNING, "check_total_points: troppo alto (%d) per pg %s", total, ch->name );*/

	return max - total;

}


/*
 * How mental state could affect finding an object
 * Used by get/drop/put/quaff/recite/etc
 * Increasingly freaky based on mental state and drunkeness
 */
bool ms_find_obj( CHAR_DATA *ch )
{
	char	t[MSL];
	int		ms = ch->mental_state;
	int		drunk = IS_MOB(ch)  ?  0  :  ch->pg->condition[CONDITION_DRUNK];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "ms_find_obj: ch è NULL" );
		return FALSE;
	}

	/*
	 * we're going to be nice and let nothing weird happen unless
	 * you're a tad messed up
	 */
	drunk = UMAX( 1, drunk );
	if ( abs(ms) + (drunk/3) < 30 )
		return FALSE;

	if ( (number_percent() + (ms < 0 ? 15 : 5))> abs(ms)/2 + drunk/4 )
		return FALSE;

	if ( ms > 15 )
	{
		/* (TR) */
		switch ( number_range(UMAX(1, (ms/5-15)), (ms+4) * 2/5) )	/* (TT) */
		{
		  default:
		  case  1:	strcpy( t, "As you reach for it, you forgot what it was...\r\n" );									break;
		  case  2:	strcpy( t, "Appena lo raggiungi, ti dimentichi cos'era...\n\r" );									break;
		  case  3:	strcpy( t, "As you reach for it, something inside stops you...\r\n" );								break;
		  case  4:	strcpy( t, "Appena lo raggiungi, qualcosa ti blocca...\n\r" );										break;
		  case  5:	strcpy( t, "As you reach for it, it seems to move out of the way...\r\n" );							break;
		  case  6:	strcpy( t, "Appena lo raggiungi, sembra che si sposti lontano...\n\r" );							break;
		  case  7:	strcpy( t, "You grab frantically for it, but can't seem to get a hold of it...\r\n" );				break;
		  case  8:	strcpy( t, "Cerchi freneticamente di afferrarlo, ma non riesci a prenderlo...\n\r" );				break;
		  case  9:	strcpy( t, "It disappears as soon as you touch it!\r\n" );											break;
		  case 10:	strcpy( t, "Scompare non appena lo tocchi!\n\r" );													break;
		  case 11:	strcpy( t, "You would if it would stay still!\r\n" );												break;
		  case 12:	strcpy( t, "Ci riusciresti se stesse fermo!\n\r" );													break;
		  case 13:	strcpy( t, "Whoa!  It's covered in blood!  Ack!  Ick!\r\n" );										break;
		  case 14:	strcpy( t, "Whoa! E' coperto di sangue! Ack! Ick!\n\r" );											break;
		  case 15:	strcpy( t, "Wow... trails!\r\n" );																	break;
		  case 16:	strcpy( t, "Wow...delle tracce!\n\r" );																break;
		  case 17:	strcpy( t, "You reach for it, then notice the back of your hand is growing something!\r\n" );		break;
		  case 18:	strcpy( t, "Lo hai quasi raggiunto, quando ti accorgi che sta crescendo qualcosa sul dorso della tua mano!\n\r" );	break;
		  case 19:	strcpy( t, "As you grasp it, it shatters into tiny shards which bite into your flesh!\r\n" );		break;
		  case 20:	strcpy( t, "Non appena lo tocchi si frantuma in piccoli pezzi e ti taglia la carne!\n\r" );			break;
		  case 21:	strcpy( t, "What about that huge dragon flying over your head?!?!?\r\n" );							break;
		  case 22:	strcpy( t, "Cosa vuole quel drago gigante che sta volando sopra la tua testa?!?!?\n\r" );			break;
		  case 23:	strcpy( t, "You stratch yourself instead...\r\n" );													break;
		  case 24:	strcpy( t, "Ti allunghi per afferrarlo ma non ci arrivi...\n\r" );									break;
		  case 25:	strcpy( t, "You hold the universe in the palm of your hand!\r\n" );									break;
		  case 26:	strcpy( t, "Tieni l'universo nel palmo della tua mano!\n\r" );										break;
		  case 27:	strcpy( t, "You're too scared.\r\n" );																break;
		  case 28:	strcpy( t, "Sei troppo spaventato.\n\r" );															break;
		  case 29:	strcpy( t, "Your mother smacks your hand... 'NO!'\r\n" );											break;
		  case 30:	strcpy( t, "Una bellissima fanciulla ti bacia su una guancia!\n\r" );								break;
		  case 31:	strcpy( t, "Your hand grasps the worst pile of revoltingness that you could ever imagine!\r\n" );	break;
		  case 32:	strcpy( t, "La tua mano afferra il peggior mucchio di schifezze che si possa immaginare!\n\r" );	break;
		  case 33:	strcpy( t, "You stop reaching for it as it screams out at you in pain!\r\n" );						break;
		  case 34:	strcpy( t, "Lo lasci cadere non appena si mette ad urlare di dolore!\n\r" );						break;
		  case 35:	strcpy( t, "What about the millions of burrow-maggots feasting on your arm?!?!\r\n" );				break;
		  case 36:	strcpy( t, "Da dove vengono quelle larve che ti camminano sul braccio?!?!\n\r" );					break;
		  case 37:	strcpy( t, "That doesn't matter anymore... you've found the true answer to everything!\r\n" );		break;
		  case 38:	strcpy( t, "Non t'importa piu'...hai già trovato la vera risposta a tutto!\n\r" );					break;
		  case 39:	strcpy( t, "A supreme entity has no need for that.\r\n" );											break;
		  case 40:	strcpy( t, "Una divinità come te non ne ha bisogno.\n\r" );										break;
		}
	}
    else
    {
		int		sub = URANGE(1, abs(ms)/2 + drunk, 60);

		/* (TR) */
		switch ( number_range(1, sub/5) )	/* (TT) */
		{
		  default:
		  case  1:	strcpy( t, "In just a second..\r\n" );										break;
		  case  2:	strcpy( t, "Solo un attimo..\r\n" );										break;
		  case  3:	strcpy( t, "You can't find that..\r\n" );									break;
		  case  4:	strcpy( t, "Non riesci a trovarlo..\r\n" );									break;
		  case  5:	strcpy( t, "It's just beyond your grasp..\r\n" );							break;
		  case  6:	strcpy( t, "E' irraggiungibile..\r\n" );									break;
		  case  7:	strcpy( t, ".. but it's under a pile of other stuff..\r\n" );				break;
		  case  8:	strcpy( t, ".. ma è in mezzo ad una pila di altri oggetti..\r\n" );			break;
		  case  9:	strcpy( t, "You go to reach for it, but pick your nose instead.\r\n" );		break;
		  case 10:	strcpy( t, "Cerchi di prenderlo, ma ti colpisci sul naso invece.\r\n" );	break;
		  case 11:	strcpy( t, "Which one?!?  I see two.. no three..\r\n" );					break;
		  case 12:	strcpy( t, "Quale?!? Ne vedo due.. no tre..\r\n" );							break;
		}
	}

	send_to_char( ch, t );
	return TRUE;
}


/*
 * Improve mental state
 */
void better_mental_state( CHAR_DATA *ch, int mod )
{
	int		c   = URANGE( 0, abs(mod), 20 );
	int		wil = get_curr_attr( ch, ATTR_WIL ) / 4;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "better_mental_state: ch è NULL" );
		return;
	}

	c += number_percent() < wil ? 1 : 0;

	if ( ch->mental_state < 0 )
		ch->mental_state = URANGE( -100, ch->mental_state + c, 0 );
	else if ( ch->mental_state > 0 )
		ch->mental_state = URANGE( 0, ch->mental_state - c, 100 );
	/* Se invece è uguale a zero non deve migliorare affatto il mental state */
}

/*
 * Deteriorate mental state
 */
void worsen_mental_state( CHAR_DATA *ch, int mod )
{
    int c   = URANGE( 0, abs(mod), 20 );
    int wil = get_curr_attr( ch, ATTR_WIL ) / 4;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "worsen_mental_state: ch è NULL" );
		return;
	}

	c -= number_percent() < wil ? 1 : 0;
	if ( c < 1 )
		return;

	if		( ch->mental_state < 0 )
		ch->mental_state = URANGE( -100, ch->mental_state - c, 100 );
	else if ( ch->mental_state > 0 )
		ch->mental_state = URANGE( -100, ch->mental_state + c, 100 );
	else
		ch->mental_state -= c;
}


/*
 * Add another notch on that there belt... ;)
 * Keep track of the last so many kills by vnum
 */
void add_kill( CHAR_DATA *ch, CHAR_DATA *mob )
{
	int		x;
	VNUM	vnum;


	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "add_kill: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_BUG, "add_kill: trying to add kill to npc" );
		return;
	}

	if ( IS_PG(mob) )
	{
		send_log( NULL, LOG_BUG, "add_kill: trying to add kill non-npc" );
		return;
	}

	vnum = mob->pIndexData->vnum;

	for ( x = 0;  x < MAX_KILLTRACK;  x++ )
	{
		if ( ch->pg->killed[x].vnum == vnum )
		{
			if ( ch->pg->killed[x].count < 50 )
			++ch->pg->killed[x].count;
			return;
		}
		else if ( ch->pg->killed[x].vnum == 0 )
			break;
	}

	memmove( (char *) ch->pg->killed+sizeof(KILLED_DATA),
		ch->pg->killed, (MAX_KILLTRACK-1) * sizeof(KILLED_DATA) );
	ch->pg->killed[0].vnum  = vnum;
	ch->pg->killed[0].count = 1;
}


/*
 * Return how many times this player has killed this mob
 * Only keeps track of so many (MAX_KILLTRACK), and keeps track by vnum.
 */
int times_killed( CHAR_DATA *ch, CHAR_DATA *mob )
{
	int		x;
	VNUM	vnum;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "times_killed: ch è NULL" );
		return 0;
	}

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_BUG, "times_killed: ch is not a player" );
		return 0;
	}

	if ( IS_PG(mob) )
	{
		send_log( NULL, LOG_BUG, "add_kill: mob is not a mobile" );
		return 0;
	}

	vnum = mob->pIndexData->vnum;

	for ( x = 0;  x < MAX_KILLTRACK;  x++ )
	{
		if		( ch->pg->killed[x].vnum == vnum )
			return ch->pg->killed[x].count;
		else if ( ch->pg->killed[x].vnum == 0 )
			break;
	}

	return 0;
}


/*
 * Passa il valore di wait_stat adatto al pg.
 */
void WAIT_STATE( CHAR_DATA *ch, int pulse )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "WAIT_STATE: ch è NULL" );
		return;
	}

	if ( IS_ADMIN(ch) && HAS_BIT_PLR(ch, PLAYER_NOWAIT) )
		ch->wait = 0;
	else
		ch->wait = UMAX( ch->wait, pulse );

	/* Raddoppia il waitstate per i pg in nuisance */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_NUISANCE) )
		ch->wait *= 2;
}


/*
 * Ritorna TRUE se il pg si trova nel campo di combattimento di un'area di arena
 */
bool in_arena( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "ch passato è NULL" );
		return FALSE;
	}

	if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "ch->in_room è NULL per ch %s",
			IS_MOB(ch) ? ch->short_descr : ch->name );
		return FALSE;
	}

	if ( HAS_BIT(ch->in_room->flags, ROOM_ARENA) )
		return TRUE;

	return FALSE;
}
