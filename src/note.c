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
 >						Special boards module								<
\****************************************************************************/


/* TODO:
- Uccellame o Pixie o altro che invia i messaggi, i pixie sono i più veloci e costano di più
- Permettere di impacchettare anche oggetti da inviare a pg
- Piazzare un tempo di invio di mail rpg, con quel sistema di eventi
- Aggiungere la cassetta postale quando verrà fatta la casa
- Dovrei cercare di non far creare e impugnare la pergamena per la scrittura delle note, visto che sono offgdr
- Sistema di expire per le note nelle board, con tempo max diverso per ciascuna board magari
- Se ti viene inviato una nota-mail rimane paper e non viene salvata tra le mail, più realistico, anche se forse confusionario, ci vorrebbero i contenitori per tipo lettera da vendere in posta
 */


#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "mud.h"
#include "build.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "information.h"
#include "interpret.h"
#include "note.h"
#include "object_interact.h"
#include "room.h"


/*
 * Definizioni di file e della dir contenente le bacheche
 */
#define BOARD_DIR				"../boards/"	/* Directory delle bacheche e note pubbliche */
#define NOTE_DIR	PLAYER_DIR	"/notes/"		/* Directory delle note o mail private dei giocatori */
#define BOARD_FILE				"boards.dat"	/* File in cui vengono caricate e salvate le bacheche */

/* Definizioni per le azioni sulle bacheche */
#define	ACTION_READ		0
#define	ACTION_POST		1
#define	ACTION_REMOVE	2


/*
 * Variabili esterne
 */
BOARD_DATA	*first_board = NULL;
BOARD_DATA	*last_board	 = NULL;
int			 top_board	 = 0;

NOTE_DATA	*first_note	 = NULL;
NOTE_DATA	*last_note	 = NULL;
int			 top_note	 = 0;


/* Costanti per i codici riguardanti i tipi di nota */
const CODE_DATA code_note[] =
{
	{ NOTE_NONE,	"NOTE_NONE",	"nessuna"	},
	{ NOTE_NOTE,	"NOTE_NOTE",	"nota"		},
	{ NOTE_MAIL,	"NOTE_MAIL",	"lettera"	},
	{ NOTE_BOARD,	"NOTE_BOARD",	"bacheca"	}
};
const int max_check_note = sizeof(code_note)/sizeof(CODE_DATA);


/* Costanti per i codici riguardanti i tipi di votazione */
const CODE_DATA code_vote[] =
{
	{ VOTE_NONE,	"VOTE_NONE",	"nessuna"	},
	{ VOTE_OPEN,	"VOTE_OPEN",	"aperta"	},
	{ VOTE_CLOSED,	"VOTE_CLOSED",	"chiusa"	}
};
const int max_check_vote = sizeof(code_vote)/sizeof(CODE_DATA);


/*
 * Legge una sezione di board da file
 */
void fread_board( MUD_FILE *fp )
{
	BOARD_DATA *board;

	CREATE( board, BOARD_DATA, 1 );

	for ( ; ; )
	{
		char	*word;

		word   = feof(fp->file)  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'				  )									fread_to_eol( fp );
		else if ( !strcmp(word, "ClanPost"		) )		board->clan_post =			fread_string( fp );
		else if ( !strcmp(word, "ClanRead"		) )		board->clan_read =			fread_string( fp );
		else if ( !strcmp(word, "ClanRemove"	) )		board->clan_remove =		fread_string( fp );
		else if ( !strcmp(word, "ExtraPosters"  ) )		board->extra_posters =		fread_string( fp );
		else if ( !strcmp(word, "ExtraReaders"  ) )		board->extra_readers =		fread_string( fp );
		else if ( !strcmp(word, "ExtraRemovers" ) )		board->extra_removers = 	fread_string( fp );
		else if ( !strcmp(word, "End"			) )		break;
		else if ( !strcmp(word, "Filename"		) )		board->mail_file =			fread_string( fp );
		else if ( !strcmp(word, "MinTrustRead"  ) )		board->min_trust_read =		code_code( fp, fread_word(fp), CODE_TRUST );
		else if ( !strcmp(word, "MinTrustPost"  ) )		board->min_trust_post =		code_code( fp, fread_word(fp), CODE_TRUST );
		else if ( !strcmp(word, "MinTrustRemove") )		board->min_trust_remove =	code_code( fp, fread_word(fp), CODE_TRUST );
		else if ( !strcmp(word, "MaxPosts"		) )		board->max_posts =			fread_number( fp );
		else if ( !strcmp(word, "Vnum"			) )		board->board_obj =			get_obj_proto( fp, fread_number(fp) );
		else
			send_log( fp, LOG_FREAD, "fread_board: no match: %s", word );
	} /* chiude il for */

	if ( !VALID_STR(board->mail_file) )
	{
		send_log( fp, LOG_FREAD, "fread_board: nome del file con le mail non valido" );
		exit( EXIT_FAILURE );
	}

	board->num_posts	= 0;

	/* Se tutte le 6 stringhe qui sotto vossero NULL significa che la bachceca è solo per admin */
	if ( !board->clan_read		)	board->clan_read	  = str_dup( "" );
	if ( !board->clan_post		)	board->clan_post	  = str_dup( "" );
	if ( !board->clan_remove	)	board->clan_remove	  = str_dup( "" );
	if ( !board->extra_readers	)	board->extra_readers  = str_dup( "" );
	if ( !board->extra_posters	)	board->extra_posters  = str_dup( "" );
	if ( !board->extra_removers )	board->extra_removers = str_dup( "" );

	top_board++;
	LINK( board, first_board, last_board, next, prev );
}


/*
 * Legge una sezione di nota da un file passato
 */
