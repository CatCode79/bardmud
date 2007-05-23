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

/*
-----------------------------------------------------------------------------------------
 * The Mythran Mud Economy Snippet Version 2 (used to be banking.c)
 *
 * Copyrights and rules for using the economy system:
 *
 *      The Mythran Mud Economy system was written by The Maniac, it was
 *      loosly based on the rather simple 'Ack!'s banking system'
 *
 *      based on:
 *      Simple banking system. by -- Stephen --
 *
 *      The following changes and additions where
 *      made by the Maniac from Mythran Mud
 *      (v942346@si.hhs.nl)
 *
 *      History:
 *      18/05/96:     Added the transfer option, enables chars to transfer
 *                    money from their account to other players' accounts
 *      18/05/96:     Big bug detected, can deposit/withdraw/transfer
 *                    negative amounts (nice way to steal is
 *                    bank transfer -(lots of dogh)
 *                    Fixed it (thought this was better... -= Maniac =-)
 *      21/06/96:     Fixed a bug in transfer (transfer to MOBS)
 *                    Moved balance from ch->balance to ch->pcdata->balance
 *      21/06/96:     Started on the invest option, so players can invest
 *                    money in shares, using buy, sell and check
 *                    Finished version 1.0 releasing it monday 24/06/96
 *      24/06/96:     Mythran Mud Economy System V1.0 released by Maniac
 *
 *      If you use this code you must follow these rules.
 *              -Keep all the credits in the code.
 *              -Mail Maniac (v942346@si.hhs.nl) to say you use the code
 *              -Send a bug report, if you find 'it'
 *              -Credit me somewhere in your mud.
 *              -Follow the envy/merc/diku license
 *              -If you want to: send me some of your code
 *
 * All my snippets can be found on http://www.hhs.nl/~v942346/snippets.html
 * Check it often because it's growing rapidly  -- Maniac --
-----------------------------------------------------------------------------------------
 */

/****************************************************************************************\
 >						GESTIONE DELLA BANCA E DEGLI ACCOUNT BANCARI					<
\****************************************************************************************/

#include <errno.h>
#include <dirent.h>

#include "mud.h"
#include "calendar.h"
#include "economy.h"
#include "fread.h"
#include "interpret.h"
#include "room.h"
#include "save.h"


/*
 * Definizioni
 */
#define	MAX_BANK_MONEY( level )		(1000000 + (level)*10000)	/* Quantità massima di monete per un conto corrente bancario */
#define MAX_BANK_SHARE( level )		(2500	 + (level)*25	)	/* Quantità massima di azioni acquistabili */


/*
 * Directory contenente tutti i gli account dei pg
 */
#define ACCOUNT_DIR		PLAYER_DIR "/accounts/"


/*
 * Struttura di un account bancario
 */
typedef struct	bank_account_data		ACCOUNT_DATA;

struct bank_account_data
{
	ACCOUNT_DATA	*next;
	ACCOUNT_DATA	*prev;
	MOB_PROTO_DATA	*banker;	/* Mob che gestisce la banca di questo account */
	char			*owner;		/* Proprietario dell'account */
	int				 level;		/* Livello del proprietario dell'account, meglio salvarlo così da evitare fluttuazioni di MAX_BANK_AMOUNT e MAX_BANK_SHARE e quindi errori nella fread_account() */
	int				 money;		/* Ammontare delle monete dell'account */
	int				 shares;	/* Numero delle azioni dell'account */
};


/*
 * Variabili
 */
static	ACCOUNT_DATA *first_account = NULL;
static	ACCOUNT_DATA *last_account	= NULL;
static	int			  top_account	= 0;


/*
 * Legge un file di account bancario
 */
