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
 >							Special clan module								<
\****************************************************************************/

#include "mud.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "fread.h"
#include "handler_ch.h"
#include "interpret.h"
#include "room.h"
#include "save.h"


/* TODO: */
/* (FF) comando di gestione alleanza? */
/* (FF) comando di gestione permessi rank se vengono fatti? */
/* (FF) una funzioncina che checck i diritto di permessi per effettuare le azioni ci vorrebbe, rimpiazzerebbe tanti piccoli check qui e là ad inizio comandi */


/*
 * Variabili
 */
CLAN_DATA	 *first_clan = NULL;
CLAN_DATA	 *last_clan	 = NULL;
int			  top_clan	 = 0;


/*
 * Codici dei tipi di clan
 */
const CODE_DATA code_clantype[] =
{
	{ CLANTYPE_CLAN,	"CLANTYPE_CLAN",	"Clan"		},
	{ CLANTYPE_GUILD,	"CLANTYPE_GUILD",	"Gilda"		},
	{ CLANTYPE_SECT,	"CLANTYPE_SECT",	"Setta"		},
	{ CLANTYPE_CABALA,	"CLANTYPE_CABALA",	"Cabala"	},
	{ CLANTYPE_GROUP,	"CLANTYPE_GROUP",	"Gruppo"	},
	{ CLANTYPE_FACTION,	"CLANTYPE_FACTION",	"Fazione"	},
	{ CLANTYPE_WAR,		"CLANTYPE_WAR",		"Guerra"	}
};
const int max_check_clantype = sizeof(code_clantype)/sizeof(CODE_DATA);


/*
 * Codici di flag dei clan
 */
const CODE_DATA code_clanflag[]=
{
	{ CLANFLAG_SECRET,		"CLANFLAG_SECRET",		"segreto"	},
	{ CLANFLAG_NOSCORE,		"CLANFLAG_NOSCORE",		"nopunti"	},
	{ CLANFLAG_NOFLEE,		"CLANFLAG_NOFLEE",		"nofuga"	},
	{ CLANFLAG_NOOBJECT,	"CLANFLAG_NOOBJECT",	"nooggetto" }
};
const int max_check_clanflag = sizeof(code_clanflag)/sizeof(CODE_DATA);


/*
 * Controlla la validità di alcuni valori obbligatori per la creazione del clan
 * E' a parte perchè richiamata sia in lettura che in scrittura di clan
 */
bool check_clan( MUD_FILE *fp, CLAN_DATA *clan )
{
	int		x;
	bool	correct = TRUE;

	if ( !VALID_STR(clan->name) )
	{
		send_log( fp, LOG_FREAD, "check_clan: nome del clan non valido" );
		correct = FALSE;
	}

	if ( !VALID_STR(clan->short_descr) )
	{
		send_log( fp, LOG_FREAD, "check_clan: descrizion corta del clan non valida" );
		correct = FALSE;
	}

	if ( !VALID_STR(clan->description) )
	{
		send_log( fp, LOG_FREAD, "check_clan: descrizione del clan non valida" );
		correct = FALSE;
	}

	if ( !VALID_STR(clan->motto) )
	{
		send_log( fp, LOG_FREAD, "check_clan: motto del clan non valido" );
		correct = FALSE;
	}

	if ( clan->rank_limit >= MAX_RANK )
	{
		send_log( fp, LOG_FREAD, "check_clan: limite del rank del clan %d supera quello massimo possibile %d", clan->rank_limit, MAX_RANK );
		correct = FALSE;
	}

	if ( clan->member_limit >= 100 * MAX_RANK )
	{
		send_log( fp, LOG_FREAD, "check_clan: limite di membri" );
		correct = FALSE;
	}

	/* Controlla che tutti i rank del clan siano stati caricati e non oltre */
	for ( x = 0;  x < MAX_RANK;  x++ )
	{
		if ( x < clan->rank_limit && clan->rank[x] == NULL )
		{
			send_log( fp, LOG_FREAD, "check_clan: rank entro il limite del clan è NULL" );
			correct = FALSE;
		}
		if ( x >= clan->rank_limit && clan->rank[x] != NULL )
		{
			send_log( fp, LOG_FREAD, "check_clan: rank fuori dal limite del clan non è NULL" );
			correct = FALSE;
		}
	}

	if ( !correct )
	{
		if ( fBootDb )
			exit( EXIT_FAILURE );
		else
			return FALSE;
	}

	return TRUE;
}


/*
 * Legge una struttura di rank dal file di clan
 */
void fread_clan_rank( MUD_FILE *fp, CLAN_DATA *clan )
{
	int rank_num;

	if ( !fp )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: fp è NULL" );
		exit( EXIT_FAILURE );
	}

	if ( !clan )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: clan è NULL" );
		exit( EXIT_FAILURE );
	}

	rank_num = fread_number( fp );

	/* Qui rank_limit deve essere già stato acquisito, bisogna assicurarsi di scriverlo sopra alle etichette Rank */
	if ( rank_num < 0 || rank_num >= MAX_RANK || rank_num >= clan->rank_limit )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: numero di rank passato errato: %d%s",
			rank_num, clan->rank_limit == 0 ? "rank_limit non è stato acquisito precedentemente" : "" );
		exit( EXIT_FAILURE );
	}

	if ( clan->rank[rank_num] != NULL )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: la struttura rank numero %d è già stata inizializzata", rank_num );
		exit( EXIT_FAILURE );
	}

	CREATE( clan->rank[rank_num], RANK_DATA, 1 );
	clan->rank[rank_num]->name =			fread_string( fp );
	clan->rank[rank_num]->max_members =		fread_number( fp );
	clan->rank[rank_num]->classes =			fread_code_bit( fp, CODE_CLASS );
	clan->rank[rank_num]->races =			fread_code_bit( fp, CODE_RACE );
	clan->rank[rank_num]->align_min =		fread_number( fp );
	clan->rank[rank_num]->align_max =		fread_number( fp );

	if ( clan->rank[rank_num]->max_members < -1 || clan->rank[rank_num]->max_members > 100 )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: valore errato al rank %d di max_members: %d", rank_num, clan->rank[rank_num]->max_members );
		exit( EXIT_FAILURE );
	}

	if ( clan->rank[rank_num]->align_min < -1000 || clan->rank[rank_num]->align_min > 1000 )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: valore errato al rank %d di align_min: %d", rank_num, clan->rank[rank_num]->align_min );
		exit( EXIT_FAILURE );
	}

	if ( clan->rank[rank_num]->align_max < -1000 || clan->rank[rank_num]->align_max > 1000 )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: valore errato al rank %d di align_max: %d", rank_num, clan->rank[rank_num]->align_max );
		exit( EXIT_FAILURE );
	}

	if ( clan->rank[rank_num]->align_min > clan->rank[rank_num]->align_max )
	{
		send_log( fp, LOG_FREAD, "fread_clan_rank: al rank %d align_min %d supera align_max %d",
			rank_num,
			clan->rank[rank_num]->align_min,
			clan->rank[rank_num]->align_max );
		exit( EXIT_FAILURE );
	}
}


