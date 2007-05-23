#ifndef VALUES_H
#define VALUES_H


/*
 * Tipi di oggetti
 * (CC) dopo la conversione inserire i tipi di oggetti che hanno i vlaue in alto e in basso quelli che non ne hanno, così posso diminuire gli elementi dell'array futuro dei value negli oggetti non inserendo per forza MAX_OBJTYPE
 */
typedef enum
{
	OBJTYPE_NONE,			/* 0 */
	OBJTYPE_LIGHT,
	OBJTYPE_SCROLL,
	OBJTYPE_WAND,
	OBJTYPE_STAFF,
	OBJTYPE_WEAPON,
	OBJTYPE_FIREWEAPON,	/* (RR) da togliere */
	OBJTYPE_MISSILE,		/* (RR) da togliere */
	OBJTYPE_TREASURE,
	OBJTYPE_ARMOR,
	OBJTYPE_POTION,		/* 10 */
	OBJTYPE_WORN,
	OBJTYPE_FURNITURE,
	OBJTYPE_TRASH,
	OBJTYPE_OLDTRAP,
	OBJTYPE_CONTAINER,
	OBJTYPE_NOTE,
	OBJTYPE_DRINK_CON,
	OBJTYPE_KEY,
	OBJTYPE_FOOD,
	OBJTYPE_MONEY,			/* 20 */
	OBJTYPE_PEN,
	OBJTYPE_BOAT,
	OBJTYPE_CORPSE_MOB,	/* da unire cone quello del pg */
	OBJTYPE_CORPSE_PG,
	OBJTYPE_FOUNTAIN,
	OBJTYPE_PILL,
	OBJTYPE_BLOOD,
	OBJTYPE_BLOODSTAIN,
	OBJTYPE_SCRAPS,
	OBJTYPE_PIPE,			/* 30 */
	OBJTYPE_HERB_CON,
	OBJTYPE_HERB,
	OBJTYPE_INCENSE,
	OBJTYPE_FIRE,
	OBJTYPE_BOOK,
	OBJTYPE_SWITCH,
	OBJTYPE_LEVER,
	OBJTYPE_PULLCHAIN,
	OBJTYPE_BUTTON,
	OBJTYPE_DIAL,			/* 40 */
	OBJTYPE_RUNE,
	OBJTYPE_RUNEPOUCH,
	OBJTYPE_MATCH,
	OBJTYPE_TRAP,
	OBJTYPE_MAP,
	OBJTYPE_PORTAL,
	OBJTYPE_PAPER,
	OBJTYPE_TINDER,
	OBJTYPE_LOCKPICK,
	OBJTYPE_SPIKE,			/* 50 */
	OBJTYPE_DISEASE,
	OBJTYPE_OIL,
	OBJTYPE_FUEL,
	OBJTYPE_EMPTY1,		/* (RR) aggiungere un check sul caricamento per questi due per controllare che non vi siano, oppure aspettare (CC) */
	OBJTYPE_EMPTY2,
	OBJTYPE_MISSILE_WEAPON,
	OBJTYPE_PROJECTILE,
	OBJTYPE_QUIVER,
	OBJTYPE_SHOVEL,
	OBJTYPE_SALVE,			/* 60 */
	OBJTYPE_COOK,
	OBJTYPE_KEYRING,
	OBJTYPE_ODOR,
	OBJTYPE_CHANCE,
	OBJTYPE_DRESS,			/* (TT) */
	OBJTYPE_STATUE,
	OBJTYPE_SHEATH,
#ifdef T2_CHESS
	OBJTYPE_CHESS,			/* per gli scacchi */
#endif
	/* 69 tipi di oggetti */
	MAX_OBJTYPE
} items_types;


