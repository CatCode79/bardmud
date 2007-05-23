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

/*****************************************************
**     _________       __                           **
**     \_   ___ \_____|__| _____  ________  ___     **
**      /    \  \/_  __ \ |/     \/  ___/_ \/   \   **
**      \     \___|  | \/ |  | |  \___ \  / ) |  \  **
**       \______  /__| |__|__|_|  /____ \__/__|  /  **
**         ____\/____ _        \/ ___ \/      \/    **
**         \______   \ |_____  __| _/___            **
**          |    |  _/ |\__  \/ __ | __ \           **
**          |    |   \ |_/ __ \  / | ___/_          **
**          |_____  /__/____  /_  /___  /           **
**               \/Antipode\/  \/    \/             **
******************************************************
**       Ravager codebase modifications             **
**     (c) 2000-2003 John Bellone (Noplex)          **
**           Coders: Noplex, Krowe                  **
**        http://www.crimson-blade.org              **
*****************************************************/

/*
 * File: news.c
 * Name: Extended News (v3.2)
 * Author: John 'Noplex' Bellone (jbellone@comcast.net)
 * Terms:
 * If this file is to be re-disributed; you must send an email
 * to the author. All headers above the #include calls must be
 * kept intact. All license requirements must be met. License
 * can be found in the included license.txt document or on the
 * website.
 * Description:
 * This is Crimson Blade's extended news module. It allows for news
 * cataglories to be created; each having it's own different filesystem
 * where news can be posted. It also allows for each to be specified a
 * specific path and filename where the news-textdump of HTML can be
 * sent to.
 * The news commands are tied directly into the command interpreter; so
 * news-types are instantaniously added; removed; and edited.
 */

#include <ctype.h>

#include "mud.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "news.h"


/*
 * Definizioni
 */
#define NEWS_FILE		SYSTEM_DIR	"news.dat"


/*
 * Variabili locali
 */
NEWS_DATA	*first_news	= NULL;
NEWS_DATA	*last_news	= NULL;
int			 top_news	= 0;


/*
 * Tabella codici dei tipi di novità
 */
const CODE_DATA	code_news[] =
{
	{ NEWS_NEW,		"NEWS_NEW",		"novità"	},
	{ NEWS_BUG,		"NEWS_BUG",		"bug"		},
	{ NEWS_EVENT,	"NEWS_EVENT",	"evento"	},
	{ NEWS_AREA,	"NEWS_AREA",	"area"		}
};
const int max_check_news = sizeof(code_news)/sizeof(CODE_DATA);


/*
 * Carica una notizia
 */
