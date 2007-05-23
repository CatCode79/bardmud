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


/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *       _/          _/_/_/     _/    _/     _/    ACK! MUD is modified    *
 *      _/_/        _/          _/  _/       _/    Merc2.0/2.1/2.2 code    *
 *     _/  _/      _/           _/_/         _/    (c)Stephen Zepp 1998    *
 *    _/_/_/_/      _/          _/  _/             Version #: 4.3          *
 *   _/      _/      _/_/_/     _/    _/     _/                            *
 *                                                                         *
 *                        http://ackmud.nuc.net/                           *
 *                        zenithar@ackmud.nuc.net                          *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/* This code inspired by a snippet from :                                  */

/************************************************************************/
/* mlkesl@stthomas.edu  =====>  Ascii Automapper utility                */
/* Let me know if you use this. Give a newbie some _credit_,            */
/* at least I'm not asking how to add classes...                        */
/* Also, if you fix something could ya send me mail about, thanks       */
/* PLEASE mail me if you use this or like it, that way I will keep it up*/
/************************************************************************/

/*
 * Ported to DOTDII MUD (http://www.dotd.com) by Garil 6-15-99
 */


/*
TODO:
- Se la room e nell'oscurità non vengono elencate le uscite porta, ora come ora indica invece così:
 Sud        - Troppo scuro per saperlo.
- Se la room è oscura non deve mapparla
- Non riesce a beccare la porta verso sud della mistress, invece verso nord la vede e anche oltre.
- Cofrontare le stanze #2155 e #10300 per la porta
- mapper, come room/uscite, puntatori, NULL / NULL di volta in volta.
*/


#include <ctype.h>

#include "mud.h"
#include "command.h"
#include "group.h"
#include "interpret.h"
#include "movement.h"
#include "room.h"


/*
 * Definizioni locali
 */
#define MAX_MAP		100
#define MAX_MAP_DIR	10


#define LINK_CLOSE_NS		-1
#define LINK_CLOSE_EW		-2
#define LINK_CLOSE_NE_SW	-3
#define LINK_CLOSE_SE_NW	-4
#define LINK_NS				-5
#define LINK_EW				-6
#define LINK_NE_SW			-7
#define LINK_SE_NW			-8
#define LINK_DOOR_NS		-9
#define LINK_DOOR_EW		-10
#define LINK_DOOR_NE_SW		-11
#define LINK_DOOR_SE_NW		-12
#define LINK_SWIM_NS		-13
#define LINK_SWIM_EW		-14
#define LINK_SWIM_NE_SW		-15
#define LINK_SWIM_SE_NW		-16
#define LINK_FLY_NS			-17
#define LINK_FLY_EW			-18
#define LINK_FLY_NE_SW		-19
#define LINK_FLY_SE_NW		-20
#define LINK_CLIMB_NS		-21
#define LINK_CLIMB_EW		-22
#define LINK_CLIMB_NE_SW	-23
#define LINK_CLIMB_SE_NW	-24
#define LINK_WINDOW			-25
#define LINK_NULL			-26


#define LOS_INITIAL	-5
#define MAP_Y		9

struct room_content_type
{
	char string[10];
};

/*
 * Variabili locali
 */
struct room_content_type map_contents[MAX_MAP][MAX_MAP];
int		map[MAX_MAP][MAX_MAP];
char	last_back;				/* ultimo colore di background inviato */
char	last_fore;				/* ultimo colore di foreground inviato */


/*
 * Struttura per la creazione della mappa
 */
struct map_info_type
{
	int		 sector;
	char	*color_back;
	char	*color_fore;
	char	*color_invert;
	char	*symbol;
	char	*desc;
};


#define safe_strcat( len, dest, src )	strncat( (dest), (src), (len)-strlen(dest) )

int door_marks[10][2] = { {-1, 0}, {0, 1}, {1,  0}, { 0, -1},	/* n, e, s, w */
						  {-1, 1}, {1, 1},						/* u, d */
						  {-1, 1}, {-1, -1}, {1, 1}, {1, -1} };	/* ne, nw, se, sw (CC) */

int offsets	  [10][2] = { {-2, 0}, {0, 2}, {2,  0}, { 0, -2},	/* n, e, s, w */
						  {-1, 1}, {1, 1},						/* u, d */
						  {-2, 2}, {-2, -2}, {2, 2}, {2, -2} };	/* ne, nw, se, sw (CC) */