NOTE_DATA *fread_note( MUD_FILE *fp )
{
	NOTE_DATA *pnote;

	CREATE( pnote, NOTE_DATA, 1 );

	for ( ; ; )
	{
		char *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_note: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "End"		  ) )	break;
		else if ( !str_cmp(word, "Date"		  ) )	pnote->date	=			fread_string( fp );
		else if	( !str_cmp(word, "Sender"	  ) )	pnote->sender =			fread_string( fp );
		else if ( !str_cmp(word, "To"		  ) )	pnote->to_list =		fread_string( fp );
		else if ( !str_cmp(word, "Subject"	  ) )	pnote->subject =		fread_string( fp );
		else if ( !str_cmp(word, "Text"		  ) )	pnote->text	=			fread_string( fp );
		else if ( !str_cmp(word, "Type"		  ) )	pnote->type =			code_code( fp, fread_word(fp), CODE_NOTE );
		else if ( !str_cmp(word, "Voting"	  ) )	pnote->voting =			code_code( fp, fread_word(fp), CODE_VOTE );
		else if ( !str_cmp(word, "VotesYes"	  ) )	pnote->votes_yes =		fread_string( fp );
		else if ( !str_cmp(word, "VotesNo"	  ) )	pnote->votes_no =		fread_string( fp );
		else if ( !str_cmp(word, "Abstentions") )	pnote->abstentions =	fread_string( fp );
		else
			send_log( fp, LOG_FREAD, "fread_note: etichetta non trovata: %s", word );
	}

	if ( !VALID_STR(pnote->date) )
	{
		send_log( fp, LOG_FREAD, "fread_note: data non valida" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(pnote->sender) )
	{
		send_log( fp, LOG_FREAD, "fread_note: sender non valida" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(pnote->to_list) )
	{
		send_log( fp, LOG_FREAD, "fread_note: to_list non è valida" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(pnote->subject) )
	{
		send_log( fp, LOG_FREAD, "fread_note: subject non è valida" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(pnote->text) )
	{
		send_log( fp, LOG_FREAD, "fread_note: text non è valida" );
		exit( EXIT_FAILURE );
	}

	if ( pnote->type <= NOTE_NONE || pnote->type > NOTE_BOARD )
	{
		send_log( fp, LOG_BUG, "fread_note: tipo di nota acquisito errato: %d", pnote->type );
		exit( EXIT_FAILURE );
	}

	if ( pnote->voting < VOTE_NONE || pnote->voting > VOTE_CLOSED )
	{
		send_log( fp, LOG_BUG, "fread_note: tipo di votazione acquisita errata: %d", pnote->voting );
		exit( EXIT_FAILURE );
	}

	if ( !pnote->votes_yes	 )	pnote->votes_yes	= str_dup( "" );
	if ( !pnote->votes_no	 )	pnote->votes_no		= str_dup( "" );
	if ( !pnote->abstentions )	pnote->abstentions	= str_dup( "" );

	return pnote;
}


/*
 * Carica tutte le lettere di una bacheca passandogli una board
 * Oppure tutte le note inviate da un giocatore se board è NULL
 */
void load_notes( char *filename, BOARD_DATA *board )
{
	MUD_FILE	*fp;
	NOTE_DATA	*pnote;

	fp = mud_fopen( "", filename, "r", FALSE );
	if ( !fp )
		return;

	for ( ; ; )
	{
		char   *word;
		char	letter;

		letter = fread_letter( fp );

		if ( letter == EOF )
		{
			send_log( fp, LOG_FREAD, "fread_section: fine del file prematura durante la lettura" );
			if ( fBootDb )
				exit( EXIT_FAILURE );
		}

		/* Salta a fine riga se trova il carattere di commento */
		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		/* Errore se non trova l'etichetta di caricamento sezione. */
		if ( letter != '#' )
		{
			send_log( fp, LOG_FREAD, "fread_section: # non trovato" );
			break;
		}

		word = fread_word( fp );
		if ( !strcmp(word, "NOTE") )
		{
			pnote = fread_note( fp );
			if ( board )
			{
				board->num_posts++;
				LINK( pnote, board->first_mail, board->last_mail, next, prev );
			}
			else
			{
				top_note++;
				LINK( pnote, first_note, last_note, next, prev );
			}
		}
		else if ( !strcmp(word, "END") )	/* Ricordarsi di inserire un a capo dopo l'etichetta #END nei file altrimenti non lo legge correttamente */
		{
			break;
		}
		else
		{
			send_log( fp, LOG_FREAD, "load_notes: sezione errata %s", word );
			if ( fBootDb )
				exit( EXIT_FAILURE );
		}
	} /* chiude il for */
}


/*
 * Carica tutte le note delle board
 */
void load_boards( void )
{
	BOARD_DATA	*board;
	char		 path[MIL];

	/* Legge tutte le strutture base delle bacheche */
	sprintf( path, "%s%s", BOARD_DIR, BOARD_FILE );
	fread_section( path, "BOARD", fread_board, FALSE );

	/* Carica tutte le lettere per ogni bacheca */
	for ( board = first_board;  board;  board = board->next )
	{
		sprintf( path, "%s%s", BOARD_DIR, board->mail_file );
		load_notes( path, board );
	}
}


/*
 * Carica tutte le note o mail inviate dai pg
 */
void load_player_notes( void )
{
	DIR			  *dp;
	struct dirent *dentry;

	dp = opendir( NOTE_DIR );
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
			load_notes( dentry->d_name, NULL );
		dentry = readdir( dp );
	}
	closedir( dp );
}


/*
 * Scrive tutte le informazioni delle board su di un file
 */
void fwrite_boards( void )
{
	BOARD_DATA	*board;
	MUD_FILE	*fpout;

	fpout = mud_fopen( BOARD_DIR, BOARD_FILE, "w", TRUE );
	if ( !fpout )
		return;

	for ( board = first_board;  board;  board = board->next )
	{
		fprintf( fpout->file, "#BOARD\n" );
		fprintf( fpout->file, "Vnum           %d\n",	board->board_obj->vnum );
		fprintf( fpout->file, "NoteFile       %s~\n",	board->mail_file );
		fprintf( fpout->file, "MinTrustRead   %s\n",	code_str(fpout, board->min_trust_read,	  CODE_TRUST) );
		fprintf( fpout->file, "MinTrustPost   %s\n",	code_str(fpout, board->min_trust_post,	  CODE_TRUST) );
		fprintf( fpout->file, "MinTrustRemove %s\n",	code_str(fpout, board->min_trust_remove, CODE_TRUST) );
		fprintf( fpout->file, "MaxPosts       %d\n",	board->max_posts );
		if ( VALID_STR(board->clan_read) )
			fprintf( fpout->file, "ClanRead       %s~\n",	board->clan_read );
		if ( VALID_STR(board->clan_post) )
			fprintf( fpout->file, "ClanPost       %s~\n",	board->clan_post );
		if ( VALID_STR(board->clan_remove) )
			fprintf( fpout->file, "ClanRemove     %s~\n",	board->clan_remove );
		if ( VALID_STR(board->extra_readers) )
			fprintf( fpout->file, "ExtraReaders   %s~\n",	board->extra_readers );
		if ( VALID_STR(board->extra_posters) )
			fprintf( fpout->file, "ExtraPosters   %s~\n",	board->extra_posters );
		if ( VALID_STR(board->extra_removers) )
			fprintf( fpout->file, "ExtraRemovers  %s~\n",	board->extra_removers );
		fprintf( fpout->file, "End\n\n" );
	}

	fprintf( fpout->file, "#END\n" );
	MUD_FCLOSE( fpout );
}


/*
 * Scrive una singola nota o lettera
 */
void fwrite_note( MUD_FILE *fp, NOTE_DATA *pnote )
{
	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fwrite_note: fp è NULL" );
		return;
	}

	if ( !pnote )
	{
		send_log( NULL, LOG_BUG, "fwrite_note: pnote è NULL" );
		return;
	}

	fputs( "#NOTE", fp->file );
	fprintf( fp->file, "Sender      %s~\n",		 pnote->sender );
	fprintf( fp->file, "Date        %21.21s~\n", pnote->date );
	fprintf( fp->file, "To 	        %s~\n",		 pnote->to_list );
	fprintf( fp->file, "Subject     %s~\n",		 pnote->subject );
	fprintf( fp->file, "Type        %s\n",		 code_str(fp, pnote->type, CODE_NOTE) );
	fprintf( fp->file, "Voting      %s\n",		 code_str(fp, pnote->voting, CODE_VOTE) );
	if ( VALID_STR(pnote->votes_yes) )
		fprintf( fp->file, "VotesYes    %s~\n",	 pnote->votes_yes );
	if ( VALID_STR(pnote->votes_no) )
		fprintf( fp->file, "VotesNo     %s~\n",	 pnote->votes_no );
	if ( VALID_STR(pnote->abstentions) )
		fprintf( fp->file, "Abstentions %s~\n",	 pnote->abstentions );
	fprintf( fp->file, "Text\n%s~\n",			 pnote->text );
	fputs( "End\n\n", fp->file );
}


/*
 * Scrive tutte le note di una board
 */
void fwrite_notes_board( BOARD_DATA *board )
{
	MUD_FILE  *fp;
	NOTE_DATA *pmail;

	/* Rewrite entire list */
	fp = mud_fopen( BOARD_DIR, board->mail_file, "w", TRUE );
	if ( !fp )
		return;

	for ( pmail = board->first_mail;  pmail;  pmail = pmail->next )
		fwrite_note( fp, pmail );

	fputs( "\n#END\n", fp->file );
	MUD_FCLOSE( fp );
}


/*
 * Scrive tutte le note di un pg
 */
void fwrite_notes_player( CHAR_DATA *ch )
{
	MUD_FILE  *fp;
	NOTE_DATA *pnote;
	char	   filename[MIL];

	/* Rewrite entire list */
	sprintf( filename, "%s%s.notes", NOTE_DIR, ch->name );
	fp = mud_fopen( "", filename, "w", TRUE );
	if ( !fp )
		return;

	for ( pnote = first_note;  pnote;  pnote = pnote->next )
	{
		if ( !str_cmp(pnote->sender, ch->name) )
			fwrite_note( fp, pnote );
	}

	fputs( "\n#END\n", fp->file );
	MUD_FCLOSE( fp );
}


/*
 * Rinomina un file di note
 */
void rename_notes( CHAR_DATA *ch, const char *newname )
{
	BOARD_DATA	*board;
	NOTE_DATA	*note;
	bool		 found;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "rename_notes: ch passato è NULL" );
		return;
	}

	if ( !VALID_STR(newname) )
	{
		send_log( NULL, LOG_BUG, "rename_notes: newname passato non è valido" );
		return;
	}

	found = FALSE;
	for ( board = first_board;  board;  board = board->next )
	{
		for ( note = board->first_mail;  note;  note = note->next )
		{
			if ( !str_cmp(note->sender, ch->name) )
			{
				found = TRUE;
				DISPOSE( note->sender );
				note->sender = str_dup( newname );
			}
		}

		if ( found )
			fwrite_notes_board( board );
	}

	found = FALSE;
	for ( note = first_note;  note;  note = note->next )
	{
		if ( !str_cmp(note->sender, ch->name) )
		{
			found = TRUE;
			DISPOSE( note->sender );
			note->sender = str_dup( newname );
		}
	}

	if ( found )
		fwrite_notes_player( ch );
}


