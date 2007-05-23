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
 >							Informational module							<
\****************************************************************************/

#include <ctype.h>

#include "mud.h"
#include "affect.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "gramm.h"
#include "interpret.h"
#include "liquid.h"
#include "mapper.h"
#include "movement.h"
#include "mprog.h"
#include "mxp.h"
#include "note.h"
#include "olc.h"
#include "morph.h"
#include "room.h"
#include "shop.h"


/*
 * Keep players from defeating examine progs
 * False = do not trigger
 * True = Trigger
 */
bool EXA_prog_trigger = TRUE;


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
	static char buf[MSL];
	bool		glowsee = FALSE;

	/* can see glowing invis items in the dark */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_GLOW) && HAS_BIT(obj->extra_flags, OBJFLAG_INVIS)
	  && !HAS_BIT(ch->affected_by, AFFECT_TRUESIGHT) && !HAS_BIT(ch->affected_by, AFFECT_DETECT_INVIS) )
	{
		glowsee = TRUE;
	}

	buf[0] = '\0';

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVIS) )				strcat( buf, "&w(Invisibile) ");

	if ( IS_ADMIN(ch)
	  || HAS_BIT(ch->affected_by, AFFECT_DETECT_ALIGN)
	  || HAS_BIT(ch->affected_by, AFFECT_DETECT_GOOD)
	  || HAS_BIT(ch->affected_by, AFFECT_DETECT_EVIL) )
	{
		bool see_good = FALSE;
		bool see_evil = FALSE;

		if ( IS_ADMIN(ch) )
		{
			see_good = TRUE;
			see_evil = TRUE;
		}

		if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_ALIGN) )
		{
			if ( is_align(ch, ALIGN_GOOD) )
				see_evil = TRUE;
			if ( is_align(ch, ALIGN_EVIL) )
				see_good = TRUE;
			if ( is_align(ch, ALIGN_NEUTRAL) )
			{
				see_evil = TRUE;
				see_good = TRUE;
			}
		}

		if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_GOOD) )
			see_good = TRUE;
		if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_EVIL) )
			see_evil = TRUE;

		if ( see_evil && HAS_BIT(obj->extra_flags, OBJFLAG_EVIL) )		strcat( buf, "&w(&RAura rossa&w) " );
#ifdef T2_ALFA
		if ( see_good && HAS_BIT(obj->extra_flags, OBJFLAG_GOOD)) )	strcat( buf, "(Aura bianca) " );	/* (RR) da aggiungere OBJFLAG_GOOD dopo la (CC) */
#endif
		if ( see_evil
		  &&  HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_EVIL)
		  && !HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_GOOD) )		strcat( buf, "&w(&WFiamma bianca&w) "  );

		if ( see_good
		  && !HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_EVIL)
		  &&  HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_GOOD) )		strcat( buf, "&w(&RFiamma rossa&w) "  );

		if ( (see_good || see_evil)
		  && HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_EVIL)
		  && HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_GOOD) )		strcat( buf, "&w(Bagliore &Rrosso &we &Wbianco&w) " );
	}

	if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_MAGIC)
	  && HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )				strcat( buf, "&w(&CMagico&w) " );

	if ( !glowsee && HAS_BIT(obj->extra_flags, OBJFLAG_GLOW) )		strcat( buf, "&w(&WLuminoso&w) " );

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_HUM) )					strcat( buf, "&w(Ronzante) " );

	if ( (HAS_BIT(ch->affected_by, AFFECT_DETECT_HIDDEN) || IS_ADMIN(ch))
	  && HAS_BIT(obj->extra_flags, OBJFLAG_HIDDEN) )				strcat( buf, "&w(Nascosto) " );

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_BURIED) )				strcat( buf, "&w(&OSepolto&w) " );

	if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_TRAPS)
	  && is_trapped(obj) )										strcat( buf, "&w(&YTrappola&w) " );

	if ( fShort )
	{
		if ( glowsee && !IS_ADMIN(ch) )
			strcat( buf, "un leggero bagliore di qualcosa" );
		else if ( obj->short_descr )
			strcat( buf, obj->short_descr );
	}
	else
	{
		if ( glowsee )
			strcat( buf, "Vedo un debole bagliore di qualcosa nei pressi." );
		if ( obj->long_descr )
			strcat( buf, obj->long_descr );
	}

	if ( IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_VNUM) )
		sprintf( buf+strlen(buf), " &w(#%d)", obj->vnum );

	return buf;
}


/*
 * Some increasingly freaky hallucinated objects
 * (Hats off to Albert Hoffman's "problem child")
 */
char *hallucinated_object( int ms, bool fShort )
{
	int sms = URANGE( 1, (ms+10)/5, 20 );

	if ( fShort )
	{
		switch ( number_range(6-URANGE(1,sms/2,5), sms) )
		{
		  case  1:	return "una spada";
		  case  2:	return "una bacchetta";
		  case  3:	return "qualcosa di splendente";
		  case  4:	return "qualcosa";
		  case  5:	return "qualcosa di interessante";
		  case  6:	return "qualcosa di colorito";
		  case  7:	return "qualcosa che appare fantastico";
		  case  8:	return "un ingegnoso oggetto ";
		  case  9:	return "una veste dai colori sgargianti";
		  case 10:	return "una mistica e fiammante spada";
		  case 11:	return "uno sciame di insetti";
		  case 12:	return "un foglietto per terra";
		  case 13:	return "un frutto della tua immaginazione";
		  case 14:	return "la tua tomba";
		  case 15:	return "il Perduto Arco del Ranger Wisdul.";
		  case 16:	return "un abbagliante Volume di Arcane Conoscenze";
		  case 17:	return "il Segreto tanto bramato";
		  case 18:	return "il Significato di tutto ciò";
		  case 19:	return "la Risposta";
		  case 20:	return "la Chiave della Vita, dell'Anima e dell'Universo";
		}
	}

	switch ( number_range(6-URANGE(1,sms/2,5), sms) )
	{
	  case  1:	return "Una bella spada cattura il mio sguardo.";
	  case  2:	return "Il suolo è ricoperto di piccole bacchette.";
	  case  3:	return "Qualcosa di splendente cattura il mio sguardo.";
	  case  4:	return "Qualcosa cattura la mia attenzione.";
	  case  5:	return "Qualcosa di interessante cattura il mio sguardo.";
	  case  6:	return "Qualcosa di colorito ondeggia attorno.";
	  case  7:	return "Qualcosa che appare fantastico ricihiama la mia attenzione.";
	  case  8:	return "Un ingegnoso oggetto di grande importanza si trova qui.";
	  case  9:	return "Una veste dai colori sgargianti è come se chiedesse di essere indossata.";
	  case 10:	return "Una misitca e fiammante spada attende la mia stretta.";
	  case 11:	return "Uno sciame di insetti ronza davanti al mio viso!";
	  case 12:	return "Un foglietto per terra, vi è scritto: Non vuoi una quest?.";
	  case 13:	return "Il frutto della mia immaginazione è al tuo comando.";
	  case 14:	return "Ti accorgi che c'è una tomba qui.. e leggo il mio nome su di essa!";
	  case 15:	return "Il Perduto Arco del Ranger Wisdul si trova a portata di mano.";
	  case 16:	return "Un abbagliante Volume di Arcane Conoscenze fluttua nell'aria proprio davanti a me.";
	  case 17:	return "Il tanto a lungo bramato Segreto di tutti i Piani mi è ora chiaro.";
	  case 18:	return "Il significato di tutto ciò, così semplice, così chiaro.. ma certo!";
	  case 19:	return "La Risposta.  Una.  Era sempre stata solo Una.";
	  case 20:	return "La Chiave della Vita, dell'Anime e dell'Universo è tiepida nella mia mano.";
	}

	return "Perdincibacco!!";
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
#ifdef T2_MXP
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing, const int iDefaultAction )
#else
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
#endif
{
	char	**prgpstrShow;
	int		 *prgnShow;
	int		 *pitShow;
	char	 *pstrShow;
	OBJ_DATA *obj;
	int		  nShow;
	int		  x;
	int		  count;
	int		  offcount;
	int		  tmp;
	int		  ms = 0;
	int		  cnt;
	bool	  fCombine;
#ifdef T2_MXP
	char	**prgpstrName;
	char	**prgpstrShortName;
	char	 *pstrName;
	char	 *pstrShortName;
	char	 *pAction = NULL;
#endif

	if ( !ch->desc )
		return;

	/*
	 * If there's no list... then don't do all this crap!
	 */
	if ( !list )
	{
		if ( fShowNothing )
		{
			if ( IS_MOB(ch) || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_COMBINE)) )
				send_to_char( ch, "     " );
			set_char_color( AT_OBJECT, ch );
			send_to_char( ch, "Nulla.\r\n" );
		}
		return;
	}

#ifdef T2_MXP
	/* work out which MXP tag to use */
	switch ( iDefaultAction )
	{
	  case eItemGet:	pAction = "get";		break;	/* get, item on ground		*/
	  case eItemDrop:	pAction = "drp";		break;	/* drop, item in inventory	*/
	}
#endif

	/*
	 * Alloc space for output lines.
	 */
	count = 0;
	for ( obj = list;  obj;  obj = obj->next_content )
		count++;

	if ( ch->mental_state )
		ms = ch->mental_state * (IS_MOB(ch)  ?  1  :  (ch->pg->condition[CONDITION_DRUNK]  ?  (ch->pg->condition[CONDITION_DRUNK]/12)  :  1) );
	else
		ms = 1				  * (IS_MOB(ch)  ?  1  :  (ch->pg->condition[CONDITION_DRUNK]  ?  (ch->pg->condition[CONDITION_DRUNK]/12)  :  1) );

	/*
	 * If not mentally stable...
	 */
	if ( abs(ms) > 40 )
	{
		offcount = URANGE( -(count), (count * ms) / 100, count*2 );
		if		( offcount < 0 )
			offcount += number_range( 0, abs(offcount) );
		else if ( offcount > 0 )
			offcount -= number_range( 0, offcount );
	}
	else
		offcount = 0;

	if ( count + offcount <= 0 )
	{
		if ( fShowNothing )
		{
			if ( IS_MOB(ch) || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_COMBINE)) )
				send_to_char( ch, "     " );
			set_char_color( AT_OBJECT, ch );
			send_to_char( ch, "Nulla.\r\n" );
		}
		return;
	}

	CREATE( prgpstrShow,		char*,	count + ((offcount > 0)  ?  offcount  :  0) );
#ifdef T2_MXP
	CREATE( prgpstrName,		char*,	count + ((offcount > 0)  ?  offcount  :  0) );
	CREATE( prgpstrShortName,	char*,	count + ((offcount > 0)  ?  offcount  :  0) );
