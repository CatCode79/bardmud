/* Header per il modulo grammaticale */

#ifndef GRAMM_H
#define GRAMM_H


/*
 * Tipi di flag per l'articolo
 */
typedef enum
{
	ARTICLE_FEMININE,
	ARTICLE_PLURAL,
	ARTICLE_INDETERMINATE,
	ARTICLE_POSSESSIVE,
	MAX_ARTICLE
} article_flags;


/*
 * Funzioni
 */
bool	is_accent	( const char accent );
char   *myobj		( OBJ_DATA *obj );
char	gramm_ao	( CHAR_DATA *ch );
char   *get_article	( char *string, int bit, ... );
char   *aoran		( const char *str );	/* (RR) da cambiare con la get_article */


#endif	/* GRAMM_H */
