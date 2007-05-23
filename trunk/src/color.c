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


/****************************************************************************
 *               Color Module -- Allow user customizable Colors.            *
 *                                   --Matthew                              *
 *                      Enhanced ANSI parser by Samson                      *
 ****************************************************************************/

/*
 * The following instructions assume you know at least a little bit about
 * coding.  I firmly believe that if you can't code (at least a little bit),
 * you don't belong running a mud.  So, with that in mind, I don't hold your
 * hand through these instructions.
 *
 * You may use this code provided that:
 *
 *     1)  You understand that the authors _DO NOT_ support this code
 *         Any help you need must be obtained from other sources.  The
 *         authors will ignore any and all requests for help.
 *     2)  You will mention the authors if someone asks about the code.
 *         You will not take credit for the code, but you can take credit
 *         for any enhancements you make.
 *     3)  This message remains intact.
 *
 * If you would like to find out how to send the authors large sums of money,
 * you may e-mail the following address:
 *
 * Matthew Bafford & Christopher Wigginton
 * wiggy@mudservices.com
 */

/*
 * Per aggiungere un nuovo tipo di colore:
 * 1. Edita color.h, e:
 *    - Aggiungi un nuovo define AT_
 *    - Incrementa MAX_AT_COLORS.
 * 2. Edita color.c e:
 *    - Aggiungi il nome per il tuo colore alla fine dell'array color_default_set.
 *    - Aggiungi il colore di default accanto al nome del tuo colore in color_default_set.
 */

#include <ctype.h>
#include "mud.h"


/*===============================================================================================*/


/*
 * Tabella dei colori.
 * Gli elementi di questa tabella vengono chiamati con l'enumerazione dei 16 codici di
 *	colore AT_ principali dichiarati in color.h.
 * La table_color è fatta in maniera tale che invia cmq i primi 8 colori di background
 *	scuri anche richimandoli con i codici di colore chiari.
 * (FF) Riprovare a mettere qui la struttura e non nell'h e quindi fare modulare table_color
 */
