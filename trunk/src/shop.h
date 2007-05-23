#ifndef SHOP_H
#define SHOP_H


/****************************************************************************\
 >							Header shop module								<
\****************************************************************************/


/*
 * Struttura per i Negozi.
 */
#define MAX_TRADE	5


/*
 * Variabili esterne
 */
extern	SHOP_DATA	   *first_shop;
extern	SHOP_DATA	   *last_shop;
extern	REPAIR_DATA	   *first_repair;
extern	REPAIR_DATA	   *last_repair;
extern	VNUM			top_shop;
extern	VNUM			top_repair;


struct shop_data
{
	SHOP_DATA *next;					/* Next shop in list			*/
	SHOP_DATA *prev;					/* Previous shop in list		*/
	VNUM	   keeper;					/* Vnum of shop keeper mob		*/
	int		   buy_type[MAX_TRADE];		/* Item types shop will buy		*/
	int		   profit_buy;				/* Cost multiplier for buying	*/
	int		   profit_sell;				/* Cost multiplier for selling	*/
	int		   open_hour;				/* First opening hour			*/
	int		   close_hour;				/* First closing hour			*/
};


#define INIT_WEAPON_CONDITION	12
#define MAX_ITEM_IMPACT			30	/* (TT) interessante */


#define MAX_FIX			3
#define SHOP_FIX		1
#define SHOP_RECHARGE	2

struct repairshop_data
{
	REPAIR_DATA *next;					/* Next shop in list			*/
	REPAIR_DATA *prev;					/* Previous shop in list		*/
	VNUM		 keeper;				/* Vnum of shop keeper mob		*/
	int			 fix_type[MAX_FIX];		/* Item types shop will fix		*/
	int			 profit_fix;			/* Cost multiplier for fixing	*/
	int			 shop_type;				/* Repair shop type				*/
	int			 open_hour;				/* First opening hour			*/
	int			 close_hour;			/* First closing hour			*/
};


/*
 * Funzioni
 */
void	load_shops		( MUD_FILE *fp );
void	load_repairs	( MUD_FILE *fp );
void	free_all_shops	( void );
void	free_all_repairs( void );


#endif	/* SHOP_H */
