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
#include "calendar.h"
#include "db.h"
#include "interpret.h"
#include "mprog.h"
#include "room.h"


/*
 * Variabili esterne
 */
struct act_prog_data	*mob_act_list;


/*
 * Variabili locali
 */
struct act_prog_data	*obj_act_list;
struct act_prog_data	*room_act_list;


const CODE_DATA code_mptrigger[] =
{
/*		trigger code			trigger name */
	{ MPTRIGGER_ACT,		"MPTRIGGER_ACT",		"act_prog",			},
	{ MPTRIGGER_SPEECH,		"MPTRIGGER_SPEECH",		"speech_prog",		},
	{ MPTRIGGER_RAND,		"MPTRIGGER_RAND",		"rand_prog",		},
	{ MPTRIGGER_FIGHT,		"MPTRIGGER_FIGHT",		"fight_prog",		},
	{ MPTRIGGER_DEATH,		"MPTRIGGER_DEATH",		"death_prog",		},
	{ MPTRIGGER_HITPRCNT,	"MPTRIGGER_HITPRCNT",	"hitprcnt_prog",	},
	{ MPTRIGGER_ENTER,		"MPTRIGGER_ENTER",		"entry_prog",		},
	{ MPTRIGGER_GREET,		"MPTRIGGER_GREET",		"greet_prog",		},
	{ MPTRIGGER_GREET_ALL,	"MPTRIGGER_GREET_ALL",	"all_greet_prog",	},
	{ MPTRIGGER_GIVE,		"MPTRIGGER_GIVE",		"give_prog",		},
	{ MPTRIGGER_BRIBE,		"MPTRIGGER_BRIBE",		"bribe_prog",		},
	{ MPTRIGGER_TIME,		"MPTRIGGER_TIME",		"time_prog",		},
	{ MPTRIGGER_WEAR,		"MPTRIGGER_WEAR",		"wear_prog",		},
	{ MPTRIGGER_REMOVE,		"MPTRIGGER_REMOVE",		"remove_prog",		},
	{ MPTRIGGER_SAC,		"MPTRIGGER_SAC",		"sac_prog",			},
	{ MPTRIGGER_LOOK,		"MPTRIGGER_LOOK",		"look_prog",		},
	{ MPTRIGGER_EXA,		"MPTRIGGER_EXA",		"exa_prog",			},
	{ MPTRIGGER_ZAP,		"MPTRIGGER_ZAP",		"zap_prog",			},
	{ MPTRIGGER_GET,		"MPTRIGGER_GET",		"get_prog",			},
	{ MPTRIGGER_DROP,		"MPTRIGGER_DROP",		"drop_prog",		},
	{ MPTRIGGER_DAMAGE,		"MPTRIGGER_DAMAGE",		"damage_prog",		},
	{ MPTRIGGER_REPAIR,		"MPTRIGGER_REPAIR",		"repair_prog",		},
	{ MPTRIGGER_RANDIW,		"MPTRIGGER_RANDIW",		"randiw_prog",		},
	{ MPTRIGGER_SPEECHIW,	"MPTRIGGER_SPEECHIW",	"speechiw_prog",	},
	{ MPTRIGGER_PULL,		"MPTRIGGER_PULL",		"pull_prog",		},
	{ MPTRIGGER_PUSH,		"MPTRIGGER_PUSH",		"push_prog",		},
	{ MPTRIGGER_SLEEP,		"MPTRIGGER_SLEEP",		"sleep_prog",		},
	{ MPTRIGGER_REST,		"MPTRIGGER_REST",		"rest_prog",		},
	{ MPTRIGGER_LEAVE,		"MPTRIGGER_LEAVE",		"leave_prog",		},
	{ MPTRIGGER_USE,		"MPTRIGGER_USE",		"use_prog",			}
};
const int max_check_mptrigger = sizeof(code_mptrigger)/sizeof(CODE_DATA);


/*
 * Ritorna vero se la struttura di mudprog contiene un mudprog con il typo di trigger passato
 */
bool has_trigger( MPROG_DATA *mudprog, const int type )
{
	MPROG_DATA *mprg;

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "has_trigger: tipo di trigger passato errato: %d", type );
		return FALSE;
	}

	for ( mprg = mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type == type )
			return TRUE;
	}

	return FALSE;
}


bool mprog_keyword_check( const char *argu, const char *argl )
{
	char	*parg;
	char	*parglist;
	char	*start;
	char	*end;
	char	 arg[MIL];
	char	 arglist[MIL];
	char	 word[MIL];

	if ( !VALID_STR(argu) )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_keyword_check: argu è una stringa non valida" );
		return FALSE;
	}

	if ( !VALID_STR(argl) )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_keyword_check: arg è una stringa non valida" );
		return FALSE;
	}

	/* Prepara l'argomento che sarebbe il modello di confronto */
	strcpy( arg, strlower(argu) );
	strcpy( arg, strip_punct(arg) );
	parg = arg;

	/* Prepara l'argomento che deriva dall'argomento del trigger del mudprog */
	strcpy( arglist, strlower(argl) );
	strcpy( arglist, strip_punct(arglist) );
	parglist = arglist;

	/* Se c'è il carattere opzionale p significa che deve fare il check su tutta la stringa */
	if ( parglist[0] == 'p' && parglist[1] == ' ' )
	{
		parglist += 2;
		while ( (start = strstr(parg, parglist)) )
		{
			if ( (start == parg || *(start-1) == ' ')
			  && (*(end = start+strlen(parglist)) == ' '
			  || *end == '\r'
			  || *end == '\n'
			  || *end == '\0') )
			{
				return TRUE;
			}
			else
			{
				parg = start+1;
			}
		}
	}
	else
	{
		/* Altrimenti fa il check su almeno una parola della lista */
		parglist = one_argument( parglist, word );
		for ( ;  VALID_STR(word);  parglist = one_argument(parglist, word) )
		{
			while ( (start = strstr(parg, word)) )
			{
				if ( (start == parg || *(start-1) == ' ')
				  && (*(end = start+strlen(word)) == ' '
				  || *end == '\r'
				  || *end == '\n'
				  || *end == '\0') )
				{
					return TRUE;
				}
				else
				{
					parg = start +1;
				}
			}
		}
	}

