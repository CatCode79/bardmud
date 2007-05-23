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
 >				File con le funzioni per gestire il sysdata					<
\****************************************************************************/

#include "mud.h"
#include "command.h"
#include "sysdata.h"
#include "fread.h"
#include "interpret.h"


/*
 * Definizioni di file sysdata
 */
#define SYSDATA_FILE		SYSTEM_DIR "sysdata.dat"


/*
 * Tabella con tutte le opzioni configurabili da linea di comando
 */
const struct olc_sysdata_type tableolc_option[] =
{
/*	  label					type			number			string	min		max			descr */
	{ "CanGetAll",			TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"Permette di gettare tutto senza restrizioni di proprietà oggetto"	},
	{ "CheckAdmHost",		TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"Esegue il controllo degli Adminhost alla connessione di un Admin"	},
	{ "CopyoverReboot",		TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"Il reboot automatico del mud è eseguito con un copyover"	},
	{ "DenyNewPlayers",		TYPE_BOOLEAN,	/*FALSE,		NULL,*/	0,		1,			"Blocca la creazione a nuovi giocatori"	},
	{ "NameResolving",		TYPE_BOOLEAN,	/*FALSE,		NULL,*/	0,		1,			"Risolvere l'host dell'utente alla connessione"	},
	{ "MascotteRestring",	TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"Permette di restringare le mascotte"	},
	{ "MascotteSave",		TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"Salva le mascotte"	},
	{ "FreeStealing",		TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"I giocatori possono rubare ad altri giocatori"	},
	{ "OrderCommands",		TYPE_BOOLEAN,	/*FALSE,		NULL,*/	0,		1,			"All'avvio del mud i comandi vengono ordinati secondo il loro utilizzo"	},
	{ "PkLoot",				TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"Permette il loot dei cadaveri dei giocatori"	},
	{ "QuestionCreation",	TYPE_BOOLEAN,	/*FALSE,		NULL,*/	0,		1,			"La creazione del pg viene fatta con delle domande"	},
	{ "TrackTroughDoor",	TYPE_BOOLEAN,	/*TRUE,			NULL,*/	0,		1,			"Il track funziona anche attraverso le porte chiuse"	},
	{ "WizLock",			TYPE_BOOLEAN,	/*FALSE,		NULL,*/	0,		1,			"Blocca l'accesso ai giocatori non Admin"	},
	{ "BashNonTank",		TYPE_NUMBER,	/*0,			NULL,*/	0,		100,		"Chance nel bash contro i mob"	},
	{ "BashPlrVsPlr",		TYPE_NUMBER,	/*0,			NULL,*/	0,		100,		"Chance nel bash tra pg"	},
	{ "DamPlrVsMob",		TYPE_NUMBER,	/*100,			NULL,*/	50,		200,		"Percentuale di danno contro i mob"	},
	{ "DamPlrVsPlr",		TYPE_NUMBER,	/*100,			NULL,*/	50,		200,		"Percentuale di danno tra pg"	},
	{ "DamMobVsMob",		TYPE_NUMBER,	/*100,			NULL,*/	50,		200,		"Percentuale di danno che i mob danno ad altri mob"	},
	{ "DamMobVsPlr",		TYPE_NUMBER,	/*100,			NULL,*/	50,		200,		"Percentuale di danno che i mob danno ai pg"	},
	{ "DeahHoleMult",		TYPE_BOOLEAN,	/*FALSE,		NULL,*/	1,		80,			"Multiplicatore per perdita di px alla morte dei pg"	},
	{ "GougeNonTank",		TYPE_NUMBER,	/*0,			NULL,*/	0,		100, 		"Chance nel gouge contro i mob"	},
	{ "GougePlrVsPlr",		TYPE_NUMBER,	/*0,			NULL,*/	0,		100, 		"Chance nel gouge tra pg"	},
	{ "LvLGroupDiff",		TYPE_NUMBER,	/*0,			NULL,*/	0,		12, 		"Differenza di livello massima tra i membri del gruppo e il capo"	},
	{ "MaxSocketCon",		TYPE_NUMBER,	/*60,			NULL,*/	1,		500,		"Massimo di connessioni socket accettate dal mud"	},
	{ "ModDodge",			TYPE_NUMBER,	/*2,			NULL,*/	1,		5,			"Divisore della chance del dodge"	},
	{ "ModParry",			TYPE_NUMBER,	/*2,			NULL,*/	1,		5,			"Divisore della chance del parry"	},
	{ "ModTumble",			TYPE_NUMBER,	/*4,			NULL,*/	1,		5,			"Divisore della chance del tumble"	},
	{ "MudPort",			TYPE_NUMBER,	/*5001,			NULL,*/	1024,	32700,		"Porta del mud"	},
	{ "PurgeAfterMonth",	TYPE_NUMBER,	/*3,			NULL,*/	3,		12,			"Distrugge i pg inattivi dopo tot mesi"	},
	{ "PurgeMinHour",		TYPE_NUMBER,	/*5,			NULL,*/	0,		64,			"Considera pg inattivo chi ha giocato meno di tot ore"	},
	{ "PurgeMinLevel",		TYPE_NUMBER,	/*LVL_NEWBIE/2,	NULL,*/	0,		LVL_NEWBIE,	"Considera pg inattivo chi ha meno di tot livelli"	},
	{ "SaveFreq",			TYPE_NUMBER,	/*5,			NULL,*/	0,		60,			"Salva con una frequenza di tot minuti"	},
	{ "SaveLevel",			TYPE_NUMBER,	/*0,			NULL,*/	0,		LVL_LEGEND,	"Permette di salvare dal livello tot in sù"	},
	{ "StunRegular",		TYPE_NUMBER,	/*15,			NULL,*/	0,		100,		"Chance di stun contro i mob"	},
	{ "StunPlrVsPlr",		TYPE_NUMBER,	/*65,			NULL,*/	0,		100,		"Chance di stun tra giocatori"	},
	{ NULL,					TYPE_NUMBER,	/*-1,			NULL,*/	0,		0,			NULL }
};


