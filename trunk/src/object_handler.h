#ifndef OBJECT_HANDLER_H
#define OBJECT_HANDLER_H


/*
 * Extra flags.
 */
typedef enum
{
	OBJFLAG_GLOW,
	OBJFLAG_HUM,				/* (FF) Se uno ha un ogg del genere che fa rumore lo snaek e l'hide perdono potenza */
	OBJFLAG_DARK,
	OBJFLAG_LOYAL,				/* Evita di cadere dalla presa dell'avventuriero */
	OBJFLAG_EVIL,				/* (CC) L'oggetto lo con meno sbalzo di livello un evil */
	OBJFLAG_INVIS,
	OBJFLAG_MAGIC,
	OBJFLAG_NODROP,
	OBJFLAG_BLESS,
	OBJFLAG_ANTI_GOOD,
	OBJFLAG_ANTI_EVIL,
	OBJFLAG_ANTI_NEUTRAL,
	OBJFLAG_NOREMOVE,
	OBJFLAG_INVENTORY,
	OBJFLAG_ANTI_MAGE,			/* (CC) da togliere questi 4 e fare un campo apposito e uno anche razziale per restrizione di wearing */
	OBJFLAG_ANTI_THIEF,
	OBJFLAG_ANTI_WARRIOR,
	OBJFLAG_ANTI_CLERIC,
	OBJFLAG_ORGANIC,			/* (CC) da togliere questi due e inserire nel sistema materiali */
	OBJFLAG_METAL,
	OBJFLAG_DONATION,
	OBJFLAG_CLANOBJECT,
	OBJFLAG_CLANCORPSE,
	OBJFLAG_ANTI_VAMPIRE,		/* (CC) questi due come sopra */
	OBJFLAG_ANTI_DRUID,
	OBJFLAG_HIDDEN,
	OBJFLAG_POISONED,
	OBJFLAG_COVERING,
	OBJFLAG_DEATHROT,
	OBJFLAG_BURIED,
	OBJFLAG_SECRET,			/* (CC) attenzione era la flag prototipe, ora serve a non visualizzare l'oggetto nella lista di oggetti della stanza */
	OBJFLAG_NOLOCATE,
	OBJFLAG_GROUNDROT,
	OBJFLAG_LOOTABLE,
	OBJFLAG_SMUGGLE,			/* (FF) Tipologia di oggetto che serve per tracciare i contrabbandi e i sequestri */
	OBJFLAG_GOOD,				/* (GG) (FF) L'oggetto lo impugna con meno sbalzo di livello un buono */
	OBJFLAG_NEUTRAL,			/* (GG) (FF) L'oggetto lo impugna con meno sbalzo di livello un neutrale */
	/* 34 flag di oggetto */
	MAX_OBJFLAG
} object_flags;


/*
 * Flag magiche.
 * Servono per gli oggetti che sono utilizzati negli incantesimi
 * (FF) le accomunerei agli OBJFLAG_ ma è da vedere
 */
typedef enum
{
	OBJMAGIC_RETURNING,
	OBJMAGIC_BACKSTABBER,
	OBJMAGIC_BANE,
	OBJMAGIC_MAGIC_LOYAL,
	OBJMAGIC_HASTE,
	OBJMAGIC_DRAIN,
	OBJMAGIC_LIGHTNING_BLADE,
	OBJMAGIC_PKDISARMED,		/* (FF) questa flag da qualche parte deve essere tolta all'oggetto che la ha, forse in save? */
	/* 8 tipi di magic flags per oggetti */
	MAX_OBJMAGIC
} object_magic_flags;


/*
 * Flag di Wear.
 */
typedef enum
{
	OBJWEAR_TAKE,
	OBJWEAR_FINGER,
	OBJWEAR_NECK,
	OBJWEAR_BODY,
	OBJWEAR_HEAD,
	OBJWEAR_LEGS,
	OBJWEAR_FEET,
	OBJWEAR_HANDS,
	OBJWEAR_ARMS,
	OBJWEAR_SHIELD,
	OBJWEAR_ABOUT,
	OBJWEAR_WAIST,
	OBJWEAR_WRIST,
	OBJWEAR_WIELD,
	OBJWEAR_HOLD,
	OBJWEAR_DUAL,
	OBJWEAR_EARS,
	OBJWEAR_EYES,
	OBJWEAR_MISSILE,
	OBJWEAR_BACK,
	OBJWEAR_FACE,
	OBJWEAR_ANKLE,
	OBJWEAR_FLOAT,
	/* 22 wear flags */
	MAX_OBJWEAR
} objext_wear_types;


/*
 * Locazioni Wear per l'equipaggiamento.
 * Utilizzati in #RESETS.
 */
