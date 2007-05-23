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


/* Modulo per la gestione dell'editor */

#include "mud.h"
#include "command.h"
#include "editor.h"
#include "interpret.h"
#include "nanny.h"


/*
 * Viene printato anche in nanny.c, quindi per meglio gestirlo si trova in una funzione
 */
void editor_header( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "ch passato è NULL" );
		return;
	}

	set_char_color( AT_GREEN, ch );
	send_to_char( ch, "Scrivi il tuo testo (/? = aiuto /s = salva /c = cancella /l = lista)\r\n" );
	send_to_char( ch, "--------------------------------------------------------------------------------\r\n" );
	send_to_char( ch, "> " );
}


void start_editing( CHAR_DATA *ch, char *data )
{
	EDITOR_DATA *edit;
	char		 c;
	int			 lines = 0;
	int			 size = 0;
	int			 lpos = 0;

	if ( !ch->desc )
	{
		send_log( NULL, LOG_BUG, "start_editing: no desc" );
		return;
	}

	if ( ch->substate == SUB_RESTRICTED )
		send_log( NULL, LOG_BUG, "start_editing: ch->substate è uguale a SUB_RESTRICTED" );

	editor_header( ch );

	if ( ch->editor )
		stop_editing( ch );

	CREATE( edit, EDITOR_DATA, 1 );
	edit->numlines = 0;
	edit->on_line  = 0;
	edit->size     = 0;

	if ( !data )
	{
		send_log( NULL, LOG_BUG, "start_editing: data è NULL." );
	}
	else
	{
		for ( ; ; )
		{
			c = data[size++];
			if ( c == '\0' )
			{
				edit->line[lines][lpos] = '\0';
				break;
			}
			else if ( c == '\r' )
			{
				;
			}
			else if ( c == '\n' || lpos > 78 )
			{
				edit->line[lines][lpos] = '\0';
				++lines;
				lpos = 0;
			}
			else
			{
				edit->line[lines][lpos++] = c;
			}

			if ( lines >= 49 || size > 4096 )
			{
				edit->line[lines][lpos] = '\0';
				break;
			}
		}
	}

	if ( lpos > 0 && lpos < 78 && lines < 49 )
	{
		edit->line[lines][lpos] = '~';
		edit->line[lines][lpos+1] = '\0';
		++lines;
		lpos = 0;
	}

	edit->numlines = lines;
	edit->size = size;
	edit->on_line = lines;
	ch->editor = edit;
	ch->desc->connected = CON_EDITING;
}


char *copy_buffer( CHAR_DATA *ch )
{
	char	buf[MSL];
	char	tmp[100];
	int		x;
	int		len;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "copy_buffer: null ch" );
		return str_dup( "" );
	}

	if ( !ch->editor )
	{
		send_log( NULL, LOG_BUG, "copy_buffer: null editor" );
		return str_dup( "" );
	}

	buf[0] = '\0';
	for ( x = 0;  x < ch->editor->numlines;  x++ )
	{
		strcpy( tmp, ch->editor->line[x] );

		len = strlen(tmp);
		if ( tmp && tmp[len-1] == '~' )
			tmp[len-1] = '\0';
		else
			strcat( tmp, "\r\n" );

		smash_tilde( tmp );
		strcat( buf, tmp );
	}

	return str_dup( buf );
}


void stop_editing( CHAR_DATA *ch )
{
	set_char_color( AT_PLAIN, ch );

	DISPOSE( ch->editor );
	ch->editor		= NULL;
	ch->dest_buf	= NULL;
	ch->spare_ptr	= NULL;
	ch->substate	= SUB_NONE;

	send_to_char( ch, "Fatto.\r\n" );

	if ( !ch->desc )
	{
		send_log( NULL, LOG_BUG, "stop_editing: no desc" );
		return;
	}

	ch->desc->connected = CON_PLAYING;
}


/*
 * Simple but nice and handy line editor.
 */