/*	send_log( NULL, LOG_TRIGGER, "mprog_keyword_check: %s e %s non corrispondono", argu, argl );*/	/* (TT) da attivare nel caso di test */
	return FALSE;
}


/*
 * The next two routines are the basic trigger types.
 * Either trigger on a certain percent, or trigger on a keyword or word phrase.
 * To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type )
{
	MPROG_DATA *mprg;
	char	   *list = NULL;
	char	   *dupl = NULL;
	char	   *start;
	char	   *end;
	char		temp1[MSL];
	char		temp2[MIL];
	char		word[MIL];

	if ( !VALID_STR(arg) )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_wordlist_check: stringa arg non valida" );
		return;
	}

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_wordlist_check: mob è NULL (con arg = %s)", arg );
		return;
	}

	if ( !actor )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_wordlist_check: actor è NULL (con arg = %s e mob #%d)",
			arg, mob->pIndexData->vnum );
		return;
	}

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_wordlist_check: tipo di trigger passato errato: %d", type );
		return;
	}

	for ( mprg = mob->pIndexData->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != type )
			continue;

		/* Prepara la parte riguardante l'argomento del trigger mudprog */
		strcpy( temp1, strlower(mprg->arglist) );
		strcpy( temp1, strip_punct(temp1) );
		list = temp1;

		/* Prepara la parte riguardante il modello da confrontare */
		strcpy( temp2, strlower(arg) );
		strcpy( temp2, strip_punct(temp2) );
		dupl = temp2;

		if ( list[0] == 'p' && list[1] == ' ' )
		{
			list += 2;
			while ( (start = strstr(dupl, list)) )
			{
				if ( (start == dupl || *(start-1) == ' ' )
				  && (*(end = start+strlen(list)) == ' '
				  || *end == '\r'
				  || *end == '\n'
				  || *end == '\0') )
				{
					mprog_driver( mprg, mob, actor, obj, vo );
					break;
				}
				else
				{
					dupl = start+1;
				}
			}
		}
		else
		{
			list = one_argument( list, word );
			for ( ;  VALID_STR(word);  list = one_argument(list, word) )
			{
				while ( (start = strstr(dupl, word)) )
				{
					if ( (start == dupl || *(start-1) == ' ')
					  && (*(end = start+strlen(word)) == ' '
					  || *end == '\r'
					  || *end == '\n'
					  || *end == '\0') )
					{
						mprog_driver( mprg, mob, actor, obj, vo );
						break;
					}
					else
					{
						dupl = start+1;
					}
				}
			}
		}
	} /* chiude il for */

/*	send_log( NULL, LOG_TRIGGER, "mprog_wordlist_check: %s e %s non corrispondono", dupl, list );*/	/* (TT) da attivare nel caso di test */
}


void mprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type )
{
	MPROG_DATA *mprg;

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_percent_check: mob è NULL" );
		return;
	}

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_percent_check: tipo di trigger passato errato: %d", type );
		return;
	}

	for ( mprg = mob->pIndexData->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != type )
			continue;

		if ( !is_number(mprg->arglist) )
		{
			send_log( NULL, LOG_TRIGGER, "mprog_percent_check: l'argomento del trigger non è numerico: %s", mprg->arglist );
			return;
		}

		if ( number_percent() <= atoi(mprg->arglist) )
		{
			mprog_driver( mprg, mob, actor, obj, vo );

			if ( type != MPTRIGGER_GREET && type != MPTRIGGER_GREET_ALL )
				break;
		}
	}
}


void mprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type )
{
	MPROG_DATA *mprg;
	bool		trigger_time;

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_time_check: mob è NULL" );
		return;
	}

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_time_check: tipo di trigger passato errato: %d", type );
		return;
	}

	for ( mprg = mob->pIndexData->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( !is_number(mprg->arglist) )
		{
			send_log( NULL, LOG_TRIGGER, "mprog_time_check: l'argomento del trigger non è numerico: %s", mprg->arglist );
			return;
		}

		trigger_time = ( calendar.hour == atoi(mprg->arglist) );

		if ( !trigger_time )
		{
			if ( mprg->triggered )
				mprg->triggered = FALSE;

			continue;
		}

		if ( mprg->type != type )
			continue;

		if ( !mprg->triggered )
		{
			mprg->triggered = TRUE;
			mprog_driver( mprg, mob, actor, obj, vo );
		}
	}
}


void mob_act_add( CHAR_DATA *mob )
{
	struct act_prog_data	*runner;
	struct act_prog_data	*tmp_mal;

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mob_act_add: mob è NULL" );
		return;
	}

	for ( runner = mob_act_list;  runner;  runner = runner->next )
	{
		if ( runner->vo == mob )
			return;
	}

	CREATE( runner, struct act_prog_data, 1 );
	runner->vo = mob;
	runner->next = NULL;

	/* The head of the list is being changed in
	 *	update_aggr, So append to the end of the list instead. */
	if ( mob_act_list )
	{
		tmp_mal = mob_act_list;

		while ( tmp_mal->next )
			tmp_mal = tmp_mal->next;

		/* put at the end */
		tmp_mal->next = runner;
	}
	else
	{
		mob_act_list = runner;
	}
}