const struct color_type table_color[] =
{
/*			CODE_DATA colori					CODE_DATA colore capelli			foreground		background		blink */
	{ 'x', { AT_BLACK,	 "AT_BLACK",	"nero"			},	{ AT_BLACK,	  "AT_BLACK",	"nero"		 	},	{ "\033[1;30m",	"\033[40m",	"\033[1;5;30m" },	"<font color=#000000>" },
	{ 'r', { AT_DRED,	 "AT_DRED",		"rosso scuro"	},	{ AT_DRED,	  "AT_DRED",	"rosso scuro"	},	{ "\033[0;31m",	"\033[41m",	"\033[0;5;31m" },	"<font color=#7F0000>" },
	{ 'g', { AT_DGREEN,	 "AT_DGREEN",	"verde scuro"	},	{ AT_DGREEN,  "AT_DGREEN",	"verde scuro"	},	{ "\033[0;32m",	"\033[42m",	"\033[0;5;32m" },	"<font color=#007F00>" },
	{ 'O', { AT_ORANGE,	 "AT_ORANGE",	"marrone"		},	{ AT_ORANGE,  "AT_ORANGE",	"castano"		},	{ "\033[0;33m",	"\033[43m",	"\033[0;5;33m" },	"<font color=#7F7F00>" },
	{ 'b', { AT_DBLUE,	 "AT_DBLUE",	"blu scuro"		},	{ AT_DBLUE,	  "AT_DBLUE",	"blu scuro"		},	{ "\033[0;34m",	"\033[44m",	"\033[0;5;34m" },	"<font color=#00007F>" },
	{ 'p', { AT_MAGENTA, "AT_MAGENTA",	"magenta"		},	{ AT_MAGENTA, "AT_MAGENTA",	"magenta"		},	{ "\033[0;35m",	"\033[45m",	"\033[0;5;35m" },	"<font color=#7F007F>" },
	{ 'c', { AT_CYAN,	 "AT_CYAN",		"ciano"			},	{ AT_CYAN,	  "AT_CYAN",	"ciano"			},	{ "\033[0;36m",	"\033[46m",	"\033[0;5;36m" },	"<font color=#007F7F>" },
	{ 'w', { AT_GREY,	 "AT_GREY",		"grigio"		},	{ AT_GREY,	  "AT_GREY",	"grigio"		},	{ "\033[0;37m",	"\033[47m",	"\033[0;5;37m" },	"</font>" },
	{ 'z', { AT_DGREY,	 "AT_DGREY",	"grigio scuro"	},	{ AT_DGREY,	  "AT_DGREY",	"grigio scuro"	},	{ "\033[1;30m",	"\033[40m",	"\033[1;5;30m" },	"<font color=#7F7F7F>" },
	{ 'R', { AT_RED,	 "AT_RED",		"rosso"			},	{ AT_RED,	  "AT_RED",		"rosso"			},	{ "\033[1;31m",	"\033[41m",	"\033[1;5;31m" },	"<font color=#FF0000>" },
	{ 'G', { AT_GREEN,	 "AT_GREEN",	"verde"			},	{ AT_GREEN,	  "AT_GREEN",	"verde"			},	{ "\033[1;32m",	"\033[42m",	"\033[1;5;32m" },	"<font color=#00FF00>" },
	{ 'Y', { AT_YELLOW,	 "AT_YELLOW",	"giallo"		},	{ AT_YELLOW,  "AT_YELLOW",	"biondo"		},	{ "\033[1;33m",	"\033[43m",	"\033[1;5;33m" },	"<font color=#FFFF00>" },
	{ 'B', { AT_BLUE,	 "AT_BLUE",		"blu"			},	{ AT_BLUE,	  "AT_BLUE",	"blu"			},	{ "\033[1;34m",	"\033[44m",	"\033[1;5;34m" },	"<font color=#0000FF>" },
	{ 'P', { AT_PINK,	 "AT_PINK",		"rosa"			},	{ AT_PINK,	  "AT_PINK",	"rosa"			},	{ "\033[1;35m",	"\033[45m",	"\033[1;5;35m" },	"<font color=#FF00FF>" },
	{ 'C', { AT_LCYAN,	 "AT_LCYAN",	"azzurro"		},	{ AT_LCYAN,	  "AT_LCYAN",	"azzurro"		},	{ "\033[1;36m",	"\033[46m",	"\033[1;5;36m" },	"<font color=#00FFFF>" },
	{ 'W', { AT_WHITE,	 "AT_WHITE",	"bianco"		},	{ AT_WHITE,	  "AT_WHITE",	"bianco"		},	{ "\033[1;37m",	"\033[47m",	"\033[1;5;37m" },	"<font color=#FFFFFF>" },
	/* NONE, serve a gestire cose come indicare che non vi è nessun colore che illumini gli occhi */
	{ 'N', { AT_NONE,	 "AT_NONE",		"nessuno"		},	{ AT_NONE,	  "AT_NONE",	"nessuno"		},	{ "",			"",			""			   },	"" }
};
const int max_check_color = sizeof(table_color)/sizeof(struct color_type);


/*
 * Passata la categoria di un synonym che rappresenta un colore cerca nella table_color
 *	il relativo carattere
 */
char get_clr( const int category )
{
	if ( category < 0 || category >= MAX_COLOR )
	{
		send_log( NULL, LOG_BUG, "get_clr: valore category errato: %d", category );
		return 'w';
	}

	return table_color[category].clr;
}


/*===============================================================================================*/


/*
 * Schema dei colori del mud di default.
 */
