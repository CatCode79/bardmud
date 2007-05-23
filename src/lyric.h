#ifndef LYRIC_H
#define LYRIC_H


/*
 * Struttura di un pentagramma
 * Cioè una singola riga del brano che il bardo canta
 */
struct pentagram_data
{
	PENTAGRAM_DATA	*prev;
	PENTAGRAM_DATA	*next;
	int				 rhythm;	/* Numero di pulse di attesa per iniziare questo pentagramma */
	char			*action;	/* canale o emote con il quale canta ed esprime il canto */
};


/*
 * Struttura di una lirica
 * Cioè tutto il brano che un bardo canta
 */
struct lyric_data
{
	LYRIC_DATA		*prev;
	LYRIC_DATA		*next;
	char			*name;		/* Nome della lirica, con cui si può caricarla */
	char			*title;		/* Titolo della lirica */
	bool			 secret;	/* Se è conosciuta pubblicamente o solo da pochi */
	int				 vnum_mob;	/* Vnum del mob che canta quella canzone (GG) */
	int				 language;	/* La lingua con cui può essere cantata */
	BITVECTOR		*races;		/* Le razza che la conoscono */
	PENTAGRAM_DATA	*first_pentagram;
	PENTAGRAM_DATA	*last_pentagram;
};


/*
 * Variabile esterna
 */
extern	int	top_lyric;


/*
 * Funzioni
 */
void	load_lyrics		( void );
void	update_lyric	( void );
void	free_all_lyrics	( void );


#endif	/* LYRIC_H */