/* The triggers.. These are really basic, and since most appear only
 *  once in the code (hmm. i think they all do) it would be more efficient
 *  to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it easier
 *  to see what they look like. If you do substitute them back in, make sure
 *  you remember to modify the variable names to the ones in the trigger calls.
 */
void mprog_act_trigger( char *buf, CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
	MPROG_ACT_LIST *tmp_act;
	MPROG_ACT_LIST *tmp_mal;
	MPROG_DATA	   *mprg;
	bool			found = FALSE;

	if ( !VALID_STR(buf) )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_act_trigger: stringa buf non valida" );
		return;
	}

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_act_trigger: mob è NULL (con buf = %s)", buf );
		return;
	}

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_act_trigger: ch è NULL (con buf = %s e mob #%d)",
			buf, mob->pIndexData->vnum);
		return;
	}

	if ( IS_PG(mob) )
		return;

	if ( !has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_ACT) )
		return;

	/* Don't let a mob trigger itself, nor one instance of a mob trigger another instance */
	if ( IS_MOB(ch) && ch->pIndexData == mob->pIndexData )
		return;

	/* make sure this is a matching trigger */
	for ( mprg = mob->pIndexData->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != MPTRIGGER_ACT )
			continue;

		if ( mprog_keyword_check(buf, mprg->arglist) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
		return;

	CREATE( tmp_act, MPROG_ACT_LIST, 1 );
	/* Losing the head of the list */
	if ( mob->mpactnum > 0 )
	{
		tmp_mal = mob->mpact;

		while ( tmp_mal->next != NULL )
			tmp_mal = tmp_mal->next;

		/* Put at the end */
		tmp_mal->next = tmp_act;
	}
	else
	{
		mob->mpact = tmp_act;
	}

	tmp_act->next = NULL;
	tmp_act->buf = str_dup( buf );
	tmp_act->ch = ch;
	tmp_act->obj = obj;
	tmp_act->vo = vo;

	mob->mpactnum++;
	mob_act_add( mob );
}


void mprog_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{
	MPROG_DATA *mprg;
	OBJ_DATA   *obj;
	char		buf[MSL];

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_bribe_trigger: mob è NULL" );
		return;
	}

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_bribe_trigger: ch è NULL (con mob #%d)", mob->pIndexData->vnum );
		return;
	}

	if ( amount < 0 )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_bribe_trigger: ammontare monete negativo: %d", amount );
		return;
	}

	if ( IS_PG(mob) )
		return;

	if ( !can_see(mob, ch) )
		return;

	if ( !has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_BRIBE) )
		return;

	/* Don't let a mob trigger itself, nor one instance of a mob
	 * trigger another instance. */
	if ( IS_MOB(ch) && ch->pIndexData == mob->pIndexData )
		return;

	obj = make_object( get_obj_proto(NULL, VNUM_OBJ_MONEY_SOME), 0 );
	sprintf( buf, obj->short_descr, amount );
	DISPOSE( obj->short_descr );
	obj->short_descr = str_dup( buf );
	obj->money->qnt = amount;

	obj = obj_to_char( obj, mob );
	mob->gold -= amount;

	for ( mprg = mob->pIndexData->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != MPTRIGGER_BRIBE )
			continue;

		if ( !is_number(mprg->arglist) )
		{
			send_log( NULL, LOG_TRIGGER, "mprog_bribe_trigger: l'argomento del trigger di bribe non è un numero: %s", mprg->arglist );
			return;
		}

		if ( amount >= atoi(mprg->arglist) )
		{
			mprog_driver( mprg, mob, ch, obj, NULL );
			break;
		}
	}
}


void mprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *mob )
{
	if ( !killer )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_death_trigger: killer è NULL" );
		return;
	}

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_death_trigger: mob è NULL (com killer %s)",
			IS_PG(killer) ? killer->name : killer->short_descr );
		return;
	}

	if ( IS_PG(mob) )
		return;

	if ( killer == mob )
		return;

	if ( !has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_DEATH) )
		return;

	mob->position = POSITION_STAND;
	mprog_percent_check( mob, killer, NULL, NULL, MPTRIGGER_DEATH );
	mob->position = POSITION_DEAD;

	death_cry( mob );
}


void mprog_entry_trigger( CHAR_DATA *mob )
{
	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_entry_trigger: mob è NULL" );
		return;
	}

	if ( IS_PG(mob) )
		return;

	if ( has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_ENTER) )
		mprog_percent_check( mob, NULL, NULL, NULL, MPTRIGGER_ENTER );
}


void mprog_fight_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_fight_trigger: mob è NULL" );
		return;
	}

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_fight_trigger: ch è NULL (con mob #%d)",
			mob->pIndexData->vnum );
		return;
	}

	if ( IS_PG(mob) )
		return;

	if ( has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_FIGHT) )
		mprog_percent_check( mob, ch, NULL, NULL, MPTRIGGER_FIGHT );
}