/*
 * Legge dal file di clan una struttura di membro del clan
 */
void fread_clan_member( MUD_FILE *fp, CLAN_DATA *clan )
{
	MEMBER_DATA *member;

	if ( !fp )
	{
		send_log( fp, LOG_FREAD, "fread_clan_member: fp è NULL" );
		exit( EXIT_FAILURE );
	}

	if ( !clan )
	{
		send_log( fp, LOG_FREAD, "fread_clan_member: clan è NULL" );
		exit( EXIT_FAILURE );
	}

	CREATE( member, MEMBER_DATA, 1 );
	member->name =			str_dup( fread_word(fp) );
	member->rank =			fread_number( fp );
	member->inducted_by =	str_dup( fread_word(fp) );
	member->inducted_date =	fread_number( fp );

	if ( member->rank < 0 || member->rank >= MAX_RANK || member->rank >= clan->rank_limit )
	{
		send_log( fp, LOG_FREAD, "fread_clan_member: rank del membro acquisito errato: %d%s",
			member->rank, clan->rank_limit == 0 ? "rank_limit non è stato acquisito precedentemente" : "" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(member->name) )
	{
		send_log( fp, LOG_FREAD, "fread_clan_member: nome del membro non valido" );
		exit( EXIT_FAILURE );
	}

	LINK( member, clan->first_member, clan->last_member, next, prev );
	clan->members++;
}


/*
 * Legge le informazioni per caricare un Clan
 */
void fread_clan( MUD_FILE *fp )
{
	CLAN_DATA	*clan;

	CREATE( clan, CLAN_DATA, 1 );

	for ( ; ; )
	{
		char   *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_clan: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Badge"		) )		clan->badge =			fread_vnum_obj( fp );
		else if ( !str_cmp(word, "Board"		) )		clan->board =			fread_vnum_obj( fp );
		else if ( !str_cmp(word, "ClanObj"		) )		clan->object =			fread_vnum_obj( fp );
		else if ( !str_cmp(word, "Description"	) )		clan->description =		fread_string( fp );
		else if ( !str_cmp(word, "End"			) )		break;
		else if ( !str_cmp(word, "Flags"		) )		clan->flags =			fread_code_bit( fp, CODE_CLANFLAG );
		else if ( !str_cmp(word, "GuardOne"		) )		clan->guard1 =			fread_vnum_mob( fp );
		else if ( !str_cmp(word, "GuardTwo"		) )		clan->guard2 =			fread_vnum_mob( fp );
		else if ( !str_cmp(word, "IllegalPk"	) )		clan->illegal_pk =		fread_positive( fp );
		else if ( !str_cmp(word, "MDeaths"		) )		clan->mdeaths =			fread_positive( fp );
		else if ( !str_cmp(word, "Member"		) )								fread_clan_member( fp, clan );
		else if ( !str_cmp(word, "MemberLimit"	) )		clan->member_limit =	fread_positive( fp );
		else if ( !str_cmp(word, "MKills"		) )		clan->mkills =			fread_positive( fp );
		else if ( !str_cmp(word, "Motto"		) )		clan->motto =			fread_string( fp );
		else if ( !str_cmp(word, "MsgReunion"	) )		clan->msg_reunion =		fread_string( fp );		/* Se non c'è significa che non prevedono riunioni a chiamata */
		else if ( !str_cmp(word, "MsgAlarm"		) )		clan->msg_alarm =		fread_string( fp );		/* Se non c'è significa che non è negato l'accesso ad estranei del clan */
		else if ( !str_cmp(word, "MsgObjLost"	) )		clan->msg_obj_lost =	fread_string( fp );		/* Se non c'è significa che non è un clan senza oggetto */
		else if ( !str_cmp(word, "MsgObjAquired") )		clan->msg_obj_aquired =	fread_string( fp );		/* Se non c'è significa che non è un clan senza oggetto */
		else if ( !str_cmp(word, "Name"			) )		clan->name =			fread_string( fp );
		else if ( !str_cmp(word, "PDeaths"		) )		clan->pdeaths =			fread_positive( fp );
		else if ( !str_cmp(word, "PKills"		) )		clan->pkills =			fread_positive( fp );
		else if ( !str_cmp(word, "Rank"			) )								fread_clan_rank( fp, clan );
		else if ( !str_cmp(word, "RankLimit"	) )		clan->rank_limit =		fread_positive( fp );
		else if ( !str_cmp(word, "Recall"		) )		clan->recall =			fread_vnum_room( fp );
		else if ( !str_cmp(word, "ShortDescr"	) )		clan->short_descr =		fread_string( fp );
		else if ( !str_cmp(word, "Strikes"		) )		clan->strikes =			fread_positive( fp );
		else if ( !str_cmp(word, "Type"			) )		clan->type =			code_code( fp, fread_word(fp), CODE_CLANTYPE );
		else
			send_log( fp, LOG_FREAD, "fread_clan: nessuna etichetta trovata per la parola: %s", word );
	} /* chiude il for */

	check_clan( fp, clan );

	/* Aggiunge il clan alla lista dei clan */
	LINK( clan, first_clan, last_clan, next, prev );
	top_clan++;
}


/*
 * Carica tutti i clan
 */
void load_clans( void )
{
	fread_list( CLAN_LIST, "CLAN", fread_clan, TRUE );
}


/*
 * Scrive la lista dei clan.
 */
void write_clan_list( void )
{
	CLAN_DATA *clan;
	MUD_FILE  *fpout;

	fpout = mud_fopen( CLAN_DIR, CLAN_LIST, "w", TRUE );
	if ( !fpout )
		return;

	for ( clan = first_clan;  clan;  clan = clan->next )
		fprintf( fpout->file, "%s.clan\n", clan->name );
	fprintf( fpout->file, "End\n" );

	MUD_FCLOSE( fpout );
}


/*
 * Salva le informazioni di un clan su file.
 */
void fwrite_clan( CLAN_DATA *clan )
{
	MUD_FILE	*fp;
	MEMBER_DATA *member;
	char		 filename[MIL];
	int			 x;

	if ( !clan )
	{
		send_log( NULL, LOG_FWRITE, "fwrite_clan: null clan pointer!" );
		return;
	}

	sprintf( filename, "%s.clan", clan->name );
	fp = mud_fopen( CLAN_DIR, filename, "w", TRUE );
	if ( !fp )
		return;

	if ( !check_clan(fp, clan) )
		return;

	fprintf( fp->file, "#CLAN\n" );
	fprintf( fp->file, "Name           %s~\n",	clan->name );
	fprintf( fp->file, "ShortDescr     %s~\n",	clan->short_descr );
	fprintf( fp->file, "Description    %s~\n",	clan->description );
	fprintf( fp->file, "Motto          %s~\n",	clan->motto );
	fprintf( fp->file, "MsgReunion     %s~\n",	clan->msg_reunion );
	fprintf( fp->file, "MsgAlarm       %s~\n",	clan->msg_alarm );
	fprintf( fp->file, "MsgObjLost     %s~\n",	clan->msg_obj_lost );
	fprintf( fp->file, "MsgObjAquired  %s~\n",	clan->msg_obj_aquired );
	fprintf( fp->file, "Type           %s\n",	code_str(fp, clan->type, CODE_CLANTYPE) );
	fprintf( fp->file, "Flags          %s~\n",	code_bit(fp, clan->flags, CODE_CLANFLAG) );
	fprintf( fp->file, "PKills         %d\n",	clan->pkills );
	fprintf( fp->file, "PDeaths       %d\n",	clan->pdeaths );
	fprintf( fp->file, "MKills         %d\n",	clan->mkills );
	fprintf( fp->file, "MDeaths        %d\n",	clan->mdeaths );
	fprintf( fp->file, "IllegalPk      %d\n",	clan->illegal_pk );
	fprintf( fp->file, "Strikes        %d\n",	clan->strikes );

	if ( clan->badge )
		fprintf( fp->file, "Badge          %d\n",	clan->badge->vnum );
	if ( clan->board )
		fprintf( fp->file, "Board          %d\n",	clan->board->vnum );
	if ( clan->object )
		fprintf( fp->file, "ClanObj        %d\n",	clan->object->vnum );
	if ( clan->recall )
		fprintf( fp->file, "Recall         %d\n",	clan->recall->vnum );
	if ( clan->guard1 )
		fprintf( fp->file, "GuardOne       %d\n",	clan->guard1->vnum );
	if ( clan->guard2 )
		fprintf( fp->file, "GuardTwo       %d\n",	clan->guard2->vnum );

	fprintf( fp->file, "RankLimit      %d\n",	clan->rank_limit );
	for ( x = 0;  x < clan->rank_limit;  x++ )
	{
		fprintf( fp->file, "Rank %d\n", x );
		fprintf( fp->file, "    %s~\n",	clan->rank[x]->name );
		fprintf( fp->file, "    %d\n",	clan->rank[x]->max_members );
		fprintf( fp->file, "    %s~\n",	code_bit(fp, clan->rank[x]->classes, CODE_CLASS) );
		fprintf( fp->file, "    %s~\n",	code_bit(fp, clan->rank[x]->races, CODE_RACE) );
		fprintf( fp->file, "    %d\n",	clan->rank[x]->align_min );
		fprintf( fp->file, "    %d\n",	clan->rank[x]->align_max );
	}
	for ( member = clan->first_member;  member;  member = member->next )
	{
		fprintf( fp->file, "Member         %s %d %s %d\n",
			member->name, member->rank, member->inducted_by, (int) member->inducted_date );
	}
	fprintf( fp->file, "MemberLimit    %d\n",	clan->member_limit );
	fprintf( fp->file, "End\n\n" );

	fprintf( fp->file, "#END\n" );

	MUD_FCLOSE( fp );
}


/*
 * Libera dalla memoria tutti i clan
 */
void free_all_clans( void )
{
	CLAN_DATA *clan;

	for ( clan = first_clan;  clan;  clan = first_clan )
	{
		MEMBER_DATA *member;
		int x;

		UNLINK( clan, first_clan, last_clan, next, prev );
		top_clan--;

		DISPOSE( clan->name );
		DISPOSE( clan->short_descr );
		DISPOSE( clan->description );
		DISPOSE( clan->motto );
		DISPOSE( clan->msg_reunion );
		DISPOSE( clan->msg_alarm );
		DISPOSE( clan->msg_obj_lost );
		DISPOSE( clan->msg_obj_aquired );
		destroybv( clan->flags );

		/* Libera i ranghi */
		for ( x = 0;  x < MAX_RANK;  x++ )
		{
			if ( clan->rank[x] )
			{
				DISPOSE( clan->rank[x]->name );
				destroybv( clan->rank[x]->classes );
				destroybv( clan->rank[x]->races );
				DISPOSE( clan->rank[x] );
			}
		}

		/* Libera i membri */
		for ( member = clan->first_member;  member;  member = clan->first_member )
		{
			DISPOSE( member->name );
			DISPOSE( member->inducted_by );
			UNLINK( member, clan->first_member, clan->last_member, next, prev );
			DISPOSE( member );
		}

		DISPOSE( clan );
	}

	if ( top_clan != 0 )
		send_log( NULL, LOG_BUG, "free_all_clans: top_clan non è 0 dopo aver liberato i clan: %d", top_clan );
}


/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( const char *name, bool exact )
{
	CLAN_DATA *clan;

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "get_clan: name passato non valido" );
		return NULL;
	}

	for ( clan = first_clan;  clan;  clan = clan->next )
	{
		if ( !str_cmp(name, clan->name) )
			return clan;
	}

	if ( exact )
		return NULL;

	for ( clan = first_clan;  clan;  clan = clan->next )
	{
		if ( !str_prefix(name, clan->name) )
			return clan;
	}

	return NULL;
}