/*
 * Libera dalla memoria una struttura di nota
 */
void free_note( NOTE_DATA *note )
{
	if ( !note )
	{
		send_log( NULL, LOG_BUG, "free_note: note è NULL" );
		return;
	}

	DISPOSE( note->text );
	DISPOSE( note->subject );
	DISPOSE( note->to_list );
	DISPOSE( note->date );
	DISPOSE( note->sender );
	DISPOSE( note->votes_yes );
	DISPOSE( note->votes_no );
	DISPOSE( note->abstentions );

	DISPOSE( note );
}


/*
 * Rimuove dalla boarda una nota
 */
void remove_note( BOARD_DATA *board, NOTE_DATA *pnote )
{
	if ( !board )
	{
		send_log( NULL, LOG_BUG, "note remove: board è NULL" );
		return;
	}

	if ( !pnote )
	{
		send_log( NULL, LOG_BUG, "note remove: pnote è NULL" );
		return;
	}

	/* Rimuove la nota dalla lista */
	UNLINK( pnote, board->first_mail, board->last_mail, next, prev );
	free_note( pnote );

	board->num_posts--;
	fwrite_notes_board( board );
}


/*
 * Libera dalla memoria tutte le board e relativa note pubbliche
 */
void free_all_boards( void )
{
	BOARD_DATA *board;

	for ( board = first_board;  board;  board = first_board )
	{
		NOTE_DATA *mail;

		UNLINK( board, first_board, last_board, next, prev );
		top_board--;

		DISPOSE( board->mail_file );
		DISPOSE( board->clan_read );
		DISPOSE( board->clan_post );
		DISPOSE( board->clan_remove );
		DISPOSE( board->extra_readers );
		DISPOSE( board->extra_posters );
		DISPOSE( board->extra_removers );

		/* Libera dalla bacheca tutte le mail */
		for ( mail = board->first_mail;  mail;  mail = board->first_mail )
			remove_note( board, mail );
		if ( board->num_posts != 0 )
			send_log( NULL, LOG_BUG, "free_all_boards: numero di post della bacheca non è 0 dopo aver liberato le mail: %d", board->num_posts );

		DISPOSE( board );
	}

	if ( top_board != 0 )
		send_log( NULL, LOG_BUG, "free_all_boards: top_board diverso da 0 dopo aver liberato le brcheche: %d", top_board );
}


/*
 * Libera dalla memoria tutte le note dei giocatori
 */
void free_all_player_notes( void )
{
	NOTE_DATA *note;

	for ( note = first_note;  note;  note = last_note )
	{
		UNLINK( note, first_note, last_note, next, prev );
		top_note--;

		free_note( note );
	}

	if ( top_note != 0 )
		send_log( NULL, LOG_BUG, "free_all_player_notes: top_note diverso da 0 dopo aver liberato le note: %d", top_note );
}


/*
 * Controlla se si possa leggere, postare o rimuovere una nota da una board
 */
bool can_board_action( CHAR_DATA *ch, BOARD_DATA *board, const int action )
{
	char	*clan_action = NULL;
	char	*extra_action = NULL;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "can_board_action: (%d) ch è NULL", action );
		return FALSE;
	}

	if ( !board )
	{
		send_log( NULL, LOG_BUG, "can_board_action: (%d) board è NULL", action );
		return FALSE;
	}

	if ( action < 0 || action > ACTION_REMOVE )
	{
		send_log( NULL, LOG_BUG, "can_board_action: azione passata è errata: %d", action );
		return FALSE;
	}

	if ( IS_ADMIN(ch) )
		return TRUE;

	if ( IS_MOB(ch) )
		return FALSE;

	switch ( action )
	{
	  case ACTION_READ:
		clan_action =	board->clan_read;
		extra_action =	board->extra_readers;
		break;
	  case ACTION_POST:
		clan_action =	board->clan_post;
		extra_action =	board->extra_posters;
		break;
	  case ACTION_REMOVE:
		clan_action =	board->clan_remove;
		extra_action =	board->extra_removers;
		break;
	}

	/* Se l'azione è lettura o posting, se non ci sono clan o extra settati la bacheca è pubblica */
	if ( (action == ACTION_READ || action == ACTION_POST)
	  && (!VALID_STR(clan_action) && !VALID_STR(extra_action)) )
	{
		return TRUE;
	}

	/* Controlla se il clan del pg può eseguire l'azione sulla bacheca */
	if ( ch->pg->clan && is_name(ch->pg->clan->name, clan_action) )
		return TRUE;

	/* Controlla se un pg può eseguire l'azione sulla bacheca */
	if ( is_name(ch->name, extra_action) )
		return TRUE;

	return FALSE;
}


BOARD_DATA *get_board( OBJ_DATA *obj )
{
	BOARD_DATA *board;

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "get_board: obj è NULL" );
		return NULL;
	}

	for ( board = first_board;  board;  board = board->next )
	{
		if ( board->board_obj == obj->pObjProto )
			return board;
	}

	return NULL;
}


/*
 * Trova una bacheca nella stanza in cui si trova ch
 */
BOARD_DATA *find_board( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "find_board: ch è NULL" );
		return NULL;
	}

	for ( obj = ch->in_room->first_content;  obj;  obj = obj->next_content )
	{
		BOARD_DATA  *board;

		board = get_board( obj );
		if ( board )
			return board;
	}

	return NULL;
}


/*
 * Inizializza la variabile per la nota a ch
 */
void init_note( CHAR_DATA *ch )
{
	NOTE_DATA *pnote;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "note_attach: ch è NULL" );
		return;
	}

	if ( ch->pnote )
		return;

	CREATE( pnote, NOTE_DATA, 1 );
	pnote->sender	= str_dup( ch->name );
	pnote->date		= str_dup( "" );
	pnote->to_list	= str_dup( "" );
	pnote->subject	= str_dup( "" );
	pnote->text		= str_dup( "" );

	ch->pnote		= pnote;
}


OBJ_DATA *find_quill( CHAR_DATA *ch )
{
	OBJ_DATA *quill;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "find_quill: ch è NULL" );
		return NULL;
	}

	for ( quill = ch->last_carrying;  quill;  quill = quill->prev_content )
	{
		if ( !quill->pen ) 						continue;
		if ( !can_see_obj(ch, quill, TRUE) )	continue;

		return quill;
	}

	return NULL;
}


OBJ_DATA *give_parchement( CHAR_DATA *ch, const int note_type )
{
	OBJ_DATA	*paper;
	OBJ_DATA	*tmpobj;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "give_parchement: ch è NULL" );
		return NULL;
	}

	if ( note_type <= NOTE_NONE || note_type > NOTE_BOARD )
	{
		send_log( NULL, LOG_BUG, "give_parchement: tipo di nota passata errata: %d", note_type );
		return NULL;
	}

	paper = get_eq_char( ch, WEARLOC_HOLD );	/* (FF) quando son stati rifatti i values qui piazzarci una ricerca per tipo di oggetto OBJTYPE_PAPER, così cerca anche nell'inventario, e gliela deve mettere in mano */
	if ( paper && paper->paper )
	{
		/* Se ha trovato il pezzo di pergamena si assicura che sia dello stesso tipo della variabile note_type */
		if ( paper->paper->type != note_type )
		{
			if ( paper->paper->type == NOTE_NOTE )
			{
				ch_printf( ch, "Avevi iniziato a scrivere una nota ma la stai utilizzando come un%s.\r\n",
					note_type == NOTE_MAIL ? "a lettera" : " messaggio pubblico" );
				ch_printf( ch, "Utilizza il comando %s per continuare a scrivere su questa nota.\r\n", translate_command(ch, "note") );
			}
			else if ( paper->paper->type == NOTE_MAIL )
			{
				ch_printf( ch, "Avevi iniziato a scrivere una lettera ma la stai utilizzando come un%s.\r\n",
					note_type == NOTE_NOTE ? "a nota" : " messaggio pubblico" );
				ch_printf( ch, "Utilizza il comando %s per continuare a scrivere su questa lettera.\r\n", translate_command(ch, "mail") );
			}
			else
			{
				ch_printf( ch, "Avevi iniziato a scrivere un messaggio pubblico ma lo stai utilizzando come un%s.\r\n",
					note_type == NOTE_BOARD ? "a lettera" : " a nota" );
				ch_printf( ch, "Utilizza il comando %s per continuare a scrivere su questa lettera.\r\n", translate_command(ch, "board") );
			}
			return NULL;
		}

		return paper;
	}

	/* Se si è pg oppure si sta scrivendo una lettera bisogna avere una pergamena */
	if ( !IS_ADMIN(ch) && note_type != NOTE_NOTE )
	{
		send_to_char( ch, "Ho bisogno di impugnare un pezzo di pergamena per scrivere una lettera.\r\n" );
		return NULL;
	}

	/* Crea la pergamena */
	paper = make_object( get_obj_proto(NULL, VNUM_OBJ_NOTE), 0 );
	paper->paper->type = note_type;

	tmpobj = get_eq_char( ch, WEARLOC_HOLD );
	if ( tmpobj )
		unequip_char( ch, tmpobj );
	paper = obj_to_char( paper, ch );
	equip_char( ch, paper, WEARLOC_HOLD );

	send_to_char( ch, "Un pezzo di pergamena ti viene dato per poter scrivere la nota.\r\n" );

	return paper;
}