void mprog_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{
	MPROG_DATA *mprg;
	char        buf[MIL];

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_give_trigger: mob è NULL" );
		return;
	}

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_give_trigger: ch è NULL (con mob #%d)",
			mob->pIndexData->vnum );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_give_trigger: obj è NULL (con mob #%d)",
			mob->pIndexData->vnum );
		return;
	}

	if ( IS_PG(mob) )
		return;

	if ( !can_see(mob, ch) )
		return;

	if ( !has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_GIVE) )
		return;

	/* Don't let a mob trigger itself, nor one instance of a mob
	 * trigger another instance. */
	if ( IS_MOB(ch) && ch->pIndexData == mob->pIndexData )
		return;

	for ( mprg = mob->pIndexData->first_mudprog;  mprg;  mprg = mprg->next )
	{
		one_argument( mprg->arglist, buf );

		if ( mprg->type != MPTRIGGER_GIVE )
			continue;

		if ( is_name(mprg->arglist, obj->name) || !str_cmp_all(buf, FALSE) )
		{
			mprog_driver( mprg, mob, ch, obj, NULL );
			break;
		}
	}
}


void mprog_greet_trigger( CHAR_DATA *ch )
{
	CHAR_DATA *vmob;
	CHAR_DATA *vmob_next;

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_greet_trigger: ch è NULL" );
		return;
	}

/*	send_log( NULL, LOG_TRIGGER, "mprog_greet_trigger -> %s", ch->name );*/		/* (TT) da usare in caso di test */

	for ( vmob = ch->in_room->first_person;  vmob;  vmob = vmob_next )
	{
		vmob_next = vmob->next_in_room;

		if ( IS_PG(vmob) )			continue;
		if ( !can_see(vmob, ch) )	continue;
		if ( vmob->fighting )		continue;
		if ( !is_awake(vmob) )		continue;

		/* Don't let a mob trigger itself, nor one instance of a mob
		 * trigger another instance. */
		if ( IS_MOB(ch) && ch->pIndexData == vmob->pIndexData )
			continue;

		if ( has_trigger(vmob->pIndexData->first_mudprog, MPTRIGGER_GREET) )
			mprog_percent_check( vmob, ch, NULL, NULL, MPTRIGGER_GREET );
		else if ( has_trigger(vmob->pIndexData->first_mudprog, MPTRIGGER_GREET_ALL) )
			mprog_percent_check( vmob, ch, NULL, NULL, MPTRIGGER_GREET_ALL );
	}
}


void mprog_hitprcnt_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
	MPROG_DATA *mprg;

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_hitprcnt_trigger: mob è NULL" );
		return;
	}

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_hitprcnt_trigger: ch è NULL (con mob #%d)",
			mob->pIndexData->vnum );
		return;
	}

	if ( IS_PG(mob) )
		return;

	if ( !has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_HITPRCNT) )
		return;

	for ( mprg = mob->pIndexData->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != MPTRIGGER_HITPRCNT )
			continue;

		if ( !is_number(mprg->arglist) )
		{
			send_log( NULL, LOG_TRIGGER, "mprog_hitprcnt_trigger: l'argomento del trigger hitprcnt non è numerico: %s", mprg->arglist );
			return;
		}

		if ( get_percent(mob->points[POINTS_LIFE], mob->points_max[POINTS_LIFE]) < atoi(mprg->arglist) )
		{
			mprog_driver( mprg, mob, ch, NULL, NULL );
			break;
		}
	}
}


void mprog_random_trigger( CHAR_DATA *mob )
{
	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_random_trigger: mob è NULL" );
		return;
	}

	if ( has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_RAND) )
		mprog_percent_check( mob, NULL, NULL, NULL, MPTRIGGER_RAND );
}


void mprog_time_trigger( CHAR_DATA *mob )
{
	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "mprog_time_trigger: mob è NULL" );
		return;
	}

	if ( has_trigger(mob->pIndexData->first_mudprog, MPTRIGGER_TIME) )
		mprog_time_check( mob, NULL, NULL, NULL, MPTRIGGER_TIME );
}


void mprog_speech_trigger( char *txt, CHAR_DATA *actor )
{
	CHAR_DATA *vmob;

	for ( vmob = actor->in_room->first_person;  vmob;  vmob = vmob->next_in_room )
	{
		if ( IS_PG(vmob) )														continue;
		if ( !has_trigger(vmob->pIndexData->first_mudprog, MPTRIGGER_SPEECH) )	continue;
		if ( IS_MOB(actor) && actor->pIndexData == vmob->pIndexData )			continue;

		mprog_wordlist_check( txt, vmob, actor, NULL, NULL, MPTRIGGER_SPEECH );
	}
}


void init_supermob( void )
{
	ROOM_DATA		*office;
	MOB_PROTO_DATA	*imob;

	imob = get_mob_index( NULL, VNUM_MOB_SUPERMOB );
	if ( !imob )
	{
		send_log( NULL, LOG_SUPERMOB, "init_supermob: vnum di supermob %d inesistente", VNUM_MOB_SUPERMOB );
		return;
	}

	office = get_room_index( NULL, VNUM_ROOM_POLY );
	if ( !office )
	{
		send_log( NULL, LOG_SUPERMOB, "init_supermob: la stanza per il supermob %d è inesistente!", VNUM_ROOM_POLY );
		return;
	}

	supermob = make_mobile( imob );
	char_to_room( supermob, office );
}


/*
 *  Mudprogram additions begin here
 */