#endif
	CREATE( prgnShow,			int,	count + ((offcount > 0)  ?  offcount  :  0) );
	CREATE( pitShow,			int,	count + ((offcount > 0)  ?  offcount  :  0) );
	nShow	= 0;
	tmp		= (offcount > 0)  ?  offcount  :  0;
	cnt		= 0;

	/*
	 * Format the list of objects.
	 */
	for ( obj = list;  obj;  obj = obj->next_content )
	{
		if ( offcount < 0 && ++cnt > (count + offcount) )
			break;

		if ( tmp > 0 && number_bits(1) == 0 )
		{
			prgpstrShow		[nShow] = str_dup( hallucinated_object(ms, fShort) );
#ifdef T2_MXP
			prgpstrName		[nShow] = str_dup( hallucinated_object(ms, TRUE) );
			prgpstrShortName[nShow] = str_dup( hallucinated_object(ms, TRUE) );
#endif
			prgnShow		[nShow] = 1;
			pitShow			[nShow] = number_range( OBJTYPE_LIGHT, OBJTYPE_BOOK );
			nShow++;
			--tmp;
		}

		if ( obj->wear_loc != WEARLOC_NONE )
			continue;

		/* equip a FALSE per qualsiasi lista perché anche se in equip le stringhe descriventi gli oggetti non li deve vedere */
		if ( !can_see_obj(ch, obj, FALSE) )
			continue;

		if ( obj->type == OBJTYPE_TRAP && !HAS_BIT(ch->affected_by, AFFECT_DETECT_TRAPS) )
			continue;

		if ( HAS_BIT(obj->extra_flags, OBJFLAG_SECRET) )
			continue;

		pstrShow = format_obj_to_char( obj, ch, fShort );
#ifdef T2_MXP
		pstrName = obj->name;
		pstrShortName = obj->short_descr;
#endif
		fCombine = FALSE;

		if ( IS_MOB(ch) || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_COMBINE)) )
		{
			/* Look for duplicates, case sensitive.
			 * Matches tend to be near end so run loop backwords. */
			for ( x = nShow - 1;  x >= 0;  x-- )
			{
				if ( !strcmp(prgpstrShow[x], pstrShow) )
				{
					prgnShow[x] += obj->count;
					fCombine = TRUE;
					break;
				}
			}
		}

		pitShow[nShow] = obj->type;
		/*
		 * Couldn't combine, or didn't want to.
		 */
		if ( !fCombine )
		{
			prgpstrShow		[nShow] = str_dup( pstrShow );
#ifdef T2_MXP
			prgpstrName		[nShow] = str_dup( pstrName );
			prgpstrShortName[nShow] = str_dup( pstrShortName );
#endif
			prgnShow		[nShow] = obj->count;
			nShow++;
		}
	}

	if ( tmp > 0 )
	{
		for ( x = 0;  x < tmp;  x++ )
		{
			prgpstrShow		[nShow] = str_dup( hallucinated_object(ms, fShort) );
#ifdef T2_MXP
			prgpstrName		[nShow] = str_dup( hallucinated_object(ms, TRUE) );
			prgpstrShortName[nShow] = str_dup( hallucinated_object(ms, TRUE) );
#endif
			prgnShow		[nShow] = 1;
			pitShow			[nShow] = number_range( OBJTYPE_LIGHT, OBJTYPE_BOOK );
			nShow++;
		}
	}

	/*
	 * Output the formatted list.
	 */
	for ( x = 0;  x < nShow;  x++ )
	{
		switch ( pitShow[x] )
		{
		  default:
			set_char_color( AT_OBJECT, ch );
			break;
		  case OBJTYPE_BLOOD:
			set_char_color( AT_DRED, ch );
			break;
		  case OBJTYPE_MONEY:
		  case OBJTYPE_TREASURE:
			set_char_color( AT_YELLOW, ch );
			break;
		  case OBJTYPE_COOK:
		  case OBJTYPE_FOOD:
			set_char_color( AT_HUNGRY, ch );
			break;
		  case OBJTYPE_DRINK_CON:
		  case OBJTYPE_FOUNTAIN:
			set_char_color( AT_THIRSTY, ch );
			break;
		  case OBJTYPE_FIRE:
			set_char_color( AT_FIRE, ch );
			break;
		  case OBJTYPE_SCROLL:
		  case OBJTYPE_WAND:
		  case OBJTYPE_STAFF:
			set_char_color( AT_MAGIC, ch );
			break;
		}

		if ( fShowNothing )
			send_to_char( ch, "     " );

#ifdef T2_MXP
		if ( pAction )
			ch_printf( ch, MXPTAG("%s \"%s\" \"%s\""), pAction, prgpstrName[x], prgpstrShortName[x] );
#endif
		send_to_char( ch, prgpstrShow[x] );
#ifdef T2_MXP
		if ( pAction )
			ch_printf( ch, MXPTAG("/%s"), pAction );
#endif

		if ( IS_MOB(ch) || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_COMBINE)) )
		{
			if ( prgnShow[x] < 1 )
			{
				send_log( NULL, LOG_BUG, "show_list_to_char: oggetto %s #%d con caricamento reset 0 o meno nella stanza #%d",
					(obj)  ?  obj->short_descr  :  "sconosciuto",
					(obj)  ?  obj->vnum  :  0,
					ch->in_room->vnum );
			}

			if ( prgnShow[x] > 1 )
				ch_printf( ch, " (%d)", prgnShow[x] );
		}

		send_to_char( ch, "\r\n" );
		DISPOSE( prgpstrShow[x] );
#ifdef T2_MXP
		DISPOSE( prgpstrName[x] );
		DISPOSE( prgpstrShortName[x] );
#endif
	}

	if ( fShowNothing && nShow == 0 )
	{
		if ( IS_MOB(ch) || (IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_COMBINE)) )
			send_to_char( ch, "     " );
		set_char_color( AT_OBJECT, ch );
		send_to_char( ch, "Nulla.\r\n" );
	}

	/*
	 * Clean up.
	 */
	DISPOSE( prgpstrShow	  );
#ifdef T2_MXP
	DISPOSE( prgpstrName	  );
	DISPOSE( prgpstrShortName );
#endif
	DISPOSE( prgnShow		  );
	DISPOSE( pitShow		  );
}


/*
 * Show fancy descriptions for certain spell affects.
 */
void show_visible_affects_to_char( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char name[MSL];

	/* (FF) Se invece di questo check per creare il nome si utilizzaressero tutte delle act forse sarebbe meglio */

	if ( IS_MOB( victim ) )
		strcpy( name, victim->short_descr );
	else
		strcpy( name, victim->name);

	name[0] = toupper(name[0]);

	if ( HAS_BIT(victim->affected_by, AFFECT_SANCTUARY) )
	{
		set_char_color( AT_WHITE, ch );

		if ( is_align(victim, ALIGN_GOOD) )
		{
			if ( ch == victim )
				send_to_char( ch, "Splendo con una raggiante aura bianca.&w\r\n" );
			else
				ch_printf( ch, "%s splende con una raggiante aura bianca.&w\r\n", name );
		}
		else if ( is_align(victim, ALIGN_EVIL) )
		{
			if ( ch == victim )
				send_to_char( ch, "Luccico al di là di un'aura di energia scura.&w\r\n" );
			else
				ch_printf( ch, "%s luccica al di là di un'aura di energia scura.&w\r\n", name );
		}
		else
		{
			if ( ch == victim )
				ch_printf( ch, "Sono avvolt%c in luci e ombre fluttuanti.&w\r\n", gramm_ao(victim) );
			else
				ch_printf( ch, "%s è avvolt%c in luci e ombre fluttuanti.&w\r\n", name, gramm_ao(victim) );
		}
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_FIRESHIELD) )
	{
		set_char_color( AT_FIRE, ch );
		if ( ch == victim )
			ch_printf( ch, "Sono inghiottit%c da vampate di fiamme mistiche.&w\r\n", gramm_ao(victim) );
		else
			ch_printf( ch, "%s è inghiottit%c da vampate di fiamme mistiche.&w\r\n", name, gramm_ao(victim) );
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_SHOCKSHIELD) )
	{
		set_char_color( AT_BLUE, ch );
		if ( ch == victim )
			send_to_char( ch, "Emano delle scariche elettriche azzurre di pura energia.&w\r\n" );
		else
			ch_printf( ch, "%s emana delle scariche elettriche azzurre di pura energia.&w\r\n", name );
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_ACIDMIST) )
	{
		set_char_color( AT_GREEN, ch );
		if ( ch == victim )
			send_to_char( ch, "Sono visibile attraverso una nube di agitata foschia.&w\r\n" );
		else
			ch_printf( ch, "%s è visibile attraverso una nube di agitata foschia.&w\r\n", name );
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_ICESHIELD) )
	{
		set_char_color( AT_LCYAN, ch );
		if ( ch == victim )
			ch_printf( ch, "Sono attorniat%c da frammenti di ghiaccio scintillante.&w\r\n", gramm_ao(victim) );
		else
			ch_printf( ch, "%s è attorniat%c da frammenti di ghiaccio scintillante.&w\r\n", name, gramm_ao(victim) );
	}

	if ( HAS_BIT(victim->affected_by, AFFECT_CHARM) )
	{
		set_char_color( AT_MAGIC, ch );
		if ( ch == victim )
			send_to_char( ch, "Ho gli occhi persi nel vuoto.&w\r\n" );
		else
			ch_printf( ch, "%s ha gli occhi persi nel vuoto.&w\r\n", name );
	}

	if ( IS_PG(victim) && !victim->desc
		&& victim->switched && HAS_BIT(victim->switched->affected_by, AFFECT_POSSESS) )
	{
		set_char_color( AT_MAGIC, ch );
		if ( ch == victim )
			send_to_char( ch, "Osservo il mondo circostante con sguardo allucinato..&w\r\n" );
		else
			ch_printf( ch, "%s mi osserva con sguardo allucinato..&w\r\n", PERS(victim, ch) );
	}
}


