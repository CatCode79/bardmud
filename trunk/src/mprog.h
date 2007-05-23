#ifndef MUD_PROG_H
#define MUD_PROG_H


/****************************************************************************\
 >					Header riguardante i Mud Programs						<
\****************************************************************************/


/* Enumerazione dei tipi di mud prog */
typedef enum
{
	MPTRIGGER_ERROR = -1,
	MPTRIGGER_ACT,
	MPTRIGGER_SPEECH,
	MPTRIGGER_RAND,
	MPTRIGGER_FIGHT,
	MPTRIGGER_DEATH,
	MPTRIGGER_HITPRCNT,
	MPTRIGGER_ENTER,
	MPTRIGGER_GREET,
	MPTRIGGER_GREET_ALL,
	MPTRIGGER_GIVE,
	MPTRIGGER_BRIBE,
	MPTRIGGER_TIME,
	MPTRIGGER_WEAR,
	MPTRIGGER_REMOVE,
	MPTRIGGER_SAC,
	MPTRIGGER_LOOK,
	MPTRIGGER_EXA,
	MPTRIGGER_ZAP,
	MPTRIGGER_GET,
	MPTRIGGER_DROP,
	MPTRIGGER_DAMAGE,
	MPTRIGGER_REPAIR,
	MPTRIGGER_RANDIW,
	MPTRIGGER_SPEECHIW,
	MPTRIGGER_PULL,
	MPTRIGGER_PUSH,
	MPTRIGGER_SLEEP,
	MPTRIGGER_REST,
	MPTRIGGER_LEAVE,
	MPTRIGGER_USE,
	MAX_MPTRIGGER
} mptrigger_types;


/*
 * Definizioni dei tipi di struttura
 */
typedef struct	mud_prog_com_list	MPROG_COM_LIST;


/*
 * Strutture di un azione di mud program dell'entità
 */
struct act_prog_data
{
	struct act_prog_data *next;
	void				 *vo;
};

struct mud_prog_act_list
{
	MPROG_ACT_LIST *next;
	char		   *buf;
	CHAR_DATA	   *ch;
	OBJ_DATA	   *obj;
	void		   *vo;
};


/*
 * Struttura della lista del comandi del mudprog
 */
struct mud_prog_com_list
{
	MPROG_COM_LIST	*prev;
	MPROG_COM_LIST	*next;
	char			*com;
};


/*
 * Struttura di un mudprog che va alle entità indicizzate
 */
struct mud_prog_data
{
	MPROG_DATA		*prev;
	MPROG_DATA		*next;
	int				 type;			/* Tipo di trigger */
	char			*arglist;		/* Argomento del trigger */
	bool			 triggered;		/* Serve per il TRIGGER_TIME */
	MPROG_COM_LIST	*first_mpcom;	/* Primo comando del mud prog */
	MPROG_COM_LIST	*last_mpcom;	/* Ultimo comando del mud prog */
};


/*
 * Variabili
 */
bool	MOBtrigger;		/* Se FALSE evita di triggare delle azioni dei pg */

CHAR_DATA						*supermob;
extern	struct act_prog_data	*mob_act_list;
extern	const CODE_DATA			 code_mptrigger[];
extern	const int				 max_check_mptrigger;


/*
 * Funzioni
 */
