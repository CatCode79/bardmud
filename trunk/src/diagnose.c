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
 >					Gorog's Revenge on Unruly Bastards						<
\****************************************************************************/

#include <ctype.h>

#include "mud.h"
#include "db.h"
#include "interpret.h"
#include "room.h"


/*
 * Truncate a char string if it's length exceeds a given value.
 */
void truncate( char *s, const int len )
{
	if ( strlen(s) > len )
		s[len] = '\0';
}

/*
 * New owhere
 */
DO_RET do_owhere( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	OBJ_DATA *outer_obj;
	char	  buf[MSL], field[MIL];
	char	  arg[MIL];
	int		  icnt = 0;
	VNUM	  vnum = 0;
	bool	  found = FALSE;
	char	  heading[] ="	Vnum	Short Desc		Vnum  Room/Char		  Vnum  Container\r\n";

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Owhere what?\r\n" );
		return;
	}

	if ( is_number(arg) )
		vnum = atoi(arg);

	for ( obj = first_object;  obj;  obj = obj->next )
	{
		if ( vnum )
		{
			if ( vnum != obj->vnum )
				continue;
		}
		else if ( !nifty_is_name(arg, obj->name) )
			continue;

		if ( !found )
			send_to_pager( ch, heading );		/* print report heading */

		found = TRUE;

		outer_obj = obj;
		while ( outer_obj->in_obj )
			outer_obj = outer_obj->in_obj;

		sprintf( field, "%-18s", obj_short(obj) );
		truncate( field, 18 );
		sprintf( buf, "%3d %5d %-18s ", ++icnt, obj->vnum, field );

		if ( outer_obj->carried_by )
		{
			sprintf( field, "%-18s", PERS(outer_obj->carried_by, ch) );
			truncate( field, 18 );
			sprintf( buf + strlen(buf), "%5d %-18s ",
				(IS_MOB(outer_obj->carried_by))  ?  outer_obj->carried_by->pIndexData->vnum  :  0, field );

			if ( outer_obj!=obj )
			{
				sprintf( field, "%-18s", obj->in_obj->name );
				truncate( field, 18 );
				sprintf( buf+strlen(buf), "%5d %-18s ",
					obj->in_obj->vnum, field );
			}

			sprintf( buf+strlen(buf), "\r\n" );
			send_to_pager( ch, buf );
		}
		else if ( outer_obj->in_room )
		{
			sprintf( field, "%-18s", outer_obj->in_room->name );
			truncate( field, 18 );
			sprintf( buf+strlen(buf), "%5d %-18s ",
				outer_obj->in_room->vnum, field );

			if ( outer_obj!=obj )
			{
				sprintf( field, "%-18s", obj->in_obj->name );
				truncate( field, 18 );
				sprintf( buf+strlen(buf), "%5d %-18s ",
					obj->in_obj->vnum, field );
			}

			sprintf( buf+strlen(buf), "\r\n" );
			send_to_pager( ch, buf );
		}
	}
	if ( !found )
		send_to_pager( ch, "None found.\r\n" );
}


/*
 * Sort function used by rgrub to sort integers
 */

int rgrub_int_comp(const void *i, const void *j)
{
	return *(int*)i - *(int*)j;
}


/*
 * Displays the help screen for the "rgrub" command
 */
void rgrub_help( CHAR_DATA *ch )
{
	send_to_char( ch, "&YSintassi&w:\r\n" );
	send_to_char( ch, "rgrub st n lo hi - sector type search.\r\n" );
	send_to_char( ch, "   list room vnums between lo and hi that match n.\r\n" );

	send_to_char( ch, "   e.g. rgrub st 6 901 969 - list all rooms in Olympus\r\n" );
	send_to_char( ch, "	  that are sectortype 6.\r\n" );
	send_to_char( ch, "   e.g. rgrub st 2 - list all rooms sectortype 2.\r\n" );
}