/* Tipi di oggetti senza struttura per ora:
worn
note			(forse fare un generico foglio paper gestitocome mail, note, pergamena, etc etc)
boat			(farla oggetto da indossare, oppure mezzo in cui entrare, da pensare anche per le ship della wild)
pen
blood
bloostain		(cercare di unirlo con blood, non è meglio creare un oggetto pozza, e farlo variare come un liquido? magari contenitore liquido?)
scraps
pipe
herb
herbcon			(non è meglio gestire i contenitori tutti in maniera uguale? cioè questo qui per esempio si differenzia perché gestisce solo un certo tipo di oggetti, buona idea)
incense
fire			(questo è da fare bene, per gestire gli incendi)
book			(questo è da fare mega bene, forse associarlo al tipo paper? su cui scrivere? ma no, bho)
switch			(per questi 4 direi di farne uno solo con una struttura che li spiega un po')
lever
pullchain
button
dial
rune
runepouch		(stesso discorso del contenitore erbe sopra)
match
map				(forse questa non serve più)
portal			(forse questo non serve più, a meno che non serve per fare enter nell'oggetto, uhm sì, carino)
paper
tinder
lockpick
spike
disease			(questa no, deve stare nella struttura oggetto, è una specie di sintonia che l'oggetto si becca, però è il virus, che si può spostare quindi)
oil				(lanterne, quindi nella struttura delle luci un campo che indichi la quantità d'olio inseribile, e l'attuale (o forse uso le ore e calclo la quantità in base al peso dell'oggetto, sì ok))
fuel			(stessa cosa delle lampade ma per i mezzi delle wild, diversi tipi di fuel, magico, energetico, combustibile etc etc)
missile weapon	(questa è un'arma)
projectile		(però invece questo forse non si può gestirlo come un'arma, no direi proprio di no, meglio una struttura a parte con tanti valori per diversificare il tipo di proiettili)
quiver			(un altro contenitore a tema)
shover
cook			(questo tipo di oggetto si può togliere se inserito nel food)
keyring			(altro contenitore a tema mucho interessanto, devo ricordarmi che se uno fa put all nei contenitori a tema vengono inseriti tutti gli oggetti con il tipo, è utile fare dei contenitori per i reagenti direi)
odor			(no, questo bisogna spostarlo nella struttura dell'oggetto)
chance			(oggetto che serve a "schivare" la morte, si distrugge all'utilizzo)
dress			(da spostare)
statue			(da gestire meglio, dona gloria? ricordarsi delle statue di sacred, sono oggetti di culto? sono oggetti curati dai soldati? sono oggetti con il nome del pg? commemorativi?)
*/


/*
 * Elenco dei tipi di luce
 */
typedef enum
{
	LIGHT_FIRE,
	LIGHT_SUN,
	LIGHT_ELECTRICAL,
	LIGHT_MAGICAL,
	LIGHT_SACRED,
	MAX_LIGHT
} light_types;


/*
 * Valori per le flag dei contenitori
 */
typedef enum
{
	CONTAINER_CLOSEABLE,
	CONTAINER_PICKPROOF,
	CONTAINER_CLOSED,
	CONTAINER_LOCKED,
	CONTAINER_EATKEY,
	MAX_CONTAINER
} container_flags;


/* Lever/dial/switch/button/pullchain flags (FF) convertirle in ext */
typedef enum
{
	TRIG_UP,
	TRIG_UNLOCK,
	TRIG_LOCK,
	TRIG_D_NORTH,
	TRIG_D_SOUTH,
	TRIG_D_EAST,
	TRIG_D_WEST,
	TRIG_D_UP,
	TRIG_D_DOWN,
	TRIG_DOOR,
	TRIG_CONTAINER,
	TRIG_OPEN,
	TRIG_CLOSE,
	TRIG_PASSAGE,
	TRIG_OLOAD,
	TRIG_MLOAD,
	TRIG_TELEPORT,
	TRIG_TELEPORTALL,
	TRIG_TELEPORTPLUS,
	TRIG_DEATH,
	TRIG_CAST,
	TRIG_FAKEBLADE,
	TRIG_RAND4,
	TRIG_RAND6,
	TRIG_TRAPDOOR,
	TRIG_ANOTHEROOM,
	TRIG_USEDIAL,
	TRIG_ABSOLUTEVNUM,
	TRIG_SHOWROOMDESC,
	TRIG_AUTORETURN,
	/* 30 tipi di trigger-flag per leve e simili */
	MAX_TRIG
} trig_flags;


typedef enum
{
	TRAPFLAG_ROOM,
	TRAPFLAG_OBJ,
	TRAPFLAG_ENTER_ROOM,
	TRAPFLAG_LEAVE_ROOM,
	TRAPFLAG_OPEN,
	TRAPFLAG_CLOSE,
	TRAPFLAG_GET,
	TRAPFLAG_PUT,
	TRAPFLAG_PICK,
	TRAPFLAG_UNLOCK,
	TRAPFLAG_N,
	TRAPFLAG_S,
	TRAPFLAG_E,
	TRAPFLAG_W,
	TRAPFLAG_U,
	TRAPFLAG_D,
	TRAPFLAG_EXAMINE,
	TRAPFLAG_NE,
	TRAPFLAG_NW,
	TRAPFLAG_SE,
	TRAPFLAG_SW,
	/* 21 flag di trappole */
	MAX_TRAPFLAG
} trap_flags;