const struct color_set_type color_default_set[] =
{
	{ "nero",			AT_BLACK			},
	{ "scuro-rosso",	AT_DRED				},
	{ "scuro-verde",	AT_DGREEN			},
	{ "marrone",		AT_ORANGE			},
	{ "scuro-blu",		AT_DBLUE			},
	{ "magenta",		AT_MAGENTA			},
	{ "ciano",			AT_CYAN				},
	{ "grigio",			AT_GREY				},
	{ "scuro-grigio",	AT_DGREY			},
	{ "rosso",			AT_RED				},
	{ "verde",			AT_GREEN			},
	{ "giallo",			AT_YELLOW			},
	{ "blu",			AT_BLUE				},
	{ "rosa",			AT_PINK				},
	{ "azzurro",		AT_LCYAN			},
	{ "bianco",			AT_WHITE			},

	{ "blink",			AT_RED+AT_BLINK		},
	{ "plain",			AT_GREY				},
	{ "action",			AT_GREY				},
	{ "murmur",			AT_MAGENTA			},
	{ "whisper",		AT_MAGENTA			},	/* 20 */
	{ "say",			AT_WHITE			},
	{ "yells",			AT_YELLOW			},
	{ "shout",			AT_YELLOW			},
	{ "tell",			AT_PINK				},
	{ "hit",			AT_WHITE			},	/* 25 */
	{ "hitme",			AT_YELLOW			},
	{ "admin",			AT_YELLOW			},
	{ "hurting",		AT_GREY				},
	{ "falling",		AT_WHITE + AT_BLINK	},
	{ "danger",			AT_RED + AT_BLINK	},	/* 30 */
	{ "magic",			AT_BLUE				},
	{ "consider",		AT_GREY				},
	{ "poison",			AT_DGREEN			},
	{ "social",			AT_GREY				},
	{ "dying",			AT_YELLOW			},	/* 35 */
	{ "dead",			AT_RED				},
	{ "skills",			AT_GREEN			},
	{ "carnage",		AT_DRED				},
	{ "damage",			AT_WHITE			},
	{ "fleeing",		AT_YELLOW			},	/* 40 */
	{ "rmname",			AT_RED				},
	{ "rmdesc",			AT_GREY				},
	{ "objects",		AT_GREY				},
	{ "person",			AT_GREY				},
	{ "list",			AT_BLUE				},	/* 45 */
	{ "bye",			AT_GREEN			},
	{ "gold",			AT_YELLOW			},
	{ "gtells",			AT_BLUE				},
	{ "note",			AT_GREEN			},
	{ "hungry",			AT_ORANGE			},	/* 50 */
	{ "thirsty",		AT_BLUE				},
	{ "fire",			AT_RED				},
	{ "sober",			AT_WHITE			},
	{ "wearoff",		AT_YELLOW			},
	{ "exits",			AT_YELLOW			},	/* 55 */
	{ "score",			AT_LCYAN			},
	{ "reset",			AT_DGREEN			},
	{ "log",			AT_MAGENTA			},
	{ "die_msg",		AT_WHITE			},
	{ "arena",			AT_GREY				},	/* 60 */
	{ "think",			AT_RED				},
	{ "aflags",			AT_BLUE				},
	{ "who",			AT_RED				},
	{ "divider",		AT_PLAIN			},
	{ "morph",			AT_GREY				},	/* 65 */
	{ "rflags",			AT_BLUE				},
	{ "stype",			AT_BLUE				},
	{ "aname",			AT_BLUE				},
	{ "score2",			AT_GREY				},
	{ "who2",			AT_YELLOW			},	/* 70 */
	{ "helpfiles",		AT_DGREEN			},
	{ "prac",			AT_DGREEN			},
	{ "prac2",			AT_CYAN				},
	{ "comando",		AT_GREEN			},	/* 74 */
	{ "cantare",		AT_GREY				}
};


/*===============================================================================================*/


/*
 * Color align functions by Justice@Aaern
 */
int strlen_nocolor_const( const char *argument )
{
	int		str;
	int		count = 0;
	bool	color = FALSE;

	if ( !VALID_STR(argument) )
		return 0;

	for ( str = 0;  argument[str] != '\0';  str++ )
	{
		if ( argument[str] == '&' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '^' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '}' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else
		{
			if ( !color )
				count++;
			else
				color = FALSE;
		}
	}

	return count;
}


int strnlen_nocolor_const( const char *argument, int maxlength )
{
	int		str;
	int		count = 0;
	bool	color = FALSE;

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "strnlen_nocolor_const: argument è NULL" );
		return 0;
	}

	for ( str = 0;  argument[str] != '\0';  str++ )
	{
		if ( argument[str] == '&' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '^' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '}' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else
		{
			if ( !color )
				count++;
			else
				color = FALSE;
		}

		if ( count >= maxlength )
			break;
	}
	if ( count < maxlength )
		return (str - count) + maxlength;

	str++;
	return str;
}