/*
 * Stampa la sintassi delle opzioni di linea di comando
 */
void print_syntax_option( void )
{
	int x;

	printf( "Sintassi:\n" );
	printf( "./bard [-(No)label] (value)\n\n" );

	printf( "In dettaglio:\n" );
	printf( "Per attivare una opzione passare la label relativa all'opzione con un trattino davanti:\n" );
	printf( "       [-label]\n" );
	printf( "Per disattivare una opzione passare la label relativa all'opzione con un \"-No\" davanti:\n" );
	printf( "       [-Nolabel]\n" );
	printf( "Per impostare una opzione numerica passare la label relativa all'opzione con il trattino davanti, e poi il valore numerico da impostare:\n" );
	printf( "       [-label] [value]\n\n" );

	printf( "Ecco tutte le label relative alle opzioni attivabili e disattivabili:\n" );
	for ( x = 0;  tableolc_option[x].label;  x++ )
	{
		if ( tableolc_option[x].type == TYPE_BOOLEAN )
			printf( "%18s %s\n", tableolc_option[x].label, tableolc_option[x].descr );
	}
	printf( "\n" );

	printf( "Ecco tutte le label relative alle opzioni numeriche, a cui bisogna passare un valore, con relativo minimo e massimo impostabile:\n" );
	for ( x = 0;  tableolc_option[x].label;  x++ )
	{
		if ( tableolc_option[x].type == TYPE_NUMBER )
		{
			printf( "%18s %3d %3d %s\n",
				tableolc_option[x].label,
				tableolc_option[x].limit_min,
				tableolc_option[x].limit_max,
				tableolc_option[x].descr );
		}
	}
	printf( "\n\n" );

	printf( "Attenzione: le opzioni passate rimangono permanenti, vengono salvate nel file ../system/sysdata.dat\n\n" );

	printf( "Esempi:\n" );
	printf( "Per attivare il wizlock:                       ./bard -wizlock\n" );
	printf( "Per disattivare il salvataggio delle mascotte: ./bard -Nomascottesave\n" );
	printf( "Per impostare le connessioni massime a 15:     ./bard -maxsocketcon 15\n" );
	printf( "Ovviamente si possono inviare più opzioni:     ./bard -wizlock -Nomascottesave -maxsocketcon 15\n\n" );
}


/*
 * Legge da linea di comando le opzione passate all'avvio del Mud
 */