/*
 * Ritorna il membro del clan con il nome del pg
 * (RR) nella fase di renaming del pg bisogna modificare anche il nome del pg nella lista dei membri del clan
 */
MEMBER_DATA *get_clan_member( const char *name, CLAN_DATA *clan, bool exact )
{
	MEMBER_DATA *member;

	if ( !clan )
	{
		send_log( NULL, LOG_BUG, "get_clan_member: clan è NULL" );
		return NULL;
	}

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "get_clan_member: name passato non valido per il clan %s", clan->name );
		return NULL;
	}

	for ( member = clan->first_member;  member;  member = member->next )
	{
		if ( !str_cmp(member->name, name) )
			return member;
	}

	if ( exact )
		return NULL;

	for ( member = clan->first_member;  member;  member = member->next )
	{
		if ( !str_prefix(member->name, name) )
			return member;
	}

	return NULL;
}


/*
 * Calcola l'allineamento del clan (con rank -1) oppure del rango passato
 */
int compute_clan_align( CLAN_DATA *clan, const int rank )
{
	MEMBER_DATA *member;
	int			 align = 0;

	if ( !clan )
	{
		send_log( NULL, LOG_BUG, "compute_clan_align: clan è NULL" );
		return 0;
	}

	if ( rank < -1 || rank >= clan->rank_limit )
	{
		send_log( NULL, LOG_BUG, "compute_clan_align: rank passatto errato: %d", rank );
		return 0;
	}

	for ( member = clan->first_member;  member;  member = member->next )
	{
		CHAR_DATA *victim;

		if ( rank > -1 && member->rank != rank )
			continue;

		for ( victim = first_player;  victim;  victim = victim->next_player )
		{
			if ( !str_cmp(victim->name, member->name) )
				break;
		}

		if ( !victim )
			victim = get_offline( member->name, TRUE );

		if ( !victim )
			send_log( NULL, LOG_BUG, "compute_clan_align: membro %s non trovato neppure offline", member->name );
		else
			align += victim->alignment;
	}

	/* ritorna la media */
	return align / clan->members;
}