typedef enum
{
	WEARLOC_NONE = -1,
	WEARLOC_LIGHT = 0,
	WEARLOC_FINGER_L,
	WEARLOC_FINGER_R,
	WEARLOC_NECK,
	WEARLOC_NECK_2,	/* (CC) da togliere */
	WEARLOC_BODY,
	WEARLOC_HEAD,
	WEARLOC_LEGS,
	WEARLOC_FEET,
	WEARLOC_HANDS,
	WEARLOC_ARMS,
	WEARLOC_SHIELD,
	WEARLOC_ABOUT,
	WEARLOC_WAIST,
	WEARLOC_WRIST_L,
	WEARLOC_WRIST_R,
	WEARLOC_WIELD,
	WEARLOC_HOLD,
	WEARLOC_DUAL,
	WEARLOC_EARS,
	WEARLOC_EYES,
	WEARLOC_MISSILE,
	WEARLOC_BACK,
	WEARLOC_FACE,
	WEARLOC_ANKLE_L,
	WEARLOC_ANKLE_R,
	WEARLOC_FLOAT,
	MAX_WEARLOC
} wear_location_types;


/*
 * Struttura di un oggetto, sia indicizzato che non
 */
struct object_data
{
	OBJ_DATA		   *next;
	OBJ_DATA		   *prev;

	bool				prototype;			/* Indica se l'oggetto è un prototipo */

	/* Queste variabili cambiano significato per gli oggetti prototipo o no */
	VNUM				vnum;				/* Prototipo: Vnum del prototipo / Non prototipo: vnum del prototipo da cui l'oggetto è stato creato */
	AREA_DATA		   *area;				/* Prototipo: indica l'area in cui si trova l'oggetto / Non prototipo: indica l'area in cui si trova il suo prototipo */
	int					count;				/* Prototipo: conta quanti oggetti di quel tipo sono stati caricati nel mud / Non prototipo: indica il contatore di gruppo */

	/* Variabili utilizzate per gli oggetti non prototipo */
	OBJ_DATA		   *pObjProto;			/* Puntatore all'oggetto prototipo */
	OBJ_DATA		   *first_content;		/* Primo oggetto contenuto */
	OBJ_DATA		   *last_content;		/* Ultimo oggetto contenuto */
	OBJ_DATA		   *prev_content;		/* Contenuto precedente */
	OBJ_DATA		   *next_content;		/* Contenuto seguente */
	OBJ_DATA		   *in_obj;				/* Oggetto che lo contiene */
	CHAR_DATA		   *carried_by;			/* Chi lo trasporta */
	ROOM_DATA		   *in_room;			/* Stanza in cui si trova */

	/* Variabili sia per gli oggetti prototipo che non */
	AFFECT_DATA 	   *first_affect;
	AFFECT_DATA 	   *last_affect;
	EXTRA_DESCR_DATA   *first_extradescr;	/* Prima decrizione extra */
	EXTRA_DESCR_DATA   *last_extradescr;	/* Ultima decrizione extra */
	char			   *name;				/* Keyword per l'interazione con l'oggetto */
	char			   *short_descr;		/* Descrizione corta */
	char			   *long_descr;			/* Descrizione lunga */
#ifdef T2_ALFA
	char			   *description;		/* (GG) da mettere qui la extra con la stessa keyword delle keywords */
#endif
	char			   *action_descr;
	char			   *owner;				/* Propretario dell'oggetto */
	int					level;
	int					type;
	BITVECTOR		   *extra_flags;
	BITVECTOR		   *magic_flags;		/* Need more bitvectors for spells */
	BITVECTOR		   *wear_flags;
	int					wear_loc;
	int					weight;
	int					cost;
	int					condition;			/* (GG) */
	int					timer;
	BITVECTOR		   *layers;				/* (FF) convertirlo in int, che indichi che tipo di abbigliamento sia, quindi alla fin fine è da mettere tra i value della armor, dopo questa modifica si può togliere questa variabile e anche l'ITEM_DRESS */
	int					serial;				/* serial number			   */
	/* Queste strutture sono state inserite al posto dei value per maggiore chiarezza e potenza
	 *	molti oggetti possono essere utilizzati quindi per più cose, es: luce, arma e contenitore */
	VAL_LIGHT_DATA	   *light;
	VAL_SCROLL_DATA	   *scroll;
	VAL_SCROLL_DATA	   *potion;
	VAL_SCROLL_DATA	   *pill;
	VAL_WAND_DATA	   *wand;
	VAL_WEAPON_DATA	   *weapon;
	VAL_TREASURE_DATA  *treasure;
	VAL_ARMOR_DATA	   *armor;
	VAL_FURNITURE_DATA *furniture;
	VAL_TRASH_DATA	   *trash;
	VAL_CONTAINER_DATA *container;
	VAL_DRINKCON_DATA  *drinkcon;
	VAL_KEY_DATA	   *key;
	VAL_FOOD_DATA	   *food;
	VAL_MONEY_DATA	   *money;
	VAL_PEN_DATA	   *pen;
	VAL_BOAT_DATA	   *boat;
	VAL_CORPSE_DATA	   *corpse;
	VAL_PIPE_DATA	   *pipe;
	VAL_HERB_DATA	   *herb;
	VAL_LEVER_DATA	   *lever;
	VAL_TRAP_DATA	   *trap;
	VAL_PAPER_DATA	   *paper;
	VAL_SALVE_DATA	   *salve;
	VAL_SHEATH_DATA	   *sheath;
	/* Variabili per il mudprog */
	MPROG_DATA		   *first_mudprog;		/* objprogs */
	MPROG_DATA		   *last_mudprog;
	MPROG_ACT_LIST	   *mpact;				/* objprogs */
	int					mpactnum;			/* objprogs */
};