const struct map_info_type door_info[26] =
{
/*		tipo uscita		back	fore	inv		symb	nome uscita */
	{ LINK_NS,			"z",	"W",	"",		"|",	"Uscita Nord/Sud"				},
	{ LINK_EW,			"z",	"W",	"",		"-",	"Uscita Est/Ovest"				},
	{ LINK_NE_SW,		"z",	"W",	"",		"/",	"Uscita Nordest/Sudovest"		},
	{ LINK_SE_NW,		"z",	"W",	"",		"\\",	"Uscita Sudest/Nordovest"		},

	{ LINK_DOOR_NS,		"z",	"O",	"",		"|",	"Porta aperta Nord/Sud"			},
	{ LINK_DOOR_EW,		"z",	"O",	"",		"-",	"Porta aperta Est/Ovest"		},
	{ LINK_DOOR_NE_SW,	"z",	"O",	"",		"/",	"Porta aperta Nordest/Sudovest"	},
	{ LINK_DOOR_SE_NW,	"z",	"O",	"",		"\\",	"Porta aperta Sudest/Nordovest"	},

	{ LINK_CLOSE_NS,	"z",	"O",	"",		"+",	"Porta chiusa Nord/Sud"			},
	{ LINK_CLOSE_EW,	"z",	"O",	"",		"+",	"Porta chiusa Est/Ovest"		},
	{ LINK_CLOSE_NE_SW,	"z",	"O",	"",		"x",	"Porta chiusa Nordest/Sudovest"	},
	{ LINK_CLOSE_SE_NW,	"z",	"O",	"",		"x",	"Porta chiusa Sudest/Nordovest"	},

	{ LINK_SWIM_NS,		"z",	"B",	"",		"|",	"Nuotabile Nord/Sud"			},
	{ LINK_SWIM_EW,		"z",	"B",	"",		"-",	"Nuotabile Est/Ovest"			},
	{ LINK_SWIM_NE_SW,	"z",	"B",	"",		"/",	"Nuotabile Nordest/Sudovest"	},
	{ LINK_SWIM_SE_NW,	"z",	"B",	"",		"\\",	"Nuotabile Sudest/Nordovest"	},

	{ LINK_FLY_NS,		"z",	"C",	"",		"|",	"Volabile Nord/Sud"				},
	{ LINK_FLY_EW,		"z",	"C",	"",		"-",	"Volabile Est/Ovest"			},
	{ LINK_FLY_NE_SW,	"z",	"C",	"",		"/",	"Volabile Nordest/Sudovest"		},
	{ LINK_FLY_SE_NW,	"z",	"C",	"",		"\\",	"Volabile Sudest/Nordovest"		},

	{ LINK_CLIMB_NS,	"z",	"z",	"",		"|",	"Scalabile Nord/Sud"			},
	{ LINK_CLIMB_EW,	"z",	"z",	"",		"-",	"Scalabile Est/Ovest"			},
	{ LINK_CLIMB_NE_SW,	"z",	"z",	"",		"/",	"Scalabile Nordest/Sudovest"	},
	{ LINK_CLIMB_SE_NW,	"z",	"z",	"",		"\\",	"Scalabile Sudest/Nordovest"	},

	{ LINK_WINDOW,		"z",	"C",	"",		"#",	"Finestra o portafinestra"		},

	{ LINK_NULL,		"z",	"w",	"",		"",		"Non valida"					}
};


#define SECTOR_BLOCKED	MAX_SECTOR
#define SECTOR_UNSEEN		MAX_SECTOR + 1
#define SECTOR_HERE		MAX_SECTOR + 2