/*
 * Gestisce il comando do_mail e do_note
 */
DO_RET note_handler( CHAR_DATA *ch, char *argument, const int note_type )
{
	/* (RR) provare a sfoltire le variabili qui e inserirle nei vari punti, è più friendly */
	EXTRA_DESCR_DATA *ed = NULL;
	NOTE_DATA	*pnote;
	BOARD_DATA	*board = NULL;
	OBJ_DATA	*quill = NULL;
	OBJ_DATA	*paper = NULL;
	char		 buf[MSL];
	char		 arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "mail_handler: ch è NULL" );
		return;
	}

	if ( !ch->desc )
	{
		send_log( NULL, LOG_BUG, "mail_handler: no descriptor" );
		return;
	}

	if ( note_type <= NOTE_NONE || note_type > NOTE_BOARD )
	{
		send_log( NULL, LOG_BUG, "note_handler: tipo di nota passato errato: %d", note_type );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	switch ( ch->substate )
	{
	  default:
		break;

	  case SUB_WRITING_NOTE:
		paper = get_eq_char( ch, WEARLOC_HOLD );	/* (FF) idem anche a questo la cosa del give_parchement */
		if ( !paper || !paper->paper )
		{
			send_log( NULL, LOG_BUG, "mail_handler: player not holding paper" );
			stop_editing( ch );
			return;
		}
		ed = ch->dest_buf;
		DISPOSE( ed->description );
		ed->description = copy_buffer( ch );
		stop_editing( ch );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		char	cmd[MIL];

		if		( note_type == NOTE_NOTE )	strcpy( cmd, translate_command(ch, "note") );
		else if	( note_type == NOTE_MAIL )	strcpy( cmd, translate_command(ch, "mail") );
		else								strcpy( cmd, translate_command(ch, "board") );

		ch_printf( ch, "&YSintassi&w:  %s lista\r\n",						cmd );
		ch_printf( ch, "&YSintassi&w:  %s leggi\r\n",						cmd );
		if ( note_type != NOTE_BOARD )
			ch_printf( ch, "&YSintassi&w:  %s a <nome/i destinatari>\r\n",	cmd );
		ch_printf( ch, "&YSintassi&w:  %s soggetto <soggetto>\r\n",			cmd );
		ch_printf( ch, "&YSintassi&w:  %s scrivi <testo>\r\n",				cmd );
		ch_printf( ch, "&YSintassi&w:  %s mostra\r\n",						cmd );
		ch_printf( ch, "&YSintassi&w:  %s invia\r\n",						cmd );
		if ( note_type == NOTE_BOARD )
		{
			ch_printf( ch, "&YSintassi&w:  %s rimuovi <numero lettera>\r\n",cmd );
			ch_printf( ch, "&YSintassi&w:  %s prendi <numero lettera>\r\n",	cmd );
			ch_printf( ch, "&YSintassi&w:  %s copia <numero lettera>\r\n",	cmd );
			ch_printf( ch, "&YSintassi&w:  %s vota si/no\r\n",				cmd );
		}

		return;
	}

	set_char_color( AT_NOTE, ch );
	argument = one_argument( argument, arg );
	smash_tilde( argument );


/* punto 1 (note lista) */
	if ( is_name(arg, "lista list") )
	{
		int	 num_first;
		int	 count;

		if ( note_type == NOTE_BOARD )
		{
			board = find_board( ch );
			if ( !board )
			{
				send_to_char( ch, "Non trovo nessuna bacheca qui attorno.\r\n" );
				return;
			}

			if ( !can_board_action(ch, board, ACTION_READ) )
			{
				send_to_char( ch, "Non riesco a leggere nulla di questa bacheca, è in un linguaggio criptico..\r\n" );
				return;
			}
		}

		if ( !is_number(argument) )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Devi passare il numero della nota da cui iniziare la lista.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Devo passare il numero della lettera da cui iniziare la lista.\r\n" );
			else
				send_to_char( ch, "Devo passare il numero del messaggio in bacheca da cui iniziare la lista.\r\n" );
			return;
		}

		num_first = atoi( argument );
		if ( num_first < 1 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non puoi inviare numeri negativi per la scelta della nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non posso scegliere di leggere una lettera numerata negativamente.\r\n" );
			else
				send_to_char( ch, "Non vedo messaggi numerati negativamente in questa bacheca.\r\n" );
			return;
		}

		set_char_color( AT_NOTE, ch );

		if ( note_type == NOTE_NOTE )
		{
			send_to_char( ch , "Ecco una lista delle tue note personali:" );
			send_to_pager( ch, "N.  Mittente      Data        Soggetto\r\n" );
			pnote = first_note;
		}
		else if ( note_type == NOTE_MAIL )
		{
			send_to_char( ch , "Faccio una lista delle mie lettere personali:" );
			send_to_pager( ch, "N.  Mittente      Data        Soggetto\r\n" );
			pnote = first_note;
		}
		else
		{
			ch_printf( ch , "Faccio una lista delle lettere pubbliche affisse alla %s:", board->board_obj->short_descr );
			send_to_pager( ch, "N.  Mittente      Data        Soggetto                                   Voto\r\n" );
			pnote = board->first_mail;
		}

		count = 0;
		for ( ;  pnote;  pnote = pnote->next )
		{
			if ( note_type == NOTE_NOTE	 && pnote->type != NOTE_NOTE )	continue;
			if ( note_type == NOTE_MAIL	 && pnote->type != NOTE_MAIL )	continue;
			if ( note_type == NOTE_BOARD && pnote->type != NOTE_BOARD )	continue;

			count++;
			if ( num_first != 0 && count < num_first )
				continue;

			pager_printf( ch, "&W%2d&w] &W%-12.12s&w | %-10s | &W%-44.44s&w",
				count,
				pnote->sender,
				pnote->date,
				pnote->subject );

			/* Visualizza il campo delle votazioni nelle bacheche */
			if ( note_type == NOTE_BOARD && pnote->voting != VOTE_NONE )
			{
				/* Per contare i voti conta gli spazi che dividono i nomi di chi ha votato e ci aggiunge un uno */
				pager_printf( ch, " | &%c%2d&w/&%c%2d&w/&%c%2d&w | ",
					(pnote->voting == VOTE_OPEN) ? 'G' : 'g',		/* colore per il sì */
					str_count(pnote->votes_yes, " ") + 1,
					(pnote->voting == VOTE_OPEN) ? 'R' : 'r',		/* colore per il no */
					str_count(pnote->votes_no, " ") + 1,
					(pnote->voting == VOTE_OPEN) ? 'W' : 'w',		/* colore per gli astenuti */
					str_count(pnote->abstentions, " ") + 1 );
			}
			send_to_pager( ch, "\r\n" );
		}

		if ( count == 0 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non hai nessuna nota privata.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non ho trovato nessuna lettera privata.\r\n" );
			else
				send_to_char( ch, "Non ho trovato nessun messaggio nella bacheca" );
			return;
		}

		if ( note_type == NOTE_BOARD )
			act( AT_ACTION, "$n dà una occhiata alla bacheca.", ch, NULL, NULL, TO_ROOM );
		else if ( note_type == NOTE_MAIL )
			act( AT_ACTION, "$n rovista tra delle lettere.", ch, NULL, NULL, TO_ROOM );

		return;
	} /* chiude l'if */