void get_sysdata_options( char **argv )
{
#if 0
	int		x;

	for ( x = 0;  ; x++ )
	{
		int		y;
		bool	found = FALSE;	/* found a FALSE indica che non ha ancora trovato la corrispondente label con l'opzione passata */
		bool	on = TRUE;		/* on a FALSE indica che l'opzione booleana vuole essere disattivata */

		if ( !VALID_STR(argv[x]) )
			break;

		if ( argv[x][0] != '-' )
		{
			print_syntax_option( );
			return;
		}

		/* Se trova -No significa che si vuole disattivare una opzione */
		if ( !str_prefix(str_cmp(argv[x]+1, "No") )
			on = FALSE;

		for ( y = 0;  tableolc_option[y];  y++ )
		{
			if ( ( on && !str_cmp(argv[x]+1, tableolc_option[y]))
			  || (!on && !str_cmp(argv[x]+3, tableolc_option[y]) )
			{
				found = TRUE;
				break;
			}
		}

		/* Se non lo ha trovato è errore */
		if ( !found )
		{
			print_syntax_option( );
			return;
		}

		/* Se c'è una opzione -No e la label trovata non è un tipo booleano, allora è errore */
		if ( !on && tableolc_option[y].type != TYPE_BOOLEAN )
		{
			print_syntax_option( );
			return;
		}

		if ( tableolc_option[y].type == TYPE_NUMBER )
		{
			int val;

			x++;	/* aumenta di uno per prendere il valore passato all'opzione */

			if ( !is_number(argv[x]) )
			{
				print_syntax_option( );
				return;
			}

			val = atoi( argv[x] );
			if ( val < tableolc_option[y] || val > tableolc_option[y] )
			{
				print_syntax_option( );
				return;
			}

			if		( !str_cmp(tableolc_option[y], "BashNonTank"	) )		sysdata.bash_nontank	 = val;
			else if ( !str_cmp(tableolc_option[y], "BashPlrVsPlr"	) )		sysdata.bash_plr_vs_plr	 = val;
			else if ( !str_cmp(tableolc_option[y], "DamPlrVsMob"	) )		sysdata.dam_mob_vs_mob	 = val;
			else if ( !str_cmp(tableolc_option[y], "DamPlrVsPlr"	) )		sysdata.dam_mob_vs_plr	 = val;
			else if ( !str_cmp(tableolc_option[y], "DamMobVsMob"	) )		sysdata.dam_plr_vs_mob	 = val;
			else if ( !str_cmp(tableolc_option[y], "DamMobVsPlr"	) )		sysdata.dam_plr_vs_plr	 = val;
			else if ( !str_cmp(tableolc_option[y], "DeathHoleMult"	 ) )	sysdata.death_hole_mult	 = val;
			else if ( !str_cmp(tableolc_option[y], "GougeNonTank"	) )		sysdata.gouge_nontank	 = val;
			else if ( !str_cmp(tableolc_option[y], "GougePlrVsPlr"	) )		sysdata.gouge_plr_vs_plr = val;
			else if ( !str_cmp(tableolc_option[y], "LvLGroupDiff"	) )		sysdata.lvl_group_diff	 = val;
			else if ( !str_cmp(tableolc_option[y], "MaxSocketCon"	) )		sysdata.max_socket_con	 = val;
			else if ( !str_cmp(tableolc_option[y], "ModDodge"		) )		sysdata.mod_dodge		 = val;
			else if ( !str_cmp(tableolc_option[y], "ModParry"		) )		sysdata.mod_parry		 = val;
			else if ( !str_cmp(tableolc_option[y], "ModTumble"		) )		sysdata.mod_tumble		 = val;
			else if ( !str_cmp(tableolc_option[y], "MudPort"		) )		sysdata.mud_port		 = val;
			else if ( !str_cmp(tableolc_option[y], "PurgeAfterMonth") )		sysdata.purge_after_month= val;
			else if ( !str_cmp(tableolc_option[y], "PurgeMinHour"	) )		sysdata.purge_min_hour	 = val;
			else if ( !str_cmp(tableolc_option[y], "PurgeMinLevel"	) )		sysdata.purge_min_level	 = val;
			else if ( !str_cmp(tableolc_option[y], "SaveFreq"		) )		sysdata.save_frequency	 = val;
			else if ( !str_cmp(tableolc_option[y], "SaveLevel"		) )		sysdata.save_level		 = val;
			else if ( !str_cmp(tableolc_option[y], "StunPlrVsPlr"	) )		sysdata.stun_plr_vs_plr	 = val;
			else if ( !str_cmp(tableolc_option[y], "StunRegular"	) )		sysdata.stun_regular		 = val;
			else
			{
				send_log( NULL, LOG_BUG, "get_sysdata_options: label mancante per numero: %d (%s)",
					y, tableolc_option[y].label );
			}
		}
		else if ( tableolc_option[y].type == TYPE_BOOLEAN )
		{
			if		( !str_cmp(tableolc_option[y], "CanGetAll"		 ) )	sysdata.check_adm_host		= (on) ? TRUE : FALSE;
			else if	( !str_cmp(tableolc_option[y], "CheckAdmHost"	 ) )	sysdata.check_adm_host		= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "CopyoverReboot"	 ) )	sysdata.copyover_reboot		= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "DenyNewPlayers"	 ) )	sysdata.deny_new_players	= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "MascotteRestring") )	sysdata.mascotte_restring	= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "MascotteSave"	 ) )	sysdata.mascotte_save		= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "NameResolving"	 ) )	sysdata.name_resolving		= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "FreeStealing"	 ) )	sysdata.free_stealing		= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "OrderCommands"	 ) )	sysdata.order_commands		= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "PkLoot"			 ) )	sysdata.pk_loot				= (on) ? TRUE : FALSE;
#ifdef T2_QUESTION
			else if ( !str_cmp(tableolc_option[y], "QuestionCreation") )	sysdata.question_creation	= (on) ? TRUE : FALSE;
#endif
			else if ( !str_cmp(tableolc_option[y], "TrackTroughDoor" ) )	sysdata.track_trough_door	= (on) ? TRUE : FALSE;
			else if ( !str_cmp(tableolc_option[y], "WizLock"		 ) )	sysdata.wiz_lock			= (on) ? TRUE : FALSE;
			else
			{
				send_log( NULL, LOG_BUG, "get_sysdata_options: label mancante per booleano: %d (%s)",
					y, tableolc_option[y].label );
			}
		}
		else if ( tableolc_option[y].type == TYPE_STRING )
		{
			send_log( NULL, LOG_BUG, "get_sysdata_options: label mancante per stringa: %d (%s)",
				y, tableolc_option[y].label );
		}
	}
#endif
}


/*
 * Carica i valori di default per il sysdata
 * (FF) controllare che non sia il caso di aggiungere altri valori di default
 */
void init_sysdata( void )
{   
	sysdata.check_adm_host		= TRUE;
	sysdata.deny_new_players	= FALSE;
	sysdata.free_stealing		= TRUE;
	sysdata.mascotte_restring	= FALSE;
	sysdata.mascotte_save		= TRUE;
	sysdata.name_resolving		= FALSE;
	sysdata.order_commands		= FALSE;
	sysdata.pk_loot				= TRUE;		/* (FF) forse a FALSE in futuro, visto che c'è il get all proprietario? */
#ifdef T2_QUESTION
	sysdata.question_creation	= FALSE;
#endif
	sysdata.track_trough_door	= TRUE;
	sysdata.wiz_lock			= FALSE;

	sysdata.bash_nontank		= 0;
	sysdata.bash_plr_vs_plr		= 0;
	sysdata.dam_mob_vs_mob		= 100;
	sysdata.dam_mob_vs_plr		= 100;
	sysdata.dam_plr_vs_mob		= 100;
	sysdata.dam_plr_vs_plr		= 100;
	sysdata.gouge_nontank		= 0;
	sysdata.gouge_plr_vs_plr	= 0;
	sysdata.mod_dodge			= 2;
	sysdata.mod_parry			= 2;
	sysdata.mod_tumble			= 4;
	sysdata.purge_after_month	= 3;
	sysdata.purge_min_hour		= 5;
	sysdata.purge_min_level		= LVL_NEWBIE/2;
	sysdata.save_frequency		= 5;		/* in minuti */
	sysdata.save_level			= 0;
	sysdata.stun_plr_vs_plr		= 65;
	sysdata.stun_regular		= 15;

	sysdata.control				= -1;
	sysdata.death_hole_mult		= 20;
	sysdata.lvl_group_diff		= 60;
	sysdata.max_socket_con		= 60;
	sysdata.mud_port			= 5001;
}