void edit_buffer( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA	*d;
	EDITOR_DATA		*edit;
	char			 cmd[MIL];
	char			 buf[MIL];
	int				 x;
	int				 line;
	int				 max_buf_lines;
	bool			 save;

	d = ch->desc;
	if ( !d )
	{
		send_to_char( ch, "Non hai una connessione valida.\r\n" );
		return;
	}

	if ( d->connected != CON_EDITING )
	{
		send_to_char( ch, "Non ti è possibile farlo.\r\n" );
		send_log( NULL, LOG_BUG, "edit_buffer: d->connected != CON_EDITING" );
		return;
	}

	if ( ch->substate <= SUB_PAUSE )
	{
		send_to_char( ch, "Non ti è possibile farlo.\r\n" );
		send_log( NULL, LOG_BUG, "edit_buffer: illegal ch->substate: %d", ch->substate );
		d->connected = CON_PLAYING;
		return;
	}

	if ( !ch->editor )
	{
		send_to_char( ch, "Non ti è possibile farlo.\r\n" );
		send_log( NULL, LOG_BUG, "edit_buffer: editor è NULL" );
		d->connected = CON_PLAYING;
		return;
	}

	edit = ch->editor;
	save = FALSE;
	max_buf_lines = 100;

	if ( ch->substate == SUB_MPROG_EDIT )
		max_buf_lines = 100;

	if ( argument[0] == '/' || argument[0] == '\\' )
	{
		one_argument( argument, cmd );

		if ( cmd[1] == '?' )
		{
			send_to_char( ch, "Comandi di editing\r\n"						 );
			send_to_char( ch, "----------------------------------------\r\n" );
			send_to_char( ch, "/l                    list buffer\r\n"		 );
			send_to_char( ch, "/c                    clear buffer\r\n"		 );
			send_to_char( ch, "/d [linea]            delete line\r\n"		 );
			send_to_char( ch, "/g <linea>            goto line\r\n"			 );
			send_to_char( ch, "/i <linea>            insert line\r\n"		 );
			send_to_char( ch, "/r <vecchia> <nuova>  global replace\r\n"	 );
			send_to_char( ch, "/a                    abort editing\r\n"		 );
			send_to_char( ch, "/s                    save buffer\r\n> "		 );
			if ( IS_ADMIN(ch) )
				send_to_char( ch, "/! <command>          execute command (do not use another editing command)\r\n\r\n" );

			return;
		}

		if ( cmd[1] == 'c' )
		{
			memset( edit, '\0', sizeof(EDITOR_DATA) );
			edit->numlines	= 0;
			edit->on_line	= 0;
			send_to_char( ch, "Buffer cleared.\r\n> " );
			return;
		}

		if ( cmd[1] == 'r' )
		{
			char   *sptr;
			char   *wptr;
			char   *lwptr;
			char	word1[MIL];
			char	word2[MIL];
			int		count;
			int		wordln;
			int		word2ln;
			int		lineln;

			sptr = one_argument( argument, word1 );
			sptr = one_argument( sptr, word1 );
			sptr = one_argument( sptr, word2 );

			if ( !VALID_STR(word1) || !VALID_STR(word2) )
			{
				send_to_char( ch, "Need word to replace, and replacement.\r\n> " );
				return;
			}

			if ( strcmp(word1, word2) == 0 )
			{
				send_to_char( ch, "Fatto.\r\n> " );
				return;
			}
			count = 0;
			wordln  = strlen( word1 );
			word2ln = strlen( word2 );
			ch_printf( ch, "Replacing all occurrences of %s with %s...\r\n", word1, word2 );

			for ( x = edit->on_line; x < edit->numlines; x++ )
			{
				lwptr = edit->line[x];
				while ( (wptr = strstr( lwptr, word1 )) != NULL )
				{
					++count;
					lineln = sprintf( buf, "%s%s", word2, wptr + wordln );
					if ( lineln + wptr - edit->line[x] > 79 )
						buf[lineln] = '\0';
					strcpy( wptr, buf );
					lwptr = wptr + word2ln;
				}
			}
			ch_printf( ch, "Found and replaced %d occurrence(s).\r\n> ", count );
			return;
		}

		if ( cmd[1] == 'i' )
		{
			if ( edit->numlines >= max_buf_lines )
			{
				send_to_char( ch, "Buffer is full.\r\n> " );
			}
			else
			{
				if ( argument[2] == ' ' )
					line = atoi( argument + 2 ) - 1;
				else
					line = edit->on_line;

				if ( line < 0 )
					line = edit->on_line;

				if ( line < 0 || line > edit->numlines )
					send_to_char( ch, "Out of range.\r\n> " );
				else
				{
					for ( x = ++edit->numlines; x > line; x-- )
						strcpy( edit->line[x], edit->line[x-1] );
					strcpy( edit->line[line], "" );
					send_to_char( ch, "Line inserted.\r\n> " );
				}
 	    	}
			return;
		}

		if ( cmd[1] == 'd' )
		{
			if ( edit->numlines == 0 )
			{
				send_to_char( ch, "Buffer is empty.\r\n> " );
			}
			else
			{
				if ( argument[2] == ' ' )
					line = atoi( argument+2 ) - 1;
				else
					line = edit->on_line;

				if ( line < 0 )
					line = edit->on_line;

				if ( line < 0 || line > edit->numlines )
					send_to_char( ch, "Out of range.\r\n> " );
				else
				{
					if ( line == 0 && edit->numlines == 1 )
					{
						memset( edit, '\0', sizeof(EDITOR_DATA) );
						edit->numlines = 0;
						edit->on_line   = 0;
						send_to_char( ch, "Line deleted.\r\n> " );
						return;
					}

					for ( x = line;  x < (edit->numlines - 1);  x++ )
						strcpy( edit->line[x], edit->line[x+1] );

					strcpy( edit->line[edit->numlines--], "" );
					if ( edit->on_line > edit->numlines )
						edit->on_line = edit->numlines;
					send_to_char( ch, "Line deleted.\r\n> " );
				}
			}
			return;
		}

		if ( cmd[1] == 'g' )
		{
			if ( edit->numlines == 0 )
			{
				send_to_char( ch, "Buffer is empty.\r\n> " );
			}
			else
			{
				if ( argument[2] == ' ' )
					line = atoi( argument+2 ) - 1;
				else
				{
					send_to_char( ch, "Goto what line?\r\n> " );
					return;
				}

				if ( line < 0 )
					line = edit->on_line;
				if ( line < 0 || line > edit->numlines )
					send_to_char( ch, "Out of range.\r\n> " );
				else
				{
					edit->on_line = line;
					ch_printf( ch, "(On line %d)\r\n> ", line+1 );
				}
			}
			return;
		}

		if ( cmd[1] == 'l' )
		{
			if ( edit->numlines == 0 )
			{
				send_to_char( ch, "Buffer is empty.\r\n> " );
			}
			else
			{
				send_to_char( ch, "------------------\r\n" );
				for ( x = 0; x < edit->numlines; x++ )
					ch_printf( ch, "%2d> %s\r\n", x+1, edit->line[x] );
				send_to_char( ch, "------------------\r\n> " );
			}
			return;
		}

		if ( cmd[1] == 'a' )
		{
			send_to_char( ch, "\r\nAborting... " );
			stop_editing( ch );
			return;
		}

		if ( IS_ADMIN(ch) && cmd[1] == '!' )
		{
			DO_FUN	*last_cmd;
			int		 substate = ch->substate;

			last_cmd = ch->last_cmd;
			ch->substate = SUB_RESTRICTED;
			/* Si suppone che ch abbia inviato argument nella sua lingua quindi niente send_command() */
			interpret( ch, argument+3 );
			ch->substate = substate;
			ch->last_cmd = last_cmd;
			set_char_color( AT_GREEN, ch );
			send_to_char( ch, "\r\n> " );
			return;
		}

		if ( cmd[1] == 's' )
		{
			d->connected = CON_PLAYING;
			if ( !ch->last_cmd )
				return;
			(*ch->last_cmd) ( ch, "" );
			return;
		}
	}

	if ( edit->size + strlen(argument) + 1 >= MSL-1 )
	{
		send_to_char( ch, "You buffer is full.\n\r" );
	}
	else
	{
		if ( strlen_nocolor(argument) > 80 )	/* (TT) ho cambiato tutti i 79 in 80, occhio */
		{
			char arg[MSL];
			buf[0] = '\0';

			while ( VALID_STR(argument) )
			{
				argument = one_argument_editor( argument, arg );

				if ( strlen_nocolor(arg) > 80 )	/* A single word more than 80 long? Skip! */
				{
					send_to_char( ch, "Non inviare parole lunghe più di 80 caratteri!" );
					continue;
				}

				if ( (strlen_nocolor(buf) + strlen_nocolor(arg) + 1) <= 80 )
				{
					sprintf( buf, "%s%s%s", buf, buf[0] == '\0' ? "" : " ", arg );
				}
				else		/* Ok end this line and move onto the next */
				{
					strcpy( edit->line[edit->on_line++], buf );
					if ( edit->on_line > edit->numlines )
						edit->numlines++;
					buf[0] = '\0';			/* Resets the buffer to empty so can start over again. */
					strcat( buf, arg );		/* Adds the word that was too much, to buffer and continues processing */
				}
				if ( edit->numlines > max_buf_lines )
				{
					edit->numlines = max_buf_lines;
					send_to_char( ch, "You've run out of room in the editing buffer.\r\n" );
					save = TRUE;
				}
			}
			strcpy( edit->line[edit->on_line++], buf );
			if ( edit->on_line > edit->numlines )
				edit->numlines++;
			buf[0] = '\0';		/* Resets the buffer to empty so can start over again. */
			if ( edit->numlines > max_buf_lines )
			{
				edit->numlines = max_buf_lines;
				send_to_char( ch, "You've run out of room in the editing buffer.\r\n" );
				save = TRUE;
			}
		}
		else
		{
			strcpy( buf, argument );
		}

		if ( VALID_STR(buf) )
			strcpy( edit->line[edit->on_line++], buf );
		if ( edit->on_line > edit->numlines )
			edit->numlines++;
		if ( edit->numlines > max_buf_lines )
		{
			edit->numlines = max_buf_lines;
			send_to_char( ch, "Buffer full.\r\n" );
			save = TRUE;
		}
	}

	if ( save )
	{
		d->connected = CON_PLAYING;
		if ( !ch->last_cmd )
			return;
		(*ch->last_cmd) ( ch, "" );
		return;
	}

	send_to_char( ch, "> " );
}


/*
 * Ritorna vero se il pg sta editando
 */
bool is_editing( DESCRIPTOR_DATA *d )
{
	if ( !d )
		return FALSE;

	if ( !d->character || IS_MOB(d->character) )
		return FALSE;

	if ( d->connected == CON_EDITING )
		return TRUE;

	return FALSE;
}