void set_supermob( OBJ_DATA *obj )
{
	ROOM_DATA	*room;
	OBJ_DATA	*in_obj;
	CHAR_DATA	*mob;
	char		 buf[200];

	if ( !supermob )
	{
		MOB_PROTO_DATA	*imob;

		send_log( NULL, LOG_SUPERMOB, "Il supermob non è stato inizializzato! Verrà inizializza ora.." );
		imob = get_mob_index( NULL, VNUM_MOB_SUPERMOB );
		if ( !imob )
		{
			send_log( NULL, LOG_SUPERMOB, "set_supermob: vnum di supermob %d inesistente", VNUM_MOB_SUPERMOB );
			return;
		}
		supermob = make_mobile( imob );
	}

	mob = supermob;		/* per debuggare */

	if ( obj == NULL )
	{
		send_log( NULL, LOG_TRIGGER, "set_supermob: l'oggetto passato è NULL" );
		return;
	}

	for ( in_obj = obj;  in_obj->in_obj;  in_obj = in_obj->in_obj )
		;

	if ( in_obj->carried_by )
		room = in_obj->carried_by->in_room;
	else
		room = obj->in_room;

	if ( room == NULL )
	{
		send_log( NULL, LOG_SUPERMOB, "set_supermob: la stanza ove il supermob deve interagire è NULL (obj: #%d)",
			obj->vnum );
		return;
	}

	DISPOSE( supermob->short_descr );
	supermob->short_descr = str_dup( obj->short_descr );

	/* Added to allow bug messages to show the vnum
	 * of the object, and not just supermob's vnum */
	sprintf( buf, "Object #%d", obj->vnum );
	DISPOSE( supermob->description );
	supermob->description = str_dup( buf );

	char_from_room( supermob );
	char_to_room( supermob, room );
}


void release_supermob( void )
{
	ROOM_DATA	*office;

	office = get_room_index( NULL, VNUM_ROOM_POLY );
	if ( !office )
	{
		send_log( NULL, LOG_SUPERMOB, "release_supermob: la stanza per il supermob %d è inesistente!", VNUM_ROOM_POLY );
		return;
	}

	char_from_room( supermob );
	char_to_room( supermob, office );
}


bool oprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type )
{
	MPROG_DATA *mprg;
	bool		executed = FALSE;

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_percent_check: mob è NULL" );
		return FALSE;
	}

	if ( !actor )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_percent_check: actor è NULL (con mob #%d)", mob->pIndexData->vnum );
		return FALSE;
	}

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_percent_check: tipo di trigger passato errato: %d", type );
		return FALSE;
	}

	for ( mprg = obj->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != type )
			continue;

		if ( !is_number(mprg->arglist) )
		{
			send_log( NULL, LOG_TRIGGER, "oprog_percent_check: l'argoment del trigger non è numerico: %s", mprg->arglist );
			return FALSE;
		}

		if ( number_percent() <= atoi(mprg->arglist) )
		{
			executed = TRUE;
			mprog_driver( mprg, mob, actor, obj, vo );
			if ( type != MPTRIGGER_GREET )
				break;
		}
	}

	return executed;
}


void oprog_greet_trigger( CHAR_DATA *ch )
{
	OBJ_DATA *vobj;

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_greet_trigger: ch è NULL" );
		return;
	}

	for ( vobj = ch->in_room->first_content;  vobj;  vobj = vobj->next_content )
	{
		if ( has_trigger(vobj->first_mudprog, MPTRIGGER_GREET) )
		{
			set_supermob( vobj );
			oprog_percent_check( supermob, ch, vobj, NULL, MPTRIGGER_GREET );
			release_supermob( );
		}
	}
}


void oprog_speech_trigger( char *txt, CHAR_DATA *ch )
{
	OBJ_DATA *vobj;

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_speech_trigger: ch è NULL" );
		return;
	}

	/* supermob is set and released in oprog_wordlist_check */
	for ( vobj = ch->in_room->first_content;  vobj;  vobj = vobj->next_content )
	{
		if ( has_trigger(vobj->first_mudprog, MPTRIGGER_SPEECH) )
			oprog_wordlist_check(txt, supermob, ch, vobj, NULL, MPTRIGGER_SPEECH, vobj);
	}
}


/*
 * Called at top of update_obj
 * make sure to put an if (!obj) continue
 * after it
 */
void oprog_random_trigger( OBJ_DATA *obj )
{
	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_random_trigger: obj è NULL" );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_RAND) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, NULL, obj, NULL, MPTRIGGER_RAND );
	release_supermob( );
}


/*
 * In wear_obj, between each successful equip_char
 * the subsequent return
 */
void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_wear_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_wear_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_WEAR) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_WEAR );
	release_supermob( );
}


bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict, OBJ_DATA *targ, void *vo )
{
	bool executed = FALSE;

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_use_trigger: ch è NULL" );
		return FALSE;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_wear_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return FALSE;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_USE) )
		return FALSE;

	set_supermob( obj );

	if ( obj->type == OBJTYPE_STAFF
	  || obj->type == OBJTYPE_WAND
	  || obj->type == OBJTYPE_SCROLL )
	{
		if ( vict )
			executed = oprog_percent_check( supermob, ch, obj, vict, MPTRIGGER_USE );
		else
			executed = oprog_percent_check( supermob, ch, obj, targ, MPTRIGGER_USE );
	}
	else
	{
		executed = oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_USE );
	}

	release_supermob( );

	return executed;
}


/*
 * Call in remove_obj, right after unequip_char
 * do a if (!ch) return right after, and return TRUE (?)
 * if !ch
 */
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_remove_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_remove_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_REMOVE) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_REMOVE );
	release_supermob( );
}


/*
 * Call in do_sac, right before free_object
 */
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_sac_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_sac_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_SAC) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_SAC );
	release_supermob( );
}


