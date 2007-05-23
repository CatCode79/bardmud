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
 >						Modulo per la gestione dei locker					<
\****************************************************************************/


#ifdef T2_LOCKER

#include <errno.h>
#include <dirent.h>

#include "mud.h"
#include "economy.h"
#include "fread.h"
#include "information.h"
#include "interpret.h"
#include "locker.h"
#include "mprog.h"
#include "room.h"
#include "save.h"
#include "utility.h"


/*
 * Definizioni
 */
#define LOCKER_DIR				PLAYER_DIR	"/lockers/"
#define LOCKER_COST				5000	/* 5000 monete (FF) gestire in futuro il costo mensilmente */
#define LOCKER_CAPACITY_WEIGHT	100000	/* 100 kg */
#define LOCKER_CAPACITY_NUMBER	100		/* 100 oggetti max */


/*
 * Variabili
 */
LOCKER_DATA		*first_locker = NULL;
LOCKER_DATA		*last_locker  = NULL;
int				 top_locker	  = 0;
static OBJ_DATA	*rgObjNest[MAX_NEST];	/* per il caricamento dei locker */


/*
 * Legge un file di locker
 * (FF) tra fread_storage, load_corpse e questa non andiamo mica bene...
 *	bisogna cercare di accorpare qualcosa
 */
void fread_locker( MUD_FILE *fp )
{
	LOCKER_DATA	*locker;
	OBJ_DATA	*tobj;
	OBJ_DATA	*tobj_next;
	char		*name;
	int			 iNest;

	CREATE( locker, LOCKER_DATA, 1 );

	name = name_from_file( fp->path );
	locker->owner = str_dup( name );

	for ( iNest = 0;  iNest < MAX_NEST;  iNest++ )
		rgObjNest[iNest] = NULL;

	for ( ; ; )
	{
		char *word;
		char  letter;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_locker: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		letter = fread_letter( fp );
		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if ( letter != '#' )
		{
			send_log( fp, LOG_FREAD, "fread_locker: # non trovato" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( !strcmp(word, "OBJECT") )
			fread_object_save( supermob, fp, OS_CARRY );
		else if ( !strcmp(word, "END") )
			break;
		else
		{
			send_log( fp, LOG_FREAD, "fread_locker: sezione errata %s", word );
			exit( EXIT_FAILURE );
		}
	}

	for ( tobj = supermob->first_carrying;  tobj;  tobj = tobj_next )
	{
		tobj_next = tobj->next_content;

		obj_from_char( tobj );
		LINK( tobj, locker->first_object, locker->last_object, next_content, prev_content );
	}

	LINK( locker, first_locker, last_locker, next, prev );
	top_locker++;
}


/*
 * Carica tutti i lockers
 */
void load_lockers( void )
{
	DIR			  *dp;
	struct dirent *dentry;
	MUD_FILE	  *fp;

	dp = opendir( LOCKER_DIR );
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
		{
			if ( !str_suffix(".locker", dentry->d_name) )
			{
				char	filename[MIL];

				sprintf( filename, "%s%s", LOCKER_DIR, dentry->d_name );
				fp = mud_fopen( "", filename, "r", TRUE );
				if ( fp )
					fread_locker( fp );
				MUD_FCLOSE( fp );
			}
		}
		dentry = readdir( dp );
	}
	closedir( dp );
}


/*
 * Scrive su file un locker
 */
void fwrite_locker( CHAR_DATA *ch, LOCKER_DATA *locker )
{
	MUD_FILE *fp;
	char	  filename[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "fwrite_locker: ch passato è NULL" );
		return;
	}

	if ( !locker )
	{
		send_log( NULL, LOG_BUG, "fwrite_locker: locker passato è NULL" );
		return;
	}

	/* Salva il locker */
	sprintf( filename, "%s%s.locker", LOCKER_DIR, locker->owner );
	fp = mud_fopen( "", filename, "w", TRUE );
	if ( locker->first_object )
		fwrite_object( ch, locker->last_object, fp, 0, OS_CARRY );
	else
		fputs( "* Anche senza oggetti questo file esistendo indica che il pg possiede un locker.\n\n", fp->file );
	fputs( "#END\n", fp->file );
	MUD_FCLOSE( fp );
}


/*
 *
 */
