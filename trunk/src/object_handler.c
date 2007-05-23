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
 >		Modulo con le funzioni generiche per la gestione degli Oggetti		<
\****************************************************************************/

#include "mud.h"
#include "affect.h"
#include "build.h"
#include "calendar.h"
#include "db.h"
#include "interpret.h"
#include "movement.h"
#include "mprog.h"
#include "object_interact.h"
#include "room.h"
#include "save.h"


/*
 * Variabili
 */
int		  cur_obj;
int		  cur_obj_serial;
bool	  cur_obj_extracted;
obj_ret	  global_objcode;
int		  falling;


/*
 * Tabella parsing sulle flag di oggetto.
 * Extended Bitvector.
 */
const CODE_DATA code_objflag[] =
{
	{ OBJFLAG_GLOW,			"OBJFLAG_GLOW",			"glow"			},
	{ OBJFLAG_HUM,			"OBJFLAG_HUM",			"hum"			},
	{ OBJFLAG_DARK,			"OBJFLAG_DARK",			"dark"			},
	{ OBJFLAG_LOYAL,		"OBJFLAG_LOYAL",		"loyal"			},
	{ OBJFLAG_EVIL,			"OBJFLAG_EVIL",			"evil"			},
	{ OBJFLAG_INVIS,		"OBJFLAG_INVIS",		"invis"			},
	{ OBJFLAG_MAGIC,		"OBJFLAG_MAGIC",		"magic"			},
	{ OBJFLAG_NODROP,		"OBJFLAG_NODROP",		"nodrop"		},
	{ OBJFLAG_BLESS,		"OBJFLAG_BLESS",		"bless"			},
	{ OBJFLAG_ANTI_GOOD,	"OBJFLAG_ANTI_GOOD",	"antigood"		},
	{ OBJFLAG_ANTI_EVIL,	"OBJFLAG_ANTI_EVIL",	"antievil"		},
	{ OBJFLAG_ANTI_NEUTRAL,	"OBJFLAG_ANTI_NEUTRAL",	"antineutral"	},
	{ OBJFLAG_NOREMOVE,		"OBJFLAG_NOREMOVE",		"noremove"		},
	{ OBJFLAG_INVENTORY,	"OBJFLAG_INVENTORY",	"inventory"		},
	{ OBJFLAG_ANTI_MAGE,	"OBJFLAG_ANTI_MAGE",	"antimage"		},
	{ OBJFLAG_ANTI_THIEF,	"OBJFLAG_ANTI_THIEF",	"antithief"		},
	{ OBJFLAG_ANTI_WARRIOR,	"OBJFLAG_ANTI_WARRIOR",	"antiwarrior"	},
	{ OBJFLAG_ANTI_CLERIC,	"OBJFLAG_ANTI_CLERIC",	"anticleric"	},
	{ OBJFLAG_ORGANIC,		"OBJFLAG_ORGANIC",		"organic"		},
	{ OBJFLAG_METAL,		"OBJFLAG_METAL",		"metal"			},
	{ OBJFLAG_DONATION,		"OBJFLAG_DONATION",		"donation"		},
	{ OBJFLAG_CLANOBJECT,	"OBJFLAG_CLANOBJECT",	"clanobject"	},
	{ OBJFLAG_CLANCORPSE,	"OBJFLAG_CLANCORPSE",	"clancorpse"	},
	{ OBJFLAG_ANTI_VAMPIRE,	"OBJFLAG_ANTI_VAMPIRE",	"antivampire"	},
	{ OBJFLAG_ANTI_DRUID,	"OBJFLAG_ANTI_DRUID",	"antidruid"		},
	{ OBJFLAG_HIDDEN,		"OBJFLAG_HIDDEN",		"hidden"		},
	{ OBJFLAG_POISONED,		"OBJFLAG_POISONED",		"poisoned"		},
	{ OBJFLAG_COVERING,		"OBJFLAG_COVERING",		"covering"		},
	{ OBJFLAG_DEATHROT,		"OBJFLAG_DEATHROT",		"deathrot"		},
	{ OBJFLAG_BURIED,		"OBJFLAG_BURIED",		"buried"		},
	{ OBJFLAG_SECRET,		"OBJFLAG_SECRET",		"secret"		},
	{ OBJFLAG_NOLOCATE,		"OBJFLAG_NOLOCATE",		"nolocate"		},
	{ OBJFLAG_GROUNDROT,	"OBJFLAG_GROUNDROT",	"groundrot"		},
	{ OBJFLAG_LOOTABLE,		"OBJFLAG_LOOTABLE",		"lootable"		},
	{ OBJFLAG_SMUGGLE,		"OBJFLAG_SMUGGLE",		"smuggle"		},
	{ OBJFLAG_GOOD,			"OBJFLAG_GOOD",			"good"			},
	{ OBJFLAG_NEUTRAL,		"OBJFLAG_NEUTRAL",		"neutral"		}
};
const int max_check_objflag = sizeof(code_objflag)/sizeof(CODE_DATA);


/*
 * Tabella codici sulle flag magiche degli oggetti
 */
const CODE_DATA code_objmagic[] =
{
	{ OBJMAGIC_RETURNING,		"OBJMAGIC_RETURNING",		"returning"			},
	{ OBJMAGIC_BACKSTABBER,		"OBJMAGIC_BACKSTABBER",		"backstabber"		},
	{ OBJMAGIC_BANE,			"OBJMAGIC_BANE",			"bane"				},
	{ OBJMAGIC_MAGIC_LOYAL,		"OBJMAGIC_MAGIC_LOYAL",		"loyal"				},
	{ OBJMAGIC_HASTE,			"OBJMAGIC_HASTE",			"haste"				},
	{ OBJMAGIC_DRAIN,			"OBJMAGIC_DRAIN",			"drain"				},
	{ OBJMAGIC_LIGHTNING_BLADE,	"OBJMAGIC_LIGHTNING_BLADE",	"lightning_blade"	},
	{ OBJMAGIC_PKDISARMED,		"OBJMAGIC_PKDISARMED",		"pkdisarmed"		}
};
const int max_check_objmagic = sizeof(code_objmagic)/sizeof(CODE_DATA);


/*
 * Tabella codici sulle tipologia wear degli oggetti
 * (FF) (TR) I campi name vengono utilizzati nella wear_wield_hold() per una code_num()
 *	quindi sarebbero da tradurre
 */
const CODE_DATA code_objwear[] =
{
	{ OBJWEAR_TAKE,		"OBJWEAR_TAKE",		"take"		},
	{ OBJWEAR_FINGER,	"OBJWEAR_FINGER",	"finger"	},
	{ OBJWEAR_NECK,		"OBJWEAR_NECK",		"neck"		},
	{ OBJWEAR_BODY,		"OBJWEAR_BODY",		"body"		},
	{ OBJWEAR_HEAD,		"OBJWEAR_HEAD",		"head"		},
	{ OBJWEAR_LEGS,		"OBJWEAR_LEGS",		"legs"		},
	{ OBJWEAR_FEET,		"OBJWEAR_FEET",		"feet"		},
	{ OBJWEAR_HANDS,	"OBJWEAR_HANDS",	"hands"		},
	{ OBJWEAR_ARMS,		"OBJWEAR_ARMS",		"arms"		},
	{ OBJWEAR_SHIELD,	"OBJWEAR_SHIELD",	"shield"	},
	{ OBJWEAR_ABOUT,	"OBJWEAR_ABOUT",	"about"		},
	{ OBJWEAR_WAIST,	"OBJWEAR_WAIST",	"waist"		},
	{ OBJWEAR_WRIST,	"OBJWEAR_WRIST",	"wrist"		},
	{ OBJWEAR_WIELD,	"OBJWEAR_WIELD",	"wield"		},
	{ OBJWEAR_HOLD,		"OBJWEAR_HOLD",		"hold"		},
	{ OBJWEAR_DUAL,		"OBJWEAR_DUAL",		"_dual_"	},	/* Probabilmente scritto con gli underscore per non essere digitato dal player */
	{ OBJWEAR_EARS,		"OBJWEAR_EARS",		"ears"		},
	{ OBJWEAR_EYES,		"OBJWEAR_EYES",		"eyes"		},
	{ OBJWEAR_MISSILE,	"OBJWEAR_MISSILE",	"missile"	},
	{ OBJWEAR_BACK,		"OBJWEAR_BACK",		"back"		},
	{ OBJWEAR_FACE,		"OBJWEAR_FACE",		"face"		},
	{ OBJWEAR_ANKLE,	"OBJWEAR_ANKLE",	"ankle"		},
	{ OBJWEAR_FLOAT,	"OBJWEAR_FLOAT",	"float"		}
};
const int max_check_objwear = sizeof(code_objwear)/sizeof(CODE_DATA);


