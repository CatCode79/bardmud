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

/****************************************************************************************\
 >					Modulo delle domande della creazione del giocatore					<
\****************************************************************************************/

#ifdef T2_QUESTION

#include "mud.h"
#include "class.h"
#include "fread.h"
#include "nanny.h"
#include "question.h"


/*
 * Definizioni di file
 */
#define QUESTION_FILE	TABLES_DIR "questions.dat"		/* File delle domande creazione pg */


/*
 * Definizioni
 */
#define MAX_QUESTION			10		/* N. max di domande caricabili */
#define MAX_QUESTION_TO_SEND	 5		/* N. max di domande chieste alla creazione */


/*
 * Struttura di una domanda
 */
typedef struct question_data	QUESTION_DATA;

struct question_data
{
	QUESTION_DATA	*next;
	QUESTION_DATA	*prev;
	char			*text;
	int				 answer[MAX_CLASS];
};


/*
 * Struttura di una gestione di invio e salvataggio risposte alle domande
 */
typedef struct question_handler_data	QHANDLER_DATA;

struct question_handler_data
{
	QHANDLER_DATA	*next;
	QHANDLER_DATA	*prev;
	DESCRIPTOR_DATA	*d;
	QUESTION_DATA	*sended[MAX_QUESTION_TO_SEND];	/* Salva quali domande ha inviato */
	int				 sended_count;					/* Numero di domande inviate al descrittore */
	int				 choised[MAX_CLASS];			/* Salva la risposta con le classi scelte */
};


/*
 * Variabili
 */
static	QUESTION_DATA	*first_question	= NULL;
static	QUESTION_DATA	*last_question	= NULL;
static	QHANDLER_DATA	*first_qhandler = NULL;
static	QHANDLER_DATA	*last_qhandler	= NULL;
		int				 top_question	= 0;


/*
 * Funzioni di caricamento domande
 * Legge una domanda e la salva nella lista delle question.
 */
static void fread_question( MUD_FILE *fp )
{
	QUESTION_DATA *question;
	int			   x;

	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fread_question: fp passato è NULL" );
		return;
	}

	CREATE( question, QUESTION_DATA, 1 );
	for ( x = 0;  x < top_class_play;  x++ )
		question->answer[x] = -1;

	for ( ; ; )
	{
		char	*word;

		word = fread_word( fp );

		if ( feof(fp->file) )
		{
			send_log( NULL, LOG_FREAD, "fread_question: fine prematura del file" );
			exit( EXIT_FAILURE );
		}

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if ( !str_cmp(word, "Answer") )
		{
			x = fread_positive( fp );
			x--;
			if ( x < 0 || x >= top_class_play )
			{
				send_log( fp, LOG_FREAD, "fread_question: numero di risposta acquisita: %d", x );
				exit( EXIT_FAILURE );
			}
			question->answer[x] =										code_code( fp, fread_word(fp), CODE_CLASS );
		}
		else if ( !str_cmp(word, "End"	 ) )	break;
		else if ( !str_cmp(word, "Text"	 ) )	question->text =		fread_string( fp );
		else
			send_log( fp, LOG_FREAD, "fread_question: nessuna etichetta per la parola %s", word );
	} /* chiude il for */

	if ( !VALID_STR(question->text) )
	{
		send_log( fp, LOG_FREAD, "fread_question: la domanda %d è senza testo.", top_question );
		exit( EXIT_FAILURE );
	}

	for ( x = 0;  x < top_class_play;  x++ )
	{
		if ( question->answer[x] == -1 )
		{
			send_log( fp, LOG_FREAD, "fread_question: classe non acquisita o errata (%d) alla risposta numero %d", question->answer[x], x );
			exit( EXIT_FAILURE );
		}
	}

	LINK( question, first_question, last_question, next, prev );
	top_question++;
}


/*
 * Carica tutte le domande
 */
void load_questions( void )
{
	fread_section( QUESTION_FILE, "QUESTION", fread_question, FALSE );

	if ( top_question >= MAX_QUESTION )
	{
		send_log( NULL, LOG_FREAD, "load_questions: le domande caricate (%d) superano MAX_QUESTION %d",
			top_question, MAX_QUESTION );
		exit( EXIT_FAILURE );
	}
}


/*
 * Ritorna, se esiste, l'handler delle domande corrispondente al pg passato
 */