const struct map_info_type map_info[] =
{
/*		settore			 back	fore	inv		symb	nome */
	{ SECTOR_INSIDE,       "w",	"z",	"",		"=",	"interno"			},
	{ SECTOR_CITY,         "w",	"W",	"",		" ",	"città"				},
	{ SECTOR_FIELD,        "g",	"G",	"",		"\"",	"pianura"			},
	{ SECTOR_FOREST,       "g",	"G",	"",		"@",	"foresta"			},
	{ SECTOR_HILLS,        "g",	"Y",	"",		"~",	"collina"			},
	{ SECTOR_MOUNTAIN,     "O",	"Y",	"",		"^^",	"montagna"			},
	{ SECTOR_WATER_SWIM,   "b",	"C",	"",		" ",	"acque basse"		},
	{ SECTOR_WATER_NOSWIM, "b",	"C",	"",		"~",	"acque profonde"	},
	{ SECTOR_UNDERWATER,   "b",	"C",	"",		"°",	"sott'acqua"		},
	{ SECTOR_AIR,          "c",	"C",	"",		" ",	"aria"				},
	{ SECTOR_DESERT,       "O",	"Y",	"",		" ",	"deserto"			},
	{ SECTOR_DUNNO,        "O",	"Y",	"",		"~",	"sconosciuto"		},
	{ SECTOR_OCEANFLOOR,   "O",	"C",	"",		"°",	"fondale marino"	},
	{ SECTOR_UNDERGROUND,  "O",	"Y",	"",		"=",	"sottoterra"		},
	{ SECTOR_LAVA,			"r",	"Y",	"",		"%",	"lava"				},	/* Questo simbolo bisogna inviarlo con la send_to_char e non con la ch_printf, oppure ricordarsi di cambiarlo in %% al caso */
	{ SECTOR_SWAMP,			"O",	"G",	"",		"\"",	"palude"			},
	{ SECTOR_BLOCKED,		"z",	"w",	"",		" ",	"bloccato"			},
	{ SECTOR_UNSEEN,		"z",	"w",	"",		" ",	"invisibile"		},
	{ SECTOR_HERE,			"w",	"R",	"",		"*",	"tu!"				}
};


char *get_sector_display( int sector )
{
	int x;

	for ( x = 0;  x < MAX_SECTOR+3;  x++ )
	{
		if ( map_info[x].sector == sector )
			break;
	}

	return map_info[x].symbol;
}

char *get_sector_color( int sector )
{
	static char	col[MIL];
	int			x;

	for ( x = 0;  x < MAX_SECTOR;  x++ )
	{
		if ( map_info[x].sector == sector )
			break;
	}

	col[0] = '\0';
	if ( map_info[x].color_back[0] != last_back )
	{
		strcat( col, "^" );
		strcat( col, map_info[x].color_back );
		last_back = map_info[x].color_back[0];
	}
	if ( map_info[x].color_fore[0] != last_fore )
	{
		strcat( col, "&" );
		strcat( col, map_info[x].color_fore );
		last_fore = map_info[x].color_fore[0];
	}

	return col;
}

char *get_invert_color( int sector )
{
	static char	col[MIL];
	int			x;

	for ( x = 0;  x < MAX_SECTOR;  x++ )
	{
		if ( map_info[x].sector == sector )
			break;
	}

	col[0] = '\0';
	if ( map_info[x].color_invert[0] != last_fore )
	{
		strcat( col, "&" );
		strcat( col, map_info[x].color_invert );
		last_fore = map_info[x].color_invert[0];
	}

	return col;
}


char *get_door_display( int door )
{
	int x;

	for ( x = 0;  ;  x++ )
	{
		if ( door_info[x].sector == door
		  || door_info[x].sector == LINK_NULL )
			break;
	}

	return door_info[x].symbol;
}

char *get_door_color( int door )
{
	static char	col[MIL];
	int			x;

	for ( x = 0;  ; x++ )
	{
		if ( door_info[x].sector == door
		  || door_info[x].sector == LINK_NULL )
			break;
	}

	col[0] = '\0';
	if ( door_info[x].color_back[0] != last_back )
	{
		strcat( col, "^" );
		strcat( col, door_info[x].color_back );
		last_back = door_info[x].color_back[0];
	}
	if ( door_info[x].color_fore[0] != last_fore )
	{
		strcat( col, "&" );
		strcat( col, door_info[x].color_fore );
		last_fore = door_info[x].color_fore[0];
	}

	return col;
}


/*
 * This code written by Altrag for Ack!Mud
 */
#define iseol(c)	( (c)=='\r' || (c)=='\n' )


/*
 * Like one_argument but saves case, and if len isnt NULL, fills it in with the length.
 * Also accounts for color.
 * Does NOT account for quoted text.
 */