/* punto 2 (note leggi) */
	if ( is_name(arg, "leggi read") )
	{
		int		choised;
		int		count;
		bool	fAll;

		if ( note_type == NOTE_BOARD )
		{
			board = find_board( ch );
			if ( !board )
			{
				send_to_char( ch, "Non trovo nessuna bacheca qui attorno.\r\n" );
				return;
			}

			if ( !can_board_action(ch, board, ACTION_READ) )
			{
				send_to_char( ch, "Non riesco a leggere nulla di questa bacheca, è in un linguaggio criptico..\r\n" );
				return;
			}
		}

		if ( !str_cmp_all(argument, FALSE) )
		{
			fAll = TRUE;
			choised = 0;
		}
		else if ( is_number(argument) )
		{
			fAll = FALSE;
			choised = atoi( argument );
		}
		else
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Quale numero di nota privata vuoi leggere?\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Quale numero di lettera privata dovrei leggere?\r\n" );
			else
				send_to_char( ch, "Quale numero di messaggio pubblico dovrei leggere?\r\n" );
			return;
		}

		set_char_color( AT_NOTE, ch );

		if ( note_type == NOTE_NOTE )
		{
			send_to_char( ch , "Leggi le tue note personali:" );
			pnote = first_note;
		}
		else if ( note_type == NOTE_MAIL )
		{
			send_to_char( ch , "Leggo le mie lettere personali:" );
			pnote = first_note;
		}
		else
		{
			ch_printf( ch , "Leggo le lettere pubbliche nella %s:", board->board_obj->short_descr );
			pnote = board->first_mail;
		}

		count = 0;
		for ( ;  pnote;  pnote = pnote->next )
		{
			if ( note_type == NOTE_NOTE	 && pnote->type != NOTE_NOTE )	continue;
			if ( note_type == NOTE_MAIL	 && pnote->type != NOTE_MAIL )	continue;
			if ( note_type == NOTE_BOARD && pnote->type != NOTE_BOARD )	continue;

			count++;
			if ( !fAll && count != choised )
				continue;

			if ( note_type == NOTE_NOTE )
				pager_printf( ch, "\r\n&w[Nota numero &W%d&w]\r\n", count );
			else if ( note_type == NOTE_MAIL )
				pager_printf( ch, "\r\n&w[Lettera numero &W%d&w]\r\n", count );
			else
				pager_printf( ch, "\r\n&w[Messaggio numero &W%d&w]\r\n", count );
			pager_printf( ch, "&wA:        &W%s\r\n", pnote->to_list );
			pager_printf( ch, "&wDa:       &W%s\r\n", pnote->sender );
			pager_printf( ch, "&wData:     &W%s\r\n", pnote->date );
			pager_printf( ch, "&wSoggetto: &W%-70.70s&w\r\n", pnote->subject );
			send_to_pager( ch, pnote->text );
			if ( note_type == NOTE_BOARD && pnote->voting != VOTE_NONE )
			{
				bool	view_names = FALSE;

				if ( IS_ADMIN(ch) )
					view_names = TRUE;
				else if ( VALID_STR(board->clan_read) && ch->pg->member
				  && ch->pg->clan && is_name(ch->pg->clan->name, board->clan_read) )
				{
					if ( ch->pg->member->rank == ch->pg->clan->rank_limit-1 )
						view_names = TRUE;
				}

				/* se TRUE permette di visualizzare i nomi di chi ha votato, altrimenti visualizza il numero */
				send_to_pager( ch, "--------------------------------------------------------------------------------\r\n" );
				pager_printf( ch, "Voti (al momento la votazione è %s.):\r\n\r\n", VOTE_OPEN ? "aperta" : "chiusa" );
				pager_printf( ch, "Si:       %d %s\r\n", str_count(pnote->votes_yes, " ")+1, view_names ? pnote->votes_yes : "" );	/* (FF) cercare un sistema di formatazzione a capo oltre la riga 79 per i noti dei votanti */
				pager_printf( ch, "No:       %d %s\r\n", str_count(pnote->votes_no, " ")+1, view_names ? pnote->votes_no : "" );
				pager_printf( ch, "Indecisi: %d %s\r\n", str_count(pnote->abstentions, " ")+1, view_names ? pnote->abstentions : "" );
			}

			if ( note_type == NOTE_BOARD )
				act( AT_ACTION, "$n legge una lettera dalla bacheca.", ch, NULL, NULL, TO_ROOM );
			else if ( note_type == NOTE_MAIL )
				act( AT_ACTION, "$n legge una lettera.", ch, NULL, NULL, TO_ROOM );

			return;
		} /* chiude il for */

		if ( note_type == NOTE_NOTE )
			ch_printf( ch, "Non trovi nessuna nota numerata come %d.\r\n", choised );
		else if ( note_type == NOTE_MAIL )
			ch_printf( ch, "Non trovo nessuna lettera privata numerata come %d.\r\n", choised );
		else
			ch_printf( ch, "Non trovo nessun messaggio pubblico numerato come %d", choised );
		return;
	} /* chiude l'if */


/* Punto 3 (nota a)*/
	if ( is_name(arg, "a to") )
	{
		CHAR_DATA	*addressee;

		if ( note_type == NOTE_BOARD )
		{
			send_to_char( ch, "Non ho bisogno di impostare il destinatario per i messaggi pubblici in bacheca.\r\n" );
			return;
		}

		if ( !IS_ADMIN(ch) && note_type != NOTE_NOTE )
		{
			quill = find_quill( ch );
			if ( !quill )
			{
				send_to_char( ch, "Ho bisogno di una penna per scrivere una lettera.\r\n" );
				return;
			}
			if ( quill->pen->ink < 1 )
			{
				send_to_char( ch, "La mia penna è ormai secca.\r\n" );
				return;
			}
		}

		if ( !VALID_STR(argument) )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Devi specificare a chi vuoi inviare la nota.\r\n" );
			else
				send_to_char( ch, "Devo specificare a chi voglio inviare la lettera.\r\n" );
			return;
		}

		paper = give_parchement( ch, note_type );
		if ( !paper )
			return;

		if ( paper->paper->to > 1 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non puoi modificare questa nota.\r\n" );
			else
				send_to_char( ch, "Non riesco a cancellare il destinatario di questa lettera.\r\n" );
			return;
		}

		addressee = get_offline( argument, TRUE );
		if ( !addressee )
		{
			ch_printf( ch, "Non conosc%c nessun %s.\r\n",
				note_type == NOTE_NOTE ? 'i' : 'o', capitalize(argument) );	/* (FF) questo è uno dei punti in cui, se si inserisce lo snippet dell'intro, potrebbe fare acqua */
			return;
		}

		paper->paper->to = 1;
		ed = set_obj_extra( paper, "_to_" );
		DISPOSE( ed->description );
		ed->description = str_dup( argument );
		if ( note_type == NOTE_NOTE )
			send_to_char( ch, "Ok.\r\n" );
		else
			send_to_char( ch, "Ecco fatto, ho scritto il destinatario.\r\n" );
		return;
	}


/* Punto 4 (nota soggetto) */
	if ( is_name(arg, "soggetto subject") )
	{
		if ( !IS_ADMIN(ch) && note_type != NOTE_NOTE )
		{
			quill = find_quill( ch );
			if ( !quill )
			{
				send_to_char( ch, "Ho bisogno di una penna per scrivere una lettera.\r\n" );
				return;
			}
			if ( quill->pen->ink < 1 )
			{
				send_to_char( ch, "La mia penna è ormai secca.\r\n" );
				return;
			}
		}

		if ( !VALID_STR(argument) )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Devi specificare il soggetto della tua nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Devo specificare il soggetto della mia lettera.\r\n" );
			else
				send_to_char( ch, "Devo specificare il soggetto della mio messaggio pubblico.\r\n" );
			return;
		}

		paper = give_parchement( ch, note_type );
		if ( !paper )
			return;

		if ( paper->paper->subject > 1 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non puoi modificare il soggetto di questa nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non riesco a cancellare il soggetto di questa lettera.\r\n" );
			else
				send_to_char( ch, "Non riesco a cancellare il soggetto da questo messaggio pubblico.\r\n" );
			return;
		}

		paper->paper->subject = 1;
		ed = set_obj_extra( paper, "_subject_" );
		DISPOSE( ed->description );
		ed->description = str_dup( argument );

		if ( note_type == NOTE_NOTE )
			send_to_char( ch, "Ok.\r\n" );
		else
			send_to_char( ch, "Ecco fatto, ho scritto il soggetto.\r\n" );

		return;
	} /* chiude l'if */