/*
 * Struttura per la tabella dei tipi di oggetto
 * (FF) da fare e finire ovviamente
 */
struct values_type
{
	char	*name;
};


/*
 * Struttura di una luce
 */
struct values_light_data
{
	int		 intensity;			/* Intensità delle luce, se 0 è spenta, se negativa produce ombra */
	int		 type;				/* Tipo di luce, serve soprattutto per i vampiri ed altre creature dell'ombra */
	int		 hours;				/* Tempo potenziale di utilizzo della luce, value[2] */
	char	*act_light;			/* Messaggio di quando la luce viene attivata, se accendibile */
	char	*act_light_room;
	char	*act_dark;			/* Messaggio di quando la luce si spegne, se spegnibile */
	char	*act_dark_room;
	char	*act_consume;		/* Messaggio di quando la luce si consuma del tutto */
	char	*act_consume_room;
};


/*
 * Struttura di uno scroll (che volendo può essera anche una nota),
 *	potion (solitamente ha anche un drinkcon)
 *	e pill (solitamente ha anche il food)
 */
struct values_scroll_data
{
	int		 level;			/* value[0] */
	int		 sn1;			/* value[1] */
	int		 sn2;			/* value[2] */
	int		 sn3;			/* value[3] */
	char	*act_use;		/* Messaggio di recite/quaff/eat dello scroll/potion/pill */
	char	*act_use_room;
};


/*
 * Struttura di una wand o di uno staff
 */
struct values_wand_data
{
	int		 level;				/* value[0] */
	int		 max_charges;		/* value[1] */
	int		 curr_charges;		/* value[2] */
	int		 sn;				/* value[3] */
	char	*act_zap;			/* Messaggio di utilizzo della wand/staff */
	char	*act_zap_room;
	char	*act_consume;		/* Messaggio di quando la wand/staffè consumata del tutto e non può essere più utilizzata */
	char	*act_consume_room;
	/* (FF) Aggiungere una variabile di deposito mana per incanalare-canalizzare mana per spell? ma forse se la cava già così, volendo si può consumare uno spell della wand per avere un po' di mana */
};


/*
 * Struttura di una weapon, fireweapon penso anche, e missile?
 */
struct values_weapon_data
{
	int		 condition;			/* value[0] */
	int		 dice_number;		/* value[1]		v1 and v2 are optional and will be */
	int		 dice_size;			/* value[2]		autogenerated if set to 0 */
	int		 type;				/* ascia, spadone, arco, etc etc */
	int		 damage1;			/* value[3] */
	int		 damage2;			/* Danno secondario dell'arma */
	int		 range;				/* Value per le armi a distanza value[4] (GG) */
	int		 lvl_disarm;		/* Livelli di chi ha disarmato	value[5] (GG) */
	char	*act_wield;			/* Messaggio di quando viene impugnata */
	char	*act_wield_room;	/* Messaggio di quando viene impugnata */
};


/*
 * Struttura di un treasure
 * (CC) probabilmente da togliere, non ha struttura anche se il tipo esisterà
 *	assieme magari al tipo gioiello? non so
 */
struct values_treasure_data
{
	int		type;		/* value[0] */
	int		condition;	/* value[1] */
};


/*
 * Struttura di un'armatura (FF) e forse anche dei dress, da controllare
 */
struct values_armor_data
{
	int		 ac;			/* value[0]		AC adjust/current condition */
	int		 ac_orig;		/* value[1]		Original AC */
	int		 type;			/* tipo di armatura, se guanto, elmo, etc etc, si trova qui visto che non sarà più indicativo del luogo ove indossarlo, sarà più free, difatti si potranno mettere nelle varie parti del corpo */
	char	*act_wear;		/* Messaggio di quando viene indossata */
	char	*act_wear_room;	/* Messaggio di quando viene indossata */
};


/*
 * (FF) Struttura di un furniture
 */
struct values_furniture_data
{
	int		 regain[MAX_POINTS];	/* vedere lo snippet di Xerves */
	/* (FF) aggiungerci dei messaggi di sit, stand etc etc? */
};


/*
 * Struttura di un container (GG) e forse anche di un keyring e di un quiver, pipe e herbcon (herbcon però mi sa che si comporta come un drinkcon.. ma che casino quel fill) (occhio a corpse che non è un container ma si comporta come tale)
 * (FF) inglobare container in drink container
 */