/*
 * Tabella codici delle locazioni wear per gli oggetti
 * Questa tabella viene utilizzata in mob_prog.c, è una versione che serve
 *	a trovare la posizione wear passandogli il nome della locazione wear dell'oggetto.
 */
const CODE_DATA code_wearflag[] =
{
	{ -1,	"",	"take"		},
	{ -1,	"",	"finger"	},
	{ -1,	"",	"neck"		},
	{ -1,	"",	"neck"		},
	{ -1,	"",	"neck"		},
	{ -1,	"",	"body"		},
	{ -1,	"",	"head"		},
	{ -1,	"",	"legs"		},
	{ -1,	"",	"feet"		},
	{ -1,	"",	"hands"		},
	{ -1,	"",	"arms"		},
	{ -1,	"",	"shield"	},
	{ -1,	"",	"about"		},
	{ -1,	"",	"waist"		},
	{ -1,	"",	"wrist"		},
	{ -1,	"",	"wrist"		},
	{ -1,	"",	"wield"		},
	{ -1,	"",	"hold"		},
	{ -1,	"",	"dual"		},
	{ -1,	"",	"ears"		},
	{ -1,	"",	"eyes"		},
	{ -1,	"",	"missile"	},
	{ -1,	"",	"back"		},
	{ -1,	"",	"face"		},
	{ -1,	"",	"ankle"		},
	{ -1,	"",	"ankle"		},
	{ -1,	"",	"float"		}
};
const int max_check_wearflag = sizeof(code_wearflag)/sizeof(CODE_DATA);


const char *wear_name[/*MAX_WEARLOC*/] =
{
	"<come luce>         ",
	"<al dito sx>        ",
	"<al dito dx>        ",
	"<al collo>          ",
	"<al collo>          ",
	"<sul corpo>         ",
	"<sulla testa>       ",
	"<sulle gambe>       ",
	"<ai piedi>          ",
	"<alle mani>         ",
	"<sulle braccia>     ",
	"<come scudo>        ",
	"<intorno al corpo>  ",
	"<alla vita>         ",
	"<al polso sx>       ",
	"<al polso dx>       ",
	"<impugnato>         ",
	"<tenuto>            ",
	"<come seconda arma> ",
	"<sulle orecchie>    ",
	"<sugli occhi>       ",
	"<come proiettile>   ",
	"<sulle spalle>      ",
	"<sul viso>          ",
	"<alla caviglia sx>  ",
	"<alla caviglia dx>  ",
	"<fluttua accanto>   "
};


/*
 * If possible group obj2 into obj1
 * This code, along with clone_object, obj->count, and special support
 *  for it implemented throughout handler.c and save.c should show improved
 *  performance on MUDs with players that hoard tons of potions and scrolls
 *  as this will allow them to be grouped together both in memory, and in
 *  the player files.
 */
OBJ_DATA *group_object( OBJ_DATA *obj1, OBJ_DATA *obj2 )
{
	EXTRA_DESCR_DATA	*ed1;
	EXTRA_DESCR_DATA	*ed2;
	AFFECT_DATA			*paf1;
	AFFECT_DATA			*paf2;
	int					 x;
	
	if ( !obj1 || !obj2 )
		return NULL;

	if ( obj1 == obj2 )
		return obj1;

	/* Niente check sui mobprog */

	if ( obj1->pObjProto	!= obj2->pObjProto
	  || str_cmp(obj1->name,		obj2->name)
	  || str_cmp(obj1->short_descr, obj2->short_descr)
	  || str_cmp(obj1->long_descr,	obj2->long_descr)
	  || str_cmp(obj1->action_descr, obj2->action_descr)
	  || obj1->type			!= obj2->type
	  || !SAME_BITS(obj1->extra_flags, obj2->extra_flags)
	  || !SAME_BITS(obj1->magic_flags, obj2->magic_flags)
	  || !SAME_BITS(obj1->wear_flags, obj2->wear_flags)
	  || obj1->wear_loc		!= obj2->wear_loc
	  || obj1->weight		!= obj2->weight
	  || obj1->cost			!= obj2->cost
	  || obj1->level		!= obj2->level
	  || obj1->timer		!= obj2->timer
/*	  || obj1->first_mudprog		!=  obj2->first_mudprog*/	/* (TT) */
	  || obj1->first_content
	  || obj2->first_content )
	{
		return NULL;
	}

	for ( x = 0;  x < MAX_OBJTYPE;  x++ )
	{
		if ( !compare_values(obj1, obj2, x) )
			return NULL;
	}

	/* Controlla che le extra siano uguali */
	for ( ed1 = obj1->first_extradescr, ed2 = obj1->first_extradescr;  ;  ed1 = ed1->next, ed2 = ed2->next )
	{
		if (  ed1 && !ed2 )		return NULL;
		if ( !ed1 &&  ed2 )		return NULL;
		if ( !ed1 && !ed2 )		break;		/* oggetti con extra uguale, ferma il ciclo */

		/* Niente check sulle keyword, solo sulle descrizioni */
		if ( str_cmp(ed1->description, ed2->description) )
			return NULL;
	}

	/* Controlla che gli affect siano uguali */
	for ( paf1 = obj1->first_affect, paf2 = obj2->first_affect;  ;  paf1 = paf1->next, paf2 = paf2->next )
	{
		if (  paf1 && !paf2 )	return NULL;
		if ( !paf1 &&  paf2 )	return NULL;
		if ( !paf1 && !paf2 )	break;		/* oggetti con affect uguale, ferma il ciclo */

		/* Non controlla l'uguaglianza della durata dell'affect */
		if ( !is_same_affect(paf1, paf2, FALSE) )
			return NULL;
	}

	if ( obj1->count + obj2->count > 1 )	/* prevent count overflow */
	{
		obj1->count				+= obj2->count;
		obj1->pObjProto->count	+= obj2->count;	/* to be decremented in */
		objects_loaded			+= obj2->count;	/* free_object */
		free_object( obj2 );
		return obj1;
	}

	return obj2;
}


/*
 * Copia tutti i campi dall'oggetto sorgente a quello di destinazione
 */
