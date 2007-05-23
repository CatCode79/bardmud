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


#if T2_WEB

#include "mud.h"
#include "archetype.h"
#include "calendar.h"
#include "color.h"
#include "command.h"
#include "db.h"
#include "dream.h"
#include "editor.h"
#include "fread.h"
#include "help.h"
#include "herb.h"
#include "lyric.h"
#include "nanny.h"
#include "shop.h"
#include "web.h"


/*
 * Definizioni
 */
#define		WEB_DIR		"../web/"		/* Directory contenente i file per il sito web */


/*
 * Tipo di struttura di una statistica
 */
typedef	struct pg_stat_data		PG_STAT_DATA;
typedef	struct mob_stat_data	MOB_STAT_DATA;

/*
 * Struttura delle stastiche dei pg
 */
struct pg_stat_data
{
	PG_STAT_DATA	*prev;				/* Ordinata alfabeticamente */
	PG_STAT_DATA	*next;
	PG_STAT_DATA	*prev_highscore;	/* Ordinata dall'higscore + grande al minore */
	PG_STAT_DATA	*next_highscore;
	PG_STAT_DATA	*prev_time;			/* Ordinata dalle ore di gioco maggiori a quelle minori */
	PG_STAT_DATA	*next_time;
	PG_STAT_DATA	*prev_report;		/* Ordinata dalle segnalazioni maggiori a quelle minori */
	PG_STAT_DATA	*next_report;
	CHAR_DATA		*ch;
};


/*
 * Struttura utilizzata per le tabelle con le statistiche dei mob
 */
struct mob_stat_data
{
	int		number;
	int		ac;
	int		hitnodice;
	int		hitsizedice;
	int		hitplus;
	int		damnodice;
	int		damsizedice;
	int		damplus;
};


/*
 * Variabili locali
 */
PG_STAT_DATA	*first_stat				= NULL;
PG_STAT_DATA	*last_stat				= NULL;
PG_STAT_DATA	*first_stat_highscore	= NULL;
PG_STAT_DATA	*last_stat_highscore	= NULL;
PG_STAT_DATA	*first_stat_time		= NULL;
PG_STAT_DATA	*last_stat_time			= NULL;
PG_STAT_DATA	*first_stat_report		= NULL;
PG_STAT_DATA	*last_stat_report		= NULL;
MOB_STAT_DATA	 table_def_stat_lvl[LVL_LEGEND+1];
MOB_STAT_DATA	 table_def_stat_race[MAX_RACE];


/*
 * Converte i colori da codici smaug a codici esadecimali per il Web
 * (BB) attenzione, se txt passato non finisce con &w non termina la stringa con </font>
 * (FF) per gestire correttamente l'apertura e la chiusura dei colori ci vuole una variabile static in questa funzione
 */
char *web_colorcnv( const char *txt )
{
	const char	*point;
	static char	 buf[MSL*2];
	char		*pbuf = buf;

	if ( !VALID_STR(txt) )
	{
		send_log( NULL, LOG_BUG, "web_colorcnv: txt è NULL" );
		return "";
	}

	strcpy( buf, txt );
	return strip_color( buf );


	if ( !VALID_STR(txt) )
		return NULL;

	/* Se inizia con &w lo toglie */
	if ( txt[0] == '&' && txt[1] == 'w' )
		txt = txt+2;						/* (TT) */

	for ( point = txt;  *point;  point++ )
	{
		if ( *point == '&' )
		{
			int		x;
			bool	found = FALSE;

			for ( x = 0;  x < MAX_COLOR;  x++ )
			{
				if ( *(point+1) == table_color[x].clr )
				{
					char	 color[MIL];
					char	*pcolor = color;

					strcpy( color, table_color[x].html );
					while ( *pcolor != '\0' )
						*pbuf++ = *pcolor++;

					point++;	/* salta la & */
					found = TRUE;
					break;
				}
			}

			if ( !found )
				*pbuf = *point;

			continue;
		}
		*pbuf++ = *point;
	}
	*pbuf = '\0';

	/* (bb) (FF) Se non finisce con &w aggiunge </font> */

	return buf;
}


/*
 * Calcola l'highscore di un pg
 */
int compute_highscore( CHAR_DATA *ch )
{
	int		score = 0;
	int		x;

	/* Hanno delle moltiplicazioni e divisioni non intere per evitare
	 *	che i pg a livelli bassi intuiscano i valori base e le operazioni
	 *	così da ricavare i valori anche per i livelli alti */
	score += ch->level * 3.0;		/* niente get_level() qui */
	score += get_hours_played( ch ) * 4.8;
	score += ch->pg->glory * 0.04;
	score += (ch->hitroll + ch->damroll) * 2.9;
	score += ch->pg->practice * 3.2;

	for ( x = 0;  x < MAX_POINTS;  x++ )
		score += ch->points[x] / 3.8;

	for ( x = 0;  x < MAX_ATTR;  x++ )
		score += ch->attr_perm[x] / 4.7;

	for ( x = 0;  x < top_sn;  x++ )
	{
		if ( !VALID_STR(table_skill[x]->name) )							continue;
		if ( ch->pg->learned_skell[x] <= 0 )							continue;
		if ( get_level(ch) < table_skill[x]->skill_level[ch->class] )	continue;

		score += ch->pg->learned_skell[x] / 7.6;
	}

	/* (FF) Aggiungere forti malus per quelli che sono sotto punizione o altre situazioni */
	/* (FF) Aggiungere una variabile di punteggio da 1 a 10 per voto di rpg, darà una percentuale in più dell'highscore sul totale */

	return score;
}