char *break_arg( char *str, char *first_arg, int bufsize, int max, int *buflen, int *len )
{
	int		slen=0;
	char	*arg;

	while ( isspace(*str) )
		++str;

	if ( *str == '\\' && str[1] == 'b' && str[2] == 'r' )
	{
		strcpy( first_arg, "\r\n" );
		if ( buflen )
			*buflen = 0;
		if ( len )
			*len = 0;
		str += 3;
		while ( isspace(*str) )
			++str;
		return str;
	}

	arg = first_arg;
	while ( *str && arg-first_arg < bufsize && slen < max )
	{
		if ( isspace(*str) )
		{
			++str;
			break;
		}
		else if ( *str == '@' && str[1] == '@' && str[2] != '\0' )
		{
			if ( arg-first_arg >= max-3 )
				break;
			*arg++ = *str++;
			*arg++ = *str++;
			*arg++ = *str++;
		}
		else if ( *str == '\\' && str[1] == 'b' && str[2] == 'r' )
		{
			break;
		}
		else
		{
			*arg++ = *str++;
			++slen;
		}
	}

	*arg='\0';
	if ( len )
		*len = slen;
	if ( buflen )
		*buflen = arg - first_arg;
	while ( isspace(*str) )
		++str;

	return str;
}


char *string_justify( char *str, int len, int width, int numwords, int *rlen )
{
	static char	buf[MSL];
	char		arg[MIL];
	int			minspaces = numwords-1;
	int			spaces_needed;
	float		space_ratio,
				space_between;
	int			i,
				j = 0,
				alen;
	char		*bp = buf;

	spaces_needed = minspaces + ( width-(len+1) );
	if ( spaces_needed <= minspaces || minspaces <= 0 )
	{
		sprintf( buf, "%s\r\n", str );
		return buf;
	}
	space_ratio = (float) spaces_needed / (float) minspaces;
	space_between = space_ratio;
	for ( i = 0; i < minspaces; ++i )
	{
		str = break_arg( str, arg, sizeof(arg), width, &alen, NULL );
		strcpy( bp, arg );
		bp += alen;
		for ( ;  j < (int)(space_between+0.5);  ++j )
			*bp++ = ' ';
		space_between += space_ratio;
	}
	str = break_arg( str, arg, sizeof(arg), width, &alen, NULL );
	strcpy( bp, arg );
	bp += alen;
/*	bp += sprintf( bp, "\r\n%d:%d:%d", len, width, numwords );
	bp += sprintf( bp, "\r\n%d:%d:%f", minspaces, spaces_needed, space_ratio );
*/
	*bp++ = '\r';
	*bp++ = '\n';
	*bp = '\0';
	if ( rlen )
		*rlen = bp-buf;

	return buf;
}


#if 0
/*
 * Aggiunge gli spazi necessari per raggiungere la fine della cornice score.
 * Non conta i codici del colore grazie alla strlen_nocolor.
 * prebuf è quello che bisogna inserire prima del buf passatogli
 * postbuf è quello che bisogna inserire dopo il buf passatogli,
 *	se contiene degli spazi dopo vengono contati come bordo esterno al massimo scelto
 *	dal pg come colonna.
 * book, è un oggetto, solitamente un libro, che contiene delle info per formattare adeguatamente
 *	il testo (2-3 colonne, parole scritte alla rovescia, leggibilità o no, scramble di lingue,
 *	lunghezza e altezza libro, che se maggiori della colonna del giocatore vengono visualizzati al centro
 *	lunghezza bordo alto e basso e margini magari, come anche stile riga alto e basso, per pergamene soprattutto
 *	magari ci si può mettere delle chicche quali il segnalibro.)
 * Nel qual caso non vi sia un oggetto si passa NULL si passa il testo senza controlli a riguardo.
 * A volte è comodo creare una struttura BOOK_DATA fittizzia con le info di formattazione del testo
 *	anche se non esiste nessun libro in mano al pg a riguardo.
 */
char *format_string( CHAR_DATA *ch, char *initial, char *buf; char *final, /*BOOK_DATA *obj*/ )
{
	char result[MSL];	/* stringa che verrà ritornata dopo tutte le varie operazioni */
	int len;			/* lunghezza di buf, presa con la strlen_nocolor, quindi senza i colori contati */
	int len_nocol;		/* lunghezza di buf, presa con la strlen, quindi con i colori contati */
	int len_fin_nocol;	/* lunghezza, senza colori, di post_buf */
	int space_to_add;	/* spazi da aggiungere tra result e post_buf */

	if ( pre_buf)
	/* Copia la prima parte e poi ci attacca il buf */
	strcpy( initial, result );
	strcat( result, buf );

	/* Acquisisce le varie lunghezze che servono sulle stringhe */
	len				= strlen		( result );
	len_nocol		= strlen_nocolor( result );
	len_post_nocol	= strlen_nocolor( final );

	/* Calcola quanti spazi bisogna aggiungere per raggiungere gli 80, (RR) o il valore colonna del pg */
	space_to_add = 80 - len_nocol - len_fin_nocol;		/* (bb) qui ci dovrebbe essere più controlli cmq */

	/* Aggiunge spazi a result se ne necessita */
	while ( space_to_add > 0 )
	{
		result[len++] = ' ';
		space_to_add--;
	}
	result[len] = '\0';
}
#endif