void copy_object( OBJ_DATA *source, OBJ_DATA *dest )
{
	AFFECT_DATA			*paf;
	AFFECT_DATA			*paf_dest = NULL;
	EXTRA_DESCR_DATA	*ed;
	EXTRA_DESCR_DATA	*ed_dest = NULL;
	MPROG_DATA			*mprog;
	MPROG_DATA			*mprog_dest = NULL;

	/* Ripulisce la destinazione in caso abbia dei valori */
	clean_object( dest );

	dest->prototype		= source->prototype;

	dest->vnum			= source->vnum;
	dest->area			= source->area;
	dest->count			= 1;

	if ( source->prototype )
		dest->pObjProto		= source;
	else
		dest->pObjProto		= source->pObjProto;

	dest->name			= str_dup( source->name );
	dest->short_descr	= str_dup( source->short_descr );
	dest->long_descr	= str_dup( source->long_descr );
	dest->action_descr	= str_dup( source->action_descr );
	dest->owner			= str_dup( source->owner );
	dest->level			= source->level;
	dest->type			= source->type;
	SET_BITS( dest->extra_flags, source->extra_flags );
	SET_BITS( dest->magic_flags, source->magic_flags );
	SET_BITS( dest->wear_flags, source->wear_flags );
	dest->wear_loc		= -1;
	dest->weight		= source->weight;
	dest->cost			= source->cost;
	dest->condition		= source->condition;
	dest->timer			= source->timer;
	SET_BITS( dest->layers, source->layers );
	dest->serial		= 0;

	/* Copia i value */
	copy_values( source, dest );

	/* Copia gli affect */
	CREATE( paf_dest, AFFECT_DATA, 1 );
	for ( paf = source->first_affect;  paf;  paf = paf->next )
	{
		paf_dest = copy_affect( paf );
		LINK( paf_dest, dest->first_affect, dest->last_affect, next, prev );
		top_affect++;

		if ( paf->next )
		{
			CREATE( paf_dest->next, AFFECT_DATA, 1 );
			paf_dest = paf_dest->next;
		}
	}

	/* Copia le extra decrizioni */
	CREATE( ed_dest, EXTRA_DESCR_DATA, 1 );
	for ( ed = source->first_extradescr;  ed;  ed = ed->next )
	{
		ed_dest->keyword		= str_dup( ed->keyword );
		ed_dest->description	= str_dup( ed->description );
		LINK( ed_dest, dest->first_extradescr, dest->last_extradescr, next, prev );
		top_ed++;

		if ( ed->next )
		{
			CREATE( ed_dest->next, EXTRA_DESCR_DATA, 1 );
			ed_dest = ed_dest->next;
		}
	}

	/* Copia i mudprog */
	CREATE( mprog_dest, MPROG_DATA, 1 );
	for ( mprog = source->first_mudprog;  mprog;  mprog = mprog->next )
	{
		mprog_dest = copy_mudprog( mprog );
		LINK( mprog_dest, dest->first_mudprog, dest->last_mudprog, next, prev );

		if ( mprog->next )
		{
			CREATE( mprog_dest->next, MPROG_DATA, 1 );
			mprog_dest = mprog_dest->next;
		}
	}
	dest->mpact = NULL;
	dest->mpactnum = 0;
}


/*
 * Make a simple clone of an object non prototype
 */
OBJ_DATA *clone_object( OBJ_DATA *obj )
{
	OBJ_DATA *clone;

	if ( obj->prototype )
	{
		send_log( NULL, LOG_BUG, "clone_object: l'oggetto passato #%d è un prototipo", obj->vnum );
		return NULL;
	}

	CREATE( clone, OBJ_DATA, 1 );

	copy_object( obj, clone );

	DISPOSE( obj->owner );	/* pulisce il campo di proprietà per il nuovo oggetto */
	clone->count = 1;

	obj->pObjProto->count++;
	objects_loaded++;
	objects_physical++;

	cur_obj_serial = UMAX( (cur_obj_serial + 1 ) & ((1<<30)-1), 1 );
	clone->serial  = clone->pObjProto->serial = cur_obj_serial;

	LINK( clone, first_object, last_object, next, prev );

	return clone;
}


/*
 * Split off a grouped object
 * decreased obj's count to num, and creates a new object containing the rest
 * Ritorna l'oggetto rimanente
 */
OBJ_DATA *split_obj( OBJ_DATA *obj, int num )
{
	OBJ_DATA *rest;
	int		  count = obj->count;

	if ( count <= num || num == 0 )
		return NULL;

	rest = clone_object( obj );
	--obj->pObjProto->count;		/* since clone_object() ups this value */
	--objects_loaded;
	rest->count = obj->count - num;
	obj->count = num;

	if ( obj->carried_by )
	{
		LINK( rest, obj->carried_by->first_carrying, obj->carried_by->last_carrying, next_content, prev_content );

		rest->carried_by	 	= obj->carried_by;
		rest->in_room	 		= NULL;
		rest->in_obj	 		= NULL;
	}
	else if ( obj->in_room )
	{
		LINK( rest, obj->in_room->first_content, obj->in_room->last_content, next_content, prev_content );

		rest->carried_by	 	= NULL;
		rest->in_room	 		= obj->in_room;
		rest->in_obj	 		= NULL;
	}
	else if ( obj->in_obj )
	{
		LINK( rest, obj->in_obj->first_content, obj->in_obj->last_content, next_content, prev_content );

		rest->in_obj			 = obj->in_obj;
		rest->in_room			 = NULL;
		rest->carried_by		 = NULL;
	}

	return rest;
}


/*
 * Return real weight of an object, including weight of contents.
 */
int get_real_obj_weight( OBJ_DATA *obj )
{
	int weight;

	weight = obj->count * obj->weight;

	for ( obj = obj->first_content;  obj;  obj = obj->next_content )
		weight += get_real_obj_weight( obj );

	return weight;
}


/*
 * Empty an obj's contents... optionally into another obj, or a room
 */
bool empty_obj( OBJ_DATA *obj, OBJ_DATA *destobj, ROOM_DATA *destroom )
{
	OBJ_DATA  *otmp,
			  *otmp_next;
	CHAR_DATA *ch = obj->carried_by;
	bool	   movedsome = FALSE;

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "empty_obj: NULL obj" );
		return FALSE;
	}

	if ( destobj || (!destroom && !ch && (destobj = obj->in_obj) != NULL) )
	{
		for ( otmp = obj->first_content;  otmp;  otmp = otmp_next )
		{
			otmp_next = otmp->next_content;
			/* only keys on a keyring */
			if ( destobj->type == OBJTYPE_KEYRING && otmp->type != OBJTYPE_KEY )
				continue;

			if ( destobj->type == OBJTYPE_QUIVER && otmp->type != OBJTYPE_PROJECTILE )
				continue;

			if ( (destobj->type == OBJTYPE_CONTAINER || destobj->type == OBJTYPE_KEYRING
			  || destobj->type == OBJTYPE_QUIVER)
			  && get_real_obj_weight(otmp) + get_real_obj_weight(destobj)
			  > destobj->container->capacity )
			{
				continue;
			}
			obj_from_obj( otmp );
			obj_to_obj( otmp, destobj );
			movedsome = TRUE;
		}
		return movedsome;
	}

	if ( destroom || (!ch && (destroom = obj->in_room) != NULL) )
	{
		for ( otmp = obj->first_content; otmp; otmp = otmp_next )
		{
			otmp_next = otmp->next_content;
			if ( ch && has_trigger(otmp->first_mudprog, MPTRIGGER_DROP) && otmp->count > 1 )
			{
				split_obj( otmp, 1 );
				obj_from_obj( otmp );
				if ( !otmp_next )
					otmp_next = obj->first_content;
			}
			else
			{
				obj_from_obj( otmp );
			}

			otmp = obj_to_room( otmp, destroom );
			if ( ch )
			{
				oprog_drop_trigger( ch, otmp );		/* mudprogs */
				if ( char_died(ch) )
					ch = NULL;
			}
			movedsome = TRUE;
		}
		return movedsome;
	}

	if ( ch )
	{
		for ( otmp = obj->first_content; otmp; otmp = otmp_next )
		{
			otmp_next = otmp->next_content;
			obj_from_obj( otmp );
			obj_to_char( otmp, ch );
			movedsome = TRUE;
		}
		return movedsome;
	}

	send_log( NULL, LOG_BUG, "empty_obj: could not determine a destination for vnum %d", obj->vnum );
	return FALSE;
}


/*
 * Give an obj to a char.
 */