/*
 * Tipologie di tabelle html create per le statistiche
 */
#define TABLE_TIME			0
#define TABLE_HIGHSCORE		1
#define TABLE_REPORT		2


void sort_stat_top( PG_STAT_DATA *pStat, const int type )
{
	PG_STAT_DATA	*stat;
	PG_STAT_DATA	*first_sort = NULL;
	PG_STAT_DATA	*last_sort	= NULL;

	if ( !pStat )
	{
		send_log( NULL, LOG_BUG, "sort_stat_top: pStat è NULL" );
		return;
	}

	switch ( type )
	{
	  default:
		send_log( NULL, LOG_BUG, "sort_stat_top: tipo di tabella errata: %d", type );
		exit( EXIT_FAILURE );
		break;

	  case TABLE_TIME:
		if ( get_hours_played(pStat->ch) <= 0 )
			return;
		first_sort = first_stat_time;
		last_sort  = last_stat_time;
		break;

	  case TABLE_HIGHSCORE:
		first_sort = first_stat_highscore;
		last_sort  = last_stat_highscore;
		break;

	  case TABLE_REPORT:
		if ( pStat->ch->pg->report <= 0 )
			return;
		first_sort = first_stat_report;
		last_sort  = last_stat_report;
		break;
	}

	/* Ordina la lista delle statistiche in ordine dal più alto al minore */
	for ( stat = first_sort;  stat;  stat = (type == TABLE_TIME)	  ? stat->next_time :
											(type == TABLE_HIGHSCORE) ?	stat->next_highscore :
																		stat->next_report )
	{
		if ( type == TABLE_TIME )
		{
			if ( get_hours_played(pStat->ch) > get_hours_played(stat->ch) )
			{
				INSERT( pStat, stat, first_sort, next_time, prev_time );
				break;
			}
		}

		if ( type == TABLE_HIGHSCORE )
		{
			if ( compute_highscore(pStat->ch) > compute_highscore(stat->ch) )	/* (FF) non è il metodo più veloce del mondo per fare questo, magari metterci mano e migliorarlo */
			{
				INSERT( pStat, stat, first_sort, next_highscore, prev_highscore );
				break;
			}
		}

		if ( type == TABLE_REPORT )
		{
			if ( pStat->ch->pg->report > stat->ch->pg->report )
			{
				INSERT( pStat, stat, first_sort, next_report, prev_report );
				break;
			}
		}
	}

	if ( !stat )
	{
		switch ( type )
		{
		  case TABLE_TIME:	  	LINK( pStat, first_sort, last_sort, next_time,		 prev_time );		break;
		  case TABLE_HIGHSCORE:	LINK( pStat, first_sort, last_sort, next_highscore, prev_highscore );	break;
		  case TABLE_REPORT:	LINK( pStat, first_sort, last_sort, next_report,	 prev_report );	break;
		}
	}

	switch ( type )
	{
	  case TABLE_TIME:			first_stat_time		 = first_sort;	last_stat_time		 = last_sort;	break;
	  case TABLE_HIGHSCORE:		first_stat_highscore = first_sort;	last_stat_highscore  = last_sort;	break;
	  case TABLE_REPORT:		first_stat_report	 = first_sort;	last_stat_report	 = last_sort;	break;
	}
}


#define MAX_BEST	10		/* Massimo di giocatori nella toplist */

/*
 * Crea la tabella web sulle statistiche sui 10 giocatori con
 *	higscore, ore e segnalazioni maggiori
 */
char *create_table_stat_top( const int type )
{
	PG_STAT_DATA	*stat;
	PG_STAT_DATA	*first_sort = NULL;
	static char	 buf[MSL*2];
	int			 num;

	/* Crea il file che conterrà la tabella html */
	switch ( type )
	{
	  default:
		send_log( NULL, LOG_BUG, "create_table: tipo di tabella errata: %d", type );
		exit( EXIT_FAILURE );
		break;
	  case TABLE_TIME:			first_sort = first_stat_time;		break;
	  case TABLE_HIGHSCORE:		first_sort = first_stat_highscore;	break;
	  case TABLE_REPORT:		first_sort = first_stat_report;		break;
	}

	/* Crea la tabella in html con la lista ordinata in alfabetico */
	switch ( type )
	{
	  case TABLE_TIME:			sprintf( buf, "<span>Ore reali giocate su T2:</span>\n" );	break;
	  case TABLE_HIGHSCORE:		sprintf( buf, "<span>L'Highscore è un calcolo su varie caratteristiche dell'avventuriero:</span>\n" );	break;
	  case TABLE_REPORT:		sprintf( buf, "<span>Numero di segnalazioni di bachi, typo, idee e commenti:</span>\n" );	break;
	}
	sprintf( buf+strlen(buf), WEB_TABLE );
	sprintf( buf+strlen(buf), "<thead><tr>\n" );
	sprintf( buf+strlen(buf), "<th class=\"BASE-GGIALLO\">Nome</th>\n" );
	switch ( type )
	{
	  case TABLE_TIME:			sprintf( buf+strlen(buf), "<th class=\"BASE-GGIALLO\">Ore giocate</th>\n" );	break;
	  case TABLE_HIGHSCORE:		sprintf( buf+strlen(buf), "<th class=\"BASE-GGIALLO\">Highscore</th>\n" );		break;
	  case TABLE_REPORT:		sprintf( buf+strlen(buf), "<th class=\"BASE-GGIALLO\">Segnalazioni</th>\n" );	break;
	}
	sprintf( buf+strlen(buf), "</tr></thead><tbody>\n" );

	num = 0;
	for ( stat = first_sort;  stat;  stat = (type == TABLE_TIME)	  ? stat->next_time :
											(type == TABLE_HIGHSCORE) ?	stat->next_highscore :
																		stat->next_report )
	{
		if ( num >= MAX_BEST )
			break;

		sprintf( buf+strlen(buf), "<tr><td>%s</td>", stat->ch->name );
		switch ( type )
		{
		  case TABLE_TIME:			sprintf( buf+strlen(buf), "<td>%d</td>", get_hours_played(stat->ch) );	break;
		  case TABLE_HIGHSCORE:		sprintf( buf+strlen(buf), "<td>%d</td>", compute_highscore(stat->ch) );	break;
		  case TABLE_REPORT:		sprintf( buf+strlen(buf), "<td>%d</td>", stat->ch->pg->report );		break;
		}
		sprintf( buf+strlen(buf), "</tr>\n" );

		num++;
	}

	sprintf( buf+strlen(buf), "</tbody></table>\n" );

	return buf;
}