QHANDLER_DATA *get_question_handler( DESCRIPTOR_DATA *d )
{
	QHANDLER_DATA	*qhandler;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "get_question_handler: d passato è NULL" );
		return NULL;
	}

	for ( qhandler = first_qhandler;  qhandler;  qhandler = qhandler->next )
	{
		if ( qhandler->d == d )
			return qhandler;
	}

	return NULL;
}


/*
 * Libera dalla memoria tutte le domande
 */
void free_all_questions( void )
{
	QUESTION_DATA *question;

	for ( question = first_question;  question;  question = first_question )
	{
		UNLINK( question, first_question, last_question, next, prev );
		top_question--;

		DISPOSE( question->text );
		DISPOSE( question );
	}

	if ( top_question != 0 )
		send_log( NULL, LOG_BUG, "free_all_questions: top_question diverso da 0 dopo aver liberato dalla memoria le domande: %d", top_question );
}

/*
 * Libera dalla memoria una struttura di gestione di domanda di un descrittore particolare
 */
void free_question_handler_desc( DESCRIPTOR_DATA *d )
{
	QHANDLER_DATA *qhandler;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "free_question_handler: qhandler passato è NULL" );
		return;
	}

	qhandler = get_question_handler( d );
	if ( !qhandler )
		return;

	UNLINK( qhandler, first_qhandler, last_qhandler, next, prev );
	DISPOSE( qhandler );
}

/*
 * Libea dalla memoria tutte le strutture di gestione delle domande per la creazione
 */
void free_all_question_handlers( void )
{
	QHANDLER_DATA	*qhandler;

	for ( qhandler = first_qhandler;  qhandler;  qhandler = first_qhandler )
	{
		UNLINK( qhandler, first_qhandler, last_qhandler, next, prev );
		DISPOSE( qhandler );
	}
}


/*
 * Ritorna la sizeof di una struttura di domanda
 */
int get_sizeof_question( void )
{
	return sizeof( QUESTION_DATA );
}


/*
 * Visualizza la domanda sul tipo di creazione, se domande o lista di classi
 */
void show_want_question( DESCRIPTOR_DATA *d )
{
	if ( !d )
	{
		send_log( NULL, LOG_BUG, "show_want_question: d passato è NULL" );
		return;
	}

	write_to_buffer( d, ANSI_CLEAR, 4 );
	send_to_char( d->character, "\r\nVorresti attribuire la classe al tuo personaggio attraverso:"
		"&W1] &wUna lista delle stesse.\r\n"
		"&W2] &wUna serie di domande.\r\n" );
}


/*
 * Stampa a video una domanda
 */
void show_get_question( DESCRIPTOR_DATA *d )
{
	QHANDLER_DATA	*qhandler;
	QUESTION_DATA	*question = NULL;
	int				 qrandom;	/* Numero di domanda casuale */
	int				 x;
	int				 count;
	bool			 found;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "show_get_question: ch passato è NULL" );
		return;
	}

	/* Controlla se tra gli handler delle domande non ne esista già uno per questo pg, altrimenti lo crea */
	qhandler = get_question_handler( d );
	if ( !qhandler )
	{
		CREATE( qhandler, QHANDLER_DATA, 1 );
		qhandler->d = d;
		LINK( qhandler, first_qhandler, last_qhandler, next, prev );
	}
	else
		qhandler->sended_count++;	/* Se esisteva già aumenta il numero di domande inviate */

	write_to_buffer( d, ANSI_CLEAR, 4 );
	ch_printf( d->character,
		"\r\nOra ti verranno fatte alcune domande per capire quali qualita' attribuire"
		"\r\nal tuo personaggio.\r\n" );

	/* Sceglie la domanda */
	found = FALSE;
	while ( 0 )
	{
		count = 0;

		qrandom = number_range( 0, top_question-1 );
		for ( question = first_question;  question;  question = question->next )
		{
			if ( count != qrandom )
			{
				count++;
				continue;
			}

			for ( x = 0;  x < qhandler->sended_count;  x++ )
			{
				if ( qhandler->sended[x] == question )
				{
					found = TRUE;
					break;
				}
			}
			/* Se non ha trovato che la domanda prima sia stata già inviata allora esce dai cicli e invia quella */
			if ( !found )
				break;
		}
		if ( !found )
			break;
	}

	if ( !question )
	{
		send_log( NULL, LOG_BUG, "show_get_question: question non trovata per il pg %s, contatore invio a %d",
			d->character->name, qhandler->sended_count );
		return;
	}

	/* Acquisita la domanda la visualizza */
	ch_printf( d->character, "\r\n  %s\r\nQuale e' la tua risposta? ", question->text );

	/* Salva il numero della domanda inviata */
	qhandler->sended[qhandler->sended_count] = question;

