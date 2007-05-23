#ifndef OLC_H
#define OLC_H


/**************************************************************************\
 *     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)         *
 *                                                                        *
 *   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin      *
 **************************************************************************
 *                         Defines, structs, etc..                        *
\**************************************************************************/


/*
 * Funzioni
 */
void	oedit_parse				( DESCRIPTOR_DATA *d, char *arg );
void	medit_parse				( DESCRIPTOR_DATA *d, char *arg );
void	redit_parse				( DESCRIPTOR_DATA *d, char *arg );
void	strip_string			( char * );
void	cleanup_olc				( DESCRIPTOR_DATA *d );
void	olc_log					( DESCRIPTOR_DATA *d, char *format, ... ) __attribute__ ( (format (printf, 2, 3)) );
bool	is_in_olc				( DESCRIPTOR_DATA *d );
void	oedit_disp_extra_choice	( DESCRIPTOR_DATA *d );
void	medit_disp_aff_flags	( DESCRIPTOR_DATA *d );
void	medit_disp_ris			( DESCRIPTOR_DATA *d );


/*
 * Struttura OLC.
 */
struct olc_data
{
	int				  mode;
	int				  zone_num;
	int				  number;
	int				  value;
	bool			  changed;
	MOB_PROTO_DATA	 *mob;
	ROOM_DATA		 *room;
	OBJ_DATA		 *obj;
	AREA_DATA		 *area;
	SHOP_DATA		 *shop;
	EXTRA_DESCR_DATA *desc;
	AFFECT_DATA		 *paf;
	EXIT_DATA		 *xit;
};

struct olc_save_info
{
	int		zone;
	char	type;
	struct olc_save_info *next;
};


/*
 * Descriptor access macros
 */
#define OLC_MODE(d)		( (d)->olc->mode )		/* Parse input mode	 */
#define OLC_NUM(d)		( (d)->olc->number )	/* Room/Obj VNUM	 */
#define OLC_VNUM(d)		OLC_NUM(d)
#define OLC_VAL(d)		( (d)->olc->value )		/* Scratch variable	 */
#define OLC_OBJ(d)		( obj )
#define OLC_DESC(d)		( (d)->olc->desc )		/* Extra description */
#define OLC_AFF(d)		( (d)->olc->paf )		/* Affect data		 */
#define OLC_CHANGE(d)	( (d)->olc->changed )	/* Changed flag		 */
#define OLC_EXIT(d)		( (d)->olc->xit )		/* An Exit			 */


/*
 * Add/Remove save list types.
 */
typedef enum
{
	OLCSAVE_ROOM,
	OLCSAVE_OBJ,
	OLCSAVE_ZONE,
	OLCSAVE_MOB,
	OLCSAVE_SHOP
	/* 5 tipi di salvataggi ocl */
} ocl_save_types;


/*
 * Submodes of OEDIT connectedness.
 */
typedef enum
{
	OEDIT_MAIN_MENU,
	OEDIT_EDIT_NAMELIST,
	OEDIT_SHORTDESC,
	OEDIT_LONGDESC,
	OEDIT_ACTDESC,
	OEDIT_TYPE,
	OEDIT_EXTRAS,
	OEDIT_WEAR,
	OEDIT_WEIGHT,
	OEDIT_COST,
	OEDIT_TIMER,
	OEDIT_VALUE_1,
	OEDIT_VALUE_2,
	OEDIT_VALUE_3,
	OEDIT_VALUE_4,
	OEDIT_VALUE_5,
	OEDIT_VALUE_6,
	OEDIT_EXTRADESC_KEY,
	OEDIT_CONFIRM_SAVEDB,
	OEDIT_CONFIRM_SAVESTRING,
	OEDIT_EXTRADESC_DESCRIPTION,
	OEDIT_EXTRADESC_MENU,
	OEDIT_LEVEL,
	OEDIT_LAYERS,
	OEDIT_AFFECT_MENU,
	OEDIT_AFFECT_LOCATION,
	OEDIT_AFFECT_MODIFIER,
	OEDIT_AFFECT_REMOVE,
	OEDIT_AFFECT_RIS,
	OEDIT_EXTRADESC_CHOICE,
	OEDIT_EXTRADESC_DELETE
	/* 32 tipi di sottomodi di oedit */
} oedit_submode_types;


/*
 * Submodes of REDIT connectedness
 */
typedef enum
{
	REDIT_MAIN_MENU,
	REDIT_NAME,
	REDIT_DESC,
	REDIT_FLAGS,
	REDIT_SECTOR,
	REDIT_EXIT_MENU,
	REDIT_CONFIRM_SAVEDB,
	REDIT_CONFIRM_SAVESTRING,
	REDIT_EXIT_VNUM,
	REDIT_EXIT_DESC,
	REDIT_EXIT_KEYWORD,
	REDIT_EXIT_KEY,
	REDIT_EXIT_FLAGS,
	REDIT_EXTRADESC_MENU,
	REDIT_EXTRADESC_KEY,
	REDIT_EXTRADESC_DESCRIPTION,
	REDIT_TUNNEL,
	REDIT_TELEDELAY,
	REDIT_TELEVNUM,
	REDIT_EXIT_EDIT,
	REDIT_EXIT_ADD,
	REDIT_EXIT_DELETE,
	REDIT_EXIT_ADD_VNUM,
	REDIT_EXTRADESC_DELETE,
	REDIT_EXTRADESC_CHOICE
	/* 26 sottomodi di redit */
} redit_submode_types;