/*
 * Ordina alfabeticamente la lista delle statistiche
 */
void sort_stat_by_name( PG_STAT_DATA *pStat )
{
	PG_STAT_DATA *stat = NULL;

	if ( !pStat )
	{
		send_log( NULL, LOG_BUG, "sort_stat_by_name: pStat è NULL" );
		return;
	}

	for ( stat = first_stat;  stat;  stat = stat->next )
	{
		if ( strcmp(pStat->ch->name, stat->ch->name) <= 0 )
		{
			INSERT( pStat, stat, first_stat, next, prev );
			break;
		}
	}

	if ( !stat )
		LINK( pStat, first_stat, last_stat, next, prev );
}


/*
 * Crea la tabella web sulle statistiche in ordine alfabetico
 */
void create_html_stat_all( void )
{
	CHAR_DATA		*ch;
	PG_STAT_DATA	*stat;
	MUD_FILE		*fp;
	int				 total_played = 0;

	for ( ch = first_offline;  ch;  ch = ch->next )
	{
		CREATE( stat, PG_STAT_DATA, 1 );
		stat->ch = ch;

		/* Ordina la lista delle statistiche in alfabetico */
		sort_stat_by_name( stat );

		/* Non crea le liste dei top per gli admin */
		if ( IS_ADMIN(ch) )
			continue;

		/* Ordina le liste dei top */
		sort_stat_top( stat, TABLE_HIGHSCORE );
		sort_stat_top( stat, TABLE_TIME );
		sort_stat_top( stat, TABLE_REPORT );
	}

	fp = mud_fopen( WEB_DIR, "stat_alfa.html", "w", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	/* Crea una tabella con tre colonne che contiene le tre tabelle di top stats */
	fputs  ( WEB_HEADER, fp->file );
	fputs  ( "<table border=\"0\" cellspacing=\"0\">\n", fp->file );
	fputs  ( "<thead><tr>\n", fp->file );
	fprintf( fp->file, "<th width=\"33%%\">%s</th>\n", create_table_stat_top(TABLE_TIME) );
	fprintf( fp->file, "<th width=\"33%%\">%s</th>\n", create_table_stat_top(TABLE_HIGHSCORE) );
	fprintf( fp->file, "<th width=\"33%%\">%s</th>\n", create_table_stat_top(TABLE_REPORT) );
	fputs  ( "</tr></thead></table><br><br>\n", fp->file );

	/* Crea la tabella in html con la lista ordinata in alfabetico */
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th width=\"20%%\" class=\"BASE-GGIALLO\">Nome</th>\n" );
	fprintf( fp->file, "<th width=\"16%%\" class=\"BASE-GGIALLO\">Highscore</th>\n" );
	fprintf( fp->file, "<th width=\"16%%\" class=\"BASE-GGIALLO\">Ore giocate</th>\n" );
	fprintf( fp->file, "<th width=\"16%%\" class=\"BASE-GGIALLO\">Segnalazioni</th>\n" );
	fprintf( fp->file, "<th width=\"16%%\" class=\"BASE-GGIALLO\">Razza</th>\n" );
	fprintf( fp->file, "<th width=\"16%%\" class=\"BASE-GGIALLO\">Sesso</th>\n" );
	fprintf( fp->file, "</tr></thead><tbody>\n" );

	for ( stat = first_stat;  stat;  stat = stat->next )
	{
		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", stat->ch->name );
		fprintf( fp->file, "<td>%d</td>", compute_highscore(stat->ch) );
		fprintf( fp->file, "<td>%d</td>", get_hours_played(stat->ch) );
		fprintf( fp->file, "<td>%d</td>", stat->ch->pg->report );
		if ( stat->ch->sex == SEX_FEMALE )
		{
			fprintf( fp->file, "<td>%s</td>", table_race[stat->ch->race]->name_fema );
			fprintf( fp->file, "<td>%s</td>", "Femminile" );
		}
		else
		{
			fprintf( fp->file, "<td>%s</td>", table_race[stat->ch->race]->name_male );
			fprintf( fp->file, "<td>%s</td>", "Maschile" );
		}
		fprintf( fp->file, "</tr>\n" );
	}

	fprintf( fp->file, "</tbody></table><br><br>\n" );

	for ( stat = first_stat;  stat;  stat = stat->next )
		total_played += get_hours_played( stat->ch );
	fprintf( fp->file, "<span>%s è stato giocato per un totale di %d ore.</span>", web_colorcnv(MUD_NAME), total_played );

	MUD_FCLOSE( fp );

	/* Libera dalla memoria tutte le stats, visto che non vengono più utilizzate dopo */
	for ( stat = first_stat;  stat;  stat = first_stat )
	{
		UNLINK( stat, first_stat, last_stat, next, prev );
		stat->ch = NULL;	/* il puntatore a ch è un offline, non lo libera dalla memoria quindi */
		DISPOSE( stat );
	}
}