#ifdef DEBUG_QUESTION
	ch_printf( d->character, "\r\nContatore domande: %d\r\n", qhandler->sended_count );
	ch_printf( d->character, "Domanda inviata: %10.10s\r\n", question->text );

	send_to_char( d->character, "Answered =" );
	for ( x = 0;  x < MAX_QUESTION_TO_SEND;  x++ )
		ch_printf( d->character, " %10.10s ", qhandler->sended[x]->text );
	send_to_char( d->character, "\r\n" );

	send_to_char( d->character, "Choised  =" );
	for ( x = 0;  x < MAX_QUESTION_TO_SEND;  x++ )
		ch_printf( d->character, " %s %d ", code_name(NULL, x, CODE_CLASS), qhandler->choised[x] );
	send_to_char( d->character, "\r\n" );
#endif
}


/*
 * Controlla la risposta data alla domanda
 */
bool check_get_question( DESCRIPTOR_DATA *d, char *argument )
{
	QHANDLER_DATA *qhandler;
	int			   answer;
	int			   class;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "check_get_question: d passato è NULL" );
		return FALSE;
	}

	answer = atoi( argument );

	/* Invia il messaggio di errore se non si trova nel range delle risposte valide */
	if ( answer < 1 || answer >= top_class_play )
	{
		ch_printf( d->character, "\r\nNon e' una risposta valida. Scegli una risposta tra &W1&w e &W%d&w: ",
			top_class_play );
		return FALSE;
	}

	qhandler = get_question_handler( d );
	if ( !qhandler )
	{
		send_log( NULL, LOG_BUG, "check_get_question: nessun qhandler trovato per il pg %s", d->character->name );
		return FALSE;
	}

	/* Salva la risposta inserendovi di già il valore della classe relativa */
	class = qhandler->sended[qhandler->sended_count]->answer[answer];
	qhandler->choised[class]++;

	return TRUE;
}


/*
 * Controlla se dover inviare una nuova domanda oppure no
 */
bool get_another_question( DESCRIPTOR_DATA *d )
{
	QHANDLER_DATA *qhandler;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "get_another_question: d passato è NULL" );
		return FALSE;
	}

	qhandler = get_question_handler( d );
	if ( !qhandler )
	{
		send_log( NULL, LOG_BUG, "get_another_question: nessun qhandler trovato per il pg %s", d->character->name );
		return FALSE;
	}

	/* Se non ha raggiunto il numero di domande da chiedere passa alla domanda successiva */
	if ( qhandler->sended_count < MAX_QUESTION_TO_SEND-1 )
	{
		show_get_question( d );
		return TRUE;
	}

	return FALSE;
}


/*
 * Elabora le risposte date dalle domende.
 */
int answers_elaboration( DESCRIPTOR_DATA *d )
{
	QHANDLER_DATA  *qhandler;
	int				maximum;	/* Memorizza quale classe ha ottenuto più risposte */
	int				x;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "elaboration_of_answers: d passato è NULL" );
		return CLASS_NONE;
	}

	qhandler = get_question_handler( d );
	if ( !qhandler )
	{
		send_log( NULL, LOG_BUG, "answers_elaboration: nessun qhandler trovato per il pg %s", d->character->name );
		return CLASS_NONE;
	}

	maximum = 0;
	/* Calcola quale è stato il maggiormente scelto tra il contatore delle risposte */
	for ( x = 0;  x < top_class_play-1;  x++ )
	{
		if ( qhandler->choised[maximum] == qhandler->choised[x+1] )
		{
			/* Se c'e una scelta che pesa alla stessa maniera allora sceglie a caso */
			if ( number_range(0, 1) )
				maximum = x+1;
		}
		else if ( qhandler->choised[maximum] < qhandler->choised[x+1] )
			maximum = x+1;
	}

	free_question_handler_desc( d );

	/* Controllo che il valore scelto sia ok */
  	if ( check_class(d->character, maximum) )
		return maximum;

	send_to_char( d->character, "Purtroppo la classe risultante dalle tue scelte non e' disponibile\r\n." );
	close_socket( d, FALSE );
	return -1;
}


#endif	/* T2_QUESTION */