struct values_container_data
{
	int		 capacity;		/* value[0]	Comprensivo di tara, cioè del peso del contenitore, (CC) forse nelle aree metterò senza tara, e poi lo calcolo a caricamento, oppure ricontrollo il codice per vedere questa strana cosa */
	int		 key;			/* value[2]	vnum */
	int		 durability;	/* value[3]	che cambierei con la condizione (FF) */
	int		 type;			/* Tipo di oggetto di default che può essere inserito nel contenitore, per creare quiver, runepouch, herbcon etc etc (GG) */
	int		 flags;			/* value[1]	1 closeable, 2 pickproof, 4 closed, 8 locked */
	char	*act_open;		/* messaggio di quando il contenitore viene aperto */
	char	*act_open_room;
	char	*act_close;		/* messaggio di quando il contenitore viene chiuso */
	char	*act_close_room;
};


/*
 * Struttura di un drink containern (TT) pare che blood sia un drink container
 */
struct values_drinkcon_data
{
	int		 capacity;			/* value[0]	*/
	int		 curr_quant;		/* value[1]	*/
	int		 liquid;			/* value[2]	*/
	int		 poison;			/* value[3]	if non-zero, drink is poisoned, indica anche la durata dell'affect per il veleno */
	bool	 fountain;			/* (FF) Se TRUE è una fontana, serve a togliere il tipo di oggetto fountain, le fontane devono essere fisse al terreno, è meglio */
	char	*act_drink;			/* Messaggio di quando il contenitore viene bevuto */
	char	*act_drink_room;
};


/*
 * Struttura di una key
 */
struct values_key_data
{
	bool	 save;				/* Se TRUE salva la chiave al pg */
	char	*act_unlock;		/* Messaggio di lock */
	char	*act_unlock_room;	/* Messaggio di lock */
	char	*act_lock;			/* Messaggio di unlock */
	char	*act_lock_room;		/* Messaggio di unlock */
};


/*
 * Struttura di un food
 */
struct values_food_data
{
	int		 hours;			/* value[0]	*/
	int		 condition;		/* value[1]	(FF) da spostare (TT) */
	int		 dam_cook;		/* value[2]	*/
	int		 poison;		/* value[3]	if non-zero, food is poisoned */
	int		 init_cond;		/* value[4] opzionale (GG) */
	int		 flags;			/* (FF) mettere qui la parte del cooked-burned togliendo così una tipologia di oggetto */
	char	*act_eat;		/* Messaggio di quando il pg mangia il cibo */
	char	*act_eat_room;
};


/*
 * Struttura di una money
 */
struct values_money_data
{
	int		qnt;			/* value[0] */
	int		qnt_recycle;	/* (FF) Quantità da riciclare dentro quelle di qnt, se non vengono riciclat si può essere sgamati come ladri */
	int		type;			/* tipo di moneta, argento, oro... etc etc */
};


/*
 * Struttura di una penna pe scrivere una nota (GG)
 */
struct values_pen_data
{
	int		 ink;			/* Quantità d'inchiostro	value[0] */
	char	*act_write;		/* Messaggio di quando uno utilizza la penna per scrivere */
	char	*act_write_room;
};


/*
 * Struttura di un Boat (GG)
 */
struct values_boat_data
{
	int		 rooms;				/* Numero di viaggi-room che può sostenere prima che sia inutilizzabile, -1 infinito */
	int		 type;				/* tipo di boat, se sopra l'acqua, o sotto per nuotare */
	char	*act_travel;		/* Messaggio di navigazione, cioè come il pg naviga con la boat, se è vuoto non indica che il pg abbia una boat */
	char	*act_travel_room;
};


/*
 * Struttura di un corpse_ncp o corpse_pc (GG)
 */
struct values_corpse_data
{
	int		time;	/* Tempo di decomposizione per pg, per il mob è il vnum value[2] (FF) da pensare bene */
	int		tick;	/* tick per i killer					value[3] (FF) questo devo cambiarlo in vnum mi sa */
	int		level;	/* livello, per lo scuoia				value[4] */
	int		looted;	/* quante volte è stato saccheggiato	value[5] */
	/* (FF) Informazioni per creare degli zombie automaticamente oppure per registrare le ferite sul cadavare, e chi le ha inflitte, con che arma, razza che lo ha fatto, etc etc */
	bool	pg;		/* Il cadavere è quello di un pg (quindi CORPSE_PC non ci saranno più) */
};


/*
 * Struttura di una pipa (GG)
 */
