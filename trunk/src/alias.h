#ifndef ALIAS_H
#define ALIAS_H

/****************************************************************************
 *                             Alias module                                 *
 ****************************************************************************/

/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997, 1998  Jesse DeFer and Heath Leach
 http://dotd.mudservices.com  dotd@dotd.mudservices.com
 ******************************************************/


/*
 * Struttura di una Alias
 */
struct alias_type
{
	ALIAS_DATA	*next;
	ALIAS_DATA	*prev;
	char		*name;
	char		*cmd;
};


/*
 * Variabili
 */
extern	int	top_alias;


/*
 * Funzioni
 */
void		free_aliases( CHAR_DATA *ch );
bool		check_alias	( CHAR_DATA *ch, const char *command, char *argument, bool exact );


#endif	/* ALIAS_H */