OBJ_DATA *obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
	OBJ_DATA	*otmp;
	OBJ_DATA	*oret = obj;
	BITVECTOR	*extra_flags = NULL;
	int			 oweight = get_obj_weight( obj );
	int			 onum = obj->count;
	int			 wear_loc = obj->wear_loc;
	bool		 skipgroup;
	bool		 grouped;

	SET_BITS( extra_flags, obj->extra_flags );

	skipgroup = FALSE;
	grouped	  = FALSE;

	/* Vari check per i multy per il cambio di proprietà */
	if ( IS_PG(ch)
	  && (!VALID_STR(obj->owner)
	  ||  (VALID_STR(obj->owner) && str_cmp(obj->owner, ch->name))) )
	{
		if ( VALID_STR(obj->owner) && ch->desc && VALID_STR(ch->desc->host) )
		{
			CHAR_DATA	*mch = NULL;

			/* Cerca un eventuale multy tra i pg online */
			mch = get_player_mud( ch, obj->owner, FALSE );

			/* Se non lo ha trovato lo cerca in quello degli offline */
			if ( !mch )
				mch = get_offline( obj->owner, TRUE );

			/* Se ha trovato che il proprietario precedente ha lo stesso ip del pg
			 *	che sta prendendo l'oggetto avvisa anche se passa l'oggetto */
			if  ( mch && !IS_ADMIN(ch) )	/* da admin a pg avvisa, così da notare gli admin che esagerano o fanno i furbi */
			{
				if ( mch->desc && ch->desc
				  && ((VALID_STR(mch->desc->host)	  && !str_cmp(mch->desc->host, ch->desc->host))
				  ||  (VALID_STR(mch->desc->host_old) && !str_cmp(mch->desc->host_old, ch->desc->host))) )
				{
					send_log( NULL, LOG_WARNING, "obj_to_char: possibile multy %s riceve l'oggetto %s #%d avuto dal pg %s",
						ch->name, obj->short_descr, obj->vnum, mch->name );
				}
			}
		}

		DISPOSE( obj->owner );
		obj->owner = str_dup( ch->name );
	}

	if ( loading_char == ch )
	{
		int x;
		int y;

		for ( x = 0;  x < MAX_WEARLOC;  x++ )
		{
			for ( y = 0;  y < MAX_LAYERS;  y++ )
			{
				if ( save_equipment[x][y] == obj )
				{
					skipgroup = TRUE;
					break;
				}
			}
		}
	}

	if ( IS_MOB(ch) && ch->pIndexData->pShop )
		skipgroup = TRUE;

	if ( !skipgroup )
	{
		for ( otmp = ch->first_carrying;  otmp;  otmp = otmp->next_content )
		{
			if ( (oret = group_object(otmp, obj)) == otmp )
			{
				grouped = TRUE;
				break;
			}
		}
	}

	if ( !grouped )
	{
		if ( IS_PG(ch) || !ch->pIndexData->pShop )
		{
			LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

			obj->carried_by	= ch;
			obj->in_room	= NULL;
			obj->in_obj		= NULL;
		}
		else
		{
			/* If ch is a shopkeeper, add the obj using an insert sort */
			for ( otmp = ch->first_carrying;  otmp;  otmp = otmp->next_content )
			{
				if ( obj->level > otmp->level )
				{
					INSERT( obj, otmp, ch->first_carrying, next_content, prev_content );
					break;
				}
				else if ( obj->level == otmp->level && strcmp(obj->short_descr, otmp->short_descr) < 0 )
				{
					INSERT( obj, otmp, ch->first_carrying, next_content, prev_content );
					break;
				}
			}

			if ( !otmp )
				LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

			obj->carried_by = ch;
			obj->in_room	= NULL;
			obj->in_obj		= NULL;
		}
	} /* chiude l'if */

	if ( wear_loc == WEARLOC_NONE )
	{
		ch->carry_number	+= onum;	/* Conta solo il numero di quelli in inventario */
		ch->carry_weight	+= oweight;
	}
	else
	{
		/* Conta solo il peso di quelli non magici per gli indossati */
		if ( !HAS_BIT(extra_flags, OBJFLAG_MAGIC) )
			ch->carry_weight	+= oweight;
	}

	destroybv( extra_flags );

	return (oret)  ?  oret  :  obj;
}


/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
	CHAR_DATA *ch;

	ch = obj->carried_by;
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "obj_from_char: ch è NULL" );
		return;
	}

	if ( obj->wear_loc != WEARLOC_NONE )
		unequip_char( ch, obj );

	/* obj may drop during unequip... */
	if ( !obj->carried_by )
		return;

	UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_COVERING) && obj->first_content )
		empty_obj( obj, NULL, NULL );

	obj->in_room	  = NULL;
	obj->carried_by	  = NULL;

	ch->carry_number -= obj->count;
	ch->carry_weight -= get_obj_weight( obj );
}


/*
 * Find the ac value of an obj, including position effect
 */
int apply_ac( OBJ_DATA *obj, int iWear )
{
	if ( obj->type != OBJTYPE_ARMOR && obj->type != OBJTYPE_DRESS )
		return 0;

	switch ( iWear )
	{
	  case WEARLOC_BODY:		return	3 * obj->armor->ac;
	  case WEARLOC_HEAD:		return	2 * obj->armor->ac;
	  case WEARLOC_LEGS:		return	2 * obj->armor->ac;
	  case WEARLOC_FEET:		return		obj->armor->ac;
	  case WEARLOC_HANDS:		return		obj->armor->ac;
	  case WEARLOC_ARMS:		return		obj->armor->ac;
	  case WEARLOC_SHIELD:		return		obj->armor->ac;
	  case WEARLOC_FINGER_L:	return		obj->armor->ac;
	  case WEARLOC_FINGER_R:	return		obj->armor->ac;
	  case WEARLOC_NECK:		return		obj->armor->ac;
	  case WEARLOC_NECK_2:		return		obj->armor->ac;	/* (CC) da togliere */
	  case WEARLOC_ABOUT:		return	2 * obj->armor->ac;
	  case WEARLOC_WAIST:		return		obj->armor->ac;
	  case WEARLOC_WRIST_L:	return		obj->armor->ac;
	  case WEARLOC_WRIST_R:	return		obj->armor->ac;
	  case WEARLOC_HOLD:		return		obj->armor->ac;
	  case WEARLOC_EYES:		return		obj->armor->ac;
	  case WEARLOC_FACE:		return		obj->armor->ac;
	  case WEARLOC_BACK:		return		obj->armor->ac;
	  case WEARLOC_ANKLE_L:	return		obj->armor->ac;
	  case WEARLOC_ANKLE_R:	return		obj->armor->ac;
	  case WEARLOC_FLOAT:		return		obj->armor->ac / 2;
	}

	return 0;
}


/*
 * Find a piece of eq on a character.
 * Will pick the top layer if clothing is layered.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
	OBJ_DATA *obj;
	OBJ_DATA *maxobj = NULL;

	for ( obj = ch->first_carrying;  obj;  obj = obj->next_content )
	{
		if ( obj->wear_loc == iWear )
		{
			if ( IS_EMPTY(obj->layers) )
				return obj;
			else if ( !maxobj || obj->layers > maxobj->layers )	/* (BB) qui sarebbe da rifare con qualcos'altro visto che è BITVECTOR, però non ci sto manco visto che i layers come sono ora verranno rimpiazzati */
				maxobj = obj;
		}
	}

	return maxobj;
}


/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
	AFFECT_DATA	*paf;
	OBJ_DATA	*otmp;

	otmp = get_eq_char( ch, iWear );
	if ( otmp && (IS_EMPTY(otmp->layers) || IS_EMPTY(obj->layers)) )
	{
		send_log( NULL, LOG_BUG, "equip_char: con %s, ma già equipaggiato in %s con %s",
			obj->short_descr,
			code_name(NULL, iWear, CODE_WEARLOC),
			otmp->short_descr );
		return;
	}

	if ( obj->carried_by != ch )
	{
		send_log( NULL, LOG_BUG, "equip_char: obj not being carried by ch!" );
		return;
	}

	split_obj( obj, 1 );		/* just in case */
	if ( (HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_EVIL) && is_align(ch, ALIGN_EVIL))
	  || (HAS_BIT(obj->extra_flags, OBJFLAG_ANTI_GOOD) && is_align(ch, ALIGN_GOOD)) )
	{
		if ( loading_char != ch )
		{
			act( AT_MAGIC, "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
			act( AT_MAGIC, "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
		}

		if ( obj->carried_by )
			obj_from_char( obj );

		obj_to_room( obj, ch->in_room );
		oprog_zap_trigger( ch, obj);

		if ( !char_died(ch) )
			save_player( ch );

		return;
	}

	ch->armor				-= apply_ac( obj, iWear );
	obj->wear_loc			 = iWear;

	ch->carry_number		-= obj->count;
	/* Se l'oggetto è magico indossandolo non ne si sente il peso */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
		ch->carry_weight	-= get_obj_weight( obj );

	for ( paf = obj->first_affect;  paf;  paf = paf->next )
		affect_modify( ch, paf, TRUE );

	if ( obj->light && obj->light->hours != 0
	  && ch->in_room && ch->in_room->light < 100 )
	{
		++ch->in_room->light;
	}
}


/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
	AFFECT_DATA *paf;

	if ( obj->wear_loc == WEARLOC_NONE )
	{
		send_log( NULL, LOG_BUG, "unequip_char: already unequipped." );
		return;
	}

	ch->carry_number		+= obj->count;
	/* Se l'oggetto è megico togliendoselo da adosso si ritorna a sentirne il peso */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
		ch->carry_weight	+= get_obj_weight( obj );

	ch->armor				+= apply_ac( obj, obj->wear_loc );
	obj->wear_loc			 = -1;

	if ( obj->carried_by )
	{
		for ( paf = obj->first_affect;  paf;  paf = paf->next )
			affect_modify( ch, paf, FALSE );
	}

	update_aris( ch );

	if ( obj->carried_by == NULL )
		return;

	if ( obj->light && obj->light->hours != 0
	  && ch->in_room && ch->in_room->light > 0 )
	{
		--ch->in_room->light;
	}
}