/*
 * Ritorna il numero di rank del clan passato il nome del rango
 */
int get_clan_rank( const char *name, CLAN_DATA *clan, bool exact )
{
	int		x;

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "get_clan_rank: nome di rango passato non valido" );
		return -1;
	}

	for ( x = 0;  x < clan->rank_limit;  x++ )
	{
		if ( !str_cmp(name, clan->rank[x]->name) )
			return x;
	}

	if ( exact )
		return -1;

	for ( x = 0;  x < clan->rank_limit;  x++ )
	{
		if ( !str_prefix(name, clan->rank[x]->name) )
			return x;
	}

	return -1;
}


/*
 * Conta il numero di membri di un determinato rango
 */
int count_rank_member( CLAN_DATA *clan, const int rank )
{
	MEMBER_DATA *member;
	int			 count;

	if ( !clan )
	{
		send_log( NULL, LOG_BUG, "count_rank_member: clan è NULL" );
		return 0;
	}

	if ( rank < 0 || rank >= MAX_RANK )
	{
		send_log( NULL, LOG_BUG, "count_rank_member: rank passato errato: %d", rank );
		return 0;
	}

	count = 0;
	for ( member = clan->first_member;  member;  member = member->next )
	{
		if ( member->rank == rank )
			count++;
	}

	return count;
}


/*
 * Ritorna il punteggio totalizzato dal clan
 */
int get_clan_score( CLAN_DATA *clan )
{
	return 666;	/* (FF) come si può leggermente notare questa funzione è da definire */
}


/*
 * Controlla se la vittima che entra nel clan, o sta cambiando rango, può effettivamente acquisirlo
 *
 * CHAR_DATA *ch         è il pg che sta modificando il rank alla vittima
 * CHAR_DATA *victim     è la vittima a cui bisogna modificare il rango
 * CLAN_DATA *clan       è il clan di appartenenza della vittima (che se la vittima sta per essere arruolata lo ha NULL, per questo bisogna passarlo)
 * const int rank_actual è l'attuale rango della vittima
 * const int rank_next   è il rango futuro della vittima (se la vittima sta per essere arruolata rank_actual e rank_next sono tutti e due 0)
 * bool message          indica se inviare i messaggi di impedimento al cambio di rango a ch
 *
 */