char *map_format( char *str, int start, char cmap[MAP_Y][MSL],
				int *numlines, int term_width, int height, bool unjust )
{
	static char	ret[MSL];
	char		buf[MSL];
	char		arg[MIL];
	char		*pbuf = buf;
	char		*pret = ret;
	char		*sp;
	int			width = (start < MAP_Y)  ?  term_width - 13  :  term_width - 1;
	int			alen;
	int			blen = 0;
	int			len;
	int			currline = start;
	int			last;
	int			numwords = 0;
	int			jlen;

	--height;
	pret[0] = '\0';
	for ( sp = break_arg(str, arg, sizeof(arg), width, &len, &alen);
	  *arg;
	  sp = break_arg(sp, arg, sizeof(arg), width, &len, &alen) )
	{
		blen += alen;
		if ( blen+1 >= width || iseol(*arg) )
		{
			*pbuf++ = '\r';
			*pbuf++ = '\n';
			*pbuf	= '\0';

			if ( currline < MAP_Y )
				pret += sprintf( pret, "%s ", cmap[currline] );

			if ( unjust || iseol(*arg) )
			{
				strcpy( pret, buf );
				pret += pbuf - buf;
			}
			else
			{
				strcpy( pret, string_justify(buf, blen-alen, width, numwords, &jlen) );
				pret += jlen;
			}

			if ( currline == MAP_Y )
				width = term_width - 1;
			pbuf = buf;
			if ( ++currline > height )
				break;

			blen = alen;
			if ( iseol(*arg) )
				arg[0] = '\0';
			numwords = 0;
		}
		else if ( pbuf > buf )
		{
			if ( pbuf-buf > 2 && pbuf[-2] == '@' && pbuf[-3] == '@' )
			{
				if ( pbuf-buf == 3 )
					last = 0;
				else
					last =- 4;
			}
			else
			{
				last = -1;
			}

			if ( last )
			{
				if ( unjust && pbuf[last]=='.' )
				{
					*pbuf++ = ' ';
					++blen;
				}

/*				if ( !iseol(pbuf[last]) ) */
				{
					*pbuf++ = ' ';
					++blen;
				}
			}
		}
		strcpy( pbuf, arg );
		pbuf += len;
		++numwords;
	}

/*	send_log( NULL, LOG_MAPPER, "%d:%d", width, blen ); */

	if ( pbuf > buf )
	{
		if ( pbuf-buf > 2 && pbuf[-2] == '@' && pbuf[-3] == '@' )
		{
			if ( pbuf - buf == 3 )
				last = 0;
			else
				last = -4;
		}
		else
		{
			last = -1;
	}

		if ( last && pbuf[last] != '\r' && pbuf[last] != '\n' )
		{
			*pbuf++ = '\r';
			*pbuf++ = '\n';
			if ( currline < MAP_Y )
				pret += sprintf( pret, "%s ", cmap[currline] );

			++currline;
		}
	}
	*pbuf = '\0';
	strcpy( pret, buf );
	if ( numlines )
		*numlines = currline;

	return ret;
}