void fread_sysdata( MUD_FILE *fp )
{
	char   *word;

	for ( ; ; )
	{
		word = feof( fp->file )  ?  "End"  :  fread_word( fp );

		if		( word[0] == '*'					  )									fread_to_eol( fp );
		else if ( !str_cmp(word, "CheckAdmHost"		) )		sysdata.check_adm_host =	fread_bool( fp );
		else if ( !str_cmp(word, "Copyover"			) )		sysdata.copyover =			fread_bool( fp );
		else if ( !str_cmp(word, "CopyoverReboot"	) )		sysdata.copyover_reboot =	fread_bool( fp );
		else if ( !str_cmp(word, "Control"			) )		sysdata.control =			fread_number( fp );
		else if ( !str_cmp(word, "DenyNewPlayers"	) )		sysdata.deny_new_players =	fread_bool( fp );
		else if ( !str_cmp(word, "NameResolving"	) )		sysdata.name_resolving =	fread_bool( fp );
		else if ( !str_cmp(word, "MascotteRestring" ) )		sysdata.mascotte_restring =	fread_bool( fp );
		else if ( !str_cmp(word, "MascotteSave"		) ) 	sysdata.mascotte_restring =	fread_bool( fp );
		else if ( !str_cmp(word, "MascotteSave"		) ) 	sysdata.mascotte_save =		fread_bool( fp );
		else if ( !str_cmp(word, "FreeStealing"		) )		sysdata.free_stealing =		fread_bool( fp );
		else if ( !str_cmp(word, "OrderCommands"	) )		sysdata.order_commands =	fread_bool( fp );
		else if ( !str_cmp(word, "PkLoot"			) )		sysdata.pk_loot =			fread_bool( fp );
#ifdef T2_QUESTION
		else if ( !str_cmp(word, "QuestionCreation"	) )		sysdata.question_creation =	fread_bool( fp );
#else
		else if ( !str_cmp(word, "QuestionCreation"	) )									fread_bool( fp );
#endif
		else if ( !str_cmp(word, "TrackTroughDoor"	) )		sysdata.track_trough_door =	fread_bool( fp );
		else if ( !str_cmp(word, "WizLock"			) )		sysdata.wiz_lock =			fread_bool( fp );
		else if ( !str_cmp(word, "BashNonTank"		) )		sysdata.bash_nontank =		fread_number( fp );
		else if ( !str_cmp(word, "BashPlrVsPlr"		) )		sysdata.bash_plr_vs_plr =	fread_number( fp );
		else if ( !str_cmp(word, "DamPlrVsMob"		) )		sysdata.dam_plr_vs_mob =	fread_number( fp );
		else if ( !str_cmp(word, "DamPlrVsPlr"		) )		sysdata.dam_plr_vs_plr =	fread_number( fp );
		else if ( !str_cmp(word, "DamMobVsMob"		) )		sysdata.dam_mob_vs_mob =	fread_number( fp );
		else if ( !str_cmp(word, "DamMobVsPlr"		) )		sysdata.dam_mob_vs_plr =	fread_number( fp );
		else if ( !str_cmp(word, "DeathHoleMult"	) )		sysdata.death_hole_mult =	fread_positive( fp );
		else if ( !str_cmp(word, "End"				) )		break;
		else if ( !str_cmp(word, "GougeNonTank"		) )		sysdata.gouge_nontank =		fread_number( fp );
		else if ( !str_cmp(word, "GougePlrVsPlr"	) )		sysdata.gouge_plr_vs_plr =	fread_number( fp );
		else if ( !str_cmp(word, "LvLGroupDiff"		) )		sysdata.lvl_group_diff =	fread_number( fp );
		else if ( !str_cmp(word, "MaxSocketCon"		) )		sysdata.max_socket_con =	fread_number( fp );
		else if ( !str_cmp(word, "ModDodge"			) )		sysdata.mod_dodge =			fread_number( fp );
		else if ( !str_cmp(word, "ModParry"			) )		sysdata.mod_parry =			fread_number( fp );
		else if ( !str_cmp(word, "ModTumble"		) )		sysdata.mod_tumble =		fread_number( fp );
		else if ( !str_cmp(word, "MudPort"			) )		sysdata.mud_port =			fread_number( fp );
		else if ( !str_cmp(word, "PurgeAfterMonth"	) )		sysdata.purge_after_month =	fread_number( fp );
		else if ( !str_cmp(word, "PurgeMinHour"		) )		sysdata.purge_min_hour =	fread_number( fp );
		else if ( !str_cmp(word, "PurgeMinLevel"	) )		sysdata.purge_min_level =	fread_number( fp );
		else if ( !str_cmp(word, "SaveFreq"			) )		sysdata.save_frequency =	fread_number( fp );
		else if ( !str_cmp(word, "SaveLevel"		) )		sysdata.save_level =		fread_number( fp );
		else if ( !str_cmp(word, "StunPlrVsPlr"		) )		sysdata.stun_plr_vs_plr =	fread_number( fp );
		else if ( !str_cmp(word, "StunRegular"		) )		sysdata.stun_regular =		fread_number( fp );
		else
			send_log( fp, LOG_FREAD, "fread_sysdata: nessuna etichetta per %s", word );
	} /* chiude il for */

	if ( sysdata.bash_nontank < 0 || sysdata.bash_nontank > 100 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di bash contro i mob errato: %d", sysdata.bash_nontank );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.bash_plr_vs_plr < 0 || sysdata.bash_plr_vs_plr > 100 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di bash tra pg errato: %d", sysdata.bash_plr_vs_plr );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.dam_plr_vs_mob < 50 || sysdata.dam_plr_vs_mob > 200 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: danno pg vs mob errato: %d", sysdata.dam_plr_vs_mob );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.dam_plr_vs_plr < 50 || sysdata.dam_plr_vs_plr > 200 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: danno pg vs pg errato: %d", sysdata.dam_plr_vs_plr );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.dam_mob_vs_mob < 50 || sysdata.dam_mob_vs_mob > 200 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: danno mob vs mob errato: %d", sysdata.dam_mob_vs_mob );
