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


/* Modulo riguardante il reboot e il copyover */


#include <dlfcn.h>		/* libdl */
#include <unistd.h>

#include "mud.h"
#include "admin.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "mccp.h"
#include "nanny.h"
#include "olc.h"
#include "reboot.h"
#include "save.h"


/*
 * Variabili Globali
 */
char			 reboot_time[50];
time_t			 new_boot_time_t;
time_t			 boot_time;
char			 str_boot_time[MIL];
bool			 mud_down;				/* Shutdown */


/*
 * Variabili locali
 */
HOUR_MIN_SEC	 set_boot_time_struct;
HOUR_MIN_SEC	*set_boot_time;
struct tm		*new_boot_time;
struct tm		 new_boot_struct;


/*
 * Definizione di file di copyover
 */
#define COPYOVER_FILE	SYSTEM_DIR	"copyover.dat"		/* for warm reboots */


/*
 * Se modificate questa parte fate attenzione a riavviare il Mud con un copyover subito dopo
 *	perché si potrebbe inloopare
 * (TT) Fare attenzione che molto probabilmente devo tenere d'occhio i diversi protocolli
 */
void fread_copyover( MUD_FILE *fp )
{
	DESCRIPTOR_DATA *d;
	char			*name = NULL;
	bool			 fOld;

	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->connected = CON_COPYOVER_RECOVER;

	unlink( COPYOVER_FILE );	/* In case something crashes - doesn't prevent reading */

	for ( ; ; )
	{
		char *word;

		word   = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		/* L'etichetta Descriptor deve essere acquisita per prima per inizializzare il descrittore */
		if		( !str_cmp(word, "Descriptor"	) )		init_descriptor( d, fread_number(fp) );
		else if ( !str_cmp(word, "Name"			) )		name =				fread_string( fp );
		else if ( !str_cmp(word, "Host"			) )
		{
			DISPOSE( d->host );
			d->host = fread_string( fp );
		}
#ifdef T2_MCCP
		else if ( !str_cmp(word, "Mccp"			) )		d->mccp->can_compress =	fread_bool( fp );
#endif
#ifdef T2_MSP
		else if ( !str_cmp(word, "Msp"			) )		d->msp =			fread_bool( fp );
#endif
#ifdef T2_MXP
		else if ( !str_cmp(word, "Mxp"			) )		d->mxp =			fread_bool( fp );
#endif
#ifdef T2_DETCLIENT
		else if ( !str_cmp(word, "Client"		) )
		{
			DISPOSE( d->client );
			d->client = fread_string( fp );
		}
#endif
		else if ( !str_cmp(word, "End"			) )		break;
		else
			send_log( fp, LOG_FREAD, "fread_copyover: nessuna etichetta per la parola %s", word );
	}

	if ( ++num_descriptors > mudstat.boot_max_players )
		mudstat.boot_max_players = num_descriptors;

	LINK( d, first_descriptor, last_descriptor, next, prev );

	/* Scrive qualcosa al descrittore per controllare se sia tutto ok */
	if ( !write_to_descriptor(d, " *** (Recupero dal Riavvio) ***\r\n\r\nA poco a poco lo Scisma si dipana ed il Tempo ancora scorre nel Fiume Nebbioso.\r\n", 0) )
	{
		free_desc( d );	/* nope */
		return;
	}

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_FREAD, "fread_copyover: Nome acquisito da una sezione COPYOVER errato" );
		return;
	}

	/*
	 * Ora carica il file del personaggio
	 */
	fOld = load_player( d, name, FALSE );

	if ( fOld == FALSE ) /* Player file not found?! */
	{
		write_to_descriptor( d, "\r\nIn qualche modo, il tuo personaggio è stato perso nel copyover.\r\n", 0 );
		close_socket( d, FALSE );
	}
	else /* ok! */
	{
		write_to_descriptor( d, "\r\nEviti l'impatto astrale e riesci a recuperare possesso del tuo corpo.\r\n", 0 );

		/* Just In Case */
		if ( !d->character->in_room )
			d->character->in_room = get_room_index( NULL, VNUM_ROOM_TEMPLE );

		/* Insert in the char_list */
		LINK( d->character, first_char, last_char, next, prev );
		LINK( d->character, first_player, last_player, next_player, prev_player );

		char_to_room( d->character, d->character->in_room );
		send_command( d->character, "look", CO );
		act( AT_ACTION, "$n si materializza!", d->character, NULL, NULL, TO_ROOM );
		d->connected = CON_PLAYING;
	}
}