void free_locker( LOCKER_DATA *locker )
{
	OBJ_DATA *object;

	if ( !locker )
	{
		send_log( NULL, LOG_BUG, "free_locker: locker passato è NULL" );
		return;
	}

	UNLINK( locker, first_locker, last_locker, next, prev );
	top_locker--;

	DISPOSE( locker->owner );
	for ( object = locker->first_object;  object;  object = locker->first_object )
		free_object( object );
	DISPOSE( locker );
}

/*
 * Libera dalla memoria tutti i locker
 */
void free_all_lockers( void )
{
	LOCKER_DATA *locker;

	for ( locker = first_locker;  locker;  locker = first_locker )
		free_locker( locker );

	if ( top_locker != 0 )
		send_log( NULL, LOG_BUG, "free_all_lockers: top_locker non è 0 dopo aver liberato i locker: %d", top_locker );
}


/*
 * Rimuove un file di locker
 */
void remove_file_locker( LOCKER_DATA *locker )
{
	char filename[MIL];

	if ( !locker )
	{
		send_log( NULL, LOG_BUG, "removefile_locker: locker passato è NULL" );
		return;
	}

	/* Rimuove il file vecchio del locker */
	sprintf( filename, "%s%s.locker", LOCKER_DIR, locker->owner );
	if ( remove(filename) != 0 )
	{
		if ( errno != ENOENT )
		{
			send_log( NULL, LOG_SAVE, "remove_locker: impossibile rimuovere il file %s (errno: %s)",
				filename, strerror(errno) );
		}
	}
}


/*
 * Rimuove tutti i file di locker del pg passato e ne libera la memoria
 * Questo solitamente accade quando un pg
 * 
 */
void remove_all_file_locker( CHAR_DATA *ch )
{
	LOCKER_DATA *locker;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "remove_all_file_locker: ch passato è NULL" );
		return;
	}

	for ( locker = first_locker;  locker;  locker = locker->next )
	{
		if ( !str_cmp(locker->owner, ch->name) )
		{
			remove_file_locker( locker );
			free_locker( locker );
		}
	}
}


/*
 * Rinomina il proprietario di un locker in un altro
 *	questo avviene quando si rinomina un pg
 */
void rename_locker( CHAR_DATA *ch, const char *newname )
{
	LOCKER_DATA	*locker;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "rename_locker: ch passato è NULL" );
		return;
	}

	if ( !VALID_STR(newname) )
	{
		send_log( NULL, LOG_BUG, "rename_locker: newname passato non è valido" );
		return;
	}

	for ( locker = first_locker;  locker;  locker = locker->next )
	{
		if ( !str_cmp(locker->owner, newname) )
		{
			remove_file_locker( locker );

			DISPOSE( locker->owner );
			locker->owner = str_dup( newname );

			fwrite_locker( ch, locker );
		}
	}
}


/*
 * Cerca se nella stanza vi sia un custode per i locker
 */
CHAR_DATA *keeper_locker( CHAR_DATA *ch )
{
	CHAR_DATA *keeper;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "keeper_locker: ch è NULL" );
		return NULL;
	}

	if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "keeper_locker: ch->in_room è NULL" );
		return NULL;
	}

	for ( keeper = ch->in_room->first_person;  keeper;  keeper = keeper->next_in_room )
	{
		if ( IS_PG(keeper) )
			continue;

		if ( HAS_BIT_ACT(keeper, MOB_LOCKER) )
			return keeper;
	}

	return NULL;
}


/*
 * Ritorna il locker del nome del proprietario passato
 */
LOCKER_DATA *get_locker( const char *name )
{
	LOCKER_DATA *locker;

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "get_locker: nome passato non valido" );
		return NULL;
	}

	for ( locker = first_locker;  locker;  locker = locker->next )
	{
		if ( !str_cmp(locker->owner, name) )
			return locker;
	}

	return NULL;
}


/*
 * Cerca un oggetto nel locker del pg
 * E' praticamente una copia della funzione get_obj_carry
 */
