#ifndef WEAPON_H
#define WEAPON_H


/*
 * Enumerazione dei tipi di danno per la table_attack[]
 */
typedef enum
{
	DAMAGE_HIT,
	DAMAGE_SLICE,
	DAMAGE_STAB,
	DAMAGE_SLASH,
	DAMAGE_WHIP,
	DAMAGE_CLAW,
	DAMAGE_BLAST,
	DAMAGE_POUND,
	DAMAGE_CRUSH,
	DAMAGE_GREP,
	DAMAGE_BITE,
	DAMAGE_PIERCE,
	DAMAGE_SUCTION,
	DAMAGE_BOLT,
	DAMAGE_ARROW,
	DAMAGE_DART,
	DAMAGE_STONE,
	DAMAGE_PEA,
	DAMAGE_NONE,	/* serve ad indicare che non ha value di damage2 delle armi */
	MAX_DAMAGE
} damages_types;


/*
 * Enumerazione delle armi
 */
typedef enum
{
	WEAPON_DAGGER,
	WEAPON_FALCHION,
	WEAPON_AXE,
	WEAPON_SCIMITAR,
	WEAPON_DIRK,
	WEAPON_BASTARD,
	WEAPON_SHORTSWORD,
	WEAPON_KNUCKLEDUSTER,
	WEAPON_STAFF,
	WEAPON_HAMMER,
	WEAPON_MACE,
	WEAPON_FLAIL,
	WEAPON_WHIP,
	WEAPON_BOLA,
	WEAPON_COMPOSITEBOW,
	WEAPON_HEAVYCROSSBOW,
	WEAPON_CROSSBOW,
	WEAPON_LONGBOW,
	WEAPON_SHORTBOW,
	WEAPON_SLING,
	WEAPON_BATTLEAXE,
	WEAPON_LEATHERSTRAP,
	WEAPON_POLE,
	WEAPON_SWORD2H,
	WEAPON_JAVELIN,
	WEAPON_LANCE,
	MAX_WEAPON
} weapon_types;


/*
 * Struttura per la tabella dei danni
 * (FF) provare a fare modulare table_damage e mettere questa struttura in *.c
 */
struct damage_type
{
	CODE_DATA	code;
	char		gender;
	char	   **message_s;
	char	   **message_p;
};


/*
 * Variabile
 */
extern	const struct damage_type	table_damage[];
extern	const CODE_DATA				code_weapon[];
extern	const int					max_check_damage;
extern	const int					max_check_weapon;


/*
 * Funzione
 */
void	load_weapons( void );


#endif	/* WEAPON_H */