void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char buf[MSL];
	char buf1[MSL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "show_char_to_char_0: ch è NULL" );
		return;
	}

	if ( victim == NULL )
	{
		send_log( NULL, LOG_BUG, "show_char_to_char_0: victim è NULL" );
		return;
	}

	buf[0] = '\0';

	/* Etichette visualizzabili dagli admin */
	if ( IS_ADMIN(ch) )
	{
		/* Etichette dei mob */
		if ( IS_MOB(victim) )
		{
			if ( HAS_BIT(victim->affected_by, AFFECT_POSSESS) && victim->desc )
			{
				sprintf( buf1, "(%s)",victim->desc->original->name );
				strcat( buf, buf1 );
			}

			if ( HAS_BIT_ACT(victim, MOB_INCOGNITO) )
			{
				sprintf( buf1,"(Mobinvis %d) ", victim->mobinvis );
				strcat( buf, buf1 );
			}
		}

		/* Etichette dei pg */
		if ( IS_PG(victim) )
		{
			if ( !victim->desc )
			{
				if ( !victim->switched )
					strcat( buf, "(Link Dead) " );
				else if ( !HAS_BIT(victim->affected_by, AFFECT_POSSESS) )
					strcat( buf, "(Switched) " );
			}

			if ( victim->pg->incognito != 0 )
			{
				sprintf( buf1,"(Inc %d) ", victim->pg->incognito );
				strcat( buf, buf1 );
			}

			/* (FF) non mi piace così la gestione del badge, farla come un oggetto normale da indossare che se i pg perdono possano recuperare nella sede clan */
			if ( victim->pg->clan && victim->pg->clan->badge )
				send_to_char( ch, victim->pg->clan->badge->short_descr );

			if ( HAS_BIT_PLR(victim, PLAYER_ATTACKER) )
			{
				if ( ch->sex == SEX_FEMALE )			strcat( buf, "(Assalitrice) "	);
				else									strcat( buf, "(Assalitore) "	);
			}

			if ( HAS_BIT_PLR(victim, PLAYER_KILLER) )		strcat( buf, "(Omicida) "		);
			if ( HAS_BIT_PLR(victim, PLAYER_THIEF ) )		strcat( buf, "(Ladr$X) "		);
			if ( HAS_BIT_PLR(victim, PLAYER_LITTERBUG) )	strcat( buf, "(LITTERBUG) "		);
		}

		/* Altre etichette */
		if ( is_editing(victim->desc) )					strcat( buf, "(Edita) "			);
		if ( is_in_olc(victim->desc) )					strcat( buf, "(OLC) "			);
		if ( victim->morph != NULL )					strcat( buf, "(Morph) "			);
	}


	/*
	 * Etichette che possono vedere tutti
	 */
	set_char_color( AT_PERSON, ch );

	/* Etichette riguardanti i mob */
	if ( IS_MOB(victim) )
	{
	  if ( ch->mount
	    && ch->mount == victim
	    && ch->in_room == ch->mount->in_room )					strcat( buf, "(Cavalcatura) "		);
	}

	/* Etichette riguardo i pg */
	if ( IS_PG(victim) )
	{
		if ( !IS_ADMIN(victim)
		  && get_level(victim) <= LVL_NEWBIE )					strcat( buf, "(&WInespert$X&w) " );
    	if ( HAS_BIT_PLR(victim, PLAYER_AFK) )						strcat( buf, "[&OTrance&w] "	 );
    	if ( HAS_BIT_PLR(victim, PLAYER_SPIRIT) )					strcat( buf, "(&WSpirito&w) "	 );
	}

	/* Altre etichette */
	if ( (IS_ADMIN(ch) || HAS_BIT(ch->affected_by, AFFECT_DETECT_INVIS))
	  && HAS_BIT(victim->affected_by, AFFECT_INVISIBLE) )					strcat( buf, "(Invisibile) "	);
	if ( (IS_ADMIN(ch) || HAS_BIT(ch->affected_by, AFFECT_DETECT_HIDDEN))
	  && HAS_BIT(victim->affected_by, AFFECT_HIDE) )						strcat( buf, "(Nascost$X) "		);
	if ( HAS_BIT(victim->affected_by, AFFECT_PASS_DOOR) )					strcat( buf, "(Translucente) "	);
	if ( HAS_BIT(victim->affected_by, AFFECT_FAERIE_FIRE) )					strcat( buf, "(&PAura rosa&w) "	);
	if ( HAS_BIT(victim->affected_by, AFFECT_BERSERK) )						strcat( buf, "(&rBerserk&w) "		);
	if ( IS_ADMIN(ch)
	  || HAS_BIT(ch->affected_by, AFFECT_DETECT_ALIGN)
	  || HAS_BIT(ch->affected_by, AFFECT_DETECT_GOOD)
	  || HAS_BIT(ch->affected_by, AFFECT_DETECT_EVIL) )
	{
		bool	see_good = FALSE;
		bool	see_evil = FALSE;

		if ( IS_ADMIN(ch) )
		{
			see_good = TRUE;
			see_evil = TRUE;
		}

		if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_ALIGN) )
		{
			if ( is_align(ch, ALIGN_GOOD) )
				see_evil = TRUE;
			if ( is_align(ch, ALIGN_EVIL) )
				see_good = TRUE;
		}

		if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_GOOD) )
			see_good = TRUE;
		if ( HAS_BIT(ch->affected_by, AFFECT_DETECT_EVIL) )
			see_evil = TRUE;

		if		( see_evil && is_align(victim, ALIGN_EVIL) )	strcat( buf, "(&RAura rossa&w) "	);
		else if ( see_good && is_align(victim, ALIGN_GOOD) )	strcat( buf, "(&WAura bianca&w) "	);
	}

	/*
	 * Morph
	 */
	set_char_color( AT_PERSON, ch );

	if ( (victim->position == victim->defposition
	   && VALID_STR(victim->long_descr))
	  || (victim->morph && victim->morph->morph
	   && victim->morph->morph->defpos == victim->position) )
	{
		if ( victim->morph != NULL )
		{
			if ( !IS_ADMIN(ch) )
			{
				if ( victim->morph->morph != NULL )
					strcat( buf, victim->morph->morph->long_desc );
				else
					strcat( buf, victim->long_descr );
			}
			else
			{
				strcat( buf, PERS(victim, ch) );
				if ( IS_PG(victim) && (IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_BRIEF))
				  && VALID_STR(victim->pg->title) && victim->morph == NULL )
				{
					strcat( buf, victim->pg->title );
				}
				strcat( buf, "." );
			}
		}
		else
			strcat( buf, victim->long_descr );

		if ( IS_MOB(victim) && (IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_VNUM)) )
			sprintf( buf+strlen(buf), " &w(#%d)", victim->pIndexData->vnum );

		act( AT_PERSON, buf, ch, NULL, victim, TO_CHAR );
		return;
	}
	else
	{
		if ( victim->morph != NULL && victim->morph->morph != NULL && !IS_ADMIN(ch) )
			strcat( buf, MORPHPERS(victim, ch) );
		else
			strcat( buf, PERS(victim, ch) );
	}

	/* Inserisce il titolo */
	if ( IS_PG(victim) && (IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_BRIEF))
	  && VALID_STR(victim->pg->title) && victim->morph == NULL )
	{
		strcat( buf, victim->pg->title );
	}

	switch ( victim->position )
	{
	  case POSITION_DEAD:		strcat( buf, " e mort$X!!" );				break;
	  case POSITION_MORTAL:		strcat( buf, " è ferit$X mortalmente." );	break;
	  case POSITION_INCAP:		strcat( buf, " è incapacitat$X." );			break;
	  case POSITION_STUN:		strcat( buf, " giace a terra svenut$X." );	break;
	  case POSITION_SLEEP:
		if ( ch->position == POSITION_SIT || ch->position == POSITION_REST )
		{
			if ( ch->sex == SEX_FEMALE )
				strcat( buf, " sta dormendo serenamente." );
			else
				strcat( buf, " sta ronfando sonoramente." );
		}
		else
			strcat( buf, " giace qui in profondo sonno." );
		break;
	  case POSITION_REST:
		if		( ch->position == POSITION_REST )
			strcat( buf, " è sdraiat$X di fianco a te." );
		else if ( ch->position == POSITION_MOUNT )
			strcat( buf, " è sdraiat$X in cima alla sua cavalcatura." );
		else
			strcat( buf, " è qui sdraiat$X." );
		break;
	  case POSITION_SIT:
		if		( ch->position == POSITION_SIT )
			strcat( buf, " è sedut$X accanto a te." );
		else if ( ch->position == POSITION_REST )
			strcat( buf, " siede vicino a te, che poltrisci." );
		else
			strcat( buf, " è sedut$X qui." );
		break;
	  case POSITION_STAND:
		if ( (victim->in_room->sector == SECTOR_UNDERWATER)
		  && !HAS_BIT(victim->affected_by, AFFECT_AQUA_BREATH) && IS_PG(victim) )
		{
			strcat( buf, " sta annegando!" );
		}
		else if ( victim->in_room->sector == SECTOR_UNDERWATER )
			strcat( buf, " è nell'acqua." );
		else if ( (victim->in_room->sector == SECTOR_OCEANFLOOR)
		  && !HAS_BIT(victim->affected_by, AFFECT_AQUA_BREATH) && IS_PG(victim) )
		{
			strcat( buf, " sta annegando!" );
		}
		else if ( victim->in_room->sector == SECTOR_OCEANFLOOR )
			strcat( buf, " si trova in acqua." );
		else if ( HAS_BIT(victim->affected_by, AFFECT_FLOATING) || HAS_BIT(victim->affected_by, AFFECT_FLYING) )
			strcat( buf, " sta fluttuando qui." );
		else
			strcat( buf, " è qui in piedi." );
		break;
	  case POSITION_SHOVE:	strcat( buf, " viene spint$X in giro." );		break;
	  case POSITION_DRAG:	strcat( buf, " viene trascinat$X in giro." );	break;
	  case POSITION_MOUNT:
		strcat( buf, " è qui, sopra " );
		if ( !victim->mount )
			strcat( buf, "l'aria leggera??" );
		else if ( victim->mount == ch )
			strcat( buf, "la sua schiena." );
		else if ( victim->in_room == victim->mount->in_room )
		{
			strcat( buf, PERS(victim->mount, ch) );
			strcat( buf, "." );
		}
		else
			strcat( buf, "qualcosa che è sfuggito??" );
		break;
	  case POSITION_FIGHT:
	  case POSITION_EVASIVE:
	  case POSITION_DEFENSIVE:
	  case POSITION_AGGRESSIVE:
	  case POSITION_BERSERK:
		strcat( buf, " sta lottando per la sua vita " );
		if ( !victim->fighting )
		{
			strcat( buf, "l'aria leggera??" );

			/* some bug somewhere.. kinda hackey fix -h */
			if ( !victim->mount )
				victim->position = POSITION_STAND;
			else
				victim->position = POSITION_MOUNT;
		}
		else if ( who_fighting(victim) == ch )
			strcat( buf, "contro di me!!" );
		else if ( victim->in_room == victim->fighting->who->in_room )
		{
			strcat( buf, PERS(victim->fighting->who, ch) );
			strcat( buf, "." );
		}
		else
			strcat( buf, "qualcosa è sfuggito??" );
		break;
	}

	if ( IS_MOB(victim) && (IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_VNUM)) )
		sprintf( buf+strlen(buf), " &w(#%d)", victim->pIndexData->vnum );

	buf[0] = toupper( buf[0] );
	act( AT_PERSON, buf, ch, NULL, victim, TO_CHAR );
}