/*
 * Crea una tabella in html con le informazioni del calendario del Mud
 */
void create_html_calendar( void )
{
	MUD_FILE	*fp;
	char		 clr_calendar[MSL];
	int			 x;

	/* Crea il file che conterrà la tabella html */
	fp = mud_fopen( WEB_DIR, "calendar.html", "w", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	fputs  ( WEB_HEADER, fp->file );

	/* Stampa la data attuale */
	strcpy( clr_calendar, get_calendar_string() );
	strcpy( clr_calendar, strip_char(clr_calendar, '\r') );
	fprintf( fp->file, "<span class=\"BASE-GROSSO\">%s</span><br><br>\n", web_colorcnv(clr_calendar) );

	/* Prepara la tabella invisibile che contiene le due tabelle dei giorni e dei mesi */
	fputs  ( "<table border=\"0\" cellspacing=\"0\">\n", fp->file );
	fputs  ( "<thead><tr>\n", fp->file );
	fprintf( fp->file, "<th width=\"50%%\">" );

	/* Crea la tabella dei giorni */
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Nomi dei giorni</th></tr></thead>" );
	fprintf( fp->file, "<tbody>\n" );
	for ( x = 0 ;  x < DAYS_IN_WEEK;  x++ )
	{
		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>Giorno %s</td>", web_colorcnv(day_name[x]) );
		fprintf( fp->file, "</tr>\n" );
	}
	fprintf( fp->file, "</tbody></table><br><br>\n\n" );

	fputs  ( "</th>\n", fp->file );	/* chiude la colonna che contiene la tabella dei giorni */
	fprintf( fp->file, "<th width=\"50%%\">" );	/* apre la colonna che conterrà la tabella dei mesi */

	/* Crea la tabella dei mesi */
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Nomi dei mesi</th></tr></thead><tbody>\n" );
	for ( x = 0 ;  x < MONTHS_IN_YEAR;  x++ )
		fprintf( fp->file, "<tr><td>Mese %s</td></tr>\n", web_colorcnv(month_name[x]) );

	fprintf( fp->file, "</tbody></table>\n" );

	fputs  ( "</th>\n", fp->file );	/* chiude la colonna che contiene la tabella dei mesi */
	fputs  ( "</tr></thead></table>\n", fp->file );	/* chiude la tabella che contiene la tabella dei giorni e dei mesi */

	/* (FF) Futuro sistema anniversari da inserire qui */

	MUD_FCLOSE( fp );
}


/*
 * Ritorna una stringa con i segni a seconda del bonus o maluse
 */
char *get_sign_alias( const int bonus )
{
	if ( bonus < -20 || bonus > 20 )
	{
		send_log( NULL, LOG_BUG, "get_sign_alias: bonus pasato errato: %d", bonus );
		return NULL;
	}

	if		( bonus > 15  )	return "&G++++";
	else if ( bonus > 10  )	return "&G+++";
	else if ( bonus >  5  )	return "&g++";
	else if ( bonus >  0  )	return "&g+";
	else if ( bonus == 0  )	return "=";
	else if ( bonus > -6  )	return "&r-";
	else if ( bonus > -11 )	return "&r--";
	else if ( bonus > -16 )	return "&R---";
	else					return "&R----";
}


/*
 * Crea la tabella html con le informazioni di censimento dei pg, classi e razze
 */
void create_html_census( void )
{
	MUD_FILE	*fp;
	int			 x;
	int			 y;

	/* Crea il file che conterrà la tabella html */
	fp = mud_fopen( WEB_DIR, "census.html", "w", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	/* Crea la tabella delle razze utilizzate dai pg */
	fputs( WEB_HEADER, fp->file );

	/* Prepara la tabella invisibile che contiene le due tabelle delle razze e delle classi */
	fputs  ( "<table border=\"0\" cellspacing=\"0\">\n", fp->file );
	fputs  ( "<thead><tr>\n", fp->file );
	fprintf( fp->file, "<th width=\"50%%\">" );

	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Censimento delle razze utilizzate</th>\n" );
	fprintf( fp->file, "</tr></thead><tbody>\n" );
	for ( x = 0 ;  x < MAX_RACE;  x++ )
	{
		CHAR_DATA	*ch;
		char		 clr_race[MIL];
		int			 used = 0;

		if ( !HAS_BIT(table_race[x]->flags, RACEFLAG_PLAYABLE) )
			continue;

		sprintf( clr_race, "&%c%s", get_clr(table_race[x]->color->cat), table_race[x]->name_male );

		for ( ch = first_offline;  ch;  ch = ch->next )
		{
			if ( ch->race == x )
				used++;
		}

		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", web_colorcnv(clr_race) );
		fprintf( fp->file, "<td>%d</td>", used );
		fprintf( fp->file, "</tr>\n" );
	}
	fprintf( fp->file, "</tbody></table><br><br>\n\n" );

	fputs  ( "</th>\n", fp->file );	/* chiude la colonna che contiene la tabella delle razze */
	fprintf( fp->file, "<th width=\"50%%\">" );	/* apre la colonna che conterrà la tabella delle classi */


	/* Crea la tabella delle classi utilizzate dai pg */
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Censimento delle classi utilizzate</th>\n" );
	fprintf( fp->file, "</tr></thead><tbody>\n" );
	for ( x = 0 ;  x < MAX_CLASS;  x++ )
	{
		CHAR_DATA	*ch;
		char		 clr_class[MIL];
		int			 used = 0;

		if ( !HAS_BIT(table_race[x]->flags, RACEFLAG_PLAYABLE) )
			continue;

		sprintf( clr_class, "&%c%s", get_clr(table_class[x]->color->cat), table_class[x]->name_male );

		for ( ch = first_offline;  ch;  ch = ch->next )
		{
			if ( IS_MOB(ch) )
				continue;

			if ( ch->class == x )
				used++;
		}

		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", web_colorcnv(clr_class) );
		fprintf( fp->file, "<td>%d</td>", used );
		fprintf( fp->file, "</tr>\n" );
	}
	fprintf( fp->file, "</tbody></table>\n" );

	fputs  ( "</th>\n", fp->file );	/* chiude la colonna che contiene la tabella delle razze */
	fputs  ( "</tr></thead></table>\n", fp->file );	/* chiude la tabella che contiene la tabella dei giorni e dei mesi */


	/* Prepara la tabella che elenca i punti forti e deboli delle razze riguardo agli attributi */
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<span class=\"BASE-GROSSO\">Bonus e malus degli attributi razziali:</th>\n" );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Razza</th>\n" );
	for ( y = 0 ;  y < MAX_ATTR;  y++ )
		fprintf( fp->file, "<th class=\"BASE-GGIALLO\">%s</th>\n", code_name(NULL, y, CODE_ATTR) );
	fprintf( fp->file, "</tr></thead><tbody>\n" );
	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		char	clr_race[MIL];

		if ( !HAS_BIT(table_race[x]->flags, RACEFLAG_PLAYABLE) )
			continue;

		sprintf( clr_race, "&%c%s", get_clr(table_race[x]->color->cat), table_race[x]->name_male );

		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", web_colorcnv(clr_race) );
		for ( y = 0;  y < MAX_ATTR;  y++ )
		{
			char	sign[MIL];

			strcpy( sign, get_sign_alias(table_race[x]->attr_plus[y]) );
			fprintf( fp->file, "<td>%s</td>", web_colorcnv(sign) );
		}
		fprintf( fp->file, "</tr>\n" );
	}
	fprintf( fp->file, "</tbody></table><br><br>\n\n" );


	/* Prepara la tabella che elenca i punti forti e deboli delle razze riguardo ai sensi */
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<span class=\"BASE-GROSSO\">Bonus e malus dei sensi razziali:</th>\n" );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Razza</th>\n" );
	for ( y = 0 ;  y < MAX_SENSE;  y++ )
		fprintf( fp->file, "<th class=\"BASE-GGIALLO\">%s</th>\n", code_name(NULL, y, CODE_SENSE) );
	fprintf( fp->file, "</tr></thead><tbody>\n" );
	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		char	clr_race[MIL];

		if ( !HAS_BIT(table_race[x]->flags, RACEFLAG_PLAYABLE) )
			continue;

		sprintf( clr_race, "&%c%s", get_clr(table_race[x]->color->cat), table_race[x]->name_male );

		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", web_colorcnv(clr_race) );
		for ( y = 0;  y < MAX_SENSE;  y++ )
		{
			char	sign[MIL];

			strcpy( sign, get_sign_alias(table_race[x]->sense_plus[y]) );
			fprintf( fp->file, "<td>%s</td>", web_colorcnv(sign) );
		}
		fprintf( fp->file, "</tr>\n" );
	}
	fprintf( fp->file, "</tbody></table><br><br>\n\n" );

	MUD_FCLOSE( fp );
}


