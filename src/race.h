#ifndef RACE_H
#define RACE_H


/****************************************************************************\
 >							Header per la Gestione Razze					<
\****************************************************************************/


/*
 * Tipo di struttura Race
 */
typedef struct	race_data		RACE_DATA;


/* Lunghezza max per il nome di una Razza */
#define MAX_LEN_NAME_RACE	14


/*
 * Enumerazione delle Razze
 */
typedef enum
{
	RACE_NONE = -1,
	RACE_HUMAN,
	RACE_ELF,
	RACE_DWARF,
	RACE_HALFLING,
	RACE_PIXIE,
	RACE_VAMPIRE,
	RACE_HALFOGRE,		/* orchi */
	RACE_HALFORC,		/* goblin-tolkeniano-hobbitosi*/
	RACE_HALFTROLL,
	RACE_HALFELF,
	RACE_GITH,
	RACE_DROW,
	RACE_SEAELF,
	RACE_LIZARDMAN,		/* Thepa */
	RACE_GNOME,
	RACE_R5,
	RACE_R6,
	RACE_R7,
	RACE_R8,
	RACE_TROLL,
	RACE_ANT,
	RACE_APE,
	RACE_BABOON,
	RACE_BAT,
	RACE_BEAR,
	RACE_BEE,
	RACE_BEETLE,
	RACE_BOAR,
	RACE_BUGBEAR,
	RACE_CAT,
	RACE_DOG,
	RACE_DRAGON,		/* 31, forse deve rimanere come tale, vedere define nello smaug originale (TT) */
	RACE_FERRET,
	RACE_FLY,
	RACE_GARGOYLE,
	RACE_GELATIN,
	RACE_GHOUL,
	RACE_GNOLL,
	RACE_GNOME_2,
	RACE_GOBLIN,
	RACE_GOLEM,
	RACE_GORGON,
	RACE_HARPY,
	RACE_HOBGOBLIN,
	RACE_KOBOLD,
	RACE_LIZARDMAN_2,
	RACE_LOCUST,
	RACE_LYCANTHROPE,
	RACE_MINOTAUR,
	RACE_MOLD,
	RACE_MULE,
	RACE_NEANDERTHAL,
	RACE_OOZE,
	RACE_ORC,
	RACE_RAT,
	RACE_RUSTMONSTER,
	RACE_SHADOW,
	RACE_SHAPESHIFTER,
	RACE_SHREW,
	RACE_SHRIEKER,
	RACE_SKELETON,
	RACE_SLIME,
	RACE_SNAKE,
	RACE_SPIDER,
	RACE_STIRGE,
	RACE_THOUL,
	RACE_TROGLODYTE,
	RACE_UNDEAD,
	RACE_WIGHT,
	RACE_WOLF,
	RACE_WORM,
	RACE_ZOMBIE,
	RACE_BOVINE,
	RACE_CANINE,
	RACE_FELINE,
	RACE_PORCINE,
	RACE_MAMMAL,
	RACE_RODENT,
	RACE_BIRD,
	RACE_REPTILE,
	RACE_AMPHIBIAN,
	RACE_FISH,
	RACE_CRUSTACEAN,
	RACE_INSECT,
	RACE_SPIRIT,
	RACE_MAGICAL,
	RACE_HORSE,
	RACE_ANIMAL,
	RACE_HUMANOID,
	RACE_MONSTER,
	RACE_GOD,
	RACE_FELAR,
	RACE_ARIAL,
	RACE_DEEPGNOME,
	/* razze miciko */
	RACE_DEMON,
	RACE_SONGBIRD,
	RACE_EAGLE,
	RACE_GHOST,
	RACE_CENTIPEDE,
	RACE_DOLL,				/* (CC) da togliere */
	RACE_WATER_ELEMENTAL,	/* da accorpare le elementali in un unica razza, poi ci pensano gli immune e simili a gestire il tipo */
	RACE_RABBIT,
	RACE_AIR_ELEMENTAL,
	RACE_EARTH_ELEMENTAL,
	RACE_FIRE_ELEMENTAL,
	RACE_FOX,
	RACE_DRACONIAN,
	RACE_SATYR,
	RACE_MINDFLAYER,		/* e illithid */
	MAX_RACE
} races_types;


/*
 * Tipi di flag razziali
 */
typedef enum
{
	RACEFLAG_PLAYABLE,
	RACEFLAG_NOMONEY,
	MAX_RACEFLAG
} race_flags;


/*
 * Struttura di una razza
 */
struct race_data
{
	char	   *name_male;						/* Nome della razza, maschile */
	char	   *name_fema;						/*	e femminile */
	SYNO_DATA  *color;							/* Colore della razza */
	BITVECTOR  *flags;							/* Flags di razza */
	int			attr_plus [MAX_ATTR];			/* Modificatore agli attributi */
	int			sense_plus[MAX_SENSE];			/* Modificatore ai sensi */
	int			points_limit[MAX_POINTS];		/* Bonus/malus dei punteggi */
	BITVECTOR  *affected;						/* Default affect bitvectors */
	BITVECTOR  *resist;
	BITVECTOR  *suscept;
	BITVECTOR  *lang_natural;					/* Lingue razziali naturali */
	BITVECTOR  *lang_learnable;					/* Lingue apprendibili */
	int			ac_plus;
	BITVECTOR  *attacks;
	BITVECTOR  *defenses;
	int			exp_multiplier;
	int			align_start;					/* Allineamento di partenza */
	int			min_align;						/* Allineamento minimo  per questa razza */
	int			max_align;						/* Allineamento massimo per questa razza */
	int			height;
	int			weight;
	char	   *tonality_sound;					/* Verso o voce che si sente da lontano quando la razza parla (GG) */
	int			tonality;						/* Potenza vocale (GG) */
	int			hunger_mod;
	int			thirst_mod;
	int			saving_throw[MAX_SAVETHROW];	/* (GG) forse non era codato prima */
	char	   *wear_name[MAX_WEARLOC];
};


/*
 * Variabili
 */
extern	RACE_DATA		*table_race[MAX_RACE];
extern	int			 	 top_race_play;
extern	const CODE_DATA	 code_race[MAX_RACE];
extern	const CODE_DATA	 code_raceflag[];
extern	const int		 max_check_race;
extern	const int		 max_check_raceflag;


/*
 * Funzioni
 */
void		load_races		( void );
void		free_all_races	( void );
char	   *get_race		( CHAR_DATA *ch, bool color );
int			get_race_num	( const char *type, bool exact );
RACE_DATA  *is_name_race	( const char *argument, bool exact );


#endif	/* RACE_H */