void show_race_line( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( IS_PG(victim) )	/* (FF) da togliere in futuro, solo da tenere per l'età e altri particolari */
	{
		char	buf[MSL];
		char	race_name[MIL];

		/* Prepara il nome della razza con articolo */
		strcpy( race_name, get_race(victim, TRUE) );
		strcpy( race_name, get_article(race_name, ARTICLE_INDETERMINATE, (victim->sex == SEX_FEMALE) ? ARTICLE_FEMININE : -1, -1) );

		if ( victim != ch )
		{
			/* L'altezza si basa sulle proporzioni applicate da Policleto, la testa divide 7,5 volte il corpo dall'alto */
			if ( ch->height > victim->height )
			{
				if ( ch->height - victim->height < (victim->height/7.5) * 0.5 )
					sprintf( buf, "%s è appena più basso di me", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 1.0 )
					sprintf( buf, "%s è più basso di me di mezza testa", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 1.5 )
					sprintf( buf, "%s è più basso di me di una testa", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 2.0 )
					sprintf( buf, "%s mi arriva alle spalle", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 2.5 )
					sprintf( buf, "%s mi arriva al%s", PERS(victim, ch), (ch->sex == SEX_FEMALE && number_range(0,200) == 0) ? "le tette" : " petto" );	/* AAAHH! come sono burlone :D */
				else if ( ch->height - victim->height < (victim->height/7.5) * 3.0 )
					sprintf( buf, "%s mi arriva poco sotto al petto", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 3.5 )
					sprintf( buf, "%s mi arriva alla pancia", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 4.0 )
					sprintf( buf, "%s mi arriva al%s", PERS(victim, ch), (number_range(0,300) == 0) ? " bacino" : "la mia zona.. privata" );
				else if ( ch->height - victim->height < (victim->height/7.5) * 4.5 )
					sprintf( buf, "%s mi arriva poco sotto al bacino", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 5.0 )
					sprintf( buf, "%s mi arriva alle cosce", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 5.5 )
					sprintf( buf, "%s mi arriva poco sopra le ginocchia", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 6.0 )
					sprintf( buf, "%s mi arriva alle ginocchia", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 6.5 )
					sprintf( buf, "%s mi arriva poco sotto le ginocchia", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 7.0 )
					sprintf( buf, "%s mi arriva a metà tibia", PERS(victim, ch) );
				else if ( ch->height - victim->height < (victim->height/7.5) * 7.5 )
					sprintf( buf, "%s mi arriverà si e no alla caviglia", PERS(victim, ch) );
				else
					sprintf( buf, "%s è microscopico", PERS(victim, ch) );
			}
			else if ( ch->height < victim->height )
			{
				if ( victim->height - ch->height < (victim->height/7.5) * 0.5 )
					sprintf( buf, "Sono appena più basso di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 1.0 )
					sprintf( buf, "Arrivo in altezza a metà viso di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 1.5 )
					sprintf( buf, "La testa di %s mi supera in altezza", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 2.0 )
					sprintf( buf, "Arrivo in altezza spalle di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 2.5 )
					sprintf( buf, "Arrivo in altezza al petto di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 3.0 )
					sprintf( buf, "Arrivo in altezza poco sotto al petto di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 3.5 )
					sprintf( buf, "Arrivo in altezza alla pancia di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 4.0 )
					sprintf( buf, "Arrivo in altezza al bacino di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 4.5 )
					sprintf( buf, "Arrivo in altezza poco sotto al bacino di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 5.0 )
					sprintf( buf, "Arrivo in altezza alle cosce di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 5.5 )
					sprintf( buf, "Arrivo in altezza poco sopra le ginocchia di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 6.0 )
					sprintf( buf, "Arrivo in altezza alle ginocchia di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 6.5 )
					sprintf( buf, "Arrivo in altezza poco sotto le ginocchia di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 7.0 )
					sprintf( buf, "Arrivo in altezza a metà tibia di %s", PERS(victim, ch) );
				else if ( victim->height - ch->height < (victim->height/7.5) * 7.5 )
					sprintf( buf, "Arrivo in altezza si e no alla caviglia di %s", PERS(victim, ch) );
				else
					sprintf( buf, "%s è GIGANTESCO", PERS(victim, ch) );
			}
			else
				sprintf( buf, "%s è alto quanto me", PERS(victim, ch) );

			sprintf( buf+strlen(buf), " e pesa approssimativamente %d kg.", victim->weight/1000 );
		}
		else
		{
			int		metre;
			int		centimetre;

			metre		= (int) ch->height / 100;
			centimetre	= ch->height % 100;

			if ( metre == 0 )
			{
				sprintf( buf, "Sono %s&w alt$x circa %d cm e peso approssimativamente %d kg.",
					race_name, centimetre, ch->weight/1000 );
			}
			else
			{
				sprintf( buf, "Sono %s&w alt$x circa %d e %d cm e peso approssimativamente %d kg.",
					race_name, metre, centimetre, ch->weight/1000 );
			}
		}
		act( AT_PLAIN, buf, ch, NULL, victim, TO_CHAR );
	}
}


/*
 * (RR) vedere la funzione get_health_string in comm.c e vedere se è il caso di unirle.
 */
void show_condition( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char   *str;
	int		percent;

	percent = get_percent( victim->points[POINTS_LIFE], victim->points_max[POINTS_LIFE] );

	if ( victim != ch )
	{
		if		( percent >= 100 )	str = "E' in perfetta salute.";
		else if ( percent >=  90 )	str = "Ha dei graffi.";
		else if ( percent >=  80 )	str = "Ha delle contusioni sul volto.";
		else if ( percent >=  70 )	str = "Ha dei tagli sul corpo.";
		else if ( percent >=  60 )	str = "Ha dei tagli profondi sul corpo.";
		else if ( percent >=  50 )	str = "Sta sanguinando.";
		else if ( percent >=  40 )	str = "Sanguina copiosamente.";
		else if ( percent >=  30 )	str = "Ha grossi squarci aperti su tutto il corpo!";
		else if ( percent >=  20 )	str = "E' ricopert$x dal suo stesso sangue.";
		else if ( percent >=  10 )	str = "Sta per svenire.";
		else						str = "E' in coma.";
	}
	else
	{
		if		( percent >= 100 )	str = "Sono in perfetta salute.";
		else if ( percent >=  90 )	str = "Ho dei graffi.";
		else if ( percent >=  80 )	str = "Ho delle contusioni.";
		else if ( percent >=  70 )	str = "Ho tagli su tutto corpo.";
		else if ( percent >=  60 )	str = "Ho dei tagli profondi.";
		else if ( percent >=  50 )	str = "Sto sanguinando.";
		else if ( percent >=  40 )	str = "Sto sanguinando copiosamente.";
		else if ( percent >=  30 )	str = "Ho grossi squarci aperti!";
		else if ( percent >=  20 )	str = "Sono ricopert$x di sangue!!.";
		else if ( percent >=  10 )	str = "Sto per svenire!!!.";
		else						str = "Sono in coma.";
	}

	act( AT_PLAIN, str, ch, NULL, NULL, TO_CHAR );
}


void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
	if ( can_see(victim, ch) )
	{
		act( AT_ACTION, "$n mi guarda.", ch, NULL, victim, TO_VICT );
		if ( victim != ch )
			act( AT_ACTION, "$n guarda $N.",  ch, NULL, victim, TO_NOVICT );
		else
			act( AT_ACTION, "$n guarda se stess$x.", ch, NULL, victim, TO_NOVICT );
	}

	if ( VALID_STR(victim->description) )
	{
		if ( victim->morph != NULL && victim->morph->morph != NULL )
			send_to_char( ch, victim->morph->morph->description );
		else
			send_to_char( ch, victim->description );
	}
	else
	{
		if ( victim->morph != NULL && victim->morph->morph != NULL )
			send_to_char( ch, victim->morph->morph->description );
		else if ( IS_MOB(victim) )
			act( AT_PLAIN, "Non vedo nulla di speciale in $E.", ch, NULL, victim, TO_CHAR );
		else if ( ch != victim )
			act( AT_PLAIN, "Osservo attentamente $N..", ch, NULL, victim, TO_CHAR );	/* (GR) L'osservi/La osservi */
		else
			act( AT_PLAIN, "Mi guardo..", ch, NULL, NULL, TO_CHAR );
	}

	show_race_line( ch, victim );

	/* Visualizza le informazioni fisiche della vittima */
	if ( IS_PG(victim) )
	{
		char	buf[MSL];

		if ( victim->pg->hair )
		{
			/* Invia la stringa descrittiva dei capelli (FF) aggiungere la lunghezza, con stringhe tipo: lunghi quando i miei etc etc */
			ch_printf( ch, "Port%c i capelli %s di color &%c%s&w.\r\n",
				ch == victim ? 'o' : 'a',
				(victim->pg->hair->style->cat != HAIRSTYLE_NONE) ? victim->pg->hair->style->syn : victim->pg->hair->type->syn,
				(victim->pg->hair->dye->cat != AT_NONE) ? get_clr(victim->pg->hair->dye->cat) : get_clr(victim->pg->hair->color->cat),
				(victim->pg->hair->dye->cat != AT_NONE) ? victim->pg->hair->dye->syn : victim->pg->hair->color->syn );
		}
		else
		{
			send_log( NULL, LOG_BUG, "show_char_to_char1: struttura hair NULL per il pg %s", ch->name );
		}

		if ( victim->pg->eye )
		{
			/* Prepara ed invia quella degli occhi */
			sprintf( buf, "H%c gli occhi ", ch == victim ? 'o' : 'a' );

			if ( victim->pg->eye->type->cat != EYE_NORMAL )
				sprintf( buf + strlen(buf), "&%c%s ", get_clr(victim->pg->eye->type->cat), victim->pg->eye->type->syn );

			if ( victim->pg->eye->bright_color->cat != AT_NONE )
				sprintf( buf + strlen(buf), "luminosi di color &%c%s", get_clr(victim->pg->eye->bright_color->cat), victim->pg->eye->bright_color->syn );
			else
				sprintf( buf + strlen(buf), "di color &%c%s", get_clr(victim->pg->eye->color->cat), victim->pg->eye->color->syn );

			if ( victim->pg->eye->pupil_color->cat != AT_BLACK && victim->pg->eye->type->cat != EYE_NOPUPIL )
				sprintf( buf + strlen(buf), " inoltre la sua pupilla è di color &%c%s", get_clr(victim->pg->eye->pupil_color->cat), victim->pg->eye->pupil_color->syn );

			strcat( buf, "\r\n" );
			send_to_char( ch, buf );
		}
		else
		{
			send_log( NULL, LOG_BUG, "show_char_to_char1: struttura eye NULL per il pg %s", ch->name );
		}
	}

	show_condition( ch, victim );
	show_visible_affects_to_char( victim, ch );

	if ( IS_ADMIN(ch) )
	{
		char	class_name[MIL];

		if ( IS_MOB(victim) )
			ch_printf( ch, "[ADM] Mobile #%d '%s' ", victim->pIndexData->vnum, victim->name );
		else
			ch_printf( ch, "[ADM] Pg %s ", victim->name );

		strcpy( class_name, get_class(victim, TRUE) );
		ch_printf( ch, "è di livello %d, classe %s&w.\r\n", victim->level, class_name );

		if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_FLAG) )
		{
			if ( IS_MOB(victim) )
			{
				if ( IS_EMPTY(victim->act) )
					ch_printf( ch, "[ADM] Flag: nessuna\r\n" );
				else
					ch_printf( ch, "[ADM] Flag: %s\r\n", code_bit(NULL, victim->act, CODE_MOB) );
			}
			else
			{
				if ( IS_EMPTY(victim->pg->flags) )
					ch_printf( ch, "[ADM] Flag: nessuna\r\n" );
				else
					ch_printf( ch, "[ADM] Flag: %s\r\n", code_bit(NULL, victim->pg->flags, CODE_PLAYER) );
			}
		}
	}

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_LOOKEQUIP) )
	{
		char	arg[MIL];

		sprintf( arg, "equipment %s", victim->name );
		send_command( ch, arg, CO );
	}
}


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
	CHAR_DATA *rch;

	for ( rch = list;  rch;  rch = rch->next_in_room )
	{
		if ( rch == ch )
			continue;

		/* (TT) testare se funziona */
		if ( rch == supermob && !IS_ADMIN(ch) )
			continue;

		if ( can_see(ch, rch) )
			show_char_to_char_0( rch, ch );
		else if ( room_is_dark(ch->in_room)
		  && HAS_BIT(rch->affected_by, AFFECT_INFRARED)
		  && !(IS_PG(rch) && IS_ADMIN(rch)) )
		{
			set_char_color( AT_DRED, ch );
			send_to_char( ch, "Scorgi la rossa forma di una creatura vivente.\r\n" );
		}
	}
}


bool check_blind( CHAR_DATA *ch )
{
	if ( IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ADMSIGHT) )
		return TRUE;

	if ( HAS_BIT(ch->affected_by, AFFECT_TRUESIGHT) )
		return TRUE;

	if ( HAS_BIT(ch->affected_by, AFFECT_BLIND) )
	{
		send_to_char( ch, "Non vedi nulla!\r\n" );
		return FALSE;
	}

	return TRUE;
}


/*
 * Ritorna la stringa con le flags dell'oggetto se è admin e se le vuole visualizzare.
 */
char *get_oflag_string( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_FLAG) )
	{
		static char	buf[MSL];

		buf[0] = '\0';
		sprintf( strlen(buf) + buf, "\r\nType: %s", code_name(NULL, obj->type, CODE_OBJTYPE) );
		if ( IS_EMPTY(obj->extra_flags) )
			sprintf( strlen(buf) + buf, "\r\n[ADM] Flag: nessuna\r\n" );
		else
			sprintf( strlen(buf) + buf, "\r\n[ADM] Flag: %s\r\n", code_bit(NULL, obj->extra_flags, CODE_OBJFLAG) );

		return buf;
	}

	return "";
}


/*
 * Get an extra description from a list.
 */
char *get_extradescr( char *name, EXTRA_DESCR_DATA *ed )
{
	if ( !VALID_STR(name) )
		return NULL;

	for ( ;  ed;  ed = ed->next )
	{
		if ( nifty_is_name(name, ed->keyword) )	/* (CC) nifty almeno fino a che non verrà il sistema di descrizione oggetto */
			return ed->description;
	}

	return NULL;
}


/*
 * Get an extra description from a list controllando il prefisso
 */
char *get_extradescr_prefix( char *name, EXTRA_DESCR_DATA *ed )
{
	for ( ;  ed;  ed = ed->next )
	{
		if ( nifty_is_name_prefix(name, ed->keyword) )	/* (CC) nifty almeno fino a che non verrà il sistema di descrizione oggetto */
			return ed->description;
	}

	return NULL;
}


DO_RET do_look( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*original;
	EXIT_DATA	*pexit;
	CHAR_DATA	*victim;
	OBJ_DATA	*obj;
	char		*pdesc;
	char		 arg [MIL];
	char		 arg1[MIL];
	char		 arg2[MIL];
	char		 arg3[MIL];
	int			 door;
	int			 number;
	int			 cnt;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_look: ch è NULL" );
		return;
	}

	if ( !ch->desc )
		return;

	if ( ch->position < POSITION_SLEEP )
	{
		send_to_char( ch, "Ora come ora posso solo vedere le stelle!\r\n" );
		return;
	}

	if ( ch->position == POSITION_SLEEP )
	{
		send_to_char( ch, "Ora come ora posso solo vedere i miei sogni.\r\n" );
		return;
	}

	if ( !check_blind(ch) )
		return;

	if ( VALID_STR(argument)
	  && (!str_cmp(argument, "cielo") || !str_cmp(argument, "sky"))
	  && (!str_cmp(argument, "stelle") || !str_cmp(argument, "stars")) )
	{
		send_command( ch, "sky", CO );
		return;
	}

#ifdef T2_ARENA
	if ( !str_cmp(argument, "arena") && in_terrace(ch) )
	{
		do_arena( ch );
		return;
	}
#endif

	if ( (IS_ADMIN(ch) && IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_ADMSIGHT))
	  && !HAS_BIT(ch->affected_by, AFFECT_TRUESIGHT)
	  && room_is_dark(ch->in_room) )
	{
		set_char_color( AT_DGREY, ch );
		send_to_char( ch, "E' buio pesto..\r\n" );
		show_char_to_char( ch->in_room->first_person, ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	/*
	 * Guardare la descrizione della stanza, mapper o wild
	 */
	if ( !VALID_STR(arg1) )
	{
		set_char_color( AT_RMNAME, ch );

		/* Se è amministratore visualizza anche il vnum della stanza */
		send_to_char( ch, ch->in_room->name );
		if ( IS_ADMIN(ch) )
		{
			ch_printf( ch, " &w(%s)", (ch->in_room->area) ? ch->in_room->area->name : "Nel Vuoto NULL" );
			if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_VNUM) )
				ch_printf( ch, " (#%d)", ch->in_room->vnum );
		}
		send_to_char( ch, "\r\n" );

		set_char_color( AT_RMDESC, ch );
		if ( !VALID_STR(arg1) || (IS_PG(ch) && !HAS_BIT_PLR(ch, PLAYER_BRIEF)) )
		{
			/* Visualizza la mappa solo se il pg vuole. */
			if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_MAPPER) )
				do_mapper( ch, "7" );	/* Non è inviato con la send_command per poter sforare il check sulla trust */
			else
			{
				/* Invio del capilettera */
				ch_printf( ch, "&z[&%c%c&z]&w%s",
					get_clr(ch->in_room->area->color->cat),
					ch->in_room->description[0],
					ch->in_room->description+1 );
			}
		}

		if ( IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_FLAG) )
		{
			ch_printf( ch, "[ADM] Settore: %s\r\n", code_name(NULL, ch->in_room->sector, CODE_SECTOR) );
			if ( IS_EMPTY(ch->in_room->flags) )
				ch_printf( ch, "[ADM] Flag: nessuna\r\n" );
			else
				ch_printf( ch, "[ADM] Flag: %s\r\n", code_bit(NULL, ch->in_room->flags, CODE_ROOM) );
		}

		send_command( ch, "exits", CO );