/*
 * Call in do_get, right before check_for_trap
 * do a if (!ch) return right after
 */
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_get_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_get_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_GET) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_GET );
	release_supermob( );
}


/*
 * Called in damage_obj in act_obj.c
 */
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_damage_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_damage_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_DAMAGE) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_DAMAGE );
	release_supermob( );
}


/*
 * Called in do_repair in shops.c
 */
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_repair_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_repair_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_REPAIR) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_REPAIR );
	release_supermob( );
}


/*
 * Call twice in do_drop, right after the act( AT_ACTION,...)
 * do a if (!ch) return right after
 */
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_drop_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_drop_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_DROP) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_DROP );
	release_supermob( );
}


/*
 * Call towards end of do_examine, right before check_for_trap
 */
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_examine_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_examine_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_EXA) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_EXA );
	release_supermob( );
}


/*
 * Call in fight.c, group_gain, after (?) the obj_to_room
 */
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_zap_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_zap_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_ZAP) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_ZAP );
	release_supermob( );
}


/*
 * Call in levers.c, towards top of do_push_or_pull
 *  see note there
 */
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_pull_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_pull_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_PULL) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_PULL );
	release_supermob( );
}


/*
 * Call in levers.c, towards top of do_push_or_pull
 *  see note there
 */
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_push_trigger: ch è NULL" );
		return;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_push_trigger: obj è NULL (con ch %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return;
	}

	if ( !has_trigger(obj->first_mudprog, MPTRIGGER_PUSH) )
		return;

	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, MPTRIGGER_PUSH );
	release_supermob( );
}


void obj_act_add( OBJ_DATA *obj )
{
	struct act_prog_data *runner;
	struct act_prog_data *tmp_oal;

	if ( !obj )
	{
		send_log( NULL, LOG_TRIGGER, "obj_act_add: obj è NULL" );
		return;
	}

	for ( runner = obj_act_list;  runner;  runner = runner->next )
	{
		if ( runner->vo == obj )
			return;
	}

	CREATE( runner, struct act_prog_data, 1 );
	runner->vo = obj;
	runner->next = NULL;
	/* The head of the list is being changed in
	 *	update_obj_act, So append to the end of the list instead. */
	if ( obj_act_list )
	{
		tmp_oal = obj_act_list;

		while ( tmp_oal->next )
			tmp_oal = tmp_oal->next;

		/* put at the end */
		tmp_oal->next = runner;
	}
	else
	{
		obj_act_list = runner;
	}
}


void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
	MPROG_ACT_LIST *tmp_act;
	MPROG_ACT_LIST *tmp_mal;;

	if ( !VALID_STR(buf) )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_act_trigger: stringa non valida per buf" );
		return;
	}

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "oprog_act_trigger: ch è NULL (con buf = %s)", buf );
		return;
	}

	if ( !has_trigger(mobj->first_mudprog, MPTRIGGER_ACT) )
		return;

	CREATE( tmp_act, MPROG_ACT_LIST, 1 );

	if ( mobj->mpactnum > 0 )
	{
		tmp_mal = mobj->mpact;

		while ( tmp_mal->next != NULL )
		tmp_mal = tmp_mal->next;

		/* Put at the end */
		tmp_mal->next = tmp_act;
	}
	else
	{
		mobj->mpact = tmp_act;
	}

	tmp_act->next = NULL;
	tmp_act->buf = str_dup( buf );
	tmp_act->ch = ch;
	tmp_act->obj = obj;
	tmp_act->vo = vo;

	mobj->mpactnum++;
	obj_act_add( mobj );
}


void oprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo,
	int type, OBJ_DATA *iobj )
{
	MPROG_DATA *mprg;
	char	   *list = NULL;
	char	   *dupl = NULL;
	char	   *start;
	char	   *end;
	char		temp1[MSL];
	char		temp2[MIL];
	char		word[MIL];

	for ( mprg = iobj->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != type )
			continue;

		/* Prepara l'argomento del trigger mudprog */
		strcpy( temp1, strlower(mprg->arglist) );
		strcpy( temp1, strip_punct(temp1) );
		list = temp1;

		/* Prepara l'argomento modello di confronto */
		strcpy( temp2, strlower(arg) );
		strcpy( temp2, strip_punct(temp2) );
		dupl = temp2;

		if ( list[0] == 'p' && list[1] == ' ' )
		{
			list += 2;
			while ( (start = strstr(dupl, list)) )
			{
				if ( (start == dupl || *(start-1) == ' ')
				  && (*(end = start+strlen(list)) == ' '
				  || *end == '\r'
				  || *end == '\n'
				  || *end == '\0') )
				{
					set_supermob( iobj );
					mprog_driver( mprg, mob, actor, obj, vo );
					release_supermob( );
					break;
				}
				else
				{
					dupl = start+1;
				}
			}
		}
		else
		{
			list = one_argument( list, word );
			for ( ;  VALID_STR(word);  list = one_argument(list, word) )
			{
				while ( (start = strstr(dupl, word)) )
				{
					if ( (start == dupl || *(start-1) == ' ' )
					  && ( *(end = start+strlen(word)) == ' '
					  || *end == '\r'
					  || *end == '\n'
					  || *end == '\0') )
					{
						set_supermob( iobj );
						mprog_driver( mprg, mob, actor, obj, vo );
						release_supermob( );
						break;
					}
					else
					{
						dupl = start+1;
					}
				}
			}
		}
	}

/*	send_log( NULL, LOG_TRIGGER, "oprog_wordlist_check: %s e %s non corrispondono", dupl, list );*/	/* (TT) da attivare nel caso di test */
}


