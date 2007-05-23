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
 >								Modulo delle erbe						   	<
\****************************************************************************/


/* (FF) in futuro questo modulo allargarlo agli ingredienti per l'alchimista, le erbe
 *	farla una tipologia di ingrediente, forse anche i liquidi? forse anche i pezzi per
 *	cucinare il cibo? da pensare
 */

#include "mud.h"
#include "fread.h"
#include "herb.h"
#include "mprog.h"


/*
 * Definizioni locali
 */
#define HERB_FILE	"../tables/herbs.dat"		/* Tabella delle erbe */


/*
 * Tabella di codici della flag di pipa
 */
const CODE_DATA code_pipe[] =
{
	{ PIPE_TAMPED,		"PIPE_TAMPED",		"tamped"		},
	{ PIPE_LIT,			"PIPE_LIT",			"lit"			},
	{ PIPE_HOT,			"PIPE_HOT",			"hot"			},
	{ PIPE_DIRTY,		"PIPE_DIRTY",		"dirty"			},
	{ PIPE_FILTHY,		"PIPE_FILTHY",		"filthy"		},
	{ PIPE_GOINGOUT,	"PIPE_GOINGOUT",	"goingout"		},
	{ PIPE_BURNT,		"PIPE_BURNT",		"burnt"			},
	{ PIPE_FULLOFASH,	"PIPE_FULLOFASH",	"fullofhash"	}
};
const int max_check_pipe = sizeof(code_pipe)/sizeof(CODE_DATA);


/*
 * Variabili locali
 */
int			 top_herb = 0;
SKILL_DATA	*table_herb[MAX_HERB];


/*
 * Legge una sezione di HERB
 * (FF) rifare un fread_herb appena creata la struttura per le erbe-ingredienti-reagenti
 */
void fread_herb( MUD_FILE *fp )
{
	fread_skill( fp, "HERB" );
}


/*
 * Carica e controlla tutte le erbe
 */
void load_herbs( void )
{
	fread_section( HERB_FILE, "HERB", fread_herb, FALSE );

	/* (FF) Controlla le erbe dopo il caricamento */
}


/*
 * Libera dalla memoria tutte le erbe
 */
void free_all_herbs( void )
{
	int x;

	for ( x = 0;  x < MAX_HERB;  x++ )
	{
		if ( !table_herb[x] )
			continue;
		free_skill( table_herb[x] );
		top_herb--;
	}

	if ( top_herb != 0 )
		send_log( NULL, LOG_BUG, "free_all_herbs: top_herb non è 0 dopo aver liberato le erbe: %d", top_herb );
}


/*
 * Lookup an herb by name.
 */
int herb_lookup( const char *name )
{
	int sn;

	for ( sn = 0; sn < MAX_HERB; sn++ )
	{
		if ( !table_herb[sn] || !VALID_STR(table_herb[sn]->name) )
			return -1;

		if ( !str_prefix(name, table_herb[sn]->name) )
			return sn;
	}

	return -1;
}


/*
 * Controlla se il sn passato è una erbetta.
 */
bool is_valid_herb( int sn )
{
	if ( sn >=0 && sn < MAX_HERB && table_herb[sn] && table_herb[sn]->name )
		return TRUE;

	return FALSE;
}


/*
 * pipe commands (light, tamp, smoke)
 */