/*		exit( EXIT_FAILURE );*/
	}

	if ( sysdata.dam_mob_vs_plr < 50 || sysdata.dam_mob_vs_plr > 200 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: danno mov vs pg errato: %d", sysdata.dam_mob_vs_plr );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.death_hole_mult <= 1 || sysdata.death_hole_mult > 80 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: moltiplicatore di hole alla morte errato: %d", sysdata.death_hole_mult );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.gouge_nontank < 0 || sysdata.gouge_nontank > 100 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di gouge contro i mob errato: %d", sysdata.gouge_nontank );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.gouge_plr_vs_plr < 0 || sysdata.gouge_plr_vs_plr > 100 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di gouge tra pg errato: %d", sysdata.gouge_plr_vs_plr );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.lvl_group_diff < 0 || sysdata.lvl_group_diff > 12 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: range massimo di livello gruppo errato: %d", sysdata.lvl_group_diff );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.max_socket_con <= 0 || sysdata.max_socket_con > 1000 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: massimo di connessioni possibili errata: %d", sysdata.max_socket_con );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.mod_dodge < 1 || sysdata.mod_dodge > 5 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di dodge errato: %d", sysdata.mod_dodge );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.mod_parry < 1 || sysdata.mod_parry > 5 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di parry errato: %d", sysdata.mod_parry );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.mod_tumble < 1 || sysdata.mod_tumble > 5 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di tumble errato: %d", sysdata.mod_tumble );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.mud_port < 1024 || sysdata.mod_tumble > 32700 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: porta del mud errata: %d", sysdata.mud_port );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.purge_after_month <= 0 || sysdata.purge_after_month > 12 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: tempo in mesi di pulizia dei file save dei personaggio errato: %d", sysdata.purge_after_month );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.purge_min_hour <= 0 || sysdata.purge_min_hour > 64 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: tempo in ore minimo per i pg inattivi che vengono cancellati errato: %d", sysdata.purge_min_hour );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.purge_min_level <= 0 || sysdata.purge_min_level > LVL_LEGEND )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: livello minimo dei pg inattivi che vengono cancellati errato: %d", sysdata.purge_min_level );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.save_frequency <= 0 || sysdata.save_frequency > 60 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: tempo di salvataggio in minuti errato: %d", sysdata.save_frequency );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.save_level < 0 || sysdata.save_level > LVL_NEWBIE*2 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: livello minimo per salvare errato: %d", sysdata.save_level );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.stun_plr_vs_plr < 0 || sysdata.stun_plr_vs_plr > 100 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di stun contro i pg errato: %d", sysdata.stun_plr_vs_plr );
		exit( EXIT_FAILURE );
	}

	if ( sysdata.stun_regular < 0 || sysdata.stun_regular > 100 )
	{
		send_log( NULL, LOG_BUG, "fread_sysdata: modificatore di stun contro i mob errato: %d", sysdata.stun_regular );
		exit( EXIT_FAILURE );
	}
}


/*
 * Carica la configurazione sysdata
 */
void load_sysdata( void )
{
	init_sysdata( );

	fread_section( SYSDATA_FILE, "SYSDATA", fread_sysdata, TRUE );
}


/*
 * Salca le informazioni di sistema del mud su un file.
 */