OBJ_DATA *get_obj_locker( CHAR_DATA *ch, char *argument, LOCKER_DATA *locker )
{
	OBJ_DATA *obj;
	char	  arg[MIL];
	int		  number;
	int		  count;
	VNUM	  vnum;

	number = number_argument( argument, arg );
	if ( IS_ADMIN(ch) && is_number(arg) )
		vnum = atoi( arg );
	else
		vnum = -1;

	count = 0;
	for ( obj = locker->last_object;  obj;  obj = obj->prev_content )
	{
		if ( !can_see_obj(ch, obj, TRUE) )							continue;
		if ( !nifty_is_name(arg, obj->name) && obj->vnum != vnum )	continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	if ( vnum != -1 )
		return NULL;

	/*
	 * If we didn't find an exact match, run through the list of objects
	 * again looking for prefix matching, ie swo == sword.
	 */
	count = 0;
	for ( obj = locker->last_object;  obj;  obj = obj->prev_content )
	{
		if ( !can_see_obj(ch, obj, TRUE) )				continue;
		if ( !nifty_is_name_prefix(arg, obj->name) )	continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	return NULL;
}


/*
 * Comando per gestire un locker
 * I locker vengono salvati su file a parte, se un pg si deleta non vengono deletati (FF) ma sarà
 *	possibile per dei ladri svaligiarli
 * (bb) il problema più che altro è nel renaming del pg
 * Un file di locker vuoto di oggetti indica che il pg ha comprato il locker ma in quel momento non
 *	lo sta utilizzando
 */
void do_locker( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*keeper;
	LOCKER_DATA	*locker;
	char		 buf[MSL];
	char		 arg[MIL];
	int			 cost;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_locker: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono utilizzare degli armadietti di sicurezza.\r\n" );
		return;
	}

	keeper = keeper_locker( ch );
	if ( !keeper )
	{
		send_to_char( ch, "Non mi trovo in un negozio apposito o forse potrei provare in qualche banca.\r\n" );
		return;
	}

	locker = get_locker( ch->name );

	cost = compute_cost( ch, keeper, LOCKER_COST );

	argument = one_argument( argument, arg );
	if ( is_name_prefix(arg, "compra compro") || is_name(arg, "buy") )
	{
		if ( locker )
		{
			sprintf( buf, "say a %s Avete già un armadietto in cui riporre i vostri oggetti personali.", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		if ( ch->gold < cost )
		{
			sprintf( buf, "say a %s Non possedete abbastanza denaro con voi. Ci vogliono %d monete.\r\n", ch->name, cost );
			send_command( keeper, buf, CO );
			return;
		}
		ch->gold -= cost;

		CREATE( locker, LOCKER_DATA, 1 );
		locker->owner = str_dup( ch->name );
		LINK( locker, first_locker, last_locker, next, prev );

		act( AT_ACTION, "$N si avvicina ad uno degli armadietti di sicurezza aprendolo e mostrandomelo:", ch, NULL, keeper, TO_CHAR );
		act( AT_ACTION, "$N si avvicina ad uno degli armadietti di sicurezza aprendolo e mostrandolo a $n.", ch, NULL, keeper, TO_ROOM );
		send_to_char( ch, "Attualmente non ho nulla dentro il mio armadietto di sicurezza.\r\n" );
	}
	else if ( is_name_prefix(arg, "vendi vendo") || is_name(arg, "sell") )
	{
		if ( !locker )
		{
			sprintf( buf, "say a %s Non possedete nessun armadietto da vendere.", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		if ( locker->first_object )
		{
			sprintf( buf, "say a %s Il vostro armadietto non è ancora vuoto per poterlo vendere al prezzo di %d.",
				ch->name, LOCKER_COST/2 );
			send_command( keeper, buf, CO );
			return;
		}

		ch->gold += LOCKER_COST / 2;

		UNLINK( locker, first_locker, last_locker, next, prev );
		DISPOSE( locker->owner );
		DISPOSE( locker );

		act( AT_ACTION, "$N mi paga per il recupero dell'armadietto di sicurezza.", ch, NULL, keeper, TO_CHAR );
		act( AT_ACTION, "$N paga $n per un armadietto di sicurezza.", ch, NULL, keeper, TO_ROOM );
	}
	else if ( is_name_prefix(arg, "metti metto") || is_name(arg, "put") )
	{
		OBJ_DATA *obj;
		OBJ_DATA *inlocker;
		int		  wtotal;
		int		  count;

		if ( !locker )
		{
			sprintf( buf, "say a %s Non possedete nessun armadietto in cui mettere qualcosa.", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		if ( !VALID_STR(argument) )
		{
			sprintf( buf, "say a %s Quale oggetto volete depositare nel vostro armadietto?", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		obj = get_obj_carry( ch, argument, TRUE );
		if ( !obj )
		{
			ch_printf( ch, "Non trovo nessun %s da depositare dell'armadietto.\r\n", argument );
			return;
		}

		/* sommo i pesi degli oggetti nel locker */
		wtotal = 0;
		count = 0;
		for ( inlocker = locker->first_object;  inlocker;  inlocker = inlocker->next_content )
		{
			/* Se è un contenitore controllo il peso degli oggetti interni */
			if ( inlocker->container )
				wtotal += get_obj_weight( inlocker );
			else
				wtotal += inlocker->weight;
			count++;
	    }

		/* La capacità di un locker è di un terzo quella del pg (FF) magari in futuro
		 *	differenziare i tipi di locker e crearne con caratteristiche differenti */

		if ( count > can_carry_number(ch)/3 )
		{
			sprintf( buf, "say a %s Attualmente potete inserire nel vostro armadietto un massimo di %d oggetti.\r\n",
				ch->name, can_carry_number(ch)/3 );
		}
		if ( wtotal > can_carry_weight(ch)/3 )
		{
			sprintf( buf, "say a %s Attualmente potete inserire nel vostro armadietto un massimo di %d chili.\r\n",
				ch->name, can_carry_weight(ch)/3000 );
		}

		split_obj( obj, 1 );
		obj_from_char( obj );
		LINK( obj, locker->first_object, locker->last_object, next_content, prev_content );

		act( AT_ACTION, "Deposito $p nel mio armadietto di sicurezza.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n deposita $p nel suo armadietto di sicurezza.", ch, obj, NULL, TO_ROOM );
	}
	else if ( is_name_prefix(arg, "prendi prendo") || is_name(arg, "get") )
	{
		OBJ_DATA *obj;

		if ( !locker )
		{
			sprintf( buf, "say a %s Non possedete nessun armadietto da cui prendere qualcosa.", ch->name );
			send_command( keeper, buf, CO );
		}

		if ( !VALID_STR(argument) )
		{
			sprintf( buf, "say a %s Quale oggetto volete prelevare dal vostro armadietto?", ch->name );
			send_command( keeper, buf, CO );
			return;
		}

		obj = get_obj_locker( ch, argument, locker );
		if ( !obj )
		{
			ch_printf( ch, "Non trovo nessun %s da prelevare dal mio armadietto.\r\n", argument );
			return;
		}

		UNLINK( obj, locker->first_object, locker->last_object, next_content, prev_content );
		split_obj( obj, 1 );
		obj_to_char( obj, ch );

		act( AT_ACTION, "Prelevo $p dal mio armadietto di sicurezza.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n preleva $p dal suo armadietto di sicurezza.", ch, obj, NULL, TO_ROOM );
	}
	else
	{
		char	cmdtrans[MIL];

		strcpy( cmdtrans, translate_command(ch, "locker") );
		ch_printf( ch, "&YSintassi&w:  %s compra\r\n",			 cmdtrans );
		ch_printf( ch, "           %s vendi\r\n",				 cmdtrans );
		ch_printf( ch, "           %s metti <oggetto>\r\n",		 cmdtrans );
		ch_printf( ch, "           %s prendi <oggetto>\r\n\r\n", cmdtrans );

		if ( !locker )
		{
			sprintf( buf, "say a %s Non possedete nessun armadietto di sicurezza al momento.", ch->name );
			send_command( keeper, buf, CO );
			sprintf( buf, "say a %s Se volete comprarne uno sappiate che vi costerà %d d'oro.", ch->name, cost );
			send_command( keeper, buf, CO );
		}
		else
		{
			if ( can_see(keeper, ch) )
			{
				act( AT_ACTION, "$N mi riconosce al volo e si dirige verso il mio armadietto aprendolo:", ch, NULL, keeper, TO_CHAR );
				if ( locker->first_object )
					show_list_to_char( locker->first_object, ch, TRUE, TRUE );	/* (FF) con l'MXP bisognerà fare un argomento apposito */
				else
					send_to_char( ch, "Attualmente non ho nulla dentro il mio armadietto di sicurezza.\r\n" );
			}
			else
			{
				sprintf( buf, "say a %s Non aprirò nessun armadietto di sicurezza a persone che non riesco a vedere!", ch->name );
				send_command( keeper, buf, CO );
			}
		}

		return;		/* qui esce perchè è inutile eseguire i vari salvataggi */
	}

	if ( char_died(ch) )
		return;

	save_player( ch );
	fwrite_locker( ch, locker );
}


#endif	/* T2_LOCKER */