/*
 * Submodes of ZEDIT connectedness.
 */
typedef enum
{
	ZEDIT_MAIN_MENU,
	ZEDIT_DELETE_ENTRY,
	ZEDIT_NEW_ENTRY,
	ZEDIT_CHANGE_ENTRY,
	ZEDIT_COMMAND_TYPE,
	ZEDIT_IF_FLAG,
	ZEDIT_ARG1,
	ZEDIT_ARG2,
	ZEDIT_ARG3,
	ZEDIT_ZONE_NAME,
	ZEDIT_ZONE_LIFE,
	ZEDIT_ZONE_TOP,
	ZEDIT_ZONE_RESET,
	ZEDIT_CONFIRM_SAVESTRING
	/* 14 sottomodi di zedit */
} zedit_submode_types;


/*
 * Submodes of MEDIT connectedness.
 */
typedef enum
{
	MEDIT_NPC_MAIN_MENU,
	MEDIT_PC_MAIN_MENU,
	MEDIT_NAME,
	MEDIT_S_DESC,
	MEDIT_L_DESC,
	MEDIT_D_DESC,
	MEDIT_NPC_FLAGS,
	MEDIT_PLAYER_FLAGS,
	MEDIT_AFFECT_FLAGS,
	MEDIT_CONFIRM_SAVESTRING,
	MEDIT_SEX,
	MEDIT_HITROLL,
	MEDIT_DAMROLL,
	MEDIT_DAMNUMDIE,
	MEDIT_DAMSIZEDIE,
	MEDIT_DAMPLUS,
	MEDIT_HITNUMDIE,
	MEDIT_HITSIZEDIE,
	MEDIT_HITPLUS,
	MEDIT_AC,
	MEDIT_GOLD,
	MEDIT_POS,
	MEDIT_DEFAULT_POS,
	MEDIT_ATTACK,
	MEDIT_DEFENSE,
	MEDIT_LEVEL,
	MEDIT_ALIGNMENT,

	MEDIT_STR,
	MEDIT_RES,
	MEDIT_CON,
	MEDIT_INT,
	MEDIT_COC,
	MEDIT_WIL,
	MEDIT_AGI,
	MEDIT_REF,
	MEDIT_SPE,
	MEDIT_SPI,
	MEDIT_MIG,
	MEDIT_ABS,
	MEDIT_EMP,
	MEDIT_BEA,
	MEDIT_LEA,

	MEDIT_SIGHT,
	MEDIT_HEARING,
	MEDIT_SMELL,
	MEDIT_TASTE,
	MEDIT_TOUCH,
	MEDIT_SIXTH,

	MEDIT_CLAN,
	MEDIT_SPEC,
	MEDIT_RESISTANT,
	MEDIT_IMMUNE,
	MEDIT_SUSCEPTIBLE,
	MEDIT_PGDATA_FLAGS,
	MEDIT_MENTALSTATE,
	MEDIT_EMOTIONAL,
	MEDIT_THIRST,
	MEDIT_FULL,
	MEDIT_DRUNK,
	MEDIT_PARTS,

	MEDIT_LIFE,
	MEDIT_MANA,
	MEDIT_MOVE,
	MEDIT_SOUL,

	MEDIT_PRACTICE,
	MEDIT_PASSWORD,
	MEDIT_SAVE_MENU,

	MEDIT_SAV1,
	MEDIT_SAV2,
	MEDIT_SAV3,
	MEDIT_SAV4,
	MEDIT_SAV5,

	MEDIT_CLASS,
	MEDIT_RACE
	/* 75 sottomodi di medit */
} medit_submode_types;


/*
 * Submodes of SEDIT connectedness.
 */
typedef enum
{
	SEDIT_MAIN_MENU,
	SEDIT_CONFIRM_SAVESTRING,
	SEDIT_NOITEM1,
	SEDIT_NOITEM2,
	SEDIT_NOCASH1,
	SEDIT_NOCASH2,
	SEDIT_NOBUY,
	SEDIT_BUY,
	SEDIT_SELL,
	SEDIT_PRODUCTS_MENU,
	SEDIT_ROOMS_MENU,
	SEDIT_NAMELIST_MENU,
	SEDIT_NAMELIST
	/* 13 tipi di sottomodi sedit */
} sedit_submode_types;


/*
 * Numerical responses.
 */
typedef enum
{
	SEDIT_NUMERICAL_RESPONSE = 20,
	SEDIT_OPEN1,
	SEDIT_OPEN2,
	SEDIT_CLOSE1,
	SEDIT_CLOSE2,
	SEDIT_KEEPER,
	SEDIT_BUY_PROFIT,
	SEDIT_SELL_PROFIT,
	SEDIT_TYPE_MENU,
	SEDIT_DELETE_TYPE,
	SEDIT_DELETE_PRODUCT,
	SEDIT_NEW_PRODUCT,
	SEDIT_DELETE_ROOM,
	SEDIT_NEW_ROOM,
	SEDIT_SHOP_FLAGS,
	SEDIT_NOTRADE
	/* 16 tipi di risposte numeriche al sedit */
} sedit_response_types;


#endif	/* OLC_H */