/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_PROTO_DATA *pObjProto, OBJ_DATA *list )
{
	OBJ_DATA *obj;
	int		  nMatch = 0;

	for ( obj = list;  obj;  obj = obj->next_content )
	{
		if ( obj->vnum == pObjProto->vnum )
			nMatch++;
	}

	return nMatch;
}


/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
	ROOM_DATA	*in_room;
	AFFECT_DATA	*paf;

	if ( (in_room = obj->in_room) == NULL )
	{
		send_log( NULL, LOG_BUG, "obj_from_room: NULL." );
		return;
	}

	for ( paf = obj->first_affect;  paf;  paf = paf->next )
		room_affect( in_room, paf, FALSE );

	UNLINK( obj, in_room->first_content, in_room->last_content, next_content, prev_content );

	/* uncover contents */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_COVERING) && obj->first_content )
		empty_obj( obj, NULL, obj->in_room );

	if ( obj->type == OBJTYPE_FIRE )
		obj->in_room->light -= obj->count;

	obj->carried_by   = NULL;
	obj->in_obj	      = NULL;
	obj->in_room      = NULL;

	if ( obj->vnum == VNUM_OBJ_CORPSE_PC && falling < 1 )
	{
		char	name[MIL];

		one_argument( obj->name, name );
		write_corpses( NULL, name, obj );
	}
}


/*
 * Move an obj into a room.
 */
OBJ_DATA *obj_to_room( OBJ_DATA *obj, ROOM_DATA *pRoom )
{
	OBJ_DATA	*otmp;
	OBJ_DATA	*oret;
	AFFECT_DATA *paf;
	int			 count	   = obj->count;
	int			 item_type = obj->type;

	for ( paf = obj->first_affect;  paf;  paf = paf->next )
		room_affect( pRoom, paf, TRUE );

	for ( otmp = pRoom->first_content;  otmp;  otmp = otmp->next_content )
	{
		oret = group_object( otmp, obj );
		if ( oret == otmp )
		{
			if ( item_type == OBJTYPE_FIRE )
				pRoom->light += count;
			return oret;
		}
	}

	LINK( obj, pRoom->first_content, pRoom->last_content, next_content, prev_content );
	obj->in_room	= pRoom;
	obj->carried_by	= NULL;
	obj->in_obj		= NULL;

	if ( item_type == OBJTYPE_FIRE )
		pRoom->light += count;

	falling++;
	obj_fall( obj, FALSE );
	falling--;

	if ( obj->vnum == VNUM_OBJ_CORPSE_PC && falling < 1 )
	{
		char	name[MIL];

		one_argument( obj->name, name );
		write_corpses( NULL, name, NULL );
	}

	return obj;
}


/*
 * Who's carrying an item -- recursive for nested objects
 */
CHAR_DATA *carried_by( OBJ_DATA *obj )
{
	if ( obj->in_obj )
		return carried_by( obj->in_obj );

	return obj->carried_by;
}


/*
 * Return TRUE if an object is, or nested inside a magic container
 */
bool in_magic_container( OBJ_DATA *obj )
{
	if ( obj->type == OBJTYPE_CONTAINER && HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
		return TRUE;

	if ( obj->in_obj )
		return in_magic_container(obj->in_obj);

	return FALSE;
}


/*
 * Move an object into an object.
 */
OBJ_DATA *obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
	OBJ_DATA	*otmp;
	OBJ_DATA	*oret;
	CHAR_DATA	*who;

	if ( obj == obj_to )
	{
		send_log( NULL, LOG_BUG, "obj_to_obj: trying to put object inside itself: vnum %d", obj->vnum );
		return obj;
	}

	if ( !obj_to->container )
	{
		send_log( NULL, LOG_BUG, "obj_to_obj: l'oggetto #%d di destinazione non è un contenitore, non potrà contenere l'oggetto %d. (char: %s)",
			obj_to->vnum, obj->vnum, obj_to->carried_by  ?  IS_PG(obj_to->carried_by) ? obj_to->carried_by->name : obj_to->carried_by->short_descr  :  "" );
		return obj;
	}

	who = carried_by( obj_to );
	if ( who && !in_magic_container(obj_to) )
		who->carry_weight += get_obj_weight( obj );

	for ( otmp = obj_to->first_content;  otmp;  otmp = otmp->next_content )
	{
		oret = group_object( otmp, obj );
		if ( oret == otmp )
			return oret;
	}

	LINK( obj, obj_to->first_content, obj_to->last_content, next_content, prev_content );

	obj->in_obj		= obj_to;
	obj->in_room	= NULL;
	obj->carried_by	= NULL;

	return obj;
}


/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
	OBJ_DATA *obj_from;
	bool	  magic;

	if ( (obj_from = obj->in_obj) == NULL )
	{
		send_log( NULL, LOG_BUG, "obj_from_obj: obj_from è NULL" );
		return;
	}

	magic = in_magic_container( obj_from );

	UNLINK( obj, obj_from->first_content, obj_from->last_content, next_content, prev_content );

	/* uncover contents */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_COVERING) && obj->first_content )
		empty_obj( obj, obj->in_obj, NULL );

	obj->in_obj		= NULL;
	obj->in_room	= NULL;
	obj->carried_by	= NULL;

	if ( !magic )
	{
		for ( ;  obj_from;  obj_from = obj_from->in_obj )
		{
			if ( obj_from->carried_by )
				obj_from->carried_by->carry_weight -= get_obj_weight( obj );
		}
	}
}


/*
 * Check the recently extracted object queue for obj
 */
bool obj_extracted( OBJ_DATA *obj )
{
	OBJ_DATA *cod;

	if ( obj->serial == cur_obj && cur_obj_extracted )
		return TRUE;

	for ( cod = extracted_obj_queue;  cod;  cod = cod->next )
	{
		if ( obj == cod )
			return TRUE;
	}

	return FALSE;
}


/*
 * Stick obj onto extraction queue
 */
void queue_extracted_obj( OBJ_DATA *obj )
{
	cur_qobjs++;
	obj->next = extracted_obj_queue;
	extracted_obj_queue = obj;
}


/*
 * Clean out the extracted object queue
 */
void clean_obj_queue( void )
{
	OBJ_DATA  *obj;

	while ( extracted_obj_queue )
	{
		obj = extracted_obj_queue;
		extracted_obj_queue = extracted_obj_queue->next;
		clean_object( obj );
		--cur_qobjs;
	}
}


/*
 * Libera la memoria e toglie un oggetto non proto dal mud
 * Non richiama la clean_object, ci penserà la update_handler()
 *	a ripulire gli oggetti estratti di volta in volta
 */
