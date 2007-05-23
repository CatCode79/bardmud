#ifndef POLYMORPH_H
#define POLYMORPH_H


/****************************************************************************\
 >						File Header del Polimorfismo						<
\****************************************************************************/


/*
 * Variabili
 */
extern MORPH_DATA	*first_morph;
extern MORPH_DATA	*last_morph;

/*
 * Definizioni
 */
#define MORPH_DIR				"../morphs/"	/* Dir dati dei morph */
#define MORPH_LIST	MORPH_DIR	"morphs.lst"	/* Lista dei Morph	  */


/*
 * Strutture riguardo i morph.
 */
struct char_morph
{
	MORPH_DATA *morph;
	BITVECTOR  *affected_by;	/* New affected_by added			 */
	BITVECTOR  *immune;			/* Immunities added					 */
	BITVECTOR  *resistant;		/* Resistances added				 */
	BITVECTOR  *suscept;		/* Suscepts added					 */
	BITVECTOR  *no_affected_by;	/* Prevents affects from being added */
	BITVECTOR  *no_immune;		/* Prevents Immunities				 */
	BITVECTOR  *no_resistant;	/* Prevents resistances				 */
	BITVECTOR  *no_suscept;		/* Prevents Susceptibilities		 */
	int			timer;			/* How much time is left			 */
	int			points[MAX_POINTS];
	int			attr [MAX_ATTR];
	int			sense[MAX_SENSE];
	int			ac;
	int			damroll;
	int			dodge;
	int			hitroll;
	int			parry;
	int			tumble;
	int			saving_throw[MAX_SAVETHROW];
};


#define MAX_MORPH_OBJ	3

struct morph_data
{
	MORPH_DATA *prev;						/* File di morph precedente					 */
	MORPH_DATA *next;						/* File di morph seguente					 */
	BITVECTOR  *classes;					/* Classi not allowed to use this			 */
	BITVECTOR  *races;						/* Races not allowed to use this			 */
	BITVECTOR  *affected_by;				/* New affected_by added					 */
	BITVECTOR  *immune;						/* Immunities added							 */
	BITVECTOR  *resistant;					/* Resistances added						 */
	BITVECTOR  *suscept;					/* Suscepts added							 */
	BITVECTOR  *no_affected_by;				/* Prevents affects from being added		 */
	BITVECTOR  *no_immune;					/* Prevents Immunities						 */
	BITVECTOR  *no_resistant;				/* Prevents resistances						 */
	BITVECTOR  *no_suscept;					/* Prevents Susceptibilities				 */
	char	   *damroll;
	char	   *help;						/* What player sees for info on morph		 */
	char	   *hitroll;
	char	   *key_words;					/* Keywords added to your name				 */
	char	   *long_desc;					/* New long_desc for player					 */
	char	   *short_desc;					/* New short desc for player				 */
	char	   *description;
	char	   *morph_other;				/* What others see when you morph			 */
	char	   *morph_self;					/* What you see when you morph				 */
	char	   *name;						/* Name used to polymorph into this			 */
	char	   *no_skills;					/* Prevented Skills							 */
	char	   *skills;
	char	   *unmorph_other;				/* What others see when you unmorph			 */
	char	   *unmorph_self;				/* What you see when you unmorph			 */
	int			defpos;						/* Default position							 */
	int			obj[MAX_MORPH_OBJ];			/* Object needed to morph you				 */
	int			timer;						/* Timer for how long it lasts				 */
	int			used;						/* How many times has this morph been used	 */
	VNUM		vnum;						/* Unique identifier						 */
	char	   *points[MAX_POINTS];			/* Punti di vita aggiunti					 */
	int			points_used[MAX_POINTS];	/* Amount of points used to morph			 */
	int			attr[MAX_ATTR];				/* Ammontare dell'attributo guadagnato/perso */
	int			sense[MAX_SENSE];			/* Ammontare del senso guadagnato/perso		 */
	int			ac;
	int			dayfrom;					/* Starting Day you can morph into this		 */
	int			dayto;						/* Ending Day you can morph into this		 */
	int			dodge;						/* Percent of dodge added IE 1 = 1%			 */
	int			gloryused;					/* Amount of glory used to morph			 */
	int			level;						/* Minimum level to use this morph			 */
	int			parry;						/* Percent of parry added IE 1 = 1%			 */
	int			saving_throw[MAX_SAVETHROW];	/* Saving throw adjusted					 */
	int			sex;						/* The sex that can morph into this			 */
	int			timefrom;					/* Hour starting you can morph				 */
	int			timeto;						/* Hour ending that you can morph			 */
	int			tumble;						/* Percent of tumble added IE 1 = 1%		 */
	bool		no_cast;					/* Can you cast a spell to morph into it	 */
	bool		objuse[MAX_MORPH_OBJ];		/* Objects needed to morph					 */
};


/*
 * Funzioni globali
 */
void		load_morphs			( void );
void		fwrite_morph_data	( CHAR_DATA *ch, MUD_FILE *fp );
void		fread_morph_data	( CHAR_DATA *ch, MUD_FILE *fp );
void		clear_char_morph	( CHAR_MORPH *morph );
CHAR_MORPH *make_char_morph		( MORPH_DATA *morph );
void		free_char_morph		( CHAR_MORPH *morph );
void		unmorph_all			( MORPH_DATA *morph );
MORPH_DATA *get_morph			( char *arg );
MORPH_DATA *get_morph_vnum		( int arg );
int			do_morph_char		( CHAR_DATA *ch, MORPH_DATA *morph );
MORPH_DATA *find_morph			( CHAR_DATA *ch, char *target, bool is_cast );
void		do_unmorph_char		( CHAR_DATA *ch );
void		send_morph_message	( CHAR_DATA *ch, MORPH_DATA *morph, bool is_morph );
bool		can_morph			( CHAR_DATA *ch, MORPH_DATA *morph, bool is_cast );
void		do_morph			( CHAR_DATA *ch, MORPH_DATA *morph );
void		do_unmorph			( CHAR_DATA *ch );
void		write_morph_list	( void );
void		fwrite_morph		( MORPH_DATA *morph );
void		free_all_morphs		( void );
void		morph_defaults		( MORPH_DATA *morph );


#endif	/* POLYMORPH_H */