/*
 * Recupero da un Copyover - Carica i descrittori
 */
void load_copyover( void )
{
	send_log( NULL, LOG_LOAD, "Recupero da Copyover" );
	fread_section( COPYOVER_FILE, "COPYOVER", fread_copyover, TRUE );
}


/*
 * Attenzione, se uno fa copyover quando è stato modificato il cidice relativo al riavvio
 *	e al file per le info di copyover allora si potrebbe inloopare, è normale
 */
DO_RET do_copyover( CHAR_DATA *ch, char *argument )
{
	extern int		 control;

	MUD_FILE		*fp;
	DESCRIPTOR_DATA *d;
	DESCRIPTOR_DATA *d_next;
	char			 buf[MIL];

	fp = mud_fopen( "", COPYOVER_FILE, "w", TRUE );
	if ( !fp )
		return;

#ifdef T2_OLC
	/* Consider changing all saved areas here, if you use OLC */
/*	send_command( ch, "asave", CO ); */	/* autosave changed areas */
#endif

	if ( str_cmp(argument, "mud now") && ch )
	{
		set_char_color( AT_PLAIN, ch );

		for ( d = first_descriptor;  d ;  d = d->next )
		{
			if ( is_editing(d) )
			{
				ch_printf( ch, "Il giocatore %s sta editando, usare &Gcopyover &gmud now&w per riavviare comunque.\r\n", d->character->name );
				return;
			}

			if ( is_in_olc(d) )
			{
				ch_printf( ch, "Il giocatore %s è in OLC, usare &Gcopyover &gmud now&w per riavviare comunque.\r\n", d->character->name );
				return;
			}
		}
	}

	/* For each playing descriptor, save its state */
	for ( d = first_descriptor;  d ;  d = d_next )
	{
		CHAR_DATA *och = ch_original( d );
		d_next = d->next;		/* We delete from the list , so need to save this */

		if ( !d->character || d->connected < CON_PLAYING )	/* drop those logging on */
		{
			write_to_descriptor( d, "\r\nSorry, we are rebooting. Come back in a few minutes.\r\n", 0 );
			close_socket( d, FALSE );	/* throw'em out */
		}
		else
		{
			fputs( "#COPYOVER\n", fp->file );
			fprintf( fp->file, "Descriptor   %d\n", d->descriptor );
			fprintf( fp->file, "Name         %s~\n", och->name );
			fprintf( fp->file, "Host         %s~\n", d->host );
#ifdef T2_MCCP
			fprintf( fp->file, "Mccp         %s\n", fwrite_bool(d->mccp->can_compress) );
#endif
#ifdef T2_MSP
			fprintf( fp->file, "Msp          %s\n", fwrite_bool(d->msp) );
#endif
#ifdef T2_MXP
			fprintf( fp->file, "Mxp          %s\n", fwrite_bool(d->mxp) );
#endif
#ifdef T2_DETCLIENT
			fprintf( fp->file, "Client       %s~\n", d->client );
#endif
			fprintf( fp->file, "End\n\n" );

			save_player( och );

			write_to_descriptor( d, "\r\nUn immenso Scisma Astrale si impossessa di questo Piano ribaltando la Realtà Materiale!\r\n", 0 );
			write_to_descriptor( d, "\r\n *** (Riavvio del Mud) ***\r\n", 0 );
		}
	}

	fprintf( fp->file, "#END\n" );
	MUD_FCLOSE( fp );

	dlclose( sysdata.dlHandle );	/* chiude la libdl */

	/* Salva le informazioni per il copyover */
	sysdata.copyover = TRUE;
	sysdata.control = control;
	save_sysdata();

	/* exec - descriptors are inherited */
	execl( EXE_FILE, "bard", "-1", (char *) NULL );

	/* Failed - sucessful exec will not return */
	sprintf( buf, "Copyover FALLITO!" );
	if ( ch )
		ch_printf( ch, "%s\r\n", buf );
	send_log( NULL, LOG_WARNING, "%s", buf );

	sysdata.dlHandle = dlopen( NULL, RTLD_LAZY );
	if ( !sysdata.dlHandle )
	{
		send_log( NULL, LOG_LOAD, "FATAL ERROR: Unable to reopen system executable handle!" );
		exit( EXIT_FAILURE );
	}
}