DO_RET do_tamp( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *the_pipe;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo comprimere cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	the_pipe = get_obj_carry( ch, argument, TRUE );
	if ( !the_pipe )
	{
		send_to_char( ch, "Non lo sto trasportando.\r\n" );
		return;
	}

	if ( the_pipe->type != OBJTYPE_PIPE )
	{
		send_to_char( ch, "Non lo posso comprimere.\r\n" );
		return;
	}

	if ( !HAS_BITINT(the_pipe->pipe->flags, PIPE_TAMPED) )
	{
		act( AT_ACTION, "Con delicatezza comprimo $p.", ch, the_pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n con delicatezza comprime $p.", ch, the_pipe, NULL, TO_ROOM );
		SET_BITINT( the_pipe->pipe->flags, PIPE_TAMPED );
		return;
	}

	send_to_char( ch, "Non neccessita una pressione.\r\n" );
}


/*
 * (FF) Da rivedere:
 * 	1) materiali vegetali bruciabili
 *	2) elenco erbe per slot con effetti oppure utilizzo lista di affect uhm... da pensare.
 */
DO_RET do_smoke( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *the_pipe;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Che cosa devo fumare?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( (the_pipe = get_obj_carry(ch, argument, TRUE)) == NULL )
	{
		send_to_char( ch, "Non lo sto trasportando.\r\n" );
		return;
	}

	if ( the_pipe->type != OBJTYPE_PIPE )
	{
		act( AT_ACTION, "Tento di fumare $p.. ma non si accende.", ch, the_pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n tenta di fumare $p...", ch, the_pipe, NULL, TO_ROOM );
		return;
	}

	if ( !HAS_BITINT(the_pipe->pipe->flags, PIPE_LIT) )
	{
		act( AT_ACTION, "Tento di fumare $p, ma non arde.", ch, the_pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n tenta di fumare $p, con scarso successo.", ch, the_pipe, NULL, TO_ROOM );
		return;
	}

	if ( the_pipe->pipe->draws > 0 )
	{
		if ( !oprog_use_trigger( ch, the_pipe, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "Tiro profondamente da $p.", ch, the_pipe, NULL, TO_CHAR );
			act( AT_ACTION, "$n tira profondamente da $p.", ch, the_pipe, NULL, TO_ROOM );
		}

		if ( is_valid_herb(the_pipe->pipe->herb) && the_pipe->pipe->herb <= MAX_HERB )
		{
			int			sn		= the_pipe->pipe->herb + TYPE_HERB;
			SKILL_DATA *skill	= get_skilltype( sn );

			WAIT_STATE( ch, skill->beats * 3 );	/* (WT) */
			if ( skill->spell_fun )
				obj_cast_spell( sn, UMIN(the_pipe->level/2, get_level(ch)/2), ch, ch, NULL );
			if ( obj_extracted(the_pipe) )
				return;
		}
		else
		{
			send_log( NULL, LOG_BUG, "do_smoke: tipo di erba errata %d", the_pipe->pipe->herb );
		}

		SET_BITINT( the_pipe->pipe->flags, PIPE_HOT );
		if ( --the_pipe->pipe->draws < 1 )
		{
			REMOVE_BITINT( the_pipe->pipe->flags, PIPE_LIT );
			SET_BITINT( the_pipe->pipe->flags, PIPE_DIRTY );
			SET_BITINT( the_pipe->pipe->flags, PIPE_FULLOFASH );
		}
	} /* chiude l'if */
	send_log( NULL, LOG_BUG, "do_smoke: in attesa di nuova implementazione." );
}


DO_RET do_light( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *the_pipe;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo accendere cosa?\r\n" );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( (the_pipe = get_obj_carry(ch, argument, TRUE)) == NULL )
	{
		send_to_char( ch, "Non lo sto trasportando.\r\n" );
		return;
	}

	if ( the_pipe->type != OBJTYPE_PIPE )
	{
		send_to_char( ch, "Non posso accenderlo.\r\n" );
		return;
	}

	if ( !HAS_BITINT(the_pipe->pipe->flags, PIPE_LIT) )
	{
		if ( the_pipe->pipe->draws < 1 )
		{
			act( AT_ACTION, "Tento di accendere $p.", ch, the_pipe, NULL, TO_CHAR );
			act( AT_ACTION, "$n tenta di accendere $p...", ch, the_pipe, NULL, TO_ROOM );
			return;
		}
		act( AT_ACTION, "Accendo con delicatezza $p.", ch, the_pipe, NULL, TO_CHAR );
		act( AT_ACTION, "$n delicatamente accende $p.", ch, the_pipe, NULL, TO_ROOM );
		SET_BITINT( the_pipe->pipe->flags, PIPE_LIT );
		return;
	}

	send_to_char( ch, "E' gia acceso.\r\n" );
}