/* Punto 5 (nota scrivi )*/
	if ( is_name(arg, "scrivi write") )
	{
		if ( ch->substate == SUB_RESTRICTED )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non puoi scrivere il testo di una nota mentre stai scrivendo dell'altro.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non posso scrivere il testo di una lettera mentre sto scrivendo dell'altro.\r\n" );
			else
				send_to_char( ch, "Non posso scrivere il testo di un messaggio pubblico mentre sto scrivendo dell'altro.\r\n" );
			return;
		}

		if ( !IS_ADMIN(ch) || note_type != NOTE_NOTE )
		{
			quill = find_quill( ch );
			if ( !quill )
			{
				send_to_char( ch, "Ho bisogno di una penna per scrivere una lettera.\r\n" );
				return;
			}
			if ( quill->pen->ink < 1 )
			{
				send_to_char( ch, "La mia penna è ormai secca.\r\n" );
				return;
			}
		}

		paper = give_parchement( ch, note_type );
		if ( !paper )
			return;

		if ( paper->paper->text > 1 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non puoi modificare il testo di questa nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non riesco a cancellare il testo di questa lettera.\r\n" );
			else
				send_to_char( ch, "Non riesco a cancellare il testo di questo messaggio pubblico.\r\n" );
			return;
		}

		paper->paper->text = 1;
		ed = set_obj_extra( paper, "_text_" );
		ch->substate = SUB_WRITING_NOTE;
		ch->dest_buf = ed;
		if ( !IS_ADMIN(ch) && note_type != NOTE_NOTE )
			quill->pen->ink--;
		start_editing( ch, ed->description );
		return;
	} /* chiude l'if */


/* Punto 6 (nota mostra) */
	if ( is_name(arg, "mostra show") )
	{
		char	*subject;
		char	*to_list;
		char	*text;

		paper = get_eq_char( ch, WEARLOC_HOLD );	/* (FF) anche qui fare quello che si vuole fare al give_parchement */
		if ( !paper || !paper->paper )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non hai in mano nessuna nota.\r\n" );
			else
				send_to_char( ch, "Non sto impugnango nessun pezzo di pergamena.\r\n" );
			return;
		}

		subject = get_extradescr( "_subject_", paper->first_extradescr );
		if ( !VALID_STR(subject) )
			subject = "(Vuoto)";
		to_list = get_extradescr( "_to_", paper->first_extradescr );
		if ( VALID_STR(to_list) )
			to_list = "(Vuoto)";
		text = get_extradescr( "_text_", paper->first_extradescr );
		if ( !VALID_STR(text) )
			text = "(Vuoto)";

		if ( note_type == NOTE_NOTE )
			send_to_char( ch, "[Nota numero &W1&w]\r\n" );
		else if ( note_type == NOTE_MAIL )
			send_to_char( ch, "[Lettera numero &W1&w]\r\n" );
		else
			send_to_char( ch, "[Messaggio numero &W1&w]\r\n" );
		ch_printf( ch, "&wA:        &W%s\r\n",			to_list );
		ch_printf( ch, "&wDa:       &W%s\r\n",			ch->name );
		ch_printf( ch, "&wData:     &W(Qui vi sarà la data di quando invi%c)\r\n", note_type == NOTE_NOTE ? 'i' : 'o' );
		ch_printf( ch, "&wSoggetto: &W%-70.70s\r\n",	subject );
		ch_printf( ch, "&w%s\r\n",						text );

		return;
	} /* chiude l'if */


/* Punto 7 (nota invia) */
	if ( is_name(arg, "invia post") )
	{
		char	*to;
		char	*subj;
		char	*text;
		char	*strtime;

		paper = get_eq_char( ch, WEARLOC_HOLD );	/* (FF) idem come sopra */
		if ( !paper || !paper->paper )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non hai in mano nessuna nota.\r\n" );
			else
				send_to_char( ch, "Non sto impugnango nessun pezzo di pergamena.\r\n" );
			return;
		}

		if ( paper->paper->to == 0 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non hai ancora impostato il destinatario della nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non ho ancora impostato il destinatario della lettera.\r\n" );
			else
				send_to_char( ch, "Non ho ancora impostato il destinatario del messaggio pubblico.\r\n" );
			return;
		}

		if ( paper->paper->subject == 0 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non hai ancora inserito il soggetto della nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Devo ancora inserire il soggetto della lettera.\r\n" );
			else
				send_to_char( ch, "Devo ancora inserire il soggetto del messaggio pubblico.\r\n" );
			return;
		}

		if ( paper->paper->text == 0 )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non hai ancora iniziato a scrivere il testo della nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Devo ancora iniziare a scrivere il testo della lettera.\r\n" );
			else
				send_to_char( ch, "Devo ancora iniziare a scrivere il testo del messaggio pubblico.\r\n" );
			return;
		}


		/* Di default il destinatario nelle board è settato ad all perchè pubblico */
		if ( note_type == NOTE_BOARD )
		{
			ed = set_obj_extra( paper, "_to_" );
			DISPOSE( ed->description );
			ed->description = str_dup( "all" );
			to = "all";
		}
		else
		{
			to = get_extradescr( "_to_", paper->first_extradescr );
			if ( !VALID_STR(to) )
			{
				if ( note_type == NOTE_NOTE )
					send_to_char( ch, "Manca il destinatario della nota.\r\n" );
				else if ( note_type == NOTE_MAIL )
					send_to_char( ch, "Non ho ancora scritto il destinatario della lettera.\r\n" );
				return;
			}
		}

		subj = get_extradescr( "_subject_",	paper->first_extradescr );
		if ( !VALID_STR(subj) )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Manca il soggetto della nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non ho ancora scritto della lettera.\r\n" );
			else
				send_to_char( ch, "Non ho ancora scritto del messaggio pubblico.\r\n" );
			return;
		}

		text = get_extradescr( "_text_", paper->first_extradescr );
		if ( !VALID_STR(text) )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Manca il testo della nota.\r\n" );
			else if ( note_type == NOTE_MAIL )
				send_to_char( ch, "Non ho ancora scritto il testo della lettera.\r\n" );
			else
				send_to_char( ch, "Non ho ancora scritto il testo del messaggio pubblico.\r\n" );
			return;
		}

		strtime = friendly_ctime_nohour( &current_time );

		if ( note_type == NOTE_BOARD )
		{
			board = find_board( ch );
			if ( !board )
			{
				send_to_char( ch, "Non trovo nessuna bacheca in cui inserire il mio messaggio.\r\n" );
				return;
			}

			if ( !can_board_action(ch, board, ACTION_POST) )
			{
				send_to_char( ch, "Non posso inserire messaggi pubblici in questa bacheca.\r\n" );
				return;
			}

			if ( board->num_posts >= board->max_posts )
			{
				send_to_char( ch, "Non trovo spazio per inserire il mio messaggio in questa bacheca.\r\n" );
				return;
			}
		}

		CREATE( pnote, NOTE_DATA, 1 );
		pnote->to_list		= str_dup( to );
		pnote->text			= str_dup( text );
		pnote->subject		= str_dup( subj );
		pnote->date			= str_dup( strtime );
		pnote->sender		= str_dup( ch->name );
		pnote->type			= paper->paper->type;
		pnote->voting		= VOTE_NONE;
		pnote->votes_yes	= str_dup( "" );
		pnote->votes_no		= str_dup( "" );
		pnote->abstentions	= str_dup( "" );

		if ( note_type == NOTE_BOARD )
		{
			LINK( pnote, board->first_mail, board->last_mail, next, prev );
			board->num_posts++;
			fwrite_notes_board( board );
			ch_printf( ch, "Inserisco la mia lettera pubblica nella %s.\r\n", board->board_obj->short_descr );
			act( AT_ACTION, "$n inserisce un messaggio nella bacheca.", ch, NULL, NULL, TO_ROOM );
		}
		else
		{
			LINK( pnote, first_note, last_note, next, prev );
			top_note++;
			fwrite_notes_player( ch );
			if ( note_type == NOTE_NOTE )
			{
				send_to_char( ch, "Invii la tua nota.\r\n" );
			}
			else
			{
				send_to_char( ch, "Invio la mia lettera tramite piccione viaggiatore.\r\n" );
				act( AT_ACTION, "$n invia una lettera tramite piccione viaggiatore.", ch, NULL, NULL, TO_ROOM );
			}
		}

		free_object( paper );
		return;
	}