void init_reboot_time( void )
{
	struct timeval	now_time;

	/* Ora dell'inizio. */
	gettimeofday( &now_time, NULL );
	current_time = (time_t) now_time.tv_sec;
	boot_time = time(0);
	strcpy( str_boot_time, friendly_ctime(&current_time) );

	/* Ora dell'inizio del boot. */
	set_boot_time = &set_boot_time_struct;
	set_boot_time->manual = 0;

	new_boot_time = update_boot_time(localtime(&current_time));

	/* Copies *new_boot_time to new_boot_struct,
		and then points new_boot_time to new_boot_struct again. */
	new_boot_struct = *new_boot_time;
	new_boot_time = &new_boot_struct;
	new_boot_time->tm_mday += 1;

	if ( new_boot_time->tm_hour > 12 )
		new_boot_time->tm_mday += 1;

	new_boot_time->tm_sec = 0;
	new_boot_time->tm_min = 0;
	new_boot_time->tm_hour = 6;

	/* Aggiorna new_boot_time (due to day increment) */
	new_boot_time	= update_boot_time( new_boot_time );
	new_boot_struct = *new_boot_time;
	new_boot_time	= &new_boot_struct;
	new_boot_time_t = mktime( new_boot_time );
	reboot_check( mktime(new_boot_time) );

	/* Set reboot time string for do_time */
	get_reboot_string( );
}


/*
 * Struttura per i dati di reboot
 */
struct reboot_data
{
	int		 times;
	char	*tmsg1;
	char	*tmsg2;
};

#define MAX_REBOOT_MSG	8

struct reboot_data table_reboot[MAX_REBOOT_MSG] =
{
	{	  60,	"&YLa terra si scuote tutta, vibra il suolo.. la fine è vicina..",
				"&YSenti la terra tremare come se la fine fosse vicina!" },
	{	 120,	"&YUn lampo fluorescente squarcia il cielo!",
				"&YI fulmini crepitano in alto nel cielo!" },
	{	 180,	"&YFragori di tuoni frustano la terra.. l'aria è elettrica..",
				"&YIl rombo dei tuoni rimbomba in lontananza!" },
	{	 240,	"&YD'improvviso il cielo ha mutato il suo colore in un bruno mantello..",
				"&YImprovvisamente il cielo è diventato buio come a mezzanotte." },
	{	 300,	"&YTi rendi conto del fatto che le forme di vita presenti stanno diminuendo lentamente e.. inevitabilmente..",
				"&YNoti le forme di vita intorno a te diminuire lentamente." },
	{	 600,	"&YOgni specchio d'acqua sta rapidamente ghiacciando..",
				"&YI mari intorno al regno sono diventati freddi." },
	{	 900,	"&YLe raziadioni diventano improvvisamente instabili, vibrano e la loro luce s'affievolisce.. ",
				"&YL'aura magica che circonda il mondo sembra leggermente instabile." },
	{	1800,	"&YAvverti una sorta di ricambio tra le forze magiche che ti circondano.",
				"&YPercepisci un cambiamento nelle forze magiche che ti circondano." },
};


