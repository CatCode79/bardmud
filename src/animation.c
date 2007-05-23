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
 >						Modulo per la gestione delle animazioni							<
\****************************************************************************************/


#include "mud.h"
#include "animation.h"
#include "editor.h"
#include "fread.h"
#include "gramm.h"


/*
 * File con la lista delle animazioni da caricare
 */
#define	ANIMATION_LIST		"../animations/animation.lst"


/*
 * Definizioni
 */
#define	MAX_ANIMATION	 5	/* Numero massimo di animazioni caricabili */
#define MAX_FRAME		36	/* Numero massimo frame caricabili in un'animazione */


/*
 * Struttura di un frame
 */
typedef struct frame_data	FRAME_DATA;

struct frame_data
{
	FRAME_DATA	*next;
	FRAME_DATA	*prev;
	char		*text;
};


/*
 * Struttura di un'animazione
 */
typedef struct	animation_data	ANIMATION_DATA;

struct animation_data
{
	ANIMATION_DATA	*prev;
	ANIMATION_DATA	*next;
	char			*name;			/* Nome identificativo dell'animazione */
	FRAME_DATA		*first_frame;	/* Lista dei frame */
	FRAME_DATA		*last_frame;
};


/*
 * Variabili
 */
int						 top_animation		= 0;	/* Numero di animazioni caricate */
static int				 top_frame			= 0;
static ANIMATION_DATA	*first_animation	= NULL;
static ANIMATION_DATA	*last_animation		= NULL;


/*
 * Legge un fream di una animazione
 */
void subfread_animation_frame( MUD_FILE *fp, ANIMATION_DATA *animation )
{
	FRAME_DATA *frame;

	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "subfread_animation_frame: fp passato è NULL" );
		return;
	}

	if ( !animation )
	{
		send_log( fp, LOG_BUG, "subfread_animation_frame: animation passata è NULL" );
		return;
	}

	CREATE( frame, FRAME_DATA, 1 );

	frame->text = fread_string( fp );

	if ( !VALID_STR(frame->text) )
	{
		send_log( fp, LOG_FREAD, "subfread_animation_frame: text del frame non è valido" );
		exit( EXIT_FAILURE );
	}

	LINK( frame, animation->first_frame, animation->last_frame, next, prev );
	top_frame++;
}


/*
 * Legge un'animazione da un file
 * La struttura di questa funzione si basa sulle molte altre funzioni di tipo fread
 */
static void fread_animation( MUD_FILE *fp )
{
	ANIMATION_DATA	*animation;

	if ( !fp )
	{
		send_log( NULL, LOG_BUG, "fread_animation: fp passato è NULL" );
		return;
	}

	CREATE( animation, ANIMATION_DATA, 1 );
	top_frame = 0;

	for ( ; ; )
	{
		char *word;

		word = fread_word( fp );

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_animation: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Frame"	) )		subfread_animation_frame( fp, animation );
		else if ( !str_cmp(word, "Name"		) )		animation->name =		str_dup( fread_word(fp) );
		else if ( !str_cmp(word, "End"		) )		break;
		else
			send_log( fp, LOG_FREAD, "fread_animation: nessuna etichetta per la parola %s", word );
	}

	if ( !VALID_STR(animation->name) )
	{
		send_log( fp, LOG_FREAD, "fread_animation: nome dell'animazione non valido" );
		exit( EXIT_FAILURE );
	}

	if ( top_frame <= 1 || top_frame >= MAX_FRAME )
	{
		send_log( fp, LOG_FREAD, "fread_animation: numero di frame caricati errato: %d", top_frame );
		exit( EXIT_FAILURE );
	}

	LINK( animation, first_animation, last_animation, next, prev );
	top_animation++;
}


/*
 * Carica tutte le animazioni
 */
void load_animations( void )
{
	fread_list( ANIMATION_LIST, "ANIMATION", fread_animation, TRUE );

	if ( top_animation < 0 || top_animation >= MAX_ANIMATION )
	{
		send_log( NULL, LOG_FREAD, "load_animations: troppe animazioni caricate %d, superato MAX_ANIMATION %d",
			top_animation, MAX_ANIMATION );
		exit( EXIT_FAILURE );
	}
}


/*
 * Libera dalla memoria tutte le animazioni
 */
void free_all_animations( void )
{
	ANIMATION_DATA *animation;

	for ( animation = first_animation;  animation;  animation = first_animation )
	{
		FRAME_DATA *frame;

		UNLINK( animation, first_animation, last_animation, next, prev );
		top_animation--;

		DISPOSE( animation->name );
		for ( frame = animation->first_frame;  frame;  frame = animation->first_frame )
		{
			UNLINK( frame, animation->first_frame, animation->last_frame, next, prev );
			DISPOSE( frame->text );
			DISPOSE( frame );
		}
		DISPOSE( animation );
	}

	if ( top_animation != 0 )
		send_log( NULL, LOG_BUG, "free_all_animations: top_animation non è 0 dopo aver liberato tutte le animazioni: %d", top_animation );
}