void display_map( char *border, char *cmap, CHAR_DATA *ch )
{
	int		cols = 80;
	int		rows = 100;
	char	rdescr[MSL+16];		/* E' MSL + qualcosa perché aggiungo dei caratteri per il capolettera alla descrizione della stanza originale */
	char	bufs[MAP_Y][MSL];
	char	disp[MSL];
	int		y;
	int		ty = MAP_Y - 1;
	char	*x;
	char	*ox;

	strcpy( bufs[0], border );
	strcpy( bufs[ty], border );
	x = cmap;
	for ( y = 1;  y < ty && *x;  ++y )
	{
		while ( *x=='\r' || *x=='\n' )
			++x;
		ox = x;
		while ( *x!='\r' && *x!='\n' && *x!='\0' )
			++x;
		sprintf( bufs[y], "%.*s", (x-ox), ox );
	}

	/* Aggiunge il capilettera alla descrizione della stanza */
	sprintf( rdescr, "&z[&%c%c&z]&w%s",
		get_clr(ch->in_room->area->color->cat),
		ch->in_room->description[0],
		ch->in_room->description+1 );
	strcpy( disp, map_format(rdescr, 0, bufs, &y, cols, rows, TRUE) );

	if ( y < MAP_Y )
	{
		x = disp + strlen( disp );
		while ( y < MAP_Y )
		{
			x += sprintf( x, "%s\r\n", bufs[y] );
			++y;
		}
	}

	send_to_char( ch, disp );
}