void reboot_check( time_t reset )
{
	char		buf[MSL];
	static int	trun;
	static bool	init = FALSE;

	if ( !init || reset >= current_time )
	{
		for ( trun = MAX_REBOOT_MSG-1;  trun >= 0;  trun-- )
		{
			if ( reset >= current_time + table_reboot[trun].times )
				break;
		}

		init = TRUE;
		return;
	}

	if ( (current_time % 1800) == 0 )
	{
		sprintf( buf, "%.24s: %d players\n", friendly_ctime(&current_time), num_descriptors );
		append_to_file( USAGE_FILE, buf );

		sprintf( buf, "%.24s:  %dptn  %dpll  %dsc %dbr  %d global loot",
			friendly_ctime(&current_time),
			mudstat.upotion_val,
			mudstat.upill_val,
			mudstat.used_scribed,
			mudstat.used_brewed,
			mudstat.global_looted );
		append_to_file( ECONOMY_FILE, buf );
	}

	if ( new_boot_time_t - boot_time < 60*60*18 && !set_boot_time->manual )
		return;

	if ( new_boot_time_t <= current_time )
	{
		CHAR_DATA *vch;

		echo_to( "&YUna forte presenza magica ti attira verso questi Piani.\r\n"
			"Come se la vita stesse ricominciando in questo attimo..", NULL, TO_MUD );

		for ( vch = first_player;  vch;  vch = vch->next_player )
			save_player( vch );

		/* Riavvia di copyover se l'opzione relativa è attiva */
		if ( sysdata.copyover_reboot )
			do_copyover( NULL, "" );
		else
			mud_down = TRUE;

		return;
	}

	if ( trun != -1 && new_boot_time_t - current_time <= table_reboot[trun].times )
	{
		if ( trun >= 0 && trun <= MAX_REBOOT_MSG-1 )	/* non dovrebbe servire, ma non si sa mai */
		{
			if ( number_range(0, 1) )
				echo_to( table_reboot[trun].tmsg1, NULL, TO_MUD );
			else
				echo_to( table_reboot[trun].tmsg2, NULL, TO_MUD );
		}

		/* Evita di fare entrare player nel mud negli ultimi secondo prima del riavvio */
		if ( trun <= 5 )
			sysdata.deny_new_players = TRUE;

		--trun;
		return;
	}
}


/*
 * Comando di shutdown
 */
DO_RET do_shutdown( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*vch;
#ifdef T2_ALFA
	char		 gecho[MIL];
#endif

	set_char_color( AT_ADMIN, ch );

	/* (FF) aggiungere la tipologia reboot se serve */
	if ( str_cmp(argument, "mud ora") && str_cmp(argument, "mud now")
	  && !is_name(argument, "nosalva nosave") )
	{
		send_to_char( ch, "&YSintassi&w:  shutdown mud now\r\n" );
		send_to_char( ch, "           shutdown nosave\r\n" );
		return;
	}

	send_log( NULL, LOG_COMM, "Shutdown da %s.", ch->name );

#ifdef T2_ALFA
	/* (FF) trovare dei messaggi rpg per lo shutdown */
	sprintf( gecho, "gecho %s", buf );
	send_command( ch, gecho, CO );
#endif

	/* Save all characters before booting. */
	if ( !is_name(argument, "nosalva nosave") )
	{
		for ( vch = first_player;  vch;  vch = vch->next_player )
			save_player( vch );
	}

	mud_down = TRUE;
}


/*
 * (TT) (FF) da ripescare e provare il comando reboot?
 */


struct tm *update_boot_time( struct tm *old_time )
{
	time_t up_time;

	up_time = mktime( old_time );
	return localtime( &up_time );
}


void get_reboot_string( void )
{
	strcpy( reboot_time, asctime(new_boot_time) );
}