#ifdef T2_MXP
		show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE, eItemGet );
#else
		show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
#endif
		show_char_to_char( ch->in_room->first_person,  ch );

		return;
	} /* chiude l'if */

	/*
	 * Guardare sotto ad altri oggetti
	 */
	if ( is_name(arg1, "sotto under") )
	{
		int count;

		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Sotto che cosa dovrei guardare?\r\n" );
			return;
		}

		obj = get_obj_here( ch, arg2 );
		if ( !obj )
		{
			send_to_char( ch, "Non vedo nessun oggetto del genere.\r\n" );
			return;
		}

		if ( !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE) && !IS_ADMIN(ch) )
		{
			send_to_char( ch, "Non riesco a fare presa per poterlo alzare.\r\n" );
			return;
		}

		if ( ch->carry_weight + obj->weight > can_carry_weight(ch) )
		{
			send_to_char( ch, "E' troppo pesante per poterci guardare sotto.\r\n" );
			return;
		}

		count = obj->count;
		obj->count = 1;
		act( AT_PLAIN, "Alzo $p e dò una occhiata sotto:", ch, obj, NULL, TO_CHAR );
		act( AT_PLAIN, "$n alza $p e dà una occhiata sotto:", ch, obj, NULL, TO_ROOM );
		obj->count = count;
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_COVERING) )
		{
#ifdef T2_MXP
			show_list_to_char( obj->first_content, ch, TRUE, TRUE, eItemNothing );
#else
			show_list_to_char( obj->first_content, ch, TRUE, TRUE );
#endif
		}
		else
		{
			send_to_char( ch, "Nulla.\r\n" );
		}

		if ( EXA_prog_trigger )
			oprog_examine_trigger( ch, obj );

		return;
	} /* chiude l'if */

	/*
	 * Guardare dentro i contenitori
	 */
	if ( is_name(arg1, "in dentro") )
	{
		int		count;
		bool	found = FALSE;

		/* 'look in' */
		if ( !VALID_STR(arg2) )
		{
			send_to_char( ch, "Dentro che cosa dovrei guardare?\r\n" );
			return;
		}

		obj = get_obj_here( ch, arg2 );
		if ( !obj )
		{
			send_to_char( ch, "Non vedo questa cosa qui intorno.\r\n" );
			return;
		}

		/* Fa prima il check sul tipo */
		if ( obj->type == OBJTYPE_PORTAL )	/* (FF) fare il portal un values tipo, per esempio se il portale è intraroom o intraplanare */
		{
			for ( pexit = ch->in_room->first_exit;  pexit;  pexit = pexit->next )
			{
				if ( pexit->vdir == DIR_SOMEWHERE && HAS_BIT(pexit->flags, EXIT_PORTAL) )
				{
					original = ch->in_room;
					char_from_room( ch );
					char_to_room( ch, pexit->to_room );
					send_command( ch, "look", CO );
					char_from_room( ch );
					char_to_room( ch, original );
					return;
				}
			}
			send_to_char( ch, "Vedo un turbine caotico..\r\n" );
		}

		/* Poi sul tipo di value */
		if ( obj->drinkcon )
		{
			found = TRUE;

			if ( obj->drinkcon->curr_quant <= 0 )
			{
				send_to_char( ch, "E' vuoto.\r\n" );
				if ( EXA_prog_trigger )
					oprog_examine_trigger( ch, obj );
				return;
			}

			if		( obj->drinkcon->curr_quant < obj->drinkcon->capacity / 4 )
				ch_printf( ch, "E' pieno per un quarto di un liquido dal colore %s.\r\n", table_liquid[obj->drinkcon->liquid].color );
			else if ( obj->drinkcon->curr_quant < obj->drinkcon->capacity / 2 )
				ch_printf( ch, "E' pieno per metà di un liquido dal colore %s.\r\n", table_liquid[obj->drinkcon->liquid].color );
			else if ( obj->drinkcon->curr_quant < obj->drinkcon->capacity * 3/4 )
				ch_printf( ch, "E' pieno per tre quarti di un liquido dal colore %s.\r\n", table_liquid[obj->drinkcon->liquid].color );
			else
				ch_printf( ch, "E' pieno di un liquido dal colore %s.\r\n", table_liquid[obj->drinkcon->liquid].color );

			if ( EXA_prog_trigger )
				oprog_examine_trigger( ch, obj );
		}

		if ( obj->container )
		{
			found = TRUE;

			if ( HAS_BITINT(obj->container->flags, CONTAINER_CLOSED) )
			{
				send_to_char( ch, "E' chiuso.\r\n" );
				return;
			}

			count = obj->count;
			obj->count = 1;
			if ( obj->type == OBJTYPE_CONTAINER )
				act( AT_PLAIN, "$p contiene:", ch, obj, NULL, TO_CHAR );
			else
				act( AT_PLAIN, "$p contiene:", ch, obj, NULL, TO_CHAR );
			obj->count = count;
#ifdef T2_MXP
			show_list_to_char( obj->first_content, ch, TRUE, TRUE, eItemNothing );
#else
			show_list_to_char( obj->first_content, ch, TRUE, TRUE );
#endif
			if ( EXA_prog_trigger )
				oprog_examine_trigger( ch, obj );
		}

		if ( !found )
			send_to_char( ch, "Non è un contenitore.\r\n" );

		return;
	}

	/*
	 * Cerca tra le extra di una stanza in maniera esatta
	 */
	pdesc = get_extradescr( arg1, ch->in_room->first_extradescr );
	if ( pdesc )
	{
		send_to_char( ch, pdesc );
		return;
	}

	/*
	 * Guardare un'uscita
	 */
	door = get_door( arg1 );
	pexit = find_door( ch, arg1, TRUE );
	if ( pexit )
	{
		if (  HAS_BIT(pexit->flags, EXIT_CLOSED)
		  && !HAS_BIT(pexit->flags, EXIT_WINDOW) )
		{
			if ( (HAS_BIT(pexit->flags, EXIT_SECRET)
			  ||  HAS_BIT(pexit->flags, EXIT_DIG)) && door != -1 )
				send_to_char( ch, "Non noto nulla di speciale.\r\n" );
			else
				act( AT_PLAIN, "Il $d è chiuso.", ch, NULL, pexit->keyword, TO_CHAR );

			return;
		}

		if ( HAS_BIT( pexit->flags, EXIT_BASHED ) )
			act( AT_RED, "Il $d è stato strappato dai suoi cardini!",
				ch, NULL, pexit->keyword, TO_CHAR );

		if ( VALID_STR(pexit->description) )
			send_to_char( ch, pexit->description );
		else
			send_to_char( ch, "Non c'e nulla d'interessante.\r\n" );

		/*
		 * Ability to look into the next room
		 */
		if ( pexit->to_room
		  && (HAS_BIT(ch->affected_by, AFFECT_SCRYING) || HAS_BIT(pexit->flags, EXIT_xLOOK) || IS_ADMIN(ch)) )
		{
			if ( !HAS_BIT(pexit->flags, EXIT_xLOOK) && !IS_ADMIN(ch) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( ch, "Provo a scrutare..\r\n" );
				/*
				 * Change to allow characters who don't have the scry spell
				 * to benefit from objects that are affected by scry.
				 */
				if ( IS_PG(ch) )
				{
					int percent = knows_skell( ch, skill_lookup("scry") );
					if ( !percent )
					{
						if ( ch->class == CLASS_THIEF )
							percent = 95;
						else
							percent = 55;
					}

					if ( number_percent( ) > percent )
					{
						send_to_char( ch, "Ho fallito, ho perso la concentrazione.\r\n" );
						return;
					}
				}
			}

			original = ch->in_room;
			char_from_room( ch );
			char_to_room( ch, pexit->to_room );
			send_command( ch, "look", CO );
			char_from_room( ch );
			char_to_room( ch, original );
		}
		return;
	}
	else if ( door != -1 )
	{
		send_to_char( ch, "Nulla di interessante in quella direzione.\r\n" );
		return;
	}

	/*
	 * Guardare un pg o mob nella stanza
	 */
	victim = get_char_room( ch, arg1, TRUE );
	if ( victim )
	{
		show_char_to_char_1( victim, ch );
		return;
	}

	/*
	 * Cerca tra le extra di una stanza in maniera prefix
	 *	ma solo per argomenti con almeno 2 caratteri
	 */
	if ( strlen(arg1) >= 2 )
	{
		pdesc = get_extradescr_prefix( arg1, ch->in_room->first_extradescr );
		if ( pdesc )
		{
			send_to_char( ch, pdesc );
			return;
		}
	}

	/* Questo serve per cercare sia tra gli oggetti addosso sia in quelli nella stanza */
	number = number_argument( arg1, arg );
	cnt = 0;

	/*
	 * Guarda tra gli oggetti trasportati
	 */
	for ( obj = ch->last_carrying;  obj;  obj = obj->prev_content )
	{
		char	buf_weight[MIL];
		char	buf_flag[MIL];

		/* equip a False perché non può vederlo anche se potrebbe interagirci */
		if ( !can_see_obj(ch, obj, FALSE) )
			continue;

		/* Se pesa meno di un chilo visualizza in grammi */
		if ( obj->weight / 1000 == 0 )
		{
			sprintf( buf_weight, "L'oggetto pare pesare circa %d gramm%c\r\n",
				obj->weight, (obj->weight == 1) ? 'o' : 'i' );
		}
		else
		{
			sprintf( buf_weight, "L'oggetto pare pesare circa %d chil%c\r\n",
				obj->weight/1000, (obj->weight/1000 == 1) ? 'o' : 'i' );
		}

		strcpy( buf_flag, get_oflag_string(ch, obj) );

		/* Prima cerca tra le extra dell'oggetto in maniera esatta */
		pdesc = get_extradescr( arg, obj->first_extradescr );

		/* Poi se ancora non ha trovato nulla cerca in maniera prefix negli oggetti non index
		 *	non esegue la ricerca se s è passato un argemento troppo corto */
		if ( !pdesc && strlen(arg) >= 2 )
			pdesc = get_extradescr_prefix( arg, obj->first_extradescr );

		/* Se ha trovato qualcosa, o cmq se l'oggetto esiste anche se non ha descr, invia il tutto al ch */
		if ( pdesc || nifty_is_name_prefix(arg, obj->name) )
		{
			cnt += obj->count;
			if ( cnt < number )
				continue;

			/* Se non ha ancora trovato la extra descr prova a cercarne una con il nome dell'oggetto */
			if ( !pdesc )
				pdesc = get_extradescr( obj->name, obj->first_extradescr );	/* (FF) questo sistema sarebbe quello che in futuro sarà la descrizione dell'oggetto */

			if ( !pdesc )
				send_to_char( ch, "Non c'è nulla di speciale..\r\n" );
			else
				ch_printf( ch, "%s\r\n", pdesc );
			ch_printf( ch, "%s%s", buf_weight, buf_flag );

			if ( EXA_prog_trigger )
				oprog_examine_trigger( ch, obj );

			return;
		}
	}

	/*
	 * Guardare tra gli oggetti della stanza
	 */
	for ( obj = ch->in_room->last_content;  obj;  obj = obj->prev_content )
	{
		char	buf_weight[MIL];
		char	buf_flag[MIL];

		if ( !can_see_obj(ch, obj, FALSE) )
			continue;

		/* Se pesa meno di un chilo visualizza in grammi */
		if ( obj->weight / 1000 == 0 )
		{
			sprintf( buf_weight, "L'oggetto pare pesare circa %d gramm%c\r\n",
				obj->weight, (obj->weight == 1) ? 'o' : 'i' );
		}
		else
		{
			sprintf( buf_weight, "L'oggetto pare pesare circa %d chil%c\r\n",
				obj->weight/1000, (obj->weight/1000 == 1) ? 'o' : 'i' );
		}

		strcpy( buf_flag, get_oflag_string(ch, obj) );

		/* Prima cerca tra le extra dell'oggetto in maniera esatta */
		pdesc = get_extradescr( arg, obj->first_extradescr );

		/* Poi cerca tra le extra dell'oggetto in maniera prefissa */
		if ( !pdesc && strlen(arg) >= 2 )
			pdesc = get_extradescr_prefix( arg, obj->first_extradescr );

		if ( pdesc || nifty_is_name_prefix(arg, obj->name) )
		{
			cnt += obj->count;
			if ( cnt < number )
				continue;

			/* Se non ha ancora trovato una extra descr prova con il nome dell'oggetto */
			if ( !pdesc )
				pdesc = get_extradescr( obj->name, obj->first_extradescr );	/* (FF) questo sistema sarebbe quello che in futuro sarà la descrizione dell'oggetto */

			if ( !pdesc )
				send_to_char( ch, "Non noto nulla di speciale.\r\n" );
			else
				ch_printf( ch, "%s\r\n", pdesc );
			ch_printf( ch, "%s%s", buf_weight, buf_flag );

			if ( EXA_prog_trigger )
				oprog_examine_trigger( ch, obj );

			return;
		}
	}

	set_char_color( AT_PLAIN, ch );
	send_to_char( ch, "Non vedo nulla del genere qui.\r\n" );
}


