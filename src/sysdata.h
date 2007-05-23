#ifndef SYSDATA_H
#define SYSDATA_H


/****************************************************************************\
 >						File header del data config							<
\****************************************************************************/


/*
 * Enumerzione dei tipi di variabile e opzioni (FF) questa parte di codice andrà nel olc.h quando l'olc verrà rifatto
 */
typedef enum
{
	TYPE_NUMBER,
	TYPE_STRING,
	TYPE_BOOLEAN,
	MAX_TYPE
} type_types;


/*
 * Struttura del config
 * Viene utilizzato per impostare configurazioni varie e per
 *	registrare le statistiche di gioco
 */
struct system_data
{
	bool	check_adm_host;			/* Do we check admin's hosts? */
	bool	copyover;
	bool	copyover_reboot;		/* Riavvia di copyover quando il mud reboota */
	bool	deny_new_players;		/* New players cannot connect */
	bool	free_stealing;			/* Indica se lasciare libero lo stealing tra giocatori */
	bool	mascotte_restring;		/* Permette di restringare le mascotte */
	bool	mascotte_save;			/* Do mascottes save? */
	bool	name_resolving;			/* Hostnames are not resolved */
	bool	order_commands;			/* Indica se riordinare i comandi secondo l'utilizzo dei pg all'avvio */
	bool	pk_loot;				/* Pkill looting allowed? */
#ifdef T2_QUESTION
	bool	question_creation;		/* Indica se nella creazione chiederà le domande */
#endif
	bool	track_trough_door;		/* Indica se i track superano le porte chiuse o nascoste */
    bool	wiz_lock;				/* Il Mud è sotto wizlock? */

	int		bash_plr_vs_plr;		/* Bash mod player vs. player */
	int		bash_nontank;			/* Bash mod basher != primary attacker */
	int		dam_plr_vs_plr;			/* Damage mod player vs. player */
	int		dam_plr_vs_mob;			/* Damage mod player vs. mobile */
	int		dam_mob_vs_plr;			/* Damage mod mobile vs. player */
	int		dam_mob_vs_mob;			/* Damage mod mobile vs. mobile */
	int		gouge_plr_vs_plr;		/* Gouge mod player vs. player */
	int		gouge_nontank;			/* Gouge mod player != primary attacker */
	int		stun_plr_vs_plr;		/* Stun mod player vs. player */
	int		stun_regular;			/* Stun difficult */
	int		mod_dodge;				/* Divide dodge chance by */
	int		mod_parry;				/* Divide parry chance by */
	int		mod_tumble;				/* Divide tumble chance by */
	int		purge_after_month;		/* Cancella i pg inattivi da quel tot di mesi */
	int		purge_min_hour;			/* Cancella i pg inattivi che hanno giocato meno di tot ore */
	int		purge_min_level;		/* Cancella i pg inattivi che non hanno raggiunto il tot livello */
	int		save_frequency;			/* How old to autosave someone */
	int		save_level;				/* A quale livello minimo si può salvare il pg */

	int		control;				/* Descrittore di controllo */
	int		death_hole_mult;		/* Multiplicatore per la hole di perdita px nella morte */
	void   *dlHandle;				/* DLSym Snippet */
	int		lvl_group_diff;			/* Differenza massima di livello tra i membri del gruppo e il loro capo */
	int		max_socket_con;			/* Numero massimo di connessioni login permesse */
	int		mud_port;				/* La porta del Mud */
};


/*
 * Struttura con le informazioni di olc building per il sysdata
 * Servono anche per identificare le opzione di linea di comando dell'avvio del mud
 */
struct olc_sysdata_type
{
	char	*label;			/* Nome dell'opzione da passare nella riga di comando */
	int		 type;			/* Tipo che si aspetta l'opzione (int, char, bool etc etc) */
/*	int		 def_number;*/	/* Il numero di default ad un tipo di numero o booleano */
/*	char	*def_string;*/	/* La stringa di default se il tipo è una stringa */
	int		 limit_min;		/* Il minimo accettato se il tipo è un numero o booleano */
	int		 limit_max;		/* Il massimo accettato se il tipo è un numero o booleano */
	char	*descr;			/* Descrizione dell'opzione */
};


/*
 * Variabili
 */
		struct system_data				sysdata;
extern	const struct olc_sysdata_type	tableolc_option[];


/*
 * Funzioni
 */
void	get_sysdata_options	( char **argv );
void	load_sysdata		( void );
void	save_sysdata		( void );


#endif	/* CONFIG_H */