/*
 * Tabella con le statitiche del Mud
 * E' praticamente una versione ridotta del comando do_memory in html
 */
void create_html_memory( void )
{
	MUD_FILE	*fp;

	/* Crea il file che conterrà la tabella html */
	fp = mud_fopen( WEB_DIR, "memory.html", "w", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	/* Versione del codice */
	fputs( WEB_HEADER, fp->file );
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Statistiche tecniche di T2 (%s %s.%s)</th>\n", web_colorcnv(MUD_CODE_NAME), MUD_VERSION_MAJOR, MUD_VERSION_MINOR );
	fprintf( fp->file, "</tr></thead><tbody>\n" );
	fprintf( fp->file, "<tr><td>Aree</td><td>%d</td></tr>\n", top_area );
	fprintf( fp->file, "<tr><td>Mob</td><td>%d</td></tr>\n", mobs_loaded );
	fprintf( fp->file, "<tr><td>Oggetti</td><td>%d</td></tr>\n", objects_loaded );
	fprintf( fp->file, "<tr><td>Stanze</td><td>%d</td></tr>\n", top_room );
	fprintf( fp->file, "<tr><td>Negozi</td><td>%d</td></tr>\n", top_shop + top_repair );
	fprintf( fp->file, "<tr><td>Aiuti</td><td>%d</td></tr>\n", top_help );
	fprintf( fp->file, "<tr><td>Comandi Italiani</td><td>%d</td></tr>\n", top_command_it );
	fprintf( fp->file, "<tr><td>Comandi Inglesi</td><td>%d</td></tr>\n", top_command_en );
	fprintf( fp->file, "<tr><td>Sociali</td><td>%d</td></tr>\n", top_social );
	fprintf( fp->file, "<tr><td>Abilità</td><td>%d</td></tr>\n", top_sn + MAX_LANGUAGE + MAX_HERB );
	fprintf( fp->file, "<tr><td>Classi</td><td>%d</td></tr>\n", top_class_play );
	fprintf( fp->file, "<tr><td>Razze</td><td>%d</td></tr>\n", top_race_play );
	fprintf( fp->file, "<tr><td>Sogni</td><td>%d</td></tr>\n", top_dream );
	fprintf( fp->file, "<tr><td>Liriche</td><td>%d</td></tr>\n", top_lyric );
	fprintf( fp->file, "<tr><td>Pg totali</td><td>%d</td></tr>\n", top_offline );

	/* Massimale ottenuto */
	fprintf( fp->file, "<tr><td>Massimo di Pg online di %d ottenuto il %s</td></tr>\n", mudstat.alltime_max, mudstat.time_max );

	fprintf( fp->file, "</tbody></table>\n" );

	MUD_FCLOSE( fp );
}


/*
 * Crea la tabella html con le aree del Mud
 * (FF) aggiungere la popolarità dell'area quando è stata immessa
 */
void create_html_areas( void )
{
	MUD_FILE	*fp;
	AREA_DATA	*pArea;

	/* Crea il file che conterrà la tabella html */
	fp = mud_fopen( WEB_DIR, "areas.html", "w", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	fputs( WEB_HEADER, fp->file );
	fprintf( fp->file, "<span class=\"BASE-GROSSO\">Aree di %s</span><br><br>\n", web_colorcnv(MUD_NAME) );
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Autore</th>\n" );
	fprintf( fp->file, "<th class=\"BASE-GGIALLO\">Nome area</th>\n" );
	fprintf( fp->file, "</tr></thead><tbody>\n" );
	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		char clr_area[MIL];

		if ( HAS_BIT(pArea->flags, AREAFLAG_SECRET) )	continue;
		if ( HAS_BIT(pArea->flags, AREAFLAG_WILD) )		continue;
		/* (BB) qui si comportava come se ci fosse un break e non un continue */

		sprintf( clr_area, "&%c%s", get_clr(pArea->color->cat), pArea->name );

		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", pArea->author );
		fprintf( fp->file, "<td>%s</td>", web_colorcnv(clr_area) );
		fprintf( fp->file, "</tr>\n" );
	}
	fprintf( fp->file, "</tbody></table>\n" );

	MUD_FCLOSE( fp );
}


/*
 * Crea la tabella con le statistiche dei mob per livello e per razza
 */
void create_table_mobstats( MOB_STAT_DATA *table_mob_stat, const int max )
{
	MOB_PROTO_DATA	*pMobIndex;
	MUD_FILE		*fp;
	int				 x;

	/* Crea il file che conterrà la tabella html */
	fp = mud_fopen( WEB_DIR, (max == LVL_LEGEND+1) ? "mob_stats_lvl.html" : "mob_stats_race.html", "w", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	/* Inizializza le tabelle che conterranno le statistiche */
	for ( x = 0;  x < max;  x++ )
	{
		table_mob_stat[x].number		= 0;
		table_mob_stat[x].ac			= 0;
		table_mob_stat[x].hitnodice		= 0;
		table_mob_stat[x].hitsizedice	= 0;
		table_mob_stat[x].hitplus		= 0;
		table_mob_stat[x].damnodice		= 0;
		table_mob_stat[x].damsizedice	= 0;
		table_mob_stat[x].damplus		= 0;
	}

	/* Salva le informazioni per poi creare delle medie */
	for ( pMobIndex = first_mob_proto;  pMobIndex;  pMobIndex = pMobIndex->next )
	{
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].number++;	/* Numero di mob di quel livello o razza */
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].ac			+= pMobIndex->ac;
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].hitnodice	+= pMobIndex->hitnodice;
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].hitsizedice	+= pMobIndex->hitsizedice;
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].hitplus		+= pMobIndex->hitplus;
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].damnodice	+= pMobIndex->damnodice;
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].damsizedice	+= pMobIndex->damsizedice;
		table_mob_stat[(max == LVL_LEGEND+1) ? pMobIndex->level : pMobIndex->race].damplus		+= pMobIndex->damplus;
	}

	/* Stampa anche quelli 0 per sfizio (TT) diciamo più che altro per test per ora */
	fputs( WEB_HEADER, fp->file );
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th>%s</th>", (max == LVL_LEGEND+1) ? "Livello" : "Razza" );
	fprintf( fp->file, "<th>AC</th>" );
	fprintf( fp->file, "<th>HitNoDice</th>" );
	fprintf( fp->file, "<th>HitSizeDice</th>" );
	fprintf( fp->file, "<th>HitPlus</th>" );
	fprintf( fp->file, "<th>DamNoDice</th>" );
	fprintf( fp->file, "<th>DamSizeDice</th>" );
	fprintf( fp->file, "<th>DamPlus</th>" );
	fprintf( fp->file, "</tr></thead><tbody>\n" );
	for ( x = 0; x < max;  x++ )
	{
		fprintf( fp->file, "<tr>" );
		if ( max == LVL_LEGEND+1 )
		{
			fprintf( fp->file, "<td>%d</td>", x );
		}
		else
		{
			char	clr_race[MIL];
			sprintf( clr_race, "&%c%s", get_clr(table_race[x]->color->cat), table_race[x]->name_male );
			fprintf( fp->file, "<td>%s</td>", web_colorcnv(clr_race) );
		}
		fprintf( fp->file, "<td>%d</td>", (table_mob_stat[x].number != 0) ? table_mob_stat[x].ac / table_mob_stat[x].number : 0 );
		fprintf( fp->file, "<td>%d</td>", (table_mob_stat[x].number != 0) ? table_mob_stat[x].hitnodice / table_mob_stat[x].number : 0 );
		fprintf( fp->file, "<td>%d</td>", (table_mob_stat[x].number != 0) ? table_mob_stat[x].hitsizedice / table_mob_stat[x].number : 0 );
		fprintf( fp->file, "<td>%d</td>", (table_mob_stat[x].number != 0) ? table_mob_stat[x].hitplus / table_mob_stat[x].number : 0 );
		fprintf( fp->file, "<td>%d</td>", (table_mob_stat[x].number != 0) ? table_mob_stat[x].damnodice / table_mob_stat[x].number : 0 );
		fprintf( fp->file, "<td>%d</td>", (table_mob_stat[x].number != 0) ? table_mob_stat[x].damsizedice / table_mob_stat[x].number : 0 );
		fprintf( fp->file, "<td>%d</td>", (table_mob_stat[x].number != 0) ? table_mob_stat[x].damplus / table_mob_stat[x].number : 0 );
		fprintf( fp->file, "</tr>\n" );
	}
	fprintf( fp->file, "</tbody></table>\n" );

	MUD_FCLOSE( fp );
}