void free_object( OBJ_DATA *obj )
{
	OBJ_DATA	*obj_content;
	REL_DATA	*RQueue;
	REL_DATA	*rq_next;

	if ( obj->prototype )
	{
		send_log( NULL, LOG_BUG, "free_object: l'oggetto passato #%d è un prototipo", obj->vnum );
		return;
	}

	if ( obj_extracted(obj) )
	{
		send_log( NULL, LOG_BUG, "free_object: obj %d already extracted!", obj->vnum );
		return;
	}

	if ( obj->type == OBJTYPE_PORTAL )
		remove_portal( obj );

    if		( obj->carried_by )		obj_from_char( obj );
    else if ( obj->in_room )		obj_from_room( obj );
    else if ( obj->in_obj )			obj_from_obj ( obj );

	while ( (obj_content = obj->last_content) != NULL )
		free_object( obj_content );

	if ( obj == gobj_prev )
		gobj_prev = obj->prev;

	for ( RQueue = first_relation;  RQueue;  RQueue = rq_next )
	{
		rq_next = RQueue->next;
		if ( RQueue->type == relORESTRING_ON )
		{
			if ( obj == RQueue->subject )
				((CHAR_DATA *)RQueue->actor)->dest_buf = NULL;
			else
				continue;
			UNLINK( RQueue, first_relation, last_relation, next, prev );
			DISPOSE( RQueue );
		}
	}

	UNLINK( obj, first_object, last_object, next, prev );

	/* shove onto extraction queue */
	queue_extracted_obj( obj );

	obj->pObjProto->count -= obj->count;
	objects_loaded -= obj->count;
	--objects_physical;

	if ( obj->serial == cur_obj )
	{
		cur_obj_extracted = TRUE;
		if ( global_objcode == rNONE )
			global_objcode = rOBJ_EXTRACTED;
	}
}


/*
 * Find some object with a given index data.
 * Used by area-reset 'P', 'T' and 'H' commands.
 */
OBJ_DATA *get_obj_type( OBJ_PROTO_DATA *pObjProto )
{
	OBJ_DATA *obj;

	for ( obj = last_object;  obj;  obj = obj->prev )
	{
		if ( obj->vnum == pObjProto->vnum )
			return obj;
	}

	return NULL;
}


/*
 * Find an obj in a list.
 * bool equip serve per la can_see_obj
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list, bool equip )
{
	OBJ_DATA *obj;
	char	  arg[MIL];
	int		  number;
	int		  count;

	number = number_argument( argument, arg );
	count  = 0;
	for ( obj = list;  obj;  obj = obj->next_content )
	{
		if ( !can_see_obj(ch, obj, equip) )		continue;
		if ( !nifty_is_name(arg, obj->name) )	continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	/*
	 * If we didn't find an exact match, run through the list of objects
	 * again looking for prefix matching, ie swo == sword.
	 */
	count = 0;
	for ( obj = list;  obj;  obj = obj->next_content )
	{
		if ( !can_see_obj(ch, obj, equip) )				continue;
		if ( !nifty_is_name_prefix(arg, obj->name) )	continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	return NULL;
}


/*
 * Find an obj in a list...going the other way
 * Equip serve per la can_see_obj
 */
OBJ_DATA *get_obj_list_rev( CHAR_DATA *ch, char *argument, OBJ_DATA *list, bool equip )
{
	OBJ_DATA *obj;
	char	  arg[MIL];
	int		  number;
	int		  count;

	number = number_argument( argument, arg );
	count  = 0;
	for ( obj = list;  obj;  obj = obj->prev_content )
	{
		if ( !can_see_obj(ch, obj, equip) )		continue;
		if ( !nifty_is_name(arg, obj->name) )	continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	/*
	 * If we didn't find an exact match, run through the list of objects
	 * again looking for prefix matching, ie swo == sword.
	 */
	count = 0;
	for ( obj = list;  obj;  obj = obj->prev_content )
	{
		if ( !can_see_obj(ch, obj, equip) )				continue;
		if ( !nifty_is_name_prefix(arg, obj->name) )	continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	return NULL;
}


/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, bool equip )
{
	OBJ_DATA *obj;
	char	  arg[MIL];
	int		  number;
	int		  count;
	VNUM	  vnum;

	number = number_argument( argument, arg );
	if ( IS_ADMIN(ch) && is_number(arg) )
		vnum = atoi( arg );
	else
		vnum = -1;

	count = 0;
	for ( obj = ch->last_carrying;  obj;  obj = obj->prev_content )
	{
		if ( obj->wear_loc != WEARLOC_NONE )	continue;
		if ( !can_see_obj(ch, obj, equip) )		continue;
		if ( !nifty_is_name(arg, obj->name) && obj->vnum != vnum )
			continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	if ( vnum != -1 )
		return NULL;

	/*
	 * If we didn't find an exact match, run through the list of objects
	 * again looking for prefix matching, ie swo == sword.
	 */
	count = 0;
	for ( obj = ch->last_carrying;  obj;  obj = obj->prev_content )
	{
		if ( obj->wear_loc != WEARLOC_NONE )				continue;
		if ( !can_see_obj(ch, obj, equip) )				continue;
		if ( !nifty_is_name_prefix(arg, obj->name) )	continue;

		count += obj->count;
		if ( count >= number )
			return obj;
	}

	return NULL;
}


/*
 * Find an obj in player's equipment.
 * Si passa equip per la can_see_obj
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument, bool equip )
{
	OBJ_DATA *obj;
	char	  arg[MIL];
	int		  number;
	int		  count;
	VNUM	  vnum;

	number = number_argument( argument, arg );
	if ( IS_ADMIN(ch) && is_number(arg) )
		vnum = atoi( arg );
	else
		vnum = -1;

	count = 0;
	for ( obj = ch->last_carrying;  obj;  obj = obj->prev_content )
	{
		if ( obj->wear_loc == WEARLOC_NONE )			continue;
		if ( !can_see_obj(ch, obj, equip) )			continue;
		if ( !nifty_is_name(arg, obj->name ) && obj->vnum != vnum )
			continue;

		if ( ++count == number )
			return obj;
	}

	if ( vnum != -1 )
		return NULL;

	/*
	 * If we didn't find an exact match, run through the list of objects
	 * again looking for prefix matching, ie swo == sword.
	 */
	count = 0;
	for ( obj = ch->last_carrying;  obj;  obj = obj->prev_content )
	{
		if ( obj->wear_loc == WEARLOC_NONE )				continue;
		if ( !can_see_obj(ch, obj, TRUE) )				continue;
		if ( !nifty_is_name_prefix(arg, obj->name) )	continue;

		if ( ++count == number )
			return obj;
	}

	return NULL;
}


/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	obj = get_obj_list_rev( ch, argument, ch->in_room->last_content, FALSE );
	if ( obj )
		return obj;

	obj = get_obj_carry( ch, argument, TRUE );
	if ( obj )
		return obj;

	obj = get_obj_wear( ch, argument, TRUE );
	if ( obj )
		return obj;

	return NULL;
}


/*
 * Cerca nella lista passata un tipo di oggetto e ritorna il primo che trova
 */
OBJ_DATA *get_objtype_list_rev( CHAR_DATA *ch, const int type, OBJ_DATA *list, bool equip, bool see )
{
	OBJ_DATA *obj;

	for ( obj = list;  obj;  obj = obj->prev_content )
	{
		if ( see && !can_see_obj(ch, obj, equip) )
			continue;

		if ( obj->type == type )
			return obj;
	}

	return NULL;
}


/*
 * Return a pointer to the first object of a certain type found that
 *	a player is carrying
 */
OBJ_DATA *get_objtype_carry( CHAR_DATA *ch, const int type, bool equip, bool see )
{
	OBJ_DATA *obj;

	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	{
		if ( obj->wear_loc != WEARLOC_NONE )			continue;
		if ( see && !can_see_obj(ch, obj, equip) )	continue;

		if ( obj->type == type )
			return obj;
	}

	return NULL;
}


/*
 * Return a pointer to the first object of a certain type found that
 *	a player is wearing
 */
OBJ_DATA *get_objtype_wear( CHAR_DATA *ch, const int type, bool equip, bool see )
{
	OBJ_DATA *obj;

	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	{
		if ( obj->wear_loc == WEARLOC_NONE )			continue;
		if ( see && !can_see_obj(ch, obj, equip) )	continue;

		if ( obj->type == type )
			return obj;
	}

	return NULL;
}


/*
 * ritorna il primo oggetto che trova cercando prima nella stanza,
 *	poi nel'inventario del pg ed infine addosso al pg
 */