bool change_rank( CHAR_DATA *ch, CHAR_DATA *victim, CLAN_DATA *clan, const int rank_actual, const int rank_next )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "change_rank: ch è NULL" );
		return FALSE;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "change_rank: victim è NULL" );
		return FALSE;
	}

	if ( !clan )
	{
		send_log( NULL, LOG_BUG, "change_rank: clan è NULL" );
		return FALSE;
	}

	if ( rank_actual < 0 || rank_actual >= clan->rank_limit )
	{
		send_log( NULL, LOG_BUG, "change_rank: rank_actual errato: %d", rank_actual );
		return FALSE;
	}

	if ( rank_next < 0 || rank_next >= clan->rank_limit )
	{
		send_log( NULL, LOG_BUG, "change_rank: rank_next errato: %d", rank_next );
		return FALSE;
	}

	if ( clan->rank[rank_next]->max_members != -1 && count_rank_member(clan, rank_next) >= clan->rank[rank_next]->max_members )
	{
		ch_printf( ch, "Il limite dei membri massimi per il rango %s verrebbe superata con %s.\r\n",
			clan->rank[rank_next]->name, victim->name );
		return FALSE;
	}

	if ( !IS_EMPTY(clan->rank[rank_next]->classes) && !HAS_BIT(clan->rank[rank_next]->classes, victim->class) )
	{
		ch_printf( ch, "%s ha una classe che non gli permette di entrare nel rango %s.\r\n",
			victim->name, clan->rank[rank_next]->name );
		return FALSE;
	}

	if ( !IS_EMPTY(clan->rank[rank_next]->races) && !HAS_BIT(clan->rank[rank_next]->races, victim->race) )
	{
		ch_printf( ch, "%s ha una razza che non gli permette di entrare nel rango %s.\r\n",
			victim->name, clan->rank[rank_next]->name );
		return FALSE;
	}

	if ( victim->alignment <= clan->rank[rank_next]->align_min || victim->alignment >= clan->rank[rank_next]->align_max )
	{
		ch_printf( ch, "%s ha un allineamento che non gli permette di entrare nel rango %s.\r\n",
			victim->name, clan->rank[rank_next]->name );
		return FALSE;
	}

	return TRUE;
}


/*
 * Comando per Capoclan per arruolare nuovi membri
 */
DO_RET do_induct( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA	*clan;
	CHAR_DATA	*victim;
	MEMBER_DATA *member;
	char		 arg1[MIL];
	char		 arg2[MIL];

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono farlo.\r\n" );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !ch->pg->clan || !ch->pg->member )
	{
		send_to_char( ch, "Non appartengo a nessun clan.\r\n" );
		return;
	}

	if ( ch->pg->member->rank != ch->pg->clan->rank_limit-1 )
	{
		send_to_char( ch, "Non posseggo il rango necessario per farlo.\r\n" );
		return;
	}

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "Arruolare chi?\r\n" );
		return;
	}

	victim = get_player_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo avventuriero non si trova qui.\r\n" );
		return;
	}

	if ( get_level(victim) <= LVL_NEWBIE )
	{
		send_to_char( ch, "Questo avventuriero è ancora troppo inesperto per poter essere arruolato.\r\n" );
		return;
	}

	clan = ch->pg->clan;
	if ( victim->pg->clan )
	{
		if ( victim->pg->clan == clan )
			send_to_char( ch, "Questo avventuriero è già nel Clan!\r\n" );
		else
			send_to_char( ch, "Questo avventuriero fa già parte di un Clan.\r\n" );
		return;
	}

	if ( clan->member_limit && clan->members >= clan->member_limit )
	{
		send_to_char( ch, "Il Clan è troppo grande per poter sostenere nuove reclute.\r\n" );
		return;
	}

	if ( !change_rank(ch, victim, clan, 0, 0) )
	{
		ch_printf( ch, "%s non ha i requisiti adatti per entrare nel Clan.\r\n", victim->name );
		return;
	}

	victim->pg->clan = clan;
	victim->pg->member->rank = 0;
	victim->pg->learned_lang[LANGUAGE_CLAN] = 100;

	CREATE( member, MEMBER_DATA, 1 );
	member->name = str_dup( victim->name );
	member->rank = victim->pg->member->rank;
	member->inducted_by = str_dup( ch->name );
	member->inducted_date = current_time;
	LINK( member, clan->first_member, clan->last_member, next, prev );
	clan->members++;

	act( AT_MAGIC, "Arruolo $N nel $t", ch, clan->short_descr, victim, TO_CHAR );
	act( AT_MAGIC, "$n arruola $N nel $t", ch, clan->short_descr, victim, TO_ROOM );
	act( AT_MAGIC, "$n mi arruola nel $t", ch, clan->short_descr, victim, TO_VICT );

	save_player( victim );
	fwrite_clan( clan );
}


/*
 * Comando Admin e di CapoClan per escludere un membro dal Clan
 */
DO_RET do_outcast( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	CLAN_DATA	*clan;
	MEMBER_DATA	*member;
	char		 arg[MIL];

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono farlo.\r\n" );
		return;
	}

	if ( !ch->pg->clan || !ch->pg->member )
	{
		send_to_char( ch, "Non appartengo a nessun clan.\r\n" );
		return;
	}

	if ( ch->pg->member->rank != ch->pg->clan->rank_limit-1 )
	{
		send_to_char( ch, "Non posseggo il rango necessario per farlo.\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Chi dovrei allontanare dal Clan?\r\n" );
		return;
	}

	victim = get_player_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Questo avventuriero non si trova nei paraggi.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Allontanarmi da solo dal Clan?\r\n" );
		return;
	}

	if ( get_trust(victim) > get_trust(ch) )
	{
		send_to_char( ch, "Questo avventuriero è troppo importante perchè io lo possa escludere dal Clan.\r\n" );
		return;
	}

	if ( victim->pg->clan != ch->pg->clan )
	{
		send_to_char( ch, "Questo avventuriero non fa parte del Clan!\r\n" );
		return;
	}

	clan = victim->pg->clan;

	if ( victim->speaking == LANGUAGE_CLAN )
		victim->speaking = get_lang_speakable( victim, victim );
	victim->pg->learned_lang[LANGUAGE_CLAN] = 0;
	victim->pg->clan		 = NULL;
	victim->pg->member->rank = 0;

	member = get_clan_member( victim->name, clan, TRUE );
	UNLINK( member, clan->first_member, clan->last_member, next, prev );
	DISPOSE( member->name );
	DISPOSE( member );
	clan->members--;

	act( AT_MAGIC, "Allontano $N dal $t", ch, clan->short_descr, victim, TO_CHAR );
	act( AT_MAGIC, "$n allontana $N dal $t", ch, clan->short_descr, victim, TO_ROOM );
	act( AT_MAGIC, "$n mi allontana dal $t", ch, clan->short_descr, victim, TO_VICT );

	save_player( victim );	/* clan gets saved when pfile is saved */
	fwrite_clan( clan );
}