/* mprog.c */
void		fread_mudprog			( MUD_FILE *fp, MPROG_DATA **first_mudprog, MPROG_DATA **last_mudprog, bool save );
void		fwrite_mudprog			( MUD_FILE *fp, MPROG_DATA *mprog );
void		free_mudprog			( MPROG_DATA *mprog );
MPROG_DATA *copy_mudprog			( MPROG_DATA *source );
bool		is_same_mudprog			( MPROG_DATA *mprog1, MPROG_DATA *mprog2 );
void		mprog_wordlist_check	( char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *object, void *vo, int type );
void		mprog_percent_check		( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *object, void *vo, int type );
void		mprog_act_trigger		( char *buf, CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj, void *vo );
void		mprog_bribe_trigger		( CHAR_DATA *mob, CHAR_DATA *ch, int amount );
void		mprog_entry_trigger		( CHAR_DATA *mob );
void		mprog_give_trigger		( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj );
void		mprog_greet_trigger		( CHAR_DATA *mob );
void		mprog_fight_trigger		( CHAR_DATA *mob, CHAR_DATA *ch );
void		mprog_hitprcnt_trigger	( CHAR_DATA *mob, CHAR_DATA *ch );
void		mprog_death_trigger		( CHAR_DATA *killer, CHAR_DATA *mob );
void		mprog_random_trigger	( CHAR_DATA *mob );
void		mprog_speech_trigger	( char *txt, CHAR_DATA *mob );
void		mprog_time_trigger		( CHAR_DATA *mob );
void		send_mplog				( CHAR_DATA *mob, char *str, ... )	__attribute__ ( (format(printf, 2, 3)) );
void		rset_supermob			( ROOM_DATA *room);
void		release_supermob		( void );
void		init_supermob			( void );

/* mprog_trigger.c */
bool		has_trigger				( MPROG_DATA *mprog, const int type );

/* mprog_ifcheck.c */
bool	mprog_seval			( char* lhs, char* opr, char* rhs, CHAR_DATA *mob );
bool	mprog_veval			( int lhs, char* opr, int rhs, CHAR_DATA *mob );
int		mprog_do_ifcheck	( char *ifcheck, CHAR_DATA* mob, CHAR_DATA* actor, OBJ_DATA* obj, void *vo, CHAR_DATA *rndm );
void	mprog_translate		( char ch, char* t, CHAR_DATA* mob, CHAR_DATA* actor, OBJ_DATA* obj, void *vo, CHAR_DATA* rndm );
void	mprog_driver		( MPROG_DATA *mprog, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo );

void	set_supermob		( OBJ_DATA *obj );
bool	oprog_percent_check	( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type );
void	oprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type, OBJ_DATA *iobj );
void	rprog_percent_check	( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type );
void	rprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type, ROOM_DATA *room );

char   *oprog_type_to_name		( int type );
void	oprog_greet_trigger		( CHAR_DATA *ch );
void	oprog_speech_trigger	( char *txt, CHAR_DATA *ch );
void	oprog_random_trigger	( OBJ_DATA *obj );
bool	oprog_use_trigger		( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict, OBJ_DATA *targ, void *vo );
void	oprog_wear_trigger		( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_remove_trigger	( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_sac_trigger		( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_get_trigger		( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_damage_trigger	( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_repair_trigger	( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_drop_trigger		( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_examine_trigger	( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_zap_trigger		( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_pull_trigger		( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_push_trigger		( CHAR_DATA *ch, OBJ_DATA *obj );
void	oprog_act_trigger		( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,	OBJ_DATA *obj, void *vo );
void	update_obj_act			( void );

void	rprog_leave_trigger	( CHAR_DATA *ch );
void	rprog_enter_trigger	( CHAR_DATA *ch );
void	rprog_sleep_trigger	( CHAR_DATA *ch );
void	rprog_rest_trigger	( CHAR_DATA *ch );
void	rprog_rfight_trigger( CHAR_DATA *ch );
void	rprog_death_trigger	( CHAR_DATA *killer, CHAR_DATA *ch );
void	rprog_speech_trigger( char *txt, CHAR_DATA *ch );
void	rprog_random_trigger( CHAR_DATA *ch );
void	rprog_time_trigger	( CHAR_DATA *ch );
char   *rprog_type_to_name	( int type );
void	update_room_act		( void );
void	rprog_act_trigger	( char *buf, ROOM_DATA *room, CHAR_DATA *ch, OBJ_DATA *obj, void *vo );


#endif	/* MUDPROG_H */