void save_sysdata( void )
{
	MUD_FILE	*fp;

	fp = mud_fopen( "", SYSDATA_FILE, "w", TRUE );
	if ( !fp )
		return;

	fprintf( fp->file, "#SYSDATA\n" );
	fprintf( fp->file, "Copyover          %s\n",	fwrite_bool(sysdata.copyover) );
	fprintf( fp->file, "CopyoverReboot    %s\n",	fwrite_bool(sysdata.copyover_reboot) );
	if ( sysdata.copyover )	/* Salva il controllo da un boot ad un altro nel copyover */
		fprintf( fp->file, "Control           %d\n",sysdata.control );
	fprintf( fp->file, "CheckAdmHost      %s\n",	fwrite_bool(sysdata.check_adm_host) );
	fprintf( fp->file, "DenyNewPlayers    %s\n",	fwrite_bool(sysdata.deny_new_players) );
	fprintf( fp->file, "FreeStealing      %s\n",	fwrite_bool(sysdata.free_stealing) );
	fprintf( fp->file, "MascotteRestring  %s\n",	fwrite_bool(sysdata.mascotte_restring) );
	fprintf( fp->file, "MascotteSave      %s\n",	fwrite_bool(sysdata.mascotte_save) );
	fprintf( fp->file, "NameResolving     %s\n",	fwrite_bool(sysdata.name_resolving) );
	fprintf( fp->file, "OrderCommands     %s\n",	fwrite_bool(sysdata.order_commands) );
	fprintf( fp->file, "PkLoot            %s\n",	fwrite_bool(sysdata.pk_loot) );
#ifdef T2_QUESTION
	fprintf( fp->file, "QuestionCreation  %s\n",	fwrite_bool(sysdata.question_creation) );
#else
	fprintf( fp->file, "QuestionCreation  False\n" );
#endif
	fprintf( fp->file, "TrackTroughDoor   %s\n",	fwrite_bool(sysdata.track_trough_door) );
	fprintf( fp->file, "WizLock           %s\n",	fwrite_bool(sysdata.wiz_lock) );

	fprintf( fp->file, "BashNonTank       %d\n",	sysdata.bash_nontank );
	fprintf( fp->file, "BashPlrVsPlr      %d\n",	sysdata.bash_plr_vs_plr );
	fprintf( fp->file, "DamMobVsMob       %d\n",	sysdata.dam_mob_vs_mob );
	fprintf( fp->file, "DamMobVsPlr       %d\n",	sysdata.dam_mob_vs_plr );
	fprintf( fp->file, "DamPlrVsMob       %d\n",	sysdata.dam_plr_vs_mob );
	fprintf( fp->file, "DamPlrVsPlr       %d\n",	sysdata.dam_plr_vs_plr );
	fprintf( fp->file, "DeathHoleMult     %d\n",	sysdata.death_hole_mult );
	fprintf( fp->file, "GougeNonTank      %d\n",	sysdata.gouge_nontank );
	fprintf( fp->file, "GougePlrVsPlr     %d\n",	sysdata.gouge_plr_vs_plr );
	fprintf( fp->file, "LvLGroupDiff      %d\n",	sysdata.lvl_group_diff );
	fprintf( fp->file, "MaxSocketCon      %d\n",	sysdata.max_socket_con );
	fprintf( fp->file, "ModDodge          %d\n",	sysdata.mod_dodge );
	fprintf( fp->file, "ModParry          %d\n",	sysdata.mod_parry );
	fprintf( fp->file, "ModTumble         %d\n",	sysdata.mod_tumble );
	fprintf( fp->file, "MudPort           %d\n",	sysdata.mud_port );
	fprintf( fp->file, "PurgeAfterMonth   %d\n",	sysdata.purge_after_month );
	fprintf( fp->file, "PurgeMinHour      %d\n",	sysdata.purge_min_hour );
	fprintf( fp->file, "PurgeMinLevel     %d\n",	sysdata.purge_min_level );
	fprintf( fp->file, "SaveFreq          %d\n",	sysdata.save_frequency );
	fprintf( fp->file, "SaveLevel         %d\n",	sysdata.save_level );
	fprintf( fp->file, "StunPlrVsPlr      %d\n",	sysdata.stun_plr_vs_plr );
	fprintf( fp->file, "StunRegular       %d\n",	sysdata.stun_regular );
	fprintf( fp->file, "End\n\n" );

	fprintf( fp->file, "#END\n" );

	MUD_FCLOSE( fp );
}


/*
 * Comando amministratori per cambiare le opzioni dei sysdata.
 */