int strlen_nocolor( char *argument )
{
	int		str;
	int		count = 0;
	bool	color = FALSE;

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "strlen_nocolor: argument è NULL" );
		return 0;
	}

	for ( str = 0;  argument[str] != '\0';  str++ )
	{
		if ( argument[str] == '&' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '^' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '}' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else
		{
			if ( !color )
				count++;
			else
				color = FALSE;
		}
	}

	return count;
}


int strnlen_nocolor( char *argument, int maxlength )
{
	int		str;
	int		count = 0;
	bool	color = FALSE;

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "strnlen_nocolor: argument è NULL" );
		return 0;
	}

	for ( str = 0;  argument[str] != '\0';  str++ )
	{
		if ( argument[str] == '&' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '^' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else if ( argument[str] == '}' )
		{
			if ( color )
			{
				count++;
				color = FALSE;
			}
			else
				color = TRUE;
		}
		else
		{
			if ( !color )
				count++;
			else
				color = FALSE;
		}

		if ( count >= maxlength )
			break;
	}
	if ( count < maxlength )
		return (str - count) + maxlength;

	str++;
	return str;
}


/*===============================================================================================*/


#if 0
/*
 * Questi si trovavano in color.h lo si sposta qui perchè inutilizzati
 */
/* --- inizio pezzo color.h */
const char *color_align_const	( const char *argument, int size, int align );

/*
 * Color Alignment Parameters
 */
#define ALIGN_LEFT		1
#define ALIGN_CENTER	2
#define ALIGN_RIGHT		3
/* --- fine pezzo color.h */


const char *color_align_const( const char *argument, int size, int align )
{
	static char buf[MSL];
	int			space = size - strlen_nocolor_const(argument);

	if ( align == ALIGN_RIGHT || strlen_nocolor_const(argument) >= size )
	{
		sprintf( buf, "%*.*s", strnlen_nocolor_const( argument, size ),
			strnlen_nocolor_const(argument, size), argument );
	}
	else if ( align == ALIGN_CENTER )
	{
		sprintf( buf, "%*s%s%*s",
			space/2,
			"",
			argument,
			((space/2)*2 == space)  ?  space/2  :  (space/2)+1,
			"" );
	}
	else
		sprintf( buf, "%s%*s", argument, space, "" );

	return buf;
}


char *color_align( char *argument, int size, int align )
{
	static char buf[MSL];
	int			space = size - strlen_nocolor( argument );

	if ( align == ALIGN_RIGHT || strlen_nocolor(argument) >= size )
	{
		sprintf( buf, "%*.*s", strnlen_nocolor( argument, size ),
			strnlen_nocolor( argument, size ), argument );
	}
	else if ( align == ALIGN_CENTER )
	{
		sprintf( buf, "%*s%s%*s",
			space/2,
			"",
			argument,
			((space/2)*2 == space)  ?  space/2  :  (space/2)+1,
			"" );
	}
	else if ( align == ALIGN_LEFT )
		sprintf( buf, "%s%*s", argument, space, "" );

	return buf;
}
#endif


/*===============================================================================================*/


void show_colors( CHAR_DATA *ch )
{
	int x;

	send_to_pager( ch, "&YSintassi&w: color <tipo colore> <colore>\r\n" );
	send_to_pager( ch, "              Setta il tipo di colore con un colore.\r\n" );
	send_to_pager( ch, "          color <tipo colore> <colore> default.\r\n" );
	send_to_pager( ch, "              Resetta il tipo di colore come default.\r\n" );
	send_to_pager( ch, "          color _reset_\r\n" );
	send_to_pager( ch, "              Resetta tutti i tipi di colori al set di default\r\n" );
	send_to_pager( ch, "          color _all_ <colore>\r\n" );
	send_to_pager( ch, "              Setta tutti i tipi di colori al colore scelto\r\n\r\n" );

	send_to_pager( ch, "&w********************************&W[ COLORI ]&w*********************************\r\n" );

	for ( x = 0;  x < MAX_COLOR;  x++ )
	{
		if ( (x%4) == 0 && x != 0 )
			send_to_pager( ch, "\r\n" );

		pager_printf( ch, "%s%-20s", color_str(x, ch), color_default_set[x].display );
	}

	send_to_pager( ch, "\r\n\r\n&w****************************&W[ TIPI DI COLORI ]&w*****************************\r\n" );

	for ( x = 0;  x < MAX_AT_COLORS; x++ )
	{
		if ( (x%4) == 0 && x != 0 )
			send_to_pager( ch, "\r\n" );

		pager_printf( ch, "%s%-20s%s", color_str(x, ch), color_default_set[x].display, ANSI_RESET );
	}
	send_to_pager( ch, "\r\n" );
}


