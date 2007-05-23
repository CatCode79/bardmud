#ifndef CONST_H
#define CONST_H


/* Numero massimo di VNUM utilizzabili */
#define MAX_VNUM		2000000000
#define MAX_VNUM_AREA	MAX_VNUM/2


/*
 * Lunghezza minima e massima per alcune stringhe spesso utilizzate
 * Attenzione, modificando queste, verranno potenzialmente sballati gli
 *	output di alcune funzioni come lo score
 * Questi define a volte esistono più per dare un limite ed un riferimento sul
 *	massimo veloce da trovare
 */
#define MIN_LEN_NAME_PG		 3		/* Lunghezza minima per un nome adeguato, (2 è da evitare)	*/
#define MAX_LEN_NAME_PG		12		/* Lunghezza massima per i nomi e i cognomi dei char		*/
#define MAX_LEN_TITLE		31		/* Lunghezza max per il title								*/


/*
 * Valori di default di vario genere
 */
#define DEF_MIN_ATTR		30		/* Valore minimo di attributo o senso acquisibile causalmente durante la creazione */
#define DEF_MAX_ATTR		70		/* Valore massimo di attributo o senso acquisibile causalmente durante la creazione */
#define DEF_HEIGHT		   165		/* Altezza di default (in cm)			*/
#define DEF_WEIGHT		 60000		/* Peso di default (in grammi)			*/


/*
 * Parametri di gioco.
 */
#define MAX_ROOM_EXITS		20		/* Uscite massime gestite in 1 stanza */

#define MAX_LAYERS			 8		/* Maximum clothing layers */

#define MAX_LEARN_SKELL    100		/* Massimo che un pg può imparare di una skill (GG) */
#define MAX_LEARN_ATTR	   100		/* Massimo che un pg può imparare di un attributo (e ad un senso) */
#define MAX_LEARN_POINT	  9999		/* Massimo che un pg può acquisire riguardo ai punti */

#define MAX_SKILL			500		/* Limite di skill caricabili */


/*
 * Versione e release del codice BARD
 */
#define MUD_NAME			"&RT&Yerra &RS&Yecunda&w"
#define MUD_CODE_NAME		"&gBARD&w"
#define MUD_VERSION_MAJOR 	"0"
#define MUD_VERSION_MINOR 	"6u"
#define MUD_RELEASE_NAME	"&gDriade&w"
#define MUD_TEAM_NAME		"&RT&Y2&WTeam&w"
#define MUD_URL				"www.terra-secunda.net"


/*
 * Struttura per le statistiche di default
 */
struct mob_stats_type
{
	int		ac;
	int		hitnodice;
	int		hitsizedice;
	int		hitplus;
	int		damnodice;
	int		damsizedice;
	int		damplus;
};


/*
 * Struttura dei punti.
 */
struct points_type
{
	CODE_DATA	 code;			/* Tabella di codici dei punti */
	int			 attr_pri;		/* Attributo da cui si ricava il punteggio */
	int			 attr_sec;		/* Attributo secondario da cui si ricava il punteggio */
	int			 gain_base;		/* Il minimo che può gainare ad ogni update_point */
	int			 div;			/* Divisore per il calcolo della gain in points_gain */
	float		 gain_sit;		/* Moltiplicatore per chi riposa da seduto */
	float		 gain_rest;		/* Moltiplicatore per chi riposa ad ogni update_point */
	float		 gain_sleep;	/* Moltiplicatore per chi dorme ad ogni update_point */
	int			 start;			/* Punteggio inizale nella creazione del pg	*/
};


/*
 * Struttura per la tabella delle unità di misura
 */
struct measure_unit_type
{
	char	*unit_kg;
	char	*unit_gr;
	char	*unit_mt;
	char	*unit_cm;

	char	*unit_kgs;
	char	*unit_grs;
	char	*unit_mts;
	char	*unit_cms;

	char	*sigle_kg;
	char	*sigle_gr;
	char	*sigle_mt;
	char	*sigle_cm;
};


/*
 * Struttura relativa ai bonus e malus degli attributi
 */
struct attr_bonus_type
{
	int		tohit;
	int		todam;
	int		defense;
};


/*
 * Variabili esterne
 */
extern	const struct points_type		table_points[];					/* Tabella dei punti */
extern		  struct mob_stats_type		table_mob_stats[LVL_LEGEND];	/* (CC) non ancora una costante ma lo sarà */
extern	const int						table_experience[LVL_LEGEND];	/* Tabella dei punti di esperienza per livello */
extern	const struct attr_bonus_type	table_attr_bonus[MAX_LEARN_ATTR+1];


#endif	/* CONST_H */