/* Punto 8-9-10 (nota rimuovi, prendi, copia) */
	if ( is_name(arg, "rimuovi remove")
	  || is_name(arg, "prendi take")
	  || is_name(arg, "copia copy") )
	{
		char   *verb = NULL;	/* nome del verbo relativo all'azione */
		int		take = -1;		/* remove = 0, take = 1, copy = 2 */
		int		choised;		/* numero della nota scelta */
		int		count;

		if ( note_type != NOTE_BOARD )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non puoi eseguire questa azione su di una nota privata.\r\n" );
			else
				send_to_char( ch, "Non posso eseguire questa azione su di una lettera privata.\r\n" );
		}

		board = find_board( ch );
		if ( !board )
		{
			send_to_char( ch, "Non trovo nessuna bacheca con cui interagire.\r\n" );
			return;
		}

		if ( is_name(arg, "rimuovi togli cancella remove delete") )
		{
			take = 0;
			verb = "rimuovere";
		}
		else if ( is_name(arg, "prendi raccogli get take") )
		{
			take = 1;
			verb = "prendere";
		}
		else if ( is_name(arg, "copia copy") && IS_ADMIN(ch) )
		{
			take = 2;
			verb = "copiare";
		}

		if ( !is_number(argument) )
		{
			ch_printf( ch, "Quale lettera numerata dovrei %s?\r\n", verb );
			return;
		}

		if ( !can_board_action(ch, board, ACTION_READ) )
		{
			send_to_char( ch, "Non ha senso che rimuova dei messaggi da una bacheca se non riesco a leggerli.\r\n" );
			return;
		}

		if ( can_board_action(ch, board, ACTION_REMOVE) )
		{
			send_to_char( ch, "Non ho il diritto di rimuovere messaggi da questa bacheca.\r\n" );
			return;
		}

		choised = atoi( argument );
		count = 0;
		for ( pnote = board->first_mail;  pnote;  pnote = pnote->next )
		{
			count++;

			if ( count != choised )
				continue;

			if ( take != 0 )	/* se non è rimuovere */
			{
				char	short_descr_buf[MSL];
				char	long_descr_buf[MSL];
				char	keyword_buf[MSL];

				if ( !IS_ADMIN(ch) )
				{
					if ( ch->gold < 50 )
					{
						ch_printf( ch, "Mi costerà 50 monete d'oro %s il messaggio.\r\n",
							(take == 1) ? "prendere" : "copiare" );
						return;
					}

					ch->gold -= 50;
				}

				paper = make_object( get_obj_proto(NULL, VNUM_OBJ_NOTE), 0 );
				paper->paper->text		= 2;
				paper->paper->subject	= 2;
				paper->paper->to		= 2;

				ed = set_obj_extra( paper, "_sender_" );
				DISPOSE( ed->description );
				ed->description = str_dup( pnote->sender );

				ed = set_obj_extra( paper, "_text_" );
				DISPOSE( ed->description );
				ed->description = str_dup(pnote->text);

				ed = set_obj_extra( paper, "_to_" );
				DISPOSE( ed->description );
				ed->description = str_dup( pnote->to_list );

				ed = set_obj_extra( paper, "_subject_" );
				DISPOSE( ed->description );
				ed->description = str_dup( pnote->subject );

				ed = set_obj_extra( paper, "_date_" );
				DISPOSE( ed->description );
				ed->description = str_dup( pnote->date );

				strcpy( short_descr_buf, "una lettera pubblica" );
				DISPOSE( paper->short_descr );
				paper->short_descr = str_dup( short_descr_buf );

				strcpy( long_descr_buf, "Una lettera pubblica si trova per terra." );
				DISPOSE( paper->long_descr );
				paper->long_descr = str_dup( long_descr_buf );

				strcpy( keyword_buf, "lettera pergamena carta" );
				DISPOSE( paper->name );
				paper->name = str_dup( keyword_buf );
			} /* chiude l'if */

			/* Se l'azione non  una copia la rimuove dalla bacheca */
			if ( take != 2 )
				remove_note( board, pnote );

			send_to_char( ch, "Ecco fatto.\r\n" );

			if ( take == 1 )
			{
				act( AT_ACTION, "$n prende un messaggio dalla bacheca.", ch, NULL, NULL, TO_ROOM );
				obj_to_char( paper, ch );
			}
			else if ( take == 2 )
			{
				act( AT_ACTION, "$n copia un messaggio della bacheca.", ch, NULL, NULL, TO_ROOM );
				obj_to_char( paper, ch );
			}
			else
			{
				act( AT_ACTION, "$n rimuove un messaggio della bacheca.", ch, NULL, NULL, TO_ROOM );
			}

			return;
		} /* chiude il for */

		send_to_char( ch, "Non trovo questo messaggio.\r\n" );
		return;
	} /* chiude l'if */


/* Punto 12 (nota vota) */
	if ( is_name(arg, "vota vote") )
	{
		char	arg2[MIL];
		int		choised;
		int		count;

		if ( note_type != NOTE_BOARD )
		{
			if ( note_type == NOTE_NOTE )
				send_to_char( ch, "Non puoi eseguire questa azione su di una nota privata.\r\n" );
			else
				send_to_char( ch, "Non posso eseguire questa azione su di una lettera privata.\r\n" );
		}

		board = find_board( ch );
		if ( !board )
		{
			send_to_char( ch, "Non trovo nessuna bacheca con cui interagire.\r\n" );
			return;
		}

		if ( !can_board_action(ch, board, ACTION_READ) )
		{
			send_to_char( ch, "Non ha senso che rimuova dei messaggi da una bacheca se non riesco a leggerli.\r\n" );
			return;
		}

		argument = one_argument( argument, arg2 );
		if ( !is_number(arg2) )
		{
			send_to_char( ch, "Note vote which number?\r\n" );
			return;
		}

		choised = atoi( arg2 );
		count = 1;
		for ( pnote = board->first_mail;  pnote;  pnote = pnote->next )
		{
			count++;
			if ( count == choised )
				break;
		}

		if ( !pnote )
		{
			send_to_char( ch, "Non trovo il numero di questa lettera nella bacheca.\r\n" );
			return;
		}

		/* Se ch è l'autore della lettera puoi aprire e chiudere una votazione */
		/* Se ch può legger la bacheca e la votazione aperta può votarci */

		/* Opzioni: apri, chiudi, si, no, indeciso */
		if ( is_name(argument, "apri open") )
		{
			if ( str_cmp(ch->name, pnote->sender) )
			{
				send_to_char( ch, "Non sono l'autore di questa lettera.\r\n" );
				return;
			}
			pnote->voting = VOTE_OPEN;
			act( AT_ACTION, "$n apre una votazione in una lettera pubblica.", ch, NULL, NULL, TO_ROOM );
			send_to_char( ch, "Ho aperto la votazione.\r\n" );
			fwrite_notes_board( board );
			return;
		}

		if ( is_name(argument, "chiudi close") )
		{
			if ( str_cmp(ch->name, pnote->sender) )
			{
				send_to_char( ch, "Non sono l'autore di questa lettera.\r\n" );
				return;
			}
			pnote->voting = VOTE_CLOSED;
			act( AT_ACTION, "$n chiude una votazione su una lettera pubblica.", ch, NULL, NULL, TO_ROOM );
			send_to_char( ch, "Ho chiuso la votazione.\r\n" );
			fwrite_notes_board( board );
			return;
		}

		/* Controlla se la nota è con votazione aperta per poter continuare */
		if ( pnote->voting != VOTE_OPEN )
		{
			send_to_char( ch, "La votazione non è aperta per questa lettera.\r\n" );
			return;
		}

		/* Si può votare solo una volta */
		if ( is_name(ch->name, pnote->votes_yes)
		  || is_name(ch->name, pnote->votes_no)
		  || is_name(ch->name, pnote->abstentions) )
		{
			send_to_char( ch, "Ho già votato in questa lettera.\r\n" );
			return;
		}

		if ( is_name(argument, "si yes") )
		{
			sprintf( buf, "%s %s", pnote->votes_yes, ch->name );
			DISPOSE( pnote->votes_yes );
			pnote->votes_yes = str_dup( buf );
		}
		else if ( !str_cmp(argument, "no") )
		{
			sprintf( buf, "%s %s", pnote->votes_no, ch->name );
			DISPOSE( pnote->votes_no );
			pnote->votes_no = str_dup( buf );
		}
		else if ( !str_cmp(argument, "indecisi") || !str_cmp(argument, "undecided") )
		{
			sprintf( buf, "%s %s", pnote->abstentions, ch->name );
			DISPOSE( pnote->abstentions );
			pnote->abstentions = str_dup( buf );
		}
		else
		{
			send_to_char( ch, "Devo votare con: si, no o indeciso.\r\n" );
			return;
		}

		act( AT_ACTION, "$n vota ad un messaggio di bacheca.", ch, NULL, NULL, TO_ROOM );
		send_to_char( ch, "Ok, ho votato.\r\n" );

		fwrite_notes_board( board );
		return;
	} /* chiude l'if */

	if		( note_type == NOTE_NOTE )	send_command( ch, "note", CO );
	else if	( note_type == NOTE_MAIL )	send_command( ch, "mail", CO );
	else								send_command( ch, "board", CO );

	ch_printf( ch, "Digita '%s nota' per maggiori informazioni.\r\n", translate_command(ch, "help") );
}