DO_RET do_rgrub( CHAR_DATA *ch, char *argument )
{
	char	arg1[MSL];
	char	arg2[MSL];
	char	arg3[MSL];
	char	arg4[MSL];

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	argument = one_argument (argument, arg3);
	argument = one_argument (argument, arg4);

	if ( !str_cmp(arg1, "st") )
	{
		#define RGRUB_ST_MAX_SIZE 5000

		ROOM_DATA	*pRoom;
		int			 match;
		int			 hit_cou;
		int			 cou;
		VNUM		 lo;
		VNUM		 hi;
		VNUM		 vnum[RGRUB_ST_MAX_SIZE];

		if ( !VALID_STR(arg2) )			/* empty arg gets help scrn */
		{
			rgrub_help(ch);
			return;
		}
		else
			match = atoi (arg2);

		hit_cou = 0;				/* number of vnums found */
		lo = (*arg3)  ?  atoi (arg3)  :  1;			/* (TT) era 0 */
		hi = (*arg4)  ?  atoi (arg4)  :  MAX_VNUM;

		send_to_char( ch, "\r\nRoom Vnums\r\n" );
		for ( cou = 0;  cou < MAX_KEY_HASH;  cou++ )
		{
			if ( room_index_hash[cou] )
			{
				for ( pRoom = room_index_hash[cou];  pRoom;  pRoom = pRoom->next )
				{
					if ( pRoom->vnum >= lo && pRoom->vnum <= hi )
					{
						if ( match == pRoom->sector && hit_cou < RGRUB_ST_MAX_SIZE )
							vnum[hit_cou++] = pRoom->vnum;
					}
				}
			}
		}
		qsort( vnum, hit_cou, sizeof(int), rgrub_int_comp );	/* sort vnums	 */
		for ( cou = 0;  cou < hit_cou;  cou++ )
			ch_printf( ch, "%5d %6d\r\n", cou+1, vnum[cou] );	/* display vnums */

		return;
	}
	else
	{
		rgrub_help(ch);
		return;
	}
}


/*
 * The "showlayers" command is used to list all layerable eq in the
 *	mud so that we can keep track of it.
 * It lists one line for each piece of unique eq.
 * If there are 1,000 shrouds in the game, it doesn't list 1,000 lines
 *	for each shroud - just one line for the shroud.
 */
DO_RET do_showlayers( CHAR_DATA *ch, char *argument )
{
	OBJ_PROTO_DATA	*pObj;
	char		 arg1[MSL];
	int			 cou = 0;			/* display counter	*/
	int			 display_limit;		/* display limit	*/

	argument = one_argument( argument, arg1 );
	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "&YSintassi&w:\r\n" );
		send_to_char( ch, "showlayers n  -  display maximum of n lines.\r\n" );
		return;
	}

	display_limit = atoi( arg1 );
	pager_printf( ch, "	  Vnum	  Wear Layer   Description \r\n" );
	for ( pObj = first_object_proto;  pObj;  pObj = pObj->next )
	{
		if ( IS_EMPTY(pObj->layers) )
			continue;

		if ( ++cou <= display_limit )
		{
			pager_printf( ch, "%4d %5d %5d %22s %s\r\n",
				cou,
				pObj->vnum,
				print_bitvector(pObj->layers),
				pObj->short_descr,
				code_bit(NULL, pObj->wear_flags, CODE_OBJWEAR) );
		}
	}
}


/*
 * Sorts the arrays "vnums" and "count" based on the order in "count"
 */
void zero_sort( VNUM *vnums, int *count, int left, int right )
{
	int		i = left,
			j = right,
			test;
	VNUM	swap;

	test = count[(left+right)/2];

	do
	{
		while ( count[i] > test )		i++;
		while ( test > count[j] )		j--;

		if ( i <= j )
		{
			swap = count[i];	count[i] = count[j];	count[j] = swap;
			swap = vnums[i];	vnums[i] = vnums[j];	vnums[j] = swap;
			i++;
			j--;
		}
	} while ( i <= j );

	if ( left < j )		zero_sort( vnums, count, left, j  );
	if ( i < right )	zero_sort( vnums, count, i, right );
}


void diag_visit_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	pager_printf( ch, "***obj=%s\r\n", obj->name );
	if ( obj->first_content )
	{
		diag_visit_obj( ch, obj->first_content );
		if ( obj->next_content )
			diag_visit_obj( ch, obj->next_content );
	}
	else if ( obj->next_content )
		diag_visit_obj( ch, obj->next_content );
}


/*
 * Sort function used by diagnose "rf" to sort integers
 */
int diag_int_comp( const void *i, const void *j )
{
	return *(int*)i - *(int*)j;
}


/*
 * Displays the help screen for the "diagnose" command
 */
void diagnose_help( CHAR_DATA *ch )
{
	send_to_char( ch, "&YSintassi&w:\r\n" );
	send_to_char( ch, "diagnose of n  -  object frequency top n objects\r\n" );
	send_to_char( ch, "diagnose zero  -  count objects with zero weight\r\n" );
	send_to_char( ch, "diagnose zero n - list n objects with zero weight\r\n" );
	send_to_char( ch, "diagnose rf n lo hi - room flag search.\r\n" );
	send_to_char( ch, "   list room vnums between lo and hi that match n.\r\n" );
	send_to_char( ch, "   e.g. diagnose rf 6 901 969 - list all rooms in Olympus\r\n" );
	send_to_char( ch, "	  that are nomob and deathtraps.\r\n" );
	send_to_char( ch, "   e.g. diagnose rf 2 - list all deathtraps.\r\n" );
	send_to_char( ch, "diagnose mrc num race class vnum1 vnum2 - mobs/race/class\r\n" );
	send_to_char( ch, "   display all mobs of a particular race/class combo.\r\n" );
	send_to_char( ch, "   e.g. diagnose mrc 50 0 3 7500 7534 - show 50 human warriors " );
	send_to_char( ch, " in Edo.\r\n" );
}


