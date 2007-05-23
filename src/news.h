#ifndef NEWS_H
#define NEWS_H


/*
 * Definizioni di un tipo di struttura news
 */
typedef	struct news_data	NEWS_DATA;


/*
 * Tipi di novità
 */
typedef enum
{
	NEWS_NEW,
	NEWS_BUG,
	NEWS_EVENT,
	NEWS_AREA,
	MAX_NEWS
} news_types;


/*
 * Struttura di news
 */
struct news_data
{
	NEWS_DATA	*next;
	NEWS_DATA	*prev;
	int			 type;
	char		*date;
	char		*author;
	char		*title;
	char		*text;
};


/*
 * Variabili
 */
extern	const CODE_DATA		code_news[];
extern	const int			max_check_news;
extern	int					top_news;


/*
 * Funzioni
 */
void	load_news		( void );
void	free_all_news	( void );


#endif	/* NEWS_H */