/*
 * Comando esamina, invia una prima parte attraverso il look con in più
 *	anche altre informazioni suddivise per tipologia di oggetto
 */
DO_RET do_examine( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA   *obj;
	BOARD_DATA *board;
	char		buf[MSL];
	int			dam;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_examine: ch è NULL" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Devo esaminare cosa?\r\n" );
		return;
	}

	/* Invia la descrizione dell'oggetto */
    EXA_prog_trigger = FALSE;
	sprintf( buf, "look %s", argument );
    EXA_prog_trigger = TRUE;
	send_command( ch, buf, CO );

	/*
	 * Support for looking at boards, checking equipment conditions,
	 *	and support for trigger positions.
	 */
	obj = get_obj_here( ch, argument );
	if ( !obj )
		return;

	board = get_board( obj );
	if ( board )
	{
		if ( board->num_posts )
		{
			ch_printf( ch, "Ci sono %d note inviate.  Digita '%s lista' per avere la lista.\r\n",
				board->num_posts, translate_command(ch, "note") );
		}
		else
		{
			send_to_char( ch, "Nessuna nota nuova.\r\n" );
		}
	}

	switch ( obj->type )
	{
	  default:
		break;

	  case OBJTYPE_ARMOR:
	  case OBJTYPE_DRESS:
		if ( obj->armor->ac_orig == 0 )
			obj->armor->ac_orig = obj->armor->ac;
		if ( obj->armor->ac_orig == 0 )
			obj->armor->ac_orig = 1;
		dam = (int) ( (obj->armor->ac * 10) / obj->armor->ac_orig );
		strcpy( buf, "Osservando attentamente, noti che è " );

		if		( dam >= 10 )	strcat( buf, "perfetto."				);
		else if ( dam ==  9 )	strcat( buf, "quasi perfetto."			);
		else if ( dam ==  8 )	strcat( buf, "in ottime condizioni."	);
		else if ( dam ==  7 )	strcat( buf, "in buone condizioni."		);
		else if ( dam ==  6 )	strcat( buf, "un po rovinato."			);
		else if ( dam ==  5 )	strcat( buf, "rovinato."				);
		else if ( dam ==  4 )	strcat( buf, "in condizioni mediocri."	);
		else if ( dam ==  3 )	strcat( buf, "estremamente rovinato."	);
		else if ( dam ==  2 )	strcat( buf, "pieno di crepe."			);
		else if ( dam ==  1 )	strcat( buf, "senza valore."			);
		else if ( dam <=  0 )	strcat( buf, "completamente rotto."		);

		strcat( buf, "\r\n" );
		send_to_char( ch, buf );
		break;

	  case OBJTYPE_WEAPON:
		dam = INIT_WEAPON_CONDITION - obj->weapon->condition;
		strcpy( buf, "Osservando attentamente, noti che è ");

		if		( dam ==  0 )	strcat( buf, "in superbe condizioni."				);
		else if ( dam ==  1 )	strcat( buf, "in condizioni eccellenti."			);
		else if ( dam ==  2 )	strcat( buf, "in ottime condizioni."				);
		else if ( dam ==  3 )	strcat( buf, "in buone condizioni."					);
		else if ( dam ==  4 )	strcat( buf, "sembra lievemente incrinata."			);
		else if ( dam ==  5 )	strcat( buf, "sembra un po' deformata."				);
		else if ( dam ==  6 )	strcat( buf, "ha bisogno di una riparazione."		);
		else if ( dam ==  7 )	strcat( buf, "ha urgente bisogno di riparazioni."	);
		else if ( dam ==  8 )	strcat( buf, "è in condizioni disastrose."			);
		else if ( dam ==  9 )	strcat( buf, "è quasi irriconoscibile."				);
		else if ( dam == 10 )	strcat( buf, "è ormai un oggetto senza valore."		);
		else if ( dam == 11 )	strcat( buf, "è praticamente inutilizzabile."		);
		else if ( dam == 12 )	strcat( buf, "è ormai completamente distrutta."		);

		strcat( buf, "\r\n" );
		send_to_char( ch, buf );
		break;

	  case OBJTYPE_COOK:
		strcpy( buf, "Osservandolo noti che è " );
		dam = obj->food->dam_cook;
		if		( dam >= 3 )	strcat( buf, "un ammasso bruciacchiato!" );
		else if ( dam == 2 )	strcat( buf, "cotto alla brace."		 );
		else if ( dam == 1 )	strcat( buf, "cotto a puntino."			 );
		else					strcat( buf, "crudo."					 );
		strcat( buf, "\r\n" );
		send_to_char( ch, buf );
		/* Niente break */

	  case OBJTYPE_FOOD:
		if ( obj->timer > 0 && obj->food->condition > 0 )
			dam = (obj->timer * 10) / obj->food->condition;
		else
			dam = 10;

		if ( obj->type == OBJTYPE_FOOD )
			strcpy( buf, "Esamino attentamente il cibo e noto che " );
		else
			strcpy( buf, "Inoltre " );

		if		( dam >= 10 )	strcat( buf, "sembra una vera delizia." );
		else if ( dam ==  9 )	strcat( buf, "è come un frutto appena colto." );
		else if ( dam ==  8 )	strcat( buf, "sembra ottimo." );
		else if ( dam ==  7 )	strcat( buf, "sembra davvero buono." );
		else if ( dam ==  6 )	strcat( buf, "sembra buono." );
		else if ( dam ==  5 )	strcat( buf, "sembra un po' avvizzito." );
		else if ( dam ==  4 )	strcat( buf, "è un pò avvizzito.." );
		else if ( dam ==  3 )	strcat( buf, "puzza un pò.." );
		else if ( dam ==  2 )	strcat( buf, "puzza di muffa.." );
		else if ( dam ==  1 )	strcat( buf, "ha un odore rivoltante!" );
		else if ( dam <=  0 )	strcat( buf, "Bleah! Pieno di vermi!" );

		strcat( buf, "\r\n" );
		send_to_char( ch, buf );
		break;


	  case OBJTYPE_SWITCH:
	  case OBJTYPE_LEVER:
	  case OBJTYPE_PULLCHAIN:
		if ( HAS_BITINT(obj->lever->flags, TRIG_UP) )
			send_to_char( ch, "Noto che si trova posizionato il alto.\r\n" );
		else
			send_to_char( ch, "Noto che si trova posizionato il basso.\r\n" );
		break;

	  case OBJTYPE_BUTTON:
		if ( HAS_BITINT( obj->lever->flags, TRIG_UP ) )
			send_to_char( ch, "Noto che è premuto.\r\n" );
		else
			send_to_char( ch, "Noto che non è premuto.\r\n" );
		break;

	  case OBJTYPE_CORPSE_PG:
	  case OBJTYPE_CORPSE_MOB:
	  {
		int timerfrac = obj->timer;
		if ( obj->type == OBJTYPE_CORPSE_PG )
			timerfrac = (int) obj->timer/8 + 1;

		switch ( timerfrac )
		{
		  default:
			send_to_char( ch, "Questo cadavere è stato ucciso di recente.\r\n" );
			break;
		  case 4:
			send_to_char( ch, "Questo cadavere è stato ucciso da poco tempo.\r\n" );
			break;
		  case 3:
			send_to_char( ch, "Percepisco un terribile tanfo si solleva dal cadavere che irrigidito dal rigor mortis è riverso in terra.\r\n" );
			break;
		  case 2:
			send_to_char( ch, "Vermi e insetti hanno depredato d'ogni viscera questo cadavere, irriconoscibile.\r\n" );
			break;
		  case 1:
		  case 0:
			send_to_char( ch, "Di quello che era un corpo qui rimangono solo poche ossa, i vermi hanno preso il resto.\r\n" );
			break;
		}
		/* Niente break */
	  } /* corpse */

	  case OBJTYPE_CONTAINER:
		if ( HAS_BIT(obj->extra_flags, OBJFLAG_COVERING) )
			break;
		/* Niente break */

	  case OBJTYPE_DRINK_CON:
	  case OBJTYPE_QUIVER:
		send_to_char( ch, "Guardando al suo interno trovo:\r\n" );
		/* Niente break */

	  case OBJTYPE_KEYRING:
		EXA_prog_trigger = FALSE;
		sprintf( buf, "look in %s", argument );
		EXA_prog_trigger = TRUE;
		send_command( ch, buf, CO );
		break;
	} /* chiude lo switch */

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_COVERING) )
	{
		EXA_prog_trigger = FALSE;
		sprintf( buf, "look under %s", argument );
		EXA_prog_trigger = TRUE;
		send_command( ch, buf, CO );
	}

	oprog_examine_trigger( ch, obj );
	if ( char_died(ch) || obj_extracted(obj) )
		return;

	check_for_trap( ch, obj, TRAPFLAG_EXAMINE );
}