void MapArea( ROOM_DATA *room, CHAR_DATA *ch,
			 int x, int y, int min, int max, int line_of_sight )
{
	CHAR_DATA	*victim;
	int			looper;
	EXIT_DATA	*door;
	int			door_type;

	if ( map[x][y] < 0 )
		return;		/* it's a door, not a room in the map */

	/* marks the room as visited */
	map[x][y] = room->sector;

	/* displays seen mobs nearby as numbers */
	for ( looper = 0, victim = room->first_person;  victim;  victim = victim->next_in_room )
	{
		if ( !can_see(ch, victim) )
			continue;

		if ( !is_same_group(ch, victim) )
			looper++;
	}
	if ( looper > 0 )
		sprintf( map_contents[x][y].string, "R%d", UMIN(looper, 9) );
	else
		map_contents[x][y].string[0] = '\0';

	for ( door = room->first_exit;  door;  door = door->next )
	{
		if ( door->vdir == DIR_UP || door->vdir == DIR_DOWN || door->vdir == DIR_SOMEWHERE )
			continue;

		if ( x < min || y < min || x > max || y > max )
			return;

		if ( !get_exit_to(door->to_room, table_dir[door->vdir].rev_dir, room->vnum) )
		{
			map[x][y] = SECTOR_BLOCKED;	/* one way into area OR maze */
			return;
		}

		/* selects door symbol */
		door_type = 0;
		if ( HAS_BIT(door->flags, EXIT_ISDOOR)
		  && (!HAS_BIT(door->flags, EXIT_CLOSED) && !HAS_BIT(door->flags, EXIT_LOCKED)) )
		{
			if		( door->vdir == DIR_NORTH || door->vdir == DIR_SOUTH )
				door_type = LINK_DOOR_NS;
			else if ( door->vdir == DIR_EAST || door->vdir == DIR_WEST )
				door_type = LINK_DOOR_EW;
			else if ( door->vdir == DIR_NORTHEAST || door->vdir == DIR_SOUTHWEST )
				door_type = LINK_DOOR_NE_SW;
			else if ( door->vdir == DIR_SOUTHEAST || door->vdir == DIR_NORTHWEST )
				door_type = LINK_DOOR_SE_NW;
		}
		else if ( HAS_BIT(door->flags, EXIT_ISDOOR)
		  && (HAS_BIT(door->flags, EXIT_CLOSED) || HAS_BIT(door->flags, EXIT_LOCKED)) )
		{
			if		( door->vdir == DIR_NORTH || door->vdir == DIR_SOUTH )
				door_type = LINK_CLOSE_NS;
			else if ( door->vdir == DIR_EAST || door->vdir == DIR_WEST )
				door_type = LINK_CLOSE_EW;
			else if ( door->vdir == DIR_NORTHEAST || door->vdir == DIR_SOUTHWEST )
				door_type = LINK_CLOSE_NE_SW;
			else if ( door->vdir == DIR_SOUTHEAST || door->vdir == DIR_NORTHWEST )
				door_type = LINK_CLOSE_SE_NW;
		}
		else if ( HAS_BIT(door->flags, EXIT_SWIM) )
		{
			if		( door->vdir == DIR_NORTH || door->vdir == DIR_SOUTH )
				door_type = LINK_SWIM_NS;
			else if ( door->vdir == DIR_EAST || door->vdir == DIR_WEST )
				door_type = LINK_SWIM_EW;
			else if ( door->vdir == DIR_NORTHEAST || door->vdir == DIR_SOUTHWEST )
				door_type = LINK_SWIM_NE_SW;
			else if ( door->vdir == DIR_SOUTHEAST || door->vdir == DIR_NORTHWEST )
				door_type = LINK_SWIM_SE_NW;
		}
		else if ( HAS_BIT(door->flags, EXIT_FLY) )
		{
			if		( door->vdir == DIR_NORTH || door->vdir == DIR_SOUTH )
				door_type = LINK_FLY_NS;
			else if ( door->vdir == DIR_EAST || door->vdir == DIR_WEST )
				door_type = LINK_FLY_EW;
			else if ( door->vdir == DIR_NORTHEAST || door->vdir == DIR_SOUTHWEST )
				door_type = LINK_FLY_NE_SW;
			else if ( door->vdir == DIR_SOUTHEAST || door->vdir == DIR_NORTHWEST )
				door_type = LINK_FLY_SE_NW;
		}
		else if ( HAS_BIT(door->flags, EXIT_CLIMB) || HAS_BIT(door->flags, EXIT_xCLIMB) )
		{
			if		( door->vdir == DIR_NORTH || door->vdir == DIR_SOUTH )
				door_type = LINK_CLIMB_NS;
			else if ( door->vdir == DIR_EAST || door->vdir == DIR_WEST )
				door_type = LINK_CLIMB_EW;
			else if ( door->vdir == DIR_NORTHEAST || door->vdir == DIR_SOUTHWEST )
				door_type = LINK_CLIMB_NE_SW;
			else if ( door->vdir == DIR_SOUTHEAST || door->vdir == DIR_NORTHWEST )
				door_type = LINK_CLIMB_SE_NW;
		}
		else if ( HAS_BIT(door->flags, EXIT_WINDOW) )
		{
			door_type = LINK_WINDOW;
		}
		else
		{
			if		( door->vdir == DIR_NORTH || door->vdir == DIR_SOUTH )
				door_type = LINK_NS;
			else if ( door->vdir == DIR_EAST || door->vdir == DIR_WEST )
				door_type = LINK_EW;
			else if ( door->vdir == DIR_NORTHEAST || door->vdir == DIR_SOUTHWEST )
				door_type = LINK_NE_SW;
			else if ( door->vdir == DIR_SOUTHEAST || door->vdir == DIR_NORTHWEST )
				door_type = LINK_SE_NW;
		}

		/* Salta le uscite-portale, anche se dovrebbe farlo cmq sopra con il controllo DIR_SOMEWHERE */
		if ( str_cmp(door->keyword, "") )
			continue;

		/* Se è una uscita nascosta, salta */
		if ( HAS_BIT(door->flags, EXIT_HIDDEN) )
/*		  && ch->pg->learned_skell[gsn_find_doors] > number_percent() */
			continue;

		/* Assegna alla mappa il tipo di porta */
		map[x+door_marks[door->vdir][0]][y+door_marks[door->vdir][1]] = door_type;

		/* Se il tipo di porta è chiusa o bloccata, salta */
		if ( door_type >= LINK_CLOSE_SE_NW )
			continue;

		/* Se la stanza è buia, salta */
/*		if ( HAS_BIT(door->to_room->flags, ROOM_DARK) )
			continue; (BB) non funziona, ci vuole il sistema di pointer room e vnum e non vnum o tipi negativi di uscita */

		/* (BB) Purtroppo non mi visualizza le uscite delle porte chiuse e lockate (o meglio: a volte no pare) */

		/* Se la line of sight non è all'inizio oppure non è la stessa della direzione, salta */
		if ( line_of_sight != LOS_INITIAL && door->vdir != line_of_sight )
			continue;

		/* Se il settore non era ancora stato visitato continua a mappare */
		if ( map[x+offsets[door->vdir][0]][y+offsets[door->vdir][1]] == SECTOR_UNSEEN )
		{
			MapArea( door->to_room, ch,
				x + offsets[door->vdir][0],
				y + offsets[door->vdir][1],
				min, max,
				line_of_sight == LOS_INITIAL ? door->vdir : line_of_sight );
		}
	}
}


DO_RET do_legend( CHAR_DATA *ch, char *argument )
{
	int looper;

	send_to_pager( ch, "Legenda per la Mappa:\r\n" );
	for ( looper = 0;  looper < MAX_SECTOR+3;  looper++ )
	{
		pager_printf( ch, "^%s&%s%s^x&w : %s\r\n" ,
			map_info[looper].color_back,
			map_info[looper].color_fore,
			map_info[looper].symbol,
			map_info[looper].desc );
	}
	send_to_pager( ch, "\r\n" );
	for ( looper = 0;  looper < 26;  looper++ )
	{
		pager_printf( ch, "^%s&%s%s^x&w : %s\r\n" ,
			door_info[looper].color_back,
			door_info[looper].color_fore,
			door_info[looper].symbol,
			door_info[looper].desc );
	}
}