void fread_account( MUD_FILE *fp )
{
	ACCOUNT_DATA	*account;

	CREATE( account, ACCOUNT_DATA, 1 );
	account->money = -1;
	account->shares = -1;

	for ( ; ; )
	{
		char	*word;

		word = fread_word( fp );

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_social: fine prematura del file nella lettura" );
			break;
		}

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Banker"	) )		account->banker =	fread_vnum_mob( fp );
		else if ( !str_cmp(word, "Owner"	) )		account->owner =	fread_string( fp );
		else if ( !str_cmp(word, "Level"	) )		account->level =	fread_positive( fp );
		else if ( !str_cmp(word, "Amount"	) )		account->money =	fread_positive( fp );
		else if ( !str_cmp(word, "Share"	) )		account->shares =	fread_positive( fp );
		else if ( !str_cmp(word, "End"		) )		break;
		else
			send_log( fp, LOG_FREAD, "fread_account: nessuna etichetta per la parola %s", word );
	}

	if ( !account->banker )
	{
		send_log( NULL, LOG_LOAD, "fread_account: bancario è NULL" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(account->owner) )
	{
		send_log( NULL, LOG_LOAD, "fread_account: proprietario del conto non è valido" );
		exit( EXIT_FAILURE );
	}

	if ( account->level <= 0 || account->level > LVL_LEGEND )
	{
		send_log( NULL, LOG_LOAD, "fread_account: livello del proprietario del conto errato o non acquisito: %d", account->level );
		exit( EXIT_FAILURE );
	}

	if ( account->money < 0 || account->money > MAX_BANK_MONEY(account->level) )
	{
		send_log( NULL, LOG_LOAD, "fread_account: ammontare del conto corrente di %s errato: %d", account->owner, account->money );
		exit( EXIT_FAILURE );
	}

	if ( account->shares < 0 || account->shares > MAX_BANK_SHARE(account->level) )
	{
		send_log( NULL, LOG_LOAD, "fread_account: ammontare delle azioni di %s errato: %d", account->owner, account->shares );
		exit( EXIT_FAILURE );
	}

	LINK( account, first_account, last_account, next, prev );
	top_account++;
}


/*
 * Carica tutti gli account bancari
 */
void load_accounts( void )
{
	DIR			  *dp;
	struct dirent *dentry;
	MUD_FILE	  *fp;

	dp = opendir( ACCOUNT_DIR );
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
		{
			char	filename[MIL];

			if ( !str_suffix(".accounts", dentry->d_name) )
			{
				sprintf( filename, "%s%s", ACCOUNT_DIR, dentry->d_name );
				fp = mud_fopen( "", filename, "r", TRUE );
				if ( fp )
					fread_section( filename, "ACCOUNT", fread_account, FALSE );
				MUD_FCLOSE( fp );
			}
		}
		dentry = readdir( dp );
	} 

	closedir( dp );
}


/*
 * Libera dalla memoria un account bancario
 */
void free_account( ACCOUNT_DATA *account, bool remlink )
{
	if ( !account )
	{
		send_log( NULL, LOG_BUG, "free_account: account passato è NULL" );
		return;
	}

	if ( remlink )
	{
		UNLINK( account, first_account, last_account, next, prev );
		top_account--;
	}

	DISPOSE( account->owner );
	DISPOSE( account );
}


/*
 * Libera dalla memoria tutti gli account bancari
 */
void free_all_accounts( void )
{
	ACCOUNT_DATA *account;

	for ( account = first_account;  account;  account = first_account )
		free_account( account, TRUE );
}


/*
 * Scrive su file le informazioni di un account bancario
 */
void fwrite_accounts( const char *name )
{
	ACCOUNT_DATA	*account;
	MUD_FILE		*fp;
	char			 filename[MIL];
	bool			 found;

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "fwrite_accounts: name passato non è valido" );
		return;
	}

	sprintf( filename, "%s.accounts", name );
	fp = mud_fopen( ACCOUNT_DIR, filename, "w", TRUE );
	if ( !fp )
		return;

	found = FALSE;
	for ( account = first_account;  account;  account = account->next )
	{
		if ( !str_cmp(account->owner, name) )
		{
			found = TRUE;
			fprintf( fp->file, "#ACCOUNT\n" );
			fprintf( fp->file, "Banker       %d\n",		account->banker->vnum );
			fprintf( fp->file, "Owner        %s~\n",	account->owner );
			fprintf( fp->file, "Level        %d\n",		account->level );
			fprintf( fp->file, "Amount       %d\n",		URANGE(0, account->money, MAX_BANK_MONEY(account->level)) );
			fprintf( fp->file, "Share        %d\n",		URANGE(0, account->shares, MAX_BANK_SHARE(account->level)) );
			fprintf( fp->file, "End\n\n" );
		}
	}

	fprintf( fp->file, "#END\n" );
	MUD_FCLOSE( fp );

	/* Se non ha trovato nessun account da scrivere allora cancella il file che è inutile tenerlo */
	if ( !found )
	{
		if ( remove(filename) != 0 )
		{
			if ( errno != ENOENT )
			{
				send_log( NULL, LOG_SAVE, "fwrite_accounts: impossibile rimuovere il file %s (errno: %s)",
					filename, strerror(errno) );
			}
		}
	}
}


