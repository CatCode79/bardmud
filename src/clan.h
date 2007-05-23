#ifndef CLAN_H
#define CLAN_H


/****************************************************************************\
 >							Header clan module								<
\****************************************************************************/

/*
 * Tipi di strutture
 */
typedef struct	rank_data	RANK_DATA;


/*
 * Definizioni
 */
#define CLAN_DIR	"../clans/"				/* Dir dati dei clan */
#define CLAN_LIST	CLAN_DIR	"clan.lst"	/* Lista Clan */

#define MAX_RANK	7		/* Massimo rank caricabile in un clan */
#define MAX_CLAN	4		/* Limite di Clan caricabili */


/*
 * Elenco dei tipi di Clan
 */
typedef enum
{
	CLANTYPE_CLAN,		/* clan	*/
	CLANTYPE_GUILD,		/* gilda, solitamente per motivi di profitto o di protezione a dei diritti	*/
	CLANTYPE_SECT,		/* setta, solitamente un gruppo ristretto */
	CLANTYPE_CABALA,	/* cabala */
	CLANTYPE_GROUP,		/* gruppo di persone riunite */
	CLANTYPE_FACTION,	/* fazione politica */
	CLANTYPE_WAR,		/* per affrontare delle guerre */
	MAX_CLANTYPE
} clan_types;


/*
 * Flag di Clan
 */
typedef enum
{
	CLANFLAG_SECRET,	/* Indica se il clan è segreto */
	CLANFLAG_NOSCORE,	/* Indica che il clan non può creare un punteggio score */
	CLANFLAG_NOFLEE,	/* Indica che i membri del clan non possano fuggire */
	CLANFLAG_NOOBJECT,	/* (RR) Indica che il Clan non possiede un proprio oggetto */
	MAX_CLANFLAG
} clan_flags;


/*
 * Struttura di un rank
 * (FF) sarebbe bello gestire i permessi dei singoli ranghi alla ogame
 */
struct rank_data
{
	char		*name;			/* Nome del rank */
	int			 max_members;	/* Massimo di membri per questo rank */
	BITVECTOR	*classes;		/* Classi accettate per questo rank */
	BITVECTOR	*races;			/* Razze accettate per questo rank */
	int			 align_min;		/* Allineamento minimo per accedere a questo rank */
	int			 align_max;		/* Allineamento massimo per accedere a questo rank */
};


/*
 * Struttura per il salvataggio delle informazioni dei membri
 */
struct member_data
{
	MEMBER_DATA	*next;
	MEMBER_DATA	*prev;
	char		*name;			/* nome del membro del clan */
	int			 rank;			/* rango del membro del clan */
	char		*inducted_by;	/* nome di chi lo ha arruolato */
	time_t		 inducted_date;	/* data di quando è stato arruolato */
};


/*
 * Struttura di un Clan
 */
struct clan_data
{
	CLAN_DATA		*next;				/* Clan sucessivo nella lista				*/
	CLAN_DATA		*prev;				/* Clan precedente nella lista				*/
	MOB_PROTO_DATA	*guard1;			/* Mob che fa la guardia	(FF) forse non servono, meglio usare una falg di mob che indichi che il mob è la guardia del clan, però così dovrei anche indicare quale, uhm da pensare */
	MOB_PROTO_DATA	*guard2;			/* Mob che fa la guardia	(FF) forse non servono */
	OBJ_PROTO_DATA	*board;				/* Bacheca del clan 						*/
	OBJ_PROTO_DATA	*object;			/* Oggetto del clan 						*/
	OBJ_PROTO_DATA	*badge;				/* Oggetto che fa da stemma					*/
	ROOM_DATA		*recall;			/* Stanza che fa da recall					*/
	char			*name;				/* Nome del clan							*/
	char			*short_descr;		/* Descrizione breve del clan				*/
	char			*description;		/* Descrizione del clan						*/
	char			*motto;				/* Motto del clan							*/
	char			*msg_reunion;		/* Messaggio di riunione clan				*/
	char			*msg_alarm;			/* Messaggio di allarme clan				*/
	char			*msg_obj_lost;		/* Messaggio dell'oggetto clan perduto		*/
	char			*msg_obj_aquired;	/* Messaggio dell'oggetto clan acquisito	*/
	int				 type;				/* Tipo di clan CLAN_						*/
	BITVECTOR		*flags;				/* Flag del clan							*/
	int				 pkills;			/* Numero di pkills effettuati				*/
	int				 pdeaths;			/* Numero di pkills subiti					*/
	int				 mkills;			/* Numero di mobkill effettuati dentro un clan (RR) Toglierei */
	int				 mdeaths;			/* Numero of mobkill subiti dentro il clan	(RR) Toglierei */
	int				 illegal_pk;		/* Numero di killing illegali del clan		*/
	int				 strikes;			/* Numero di attacchi contro il clan		(FF) metterci anche il numero di assalti effettuati su altri clan */
	RANK_DATA		*rank[MAX_RANK];	/* Array con le informazioni dei rank		*/
	int				 rank_limit;		/* Rank massimo acquisibile					*/
	MEMBER_DATA 	*first_member;		/* Primo membro del clan					*/
	MEMBER_DATA 	*last_member;		/* Ultimo membro del clan					*/
	int				 member_limit;		/* Numero dei membri possibili nel clan		(FF) in futuro magari metterci un minimo, ma come MIN_ forse, un valore per incitare la ricerca di nuovi membri per il clan, per fondarlo */
	int				 members;			/* Numero totale dei membri del clan		*/
};


/*
 * Variabili
 */
extern CLAN_DATA		*first_clan;
extern CLAN_DATA		*last_clan;
extern int				 top_clan;
extern const CODE_DATA	code_clanflag[];
extern const CODE_DATA	code_clantype[];
extern const int		max_check_clanflag;
extern const int		max_check_clantype;


/*
 * Funzioni
 */
void		 load_clans		( void );
void		 fwrite_clan	( CLAN_DATA *clan );
void		 free_all_clans	( void );
CLAN_DATA	*get_clan		( const char *name, bool exact );
MEMBER_DATA *get_clan_member( const char *name, CLAN_DATA *clan, bool exact );


#endif	/* CLAN_H */
