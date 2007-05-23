#ifndef CODE_H
#define CODE_H

/*
 * Enumerazioni delle varie tipologie di codici
 */
typedef enum
{
	CODE_AFFECT,
	CODE_APPLY,
	CODE_AREAFLAG,
	CODE_ATTACK,
	CODE_ATTR,
	CODE_CHANNEL,
	CODE_CLANFLAG,
	CODE_CLANTYPE,
	CODE_CLASS,
	CODE_CLASSFLAG,
	CODE_CMDFLAG,
	CODE_CMDTYPE,
	CODE_CODE,
	CODE_COLOR,
	CODE_COLORHAIR,
	CODE_CONDITION,
	CODE_CONNECTED,
	CODE_CONTAINER,
	CODE_DAMAGE,
	CODE_DEFENSE,
	CODE_DIR,
	CODE_ENFLAG,
	CODE_EYE,
	CODE_EXIT,
	CODE_HAIRTYPE,
	CODE_HAIRSTYLE,
	CODE_HELPTYPE,
	CODE_INTENT,
	CODE_LANGUAGE,
	CODE_LIGHT,
	CODE_LOG,
	CODE_MOB,
	CODE_MPTRIGGER,
	CODE_NEWS,
	CODE_NOTE,
	CODE_OBJFLAG,
	CODE_OBJMAGIC,
	CODE_OBJTYPE,
	CODE_OBJWEAR,
	CODE_PART,
	CODE_PIPE,
	CODE_PLAYER,
	CODE_POINTS,
	CODE_POSITION,
	CODE_PROTOCOL,
	CODE_PULL,
	CODE_RACE,
	CODE_RACEFLAG,
	CODE_RIS,
	CODE_ROOM,
	CODE_SAVETHROW,
	CODE_SECTOR,
	CODE_SENSE,
	CODE_SEX,
	CODE_SKELL,
	CODE_SKILLTYPE,
	CODE_SPELLACTION,
	CODE_SPELLDAMAGE,
	CODE_SPELLPOWER,
	CODE_SPELLSAVE,
	CODE_SPELLSPHERE,
	CODE_STYLE,
	CODE_TARGET,
	CODE_TRAPFLAG,
	CODE_TRIG,
	CODE_TRUST,
	CODE_VOTE,
	CODE_WEAPON,
	CODE_WEARFLAG,
	CODE_WEARLOC,
	MAX_CODE
} table_code_types;


/*
 * Utilizzato per semplificare l'accesso alle tabelle di codici
 * Queste ultime servono per rendere i file aree e dati vari più friendly all'occhio
 *	invece che pieni di numeri che solo gli area builder più esperti riuscirebbero a decifrare.
 * Inoltre rendono le aree meno soggette a cambiamenti ed a conversioni
 * Di contro c'è un tempo maggiore nel caricamento e salvataggio delle aree e di altri file,
 *	e un maggiore spazio occupato sul disco
 */
struct code_data
{
	int	  id;
	char *code;
	char *name;
};


/*
 * Struttura della tabella che contiene tutte le informazioni sulle tabelle dei codici
 */
struct table_code_type
{
	CODE_DATA		 code;
	const CODE_DATA	*table;
	const int		 max;
};


/*
 * Tabelle dei codici globali
 */
extern	const struct table_code_type table_code[];
extern	const CODE_DATA				 code_areaflag[];
extern	const CODE_DATA				 code_attr[];
extern	const CODE_DATA				 code_condition[];
extern	const CODE_DATA				 code_mob[];
extern	const CODE_DATA				 code_points[];
extern	const CODE_DATA				 code_position[];
extern	const CODE_DATA				 code_savethrow[];
extern	const CODE_DATA				 code_sense[];
extern	const CODE_DATA				 code_sex[];
extern	const CODE_DATA				 code_style[];
extern	const CODE_DATA				 code_target[];
extern	const CODE_DATA				 code_trust[];
extern	const CODE_DATA				 code_wearflag[];
extern	const CODE_DATA				 code_wearloc[];
extern	const int					 max_check_areaflag;
extern	const int					 max_check_attr;
extern	const int					 max_check_condition;
extern	const int					 max_check_mob;
extern	const int					 max_check_points;
extern	const int					 max_check_position;
extern	const int					 max_check_savethrow;
extern	const int					 max_check_sense;
extern	const int					 max_check_sex;
extern	const int					 max_check_style;
extern	const int					 max_check_target;
extern	const int					 max_check_trust;
extern	const int					 max_check_wearflag;
extern	const int					 max_check_wearloc;


/*
 * Funzioni globali
 */
int			code_max		 ( const int type );
int			code_code		 ( MUD_FILE *fp, const char *string, const int type );
int			code_num		 ( MUD_FILE *fp, const char *string, const int type );
char	   *code_str		 ( MUD_FILE *fp, int number, const int type );
char	   *code_name		 ( MUD_FILE *fp, int number, const int type );
char	   *code_bit		 ( MUD_FILE *fp, BITVECTOR *bitvector, const int type );
char	   *code_bitint		 ( MUD_FILE *fp, int bitvector, const int type );
char	   *code_all		 ( const int table, bool column );
void		check_code_table	 ( void );
BITVECTOR  *fread_code_bit	 ( MUD_FILE *fp, const int type );
int			fread_code_bitint( MUD_FILE *fp, const int type );


#endif	/* CODE_H */