/*
 * Comando admin o per leader di clan per cambiare il rank di un membro
 */
DO_RET do_rank( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA	*clan;
	CHAR_DATA	*victim;
	char		 arg1[MIL];
	int			 rank;
	int			 x;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Non puoi farlo.\r\n" );
		return;
	}

	if ( !ch->pg->clan || !ch->pg->member )
	{
		send_to_char( ch, "Non appartengo a nessun clan.\r\n" );
		return;
	}

	if ( ch->pg->member->rank != ch->pg->clan->rank_limit-1 )
	{
		send_to_char( ch, "Non posseggo il rango necessario per farlo.\r\n" );
		return;
	}

	if ( ch->pg->clan->rank_limit == 1 )
	{
		send_to_char( ch, "Il Clan non possiede ranghi al suo interno.\r\n" );
		return;
	}

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		ch_printf( ch, "Utilizzo:  %s <nome membro> <rango clan>\r\n", translate_command(ch, "rank") );
		return;
	}

	argument = one_argument( argument, arg1 );
	victim = get_player_room( ch, arg1, FALSE );
	if ( !victim )
	{
		send_to_char( ch, "Non trovo questo avventuriero nei paraggi.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Non posso cambiare il mio grado da solo.\r\n" );	/* (FF) oppure sì, con qualche permesso a livello di rango */
		return;
	}

	if ( !victim->pg->clan || !victim->pg->member || victim->pg->clan != ch->pg->clan )
	{
		send_to_char( ch, "Questo avventuriero non fa parte del nostro Clan.\r\n" );
		return;
	}

	clan = victim->pg->clan;

	rank = get_clan_rank( argument, clan, TRUE );
	if ( rank == -1 )
	{
		send_to_char( ch, "Questa tipologia di rango è inesistente nel Clan.\r\n" );
		return;
	}

	if ( victim->pg->member->rank == rank )
	{
		ch_printf( ch, "Il rango di %s è identico a quello che vorrei assegnargli.\r\n", victim->name );
		return;
	}

	if ( victim->pg->member->rank > rank )
	{
		/* Nel degradare un membro del clan non vengono effettuati i check sulle restrizioni di rango */

		act( AT_MAGIC, "Degrado $N come $t", ch, clan->rank[rank]->name, victim, TO_CHAR );
		act( AT_MAGIC, "$n degrada $N come $t", ch, clan->rank[rank]->name, victim, TO_ROOM );
		act( AT_MAGIC, "$n mi degrada come $t", ch, clan->rank[rank]->name, victim, TO_VICT );
	}
	else
	{
		for ( x = victim->pg->member->rank;  x < rank-1;  x++ )
		{
			if ( !change_rank(ch, victim, clan, x, x+1) )
			{
				ch_printf( ch, "Non posso aumentare %s dal rango %s a quello %s.\r\n",
					victim->name, clan->rank[x]->name, clan->rank[x+1]->name );
				return;
			}
		}

		act( AT_MAGIC, "Faccio salire di rango $N come $t", ch, clan->rank[rank]->name, victim, TO_CHAR );
		act( AT_MAGIC, "$n fa salire di rango $N come $t", ch, clan->rank[rank]->name, victim, TO_ROOM );
		act( AT_MAGIC, "$n mi fa salire di rango come $t", ch, clan->rank[rank]->name, victim, TO_VICT );
	}

	victim->pg->member->rank = rank;
}


/*
 * Comando admin e di leader clan per visualizzare i ranghi
 */
DO_RET do_showranks( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA *clan;
	int		   x;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_showranks: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( IS_ADMIN(ch) )
	{
		if ( !VALID_STR(argument) )
		{
			ch_printf( ch, "&YSintassi&w: %s <nomeclan>\r\n", translate_command(ch, "showranks") );
			return;
		}

		clan = get_clan( argument, FALSE );
		if ( !clan )
		{
			send_to_char( ch, "Non esiste questo Clan.\r\n" );
			return;
		}
	}
	else
	{
		if ( !ch->pg->clan || !ch->pg->member )
		{
			send_to_char( ch, "Non appartengo a nessun clan.\r\n" );
			return;
		}

		if ( ch->pg->member->rank != ch->pg->clan->rank_limit-1 )
		{
			send_to_char( ch, "Non posseggo il rango necessario per farlo.\r\n" );
			return;
		}

		clan = ch->pg->clan;
	}

	pager_printf( ch, "Limite di numero ranghi: %d\r\n", clan->rank_limit );
	for ( x = 0;  x < clan->rank_limit;  x++ )
	{
		pager_printf( ch, "\r\n- Rango %d %s:\r\n", x+1, clan->rank[x]->name );
		if ( clan->rank[x]->max_members == -1 )
			send_to_pager( ch, "  Massimo membri:   Nessuno\r\n" );
		else
			pager_printf( ch, "  Massimo membri:   %d\r\n", clan->rank[x]->max_members );
		pager_printf( ch, "  Classi accettate: %s\r\n", IS_EMPTY(clan->rank[x]->classes) ? "Tutte" : code_bit(NULL, clan->rank[x]->classes, CODE_CLASS) );
		pager_printf( ch, "  Razze accettate:  %s\r\n", IS_EMPTY(clan->rank[x]->races) ? "Tutte" : code_bit(NULL, clan->rank[x]->races, CODE_RACE) );
		pager_printf( ch, "  Allin minimo:     %s&w\r\n", get_align_alias(clan->rank[x]->align_min) );
		pager_printf( ch, "  Allin massimo:    %s&w\r\n", get_align_alias(clan->rank[x]->align_max) );
	}
}


/*
 * Struttura dati per il comando whoclan
 */
typedef struct	whoclan_data	WHOCLAN_DATA;

struct whoclan_data
{
	WHOCLAN_DATA	*next;
	WHOCLAN_DATA	*prev;
	char			*line;
	bool			 online;
	int				 rank;
};

/*
 * Comando che elenca tutti i membri di un clan suddividendoli per ranghi
 */
