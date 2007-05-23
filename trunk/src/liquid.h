#ifndef LIQUID_H
#define LIQUID_H


/*
 * Definizioni
 */
#define LIQ_WATER	 0
#define MAX_LIQ		18


/*
 * Struttura di un liquido
 */
struct liquid_data
{
	char   *name;
	char   *color;
	int		affect[3];
};


/*
 * Variabili
 */
extern const struct liquid_data		table_liquid[];		/* Tipi di liquidi */


#endif	/* LIQUID_H */