DO_RET do_sysdata( CHAR_DATA *ch, char *argument )
{
	char arg[MSL];

	set_char_color( AT_PLAIN, ch );

	if ( !VALID_STR(argument) )
	{
		char	cmd[MIL];

		strcpy( cmd, translate_command(ch, "sysdata") );
		pager_printf( ch, "&YSintassi&w: %s help    per maggiori informazioni\r\n", cmd );
		pager_printf( ch, "&YSintassi&w: %s salva   per salvare le impostazioni modificate\r\n\r\n", cmd );
        
		pager_printf( ch, "&wChecking Admin host: %s\r\n", (sysdata.check_adm_host)		? "&GSI" : "&rno" );
		pager_printf( ch, "&wCopyoverReboot:      %s\r\n", (sysdata.copyover_reboot)	? "&GSI" : "&rno" );
		pager_printf( ch, "&wDenyNewPlayers:      %s\r\n", (sysdata.deny_new_players)	? "&GSI" : "&rno" );
		pager_printf( ch, "&wFree stealing:       %s\r\n", (sysdata.free_stealing)		? "&GSI" : "&rno" );
		pager_printf( ch, "&wPkill looting:       %s\r\n", (sysdata.pk_loot)			? "&GSI" : "&rno" );
		pager_printf( ch, "&wOrder commands:      %s\r\n", (sysdata.order_commands)		? "&GSI" : "&rno" );
#ifdef T2_QUESTION
		pager_printf( ch, "&wQuestion Creation:   %s\r\n", (sysdata.question_creation)	? "&GSI" : "&rno" );
#endif
		pager_printf( ch, "&wMascotte Restring:   %s\r\n", (sysdata.mascotte_restring)	? "&GSI" : "&rno" );
		pager_printf( ch, "&wMascotte Saving:     %s\r\n", (sysdata.mascotte_save)		? "&GSI" : "&rno" );
		pager_printf( ch, "&wTrackTroughDoor:     %s\r\n", (sysdata.track_trough_door)	? "&GSI" : "&rno" );
		pager_printf( ch, "&wWizLock:             %s\r\n", (sysdata.wiz_lock)			? "&GSI" : "&rno" );
		pager_printf( ch, "&wNameResolving:       %s\r\n", (sysdata.name_resolving)		? "&GSI" : "&rno" );

		send_to_pager( ch, "&WDifese:\r\n" );
		pager_printf( ch, "&wModDodge:            &W%d\r\n", sysdata.mod_dodge );
		pager_printf( ch, "&wModParry:            &W%d\r\n", sysdata.mod_parry );
		pager_printf( ch, "&wModTumble:           &W%d\r\n", sysdata.mod_tumble );

		send_to_pager( ch, "&WPenalità a:\r\n" );
		pager_printf( ch, "&wNon-tank bash:       &W%-3d\r\n", sysdata.bash_nontank );
		pager_printf( ch, "&wNon-tank gouge:      &W%-3d\r\n", sysdata.gouge_nontank );
		pager_printf( ch, "&wBash  plr vs. plr:   &W%-3d\r\n", sysdata.bash_plr_vs_plr );
		pager_printf( ch, "&wGouge plr vs. plr:   &W%-3d\r\n", sysdata.gouge_plr_vs_plr );
		pager_printf( ch, "&wStun  plr vs. plr:   &W%-3d\r\n", sysdata.stun_plr_vs_plr );
		pager_printf( ch, "&wRegular stun chance: &W%-3d\r\n", sysdata.stun_regular );

		send_to_pager( ch, "&WPercentuali di danno:\r\n" );
		pager_printf( ch, "&wPlr vs. plr:         &W%-3d\r\n", sysdata.dam_plr_vs_plr );
		pager_printf( ch, "&wPlr vs. mob:         &W%-3d\r\n", sysdata.dam_plr_vs_mob );
		pager_printf( ch, "&wMob vs. plr:         &W%-3d\r\n", sysdata.dam_mob_vs_plr );
		pager_printf( ch, "&wMob vs. mob:         &W%-3d\r\n", sysdata.dam_mob_vs_mob );

		send_to_pager( ch, "&WPulizia file save:\r\n" );
		pager_printf( ch, "&wPurgeAfterMonth:     &W%-3d\r\n", sysdata.purge_after_month );
		pager_printf( ch, "&wPurgeMinHour:        &W%-3d\r\n", sysdata.purge_min_hour );
		pager_printf( ch, "&wPurgeMinLevel:       &W%-3d\r\n", sysdata.purge_min_level );

		send_to_pager( ch, "&WAltro:\r\n" );
		pager_printf( ch, "&wDeathHoleMult:       &W%d\r\n", sysdata.death_hole_mult );
		pager_printf( ch, "&wMaxSocketCon:        &W%d\r\n", sysdata.max_socket_con );
		pager_printf( ch, "&wPorta Mud:           &W%d\r\n", sysdata.mud_port );
		pager_printf( ch, "&wLvlGroupDiff:        &W%d\r\n", sysdata.lvl_group_diff );
		pager_printf( ch, "&wAutosave frequency:  &W%d minutes\r\n", sysdata.save_frequency );
		pager_printf( ch, "&wLivello min di save: &W%d\r\n", sysdata.save_level );

		return;
	}

	argument = one_argument( argument, arg );
	smash_tilde( argument );

	if ( is_name(arg, "aiuto help") )
		send_command( ch, "help CONTROLS", CO );
	else if ( is_name(arg, "salva save") )
		save_sysdata( );
	else if ( !str_cmp(arg, "checkadmhost") )
	{
		if ( sysdata.check_adm_host )
		{
			send_to_char( ch, "Controllo host admin disattivato.\r\n" );
			sysdata.check_adm_host = FALSE;
		}
		else
		{
			send_to_char( ch, "Controllo host admin attivato.\r\n" );
			sysdata.check_adm_host = TRUE;
		}
	}
	else if ( !str_cmp(arg, "denynewplayers") )
	{
		if ( sysdata.deny_new_players )
		{
			send_to_char( ch, "Si potranno creare nuovi giocatori.\r\n" );
			sysdata.deny_new_players = FALSE;
		}
		else
		{
			send_to_char( ch, "Non si potranno creare nuovi giocatori.\r\n" );
			sysdata.deny_new_players = TRUE;
		}
	}
	else if ( !str_cmp(arg, "freestealing") )
	{
		if ( sysdata.free_stealing )
		{
			send_to_char( ch, "Stealing libero disattivato.\r\n" );
			sysdata.free_stealing = FALSE;
		}
		else
		{
			send_to_char( ch, "Stealing libero attivato.\r\n" );
			sysdata.free_stealing = TRUE;
		}
	}
	else if ( !str_cmp(arg, "ordercommands") )
	{
		if ( sysdata.order_commands )
		{
			send_to_char( ch, "Ordinamento di comandi riguardo l'utilizzo disattivato.\r\n" );
			sysdata.pk_loot = FALSE;
		}
		else
		{
			send_to_char( ch, "Ordinamento di comandi riguardo l'utilizzo attivato.\r\n" );
			sysdata.pk_loot = TRUE;
		}
	}
	else if ( !str_cmp(arg, "pk_loot") )
	{
		if ( sysdata.pk_loot )
		{
			send_to_char( ch, "Saccheggio per i pk disattivato.\r\n" );
			sysdata.pk_loot = FALSE;
		}
		else
		{
			send_to_char( ch, "Saccheggio per i pk attivato.\r\n" );
			sysdata.pk_loot = TRUE;
		}
	}
	else if ( !str_cmp(arg, "copyoverreboot") )
	{
		if ( sysdata.copyover_reboot )
		{
			send_to_char( ch, "Riavvio di reboot con il copyover disattivato.\r\n" );
			sysdata.copyover_reboot = FALSE;
		}
		else
		{
			send_to_char( ch, "Riavvio di reboot con il copyover attivato.\r\n" );
			sysdata.copyover_reboot = TRUE;
		}
	}
#ifdef T2_QUESTION
	else if ( !str_cmp(arg, "questioncreation") )
	{
		if ( sysdata.question_creation )
		{
			send_to_char( ch, "Creazione dei pg tramite domande disattivato.\r\n" );
			sysdata.question_creation = FALSE;
		}
		else
		{
			send_to_char( ch, "Creazione dei pg tramite domande attivato.\r\n" );
			sysdata.question_creation = TRUE;
		}
	}
#endif
	else if ( !str_cmp(arg, "mascotterestring") )
	{
		if ( sysdata.mascotte_restring )
		{
			send_to_char( ch, "Restring mascotte disattivato.\r\n" );
			sysdata.mascotte_restring = FALSE;
		}
		else
		{
			send_to_char( ch, "Restring mascotte attivato.\r\n" );
			sysdata.mascotte_restring = TRUE;
		}
	}
	else if ( !str_cmp(arg, "mascottesave") )
	{
		if ( sysdata.mascotte_save )
		{
			send_to_char( ch, "Salvataggio mascotte disattivato.\r\n" );
			sysdata.mascotte_save = FALSE;
		}
		else
		{
			send_to_char( ch, "Salvataggio mascotte attivato.\r\n" );
			sysdata.mascotte_save = TRUE;
		}
	}
	else if ( !str_cmp(arg, "tracktroughdoor") )
	{
		if ( sysdata.track_trough_door )
		{
			send_to_char( ch, "Tracking attraverso le porte chiuse disattivato.\r\n" );
			sysdata.track_trough_door = FALSE;
		}
		else
		{
			send_to_char( ch, "Tracking attraverso le porte chiuse attivato.\r\n" );
			sysdata.track_trough_door = TRUE;
		}
	}
	else if ( !str_cmp(arg, "wizlock") )
	{
		if ( sysdata.wiz_lock )
		{
			send_to_char( ch, "Wizlock disattivato, entrano tutti.\r\n" );
			sysdata.wiz_lock = FALSE;
		}
		else
		{
			send_to_char( ch, "Wizlock attivato, entrano solo gli admin.\r\n" );
			sysdata.wiz_lock = TRUE;
		}
	}
	else if ( !str_cmp(arg, "nameresolving") )
	{
		if ( sysdata.name_resolving )
		{
			send_to_char( ch, "NameResolving disattivato.\r\n" );
			sysdata.name_resolving = FALSE;
		}
		else
		{
			send_to_char( ch, "NameResolving attivato.\r\n" );
			sysdata.name_resolving = TRUE;
		}
	}
	else if ( is_number(argument) )
	{
		int	 temp = (int) atoi( argument );

		/* (TT) controllare che non si abbisogni di un controllo sul massimale di variabile temp passato qui sotto e oltre */

		if		( !str_cmp(arg, "moddodge"		 ) )	sysdata.mod_dodge =			(temp > 0)  ?  temp  :  1;
		else if ( !str_cmp(arg, "modparry"		 ) )	sysdata.mod_parry =			(temp > 0)  ?  temp  :  1;
		else if ( !str_cmp(arg, "modtumble"		 ) )	sysdata.mod_tumble =		(temp > 0)  ?  temp  :  1;
		else if ( !str_cmp(arg, "non-tankbash"	 ) )	sysdata.bash_nontank =		temp;
		else if ( !str_cmp(arg, "non-tankgouge"	 ) )	sysdata.gouge_nontank =		temp;
		else if ( !str_cmp(arg, "bashpvp"		 ) )	sysdata.bash_plr_vs_plr =	temp;
		else if ( !str_cmp(arg, "gougepvp"		 ) )	sysdata.gouge_plr_vs_plr =	temp;
		else if ( !str_cmp(arg, "stunpvp"		 ) )	sysdata.stun_plr_vs_plr =	temp;
		else if ( !str_cmp(arg, "stun"			 ) )	sysdata.stun_regular =		temp;
		else if ( !str_cmp(arg, "dampvp"		 ) )	sysdata.dam_plr_vs_plr =	temp;
		else if ( !str_cmp(arg, "dampvm"		 ) )	sysdata.dam_plr_vs_mob =	temp;
		else if ( !str_cmp(arg, "dammvp"		 ) )	sysdata.dam_mob_vs_plr =	temp;
		else if ( !str_cmp(arg, "dammvm"		 ) )	sysdata.dam_mob_vs_mob =	temp;
		else if ( !str_cmp(arg, "deathholemult"	 ) )	sysdata.death_hole_mult =	temp;
		else if ( !str_cmp(arg, "purgeaftermonth") )	sysdata.purge_after_month =	temp;
		else if ( !str_cmp(arg, "purgeminhour"	 ) )	sysdata.purge_min_hour =	temp;
		else if ( !str_cmp(arg, "purgeminlevel"	 ) )	sysdata.purge_min_level =	temp;
		else if ( !str_cmp(arg, "savefrequency"	 ) )	sysdata.save_frequency =	temp;
		else if ( !str_cmp(arg, "savelevel"		 ) )	sysdata.save_level =		temp;
		else if ( !str_cmp(arg, "lvlgroupdiff"	 ) )	sysdata.lvl_group_diff =	temp;
		else if ( !str_cmp(arg, "maxsocketcon"	 ) )	sysdata.max_socket_con =	temp;
		else if ( !str_cmp(arg, "mudport"		 ) )	sysdata.mud_port =			temp;
		else
		{
			send_to_char( ch, "Argomento non valido.\r\n" );
			return;
		}
	}
	else
	{
		send_to_char( ch, "Argomento non valido.\r\n" );
		return;
	}

	send_to_char( ch, "Ok.\r\n" );
}