DO_RET do_whoclan( CHAR_DATA *ch, char *argument )
{
	WHOCLAN_DATA *first_whoclan = NULL;
	WHOCLAN_DATA *last_whoclan = NULL;
	WHOCLAN_DATA *whoclan;
	MEMBER_DATA  *member;
	CLAN_DATA	 *clan;
	char		  letter;
	int			  x;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_whoclan: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( IS_ADMIN(ch) )
	{
		if ( !VALID_STR(argument) )
		{
			ch_printf( ch, "&YSintassi&w: %s <nomeclan>\r\n", translate_command(ch, "whoclan") );
			return;
		}

		clan = get_clan( argument, FALSE );
		if ( !clan )
		{
			send_to_char( ch, "Non esiste questo Clan.\r\n" );
			return;
		}
	}
	else
	{
		if ( !ch->pg->clan || !ch->pg->member )
		{
			send_to_char( ch, "Non appartengo a nessun Clan.\r\n" );
			return;
		}

		if ( ch->pg->member->rank != ch->pg->clan->rank_limit-1 )
		{
			send_to_char( ch, "Non posseggo il rango necessario per farlo.\r\n" );
			return;
		}

		clan = ch->pg->clan;
	}

	for ( letter = 'A';  letter <= 'Z';  letter++ )
	{
		for ( member = clan->first_member;  member;  member = member->next )
		{
			WHOCLAN_DATA *whoclannew;
			CHAR_DATA	 *victim;	/* recupera il pg con il nome del membro se è online, oppure recupera la struttura offline */
			char		  buf[MSL];
			char		  buf_race[MIL];
			bool		  online = TRUE;

			if ( member->name[0] != letter )
				continue;

			victim = get_char_mud( ch, member->name, FALSE );
			if ( !victim )
			{
				online = FALSE;
				victim = get_offline( member->name, TRUE );
				if ( !victim )
				{
					send_log( NULL, LOG_BUG, "do_whoclan: membro del clan %s inesistente tra i player", member->name );
					continue;
				}
			}

			CREATE( whoclannew, WHOCLAN_DATA, 1 );
			whoclannew->online = online;
			whoclannew->rank	= member->rank;

			sprintf( buf_race, "%s%*s", get_race(victim, TRUE), 13 - strlen_nocolor(get_race(victim, TRUE)), "" );
			if ( IS_ADMIN(ch) || ch->pg->member->rank == ch->pg->clan->rank_limit-1 )
			{
				char buf_date[MIL];
				char buf_log[MIL];

				strcpy( buf_date, friendly_ctime_nohour(&member->inducted_date) );
				strcpy( buf_log, friendly_ctime_nohour(&victim->pg->logon) );	/* (TT) */

				sprintf( buf, "&%c%-12s&w %s %-12s %-s  %-s\r\n",
					online ? 'w' : 'z',
					member->name,
					buf_race,
					member->inducted_by,
					buf_date,
					buf_log );
			}
			else
			{
				sprintf( buf, "&%c%-12s&w %s\r\n",
					online ? 'w' : 'z',
					member->name,
					buf_race );
			}
			whoclannew->line = str_dup( buf );

			LINK( whoclannew, first_whoclan, last_whoclan, next, prev );
		}
	}

	if ( IS_ADMIN(ch) || ch->pg->member->rank == ch->pg->clan->rank_limit-1 )
		send_to_pager( ch, "Nome         Razza         Invitato da  Nel giorno  Ultima connessione\r\n" );
	else
		send_to_pager( ch, "Nome         Razza\r\n" );
	for ( x = clan->rank_limit-1;  x >= 0 ;  x-- )
	{
		bool	found = FALSE;

		pager_printf( ch, "- Rango: &W%s&w\r\n", clan->rank[x]->name );
		/* Stampa prima quelli online */
		for ( whoclan = first_whoclan;  whoclan;  whoclan = whoclan->next )
		{
			if ( whoclan->rank != x )
				continue;

			if ( whoclan->online )
			{
				send_to_pager( ch, whoclan->line );
				found = TRUE;
			}
		}
		for ( whoclan = first_whoclan;  whoclan;  whoclan = whoclan->next )
		{
			if ( whoclan->rank != x )
				continue;

			if ( !whoclan->online )
			{
				send_to_pager( ch, whoclan->line );
				found = TRUE;
			}
		}
		if ( !found )
			send_to_pager( ch, "Nessun membro.\r\n" );
	}
	pager_printf( ch, "\r\nMembri totali %d/%d\r\n", clan->members, clan->member_limit );

	/* Libera dalla memoria la lista dei whoclan */
	for ( whoclan = first_whoclan;  whoclan;  whoclan = first_whoclan )	/* (TT) */
	{
		UNLINK( whoclan, first_whoclan, last_whoclan, next, prev );
		DISPOSE( whoclan->line );
		DISPOSE( whoclan );
	}
}


/*
 * Gestisce il comando victories e defeats
 */
DO_RET victories_defeats_handler( CHAR_DATA *ch, char *argument, bool victories )
{
	CLAN_DATA *clan;
	char	   filename[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "victories_defeats_handler: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I Mob non possono farlo.\r\n" );
		return;
	}

	if ( IS_ADMIN(ch) )
	{
		if ( !VALID_STR(argument) )
		{
			ch_printf( ch, "&YSintassi&w: %s\r\n",
				victories ? translate_command(ch, "victories") : translate_command(ch, "defeats") );
			return;
		}

		clan = get_clan( argument, FALSE );
		if ( !clan )
		{
			ch_printf( ch, "Clan inesistente con nome %s.\r\n", argument );
			return;
		}
	}
	else
	{
		if ( (!ch->pg->clan || !ch->pg->member) && !IS_ADMIN(ch) )
		{
			send_to_char( ch, "Non appartengo a nessun clan.\r\n" );
			return;
		}

		clan = ch->pg->clan;
	}

	sprintf( filename, victories ? "%s%s.victories" : "%s%s.defeats", CLAN_DIR, clan->name );

	/* Gli admin e i leader del clan possono cancellare il file */
	if ( IS_ADMIN(ch) || ch->pg->member->rank == ch->pg->clan->rank_limit-1 )
	{
		if ( is_name(argument, "pulisci clean") )
		{
			MUD_FILE  *fp;

			fp = mud_fopen( "", filename, "w", FALSE );
			if ( fp )
				MUD_FCLOSE( fp );

			pager_printf( ch, "\r\nIl registro delle %s è stato cancellato.\r\n",
				victories ? "vittorie" : "sconfitte" );
			return;
		}
	}

	set_char_color( AT_MAGENTA, ch );
	pager_printf( ch, "\r\nNome         %s     Clan           Stanza\r\n",
		victories ? "Vittima " : "Uccisore" );
	show_file( ch->desc, filename, FALSE );
}