/**
 * Room_prog support starts here
 */

void rset_supermob( ROOM_DATA *room )
{
	char buf[50];

	if ( !room )
	{
		send_log( NULL, LOG_TRIGGER, "rset_supermob: la stanza passata è NULL" );
		return;
	}

	DISPOSE( supermob->short_descr );
	supermob->short_descr = str_dup( room->name );
	DISPOSE( supermob->name );
	supermob->name = str_dup( room->name );

	/* Added to allow bug messages to show the vnum
	 *  of the room, and not just supermob's vnum */
	sprintf( buf, "Room #%d", room->vnum );
	DISPOSE( supermob->description );
	supermob->description = str_dup( buf );

	char_from_room( supermob );
	char_to_room( supermob, room);
}


void rprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type )
{
	MPROG_DATA * mprg;

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_percent_check: mob è NULL" );
		return;
	}

	if ( !actor )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_percent_check: actor è NULL (con mob #%d)", mob->pIndexData->vnum );
		return;
	}

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_percent_check: tipo di trigger passato errato: %d", type );
		return;
	}

	if ( !mob->in_room )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_percent_check: il mob #%d non si trova in nessuna locazione",
			mob->pIndexData->vnum );
		return;
	}

	for ( mprg = mob->in_room->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != type )
			continue;

		if ( !is_number(mprg->arglist) )
		{
			send_log( NULL, LOG_TRIGGER, "rprog_percent_check: l'argomento del trigger non è numerico: %s", mprg->arglist );
			return;
		}

		if ( number_percent() <= atoi(mprg->arglist) )
		{
			mprog_driver( mprg, mob, actor, obj, vo );

			if ( type != MPTRIGGER_ENTER )
				break;
		}
	}
}


/*
 * Room act prog updates
 */
void room_act_add( ROOM_DATA *room )
{
	struct act_prog_data *runner;
	struct act_prog_data *tmp_ral;

	if ( !room )
	{
		send_log( NULL, LOG_TRIGGER, "room_act_add: room è NULL" );
		return;
	}

	for ( runner = room_act_list;  runner;  runner = runner->next )
	{
		if ( runner->vo == room )
			return;
	}

	CREATE( runner, struct act_prog_data, 1 );
	runner->vo = room;
	runner->next = NULL;

	/*
	 * The head of the list is being changed in
	 * update_room_act, So append to the end of the list
	 * instead. -Druid
	 */
	if ( room_act_list )
	{
		tmp_ral = room_act_list;

		while ( tmp_ral->next )
			tmp_ral = tmp_ral->next;

		/* put at the end */
		tmp_ral->next = runner;
	}
	else
	{
		room_act_list = runner;
	}
}


void rprog_act_trigger( char *buf, ROOM_DATA *room, CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
	MPROG_ACT_LIST *tmp_act;
	MPROG_ACT_LIST *tmp_mal;

	if ( !VALID_STR(buf) )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_act_trigger: stringa non valida per buf" );
		return;
	}

	if ( !room )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_push_trigger: room è NULL (con buf = %s)", buf );
		return;
	}

	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_push_trigger: ch è NULL (con buf = %s e room #%d)",
			buf, room->vnum );
		return;
	}

	if ( !has_trigger(room->first_mudprog, MPTRIGGER_ACT) )
		return;

	CREATE( tmp_act, MPROG_ACT_LIST, 1 );

	/* Losing the head of the list */
	if ( room->mpactnum > 0 )
	{
		tmp_mal = room->mpact;

		while( tmp_mal->next )
			tmp_mal = tmp_mal->next;

		/* Put at the end */
		tmp_mal->next = tmp_act;
	}
	else
	{
		room->mpact = tmp_act;
	}

	tmp_act->next = NULL;
	tmp_act->buf = str_dup( buf );
	tmp_act->ch = ch;
	tmp_act->obj = obj;
	tmp_act->vo = vo;

	room->mpactnum++;
	room_act_add( room );
}


void rprog_leave_trigger( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_leave_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_LEAVE) )
		return;

	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, MPTRIGGER_LEAVE );
	release_supermob( );
}


void rprog_enter_trigger( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_enter_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_ENTER) )
	{
		rset_supermob( ch->in_room );
		rprog_percent_check( supermob, ch, NULL, NULL, MPTRIGGER_ENTER );
		release_supermob( );
	}
}


void rprog_sleep_trigger( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_sleep_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_SLEEP) )
		return;

	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, MPTRIGGER_SLEEP );
	release_supermob( );
}


void rprog_rest_trigger( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_rest_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_REST) )
		return;

	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, MPTRIGGER_REST );
	release_supermob( );
}


void rprog_rfight_trigger( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_rfight_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_FIGHT) )
		return;

	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, MPTRIGGER_FIGHT );
	release_supermob( );
}


void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_death_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_DEATH) )
		return;

	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, MPTRIGGER_DEATH );
	release_supermob( );
}


void rprog_speech_trigger( char *txt, CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_speech_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_SPEECH) )
		return;

	/* supermob is set and released in rprog_wordlist_check */
	rprog_wordlist_check( txt, supermob, ch, NULL, NULL, MPTRIGGER_SPEECH, ch->in_room );
}


void rprog_random_trigger( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_random_trigger: ch è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_RAND) )
		return;

	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, MPTRIGGER_RAND );
	release_supermob( );
}


void rprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo,
	int type, ROOM_DATA *room )
{
	MPROG_DATA *mprg;
	char	   *list = NULL;
	char	   *dupl = NULL;
	char	   *start;
	char	   *end;
	char		temp1[MSL];
	char		temp2[MIL];
	char		word [MIL];

	if ( !VALID_STR(arg) )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_wordlist_check: stringa arg non valida" );
		return;
	}

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_wordlist_check: mob è NULL (con arg = %s)", arg );
		return;
	}

	if ( !actor )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_wordlist_check: actor è NULL (con arg = %s e mob #%d)",
			arg, mob->pIndexData->vnum );
		return;
	}

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_wordlist_check: tipo di trigger passato errato: %d", type );
		return;
	}

	if ( actor && !char_died(actor) && actor->in_room )
		room = actor->in_room;

	for ( mprg = room->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( mprg->type != type )
			continue;

		/* Prepara la parte riguardante il trigger del mudprog */
		strcpy( temp1, strlower(mprg->arglist) );
		strcpy( temp1, strip_punct(temp1) );
		list = temp1;

		/* Prepara la parte riguardant il modello da confrontare */
		strcpy( temp2, strlower(arg) );
		strcpy( temp2, strip_punct(temp2) );
		dupl = temp2;

		if ( list[0] == 'p' && list[1] == ' ' )
		{
			list += 2;
			while ( (start = strstr(dupl, list)) )
			{
				if ( (start == dupl || *(start-1) == ' ' )
				  && (*(end = start+strlen(list)) == ' '
				  || *end == '\r'
				  || *end == '\n'
				  || *end == '\0') )
				{
					rset_supermob( room );
					mprog_driver( mprg, mob, actor, obj, vo );
					release_supermob( );
					break;
				}
				else
				{
					dupl = start+1;
				}
			}
		}
		else
		{
			list = one_argument( list, word );
			for ( ;  VALID_STR(word);  list = one_argument(list, word) )
			{
				while ( (start = strstr(dupl, word)) )
				{
					if ( (start == dupl || *(start-1) == ' ')
					  && (*(end = start+strlen(word)) == ' '
					  || *end == '\r'
					  || *end == '\n'
					  || *end == '\0') )
					{
						rset_supermob( room );
						mprog_driver( mprg, mob, actor, obj, vo );
					release_supermob( );
						break;
					}
					else
						dupl = start+1;
				}
			}
		}
	} /* chiude il for */

/*	send_log( NULL, LOG_TRIGGER, "rprog_wordlist_check: %s e %s non corrispondono", dupl, list );*/	/* (TT) da attivare nel caso di test */
}


void rprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type )
{
	ROOM_DATA	*room = (ROOM_DATA *) vo;
	MPROG_DATA	*mprg;
	bool		 trigger_time;

	if ( !mob )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_time_check: mob è NULL" );
		return;
	}

	if ( !actor )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_time_check: actor è NULL (con mob #%d)", mob->pIndexData->vnum );
		return;
	}

	if ( type < 0 && type >= MAX_MPTRIGGER )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_time_check: tipo di trigger passato errato: %d", type );
		return;
	}

	for ( mprg = room->first_mudprog;  mprg;  mprg = mprg->next )
	{
		if ( !is_number(mprg->arglist) )
		{
			send_log( NULL, LOG_TRIGGER, "rprog_time_check: l'argomento del trigger non è numerico: %s", mprg->arglist );
			return;
		}

		trigger_time = ( calendar.hour == atoi(mprg->arglist) );

		if ( trigger_time == FALSE )
		{
			if ( mprg->triggered )
				mprg->triggered = FALSE;

			continue;
		}

		if ( mprg->type != type )
			continue;

		if ( !mprg->triggered )
		{
			mprg->triggered = TRUE;
			mprog_driver( mprg, mob, actor, obj, vo );
		}
	}
}


void rprog_time_trigger( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_TRIGGER, "rprog_time_trigger: mob è NULL" );
		return;
	}

	if ( !has_trigger(ch->in_room->first_mudprog, MPTRIGGER_TIME) )
		return;

	rset_supermob( ch->in_room );
	rprog_time_check( supermob, NULL, NULL, ch->in_room, MPTRIGGER_TIME );
	release_supermob( );
}


void update_room_act( void )
{
	struct act_prog_data *runner;
	MPROG_ACT_LIST		 *mpact;

	while ( (runner = room_act_list) )
	{
		ROOM_DATA *room = runner->vo;

		while ( (mpact = room->mpact) )
		{
			if ( mpact->ch->in_room == room )
			{
				rprog_wordlist_check( mpact->buf, supermob, mpact->ch, mpact->obj,
					mpact->vo, MPTRIGGER_ACT, room );
			}

			room->mpact = mpact->next;
			DISPOSE( mpact->buf );
			DISPOSE( mpact );
		}

		room->mpact = NULL;
		room->mpactnum = 0;
		room_act_list = runner->next;
		DISPOSE( runner );
	}
}


void update_obj_act( void )
{
	struct act_prog_data *runner;
	MPROG_ACT_LIST		 *mpact;

	while ( (runner = obj_act_list) )
	{
		OBJ_DATA *obj = runner->vo;

		while ( (mpact = obj->mpact) )
		{
			oprog_wordlist_check( mpact->buf, supermob, mpact->ch, mpact->obj, mpact->vo, MPTRIGGER_ACT, obj );

			obj->mpact = mpact->next;
			DISPOSE( mpact->buf );
			DISPOSE( mpact );
		}

		obj->mpact = NULL;
		obj->mpactnum = 0;
		obj_act_list = runner->next;
		DISPOSE( runner );
	}
}