void reset_colors( CHAR_DATA *ch )
{
	int x;

	for ( x = 0;  x < MAX_AT_COLORS;  x++ )
		ch->pg->colors[x] = color_default_set[x].code;	/* (TT) provare a vedere se funziona anche con la struttura color_data */
}


DO_RET do_colors( CHAR_DATA *ch, char *argument )
{
	bool	dMatch;
	bool	cMatch;
	int		count = 0;
	int		x;
	int		y = 0;
	char	arg[MIL];
	char	arg2[MIL];
	char	arg3[MIL];
	char	log_buf[MSL];


	dMatch = FALSE;
	cMatch = FALSE;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Solo i giocatori possono cambiare i colori.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		show_colors( ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( !str_cmp(arg, "ansitest") )
	{
		for ( x = 0;  x < MAX_COLOR;  x++ )
		{
			sprintf( log_buf, "%s%s\r\n",
				table_color[x].code.name,
				table_color[x].ansi[CLR_FORE] );
			write_to_buffer( ch->desc, log_buf, 0 );
		}
		send_to_pager( ch, "\r\n" );

		for ( x = 0;  x < MAX_COLOR;  x++ )
		{
			sprintf( log_buf, "%s%s\r\n",
				table_color[x].code.name,
				table_color[x].ansi[CLR_BACK] );
			write_to_buffer( ch->desc, log_buf, 0 );
		}
		send_to_pager( ch, "\r\n" );
		write_to_buffer( ch->desc, ANSI_RESET, 0 );

		for ( x = 0;  x < 8;  x++ )
		{
			sprintf( log_buf, "%s%s\r\n",
				table_color[x].code.name,
				table_color[x].ansi[CLR_BLINK] );
			write_to_buffer( ch->desc, log_buf, 0 );
		}
		write_to_buffer( ch->desc, ANSI_RESET, 0 );

		return;
	}

	if ( !str_prefix(arg, "_reset_") )
	{
		reset_colors( ch );
		send_to_char( ch, "All color types reset to default colors.\r\n" );
		return;
	}

	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Change which color type?\r\n" );
		return;
	}

	argument = one_argument( argument, arg3 );
	if ( !str_prefix(arg, "_all_") )
	{
		dMatch = TRUE;
		count = -1;

		/* search for a valid color setting*/
		for ( y = 0;  y < MAX_COLOR ;  y++ )
		{
			if ( !str_cmp(arg2, color_default_set[y].display) )
			{
				cMatch = TRUE;
				break;
			}
		}
	}
	else if ( !VALID_STR(arg2) )
	{
		cMatch = FALSE;
	}
	else
	{
		/* search for the display type and str_cmp*/
		for ( count = 0;  count < MAX_AT_COLORS ;  count++ )
		{
			if ( !str_prefix(arg, color_default_set[count].display) )
			{
				dMatch = TRUE;
				break;
			}
		}

		if ( !dMatch )
		{
			ch_printf( ch, "%s is an invalid color type.\r\n", arg );
			send_to_char( ch, "Type color with no arguments to see available options.\r\n" );
			return;
		}

		if ( !str_cmp(arg2, "default") )
		{
			ch->pg->colors[count] = color_default_set[count].code;
			ch_printf( ch, "Display %s set back to default.\r\n", color_default_set[count].display );
			return;
		}

		/* search for a valid color setting*/
		for ( y = 0;  y < MAX_COLOR ;  y++ )
		{
			if ( !str_cmp(arg2, color_default_set[y].display) )
			{
				cMatch = TRUE;
				break;
			}
		}
	}

	if ( !cMatch )
	{
		if ( arg && arg[0] )
			pager_printf( ch, "Invalid color for type %s.\n", arg );
		else
			send_to_pager( ch, "Invalid color.\r\n" );

		pager_printf( ch, "Choices are:\r\n" );

		for ( count = 0;  count < MAX_COLOR;  count++ )
		{
			if ( (count%5) == 0 && count != 0 )
				send_to_pager( ch, "\r\n" );

			pager_printf( ch, "%-10s", color_default_set[count].display );
		}

		pager_printf( ch, "%-10s\r\n", "default" );
		return;
	}
	else
	{
		pager_printf( ch, "Color type %s set to color %s.\r\n",
			(count == -1)  ?  "_all_"  :  color_default_set[count].display,
			color_default_set[y].display );
	}

	if ( !str_cmp(arg3, "blink") )
		y += AT_BLINK;

	if ( count == -1 )
	{
		int ccount;

		for ( ccount = 0;  ccount < MAX_AT_COLORS;  ccount++ )
			ch->pg->colors[ccount] = y;

		set_char_color( y, ch );

		if ( y > AT_BLINK )
		{
			pager_printf( ch, "All color types set to color %s%s.%s\r\n",
				color_default_set[y-AT_BLINK].display, " [BLINKING]", ANSI_RESET );
		}
		else
		{
			pager_printf( ch, "All color types set to color %s%s.%s\r\n",
				color_default_set[y].display, "", ANSI_RESET );
		}
	}
	else
	{
		ch->pg->colors[count] = y;

		set_char_color( count , ch );

		if ( !str_cmp(arg3, "blink") )
		{
			pager_printf( ch, "Display %s set to color %s [BLINKING]%s\r\n",
				color_default_set[count].display, color_default_set[y-AT_BLINK].display, ANSI_RESET );
		}
		else
		{
			pager_printf( ch, "Display %s set to color %s.\r\n",
				color_default_set[count].display, color_default_set[y].display );
		}
	}
	set_char_color( AT_PLAIN , ch );
}