/*
 * Comando per leggere libri, note e iscrizioni
 */
DO_RET do_read( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char	  buf[MIL];

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Che cosa dovrei leggere?\r\n" );
		return;
	}

	obj = get_obj_here( ch, argument );

	if ( obj->type != OBJTYPE_SCROLL || obj->type != OBJTYPE_NOTE
	  || obj->type != OBJTYPE_BOOK	  || obj->type != OBJTYPE_MAP
	  || obj->type != OBJTYPE_PAPER )
	{
		send_to_char( ch, "Non saprei proprio come leggere questo tipo di oggetto.\r\n" );
		return;
	}

	if ( obj->paper )
	{
		send_to_char( ch, "Leggo la lettera:\r\n" );
		ch_printf( ch, "Da:       %s\r\n",	get_extradescr("_sender_",  obj->first_extradescr) );
		ch_printf( ch, "A:        %s\r\n",	get_extradescr("_to_",	  obj->first_extradescr) );
		ch_printf( ch, "Soggetto: %s\r\n",	get_extradescr( "_subject_", obj->first_extradescr) );
		ch_printf( ch, "%s\r\n",			get_extradescr("_text",	  obj->first_extradescr) );
		return;
	}

	sprintf( buf, "look %s", argument );
	send_command( ch, buf, CO );
}


/*
 * Comando da utilizzare in presenza di mob che possano identificare oggetti al giocatore
 */
void do_identify( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA	*obj;
	CHAR_DATA	*rch;
	char		 buf[MSL];
	int			 value;

	if ( (obj = get_obj_carry(ch, argument, TRUE)) == NULL )
	{
		send_to_char( ch, "Non hai nessun oggetto con quel nome.\r\n" );
		return;
	}

	for ( rch = ch->in_room->first_person;  rch;  rch = rch->next_in_room )
	{
		if ( IS_MOB(rch) && rch->pIndexData->vnum == VNUM_MOB_IDENTIFY )
			break;
	}

	if ( !rch )
	{
		send_to_char( ch, "Non c'è chi possa dirti qualcosa riguardo quell'oggetto.\r\n" );
		return;
	}

	/* equip a FALSE perché non riesce ad identificarlo correttamente se non lo vede */
	if ( !can_see_obj(rch, obj, FALSE) )
	{
		sprintf( buf, "say a %s Non riesco a vedere tale oggetto.", ch->name );
		send_command( rch, buf, CO );
		return;
	}

	value = obj->level * 50 + 150;
	if ( ch->gold < value )
	{
		act( AT_ACTION, "$n esamina scrupolosamente $p.", rch, obj, 0, TO_ROOM );
		sprintf( buf, "say a %s Servono almeno %d monete d'oro per l'identificazione.", ch->name, value );
		send_command( rch, buf, CO );
		return;
	}
	else
	{
		ch->gold -= value;
		ch_printf( ch, "Pago %s per ottenere un'identificazione adeguata.\r\n", rch->short_descr );
	}

	act( AT_ACTION, "$n esamina scrupolosamente $p.", rch, obj, 0, TO_ROOM );
	identify_object( ch, obj );
}


/*
 * Comando che elenca le uscite di una stanza.
 */
DO_RET do_exits( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA *pexit;
	char	   buf[MSL];
#ifdef T2_MXP
	int		   spaces;
#endif
	bool	   found;
	bool	   fAuto;
	bool	   allexits = FALSE;

	fAuto  = ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_AUTOEXIT) );

	if ( !check_blind(ch) )
		return;

	set_char_color( AT_EXITS, ch );
	strcpy( buf, (fAuto) ? "Uscite:&w\r\n" : "Uscite:&w" );

	if ( IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ALLEXITS) )
		allexits = TRUE;

	found = FALSE;
	for ( pexit = ch->in_room->first_exit;  pexit;  pexit = pexit->next )
	{
		char	color[8];
		char	tag_ex_open[8];
		char	tag_ex_close[8];
		char	dir_it[MIL];
		char	dir_en[MIL];

		/* Se l'uscita non porta da nessuna parte continua il ciclo */
		if ( !pexit->to_room )
		{
			send_log( NULL, LOG_BUG, "do_exits: uscita NULL alla stanza #%d", ch->in_room->vnum );
			continue;
		}

		/* A meno che non sia un Admin non visualizza le direzioni i tipologia somewhere/portale richiamate con keyword */
		if ( pexit->vdir == DIR_SOMEWHERE
		  || (HAS_BIT(pexit->flags, EXIT_WINDOW) && !HAS_BIT(pexit->flags, EXIT_ISDOOR)) )
		{
			if ( !allexits )
				continue;
		}

		if ( (HAS_BIT(pexit->flags, EXIT_HIDDEN)
		  || HAS_BIT(pexit->flags, EXIT_SECRET))
/*		  || HAS_BIT(pexit->flags, EXIT_DIG)
		  || HAS_BIT(pexit->flags, EXIT_xSEARCHABLE)
		  || HAS_BIT(pexit->flags, EXIT_PORTAL) (TT) da controllare */
		  && HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			if ( !allexits )
				continue;
		}

		/* colora le direzioni delle uscite in maniera differente */
		switch ( pexit->vdir )
		{
		  case DIR_NORTH:		strcpy( color, "&c" );	break;	/* (FF) AT_dir */
		  case DIR_EAST:		strcpy( color, "&Y" );	break;
		  case DIR_SOUTH:		strcpy( color, "&W" );	break;
		  case DIR_WEST:		strcpy( color, "&R" );	break;
		  case DIR_UP:			strcpy( color, "&C" );	break;
		  case DIR_DOWN:		strcpy( color, "&O" );	break;
		  case DIR_NORTHEAST:	strcpy( color, "&g" );	break;
		  case DIR_NORTHWEST:	strcpy( color, "&G" );	break;
		  case DIR_SOUTHEAST:	strcpy( color, "&B" );	break;
		  case DIR_SOUTHWEST:	strcpy( color, "&p" );	break;
		}

		if ( HAS_BIT(pexit->flags, EXIT_CLOSED) )
		{
			strcpy( tag_ex_open, "[" );
			strcpy( tag_ex_close, "]" );
		}
		else if ( (IS_ADMIN(ch) || ((ch->class == CLASS_THIEF) && HAS_BIT(ch->affected_by, AFFECT_DETECT_TRAPS)) )
		  && HAS_BIT(pexit->to_room->flags, ROOM_DEATH) )
		{
			strcpy( tag_ex_open, ">" );
			strcpy( tag_ex_close, "<" );
		}
		else
		{
			if ( fAuto )
			{
				strcpy( tag_ex_open, " " );
				strcpy( tag_ex_close, " " );
			}
			else
			{
				strcpy( tag_ex_open, "" );
				strcpy( tag_ex_close, "" );
			}
		}

		found = TRUE;

		strcpy( dir_it, code_name(NULL, pexit->vdir, CODE_DIR) );
		strcpy( dir_en, table_dir[pexit->vdir].eng );

		if ( fAuto )
		{
#ifdef T2_MXP
			/* I don't want to underline spaces, so I'll calculate the number we need */
			if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
				spaces = 9 - strlen( dir_it );
			else
				spaces = 9 - strlen( dir_en );

			if ( spaces < 0 )
				spaces = 0;

			sprintf( buf + strlen(buf), "%s" MXPTAG("ex") "%s%s&w" MXPTAG("/ex") "%s%*s %c %s",
				tag_ex_open,
				color,
				(IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN))  ?  capitalize(dir_it)  :  capitalize(dir_en),
				tag_ex_close,
				spaces,			/* number of spaces */
				"",
				(allexits)  ?  '*'  :  '-',
				room_is_dark(pexit->to_room) && !HAS_BIT(pexit->flags, EXIT_CLOSED)  ?  "Troppo scuro per saperlo."  :
					HAS_BIT(pexit->flags, EXIT_CLOSED)  ?  "Chiusa"  :  pexit->to_room->name );	/* (RR) Inserire al posto di chiusa il nome del tipo di porta */
#else
			sprintf( buf + strlen(buf), "%s%s%-9s&w%s %c %s",
				tag_ex_open,
				color,
				(IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN))  ?  capitalize(dir_it)  :  capitalize(dir_en),
				tag_ex_close,
				(allexits)  ?  '*'  :  '-',
				room_is_dark(pexit->to_room) && !HAS_BIT(pexit->flags, EXIT_CLOSED)  ?  "Troppo scuro per saperlo."  :
					HAS_BIT(pexit->flags, EXIT_CLOSED)  ?  "Chiusa"  :  pexit->to_room->name );	/* (RR) Inserire al posto di chiusa il nome del tipo di porta */
#endif
			if ( IS_ADMIN(ch) )
			{
				if ( HAS_BIT_PLR(ch, PLAYER_VNUM) )
					sprintf( buf + strlen(buf), " &w(#%d)", pexit->vnum );

				if ( HAS_BIT_PLR(ch, PLAYER_FLAG) )
				{
					if ( IS_EMPTY(pexit->flags) )
						sprintf( buf + strlen(buf), " &w(Flag: nessuna)" );
					else
						sprintf( buf + strlen(buf), " &w(Flag: %s)", code_bit(NULL, pexit->flags, CODE_EXIT) );
				}

				if ( allexits )
				{
					if ( VALID_STR(pexit->keyword) )
						sprintf( buf + strlen(buf), " &w(Keyword: %s)", pexit->keyword );
					if ( HAS_BIT(pexit->flags, EXIT_LOCKED) )
						sprintf( buf + strlen(buf), " &w(Key: #%d)", pexit->key );
				}
			}

			strcat( buf, "\r\n" );
		}
		else
		{
#ifdef T2_MXP
			sprintf( buf + strlen(buf), " %s" MXPTAG("ex") "%s%s&w" MXPTAG("/ex") "%s",
				tag_ex_open,
				color,
				(IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN))  ?  capitalize(dir_it)  :  capitalize(dir_en),
				tag_ex_close );