OBJ_DATA *get_objtype_here( CHAR_DATA *ch, const int type, bool see )
{
	OBJ_DATA *obj;

	obj = get_objtype_list_rev( ch, type, ch->in_room->last_content, FALSE, see );
	if ( obj )
		return obj;

	obj = get_objtype_carry( ch, type, TRUE, see );
	if ( obj )
		return obj;

	obj = get_objtype_wear( ch, type, TRUE, see );
	if ( obj )
		return obj;

	return NULL;
}


/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char	  arg[MIL];
	int		  number;
	int		  count;
	VNUM	  vnum;

	obj = get_obj_here( ch, argument );
	if ( obj )
		return obj;

	number = number_argument( argument, arg );

	/*
	 * Allow reference by vnum for Admin
	 */
	if ( IS_ADMIN(ch) && is_number(arg) )
		vnum = atoi( arg );
	else
		vnum = -1;

	count  = 0;
	for ( obj = first_object;  obj;  obj = obj->next )
	{
		/* equip a TRUE perché tale funzione viene utilizzata solo dagli admin */
		if ( !can_see_obj(ch, obj, TRUE) )		continue;
		if ( !nifty_is_name(arg, obj->name) && vnum != obj->vnum )
			continue;

		if ( (count += obj->count) >= number )
			return obj;
	}

	/* bail out if looking for a vnum */
	if ( vnum != -1 )
		return NULL;

	/*
	 * If we didn't find an exact match, run through the list of objects
	 * again looking for prefix matching, ie swo == sword.
	 */
	count  = 0;
	for ( obj = first_object;  obj;  obj = obj->next )
	{
		/* TRUE perché tale funzione viene utilizzata solo dagli admin */
		if ( !can_see_obj(ch, obj, TRUE) )				continue;
		if ( !nifty_is_name_prefix(arg, obj->name) )	continue;

		if ( (count += obj->count) >= number )
			return obj;
	}

	return NULL;
}


/*
 * Generic get obj function that supports optional containers.
 * currently only used for "eat" and "quaff".
 */
OBJ_DATA *find_obj( CHAR_DATA *ch, char *argument, bool carryonly )
{
	char	  arg1[MIL];
	char	  arg2[MIL];
	OBJ_DATA *obj = NULL;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( is_name(arg2, "da from") && VALID_STR(argument) )
		argument = one_argument( argument, arg2 );

	set_char_color( AT_PLAIN, ch );

	if ( !VALID_STR(arg2) )
	{
		if ( carryonly && (obj = get_obj_carry(ch, arg1, TRUE)) == NULL )
		{
			send_to_char( ch, "Non ho con me quest'oggetto.\r\n" );
			return NULL;
		}
		else if ( !carryonly && (obj = get_obj_here(ch, arg1)) == NULL )
		{
			act( AT_PLAIN, "Non trovo nessun $T qui.", ch, NULL, arg1, TO_CHAR );
			return NULL;
		}

		return obj;
	}
	else
	{
		OBJ_DATA *container;

		if ( carryonly
		  && (container = get_obj_carry(ch, arg2, TRUE)) == NULL
		  && (container = get_obj_wear (ch, arg2, TRUE)) == NULL )
		{
			send_to_char( ch, "Non posseggo quest'oggetto.\r\n" );
			return NULL;
		}

		if ( !carryonly && (container = get_obj_here(ch, arg2)) == NULL )
		{
			act( AT_PLAIN, "Non trovo nessun $T qui.", ch, NULL, arg2, TO_CHAR );
			return NULL;
		}

		if ( !container->container )
		{
			send_to_char( ch, "Quest'oggetto non è un contenitore.\r\n" );
			return NULL;
		}

		if ( !HAS_BIT(container->extra_flags, OBJFLAG_COVERING )
		  &&  HAS_BITINT(container->container->flags, CONTAINER_CLOSED) )
		{
			act( AT_PLAIN, "$d è chiuso.", ch, NULL, container->name, TO_CHAR );
			return NULL;
		}

		obj = get_obj_list( ch, arg1, container->first_content, TRUE );
		if ( !obj )
		{
			act( AT_PLAIN, HAS_BIT(container->extra_flags, OBJFLAG_COVERING)  ?
				"Non trovo nulla del genere sotto $p."  :
				"Non trovo nulla del genere dentro $p.", ch, container, NULL, TO_CHAR );
		}
		return obj;
	}

	return NULL;
}


/*
 * Return weight of an object, including weight of contents (unless magic).
 */
int get_obj_weight( OBJ_DATA *obj )
{
	int		weight;

	weight = obj->count * obj->weight;

	/* magic containers */
	if ( obj->type != OBJTYPE_CONTAINER || !HAS_BIT(obj->extra_flags, OBJFLAG_MAGIC) )
	{
		for ( obj = obj->first_content;  obj;  obj = obj->next_content )
			weight += get_obj_weight( obj );
	}

	return weight;
}


/*
 * Funzioncina che ritorna vero se l'oggetto lo si vede anche se la stanza è buia
 * Il controllo su questa funzione deve avvenire in stanze buie
 * Questa funzione faceva parte della can_see_obj()
 *	è stata divisa per essere utilizzata anche nel comando do_equipment()
 */
bool can_see_obj_dark( CHAR_DATA *ch, OBJ_DATA *obj, bool equip )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "can_see_obj_dark: ch è NULL" );
		return FALSE;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "can_see_obj_dark: obj è NULL" );
		return FALSE;
	}

	if ( !room_is_dark(ch->in_room) )
	{
		send_log( NULL, LOG_BUG, "can_see_obj_dark: la stanza in cui si trova il ch %s non è buia", ch->name );
		return FALSE;
	}

	/* can see lights in the dark */
	if ( obj->light && obj->light->hours != 0 )
		return TRUE;

	/* can see glowing items in the dark... invisible or not */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_GLOW) )
		return TRUE;

	if ( HAS_BIT(ch->affected_by, AFFECT_INFRARED) )
		return TRUE;

	if ( equip )
		return TRUE;

	return FALSE;
}


/*
 * True if char can see obj.
 * equip è un booleano che se TRUE indica che il pg sta cercando
 *	di interagire con il proprio equipaggiamento o inventario,
 *	e quindi se al buio ha qualche probabilità di prenderlo.
 * A volte bisogna settare il valore equip a TRUE per interagire
 *	con oggetti anche non del pg, tipo la lista nei negozi.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool equip )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "can_see_obj: ch è NULL" );
		return FALSE;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "can_see_obj: obj è NULL (ch = %s)",
			IS_PG(ch) ? ch->name : ch->short_descr );
		return FALSE;
	}

	if ( IS_ADMIN(ch) && IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ADMSIGHT) )
		return TRUE;

	if ( IS_MOB(ch) && ch->pIndexData->vnum == VNUM_MOB_SUPERMOB )
		return TRUE;

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_BURIED) )
		return FALSE;

	if ( HAS_BIT(obj->extra_flags, OBJFLAG_HIDDEN) )
		return FALSE;

	if ( HAS_BIT(ch->affected_by, AFFECT_TRUESIGHT) )
		return TRUE;

	/* Gli si è spiriti allora vedono */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SPIRIT) )
		return TRUE;

	/* Se equip è a TRUE allora controlla se l'oggetto sia addosso al pg, se non lo è setta a FALSE */
	if ( equip && carried_by(obj) != ch )
		equip = FALSE;

	if ( HAS_BIT(ch->affected_by, AFFECT_BLIND) && !equip )
		return FALSE;

	if ( room_is_dark(ch->in_room) )
		return can_see_obj_dark( ch, obj, equip );

	/* Tenta di trovarlo a tastoni se è un oggetto nel proprio equip */
	if ( HAS_BIT(obj->extra_flags, OBJFLAG_INVIS) && !HAS_BIT(ch->affected_by, AFFECT_DETECT_INVIS) && !equip )
		return FALSE;

	return TRUE;
}


/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !HAS_BIT(obj->extra_flags, OBJFLAG_NODROP) )
		return TRUE;

	if ( IS_ADMIN(ch) )
		return TRUE;

	if ( IS_MOB(ch) && ch->pIndexData->vnum == VNUM_MOB_SUPERMOB )
		return TRUE;

	return FALSE;
}


/*
 * Set off a trap (obj) upon character (ch)
 */