/*
 * Variabili
 */
extern	const char		   *wear_name[];
extern	const CODE_DATA		code_objflag[];
extern	const CODE_DATA		code_objmagic[];
extern	const CODE_DATA		code_objwear[];
extern	const int			max_check_objflag;
extern	const int			max_check_objmagic;
extern	const int			max_check_objwear;


/*
 * Funzioni
 */
bool		is_trapped			( OBJ_DATA *obj );
OBJ_DATA   *find_obj			( CHAR_DATA *ch, char *argument, bool carryonly );
void		copy_object			( OBJ_DATA *source, OBJ_DATA *dest );
OBJ_DATA   *clone_object 		( OBJ_DATA *obj );
OBJ_DATA   *get_objtype_carry	( CHAR_DATA *ch, int type, bool equip, bool see );
OBJ_DATA   *get_obj_type		( OBJ_PROTO_DATA *pObjProto );
OBJ_DATA   *get_obj_list		( CHAR_DATA *ch, char *argument, OBJ_DATA *list, bool equip );
OBJ_DATA   *get_obj_list_rev	( CHAR_DATA *ch, char *argument, OBJ_DATA *list, bool equip );
int			get_real_obj_weight	( OBJ_DATA *obj );
int			count_obj_list		( OBJ_PROTO_DATA *pObjProto, OBJ_DATA *list );
void		clean_obj_queue		( void );
OBJ_DATA   *obj_to_char			( OBJ_DATA *obj, CHAR_DATA *ch );
void		obj_from_char		( OBJ_DATA *obj );
int			apply_ac			( OBJ_DATA *obj, int iWear );
OBJ_DATA   *get_eq_char			( CHAR_DATA *ch, int iWear );
void		equip_char			( CHAR_DATA *ch, OBJ_DATA *obj, int iWear );
void		unequip_char		( CHAR_DATA *ch, OBJ_DATA *obj );
void		obj_from_room		( OBJ_DATA *obj );
OBJ_DATA   *obj_to_room			( OBJ_DATA *obj, ROOM_DATA *pRoom );
void		obj_from_obj		( OBJ_DATA *obj );
OBJ_DATA   *obj_to_obj			( OBJ_DATA *obj, OBJ_DATA *obj_to );
void		free_object			( OBJ_DATA *obj );
void		extract_exit		( ROOM_DATA *room, EXIT_DATA *pexit );
OBJ_DATA   *get_obj_carry		( CHAR_DATA *ch, char *argument, bool equip );
OBJ_DATA   *get_obj_wear		( CHAR_DATA *ch, char *argument, bool equip );
OBJ_DATA   *get_obj_here		( CHAR_DATA *ch, char *argument );
OBJ_DATA   *get_obj_world		( CHAR_DATA *ch, char *argument );
int			get_obj_weight		( OBJ_DATA *obj );
bool		can_see_obj_dark	( CHAR_DATA *ch, OBJ_DATA *obj, bool equip );
bool		can_see_obj			( CHAR_DATA *ch, OBJ_DATA *obj, bool equip );
bool		can_drop_obj		( CHAR_DATA *ch, OBJ_DATA *obj );
ch_ret		check_for_trap		( CHAR_DATA *ch, OBJ_DATA *obj, int flag );
ch_ret		check_room_for_traps( CHAR_DATA *ch, int flag );
OBJ_DATA   *get_trap			( OBJ_DATA *obj );
ch_ret		spring_trap			( CHAR_DATA *ch, OBJ_DATA *obj );
void		set_cur_obj			( OBJ_DATA *obj );
bool		obj_extracted		( OBJ_DATA *obj );
OBJ_DATA   *split_obj			( OBJ_DATA *obj, int num );
bool		empty_obj			( OBJ_DATA *obj, OBJ_DATA *destobj, ROOM_DATA *destroom );
AREA_DATA  *get_area			( char *name );
AREA_DATA  *get_area_obj		( OBJ_PROTO_DATA * pObjProto );


#endif	/* OBJECT_HANDLER_H */