DO_RET do_setboot( CHAR_DATA *ch, char *argument )
{
	char	arg[MIL];
	bool	check = FALSE;

	set_char_color( AT_ADMIN, ch );

	argument = one_argument( argument, arg );

	if ( is_name(arg, "orario time") )
	{
		struct tm *now_time;
		char	   arg1[MIL];

 		argument = one_argument( argument, arg );
		argument = one_argument( argument, arg1 );
		
		if ( !VALID_STR(arg) || !VALID_STR(arg1)
		  || !is_number(arg) || !is_number(arg1) )
		{
			send_to_char( ch, "Devi inserire almeno ora e minuto del reboot nel formato setboot time <ora> <minuto>.\r\n" );
			return;
		}

		now_time = localtime(&current_time);
		if ( (now_time->tm_hour = atoi(arg)) < 0 || now_time->tm_hour > 23 )
		{
			send_to_char( ch, "L'orario del reboot non è valido, i valori per le ore sono compresi tra 0 e 23.\r\n" );
			return;
		}

		if ( (now_time->tm_min = atoi(arg1)) < 0 || now_time->tm_min > 59 )
		{
			send_to_char( ch, "L'orario del reboot non è valido, i valori per i minuti sono compresi tra 0 e 59.\r\n" );
			return;
		}

		argument = one_argument( argument, arg );
		if ( VALID_STR(arg) && is_number(arg) )
		{
			if ( (now_time->tm_mday = atoi(arg)) < 1 || now_time->tm_mday > 31 )
			{
				send_to_char( ch, "La data per il reboot non è valida, i valori per i giorni sono compresi tra 1 e 31.\r\n" );
				return;
			}

			argument = one_argument( argument, arg );
			if ( VALID_STR(arg) && is_number(arg) )
			{
				if ( (now_time->tm_mon = atoi(arg)) < 1 || now_time->tm_mon > 12 )
				{
					send_to_char( ch, "La data per il reboot non è valida, i valori per i mesi sono compresi tra 1 e 12.\r\n" );
					return;
				}
				now_time->tm_mon--;
				argument = one_argument( argument, arg );

				if ( (now_time->tm_year = atoi(arg)-1900) < 0
				  || now_time->tm_year > 199 )
				{
					send_to_char( ch, "La data per il reboot non è valida, i valori per gli anni sono compresi tra 1900 e 2099.\r\n" );
					return;
				}
			}
		}

		now_time->tm_sec = 0;
		if ( mktime(now_time) < current_time )
		{
			send_to_char( ch, "Non puoi impostare una data precedente a questo istante!\r\n" );
			return;
		}

		if ( set_boot_time->manual == 0 )
			set_boot_time->manual = 1;

		new_boot_time	= update_boot_time( now_time );
		new_boot_struct	= *new_boot_time;
		new_boot_time	= &new_boot_struct;
		reboot_check( mktime(new_boot_time) );
		get_reboot_string( );

		ch_printf( ch, "data per il reboot fissata: %s\r\n", reboot_time );
		check = TRUE;
	}
	else if ( is_name(arg, "manuale manual") )	/* (FF) sarà il caso di fare una opzione apposita e di toglierlo da qui? */
	{
		int value;

		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Devi inserire un valore (1/0) on/off\r\n" );
			return;
		}

		if ( !is_number(argument) || (value = atoi(argument)) < 0 || value > 1 )
		{
			send_to_char( ch, "Il valore deve essere 0 (off) o 1 (on)\r\n" );
			return;
		}

		set_boot_time->manual = value;
		ch_printf( ch, "Flag manuale impostata a: %d\r\n", value );
		check = TRUE;
		get_reboot_string( );
		return;
	}
	else if ( !str_cmp(arg, "default") )
	{
		set_boot_time->manual = 0;
		/* Reinitialize new_boot_time */
		new_boot_time = localtime(&current_time);
		new_boot_time->tm_mday += 1;
		if (new_boot_time->tm_hour > 12)
			new_boot_time->tm_mday += 1;
		new_boot_time->tm_hour = 6;
		new_boot_time->tm_min = 0;
		new_boot_time->tm_sec = 0;
		new_boot_time = update_boot_time( new_boot_time );

		sysdata.deny_new_players = FALSE;

		send_to_char( ch, "Il reboot è stato reimpostato al default.\r\n" );
		check = TRUE;
	}
	else
	{
		send_to_char( ch, "&YSintassi&w: setboot time <ora> <minuto> <giorno> <mese> <anno>\r\n" );
		send_to_char( ch, "          setboot manual 0|1\r\n" );
		send_to_char( ch, "          setboot default\r\n\r\n" );

		ch_printf( ch, "Reboot impostato a: %s, la flag manuale: %d\r\n",
			reboot_time, set_boot_time->manual );
		return;
	}

	if ( check )
	{
		get_reboot_string( );
		new_boot_time_t = mktime(new_boot_time);
	}
}