/*
 * Rimuove tutti i file dei conti correnti del pg e li libera dalla memoria
 * Succede quando viene distrutto o si distrugge
 */
void remove_all_accounts_player( CHAR_DATA *ch )
{
	ACCOUNT_DATA *account;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "remove_all_file_account: ch passato è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_BUG, "remove_all_accounts_player: ch passato è un mob: %s", ch->short_descr );
		return;
	}

	for ( account = first_account;  account;  account = account->next )
	{
		if ( !str_cmp(account->owner, ch->name) )
			free_account( account, TRUE );
	}

	fwrite_accounts( ch->name );
}


/*
 * Rinomina il prorietario di un conto bancario in un altro
 * Questo avviene quando il pg viene rinominato
 */
void rename_account( CHAR_DATA *ch, const char *newname )
{
	MUD_FILE	*fp;
	char		 filename[MIL];
	char		 filenew[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "rename_account: ch è pasato è NULL" );
		return;
	}

	if ( !VALID_STR(newname) )
	{
		send_log( NULL, LOG_BUG, "rename_account: newname passato non è valido" );
		return;
	}

	sprintf( filename, "%s.accounts", ch->name );
	fp = mud_fopen( ACCOUNT_DIR, filename, "w", FALSE );
	if ( !fp )
		return;
	sprintf( filenew, "%s.accounts", newname );

	if ( rename(filename, filenew) != 0 )
	{
		send_log( NULL, LOG_SAVE, "rename_account: errore nella rinominazione da %s a %s: %s",
			filename, filenew, strerror(errno) );
	}
}


/*
 * Cerca l'account del pg relativo al bancario
 */
ACCOUNT_DATA *get_account( const char *name, MOB_PROTO_DATA *banker )
{
	ACCOUNT_DATA *account;

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "get_account: nome passato non è valido" );
		return NULL;
	}

	if ( !banker )
	{
		send_log( NULL, LOG_BUG, "get_account: banker passato è NULL" );
		return NULL;
	}

	for ( account = first_account;  account;  account = account->next )
	{
		if ( account->banker != banker )
			continue;

		if ( !str_cmp(account->owner, name) )
			return account;
	}

	return NULL;
}


/*
 * Invia una lista degli account del pg con relativi ammontare
 */
void show_player_accounts( CHAR_DATA *ch, CHAR_DATA *victim )
{
	ACCOUNT_DATA *account;

	for ( account = first_account;  account;  account = account->next )
	{
		if ( !str_cmp(account->owner, victim->name) )
		{
			ch_printf( ch, "Account gestito dal banchiere: %s(#%d):\r\n",
				account->banker->short_descr, account->banker->vnum );
			ch_printf( ch, "Monete: %d  Azioni: %d (val tot:%d) (cadauna:%d)\r\n",
				account->money, account->shares, account->shares * share_value, share_value );
		}
	}
}


/*
 * Comando bank, con opzioni di:
 * balance, deposit, transfer, withdraw, buy, sell, check
 * (FF) aggiungere che se ci si trova in una stanza clan banca
 *	allora lì c'è un check sul conto del clan e non su quello del pg
 */