#else
			sprintf( buf + strlen(buf), " %s%s%s&w%s",
				tag_ex_open,
				color,
				(IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN))  ?  capitalize(dir_it)  :  capitalize(dir_en),
				tag_ex_close );
#endif
		}
	} /* chiude il for */

	if		( !found )
		strcat( buf, (fAuto) ? "Nessuna.\r\n" : "nessuna.\r\n" );
	else if ( !fAuto )
		strcat( buf, ".\r\n" );

	send_to_char( ch, buf );
}


/*
 * LAWS command
 */
DO_RET do_rules( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	if ( !VALID_STR(argument) )
		send_command( ch, "help RULES", CO );
	else
	{
		sprintf( buf, "help RULES %s", argument );
		send_command( ch, buf, CO );
	}
}


DO_RET do_compare( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	char	 *msg;
	char	  arg[MIL];
	int		  value1;
	int		  value2;

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Devo confrontare con cosa?\r\n" );
		return;
	}

	obj1 = get_obj_carry( ch, arg, TRUE );
	if ( !obj1 )
	{
		send_to_char( ch, "Spiacente non posseggo quest'oggetto.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		for ( obj2 = ch->first_carrying;  obj2;  obj2 = obj2->next_content )
		{
			int		x;
			bool	found = FALSE;

			if ( obj2->wear_loc == WEARLOC_NONE )	continue;
			/* equip a FALSE, difatti come farebbe a comparare bene se non vede l'oggetto? */
			if ( !can_see_obj(ch, obj2, FALSE) )	continue;
			if ( obj1->type != obj2->type )			continue;

			/* Controlla se obj e obj2 hanno una flag di wear in comune, escluso il take */
			for ( x = 0;  x < MAX_OBJWEAR;  x++ )
			{
				if ( x == OBJWEAR_TAKE )
					continue;

				if ( HAS_BIT(obj1->wear_flags, x) && HAS_BIT(obj2->wear_flags, x) )
				{
					found = TRUE;
					break;
				}
			}
			if ( found )
				break;
		}

		if ( !obj2 )
		{
			send_to_char( ch, "Non sto portando nulla con cui poterlo comparare.\r\n" );
			return;
		}
	}
	else
	{
		obj2 = get_obj_carry( ch, argument, TRUE );
		if ( !obj2 )
		{
			send_to_char( ch, "Non ho quell'oggetto.\r\n" );
			return;
		}
	}

	msg		= NULL;
	value1	= 0;
	value2	= 0;

	if ( obj1 == obj2 )
	{
		msg = "Confronto $p con se stesso.. Chissà come mai mi sembra lo stesso!";
	}
	else if ( obj1->type != obj2->type )
	{
		msg = "Non posso confrontare $p e $P.";
	}
	else
	{
		/* Qui check sugli ITEM_ e non sulla possibile funzionalità dell'oggetto */
		switch ( obj1->type )
		{
		  default:
			msg = "Non posso confrontare $p e $P.";
			break;

		  case OBJTYPE_ARMOR:
		  case OBJTYPE_DRESS:
			value1 = obj1->armor->ac;
			value2 = obj2->armor->ac;
			break;

		  case OBJTYPE_WEAPON:
			value1 = obj1->weapon->dice_size;
			value2 = obj2->weapon->dice_size;
			break;
		}
	}

	if ( !msg )
	{
		if		( value1 == value2 ) msg = "$p e $P sembrano uguali.";
		else if ( value1  > value2 ) msg = "$p sembra migliore di $P.";
		else						 msg = "$p sembra peggiore di $P.";
	}

	act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
}


DO_RET do_consider( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	  *msg;
	int		   diff;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Chi dovrei considerare?\r\n" );
		return;
	}

	victim = get_char_room( ch, argument, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non lo vedo nei paraggi.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Considerare me stesso? Meglio considerare altre persone.\r\n" );
		return;
	}

	diff = get_level( victim ) - get_level( ch );

	if		( diff <= -10 )	msg = "Non credo che $N abbia una sola speranza di vittoria contro di me.";
	else if ( diff <=  -5 )	msg = "Sembra che io e  $N siamo equivalenti, sarebbe un buon duello.";
	else if ( diff <=  -2 )	msg = "Penso che $N non mi debba dare troppi problemi in un duello.";
	else if ( diff <=   1 )	msg = "Credo che  $N sia quasi alla mia portata.. forse impegnandomi a fondo ce la potrei fare.";
	else if ( diff <=   4 )	msg = "Non credo d'avere molte possibilità contro di $N.";
	else if ( diff <=   9 )	msg = "Non ho assolutamente possibilità di vincere contro $N!";
	else					msg = "$N potrebbe addirittura insegnarmi a combattere!";

	act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

	diff = (int) (victim->points_max[POINTS_LIFE] - ch->points[POINTS_LIFE]) / 6;

	if		( diff <= -200 )	msg = "$N non si regge più in piedi! E' già fatta.";
	else if	( diff <= -150 )	msg = "Potrei uccidere $N con una sola mano!";
	else if	( diff <= -100 )	msg = "$N è nettamente in svantaggio, non avrò problemi.";
	else if	( diff <=  -50 )	msg = "Mi chiedo quando $N si deciderà a scappare viste le sue condizioni..";
	else if	( diff <=    0 )	msg = "$N dovrei farcela senza problemi.. credo..";
	else if	( diff <=   50 )	msg = "$N ridacchia con aria di sfida.";
	else if	( diff <=  100 )	msg = "Dovrò impegnarmi per farcela...";
	else if	( diff <=  150 )	msg = "Oltre all'impegno.. avrò bisogno di fortuna, molta fortuna...";
	else if	( diff <=  200 )	msg = "Per farcela avrò bisogno di impegno.. fortuna.. ed un piccolo miracolo!";
	else						msg = "Meglio lasciar perdere $N.. non mi sembra..dell'umore adatto!!";

	act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );
}


DO_RET do_credits( CHAR_DATA *ch, char *argument )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_credits: ch è NULL" );
		return;
	}

	send_command( ch, "help credits", CO );
}


/*
 * Comando do_areas: Lists areas in alphanumeric order
 * (FF) da fare un ordinamento per livelli e vnum
 */
DO_RET do_areas( CHAR_DATA *ch, char *argument )
{
	AREA_DATA	*pArea;

	set_char_color( AT_PLAIN, ch );
	if ( IS_ADMIN(ch) )
	{
		send_to_pager( ch, "\r\n  Autore/i         |  Area                          | Livelli |  Vnum         \r\n" );
		send_to_pager( ch,     "-------------------+--------------------------------+---------+---------------\r\n" );
	}
	else
	{
		send_to_pager( ch, "\r\n  Autore/i         |  Area                          |  Difficoltà              \r\n" );
		send_to_pager( ch,     "-------------------+--------------------------------+--------------------------\r\n" );
	}

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		if ( IS_ADMIN(ch) )
		{
			pager_printf( ch, "%-18s | &%c%-30s&w | %3d-%3d | %6d-%6d\r\n",
				pArea->author,
				get_clr(pArea->color->cat),
				pArea->name,
				pArea->level_low,
				pArea->level_high,
				pArea->vnum_low,
				pArea->vnum_high );
		}
		else
		{
			char   *difficult;	/* Stringa di campo difficoltà che il pg leggerà */
			int		lvl_diff;	/* Differenza di livello tra quello medio dell'area e quello del pg */

			if ( HAS_BIT(pArea->flags, AREAFLAG_SECRET) || HAS_BIT(pArea->flags, AREAFLAG_WILD) )
				continue;

			lvl_diff = ((pArea->level_high + pArea->level_low) / 2) - get_level( ch );
			if		( lvl_diff  < -5 )	difficult = "La mangio a colazione!";
			else if ( lvl_diff  <  0 )	difficult = "Facile facile";
			else if ( lvl_diff  <  5 )	difficult = "Adatta a me!";
			else if ( lvl_diff  < 10 )	difficult = "Comincia l'avventura!";
			else if ( lvl_diff  < 15 )	difficult = "Uhmm.. difficile";
			else 						difficult = "Diventerò eroe o morto?";

			pager_printf( ch, "%-18s | %-30s | %-24s |\r\n",
				pArea->author,
				pArea->name,
				difficult );
		}
	}
}


/*
 * Comando Version info
 */
DO_RET do_version( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
		return;

	ch_printf( ch, "Codice %s %s.%s release %s per %s del %s\r\n",
		MUD_CODE_NAME,
		MUD_VERSION_MAJOR,
		MUD_VERSION_MINOR,
		MUD_RELEASE_NAME,
		MUD_NAME,
		MUD_TEAM_NAME );

	if ( IS_ADMIN(ch) )
		ch_printf(ch, "Compilato il %s alle %s.\r\n", __DATE__, __TIME__ );
}


/*
 * Genera una messaggio di azione descrittiva.
 */
void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj )
{
	char		charbuf[MSL];
	char		roombuf[MSL];
	char	   *srcptr = obj->action_descr;
	char	   *charptr = charbuf;
	char	   *roomptr = roombuf;
	const char *ichar = "Mi";
	const char *iroom = "Qualcuno";

	while ( VALID_STR(srcptr) )
	{
		if ( *srcptr == '$' )
		{
			srcptr++;
			switch ( *srcptr )
			{
			  case 'e':
				ichar = "mi";
				iroom = "$e";
				break;

			  case 'm':
				ichar = "mi";
				iroom = "$m";
				break;

			  case 'n':
				ichar = "mi";
				iroom = "$n";
				break;

			  case 's':
				ichar = "mio";
				iroom = "$x";
				break;

/*			  case 'q':
				iroom = "s";
				break;
*/
			  default:
				srcptr--;
				*charptr++ = *srcptr;
				*roomptr++ = *srcptr;
				break;
			}
		}
		else if ( (*srcptr == '%') && (*++srcptr == 's') )
		{
			ichar = "Mi";
			iroom = IS_MOB( ch ) ? ch->short_descr : ch->name;
		}
		else
		{
			*charptr++ = *srcptr;
			*roomptr++ = *srcptr;
			srcptr++;
			continue;
		}

		while ( (*charptr = *ichar) != '\0' )
		{
			charptr++;
			ichar++;
		}

		while ( (*roomptr = *iroom) != '\0' )
		{
			roomptr++;
			iroom++;
		}
		srcptr++;
	} /* chiude il while */

	*charptr = '\0';
	*roomptr = '\0';

/*
	send_log( NULL, LOG_HIGH, "Charbuf: %s", charbuf );
	send_log( NULL, LOG_HIGH, "Roombuf: %s", roombuf );
*/
	switch ( obj->type )
	{
	  case OBJTYPE_BLOOD:
	  case OBJTYPE_FOUNTAIN:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		break;

	  case OBJTYPE_DRINK_CON:
		act( AT_ACTION, charbuf, ch, obj, table_liquid[obj->drinkcon->liquid].name, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, table_liquid[obj->drinkcon->liquid].name, TO_ROOM );
		break;

	  case OBJTYPE_PIPE:
		break;

	  case OBJTYPE_ARMOR:
	  case OBJTYPE_DRESS:
	  case OBJTYPE_WEAPON:
	  case OBJTYPE_LIGHT:
		break;

	  case OBJTYPE_COOK:
	  case OBJTYPE_FOOD:
	  case OBJTYPE_PILL:
		act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
		act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
		break;

	  default:
		break;
	}
}