/*===============================================================================================*/


char *color_str( int AType, CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "color_str: ch è NULL!" );
		return "";
	}

	ch = get_original( ch );

	if ( IS_MOB(ch) || (IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_COLOR)) )
		return "";

	if ( ch->pg->colors[AType] >= 0 && ch->pg->colors[AType] < 32 )
	{
		/* Se minore di sedici invia i foreground color, altrimenti i blinking */
		if ( ch->pg->colors[AType] < MAX_COLOR )
			return table_color[ch->pg->colors[AType]].ansi[CLR_FORE];
		else
			return table_color[ch->pg->colors[AType]-MAX_COLOR].ansi[CLR_BLINK];
	}

	send_log( NULL, LOG_BUG, "color_str: colore AType del pg errato: %d",
		ch->pg->colors[AType] );

	return ANSI_RESET;
}


void set_char_color( int AType, CHAR_DATA *ch )
{
	CHAR_DATA *och;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "set_char_color: ch è NULL" );
		return;
	}

	och = get_original( ch );

	if ( !och || !ch->desc )
		return;

	write_to_buffer( ch->desc, color_str(AType, ch), 0 );
	ch->desc->pagecolor = och->pg->colors[AType];
}


/*===============================================================================================*/


int colorcode( const char *col, char *code, CHAR_DATA *ch )
{
	const char *ctype = col;
	int			ln;
	bool		ansi;

	if ( ch == NULL || ((ch = get_original(ch)) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_COLOR)) )
		ansi = TRUE;
	else
		ansi = FALSE;

	col++;
	if ( !VALID_STR(col) )
		ln = -1;
	else if ( *ctype != '&' && *ctype != '^' && *ctype != '}' )
	{
		send_log( NULL, LOG_BUG, "colorcode ctype '%c' non è '&', '^' o '}'", *ctype );
		ln = -1;
	}
	else if ( *col == *ctype )
	{
		code[0] = *col;
		code[1] = '\0';
		ln = 1;
	}
	else if ( !ansi )
		ln = 0;
	else
	{
		int mode;
		bool found = FALSE;
		int x;

		if		( ctype[0] == '&' )	mode = CLR_FORE;
		else if ( ctype[0] == '^' )	mode = CLR_BACK;
		else if ( ctype[0] == '}' )	mode = CLR_BLINK;	/* Anche se sa già che qui è il carattere '}' poiché passa il test sopra c'è cmq il test per prevenzione */
		else
		{
			send_log( NULL, LOG_BUG, "colorcode: ctype '%c' non è '&', '^' o '}'", *ctype );
			mode = CLR_FORE;
		}

		for ( x = 0;  x < MAX_COLOR;  x++ )
		{
			if ( col[0] == table_color[x].clr )
			{
				strcpy( code, table_color[x].ansi[mode] );
				found = TRUE;
				break;
			}
		}

		if ( !found )
		{
			if ( mode == CLR_FORE )
			{
				switch ( *col )
				{
				  default:
					code[0] = *ctype;
					code[1] = *col;
					code[2] = '\0';
					return 2;
				  case 'i':	case 'I':	strcpy( code, ANSI_ITALIC	 );		break;	/* Italic text */
				  case 'v': case 'V':	strcpy( code, ANSI_REVERSE	 );		break;	/* Reverse colors */
				  case 'u': case 'U':	strcpy( code, ANSI_UNDERLINE );		break;	/* Underline */
				  case 's': case 'S':	strcpy( code, ANSI_STRIKEOUT );		break;	/* Strikeover */
				  case 'd':				strcpy( code, ANSI_RESET	 );		break;	/* Player's client default color */
				  /* Reset to custom color for whatever is being displayed.
				   * Yes, this reset here is quite necessary to cancel out other things */
				  case 'D':
					strcpy( code, ANSI_RESET );
					strcat( code, color_str(ch->desc->pagecolor, ch) );
					break;
				}
			}
			else
			{
				code[0] = *ctype;
				code[1] = *col;
				code[2] = '\0';
				return 2;
			}
		}

		ln = strlen( code );
	}

	if ( ln <= 0 )
		*code = '\0';

	return ln;
}