ch_ret spring_trap( CHAR_DATA *ch, OBJ_DATA *obj )
{
	char	*txt;
	char	 buf[MSL];
	int		 dam;
	int		 typ;
	int		 lev;
	ch_ret	 retcode;

	retcode = rNONE;
	
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "spring_trap: ch è NULL" );
		return retcode;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_BUG, "spring_trap: obj è NULL" );
		return retcode;
	}

	if ( !obj->trap )
	{
		send_log( NULL, LOG_BUG, "spring_trap: l'oggetto non è una trappola" );
		return retcode;
	}

	typ = obj->trap->type;
	lev = obj->trap->level;

	switch ( typ )
	{
	  default:						txt = "hit by a trap";							break;
	  case TRAPTYPE_POISON_GAS:		txt = "surrounded by a green cloud of gas";		break;
	  case TRAPTYPE_POISON_DART:	txt = "hit by a dart";							break;
	  case TRAPTYPE_POISON_NEEDLE:	txt = "pricked by a needle";					break;
	  case TRAPTYPE_POISON_DAGGER:	txt = "stabbed by a dagger";					break;
	  case TRAPTYPE_POISON_ARROW:	txt = "struck with an arrow";					break;
	  case TRAPTYPE_BLINDNESS_GAS:	txt = "surrounded by a red cloud of gas";		break;
	  case TRAPTYPE_SLEEPING_GAS:	txt = "surrounded by a yellow cloud of gas";	break;
	  case TRAPTYPE_FLAME:			txt = "struck by a burst of flame";				break;
	  case TRAPTYPE_EXPLOSION:		txt = "hit by an explosion";					break;
	  case TRAPTYPE_ACID_SPRAY:		txt = "covered by a spray of acid";				break;
	  case TRAPTYPE_ELECTRIC_SHOCK:	txt = "suddenly shocked";						break;
	  case TRAPTYPE_BLADE:			txt = "sliced by a razor sharp blade";			break;
	  case TRAPTYPE_SEX_CHANGE:		txt = "surrounded by a mysterious aura";		break;
	}

	dam = number_range( lev, lev * 2);
	sprintf( buf, "You are %s!", txt );
	act( AT_HITME, buf, ch, NULL, NULL, TO_CHAR );
	sprintf( buf, "$n is %s.", txt );
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
	--obj->trap->charges;

	if ( obj->trap->charges <= 0 )
	{
		/* Se era una trappola di per sè la estrae */
		if ( obj->type == OBJTYPE_TRAP )
			free_object( obj );
		else	/* Se invece per esempio era una arma con una trappola toglie solo i value della trappola */
			free_values( obj, OBJTYPE_TRAP );
	}

	switch ( typ )
	{
	  default:
	  case TRAPTYPE_POISON_DART:
	  case TRAPTYPE_POISON_NEEDLE:
	  case TRAPTYPE_POISON_DAGGER:
	  case TRAPTYPE_POISON_ARROW:
		/* hmm... why not use spell_poison() here? */
		retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
		if ( retcode == rNONE )
			retcode = damage( ch, ch, dam, TYPE_UNDEFINED );
		break;

	  case TRAPTYPE_POISON_GAS:
		retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
		break;

	  case TRAPTYPE_BLINDNESS_GAS:
		retcode = obj_cast_spell( gsn_blindness, lev, ch, ch, NULL );
		break;

	  case TRAPTYPE_SLEEPING_GAS:
		retcode = obj_cast_spell( skill_lookup("sleep"), lev, ch, ch, NULL );
		break;

	  case TRAPTYPE_ACID_SPRAY:
		retcode = obj_cast_spell( skill_lookup("acid blast"), lev, ch, ch, NULL );
		break;

	  case TRAPTYPE_SEX_CHANGE:
		retcode = obj_cast_spell( skill_lookup("change sex"), lev, ch, ch, NULL );
		break;

	  case TRAPTYPE_FLAME:
	  case TRAPTYPE_EXPLOSION:
		retcode = obj_cast_spell( gsn_fireball, lev, ch, ch, NULL );
		break;

	  case TRAPTYPE_ELECTRIC_SHOCK:
	  case TRAPTYPE_BLADE:
		retcode = damage( ch, ch, dam, TYPE_UNDEFINED );
	}

	return retcode;
}


/*
 * Check an object for a trap
 */
ch_ret check_for_trap( CHAR_DATA *ch, OBJ_DATA *obj, int flag )
{
	OBJ_DATA *check;
	ch_ret	  retcode;

	if ( !obj->first_content )
		return rNONE;

	retcode = rNONE;

	for ( check = obj->first_content;  check;  check = check->next_content )
	{
		if ( !check->trap )
			continue;

		if ( HAS_BITINT(check->trap->flags, flag) )
		{
			retcode = spring_trap( ch, check );
			if ( retcode != rNONE )
				return retcode;
		}
	}

	return retcode;
}


/*
 * Check the room for a trap
 */
ch_ret check_room_for_traps( CHAR_DATA *ch, int flag )
{
	OBJ_DATA *check;
	ch_ret    retcode;

	retcode = rNONE;

	if ( !ch )
		return rERROR;
	if ( !ch->in_room || !ch->in_room->first_content )
		return rNONE;

	for ( check = ch->in_room->first_content;  check;  check = check->next_content )
	{
		if ( !check->trap )
			continue;

		if ( HAS_BITINT(check->trap->flags, flag) )
		{
			retcode = spring_trap( ch, check );
			if ( retcode != rNONE )
				return retcode;
		}
	}

	return retcode;
}


/*
 * Return TRUE if an object contains a trap
 */
bool is_trapped( OBJ_DATA *obj )
{
	OBJ_DATA *check;

	if ( !obj->first_content )
		return FALSE;

	for ( check = obj->first_content; check; check = check->next_content )
	{
		if ( check->type == OBJTYPE_TRAP )
			return TRUE;
	}

    return FALSE;
}


/*
 * If an object contains a trap, return the pointer to the trap	-Thoric
 */
OBJ_DATA *get_trap( OBJ_DATA *obj )
{
	OBJ_DATA *check;

	if ( !obj->first_content )
		return NULL;

	for ( check = obj->first_content;  check;  check = check->next_content )
	{
		if ( check->type == OBJTYPE_TRAP )
			return check;
	}

	return NULL;
}


/*
 * Remove an exit from a room
 */
void extract_exit( ROOM_DATA *room, EXIT_DATA *pexit )
{
	UNLINK( pexit, room->first_exit, room->last_exit, next, prev );

	if ( pexit->rexit )
		pexit->rexit->rexit = NULL;

	DISPOSE( pexit->keyword );
	DISPOSE( pexit->description );
	DISPOSE( pexit );

	top_exit--;
}


/*
 * Set the current global object to obj
 */
void set_cur_obj( OBJ_DATA *obj )
{
	cur_obj = obj->serial;
	cur_obj_extracted = FALSE;
	global_objcode = rNONE;
}


/*
 * Returns area with name matching input string
 */
AREA_DATA *get_area( char *name )
{
	AREA_DATA *pArea;

	if ( !VALID_STR(name) )
	{
		send_log( NULL, LOG_BUG, "get_area: NULL input string." );
		return NULL;
	}

	/* Cerca tra le aree caricate */
	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		if ( !str_cmp(name, pArea->name) )
			return pArea;
	}

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		if ( nifty_is_name(name, pArea->name) )
			return pArea;
	}

	/* Cerca tra le aree building */
	for ( pArea = first_build;  pArea;  pArea = pArea->next )
	{
		if ( !str_cmp(name, pArea->name) )
			return pArea;
	}

	for ( pArea = first_build;  pArea;  pArea = pArea->next )
	{
		if ( nifty_is_name(name, pArea->name) )
			return pArea;
	}

	return NULL;
}


AREA_DATA *get_area_obj( OBJ_PROTO_DATA *pObjProto )
{
	AREA_DATA *pArea;

	if ( !pObjProto )
	{
		send_log( NULL, LOG_BUG, "get_area_obj: pObjProto is NULL" );
		return NULL;
	}

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		if ( pObjProto->vnum >= pArea->vnum_low && pObjProto->vnum <= pArea->vnum_high )
			break;
	}

	return pArea;
}