void do_bank( CHAR_DATA *ch, char *argument )
{
	ACCOUNT_DATA *account;
	CHAR_DATA	 *banker;
	char		  buf [MSL];
	char		  arg1[MIL];
	char		  arg2[MIL];
	int			  amount;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Il Servizio bancario non è disponibile ai mob.\r\n" );
		return;
	}

	/* Check for mob with act_banker */
	for ( banker = ch->in_room->first_person;  banker;  banker = banker->next_in_room )
	{
		if ( IS_MOB(banker) && HAS_BIT(banker->act, MOB_BANKER) )
			break;
	}
	if ( !banker )
	{
		send_to_char( ch, "Non posso usufruire di servizi bancari qui.\r\n" );
		return;
	}

	if ( calendar.hour < 8 || calendar.hour > 18 )
	{
		send_to_char( ch, "Non posso, la banca ora è chiusa, è aperta dalle 8 alle 6 di pomeriggio.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		char	cmd[MIL];

		strcpy( cmd, translate_command(ch, "bank") );

		send_to_char( ch, "Regola di Transazione Bancaria:\r\n\r\n" );
		ch_printf	( ch, "%s bilancio          : Mostra il tuo bilancio bancario\r\n", cmd );
		ch_printf	( ch, "%s deposita #        : Deposita un numero # di monete sul tuo conto\r\n", cmd );
		ch_printf	( ch, "%s preleva #         : Preleva un numero # di monete dal tuo conto\r\n", cmd );
		ch_printf	( ch, "%s trasferisci # &gnome&w: Trasferisce # monete nel conto di &gnome&w\r\n", cmd );
		ch_printf	( ch, "%s compra #          : Compri un numero # di azioni\r\n", cmd );
		ch_printf	( ch, "%s vendi #           : Vendi un numero # di azioni\r\n", cmd );
		ch_printf	( ch, "%s azioni            : Mostra l'attuale valore delle azioni\r\n", cmd );

		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( is_name_prefix(arg1, "bilancio balance") )
	{
		account = get_account( ch->name, banker->pIndexData );
		if ( !account )
		{
			sprintf( buf, "say a %s Attualmente non avete nessun conto corrente aperto con la nostra banca.", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		sprintf( buf, "say a %s Il vostro bilancio attualmente è di %d monet%c.",
			ch->name, account->money, (account->money == 1) ? 'a' : 'e' );
		send_command( banker, buf, CO );
		return;
	}

	if ( is_name_prefix(arg1, "depositare deposito depositi") )
	{
		if ( !is_number(arg2) )
		{
			sprintf( buf, "say a %s Quante monete vorreste depositare?", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		amount = atoi( arg2 );
		if ( amount < 0 )
		{
			sprintf( buf, "say a %s Penso sia un po' difficile depositare una quantità di monete negativa!", ch->name );
			send_command( banker, buf, CO );
#ifdef T2_ALFA
			sprintf( buf, "mpecho %s Sbircia dietro di te per controllare la fila, impaziente.", ch->name, ch->name );
			send_command( banker, buf, CO );
			sprintf( buf, "mpechonovict Sbircia dietro di %s per controllare la fila, impaziente.", ch->name );
			send_command( banker, buf, CO );
#endif
			return;
		}

		if ( amount > ch->gold )
		{
			sprintf( buf, "say a %s Volete depositare la somma di %d? Ma avete solamente %d monete con voi!",
				ch->name, amount, ch->gold );
			send_command( banker, buf, CO );
			return;
		}

		account = get_account( ch->name, banker->pIndexData );
		if ( !account )
		{
			if ( amount > MAX_BANK_MONEY(ch->level) )
			{
				sprintf( buf, "say a %s Non potete aprire un conto con così tante monete, il massimo che possiamo tenere per voi è di %d",
					ch->name, MAX_BANK_MONEY(ch->level) );
				send_command( banker, buf, CO );
				return;
			}

			CREATE( account, ACCOUNT_DATA, 1 );
			account->banker	= banker->pIndexData;
			account->owner	= str_dup( ch->name );
			account->level	= ch->level;
			LINK( account, first_account, last_account, next, prev );
			top_account++;
		}
		else
		{
			if ( account->money + amount > MAX_BANK_MONEY(ch->level) )
			{
				sprintf( buf, "say a %s Non potete depositare così tante monete, il massimo che possiamo tenere per voi è di %d",
					ch->name, MAX_BANK_MONEY(ch->level) );
				send_command( banker, buf, CO );
				return;
			}
		}

		ch->gold -= amount;
		account->money += amount;
		ch_printf( ch, "Deposito %d monet%c. Il saldo ora ammonta a %d.\r\n",
			amount, (amount == 1) ? 'a' : 'e', account->money );

		save_player( ch );
		fwrite_accounts( ch->name );
		return;
	}

	if ( is_name_prefix(arg1, "trasferire trasferisco trasferisci transfer") )
	{
		ACCOUNT_DATA *vaccount;	/* account in cui trasferire il denaro */

		if ( !is_number(arg2) )
		{
			sprintf( buf, "say a %s Quante monete vorreste trasferire?", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		amount = atoi( arg2 );
		if ( amount < 0 )
		{
			sprintf( buf, "say a %s Penso sia un po' difficile traferire una quantità di monete negativa!", ch->name );
			send_command( banker, buf, CO );
#ifdef T2_ALFA
			sprintf( buf, "mpecho %s Sbircia dietro di te per controllare la fila, impaziente.", ch->name, ch->name );
			send_command( banker, buf, CO );
			sprintf( buf, "mpechonovict Sbircia dietro di %s per controllare la fila, impaziente.", ch->name );
			send_command( banker, buf, CO );
#endif
			return;
		}

		account = get_account( ch->name, banker->pIndexData );
		if ( !account )
		{
			sprintf( buf, "say a %s Attualmente non avete nessun conto corrente aperto con la nostra banca.", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		if ( amount > account->money )
		{
			sprintf( buf, "say a %s Non potete trasferire la somma di %d monet%c, il vostro bilancio è di %d.",
				ch->name, amount, (amount == 1) ? 'a' : 'e', account->money );
			send_command( banker, buf, CO );
			return;
		}

		vaccount = get_account( argument, banker->pIndexData );
		if ( !vaccount )
		{
			sprintf( buf, "say a %s Attualmente non abbiamo nessun conto aperto a nome di %s.", ch->name, argument );
			send_command( banker, buf, CO );
			return;
		}

		if ( vaccount->money + amount > MAX_BANK_MONEY(vaccount->level) )
		{
			sprintf( buf, "say a %s Non potete trasferire così tanti soldi sul suo conto.", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		account->money -= amount;
		ch_printf( ch, "Trasferisco %d monet%c sul conto di %s. Il mio saldo ammonta a %d.\r\n",
			amount, (amount == 1) ? 'a' : 'e', capitalize(argument), account->money );
		save_player( ch );
		fwrite_accounts( ch->name );

		/* (FF) se il pg vittima è online inviargli una mail con l'avvenuto trasferimento */
/*		ch_printf( victim, "&W[&YBANCA&W]&w %s ha trasferito sul vostro conto %d monete.\r\n", ch->name, amount );*/
		vaccount->money += amount;
		fwrite_accounts( vaccount->owner );
		return;
	}

	if ( is_name_prefix(arg1, "preleva withdraw") )
	{
		account = get_account( ch->name, banker->pIndexData );
		if ( !account )
		{
			sprintf( buf, "say a %s Attualmente non avete nessun conto corrente aperto con la nostra banca.", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		amount = atoi( arg2 );
		if ( !is_number(arg2) )
		{
			sprintf( buf, "say a %s Quante monete vorreste prelevare?", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		if ( amount < 0 )
		{
			sprintf( buf, "say a %s Penso sia un po' difficile prelevare una quantità di monete negativa!", ch->name );
			send_command( banker, buf, CO );
#ifdef T2_ALFA
			sprintf( buf, "mpecho %s Sbircia dietro di te per controllare la fila, impaziente.", ch->name, ch->name );
			send_command( banker, buf, CO );
			sprintf( buf, "mpechonovict Sbircia dietro di %s per controllare la fila, impaziente.", ch->name );
			send_command( banker, buf, CO );
#endif
			return;
		}

		if ( amount > account->money )
		{
			sprintf( buf, "say a %s Non potete prelevare %d monet%c quando il vostro bilancio è di %d.",
				ch->name, amount, (amount == 1) ? 'a' : 'e', account->money );
			send_command( banker, buf, CO );
			return;
		}

		account->money -= amount;
		ch->gold += amount;
		ch_printf( ch, "Prelevo %d monet%c. Il mio saldo ammonta a %d.\r\n",
			amount, (amount == 1) ? 'a' : 'e', account->money );
		fwrite_accounts( ch->name );
		save_player( ch );
		return;
	}

	if ( is_name_prefix(arg1, "compro compra comprare buy") )
	{
		account = get_account( ch->name, banker->pIndexData );
		if ( !account )
		{
			sprintf( buf, "say a %s Per effettuare tale operazione dovete avere un conto aperto con la nostra banca.", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		if ( !is_number(arg2) )
		{
			sprintf( buf, "say a %s Quante azioni vorreste comprare?", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		amount = atoi( arg2 );
		if ( amount < 0 )
		{
			sprintf( buf, "say a %s Una quantità negativa? Se volete vendere delle azioni non avete che da dirmelo.", ch->name );
			send_command( banker, buf, CO );
#ifdef T2_ALFA
			sprintf( buf, "mpecho %s Sbircia dietro di te per controllare la fila, impaziente.", ch->name, ch->name );
			send_command( banker, buf, CO );
			sprintf( buf, "mpechonovict Sbircia dietro di %s per controllare la fila, impaziente.", ch->name );
			send_command( banker, buf, CO );
#endif
			return;
		}

		if ( (amount * share_value) > account->money )
		{
			sprintf( buf, "say a %s %d azion%s %d monete, non avete abbastanza sul conto.",
				ch->name, amount, (amount == 1) ? "e vi costerà" : "i vi costeranno", (amount * share_value) );
			send_command( banker, buf, CO );
			return;
		}

		if ( (account->shares + amount) > MAX_BANK_SHARE(ch->level) )
		{
			sprintf( buf, "say a %s Non potete comprare così tante azioni, il vostro massimo è di %d.",
				ch->name, MAX_BANK_SHARE(ch->level) );
			send_command( banker, buf, CO );
			return;
		}

		account->money -= ( amount * share_value );
		account->shares  += amount;
		ch_printf( ch, "Compro %d azion%c a %d monete, ora ho in totale %d azion%c.\r\n",
			amount, (amount == 1) ? 'e' : 'i', (amount * share_value), account->shares, (account->shares == 1) ? 'e' : 'i' );
		fwrite_accounts( ch->name );
		return;
	}

	if ( is_name_prefix(arg1, "vendo vendi vendere sell") )
	{
		account = get_account( ch->name, banker->pIndexData );
		if ( !account )
		{
			sprintf( buf, "say a %s Attualmente non avete nessun conto corrente aperto con la nostra banca.", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		if ( !is_number(arg2) )
		{
			sprintf( buf, "say a %s Quante azioni vorreste vendere?", ch->name );
			send_command( banker, buf, CO );
			return;
		}

		amount = atoi( arg2 );
		if ( amount < 0 )
		{
			sprintf( buf, "say a %s Una quantità negativa? Se volete comprare delle azioni non avete che da dirmelo.", ch->name );
			send_command( banker, buf, CO );
#ifdef T2_ALFA
			sprintf( buf, "mpecho %s Sbircia dietro di te per controllare la fila, impaziente.", ch->name, ch->name );
			send_command( banker, buf, CO );
			sprintf( buf, "mpechonovict Sbircia dietro di %s per controllare la fila, impaziente.", ch->name );
			send_command( banker, buf, CO );
#endif
			return;
		}

		if ( amount > account->shares )
		{
			sprintf( buf, "say a %s Possedete solamente %d azioni, non potete venderne di più.",
				ch->name, account->shares );
			send_command( banker, buf, CO );
			return;
		}

		if ( account->money + (amount * share_value) > MAX_BANK_MONEY(ch->level) )
		{
			sprintf( buf, "say a %s Non potete vendere così tante azioni, avete raggiunto il massimo di %d per il vostro conto bancario.",
				ch->name, MAX_BANK_MONEY(ch->level) );
			send_command( banker, buf, CO );
			return;
		}

		account->money += (amount * share_value);
		account->shares -= amount;
		ch_printf( ch, "Vendo %d azion%c per %d monete, Ora ho %d azioni.\r\n",
			amount, (amount == 1) ? 'e' : 'i', (amount * share_value), account->shares );
		send_command( banker, buf, CO );

		fwrite_accounts( ch->name );
		return;
	}

	if ( is_name_prefix(arg1, "azioni check") )
	{
		sprintf( buf, "say a %s Il prezzo corrente delle azioni è di %d monete.",
			ch->name, share_value );
		send_command( banker, buf, CO );

		account = get_account( ch->name, banker->pIndexData );
		if ( account )
		{
			sprintf( buf, "say a %s Attualmente possedete %d azion%c, (%d per azione) per un totale di %d monete.",
				ch->name, account->shares, (account->shares == 1) ? 'e' : 'i', share_value, (account->shares * share_value) );
			send_command( banker, buf, CO );
		}
		return;
	}

	sprintf( buf, "say a %s Non capisco cosa vogliate, leggete la nostra Regola di Transazione.", ch->name );
	send_command( banker, buf, CO );
	send_command( ch, "bank", CO );
}