DO_RET do_mapper( CHAR_DATA *ch, char *argument )
{
	char	outbuf[MSL];
	char	catbuf[MSL];
	char	borderbuf[MSL];
	char	colorbuf[MSL];
	char	displaybuf[MSL];
	int		size, center;
	int		x, y;
	int		min, max;
	int		looper;

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		send_command( ch, "legend", CO );
		return;
	}

	size = ( is_number(argument) ) ? atoi (argument) : 21;
	if ( size != 7 )
	{
		if ( size%2  == 0 )
			size += 1;
		size = URANGE( 9, size, MAX_MAP );
	}
	size = URANGE( 3, size, MAX_MAP );

	center = MAX_MAP / 2;

	min = MAX_MAP/2 - size/2;
	max = MAX_MAP/2 + size/2;

	for ( x = 0;  x < MAX_MAP;  ++x )
	{
		for ( y = 0;  y < MAX_MAP;  ++y )
		{
			map[x][y] = SECTOR_UNSEEN;
			map_contents[x][y].string[0] = '\0';
		}
	}

	/* starts the mapping with the center room */
	MapArea( ch->in_room, ch, center, center, min-1, max, LOS_INITIAL );

	/* marks the center, where ch is */
	strcpy( map_contents[center][center].string, "R*" );


	strcpy( outbuf, "\r\n" );
	strcpy( borderbuf, "+" );

	for ( looper = 0;  looper <= size+1;  looper++ )
	{
		strcpy( catbuf, "-" );
		safe_strcat( MSL, borderbuf, catbuf );
	}

	safe_strcat( MSL, borderbuf, "+" );

	for ( x = min;  x <= max;  ++x )
	{ /* every row */

		/* Inizializza le variabili di modulo contenenti gli ultimi colori usati
		 *	nella creazione della mappa per ogni riga */
		last_back = '\0';
		last_fore = '\0';

		safe_strcat( MSL, outbuf, "| " );
		for ( y = min;  y <= max;  ++y )
		{ /* every column */
			if ( (y == min) || (map[x][y-1] != map[x][y]) )
			{
				if ( !VALID_STR(map_contents[x][y].string) )
				{
					strcpy( colorbuf, (map[x][y] < 0) ?
						get_door_color(map[x][y]) : get_sector_color(map[x][y]) );

					strcpy( displaybuf, (map[x][y] < 0) ?
						get_door_display(map[x][y]) : get_sector_display(map[x][y]) );
				}
				else
				{
					sprintf( colorbuf, "%s%s",
						(map[x][y] < 0) ? get_door_color(map[x][y]) : get_sector_color(map[x][y]),
						get_invert_color(map[x][y]) );

					strcpy( displaybuf, (map[x][y] < 0) ?
						get_door_display(map[x][y]) : map_contents[x][y].string );

					if ( map[x][y] >= 0 )
						last_fore = 'R';	/* Colore delle entità nella map_contents */
				}

				sprintf( catbuf, "%s%s", colorbuf, displaybuf );
				safe_strcat( MSL, outbuf, catbuf );
			}
			else
			{
				sprintf( catbuf, "%s",
					(map[x][y] < 0) ? get_door_display(map[x][y]) : get_sector_display(map[x][y]) );
				safe_strcat( MSL, outbuf, catbuf  );
			}
		}
		safe_strcat( MSL, outbuf, " &w|\r\n" );
	}


	if ( size == 7 )
	{
		display_map( borderbuf, outbuf, ch );
	}
	else
	{
		send_to_char( ch, "\r\n" );

		/* this is the top line of the map itself, currently not part of the mapstring */
		send_to_char( ch, borderbuf );

		/* this is the map_contents of the map */
		send_to_char( ch, outbuf );

		/* this is the bottom line of the map */
		send_to_char( ch, borderbuf );
		send_to_char( ch, "\r\n" );
		send_to_char( ch, "Per avere la lista dei simboli prova a digitare 'mapper legenda'\r\n" );
	}
}
