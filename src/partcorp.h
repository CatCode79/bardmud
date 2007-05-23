#ifndef PARTCORP_H
#define PARTCORP_H


/****************************************************************************\
 >				File Header riguardante le parti del corpo					<
\****************************************************************************/


/*
 * Enumerazione dei tipi di capelli.
 */
typedef enum
{
	HAIRTYPE_SLEEK,			/* lisci		*/
	HAIRTYPE_WAVY,			/* mossi		*/
	HAIRTYPE_WAVY2,			/* ondulati		*/
	HAIRTYPE_CURLY,			/* ricci		*/
	HAIRTYPE_FRIZZY,		/* crespi		*/
	HAIRTYPE_RUFFLED,		/* arruffati	*/
	HAIRTYPE_BALD,			/* calvo		*/
	MAX_HAIRTYPE
} hairs_types;


/*
 * Enumerazione delle acconciature e tagli dei capelli.
 * L'acconciatura "rasato" viene gestita dalla lunghezza dei capelli.
 */
typedef enum
{
	HAIRSTYLE_NONE,			/* nessuno		*/
	HAIRSTYLE_LOOSE,		/* sciolti		*/
	HAIRSTYLE_PONYTAIL,		/* coda			*/
	HAIRSTYLE_CHIGNON,		/* chignon		*/
	HAIRSTYLE_FORELOCK,		/* ciuffo		*/
	HAIRSTYLE_BANG,			/* frangetta	*/
	HAIRSTYLE_BOB,			/* caschetto	*/
	HAIRSTYLE_PLAIT,		/* treccia		*/
	MAX_HAIRSTYLE
} hairstyles_types;


/*
 * Enumerazione dei tipi di occhi
 */
typedef enum
{
	EYE_NORMAL,
	EYE_NOPUPIL,		/* Senza pupilla */
	EYE_MIRROR,			/* occhi che riflettono immagini */
	EYE_SERPENT,		/* pupilla tipo serpente */
	MAX_EYE
} eyes_types;


#if 0
/* (FF) */
/*
 * Enumerazione dei tipi di naso.
 */
typedef enum
{
	NOSE_,
	MAX_NOSE
} noses_types;


/*
 * Enumerazione dei tipi di bocca.
 */
typedef enum
{
	MOUTH_,
	MAX_MOUTH
} mouthes_types;


/*
 * Enumerazione dei tipi di orecchie.
 */
typedef enum
{
	EAR_,
	MAX_EAR
} ears_types;
#endif


/*
 * Enumerazione dei tipi di pelle.
 */
typedef enum
{
	SKIN_SMOOTH,	/* liscia			*/
	SKIN_ROUGH,		/* ruvida			*/
	SKIN_WRINKL,	/* rugosa			*/
	SKIN_SCALE,		/* scuamosa			*/
	SKIN_DRY,		/* secca			*/
	SKIN_SOFT,		/* morbida			*/
	SKIN_VELVET,	/* vellutata		*/
	SKIN_HAIR,		/* folta di peli	*/
	SKIN_THICK,		/* spessa			*/
	SKIN_CHITIN,	/* chitinosa		*/
	SKIN_SLIMY,		/* viscida			*/
	SKIN_FEATHER,	/* piumata			*/
	SKIN_COAT,		/* pelo animale		*/
	SKIN_FUR,		/* pelliccia		*/
	MAX_SKIN
} skins_types;


/*
 * Enumerazione dei tipi di corporatura.
 * (FF) dare la scelta nella nanny e/o poi in game aumentare e diminuire a seconda del movimento del pg
	gracile = malus in forza, nessun bonus in agilità.
	magro =
	longilineo = bonus in agilità, minore in forza.
	normale = default.
	robusto = bonus in forza, malus in agilità.
	grasso =
	obeso = malus in agilità, nessun bonus in forza.
 */