/*
 * Richiama la funzione create_table_mobstats() per creare le due tabelle
 */
void create_table_mobstats_all( void )
{
	create_table_mobstats( table_def_stat_lvl, LVL_LEGEND+1 );
	create_table_mobstats( table_def_stat_race, MAX_RACE );
}


/*
 * Crea la tabella html con la lista dei comandi e dei social
 */
void create_html_commands( void )
{
	COMMAND_DATA *command;
	MUD_FILE	 *fp;
	int			  x;

	/* Crea il file che conterrà la tabella html */
	fp = mud_fopen( WEB_DIR, "commands.html", "w", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	fputs( WEB_HEADER, fp->file );
	fprintf( fp->file, "<span class=\"BASE-GROSSO\">Comandi di %s suddivisi per tipologia.</span><br><br>\n", web_colorcnv(MUD_NAME) );
/*	fputs( "<span class=\"BASE-GROSSO\">Comandi, Social e Faccine.</span><br><br>\n", fp->file );*/
	for ( x = 0;  x < MAX_CMDTYPE;  x++ )
	{
		char	letter;

		if ( x >= CMDTYPE_MUDPROG )
			continue;

		switch ( x )
		{
		  default:
			send_log( NULL, LOG_BUG, "create_html_commands: tipologia di comando mancante %d", x );
			exit( EXIT_FAILURE );
			break;
		  case CMDTYPE_MOVEMENT:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi ai movimenti e posizioni:</span>\n" );
			break;
		  case CMDTYPE_CHANNEL:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi ai canali di comunicazione:</span>\n" );
			break;
		  case CMDTYPE_INFORMATION:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi alle informazioni generiche o dell'avventuriero:</span>\n" );
			break;
		  case CMDTYPE_OBJECT:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi alla interazione con gli oggetti:</span>\n" );
			break;
		  case CMDTYPE_COMBAT:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi al combattimento:</span>\n" );
			break;
		  case CMDTYPE_GAMING:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi al gioco in generale:</span>\n" );
			break;
		  case CMDTYPE_GROUPING:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi al gruppo di avventurieri:</span>\n" );
			break;
		  case CMDTYPE_OPTION:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi alla configurazione e alle opzioni di gioco:</span>\n" );
			break;
		  case CMDTYPE_CLAN:
			fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Comandi relativi alla gestione e informazioni del clan:</span>\n" );
			break;
		}

		fputs( WEB_TABLE, fp->file );
		fprintf( fp->file, "<tbody>\n" );	/* (TT) <thead></thead> dovrebbe poter essere sottinteso se non serve */

		/* Vengono saltati i comandi che iniziano per numero o simbolo visto che legge solo le lettere per l'ordine 2alfabetico" */
		for ( letter = 'a'; letter <= 'z'; letter++ )
		{
			for ( command = first_command_it;  command;  command = command->next )
			{
				char	cmd_ita[MIL];
				char	cmd_eng[MIL];
				char	minihelp[MSL];

				if ( command->name[0] != letter )			continue;
				if ( command->synonym )						continue;
				if ( command->fun->type != x )				continue;
				if ( command->fun->trust > TRUST_PLAYER )	continue;

				one_argument( command->name, cmd_ita );
				strcpy( cmd_eng, command_from_fun(command->fun->fun_name, FALSE) );
				strcpy( minihelp, strip_char(command->fun->minihelp, '\r') );
				strcpy( minihelp, strip_char(minihelp, '\n') );
	
				fprintf( fp->file, "<tr>" );
				fprintf( fp->file, "<td>%s</td>", cmd_ita );
				fprintf( fp->file, "<td>%s</td>", cmd_eng );
				fprintf( fp->file, "<td>%s</td>", web_colorcnv(minihelp) );
				fprintf( fp->file, "</tr>\n" );
			}
		}

		fprintf( fp->file, "</tbody></table><br><br>\n" );
	}

#ifdef T2_ALFA
	/* Bisogna fare i minihelp per i social */
	fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Sociali di %s</span>\n", web_colorcnv(MUD_NAME) );
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<tbody>\n" );	/* (TT) <thead></thead> dovrebbe poter essere sottinteso se non serve */
	for ( letter = 'a'; letter <= 'z'; letter++ )	/* dovrebbero essere già in alfabetico, ma già che ci siamo.. */
	{
		for ( social = first_social;  social;  social = social->next )
		{
			char	soc_ita[MIL];
			char	soc_eng[MIL];
			char	minihelp[MSL];

			if ( social->italian[0] != letter )
				continue;

			one_argument( command->italian, cmd_ita );
			one_argument( command->english, cmd_eng );
			strcpy( minihelp, strip_char(social->minihelp, '\r') );
			strcpy( minihelp, strip_char(minihelp, '\n') );

			fprintf( fp->file, "<tr>" );
			fprintf( fp->file, "<td>%s</td>", cmd_ita );
			fprintf( fp->file, "<td>%s</td>", cmd_eng );
			fprintf( fp->file, "<td>%s</td>", minihelp );
			fprintf( fp->file, "</tr>\n" );
		}
	}
	fprintf( fp->file, "</tbody></table><br><br>\n" );

	/* Stampa le faccine */
	fprintf( fp->file, "<span class=\"BASE-GGIALLO\">Faccine utilizzabili nei canali RPG:</span>\n" );
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<tbody>\n" );	/* (TT) <thead></thead> dovrebbe poter essere sottinteso se non serve */

	for ( social = first_social;  social;  social = social->next )
	{
		if ( !VALID_STR(social->smiles) )		continue;
		if ( !VALID_STR(social->expression) )	continue;

		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", social->smiles );
		fprintf( fp->file, "<td>%s</td>", social->expression );
		fprintf( fp->file, "<tr>" );
	}

	fprintf( fp->file, "</tbody></table><br><br>\n" );
#endif

	MUD_FCLOSE( fp );
}


/*
 * Crea la lista delle statistiche per il web prendendole dalla lista dei pg offline
 */
void load_web_tables( void )
{
	create_html_stat_all( );		/* Crea le tabelle in html delle varie statistiche dei pg */
	create_html_calendar( );		/* Crea le tabelle riguardanti il calendario del Mud */
	create_html_census( );			/* Crea la tabella con le statistiche classe e razza */
	create_html_memory( );			/* Crea la tabella con le statistiche tecniche del mud */
	create_html_areas( );			/* Crea la tabella delle aree nel Mud */
	create_table_mobstats_all( );	/* Crea la tabella con le statistiche dei mob */
	create_html_commands( );		/* Crea la tabella con tutti i comandi e i loro minihelp */
	/* (FF) aggiungere il sistema di web di max giocatori online al giorno */
	/* (FF) help */
	/* (FF) unità di misura e monete */
}


/* Ora che non servono più le sdefinisce */
#undef TABLE_TIME
#undef TABLE_HIGHSCORE
#undef TABLE_REPORT


/*
 * Routine di update che crea il file who per il sito web
 */
void create_html_webwho( void )
{
	CHAR_DATA	*ch;
	MUD_FILE	*fp;
	int			 nMatch = 0;

	fp = mud_fopen( WEB_DIR, "web_who.html", "w", TRUE );
	if ( !fp )
		return;

	/* Crea la tabella in html con la lista dei giocatori online */
	fputs( WEB_HEADER, fp->file );
	fputs( WEB_TABLE, fp->file );
	fprintf( fp->file, "<thead><tr>\n" );
	fprintf( fp->file, "<th width=\"12%%\">Razza</th>" );
	fprintf( fp->file, "<th width=\"12%%\">Nome</th>" );
	fprintf( fp->file, "<th width=\"34%%\">Titolo</th>" );
	fprintf( fp->file, "<th width=\"12%%\">Archetipo</th>" );
/*	fprintf( fp->file, "<th width=\"14%%\">Client</th>" ); (BB) per il baco che becca il nome del client al login non lo inserisco, a volte capita che metta la password dopo il nome del client */
	fprintf( fp->file, "<th width=\"8%%\">Ore</th>" );
	fprintf( fp->file, "<th width=\"8%%\">Genere</th>" );
	fprintf( fp->file, "</tr></thead><tbody>\n" );

	/* Scorre nella lista dei char per salvare anche i link death */
	for ( ch = last_player;  ch;  ch = ch->prev_player )
	{
		char	clr_race[MIL];

		if ( ch->desc && ch->desc->connected != CON_PLAYING )	continue;
		if ( ch->pg->incognito != 0 )							continue;
		if ( IS_ADMIN(ch) && HAS_BIT_PLR(ch, PLAYER_INVISWHO) )	continue;
		if ( ch->desc && ch->desc->original )					continue;

		nMatch++;

		strcpy( clr_race, web_colorcnv(get_race(ch, TRUE)) );

		fprintf( fp->file, "<tr>" );
		fprintf( fp->file, "<td>%s</td>", clr_race );
		fprintf( fp->file, "<td>%s</td>", ch->name );
		fprintf( fp->file, "<td>%s</td>", VALID_STR(ch->pg->title) ? web_colorcnv(ch->pg->title) : "" );
		fprintf( fp->file, "<td>%s</td>", get_archetype(ch) );
/*		fprintf( fp->file, "<td>%s</td>", (ch->desc) ? web_colorcnv(ch->desc->client) : "Sconosciuto" );*/
		fprintf( fp->file, "<td>%d</td>", get_hours_played(ch) );
		fprintf( fp->file, "<td>%c</td>",
/*			(ch->sex == SEX_FEMALE) ? table_color[AT_PINK].color.name : table_color[AT_BLUE].color.name, (FF) sistema di colori in futuro */
			(ch->sex == SEX_FEMALE) ? 'F' : 'M' );
		fprintf( fp->file, "</tr>\n" );
	}

	fprintf( fp->file, "</tbody></table>\n" );
	fprintf( fp->file, "<span class=\"BASE-GGIALLO\">%s ha %d Giocator%c</span>\n",
		web_colorcnv(MUD_NAME), nMatch, (nMatch == 1)  ?  'e'  :  'i' );

	MUD_FCLOSE( fp );
}


#endif	/* T2_WEB */