/*===============================================================================================*/


/*
-------------------------------------------------------------------------------
 * Rimuove i codici di colore da una stringa.
 * - Gavin - ur_gavin@hotmail.com
 * - Unknown Regions - http://ur.lynker.com
-------------------------------------------------------------------------------
 * (FF) da vedere in quali posti è meglio inserirla.
 */
char *strip_color( char *text )
{
	char		*buf;
	static char	 done[MIL*2];
	int			 i = 0;
	int			 j = 0;

	if ( !VALID_STR(text) )
		return NULL;

	done[0] = '\0';

	if ( (buf = (char *) malloc(strlen(text) * sizeof(text))) == NULL )
		return text;

	/* Loop through until you've hit your terminating 0 */
	while ( text[i] != '\0' )
	{
		while ( text[i] == '&' )
			i += 2;

		if ( text[i] != '\0' )
		{
			if ( isspace(text[i]) )
			{
				buf[j] = ' ';
				i++;
				j++;
			}
			else
			{
				buf[j] = text[i];
				i++;
				j++;
			}
		}
		else
			buf[j] = '\0';
	}

	buf[j] = '\0';

	strcpy( done, buf );
	buf = realloc( buf, sizeof(char) * j );
	free( buf );

	return done;
}


/*
 * Invia un colore a caso
 */
char random_color( void )
{
	return	table_color[number_range(1, MAX_COLOR-1)].clr;
}


/*
 * Aggiunge il colore &w alla fine della stringa se nella stessa vi sono dei colori.
 * (FF) Bisognerebbe evitare di strcattare &w nel caso che nella stringa
 *	vi siano solo uno o più &&.
 */
char *strcolor_close( const char *string )
{
	static char	buf[MSL*2];

	strcpy( buf, string );

	if ( strchr(string, '&') != NULL )
		strcat( buf, "&w" );

	return buf;
}
