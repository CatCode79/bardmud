#ifndef NOTE_H
#define NOTE_H


/*
 * Definizioni dei tipi di nota
 */
#define NOTE_NONE		0	/* nessuno, non impostato ancora */
#define NOTE_NOTE		1	/* nota offgdr */
#define NOTE_MAIL		2	/* lettera gdr */
#define NOTE_BOARD		3	/* messaggio pubblico */
#define MAX_NOTE		4

/*
 * Definizioni per le mail e note con voto
 */
#define VOTE_NONE		0
#define VOTE_OPEN		1
#define VOTE_CLOSED 	2
#define MAX_VOTE		3


/*
 * Struttura di una Nota
 */
struct note_data
{
	NOTE_DATA	*next;
	NOTE_DATA	*prev;
	char 		*sender;
	char 		*date;
	char 		*to_list;
	char 		*subject;
	int			 type;
	int			 voting;
	char 		*votes_yes;
	char 		*votes_no;
	char 		*abstentions;
	char 		*text;
	time_t		 expire;		/* (GG) non ancora inserita */
	time_t		 date_stamp;	/* (GG) non ancora inserita */
};


/*
 * Struttura di una Board
 */
struct board_data
{
	BOARD_DATA *next;				/* Prima bachca della lista */
	BOARD_DATA *prev;				/* Bacheca precedente nella lista */
	NOTE_DATA  *first_mail;			/* Prima mail della bacheca */
	NOTE_DATA  *last_mail;			/* Ultima mail della bacheca */
	OBJ_DATA   *board_obj;			/* Oggetto della bacheca */
	char 	   *mail_file;			/* Nome del file in cui vengono salvati le mail della bacheca */
	char 	   *clan_read;			/* Permette ai membri di uno o più Clan di leggere ad una bacheca */
	char 	   *clan_post;			/* Permette ai membri di uno o più Clan di postare in una bacheca */
	char 	   *clan_remove;		/* Permette ai membri di uno o più Clan di rimuovere posto da una bacheca */
	char 	   *extra_readers;		/* Permette ad un pg o più di leggere ad una bacheca */
	char 	   *extra_posters;		/* Permette ad un pg o più di postare ad una bacheca */
	char 	   *extra_removers;		/* Permette ad un pg o più di rimuovere post da una bacheca */
	int			min_trust_read;		/* Trust minima per accedere alla bacheca */
	int			min_trust_post;		/* Trust minima per postare nella bacheca */
	int			min_trust_remove;	/* Trust minima per rimuovere i messaggi nella bacheca */
	int			num_posts;			/* Numero di mail in questa bacheca */
	int			max_posts;			/* Numero massimo di mail postabili nella bacheca */
};


/*
 * Variabili esterne
 */
extern	int				top_board;
extern	int				top_note;
extern	const CODE_DATA code_note[];
extern	const CODE_DATA code_vote[];
extern	const int		max_check_note;
extern	const int		max_check_vote;


/*
 * Funzioni globali
 */
void		load_boards		 	 ( void );
void		load_player_notes	 ( void );
void		rename_notes		 ( CHAR_DATA *ch, const char *newname );
void		free_all_boards		 ( void );
void		free_all_player_notes( void );
BOARD_DATA *get_board		 	 ( OBJ_DATA *obj );
void		free_note		 	 ( NOTE_DATA *note );
void		note_count		 	 ( CHAR_DATA *ch, const char *date, const int note_type );


#endif	/* NOTE_H */