/* Comando che visualizza le vittorie del clan */
DO_RET do_victories( CHAR_DATA *ch, char *argument )
{
	victories_defeats_handler( ch, argument, TRUE );
}

/* Comando che visualizza le sconfitte del clan */
DO_RET do_defeats( CHAR_DATA *ch, char *argument )
{
	victories_defeats_handler( ch, argument, FALSE );
}


/*
 * Comando Admin che visualizza le informazioni di un clan
 */
DO_RET do_showclans( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA	*clan;
	MEMBER_DATA	*member;
	char		 buf[MIL];

	set_char_color( AT_PLAIN, ch );

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		int		count = 0;

		send_to_char( ch, "Usage: showclan <clan>\r\n\r\n" );

		send_to_char( ch, "Clan           Leader         Pkill Membri Oggetto\r\n" );
		send_to_char( ch, "--------------------------------------------------------------------------------\r\n" );

		for ( clan = first_clan;  clan;  clan = clan->next )
		{
			char	leader[MIL];
			int		cnt = 0;

			/* crea la stringa con il leader, se ce ne sono tanti aggiunge un paio di punti */
			for ( member = clan->first_member;  member;  member = member->next )
			{
				if ( member->rank == clan->rank_limit-1 )
				{
					strcpy( leader, member->name );
					cnt++;
				}
			}
			if ( cnt > 1 )
				strcat( leader, ".." );

			set_char_color( AT_GREY, ch );
			ch_printf( ch, "%-14s %-14s %5d %6d %-37.37s",
				clan->name,
				leader,
				clan->pkills,
				clan->members,
				clan->object ? clan->object->short_descr : "Nessun oggetto" );
			count++;
		}

		if ( count == 0 )
			send_to_char( ch, "There are no Clans currently formed.\r\n" );

		return;
	}

	clan = get_clan( argument, FALSE );
	if ( !clan )
	{
		send_to_char( ch, "Non esiste questo Clan.\r\n" );
		return;
	}

	pager_printf( ch, "&wClan        &W%s&w\r\n", clan->name );
	pager_printf( ch, "Descr Corta   %s\r\n", clan->short_descr );
	pager_printf( ch, "Descrizione   %s\r\n", clan->description );
	pager_printf( ch, "Motto         %s\r\n", clan->motto );
	pager_printf( ch, "Msg Riunione  %s\r\n", clan->msg_reunion );
	pager_printf( ch, "Msg Allarme   %s\r\n", clan->msg_alarm );
	pager_printf( ch, "Msg Ogg Perso %s\r\n", clan->msg_obj_lost );
	pager_printf( ch, "Msg Ogg Preso %s\r\n", clan->msg_obj_aquired );
	pager_printf( ch, "Tipo          %s\r\n", code_str(NULL, clan->type, CODE_CLANTYPE) );
	pager_printf( ch, "Flags         %s\r\n", code_bit(NULL, clan->flags, CODE_CLANFLAG) );
	pager_printf( ch, "PgKill        %d\r\n", clan->pkills );
	pager_printf( ch, "PgDeath       %d\r\n", clan->pdeaths );
	pager_printf( ch, "MobKill       %d\r\n", clan->mkills );
	pager_printf( ch, "MobDeath      %d\r\n", clan->mdeaths );
	pager_printf( ch, "IllegalPkill  %d\r\n", clan->illegal_pk );
	pager_printf( ch, "Assalti       %d\r\n", clan->strikes );
	pager_printf( ch, "Allineamento  %d\r\n", compute_clan_align(clan, -1) );
	pager_printf( ch, "Punteggio     %d\r\n", get_clan_score(clan) );

	if ( clan->guard1 )
		pager_printf( ch, "Mob Guardia 1 %s (#&W%d&w)\r\n", clan->guard1->short_descr, clan->guard1->vnum );
	else
		send_to_pager( ch, "Mob Guardia 1 Nessuna" );
	if ( clan->guard2 )
		pager_printf( ch, "Mob Guardia 2 %s (#&W%d&w)\r\n", clan->guard2->short_descr, clan->guard2->vnum );
	else
		send_to_pager( ch, "Mob Guardia 2 Nessuna" );
	if ( clan->board )
		pager_printf( ch, "Bacheca       %s (#&W%d&w)\r\n", clan->board->short_descr, clan->board->vnum );
	else
		send_to_pager( ch, "Bacheca       Nessuna" );
	if ( clan->badge )
		pager_printf( ch, "Stemma        %s (#&W%d&w)\r\n", clan->badge->short_descr, clan->badge->vnum );
	else
		send_to_pager( ch, "Stemma        Nessuno" );
	if ( clan->object )
		pager_printf( ch, "Oggetto Clan  %s (#&W%d&w)\r\n", clan->object->short_descr, clan->object->vnum );
	else
		send_to_pager( ch, "Oggetto Clan  Nessuno" );
	if ( clan->recall )
		pager_printf( ch, "Stanza Recall %s (#&W%d&w)\r\n", clan->recall->name, clan->recall->vnum );
	else
		send_to_pager( ch, "Stanza Recall Nessuna" );

	sprintf( buf, "showranks %s", clan->name );
	send_command( ch, buf, CO );

	sprintf( buf, "whoclan %s", clan->name );
	send_command( ch, buf, CO );
}