/* Posta una nota offgdr a qualcuno */
DO_RET do_note( CHAR_DATA *ch, char *argument )
{
	note_handler( ch, argument, NOTE_NOTE );
}

/* Posta una mail a qualcuno */
DO_RET do_mail( CHAR_DATA *ch, char *argument )
{
	note_handler( ch, argument, NOTE_MAIL );
}

/* Posta una mail in una board */
DO_RET do_board( CHAR_DATA *ch, char *argument )
{
	note_handler( ch, argument, NOTE_BOARD );
}


/*
 * Comando admin per visualizzare le stat di una board
 */
DO_RET do_showboards( CHAR_DATA *ch, char *argument )
{
	BOARD_DATA *board;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_bstat: ch è NULL" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w:  showboards\r\n" );
		send_to_char( ch, "&YSintassi&w:  showboards <board filename>\r\n\r\n" );

		if ( !first_board )
		{
			ch_printf( ch, "&GThere are no boards yet.\r\n" );
			return;
		}

		for ( board = first_board;  board;  board = board->next )
		{
			pager_printf( ch, "&G%-15.15s  #%5d  Read: %2d  Post: %2d  Remove: %2d  Posts: &g%3d/%3d\r\n",
				board->mail_file,
				board->board_obj->vnum,
				board->min_trust_read,
				board->min_trust_post,
				board->min_trust_remove,
				board->num_posts,
				board->max_posts );
		}
		return;
	}

	for ( board = first_board;  board;  board = board->next )
	{
		if ( !str_cmp(argument, board->mail_file) )
			break;
	}

	if ( !board )
	{
		ch_printf( ch, "Non è stata trovata nessuna board con argomento %s.\r\n", argument );
		return;
	}

	ch_printf( ch, "&GFilename:      &W%s\r\n",	board->mail_file );
	ch_printf( ch, "&GOVnum:         &W%d\r\n",	board->board_obj->vnum );
	ch_printf( ch, "&GClanRead:	     &W%s\r\n",	board->clan_read );
	ch_printf( ch, "&GClanPost:	     &W%s\r\n",	board->clan_post );
	ch_printf( ch, "&GClanRemove:	 &W%s\r\n",	board->clan_remove );
	ch_printf( ch, "&GExtraReaders:  &W%s\r\n",	board->extra_readers );
	ch_printf( ch, "&GExtraPosters:  &W%s\r\n",	board->extra_posters );
	ch_printf( ch, "&GExtraRemovers: &W%s\r\n",	board->extra_removers );
	ch_printf( ch, "&GRead:          &W%s\r\n",	code_name(NULL, board->min_trust_read, CODE_TRUST) );
	ch_printf( ch, "&GPost:          &W%s\r\n",	code_name(NULL, board->min_trust_post, CODE_TRUST) );
	ch_printf( ch, "&GRemove:        &W%s\r\n",	code_name(NULL, board->min_trust_remove, CODE_TRUST) );
	ch_printf( ch, "&GPosts:	     &W%d\r\n",	board->num_posts );
	ch_printf( ch, "&GMaxpost:       &W%d\r\n",	board->max_posts );
}


OBJ_DATA *recursive_note_find( OBJ_DATA *obj, char *argument )
{
	OBJ_DATA *returned_obj;
	char	 *argcopy;
	char	 *subject;
	char	  arg[MIL];
	char	  subj[MSL];
	bool	  match = TRUE;

	if ( !obj )
		return NULL;

	if ( obj->paper )
	{
		subject = get_extradescr( "_subject_", obj->first_extradescr );
		if ( VALID_STR(subject) )
		{
			strcpy( subj, strlower(subject) );
			subject = strlower( subj );

			argcopy = argument;

			while ( match )
			{
				argcopy = one_argument( argcopy, arg );

				if ( !VALID_STR(arg) )
					break;

				if ( !strstr(subject, arg) )
					match = FALSE;
			}

			if ( match )
				return obj;
		}
	}
	else if ( obj->container )
	{
		if ( obj->first_content )
		{
			returned_obj = recursive_note_find( obj->first_content, argument );
			if ( returned_obj )
				return returned_obj;
		}
	}

	return recursive_note_find( obj->next_content, argument );
}


/*
 * Reworked recursive_note_find to fix crash bug when the note was left blank
 */
DO_RET findnote_handler( CHAR_DATA *ch, char *argument, const int type )
{
	OBJ_DATA* obj;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_findnote: ch è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devi specificare almeno una parola chiave.\r\n" );
		return;
	}

	obj = recursive_note_find( ch->first_carrying, argument );
	if ( obj )
	{
		if ( obj->in_obj )
		{
			obj_from_obj( obj );
			obj = obj_to_char( obj, ch );
		}
		wear_obj( ch, obj, TRUE, -1, "impugnare" );
	}
	else
	{
		send_to_char( ch, "Nota non trovata.\r\n" );
	}
}

DO_RET do_findnote( CHAR_DATA *ch, char *argument )
{
	findnote_handler( ch, argument, NOTE_NOTE );
}

DO_RET do_findmail( CHAR_DATA *ch, char *argument )
{
	findnote_handler( ch, argument, NOTE_MAIL );
}


/*
 * Conta quante mail-note-msg di board ha un pg con la data passata
 */
void note_count( CHAR_DATA *ch, const char *date, const int note_type )
{
	NOTE_DATA	*note;
	int			 count;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "mail_count: ch è NULL" );
		return;
	}

	if ( !VALID_STR(date) )
	{
		send_log( NULL, LOG_BUG, "note_count: data passata non valida" );
		return;
	}

	if ( note_type <= NOTE_NONE || note_type > NOTE_BOARD )
	{
		send_log( NULL, LOG_BUG, "note_count: tipo di nota passata errata: %d", note_type );
		return;
	}

	if ( note_type == NOTE_BOARD )
	{
		BOARD_DATA *board;

		for ( board = first_board;  board;  board = board->next )
		{
			count = 0;
			for ( note = board->first_mail;  note;  note = note->next )
			{
				if ( !str_cmp(note->date, date) )
					count++;
			}

			if ( count > 0 )
			{
				ch_printf( ch, "Oggi sono stati affissi %s %d messaggi pubblici.\r\n",
					board->board_obj->short_descr, count );
			}
		}
	}
	else
	{
		count = 0;
		for ( note = first_note;  note;  note = note->next )
		{
			if ( note_type == NOTE_NOTE && note->type != NOTE_NOTE )	continue;
			if ( note_type == NOTE_MAIL && note->type != NOTE_MAIL )	continue;
			if ( !is_name(ch->name, note->to_list) )					continue;

			if ( !str_cmp(note->date, date) )
				count++;
		}

		if ( count > 0 )
		{
			if ( note_type == NOTE_NOTE )
				ch_printf( ch, "Oggi hai ricevuto %d not%s.\r\n", count, (count > 1) ? "e private" : "a privata" );
			else
				ch_printf( ch, "Oggi hai ricevuto %d letter%s.\r\n", count, (count > 1)  ?  "e private"  :  "a privata" );
		}
	}
}