struct values_pipe_data
{
	int		 capacity;		/* Capacità della pipa	value[0] (CC) questa deve essere tolta, verrà usata la struttura container per usare le pipe */
	int		 draws;			/* Numero delle tirate	value[1] */
	int		 herb;			/* Sn dell'erba			value[2] */
	int		 flags;			/* Flag della pipa		value[3] */
	char	*act_smoke;		/* Messaggio di quando il pg fuma la pipa */
	char	*act_smoke_room;
};


/*
 * Struttura di un'erba (GG)
 * (FF) futura struttura per i reagenti di un alchimista
 */
struct values_herb_data
{
	int		charges;	/* value[1] */
	int		hnum;		/* value[2] */
};


/*
 * Struttura di una lever, switch, button, chain (GG)
 * (FF) appena compreso meglio il funzionamento di queste leve allora cambiare i nomi in qualcosa di più friendly, per ora li ho messi sulla base dei value di herne
 */
struct values_lever_data
{
	int		 flags;			/* Flag della leva									value[0] */
	int 	 vnum_sn;		/* vnum della stanza o uscita di partenza (TT) o sn	value[1] */
	int		 vnum;			/* vnum della stanza o uscita di arrivo				value[2] */
	int		 vnum_val;		/* vnum della leva o valore							value[3] */
	char	*act_use;		/* messaggio di utilizzo della leva (o di un bottone in on) */
	char	*act_use_room;
};


/*
 * Struttura di una trap, e anche oldtrap, forse da convertire
 */
struct values_trap_data
{
	int		 charges;			/* value[0]	*/
	int		 type;				/* value[1]	*/
	int		 level;				/* value[2]	*/
	int		 flags;				/* value[3]	*/
	char	*act_activate;		/* Messaggio di attivazione della trappola */
	char	*act_activate_room;
};


/*
 * Struttura di un pezzo di carta per scrivere le note (FF) o per scroll d'incantesimi (GG)
 */
struct values_paper_data
{
	int		text;		/* Testo della nota		value[0] */
	int		subject;	/* Soggetto della nota	value[1] */
	int		to;			/* A chi inviarla		value[2] */
	int		type;		/* Tipo di nota, 0 nessuna, 1 nota, 2 lettera */
};


/*
 * Struttura di un salve (GG)
 * Forse mancano dei valori, dare una occhiata al sito herne che ha altri valori interessanti (TT)
 */
struct values_salve_data
{
	int		 level;			/* value[0] */
	int		 charges;		/* value[1] */
	int		 max_charges;	/* value[2] */
	int		 delay;			/* value[3] in herne qui sarebbe delay (TT) infatti per or l'ho cambiato da level in delay */
	int		 sn1;			/* value[4] */
	int		 sn2;			/* value[5] */
	char	*act_apply;		/* Messaggio di utilizzo del salve */
	char	*act_apply_room;
};


/*
 * Struttura di un fodero
 */
struct values_sheath_data
{
	int		 weapon_type;	/* Tipo dell'arma che conterrebbe il fodero */
	int		 vnum;			/* Vnum dell'oggetto personale del fodero */
	int		 flags;			/* Arma sfoderabile solo di gg e/o solo di notte, arma che contiene la luce delle armi e altro */
	char	*act_open;		/* Messaggio di sfodero */
	char	*act_open_room;
	char	*act_close;		/* Messaggio di rinfodero */
	char	*act_close_room;
};


/*
 * Variabili
 */
/*extern	const struct values_type	table_itemtype[MAX_ITEM_TYPE];*/
extern	const CODE_DATA	code_objtype[];
extern	const CODE_DATA	code_light[];
extern	const CODE_DATA	code_container[];
extern	const CODE_DATA	code_trig[];
extern	const CODE_DATA	code_trapflag[];
extern	const int		max_check_objtype;
extern	const int		max_check_light;
extern	const int		max_check_container;
extern	const int		max_check_trig;
extern	const int		max_check_trapflag;

/*
 * Funzioni
 */
void	load_itemtype		( void );
void	init_values			( OBJ_DATA *obj, const int type );
void	free_values			( OBJ_DATA *obj, const int type );
void	fread_values_area	( AREA_DATA *tarea, MUD_FILE *fp, OBJ_DATA *obj );
void	fread_values		( MUD_FILE *fp, OBJ_DATA *obj );
void	fwrite_values		( MUD_FILE *fp, OBJ_DATA *obj );
void	copy_values			( OBJ_DATA *source, OBJ_DATA *dest );
bool	compare_values		( OBJ_DATA *obj1, OBJ_DATA *obj2, int type );
void	show_values			( CHAR_DATA *ch, OBJ_DATA *obj );


#endif	/* VALUES_H */