void fread_news( MUD_FILE *fp )
{
	NEWS_DATA	*news;

	CREATE( news, NEWS_DATA, 1 );
	news->type = -1;

	for( ; ; )
	{
		char	*word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_news: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Author") )	news->author =		str_dup( fread_word(fp) );
		else if ( !str_cmp(word, "Date"	 ) )	news->date =		str_dup( fread_word(fp) );
		else if ( !str_cmp(word, "End"	 ) )	break;
		else if ( !str_cmp(word, "Title" ) )	news->title =		fread_string( fp );
		else if ( !str_cmp(word, "Text"	 ) )	news->text =		fread_string( fp );
		else if ( !str_cmp(word, "Type"	 ) )	news->type =		code_code( fp, fread_word(fp), CODE_NEWS );
		else
			send_log( fp, LOG_FREAD, "fread_news: etichetta non trovata: %s", word );
	}

	if ( news->type <= -1 || news->type >= MAX_NEWS )
	{
		send_log( fp, LOG_FREAD, "fread_news: tipo di news errata: %d", news->type );
		news->type = 0;
	}

	if ( !VALID_STR(news->author) )
	{
		send_log( fp, LOG_FREAD, "fread_news: autore di news inesistente" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(news->date) )
	{
		send_log( fp, LOG_FREAD, "fread_news: data di news inesistente" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(news->title) )
	{
		send_log( fp, LOG_FREAD, "fread_news: Titolo di news inesistente" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(news->text) )
	{
		send_log( fp, LOG_FREAD, "fread_news: testo di news inesistente" );
		exit( EXIT_FAILURE );
	}

	LINK( news, first_news, last_news, next, prev );
	top_news++;
}


/*
 * Carica tutte le notizie
 */
void load_news( void )
{
	fread_section( NEWS_FILE, "NEWS", fread_news, FALSE );

	/*
	 * (FF) magari controllare che le notizie siano in ordine di data
	 */
}


/*
 * Libera dalla memoria
 */
void free_all_news( void )
{
	NEWS_DATA *news;

	for ( news = first_news;  news;  news = first_news )
	{
		UNLINK( news, first_news, last_news, next, prev );
		top_news--;

		DISPOSE( news->date );
		DISPOSE( news->author );
		DISPOSE( news->title );
		DISPOSE( news->text );
		DISPOSE( news );
	}

	if ( top_news != 0 )
		send_log( NULL, LOG_BUG, "free_all_news: top_news è diverso da 0 dopo aver liberato le novità: %d", top_news );
}


/*
 * save the linked list
 */
void save_news( void )
{
	NEWS_DATA	*news;
	MUD_FILE	*fp;

	fp = mud_fopen( "", NEWS_FILE, "w", TRUE );
	if ( !fp )
		return;

	for ( news = first_news;  news;  news = news->next )
	{
		fprintf( fp->file, "#NEWS\n" );
		fprintf( fp->file, "Type     %s\n", code_str(fp, news->type, CODE_NEWS) );
		fprintf( fp->file, "Author   %s\n", news->author );
		fprintf( fp->file, "Date     %s\n", news->date );
		fprintf( fp->file, "Title    %s~\n", news->title );
		fprintf( fp->file, "Text\n" );
		fprintf( fp->file, "%s~\n", news->text );
		fprintf( fp->file, "End\n\n" );
	}
	fprintf( fp->file, "#END\n" );

	MUD_FCLOSE( fp );
}


/*
 * Snatch news up from the linked list
 */
NEWS_DATA *get_news_from_num( const int num )
{
	NEWS_DATA  *news;
	int			x;

	if ( num < 1 )
		return NULL;

	x = 1;
	for ( news = first_news;  news;  news = news->next )
	{
		if ( num == x )
			return news;

		x++;
	}

	return NULL;
}


/*
 * Comando admin per creare, editare o rimuovere news
 */
void do_editnews( CHAR_DATA *ch, char *argument )
{
	char	arg1[MIL];
	char	arg2[MIL];
	int		type;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono editare le notizie.\r\n" );
		return;
	}

	switch ( ch->substate )
	{
	  default:
		break;

	  case SUB_NEWS_POST:
	  {
		NEWS_DATA *news = NULL;

		news = ch->dest_buf;
		DISPOSE( news->text );
		news->text = copy_buffer( ch );
		stop_editing( ch );
		LINK( news, first_news, last_news, next, prev );
		ch->substate = ch->tempnum;
		save_news( );
		return;
	  }
	  break;

	  case SUB_NEWS_EDIT:
	  {
		NEWS_DATA *news = NULL;

		news = ch->dest_buf;
		DISPOSE( news->text );
		news->text = copy_buffer( ch );
		stop_editing( ch );
		ch->substate = ch->tempnum;
		save_news( );
		return;
	  }
	  break;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_to_char( ch, "Syntax: editnews add <tipo> <titolo>\r\n" );
		send_to_char( ch, "        editnews edit <numero> <nuovo titolo [opzionale]>\r\n" );
		send_to_char( ch, "        editnews remove <numero>\r\n" );

		return;
	}

	if ( !str_cmp(arg1, "addnews") )
	{
		NEWS_DATA	*news = NULL;

		if ( !VALID_STR(argument) )
		{
			send_command( ch, "editnews", CO );
			return;
		}

		type = code_num( NULL, arg2, CODE_NEWS );
		if ( type == -1 )
		{
			send_to_char( ch, "Invaild news-type. Use 'newstypes' to get a vaild listing.\r\n" );
			return;
		}

		CREATE( news, NEWS_DATA, 1 );
		news->type		= type;
		news->title		= str_dup( argument );
		news->author	= str_dup( ch->name );
		news->date		= str_dup( stamp_time() );
		news->text		= str_dup( "" );

		/* pop character into a writing buffer */
		if ( ch->substate == SUB_REPEAT_CMD )
			ch->tempnum = SUB_REPEAT_CMD;
		else
			ch->tempnum = SUB_NONE;

		ch->substate	= SUB_NEWS_POST;
		ch->dest_buf	= news;
		start_editing( ch, news->text );
	}

	if ( !str_cmp(arg1, "editnews") )
	{
		NEWS_DATA	*news = NULL;

		if ( !is_number(arg2) )
		{
			send_command( ch, "editnews", CO );
			return;
		}

		news = get_news_from_num( atoi(arg2) );
		if ( !news )
		{
			pager_printf( ch, "That's not a valid news number.\r\n" );
			return;
		}

		/* a changed title */
		news->title = str_dup( argument );

		/* new date news was edited */
		news->date	= str_dup( stamp_time() );
		/* pop character into a writing buffer */
		if( ch->substate == SUB_REPEAT_CMD )
			ch->tempnum = SUB_REPEAT_CMD;
		else
			ch->tempnum = SUB_NONE;

		ch->substate = SUB_NEWS_EDIT;
		ch->dest_buf = news;
		start_editing( ch, news->text );
	}

	if ( !str_cmp(arg1, "removenews") )
	{
		NEWS_DATA *news = NULL;

		if ( !is_number(arg2) )
		{
			send_command( ch, "editnews", CO );
			return;
		}

		news = get_news_from_num( atoi(arg2) );
		if ( !news )
		{
			ch_printf( ch, "Type %s to gain a list of the news numbers.\r\n",
				translate_command(ch, "news") );
			return;
		}

		UNLINK( news, first_news, last_news, next, prev );
		DISPOSE( news->author );
		DISPOSE( news->title );
		DISPOSE( news->date );
		DISPOSE( news->text );
		DISPOSE( news );
		save_news( );
		send_to_char( ch, "Done.\r\n" );
	}
}


/*
 * Comando per visualizzare le novità
 */
DO_RET do_news( CHAR_DATA *ch, char *argument )
{
	NEWS_DATA	*news;
	char		*divline = "\r\n&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*&g<>*<>&G*\r\n";
	int			 x;
	int			 num	= -1;
	int			 type	= -1;
	bool		 show_list	= FALSE;
	bool		 show_last	= FALSE;
	bool		 show_num	= FALSE;
	bool		 show_type	= FALSE;

	if ( !VALID_STR(argument) )
	{
		ch_printf( ch, "&YSintassi&w:  %s lista\r\n", translate_command(ch, "news") );
		ch_printf( ch, "           %s ultime\r\n", translate_command(ch, "news") );
		ch_printf( ch, "           %s <tipo>\r\n", translate_command(ch, "news") );
		ch_printf( ch, "           %s <numero>\r\n\r\n", translate_command(ch, "news") );

		send_to_char( ch, "I tipi di novità possono essere:\r\n" );
		send_to_char( ch, code_all(CODE_NEWS, FALSE) );
		send_to_char( ch, "\r\n\r\n" );

		send_to_char( ch, "Passando un numero verrà inviata la notizia relativa. Il numero lo si può\r\n" );
		send_to_char( ch, " ricavare dalla lista di notizie\r\n" );

		return;
	}

	if ( is_name(argument, "lista list") )
	{
		show_list = TRUE;
	}
	else if ( is_name(argument, "ultima ultime last") )
	{
		show_last = TRUE;
	}
	else if ( !is_number(argument) )
	{
		type = code_num( NULL, argument, CODE_NEWS );
		if ( type == -1 )
		{
			send_to_char( ch, "Tipo di notizia inesistente.\r\n" );
			send_command( ch, "news", CO );
			return;
		}
		show_type = TRUE;
	}
	else if ( is_number(argument) )
	{
		num = atoi( argument );
		if ( get_news_from_num(num) == NULL )
		{
			send_to_char( ch, "Numero di notizia inesistente.\r\n" );
			send_command( ch, "news", CO );
			return;
		}
		show_num = TRUE;
	}

	x = 0;

	send_command( ch, "help _MSG_NEWS_HEADER", CO );
	send_to_pager( ch, divline );
	send_to_pager( ch, "\r\n&G&Y[&W#&Y] [&WAutore&Y]   [&WData&Y]     [&WTipo&Y] [&WTitolo&Y]&w\r\n" );
	for ( news = first_news;  news;  news = news->next )
	{
		x++;

		if ( show_last && x < top_news-5 )		continue;
		if ( show_type && type != news->type )	continue;	/* (bb) forse se si passa novità come argomento qui non passa, problema di accenti penso */
		if ( show_num  && num != x )			continue;

		pager_printf( ch, "&W%2d&Y]&w %-10s &g%-10s&w &B%-6s&w - &W%s&w\r\n",
			x,
			news->author,
			news->date,
			code_name(NULL, news->type, CODE_NEWS),
			news->title );

		if ( show_last || show_num )
		{
			pager_printf( ch, "  %s", news->text );
			if ( news != last_news && !show_num )
				send_to_pager( ch, "\r\n" );
		}
	}
	send_to_pager( ch, divline );
}