/*
 * Takes an object vnum and the count of the number of times
 * that object occurs and decides whether or not to include it in the
 * frequency table which contains the "top n" frequently occurring objects.
 */
void diag_ins( OBJ_PROTO_DATA *p, int siz, OBJ_PROTO_DATA **f )
{
	int cou =  0;										/* temporary counter			 */
	int ins = -1;										/* insert pos in dynamic f array */

	if ( !f[siz-1] || p->count>f[siz-1]->count )		/* don't bother looping thru f	  */
	{
		while ( cou < siz && ins < 0 )					/* should this vnum be insertted? */
		{
			if ( !f[cou++] || p->count > f[cou-1]->count )
				ins = cou-1;							/* needs to go into pos "cou"	  */
		}
	}

	if ( ins >= 0 )										/* if vnum occurs more frequently */
	{
		for ( cou = siz-1;  cou > ins;  cou-- )			/* open a slot in the table		  */
			f[cou] = f[cou-1];

		f[ins] = p;										/* insert pointer in empty slot	  */
	}
}


/*
 * The "diagnose" command is designed to be expandable and take different
 * parameters to handle different diagnostic routines.
 */
DO_RET do_diagnose( CHAR_DATA *ch, char *argument )
{
	#define DIAG_MAX_SIZE 1000

	OBJ_PROTO_DATA   *pObj;
	OBJ_PROTO_DATA  **freq;			/* dynamic array of pointers */
	char		arg1[MIL];
	char		arg2[MIL];
	char		arg3[MIL];
	char		arg4[MIL];
	char		arg5[MIL];
	char		arg6[MIL];
	int			num = 20;		/* display lines requested */
	int			cou;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	argument = one_argument( argument, arg5 );
	argument = one_argument( argument, arg6 );

	if ( !VALID_STR(arg1) )					/* empty arg gets help screen */
	{
		diagnose_help(ch);
		return;
	}

	if ( !str_cmp(arg1, "time") )
	{
		struct tm *t = localtime(&current_time);

		pager_printf( ch, "mon=%d day=%d hh=%d mm=%d\r\n",
			t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);

		return;
	}

	if ( !str_cmp(arg1, "rf") )
	{
		#define DIAG_RF_MAX_SIZE 5000

		ROOM_DATA	*pRoom;
		int 		 match;
		int 		 hit_cou;
		VNUM		 lo;
		VNUM		 hi;
		VNUM		 vnum[DIAG_RF_MAX_SIZE];

		if ( !VALID_STR(arg2) )					/* empty arg gets help scrn */
		{
			diagnose_help(ch);
			return;
		}
		else
			match = atoi (arg2);

		hit_cou = 0;					/* number of vnums found */
		lo = (*arg3)  ?  atoi (arg3)  :  1;			/* (TT) era 0 */
		hi = (*arg4)  ?  atoi (arg4)  :  MAX_VNUM;

		send_to_char( ch, "\r\nRoom Vnums\r\n" );
		for ( cou = 0;  cou < MAX_KEY_HASH;  cou++ )
		{
			if ( room_index_hash[cou] )
			{
				for ( pRoom = room_index_hash[cou];  pRoom;  pRoom = pRoom->next )
				{
					if ( pRoom->vnum >= lo && pRoom->vnum <= hi )
					{
						if ( match == pRoom->sector && hit_cou < RGRUB_ST_MAX_SIZE )
							vnum[hit_cou++] = pRoom->vnum;
					}
				}
			}
		}
		qsort( vnum, hit_cou, sizeof(int), diag_int_comp );		/* sort vnums	 */

		for ( cou = 0;  cou < hit_cou;  cou++ )
			ch_printf( ch, "%5d %6d\r\n", cou+1, vnum[cou] );	/* display vnums */

		return;
	} /* chiude l'if */

	if ( !str_cmp(arg1, "of") )
	{
		if ( *arg2 )								/* empty arg gets dft number */
			num = atoi( arg2 );

		if ( num > DIAG_MAX_SIZE  || num < 1 )		/* display num out of bounds */
		{
			diagnose_help(ch);
			return;
		}

		CREATE( freq, OBJ_PROTO_DATA *, num );			/* dynamic freq array		*/

		for ( cou = 0;  cou < num;  cou++ )				/* initialize freq array	*/
			freq[cou] = NULL;							/* to NULL pointers			*/

		/* Cerca in tutti gli oggetti prototipo */
		for ( pObj = first_object_proto;  pObj;  pObj = pObj->next )
			diag_ins( pObj, num, freq );				/* insert pointer into list	*/

		send_to_char( ch, "\r\nObject Frequencies\r\n" );	/* send results to char		*/

		for ( cou = 0;  cou < num && freq[cou];  cou++ )
			ch_printf( ch, "%3d %8d %8d\r\n", cou+1, freq[cou]->vnum, freq[cou]->count );

		DISPOSE( freq );
		return;
	} /* chiude l'if */

	if ( !str_cmp(arg1, "mm") )
	{
		DESCRIPTOR_DATA *d;
		CHAR_DATA *victim;

		if ( !VALID_STR(arg2) )
			return;

		if ( get_trust(ch) < TRUST_IMPLE )
		{
			send_to_char( ch, "Alcune opzioni di utilizzo di questo comando sono irraggiungibili dalla tua fiducia" );
			return;
		}

		victim = get_char_mud( ch, arg2, TRUE );
		if ( !victim )
		{
			send_to_char( ch, "Not here.\r\n" );
			return;
		}

		if ( !victim->desc )
		{
			send_to_char( ch, "No descriptor.\r\n" );
			return;
		}

		if ( victim == ch )
		{
			send_to_char( ch, "Cancelling.\r\n" );
			for ( d = first_descriptor;  d;  d = d->next )
			{
				if ( d->snoop_by == ch->desc )
					d->snoop_by = NULL;
			}
			return;
		}

		if ( victim->desc->snoop_by )
		{
			send_to_char( ch, "Busy.\r\n" );
			return;
		}

		if ( get_trust(victim) >= get_trust(ch) )
		{
			send_to_char( ch, "Busy.\r\n" );
			return;
		}

		victim->desc->snoop_by = ch->desc;
		send_to_char( ch, "Ok.\r\n" );
		return;
	} /* chiude l'if */

	if ( !str_cmp(arg1, "zero") )
	{
#define ZERO_MAX	1500
		VNUM	vnums[ZERO_MAX];
		int		count[ZERO_MAX];
		int		zero_obj_ind =  0;		/* num of obj_ind's with 0 wt */
		int		zero_obj	 =  0;		/* num of objs with 0 wt	  */
		int		zero_num	 = -1;		/* num of lines requested	  */

		if ( *arg2 )
			zero_num = atoi( arg2 );

		for ( pObj = first_object_proto;  pObj;  pObj = pObj->next )
		{
			if ( pObj->weight == 0 )
			{
				zero_obj_ind++;
				zero_obj += pObj->count;
				if ( zero_obj_ind <= ZERO_MAX )
				{
					vnums[zero_obj_ind-1] = pObj->vnum;
					count[zero_obj_ind-1] = pObj->count;
				}
			}
		}

		if ( zero_num > 0 )
		{
			zero_sort( vnums, count, 0, zero_obj_ind - 1 );
			zero_num = UMIN( zero_num, ZERO_MAX );
			zero_num = UMIN( zero_num, zero_obj_ind );

			for ( cou = 0;  cou < zero_num;  cou++ )
				ch_printf( ch, "%6d %6d %6d\r\n", cou+1, vnums[cou], count[cou] );
		}

		ch_printf( ch, "%6d %6d\r\n", zero_obj_ind, zero_obj );
		return;
	} /* chiude l'if */

	if ( !str_cmp(arg1, "visite") || !str_cmp(arg1, "visit") )
	{
		diag_visit_obj( ch, ch->first_carrying );
		return;
	}

	if ( !str_cmp(arg1, "xxxobxxx") )
	{
		OBJ_PROTO_DATA	*px;
		OBJ_DATA		*po;
		OBJ_DATA		*pt = NULL;
		AFFECT_DATA	*pa;
		int			 x = 0;

		pa = NULL;
		ch_printf( ch, "CHAR name=%s \r\n", ch->name);
		ch_printf( ch, "   first_carry=%s\r\n", ch->first_carrying  ?  ch->first_carrying->name  :  "NULL" );
		ch_printf( ch, "   last_carry=%s\r\n", ch->last_carrying  ?  ch->last_carrying->name  :  "NULL" );

/*		for ( pa = ch->first_affect;  pa;  pa=pa->next )
		{
			ch_printf( ch, "   type= %d duration= %d location= %d modifier= %d bitvector= %d\r\n",
				pa->type, pa->duration, pa->location, pa->modifier, pa->bitvector );
		}
*/
		for ( po = first_object;  po;  po = po->next )
		{
			x++;
			pt = NULL;

			if ( !po->carried_by && !po->in_obj )
				continue;

			if ( !po->carried_by )
			{
				pt = po;
				while( pt->in_obj )		/* could be in a container on ground */
					pt=pt->in_obj;
			}

			if ( ch == po->carried_by || (pt && ch == pt->carried_by) )
			{
				px = po->pObjProto;

				ch_printf( ch, "\r\n%d OBJ name = %s \r\n", x, po->name );
				ch_printf( ch, "   next_content = %s\r\n",	po->next_content  ?  po->next_content->name  :  "NULL" );
				ch_printf( ch, "   prev_content = %s\r\n",	po->prev_content  ?  po->prev_content->name  :  "NULL" );
				ch_printf( ch, "   first_content= %s\r\n",	po->first_content ?  po->first_content->name :  "NULL" );
				ch_printf( ch, "   last_content = %s\r\n",	po->last_content  ?  po->last_content->name  :  "NULL" );

/*				ch_printf( ch, "\r\nINDEX_DATA vnum=%d name=%s level=%d extra_flags=%d\r\n",
					px->vnum, px->name, px->level, px->extra_flags );

				ch_printf( ch, "v0=%d v1=%d v2=%d v3=%d v4=%d v5=%d item_type=%d\r\n",
					px->value[0], px->value[1], px->value[2], px->value[3],
					px->value[4], px->value[5], px->type );
*/

/*				for ( pa = px->first_affect;  pa;  pa=pa->next )
				{
					ch_printf( ch, "   type= %d duration= %d location= %d modifier= %d bitvector= %d\r\n",
						pa->type, pa->duration, pa->location, pa->modifier, pa->bitvector );
				}
*/

/*				ch_printf( ch, "\r\nOBJECT_DATA %d name= %s level= %d wear_flags= %d wear_loc= %d\r\n",
					x, po->name, po->level, po->wear_flags, po->wear_loc );
*/

/*				ch_printf(ch, "v0=%d v1=%d v2=%d v3=%d v4=%d v5=%d item_type=%d\r\n",
					po->value[0], po->value[1], po->value[2], po->value[3],
					po->value[4], po->value[5], po->type );
*/

/*				for ( pa = po->first_affect;  pa;  pa = pa->next )
				{
					ch_printf( ch, "   type= %d duration= %d location= %d modifier= %d bitvector= %d\r\n",
						pa->type, pa->duration, pa->location, pa->modifier, pa->bitvector );
				}
*/
			} /* chiude l'if */
		} /* chiude il for */
		return;
	} /* chiude l'if */

	if ( !str_cmp(arg1, "mrc") )
	{
		MOB_PROTO_DATA *pm;
		int				race_arg,
						class_arg,
						dis_num,
						dis_cou = 0;
		VNUM			vnum1,
						vnum2;

		if ( !VALID_STR(arg2) || !isdigit(arg2[0])
		  || !VALID_STR(arg3) || !isdigit(arg3[0])
		  || !VALID_STR(arg4) || !isdigit(arg4[0])
		  || !VALID_STR(arg5) || !isdigit(arg5[0])
		  || !VALID_STR(arg6) || !isdigit(arg6[0]) )
		{
			send_to_char( ch, "Sorry. Invalid format.\r\n\r\n" );
			diagnose_help(ch);
			return;
		}

		dis_num		= UMIN( atoi(arg2), DIAG_MAX_SIZE );
		race_arg	= atoi( arg3 );
		class_arg	= atoi( arg4 );
		vnum1		= atoi( arg5 );
		vnum2		= atoi( arg6 );
/*
		ch_printf( ch, "dis_num = %d race = %d class = %d vnum1 = %d vnum2 = %d\r\n",
			dis_num, race_arg, class_arg, vnum1, vnum2 );
*/
		send_to_char( ch, "\r\n" );

		for ( pm = first_mob_proto;  pm;  pm = pm->next )
		{
			if ( pm->vnum < vnum1 )			continue;
			if ( pm->vnum > vnum2 )			continue;
			if ( pm->race != race_arg )		continue;
			if ( pm->class != class_arg )	continue;

			if ( dis_cou++ < dis_num )
				pager_printf( ch, "%5d %s\r\n", pm->vnum, pm->name );
		}
		return;
	} /* chiude l'if */

	diagnose_help( ch );
}