/*
 * Ritorna la grandezza sizeof delle animazioni in memoria
 */
int get_sizeof_animation( void )
{
	return top_animation * sizeof( ANIMATION_DATA );
}


/*
 * Ritorna l'animazione rispettiva al numero
 */
static ANIMATION_DATA *get_animation_from_num( const int num )
{
	ANIMATION_DATA *animation;
	int				count = 0;

	if ( num < 0 || num >= MAX_ANIMATION )
	{
		send_log( NULL, LOG_BUG, "get_animation_from_num: num passato errato: %d", num );
		return NULL;
	}

	for ( animation = first_animation;  animation;  animation = animation->next )
	{
		if ( count == num )
			return animation;
		count++;
	}

	return NULL;
}

/*
 * Recupera il numero di frame relativa all'animazione
 */
static FRAME_DATA *get_frame_from_num( ANIMATION_DATA *animation, const int num )
{
	FRAME_DATA	*frame;
	int			 count = 0;

	if ( !animation )
	{
		send_log( NULL, LOG_BUG, "get_frame_from_num: animation passata è NULL" );
		return NULL;
	}

	if ( num < 0 || num >= MAX_FRAME )
	{
		send_log( NULL, LOG_BUG, "get_frame_from_num: num passato errato: %d", num );
		return NULL;
	}

	for ( frame = animation->first_frame;  frame;  frame = frame->next )
	{
		if ( count == num )
			return frame;
		count++;
	}

	return NULL;
}


/*
 * Inizializza le variabili del descrittore per l'invio dell'animazione
 */
void init_desc_ani( DESCRIPTOR_DATA *d )
{
	if ( !d )
	{
		send_log( NULL, LOG_BUG, "init_desc_ani: d passato è NULL" );
		return;
	}

	/* Setta l'animazione da inviare a caso */
	d->animation = number_range( 0, top_animation-1 );

	/* Setta al primo frame */
	d->frame = 0;

	/* Serve per fare andare avanti frame dopo frame */
	strcpy( d->incomm, "+" );
}


/*
 * Controlla se si sta inviando un'animazione
 */
bool want_animation( DESCRIPTOR_DATA *d )
{
	if ( !d )
	{
		send_log( NULL, LOG_BUG, "want_animation: d passato è NULL" );
		return FALSE;
	}

	if ( d->animation == -1 )
		return FALSE;

	if ( d->frame == -1 )
		return FALSE;

	return TRUE;
}


/*
 * Visualizza una animazione a coloro che si stanno connettendo
 * Ritorna FALSE se non c'è bisogno di inviare un'animazione
 * Ritorna TRUE quando ha terminato di inviare un frame
 */
static void send_animation( DESCRIPTOR_DATA *d )
{
	ANIMATION_DATA *animation;
	FRAME_DATA	   *frame;
	char			csex;

	if ( !d )
	{
		send_log( NULL, LOG_BUG, "send_animation: d passato è NULL" );
		return;
	}

	/* Pulisce lo schermo */
	write_to_buffer( d, ANSI_CLEAR, 4 );

	animation = get_animation_from_num( d->animation );
	if ( !animation )
		return;

	frame = get_frame_from_num( animation, d->frame );
	if ( !frame )
		return;

	/* Invia il frame */
	if ( frame->text[0] == '.' )
		send_to_descriptor( d, frame->text+1 );
	else
		send_to_descriptor( d, frame->text );

	/* All'ultimo frame si ferma */
	if ( frame->next )
	{
		d->frame++;					/* Passa al frame sucessivo */
		strcpy( d->incomm, "+" );	/* Serve per mandarlo avanti senza inviare alcunché */
		return;
	}

	csex = gramm_ao( d->character );
	if ( d->character->level == 0 )
		ch_printf( d->character, "\r\n                      &CB&cenvenut%c &CA&cvventurier%c!\r\n", csex, csex );
	else
		ch_printf( d->character, "\r\n                      &CB&centornat%c &CA&cvventurier%c!\r\n", csex, csex );
	send_to_descriptor( d, "                           &w[&WPremi Invio&w]\r\n" );

	d->animation = -1;
	d->frame	 = -1;
}


/*
 * Funzione di update per l'invio dei frame delle animazioni
 */
void update_animation( void )
{
	DESCRIPTOR_DATA	*d;

	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( is_in_game(d) )
			continue;

		if ( want_animation(d) )
			send_animation( d );
	}
}