typedef enum
{
	BODY_FRAIL,		/* gracile	*/
	BODY_THIN,		/* magro	*/
	BODY_SLIM,		/* snello	*/
	BODY_NORMAL,	/* normale	*/
	BODY_ROBUSTE,	/* robusto	*/
	BODY_FAT,		/* grasso	*/
	BODY_OBESE,		/* obeso	*/
	MAX_BODY
} body_types;



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *
 *							Strutture delle parti del corpo.				   *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/*
 * Struttura per la tabella relativa alle parti del corpo
 */
struct part_type
{
	CODE_DATA	 code;
	int			 vnum;
	char		*msg1;
	char		*msg2;
};


/*
 * Struttura dati dei capelli. (FF) implementare la eterocromia? al limite farla una malattia (assieme all'albinismo e alla canizia precoce magari).
 */
struct hair_part
{
	SYNO_DATA	*color;			/* il colore dei capelli */
	SYNO_DATA	*dye;			/* il colore della tinta, se -1 possiede il colore naturale */
	SYNO_DATA	*type;			/* Tipo di capelli, se -1 significa che non ha i capelli (prob per razza) */
	SYNO_DATA	*style;			/* acconciatura */
	int			 length;		/* lunghezza dei capelli */
};


/*
 * Struttura dati degli occhi.
 */
struct eye_part
{
	SYNO_DATA	*color;			/* Colore dell'iride */
	SYNO_DATA	*pupil_color;	/* Colore della pupilla */
	SYNO_DATA	*bright_color;	/* Colore luce degli occhi, utilizzato per incantesimi sopratutto, se -1 non sono illuminati */
	SYNO_DATA	*type;			/* Tipo di occhio, -1 sono normali, se non ci sono ci si affida alla parte del corpo  */
};


/*
 * Body parts
 * (RR) Da tenere fino alla conversione., poi utilizzare quelle sotto.
 */
typedef enum
{
	PART_HEAD,
	PART_ARMS,
	PART_LEGS,
	PART_HEART,
	PART_BRAINS,
	PART_GUTS,
	PART_HANDS,
	PART_FEET,
	PART_FINGERS,
	PART_EAR,
	PART_EYE,
	PART_LONG_TONGUE,
	PART_EYESTALKS,
	PART_TENTACLES,
	PART_FINS,
	PART_WINGS,
	PART_TAIL,
	PART_SCALES,		/* 18 */
	/* for combat */
	PART_CLAWS,
	PART_FANGS,
	PART_HORNS,
	PART_TUSKS,
	PART_TAILATTACK,
	PART_SHARPSCALES,
	PART_BEAK,

	PART_HAUNCH,
	PART_HOOVES,
	PART_PAWS,
	PART_FORELEGS,
	PART_FEATHERS,		/* 30 */
	MAX_PART
} body_part_types;


/*
 * Variabili globali
 */
extern	const struct part_type	table_part[];
extern	const CODE_DATA			code_eye[];
extern	const CODE_DATA			code_hairtype[];
extern	const CODE_DATA			code_hairstyle[];
extern	const int				max_check_part;
extern	const int				max_check_eye;
extern	const int				max_check_hairtype;
extern	const int				max_check_hairstyle;


/*
 * Funzioni
 */
bool		HAS_BODYPART	( CHAR_DATA *ch, int part );
EYE_PART   *init_eye		( PG_DATA *pg );
HAIR_PART  *init_hair		( PG_DATA *pg );
EYE_PART   *fread_eye		( MUD_FILE *fp );
void		fwrite_eye		( EYE_PART *eye, MUD_FILE *fp );
HAIR_PART  *fread_hair		( MUD_FILE *fp );
void		fwrite_hair		( HAIR_PART *hair, MUD_FILE *fp );
void		free_eye		( EYE_PART *eye );
void		free_hair		( HAIR_PART *hair );
char	   *score_partcorp	( CHAR_DATA *ch, int line );
void		update_hair		( void );


#endif	/* PARTCORP_H */
