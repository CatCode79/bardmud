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

/************************************ STARMAP ********************************
 * Star Map - by Nebseni of Clandestine MUD. 								 *
 * Ported to Smaug Code by Desden, el Chaman Tibetano (jose@luisso.net)		 *
 *		Web:luisso.net/inicio.htm											 *
 * - July 1999																 *
 *****************************************************************************/

#include "mud.h"
#include "calendar.h"
#include "room.h"


/* Should be the string length and number of the constants below.*/
#define MAP_WIDTH	72
#define MAP_HEIGHT	 8


const char *star_map[] =
{
	"                                               C. C.                  g*",
	"    O:       R*        G*    G.  W* W. W.          C. C.    Y* Y. Y.    ",
	"  O*.                c.          W.W.     W.            C.       Y..Y.  ",
	"O.O. O.              c.  G..G.           W:      B*                   Y.",
	"     O.    c.     c.                     W. W.                  r*    Y.",
	"     O.c.     c.      G.             P..     W.        p.      Y.   Y:  ",
	"        c.                    G*    P.  P.           p.  p:     Y.   Y. ",
	"                 b*             P.: P*                 p.p:             "
};
/****************** CONSTELLATIONS and STARS *****************************
  Cygnus     Mars        Orion      Dragon       Cassiopeia          Venus
           Ursa Ninor                           Mercurius     Pluto
               Uranus              Leo                Crown       Raptor
*************************************************************************/

const char *sun_map[] =
{
"\\`|'/",
"- O -",
"/.|.\\"
};

const char *moon_map[] =
{
" @@@ ",
"@@@@@",
" @@@ "
};


DO_RET do_sky( CHAR_DATA * ch, char *argument )
{
	static char buf[MSL];
	static char buf2[4];
	int			starpos;
	int			sunpos;
	int			moonpos;
	int			moonphase;
	int			x;
	int			linenum;
	int			precip;

	if ( !is_outside(ch) )
	{
		send_to_char( ch, "Non posso vedere il cielo in un luogo chiuso.\r\n" );
		return;
	}

	precip = ( ch->in_room->area->weather->precip + (meteo.weath_unit*3) - 1 ) / meteo.weath_unit;
	if ( precip > 1 )
	{
		send_to_char( ch, "Ci sono alcune nuvole nel cielo e non riesco a vedere altro.\r\n" );
		return;
	}

	send_to_pager( ch, "Alzo lo sguardo verso il cielo..\r\n" );

	sunpos  = ( MAP_WIDTH * (24 - calendar.hour) / 24 );
	moonpos = ( sunpos + calendar.day * MAP_WIDTH / DAYS_IN_MONTH ) % MAP_WIDTH;

	moonphase = ((((MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) + (MAP_WIDTH/16)) * 8 ) / MAP_WIDTH;
	if ( moonphase > 4 )
	  moonphase -= 8;

	starpos = ( sunpos + MAP_WIDTH * calendar.month / MONTHS_IN_YEAR ) % MAP_WIDTH;
	/* The left end of the star_map will be straight overhead at midnight during month 0 */

	for ( linenum = 0;  linenum < MAP_HEIGHT;  linenum++ )
	{
		if ( (calendar.hour >= 6 && calendar.hour <= 18)
		  && (linenum < 3 || linenum >= 6) )
		{
			continue;
		}

		sprintf( buf, " " );


		/* for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)*/
		for ( x = 1;  x <= MAP_WIDTH;  x++ )
		{
			/* plot moon on top of anything else...unless new moon & no eclipse */
			if ( (calendar.hour >= 6 && calendar.hour <= 18 )						/* daytime? */
			  && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2)		/* in sky? */
			  && ( x >= moonpos - 2 ) && (x <= moonpos + 2)							/* is this pixel near moon? */
			  && ((sunpos == moonpos && calendar.hour == 12) || moonphase != 0  )	/* no eclipse */
			  && (moon_map[linenum-3][x+2-moonpos] == '@') )
			{
				if ( (moonphase < 0 && x - 2 - moonpos >= moonphase)
				  || (moonphase > 0 && x + 2 - moonpos <= moonphase) )
					strcat( buf, "&W@" );
				else
					strcat( buf, " " );
			}
			else if ( (linenum >= 3) && (linenum < 6)							/* nighttime */
			  && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2)	/* in sky? */
			  && ( x >= moonpos - 2 ) && (x <= moonpos + 2)						/* is this pixel near moon? */
			  && (moon_map[linenum-3][x+2-moonpos] == '@') )
			{
				if ( (moonphase < 0 && x - 2 - moonpos >= moonphase)
				  || (moonphase > 0 && x + 2 - moonpos <= moonphase) )
					strcat( buf, "&W@" );
				else
					strcat( buf, " " );
			}
			else /* plot sun or stars */
			{
				if ( calendar.hour >= 6 && calendar.hour <= 18)				/* daytime */
				{
					if ( x >= sunpos - 2 && x <= sunpos + 2 )
					{
						sprintf( buf2, "&Y%c", sun_map[linenum-3][x+2-sunpos] );
						strcat( buf, buf2 );
					}
					else
						strcat( buf, " " );
				}
				else
				{
					switch ( star_map[linenum][(MAP_WIDTH + x - starpos)%MAP_WIDTH] )
					{
					  default :		strcat( buf, " " );		break;
					  case ':':		strcat( buf, ":" );		break;
					  case '.':		strcat( buf, "." );		break;
					  case '*':		strcat( buf, "*" );		break;
					  case 'G':		strcat( buf, "&G " );	break;
					  case 'g':		strcat( buf, "&g " );	break;
					  case 'R':		strcat( buf, "&R " );	break;
					  case 'r':		strcat( buf, "&r " );	break;
					  case 'C':		strcat( buf, "&C " );	break;
					  case 'O':		strcat( buf, "&O " );	break;
					  case 'B':		strcat( buf, "&B " );	break;
					  case 'P':		strcat( buf, "&P " );	break;
					  case 'W':		strcat( buf, "&W " );	break;
					  case 'b':		strcat( buf, "&b " );	break;
					  case 'p':		strcat( buf, "&p " );	break;
					  case 'Y':		strcat( buf, "&Y " );	break;
					  case 'c':		strcat( buf, "&c " );	break;
					}
				}
			}
		}
		strcat( buf, "\r\n" );
		send_to_pager( ch, buf );
	}
}
